#include "Object/Object.h"
#include "GarbageCollection.h"
#include "ObjectPool.h"

Object::~Object() {
}

ObjectFlags Object::GetFlags() const {
    return GetHeaderForObject(this)->Flags;
}

u32 Object::TypeId() const { return 'OBJT'; }
String Object::TypeName() const { return "Object"; }
const Array<UniquePtr<ObjectField>>& Object::GetObjectFields() const {
    if (IsValid(GetClass())) {
        return GetClass()->Fields();
    }
    static Array<UniquePtr<ObjectField>> empty;
    return empty;
}
void Object::GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const {
}

void Object::AddToRootSet() {
    ::AddToRootSet(this);
}

void Object::RemoveFromRootSet() {
    ::RemoveFromRootSet(this);
}

void Object::Destroy() {
    ObjectHeader* header = GetHeaderForObject(this);
    SetFlag(header->Flags, ObjectFlags::IsBeingDestroyed);
    UnsetFlag(header->Flags, ObjectFlags::IsDestroyed);
    OnBeginDestroy();
    TryCompleteDestruction();
}

void Object::TryCompleteDestruction() {
    ObjectHeader* header = GetHeaderForObject(this);
    if (HasAnyFlags(header->Flags, ObjectFlags::IsBeingDestroyed) && IsDestroyFinished()) {
        OnEndDestroy();
        SetFlag(header->Flags, ObjectFlags::IsDestroyed);
        UnsetFlag(header->Flags, ObjectFlags::IsBeingDestroyed);
    }
}

void Object::OnBeginDestroy() {
}

bool Object::IsDestroyFinished() const {
    return true;
}

void Object::OnEndDestroy() {
}

u32 Object::GetGeneration() const {
    return GetHeaderForObject(this)->Generation;
}

void Object::CollectGarbage() {
    ::CollectGarbage();
}

void* Detail::AllocObject(const u32 objectSize) {
    return ObjectPool::AllocateObject(objectSize);
}

Object* NewObject(Class* objectClass) {
    if (objectClass == StaticClass<Class>()) {
        return NewObject<Class>();
    }

    Object* object = (Object*) Detail::AllocObject(objectClass->Size());
    if (!object) {
        return nullptr;
    }
    objectClass->Construct(object);
    object->classInstance = objectClass;
    return object;
}

template<>
Class* NewObject<Class>() {
    void* object = Detail::AllocObject(sizeof(Class));
    if (!object) {
        return nullptr;
    }
    new (object) Class();
    return (Class*) object;
}

bool IsValid(const Object* object) {
    if (object == nullptr) {
        return false;
    }

    const ObjectFlags flags = GetHeaderForObject(object)->Flags;
    return HasAnyFlags(flags, ObjectFlags::Allocated) && !HasAnyFlags(flags, ObjectFlags::IsBeingDestroyed | ObjectFlags::IsDestroyed);
}

WeakObjectPtrBase::WeakObjectPtrBase(Object* object) {
    if (::IsValid(object)) {
        this->object = object;
        this->generation = GetHeaderForObject(object)->Generation;
    }
}

bool WeakObjectPtrBase::IsValid() const {
    return ::IsValid(object) && GetHeaderForObject(object)->Generation == generation;
}

const Object* WeakObjectPtrBase::Get() const {
    return IsValid() ? object : nullptr;
}
Object* WeakObjectPtrBase::Get() {
    return IsValid() ? object : nullptr;
}

struct StrongObjectPtrManager : Object {
    virtual u32 TypeId() const override { return 'SOPM'; }
    virtual String TypeName() const override { return "StrongObjectPtrManager"; }
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(objects);
    }

    i32 Register(Object* object) {
        const i32 numberOfObjects = objects.size();
        for (i32 index = 0; index < numberOfObjects; ++index) {
            if (objects[index] == nullptr) {
                objects[index] = object;
                return index;
            }
        }

        objects.push_back(object);
        return numberOfObjects;
    }

    void Unregister(i32 index) {
        if (index >= 0 && index < objects.size()) {
            objects[index] = nullptr;

            // Try to shrink the array if possible
            auto nextAssignedObjectIt = std::find_if(objects.begin() + index, objects.end(), [](const Object* object) {
                return object != nullptr;
            });
            if (nextAssignedObjectIt == objects.end()) {
                objects.erase(objects.begin() + index, objects.end());
            }
        }
    }

    Array<Object*> objects;
};

DECLARE_OBJECT(StrongObjectPtrManager)
IMPL_OBJECT(StrongObjectPtrManager, Object)

StrongObjectPtrManager* StaticStrongObjectPtrManager() {
    static StrongObjectPtrManager* instance = NewObject<StrongObjectPtrManager>();
    if (!HasAnyFlags(instance->GetFlags(), ObjectFlags::InRootSet)) {
        instance->AddToRootSet();
    }
    return instance;
}

StrongObjectPtrBase::StrongObjectPtrBase(Object* object)
    : object(object),
      index(StaticStrongObjectPtrManager()->Register(object))
{}

StrongObjectPtrBase::~StrongObjectPtrBase() {
    StaticStrongObjectPtrManager()->Unregister(index);
}

bool StrongObjectPtrBase::IsValid() const {
    return ::IsValid(object);
}

const Object* StrongObjectPtrBase::Get() const {
    return IsValid() ? object : nullptr;
}

Object* StrongObjectPtrBase::Get() {
    return IsValid() ? object : nullptr;
}
