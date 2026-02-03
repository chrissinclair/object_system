#include "Object/ObjectField.h"
#include "Object/TypeTraits.h"

ObjectField::ObjectField(const ObjectFieldType type, const String& name, const u32 offset)
    : Type(type),
      Offset(offset),
      Name(name)
{}

ObjectField::ObjectField(const ObjectFieldType type, String&& name, const u32 offset)
    : Type(type),
      Offset(offset),
      Name(Move(name))
{}

bool ObjectField::HasFlag(const String& flag) const {
    return flags.find(flag) != flags.end();
}

bool ObjectField::HasParam(const String& param) const {
    return params.find(param) != params.end();
}

const String& ObjectField::GetParam(const String& param) const {
    auto it = params.find(param);
    if (it != params.end()) {
        return it->second;
    }
    static String missingTag{};
    return missingTag;
}

void* ObjectField::GetUntypedValuePtr(void* object) {
    return ((u8*) object) + Offset;
}

#define DEFINE_OBJECT_FIELD_TYPE_CTOR(type, objectFieldType) \
    type::type(const String& name, const u32 offset) \
        : ObjectField(ObjectFieldType::objectFieldType, name, offset) \
    {} \
    type::type(String&& name, const u32 offset) \
        : ObjectField(ObjectFieldType::objectFieldType, Move(name), offset) \
    {}

DEFINE_OBJECT_FIELD_TYPE_CTOR(BoolObjectField, Boolean)
DEFINE_OBJECT_FIELD_TYPE_CTOR(I32ObjectField, Int32)
DEFINE_OBJECT_FIELD_TYPE_CTOR(I64ObjectField, Int64)
DEFINE_OBJECT_FIELD_TYPE_CTOR(R32ObjectField, Real32)
DEFINE_OBJECT_FIELD_TYPE_CTOR(R64ObjectField, Real64)
DEFINE_OBJECT_FIELD_TYPE_CTOR(StringObjectField, String)

#undef DEFINE_OBJECT_FIELD_TYPE_CTOR

EnumObjectField::EnumObjectField(const String& name, const u32 offset, Enum* enumClass)
    : ObjectField(ObjectFieldType::Enum, name, offset),
      EnumClass(enumClass)
{}

EnumObjectField::EnumObjectField(String&& name, const u32 offset, Enum* enumClass)
    : ObjectField(ObjectFieldType::Enum, Move(name), offset),
      EnumClass(enumClass)
{}

ObjectObjectField::ObjectObjectField(const String& name, const u32 offset, Class* innerType)
    : ObjectField(ObjectFieldType::Object, name, offset),
      InnerType(innerType)
{}

ObjectObjectField::ObjectObjectField(String&& name, const u32 offset, Class* innerType)
    : ObjectField(ObjectFieldType::Object, Move(name), offset),
      InnerType(innerType)
{}

StructObjectField::StructObjectField(const String& name, const u32 offset, Class* structType)
    : ObjectField(ObjectFieldType::Struct, name, offset),
      StructType(structType)
{}

StructObjectField::StructObjectField(String&& name, const u32 offset, Class* structType)
    : ObjectField(ObjectFieldType::Struct, Move(name), offset),
      StructType(structType)
{}

ArrayObjectField::ArrayObjectField(const String& name, const u32 offset, UniquePtr<ObjectField>&& innerType)
    : ObjectField(ObjectFieldType::Array, name, offset),
      InnerType(Move(innerType))
{}
ArrayObjectField::ArrayObjectField(String&& name, const u32 offset, UniquePtr<ObjectField>&& innerType)
    : ObjectField(ObjectFieldType::Array, Move(name), offset),
      InnerType(Move(innerType))
{}

#define DEFINE_CREATE_OBJECT_FIELD(type, objectFieldType) \
UniquePtr<ObjectField> Detail::ObjectFieldCreator<type>::operator()(const String& name, const u32 offset) { \
    return MakeUnique<objectFieldType>(name, offset); \
} \
UniquePtr<ObjectField> Detail::ObjectFieldCreator<type>::operator()(String&& name, const u32 offset) { \
    return MakeUnique<objectFieldType>(Move(name), offset); \
}

DEFINE_CREATE_OBJECT_FIELD(bool, BoolObjectField)
DEFINE_CREATE_OBJECT_FIELD(i32, I32ObjectField)
DEFINE_CREATE_OBJECT_FIELD(i64, I64ObjectField)
DEFINE_CREATE_OBJECT_FIELD(r32, R32ObjectField)
DEFINE_CREATE_OBJECT_FIELD(r64, R64ObjectField)
DEFINE_CREATE_OBJECT_FIELD(String, StringObjectField)

#undef DEFINE_CREATE_OBJECT_FIELD
