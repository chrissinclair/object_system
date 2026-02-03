#pragma once

#include "Object/Types.h"
#include "Object/TypeTraits.h"

struct Class;
struct Enum;
struct Object;
struct ObjectField;
struct Struct;

namespace Detail {
    void ParseTags(const Array<String>& tags, Set<String>& outFlags, Map<String, String>& outParams);

    template<typename T>
    UniquePtr<ObjectField> CreateObjectField(const String& name, const u32 offset, const Array<String>& tags);
    template<typename T>
    UniquePtr<ObjectField> CreateObjectField(String&& name, const u32 offset, const Array<String>& tags);
}

enum class ObjectFieldType {
    Boolean,
    Int32,
    Int64,
    Real32,
    Real64,
    Enum,
    Array,
    Object,
    Struct,
    String,
};

struct ObjectField {
    ObjectFieldType Type;
    u32 Offset;
    String Name;

    ObjectField(ObjectFieldType type, const String& name, u32 offset);
    ObjectField(ObjectFieldType type, String&& name, u32 offset);

    bool HasFlag(const String& flag) const;
    bool HasParam(const String& param) const;
    const String& GetParam(const String& param) const;

    const Set<String>& GetFlags() const { return flags; }
    const Map<String, String>& GetParams() const { return params; }

    void* GetUntypedValuePtr(void* object);

    template<typename T>
    requires(IsDerivedFrom<T, ObjectField>)
    const T* As() const {
        return static_cast<const T*>(this);
    }

    template<typename T>
    requires(IsDerivedFrom<T, ObjectField>)
    T* As() {
        return static_cast<T*>(this);
    }

private:
    Map<String, String> params;
    Set<String> flags;

    template<typename T>
    friend UniquePtr<ObjectField> Detail::CreateObjectField(const String& name, const u32 offset, const Array<String>& tags);
    template<typename T>
    friend UniquePtr<ObjectField> Detail::CreateObjectField(String&&, u32, const Array<String>&);
};

struct BoolObjectField : ObjectField {
    BoolObjectField(const String& name, u32 offset);
    BoolObjectField(String&& name, u32 offset);

    bool* GetValuePtr(void* object) { return (bool*) GetUntypedValuePtr(object); }
};

struct I32ObjectField : ObjectField {
    I32ObjectField(const String& name, u32 offset);
    I32ObjectField(String&& name, u32 offset);

    i32* GetValuePtr(void* object) { return (i32*) GetUntypedValuePtr(object); }
};

struct I64ObjectField : ObjectField {
    I64ObjectField(const String& name, u32 offset);
    I64ObjectField(String&& name, u32 offset);

    i64* GetValuePtr(void* object) { return (i64*) GetUntypedValuePtr(object); }
};

struct R32ObjectField : ObjectField {
    R32ObjectField(const String& name, u32 offset);
    R32ObjectField(String&& name, u32 offset);

    r32* GetValuePtr(void* object) { return (r32*) GetUntypedValuePtr(object); }
};

struct R64ObjectField : ObjectField {
    R64ObjectField(const String& name, u32 offset);
    R64ObjectField(String&& name, u32 offset);

    r64* GetValuePtr(void* object) { return (r64*) GetUntypedValuePtr(object); }
};

struct EnumObjectField : ObjectField {
    EnumObjectField(const String& name, u32 offset, Enum* enumClass);
    EnumObjectField(String&& name, u32 offset, Enum* enumClass);

    Enum* EnumClass;
};

struct ArrayObjectField : ObjectField {
    ArrayObjectField(const String& name, u32 offset, UniquePtr<ObjectField>&& innerType);
    ArrayObjectField(String&& name, u32 offset, UniquePtr<ObjectField>&& innerType);

    UniquePtr<ObjectField> InnerType;
};

struct ObjectObjectField : ObjectField {
    ObjectObjectField(const String& name, u32 offset, Class* innerType);
    ObjectObjectField(String&& name, u32 offset, Class* innerType);

    Object** GetValuePtr(void* object) { return (Object**) GetUntypedValuePtr(object); }

    Class* InnerType;
};

struct StructObjectField : ObjectField {
    StructObjectField(const String& name, u32 offset, Class* structType);
    StructObjectField(String&& name, u32 offset, Class* structType);

    Struct* GetValuePtr(void* object) { return (Struct*) GetUntypedValuePtr(object); }

    Class* StructType;
};

struct StringObjectField : ObjectField {
    StringObjectField(const String& name, u32 offset);
    StringObjectField(String&& name, u32 offset);

    String* GetValuePtr(void* object) { return (String*) GetUntypedValuePtr(object); }
};

template<typename T>
Class* StaticClass();

template<typename T>
requires(IsEnumType<T>)
Enum* StaticEnum();

namespace Detail {
    template<typename T>
    constexpr bool IsObjectType = IsDerivedFrom<T, Object>;

    template<typename T>
    constexpr bool IsStructType = IsDerivedFrom<T, Struct>;

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
    DECLARE_FIND_FIELD_TYPE(String, String)

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
    requires(IsStructType<T>)
    struct FieldTypeFinder<T> {
        static constexpr ObjectFieldType Value = ObjectFieldType::Struct;
    };

    template<typename T>
    constexpr ObjectFieldType FindFieldType = FieldTypeFinder<T>::Value;

    template<typename T>
    struct ObjectFieldCreator {
        UniquePtr<ObjectField> operator()(const String& name, const u32 offset, const Array<String>& tags) {
            static_assert(false, "Attempting to expose an unsupported type to the reflection system");
            return nullptr;
        }
        UniquePtr<ObjectField> operator()(String&& name, const u32 offset, const Array<String>& tags) {
            static_assert(false, "Attempting to expose an unsupported type to the reflection system");
            return nullptr;
        }
    };

#define DECLARE_CREATE_OBJECT_FIELD(type) \
    template<> \
    struct ObjectFieldCreator<type> { \
        UniquePtr<ObjectField> operator()(const String& name, const u32 offset); \
        UniquePtr<ObjectField> operator()(String&& name, const u32 offset); \
    }; \

    DECLARE_CREATE_OBJECT_FIELD(bool)
    DECLARE_CREATE_OBJECT_FIELD(i32)
    DECLARE_CREATE_OBJECT_FIELD(i64)
    DECLARE_CREATE_OBJECT_FIELD(r32)
    DECLARE_CREATE_OBJECT_FIELD(r64)
    DECLARE_CREATE_OBJECT_FIELD(String)

#undef DECLARE_CREATE_OBJECT_FIELD

    template<typename T>
    requires(IsEnumType<T>)
    struct ObjectFieldCreator<T> {
        UniquePtr<ObjectField> operator()(const String& name, const u32 offset) {
            return MakeUnique<EnumObjectField>(name, offset, StaticEnum<T>());
        }
        UniquePtr<ObjectField> operator()(String&& name, const u32 offset) {
            return MakeUnique<EnumObjectField>(Move(name), offset, StaticEnum<T>());
        }
    };

    template<typename T>
    requires(IsStructType<T>)
    struct ObjectFieldCreator<T>{
        UniquePtr<ObjectField> operator()(const String& name, const u32 offset) {
            return MakeUnique<StructObjectField>(name, offset, StaticClass<T>());
        };
        UniquePtr<ObjectField> operator()(String&& name, const u32 offset) {
            return MakeUnique<StructObjectField>(Move(name), offset, StaticClass<T>());
        };
    };

    template<typename T>
    struct ObjectFieldCreator<T*> {
        static_assert(IsObjectType<T>, "Attempting to expose an unsupported type to the reflection system");
        UniquePtr<ObjectField> operator()(const String& name, const u32 offset) {
            return MakeUnique<ObjectObjectField>(name, offset, StaticClass<T>());
        };
        UniquePtr<ObjectField> operator()(String&& name, const u32 offset) {
            return MakeUnique<ObjectObjectField>(Move(name), offset, StaticClass<T>());
        };
    };

    template<typename T>
    struct ObjectFieldCreator<Array<T>> {
        static_assert(FindFieldType<T> != ObjectFieldType::Array, "Arrays of arrays are not supported by the reflection system");

        UniquePtr<ObjectField> operator()(const String& name, const u32 offset) {
            return MakeUnique<ArrayObjectField>(name, offset, ObjectFieldCreator<T>{}("item", 0));
        }
        UniquePtr<ObjectField> operator()(String&& name, const u32 offset) {
            return MakeUnique<ArrayObjectField>(Move(name), offset, ObjectFieldCreator<T>{}("item", 0));
        }
    };

    template<typename T>
    UniquePtr<ObjectField> CreateObjectField(const String& name, const u32 offset, const Array<String>& tags) {
        UniquePtr<ObjectField> field = ObjectFieldCreator<T>{}(name, offset);
        Detail::ParseTags(tags, field->flags, field->params);
        return field;
    }
    template<typename T>
    UniquePtr<ObjectField> CreateObjectField(String&& name, const u32 offset, const Array<String>& tags) {
        UniquePtr<ObjectField> field = ObjectFieldCreator<T>{}(Move(name), offset);
        Detail::ParseTags(tags, field->flags, field->params);
        return field;
    }
};

