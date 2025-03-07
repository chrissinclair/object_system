#pragma once

#include "Object/Types.h"
#include "Object/TypeTraits.h"

struct Object;

enum class ObjectFieldType {
    Boolean,
    Int32,
    Int64,
    Real32,
    Real64,
    Array,
    Object,
};

struct ObjectField {
    ObjectFieldType Type;
    u32 Offset;
    String Name;

    ObjectField(ObjectFieldType type, u32 offset, const String& name);
    ObjectField(ObjectFieldType type, u32 offset, String&& name);

    void* GetUntypedValuePtr(Object* object);
};

struct BoolObjectField : ObjectField {
    BoolObjectField(u32 offset, const String& name);
    BoolObjectField(u32 offset, String&& name);

    bool* GetValuePtr(Object* object) { return (bool*) GetUntypedValuePtr(object); }
};

struct I32ObjectField : ObjectField {
    I32ObjectField(u32 offset, const String& name);
    I32ObjectField(u32 offset, String&& name);

    i32* GetValuePtr(Object* object) { return (i32*) GetUntypedValuePtr(object); }
};

struct I64ObjectField : ObjectField {
    I64ObjectField(u32 offset, const String& name);
    I64ObjectField(u32 offset, String&& name);

    i64* GetValuePtr(Object* object) { return (i64*) GetUntypedValuePtr(object); }
};

struct R32ObjectField : ObjectField {
    R32ObjectField(u32 offset, const String& name);
    R32ObjectField(u32 offset, String&& name);

    r32* GetValuePtr(Object* object) { return (r32*) GetUntypedValuePtr(object); }
};

struct R64ObjectField : ObjectField {
    R64ObjectField(u32 offset, const String& name);
    R64ObjectField(u32 offset, String&& name);

    r64* GetValuePtr(Object* object) { return (r64*) GetUntypedValuePtr(object); }
};

struct ArrayObjectField : ObjectField {
    ArrayObjectField(u32 offset, const String& name, ObjectFieldType itemType);
    ArrayObjectField(u32 offset, String&& name, ObjectFieldType itemType);

    ObjectFieldType ItemType;
};

struct ObjectObjectField : ObjectField {
    ObjectObjectField(u32 offset, const String& name);
    ObjectObjectField(u32 offset, String&& name);

    Object** GetValuePtr(Object* object) { return (Object**) GetUntypedValuePtr(object); }
};

namespace Detail {
    template<typename T>
    constexpr bool IsObjectType = IsDerivedFrom<T, Object>;

    template<typename T>
    struct FieldTypeFinder {
        static_assert(false, "Attempting to expose an unsupported type to the reflection system");
    };

#define DECLARE_FIND_FIELD_TYPE(type, objectFieldType) \
    template<> \
    struct FieldTypeFinder<type> { \
        static constexpr ObjectFieldType Value = ObjectFieldType::objectFieldType; \
    };

    DECLARE_FIND_FIELD_TYPE(bool, Boolean)
    DECLARE_FIND_FIELD_TYPE(i32, Int32)
    DECLARE_FIND_FIELD_TYPE(i64, Int64)
    DECLARE_FIND_FIELD_TYPE(r32, Real32)
    DECLARE_FIND_FIELD_TYPE(r64, Real64)

#undef DECLARE_FIND_FIELD_TYPE

    template<typename T>
    struct FieldTypeFinder<T*> {
        static_assert(IsObjectType<T>, "Attempting to expose an unsupported type to the reflection system");
        static constexpr ObjectFieldType Value = ObjectFieldType::Object;
    };

    template<typename T>
    struct FieldTypeFinder<Array<T>> {
        static constexpr ObjectFieldType Value = ObjectFieldType::Array;
    };

    template<typename T>
    constexpr ObjectFieldType FindFieldType = FieldTypeFinder<T>::Value;

    template<typename T>
    struct ObjectFieldCreator {
        UniquePtr<ObjectField> operator()(const u32 offset, const String& name) {
            static_assert(false, "Attempting to expose an unsupported type to the reflection system");
            return nullptr;
        }
        UniquePtr<ObjectField> operator()(const u32 offset, String&& name) {
            static_assert(false, "Attempting to expose an unsupported type to the reflection system");
            return nullptr;
        }
    };

#define DECLARE_CREATE_OBJECT_FIELD(type) \
    template<> \
    struct ObjectFieldCreator<type> { \
        UniquePtr<ObjectField> operator()(const u32 offset, const String& name); \
        UniquePtr<ObjectField> operator()(const u32 offset, String&& name); \
    }; \

    DECLARE_CREATE_OBJECT_FIELD(bool)
    DECLARE_CREATE_OBJECT_FIELD(i32)
    DECLARE_CREATE_OBJECT_FIELD(i64)
    DECLARE_CREATE_OBJECT_FIELD(r32)
    DECLARE_CREATE_OBJECT_FIELD(r64)

#undef DECLARE_CREATE_OBJECT_FIELD

    template<typename T>
    struct ObjectFieldCreator<T*> {
        static_assert(IsObjectType<T>, "Attempting to expose an unsupported type to the reflection system");
        UniquePtr<ObjectField> operator()(const u32 offset, const String& name) {
            return MakeUnique<ObjectObjectField>(offset, name);
        };
        UniquePtr<ObjectField> operator()(const u32 offset, String&& name) {
            return MakeUnique<ObjectObjectField>(offset, Move(name));
        };
    };

    template<typename T>
    struct ObjectFieldCreator<Array<T>> {
        static_assert(FindFieldType<T> != ObjectFieldType::Array, "Arrays of arrays are not supported by the reflection system");

        UniquePtr<ObjectField> operator()(const u32 offset, const String& name) {
            return MakeUnique<ArrayObjectField>(offset, name, FindFieldType<T>);
        }
        UniquePtr<ObjectField> operator()(const u32 offset, String&& name) {
            return MakeUnique<ArrayObjectField>(offset, Move(name), FindFieldType<T>);
        }
    };

    template<typename T>
    UniquePtr<ObjectField> CreateObjectField(const u32 offset, const String& name) {
        return ObjectFieldCreator<T>{}(offset, name);
    }
    template<typename T>
    UniquePtr<ObjectField> CreateObjectField(const u32 offset, String&& name) {
        return ObjectFieldCreator<T>{}(offset, Move(name));
    }

    constexpr u32 FindOffsetOf(const void* base, const void* field) {
        return (u8*) field - (u8*) base;
    }
};

#define EXPOSE_FIELD(field) fields.emplace_back(Detail::CreateObjectField<decltype(field)>(Detail::FindOffsetOf(this, &this->field), #field))

