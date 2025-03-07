#pragma once

#include "Object/Types.h"
#include "Object/TypeTraits.h"
#include "Object/ObjectField.h"

enum class ObjectFlags : u8 {
    None = 0,
    Allocated = 1 << 0,
    Unreachable = 1 << 1,
    InRootSet = 1 << 2,
    IsBeingDestroyed = 1 << 3,
    IsDestroyed = 1 << 4,
};
DEFINE_ENUM_CLASS_FLAGS(ObjectFlags)

struct Object {
    virtual ~Object();

    ObjectFlags GetFlags() const;

    virtual u32 TypeId() const;
    virtual String TypeName() const;
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const;

    void AddToRootSet();
    void RemoveFromRootSet();

    void Destroy();
    void TryCompleteDestruction();

    virtual void OnBeginDestroy();
    virtual bool IsDestroyFinished() const;
    virtual void OnEndDestroy();

    u32 GetGeneration() const;

    static void CollectGarbage();
};

template<typename T>
T* StaticInstance() {
    static_assert(IsDerivedFrom<T, Object>, "T must be an object to create a static instance");
    static T instance{};
    return &instance;
}

template<typename T>
const String& StaticTypeName() {
    static const String typeName = []{
        return StaticInstance<T>()->TypeName();
    }();
    return typeName;
}

template<typename T>
u32 StaticTypeId() {
    static const u32 typeId = []{
        return StaticInstance<T>()->TypeId();
    }();
    return typeId;
}

namespace Detail {
    void* AllocObject(u32 objectSize);
}

template<typename T>
T* NewObject() {
    static_assert(IsDerivedFrom<T, Object>, "T must be an object to be created through NewObject");
    void* object = Detail::AllocObject(sizeof(T));
    if (!object) {
        return nullptr;
    }
    new (object) T{};
    return (T*) object;
}

bool IsValid(Object* object);

// Weak object pointers can point to objects but will not prevent them being garbage
// collected. Will become invalid if the object is destroyed or garbage collected
struct WeakObjectPtrBase {
    WeakObjectPtrBase(Object* object);

    bool IsValid() const;
    operator bool() const { return IsValid(); }

    const Object* Get() const;
    Object* Get();
    const Object* operator->() const { return Get(); }
    Object* operator->() { return Get(); }

private:
    Object* object = nullptr;
    u16 generation = 0;
};

template<typename T>
struct WeakObjectPtr : WeakObjectPtrBase {
    static_assert(Detail::IsObjectType<T>, "Can only create weak object pointers to objects");

    WeakObjectPtr(T* object)
        : WeakObjectPtrBase(object)
    {}

    const T* Get() const { return (const T*) WeakObjectPtrBase::Get(); }
    T* Get() { return (T*) WeakObjectPtrBase::Get(); }
    const T* operator->() const { return (const T*) WeakObjectPtrBase::operator->(); }
    T* operator->() { return (T*) WeakObjectPtrBase::operator->(); }
};

// Strong object pointers prevent the pointed to object being garbage collected,
// even if they are not referenced from the root set. Will become invalid if the
// underlying object is destroyed, but will still prevent it being collected, so
// use with care. You probably don't want these to hang around for too long
struct StrongObjectPtrBase {
    StrongObjectPtrBase(Object* object);
    virtual ~StrongObjectPtrBase();

    bool IsValid() const;
    operator bool() const { return IsValid(); }

    const Object* Get() const;
    Object* Get();
    const Object* operator->() const { return Get(); }
    Object* operator->() { return Get(); }

private:
    Object* object;
    i32 index = -1;
};

template<typename T>
struct StrongObjectPtr : StrongObjectPtrBase {
    static_assert(Detail::IsObjectType<T>, "Can only create weak object pointers to objects");

    StrongObjectPtr(T* object)
        : StrongObjectPtrBase(object)
    {}

    const T* Get() const { return (const T*) StrongObjectPtrBase::Get(); }
    T* Get() { return (T*) StrongObjectPtrBase::Get(); }
    const T* operator->() const { return (const T*) StrongObjectPtrBase::operator->(); }
    T* operator->() { return (T*) StrongObjectPtrBase::operator->(); }
};
