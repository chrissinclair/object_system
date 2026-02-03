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

    template<typename T>
    requires(IsEnumType<T>)
    void ConfigureEnum(Enum*);
}

#define OBJECT_MAX_FIELD_COUNT_DEFAULT 32
#ifndef OBJECT_MAX_FIELD_COUNT
#define OBJECT_MAX_FIELD_COUNT OBJECT_MAX_FIELD_COUNT_DEFAULT
#endif

#define OBJECT_BODY(ClassType, ParentType) \
public: \
    using ThisClass = ClassType; \
    using Super = ParentType; \
private: \
    static constexpr usize OBJECT_FIELD_COUNTER_BASE = __COUNTER__; \
    template<usize FieldNumber> struct ObjectFieldInitializer; \
    template<usize FieldNumber> \
    requires(IsTypeComplete<ObjectFieldInitializer<FieldNumber>>) \
    static void InitializeObjectField(Array<UniquePtr<ObjectField>>& fields) { \
        using Info = ObjectFieldInitializer<FieldNumber>; \
        fields.emplace_back(Detail::CreateObjectField<typename Info::Type>(Info::Name, Info::GetOffset(), Info::GetTags())); \
    } \
    template<usize FieldNumber> static void InitializeObjectField(Array<UniquePtr<ObjectField>>& fields) {} \
    template<typename T, usize... FieldIndices> \
    static void InitializeObjectFields(Array<UniquePtr<ObjectField>>& fields, std::integer_sequence<T, FieldIndices...>) { \
        (InitializeObjectField<FieldIndices>(fields), ...); \
    } \
public: \
    struct ClassDetail { \
        static void InitializeFields(Array<UniquePtr<ObjectField>>& fields) { \
            Super::ClassDetail::InitializeFields(fields); \
            ThisClass::InitializeObjectFields(fields, std::make_index_sequence<OBJECT_MAX_FIELD_COUNT>{}); \
        } \
    };

#define OBJECT_PROPERTY(field, ...) \
private: \
    template<> struct ObjectFieldInitializer<__COUNTER__ - OBJECT_FIELD_COUNTER_BASE - 1> { \
        using Type = decltype(field); \
        static constexpr const char* Name = #field; \
        static usize GetOffset() { return (usize)(&((ThisClass*)0)->field); } \
        static Array<String> GetTags() { return { MACRO_FOR_EACH( MACRO_QUOTE, __VA_ARGS__ ) }; } \
    }; \
public:

template<typename T>
Class* StaticClass();

struct Object {
    using ThisClass = Object;

    virtual ~Object();

    ObjectFlags GetFlags() const;

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

public:
    struct ClassDetail {
        static void InitializeFields(Array<UniquePtr<ObjectField>>& fields);
    };

private:
    template<typename T>
    friend void Detail::ConfigureClass(Class*);
    template<typename T>
    friend T* NewObject();
    friend Object* NewObject(Class*);

    Class* classInstance = nullptr;
};

struct Struct {
    using ThisClass = Struct;

    Class* GetClass() const { return classInstance; }

public:
    struct ClassDetail {
        static void InitializeFields(Array<UniquePtr<ObjectField>>& fields);
    };

private:
    template<typename T>
    friend void Detail::ConfigureClass(Class*);
    template<typename T>
    friend T* NewObject();
    friend Object* NewObject(Class*);

    Class* classInstance = nullptr;
};

template<typename T>
requires(IsDerivedFrom<T, Object>)
T* StaticInstance() {
    static T instance{};
    return &instance;
}

template<typename T> T* Cast(Object* object);
template<typename T> const T* Cast(const Object* object);

namespace Detail {
    void* AllocObject(u32 objectSize);
}

Object* NewObject(Class* objectClass);

template<typename T>
T* NewObject(Class* objectClass) {
    return Cast<T>(NewObject(objectClass));
}

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

struct Class : Object {
    OBJECT_BODY(Class, Object)

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
    u32 size;
    Array<UniquePtr<ObjectField>> fields;
    void(*constructor)(Object* object);

    template<typename T>
    friend void Detail::ConfigureClass(Class*);
    friend Object* NewObject(Class*);

    void Construct(Object*);
    void Register();
};

struct Enum : Object {
    OBJECT_BODY(Enum, Object)

    const String& Name() const { return name; }
    const Array<i32>& Values() const { return values; }
    const Array<String>& Enumerators() const { return enumerators; }
    bool IsEnumFlags() const { return isEnumFlags; }

    const String& ToString(i32 value) const;
    i32 FromString(const String& value) const;

private:
    String name;
    Array<i32> values;
    Array<String> enumerators;
    bool isEnumFlags;

    template<typename T>
    requires(IsEnumType<T>)
    friend void Detail::ConfigureEnum(Enum*);
};

namespace Detail {
    template<typename T>
    void ConfigureClass(Class* classInstance) {
        static_assert(false, "Attempting to register a class that's not been exposed properly. Ensure you have written DECLARE_OBJECT(type) in your header file, and IMPL_OBJECT(type) in your cpp file");
    }

    template<>
    void ConfigureClass<Object>(Class* classInstance);
    template<>
    void ConfigureClass<Class>(Class* classInstance);
    template<>
    void ConfigureClass<Struct>(Class* classInstance);

    template<typename T>
    requires(IsEnumType<T>)
    void ConfigureEnum(Enum* enumInstance) {
        static_assert(false, "Attempting to register an enum that's not been exposed properly. Ensure you have written DECLARE_ENUM(type) in your header file, and IMPL_ENUM(type) in your cpp file");
    }
}

#define DECLARE_OBJECT(type) \
    namespace Detail { \
        template<> \
        void ConfigureClass<type>(Class*); \
    }

DECLARE_OBJECT(Class)
DECLARE_OBJECT(Enum);

#define IMPL_OBJECT(type) \
    namespace Detail { \
        template<> \
        void ConfigureClass<type>(Class* classInstance) { \
            classInstance->name = #type; \
            classInstance->parent = StaticClass<type::Super>(); \
            classInstance->size = sizeof(type); \
            type::ClassDetail::InitializeFields(classInstance->fields); \
            classInstance->constructor = [](Object* object) { new (object) type{}; }; \
            StaticInstance<type>()->classInstance = classInstance; \
            classInstance->staticInstance = StaticInstance<type>(); \
            classInstance->Register(); \
        } \
    } \
    static bool configuredObjectClassInstance_##type = []{ \
        StaticClass<type>(); \
        return true; \
    }();

#define DECLARE_STRUCT(type) \
    namespace Detail { \
        template<> \
        void ConfigureClass<type>(Class*); \
    }

#define IMPL_STRUCT(type) \
    namespace Detail { \
        template<> \
        void ConfigureClass<type>(Class* classInstance) { \
            classInstance->name = #type; \
            classInstance->parent = StaticClass<type::Super>(); \
            classInstance->size = sizeof(type); \
            type::ClassDetail::InitializeFields(classInstance->fields); \
            classInstance->Register(); \
        } \
    } \
    static bool configuredStructClassInstance_##type = []{ \
        StaticClass<type>(); \
        return true; \
    }();


#define DECLARE_ENUM(type) \
    namespace Detail { \
        template<> \
        void ConfigureEnum<type>(Enum*); \
    }

#define IMPL_ENUM(type) \
    namespace Detail { \
        template<> \
        void ConfigureEnum<type>(Enum* enumInstance) { \
            enumInstance->name = #type; \
            enumInstance->isEnumFlags = EnumTraits<type>::IsFlags; \
            constexpr auto entries = magic_enum::enum_entries<type>(); \
            enumInstance->values.reserve(entries.size()); \
            enumInstance->enumerators.reserve(entries.size()); \
            for (const auto& entry : entries) { \
                enumInstance->values.push_back(static_cast<i32>(entry.first)); \
                enumInstance->enumerators.emplace_back(entry.second); \
            } \
        } \
    } \
    static bool configuredEnumClassInstance_##type = [] { \
        StaticEnum<type>(); \
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

template<typename T>
requires(IsEnumType<T>)
Enum* StaticEnum() {
    static Enum* instance = []{
        Enum* i = NewObject<Enum>();
        Detail::ConfigureEnum<T>(i);
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

