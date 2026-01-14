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

struct Class;
namespace Detail {
template<typename T>
void ConfigureClass(Class*);
}
template<typename T>
Class* StaticClass();

struct Object {
    virtual ~Object();

    ObjectFlags GetFlags() const;

    virtual u32 TypeId() const;
    virtual String TypeName() const;
    const Array<UniquePtr<ObjectField>>& GetObjectFields() const;

    void AddToRootSet();
    void RemoveFromRootSet();

    void Destroy();
    void TryCompleteDestruction();

    virtual void OnBeginDestroy();
    virtual bool IsDestroyFinished() const;
    virtual void OnEndDestroy();

    u32 GetGeneration() const;

    static void CollectGarbage();

    Class* GetClass() const { return classInstance; }

protected:
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const;

private:
    template<typename T>
    friend void Detail::ConfigureClass(Class*);
    template<typename T>
    friend T* NewObject();
    friend Object* NewObject(Class*);

    Class* classInstance = nullptr;
};

template<typename T>
T* StaticInstance() {
    static_assert(IsDerivedFrom<T, Object>, "T must be an object to create a static instance");
    static T instance{};
    return &instance;
}

template<typename T>
const String& StaticTypeName() {
    static const String typeName = StaticInstance<T>()->TypeName();
    return typeName;
}

template<typename T>
u32 StaticTypeId() {
    static const u32 typeId = StaticInstance<T>()->TypeId();
    return typeId;
}

namespace Detail {
    void* AllocObject(u32 objectSize);
}

Object* NewObject(Class* objectClass);

template<typename T>
T* NewObject() {
    static_assert(IsDerivedFrom<T, Object>, "T must be an object to be created through NewObject");
    return (T*) NewObject(StaticClass<T>());
}

template<>
Class* NewObject<Class>();

bool IsValid(const Object* object);

template<typename T>
struct Class* StaticClass();


template<typename T> T* Cast(Object* object);
template<typename T> const T* Cast(const Object* object);

struct Class : Object {
    virtual u32 TypeId() const override { return 'CLAS'; }
    virtual String TypeName() const override { return "Class"; }

    u32 ClassTypeId() const { return typeId; }
    u32 Size() const { return size; }
    const String& Name() const { return name; }
    Class* Parent() const { return parent; }
    const Array<UniquePtr<ObjectField>>& Fields() const { return fields; }

    template<typename T>
    bool IsDerivedFrom() {
        return IsDerivedFrom(StaticClass<T>());
    }

    Array<Class*> GetDerivedClasses() const;

    bool IsDerivedFrom(const Class* parentClass) const;

    Object* StaticInstance() const { return staticInstance; }
    template<typename T>
    T* StaticInstance() const { return Cast<T>(staticInstance); }

private:
    Class* parent = nullptr;
    Object* staticInstance = nullptr;
    String name;
    u32 typeId;
    u32 size;
    Array<UniquePtr<ObjectField>> fields;
    void(*constructor)(Object* object);

    template<typename T>
    friend void Detail::ConfigureClass(Class*);
    friend Object* NewObject(Class*);

    void Construct(Object*);
    void Register();
};

namespace Detail {
    template<typename T>
    void ConfigureClass(Class* classInstance) {
        static_assert(false, "Attempting to register a class that's not been exposed properly. Ensure you have written DECLARE_OBJECT(type) in your header file, and IMPL_OBJECT(type, parentType) in your cpp file");
    }

    template<>
    void ConfigureClass<Object>(Class* classInstance);
    template<>
    void ConfigureClass<Class>(Class* classInstance);
}

#define DECLARE_OBJECT(type) \
    namespace Detail { \
        template<> \
        void ConfigureClass<type>(Class* classInstance); \
    }

DECLARE_OBJECT(Class)

#define IMPL_OBJECT(type, parentType) \
    namespace Detail { \
        template<> \
        void ConfigureClass<type>(Class* classInstance) { \
            classInstance->name = #type; \
            classInstance->parent = StaticClass<parentType>(); \
            classInstance->typeId = StaticTypeId<type>(); \
            classInstance->size = sizeof(type); \
            classInstance->constructor = [](Object* object) { new (object) type{}; }; \
            StaticInstance<type>()->GetObjectFields(classInstance->fields); \
            StaticInstance<type>()->classInstance = classInstance; \
            classInstance->staticInstance = StaticInstance<type>(); \
            classInstance->Register(); \
        } \
    } \
    static bool configuredObjectClassInstance_##type = []{ \
        StaticClass<type>(); \
        return true; \
    }();


template<typename T>
Class* StaticClass() {
    static Class* instance = []{
        Class* i = NewObject<Class>();
        Detail::ConfigureClass<T>(i);
        i->AddToRootSet();
        return i;
    }();
    return instance;
};


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

template<typename T>
const T* Cast(const Object* object) {
    static_assert(IsDerivedFrom<T, Object>, "T must be an object to cast an object to it");
    if (object && object->GetClass()->IsDerivedFrom<T>()) {
        return static_cast<T*>(object);
    }
    return nullptr;
}

template<typename T>
T* Cast(Object* object) {
    static_assert(IsDerivedFrom<T, Object>, "T must be an object to cast an object to it");
    if (object && object->GetClass()->IsDerivedFrom<T>()) {
        return static_cast<T*>(object);
    }
    return nullptr;
}

