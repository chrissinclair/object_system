#include "Object/Object.h"
#include "ObjectPool.h"

Array<Object*>& GetRootSet() {
    static Array<Object*> rootSet;
    return rootSet;
}

void MarkObjectsReachableFrom(Object* object);
void MarkObjectsReachableFromObject(Object* object, Object** referencer, ObjectObjectField& objectField);
void MarkObjectsReachableFromStruct(void* structBase, Class* structClass);
void MarkObjectsReachableFromArray(void* arrayBase, ArrayObjectField& arrayField);
void MarkObjectsReachableFromObjectField(Object* object, ObjectObjectField& objectField);
void MarkObjectsReachableFromStructField(Object* object, StructObjectField& structField);
void MarkObjectsReachableFromArrayField(Object* object, ArrayObjectField& arrayField);

void MarkObjectsReachableFromObject(Object** objectReference) {
     if (!objectReference || !*objectReference) {
        return;
     }

    ObjectHeader* objectHeader = GetHeaderForObject(*objectReference);
    if (!objectHeader || objectHeader->Magic != ObjectHeader::RequiredMagic) {
        // TODO: this is bad, we should really warn about it
        return;
    }

    if (HasAnyFlags(objectHeader->Flags, ObjectFlags::Unreachable)) {
        UnsetFlag(objectHeader->Flags, ObjectFlags::Unreachable);
        MarkObjectsReachableFrom(*objectReference);
    }

    // Remove pointers to destroyed / destroying objects
    if (HasAnyFlags(objectHeader->Flags, ObjectFlags::IsBeingDestroyed | ObjectFlags::IsDestroyed)) {
        *objectReference = nullptr;
    }
}

void MarkObjectsReachableFromObjectField(Object* object, ObjectObjectField& objectField) {
    Object** referencedObject = objectField.GetValuePtr(object);
    MarkObjectsReachableFromObject(referencedObject);
}

void MarkObjectsReachableFromArray(void* arrayBase, ArrayObjectField& arrayField) {
    if (arrayField.InnerType->Type == ObjectFieldType::Object) {
        Array<Object*>* objects = (Array<Object*>*) arrayBase;
        for (Object*& referencedObject : *objects) {
            MarkObjectsReachableFromObject(&referencedObject);
        }
    } else if (arrayField.InnerType->Type == ObjectFieldType::Struct) {
        Class* structClass = arrayField.InnerType->As<StructObjectField>()->StructType;
        const usize structSize = structClass->Size();
        Array<u8>* structs = (Array<u8>*) arrayBase;
        const usize numStructs = structs->size() / structSize;
        for (u32 structIndex = 0; structIndex < numStructs; ++structIndex) {
            MarkObjectsReachableFromStruct(structs->data() + (structIndex * structSize), structClass);
        }
    }
}
void MarkObjectsReachableFromArrayField(Object* object, ArrayObjectField& arrayField) {
    MarkObjectsReachableFromArray(arrayField.GetUntypedValuePtr(object), arrayField);
}

void MarkObjectsReachableFromStructField(Object* object, StructObjectField& structField) {
    MarkObjectsReachableFromStruct(structField.GetValuePtr(object), structField.StructType);
}

void MarkObjectsReachableFromStruct(void* structBase, Class* structClass) {
    for (const UniquePtr<ObjectField>& field : structClass->Fields()) {
        void* fieldStructBase = field->GetUntypedValuePtr(structBase);
        if (field->Type == ObjectFieldType::Object) {
            MarkObjectsReachableFromObject(field->As<ObjectObjectField>()->GetValuePtr(structBase));
        } else if (field->Type == ObjectFieldType::Struct) {
            MarkObjectsReachableFromStruct(fieldStructBase, field->As<StructObjectField>()->StructType);
        } else if (field->Type == ObjectFieldType::Array) {
            MarkObjectsReachableFromArray(fieldStructBase, *field->As<ArrayObjectField>());
        }
    }
}

void MarkObjectsReachableFrom(Object* object) {
    const Array<UniquePtr<ObjectField>>& fields = object->GetClass()->Fields();
    for (const UniquePtr<ObjectField>& field : fields) {
        if (field->Type == ObjectFieldType::Object) {
            MarkObjectsReachableFromObjectField(object, *field->As<ObjectObjectField>());
        } else if (field->Type == ObjectFieldType::Struct) {
            StructObjectField* structField = field->As<StructObjectField>();
            MarkObjectsReachableFromStructField(object, *structField);
        } else if (field->Type == ObjectFieldType::Array) {
            ArrayObjectField* arrayField = field->As<ArrayObjectField>();
            if (arrayField->InnerType->Type != ObjectFieldType::Object && arrayField->InnerType->Type != ObjectFieldType::Struct) {
                continue;
            }
            MarkObjectsReachableFromArrayField(object, *arrayField);
        }
    }
}

void FreeUnreachableObjectsInPool(ObjectPool& pool) {
    u64 stride = pool.PoolElementSize + sizeof(ObjectHeader);
    for (Array<u8>& block : pool.GetBlocks()) {
        const u8* end = block.data() + block.size();
        for (u8* headerAddress = block.data(); headerAddress < end; headerAddress += stride) {
            ObjectHeader* header = (ObjectHeader*) headerAddress;
            if (header->Magic != ObjectHeader::RequiredMagic) {
                // TODO: This is bad, we should really warn about it
                continue;
            }

            if (!HasAnyFlags(header->Flags, ObjectFlags::Allocated)) {
                continue;
            }

            if (HasAnyFlags(header->Flags, ObjectFlags::Unreachable)) {
                Object* object = (Object*) (header + 1);
                if (!HasAnyFlags(header->Flags, ObjectFlags::IsBeingDestroyed | ObjectFlags::IsDestroyed)) {
                    object->Destroy();
                }
                if (!HasAnyFlags(header->Flags, ObjectFlags::IsDestroyed)) {
                    object->TryCompleteDestruction();
                }
                if (HasAnyFlags(header->Flags, ObjectFlags::IsDestroyed)) {
                    pool.DestroyObject(object);
                }
            } else {
                // Reset for next GC run
                SetFlag(header->Flags, ObjectFlags::Unreachable);
            }
        }
    }
}

void CollectGarbage() {
    // Mark
    for (Object* object : GetRootSet()) {
        ObjectHeader* header = GetHeaderForObject(object);
        if (!header || header->Magic != ObjectHeader::RequiredMagic) {
            // TODO: This is bad, we should really warn about it
            continue;
        }

        if (!HasAnyFlags(header->Flags, ObjectFlags::Unreachable)) {
            // Already traced this object
            continue;
        }
        UnsetFlag(header->Flags, ObjectFlags::Unreachable);
        MarkObjectsReachableFrom(object);
    }

    // Free
    for (ObjectPool& pool : ObjectPool::GetPools()) {
        FreeUnreachableObjectsInPool(pool);
    }
}

void AddToRootSet(Object* object) {
    GetRootSet().push_back(object);
    SetFlag(GetHeaderForObject(object)->Flags, ObjectFlags::InRootSet);
}

void RemoveFromRootSet(Object* object) {
    Array<Object*>& roots = GetRootSet();
    roots.erase(std::remove(roots.begin(), roots.end(), object), roots.end());
    UnsetFlag(GetHeaderForObject(object)->Flags, ObjectFlags::InRootSet);
}
