#include "Object/Object.h"
#include "ObjectPool.h"

Array<Object*> RootSet;

void MarkObjectsInPoolAsUnreachable(ObjectPool& pool) {
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

            SetFlag(header->Flags, ObjectFlags::Unreachable);
        }
    }
}

void MarkObjectsReachableFrom(Object* object) {
    Array<UniquePtr<ObjectField>> fields;
    object->GetObjectFields(fields);
    for (UniquePtr<ObjectField>& field : fields) {
        if (field->Type == ObjectFieldType::Object) {
            ObjectObjectField& objectField = static_cast<ObjectObjectField&>(*field);
            Object** referencedObject = objectField.GetValuePtr(object);
            if (!*referencedObject) {
                continue;
            }

            ObjectHeader* referencedObjectHeader = GetHeaderForObject(*referencedObject);
            if (!referencedObjectHeader || referencedObjectHeader->Magic != ObjectHeader::RequiredMagic) {
                // TODO: this is bad, we should really warn about it
                continue;
            }

            if (HasAnyFlags(referencedObjectHeader->Flags, ObjectFlags::Unreachable)) {
                UnsetFlag(referencedObjectHeader->Flags, ObjectFlags::Unreachable);
                MarkObjectsReachableFrom(*referencedObject);
            }

            // Remove pointers to destroyed / destroying objects
            if (HasAnyFlags(referencedObjectHeader->Flags, ObjectFlags::IsBeingDestroyed | ObjectFlags::IsDestroyed)) {
                *referencedObject = nullptr;
            }
        } else if (field->Type == ObjectFieldType::Array) {
            ArrayObjectField& arrayField = static_cast<ArrayObjectField&>(*field);
            if (arrayField.ItemType != ObjectFieldType::Object) {
                continue;
            }

            Array<Object*>* objects = (Array<Object*>*) arrayField.GetUntypedValuePtr(object);
            for (Object*& referencedObject : *objects) {
                if (!referencedObject) {
                    continue;
                }
                ObjectHeader* referencedObjectHeader = GetHeaderForObject(referencedObject);
                if (!referencedObjectHeader || referencedObjectHeader->Magic != ObjectHeader::RequiredMagic) {
                    // TODO: this is bad, we should really warn about it
                    continue;
                }

                if (HasAnyFlags(referencedObjectHeader->Flags, ObjectFlags::Unreachable)) {
                    UnsetFlag(referencedObjectHeader->Flags, ObjectFlags::Unreachable);
                    MarkObjectsReachableFrom(referencedObject);
                }

                // Remove pointers to destroyed / destroying objects
                if (HasAnyFlags(referencedObjectHeader->Flags, ObjectFlags::IsBeingDestroyed | ObjectFlags::IsDestroyed)) {
                    referencedObject = nullptr;
                }
            }
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
            }
        }
    }
}

void CollectGarbage() {
    // Mark
    for (ObjectPool& pool : ObjectPool::GetPools()) {
        MarkObjectsInPoolAsUnreachable(pool);
    }

    // Sweep
    for (Object* object : RootSet) {
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
    RootSet.push_back(object);
    SetFlag(GetHeaderForObject(object)->Flags, ObjectFlags::InRootSet);
}

void RemoveFromRootSet(Object* object) {
    RootSet.erase(std::remove(RootSet.begin(), RootSet.end(), object), RootSet.end());
    UnsetFlag(GetHeaderForObject(object)->Flags, ObjectFlags::InRootSet);
}
