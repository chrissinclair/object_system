#include "Object/ObjectField.h"
#include "Object/TypeTraits.h"

ObjectField::ObjectField(const ObjectFieldType type, const u32 offset, const String& name)
    : Type(type),
      Offset(offset),
      Name(name)
{}

ObjectField::ObjectField(const ObjectFieldType type, const u32 offset, String&& name)
    : Type(type),
      Offset(offset),
      Name(Move(name))
{}

bool ObjectField::HasTag(const String& tag) const {
    return tags.find(tag) != tags.end();
}

const String& ObjectField::GetTag(const String& tag) const {
    auto it = tags.find(tag);
    if (it != tags.end()) {
        return it->second;
    }
    static String missingTag{};
    return missingTag;
}

ObjectField* ObjectField::WithTag(const String& tag, const String& value) {
    tags.emplace(tag, value);
    return this;
}

ObjectField* ObjectField::WithTag(String&& tag, String&& value) {
    tags.emplace(Move(tag), Move(value));
    return this;
}

void* ObjectField::GetUntypedValuePtr(Object* object) {
    return ((u8*) object) + Offset;
}

#define DEFINE_OBJECT_FIELD_TYPE_CTOR(type, objectFieldType) \
    type::type(const u32 offset, const String& name) \
        : ObjectField(ObjectFieldType::objectFieldType, offset, name) \
    {} \
    type::type(const u32 offset, String&& name) \
        : ObjectField(ObjectFieldType::objectFieldType, offset, Move(name)) \
    {}

DEFINE_OBJECT_FIELD_TYPE_CTOR(BoolObjectField, Boolean)
DEFINE_OBJECT_FIELD_TYPE_CTOR(I32ObjectField, Int32)
DEFINE_OBJECT_FIELD_TYPE_CTOR(I64ObjectField, Int64)
DEFINE_OBJECT_FIELD_TYPE_CTOR(R32ObjectField, Real32)
DEFINE_OBJECT_FIELD_TYPE_CTOR(R64ObjectField, Real64)
DEFINE_OBJECT_FIELD_TYPE_CTOR(StringObjectField, String)

#undef DEFINE_OBJECT_FIELD_TYPE_CTOR

ObjectObjectField::ObjectObjectField(const u32 offset, const String& name, Class* innerType)
    : ObjectField(ObjectFieldType::Object, offset, name),
      InnerType(innerType)
{}

ObjectObjectField::ObjectObjectField(const u32 offset, String&& name, Class* innerType)
    : ObjectField(ObjectFieldType::Object, offset, Move(name)),
      InnerType(innerType)
{}

ArrayObjectField::ArrayObjectField(const u32 offset, const String& name, UniquePtr<ObjectField>&& innerType)
    : ObjectField(ObjectFieldType::Array, offset, name),
      InnerType(Move(innerType))
{}
ArrayObjectField::ArrayObjectField(const u32 offset, String&& name, UniquePtr<ObjectField>&& innerType)
    : ObjectField(ObjectFieldType::Array, offset, Move(name)),
      InnerType(Move(innerType))
{}

#define DEFINE_CREATE_OBJECT_FIELD(type, objectFieldType) \
UniquePtr<ObjectField> Detail::ObjectFieldCreator<type>::operator()(const u32 offset, const String& name) { \
    return MakeUnique<objectFieldType>(offset, name); \
} \
UniquePtr<ObjectField> Detail::ObjectFieldCreator<type>::operator()(const u32 offset, String&& name) { \
    return MakeUnique<objectFieldType>(offset, Move(name)); \
}

DEFINE_CREATE_OBJECT_FIELD(bool, BoolObjectField)
DEFINE_CREATE_OBJECT_FIELD(i32, I32ObjectField)
DEFINE_CREATE_OBJECT_FIELD(i64, I64ObjectField)
DEFINE_CREATE_OBJECT_FIELD(r32, R32ObjectField)
DEFINE_CREATE_OBJECT_FIELD(r64, R64ObjectField)
DEFINE_CREATE_OBJECT_FIELD(String, StringObjectField)

#undef DEFINE_CREATE_OBJECT_FIELD
