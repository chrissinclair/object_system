#include "TestObjects.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Static type info should be correct", "[object]") {
    REQUIRE(StaticClass<TestObject>()->Name() == "TestObject");
}

TEST_CASE("Object fields should be correct", "[object]") {
    const Array<UniquePtr<ObjectField>>& fields = StaticClass<TestObject>()->Fields();

    TestObject* object = NewObject<TestObject>();
    TestObject* otherObject = (TestObject*) NewObject(StaticClass<TestObject>());
    REQUIRE(object);
    object->SomeBoolean = true;
    object->SomeInt32 = 123;
    object->SomeInt64 = 456;
    object->SomeReal32 = 1.0;
    object->SomeReal64 = 4.56;
    object->SomeOtherObject = otherObject;
    object->SomeOtherObjects.push_back(otherObject);
    object->SomeString = "Test string";
    object->SomeEnum = TestEnum::SecondEnumerator;

    REQUIRE(fields.size() == 10);
    // Bool field
    REQUIRE(fields[0]->Type == ObjectFieldType::Boolean);
    REQUIRE(fields[0]->Name == "SomeBoolean");
    REQUIRE(*fields[0]->As<BoolObjectField>()->GetValuePtr(object) == true);

    // Int32 field
    REQUIRE(fields[1]->Type == ObjectFieldType::Int32);
    REQUIRE(fields[1]->Name == "SomeInt32");
    REQUIRE(*fields[1]->As<I32ObjectField>()->GetValuePtr(object) == 123);

    // Int64 field
    REQUIRE(fields[2]->Type == ObjectFieldType::Int64);
    REQUIRE(fields[2]->Name == "SomeInt64");
    REQUIRE(*fields[2]->As<I64ObjectField>()->GetValuePtr(object) == 456);

    // Real32 field
    REQUIRE(fields[3]->Type == ObjectFieldType::Real32);
    REQUIRE(fields[3]->Name == "SomeReal32");
    REQUIRE(*fields[3]->As<R32ObjectField>()->GetValuePtr(object) == 1.0);

    // Real64 field
    REQUIRE(fields[4]->Type == ObjectFieldType::Real64);
    REQUIRE(fields[4]->Name == "SomeReal64");
    REQUIRE(*fields[4]->As<R64ObjectField>()->GetValuePtr(object) == 4.56);

    // Object* field
    REQUIRE(fields[5]->Type == ObjectFieldType::Object);
    REQUIRE(fields[5]->Name == "SomeOtherObject");
    REQUIRE(fields[5]->As<ObjectObjectField>()->InnerType == StaticClass<Object>());
    REQUIRE(*fields[5]->As<ObjectObjectField>()->GetValuePtr(object) == otherObject);

    // Array field
    REQUIRE(fields[6]->Type == ObjectFieldType::Array);
    REQUIRE(fields[6]->Name == "SomeOtherObjects");
    REQUIRE(fields[6]->As<ArrayObjectField>()->InnerType->Type == ObjectFieldType::Object);
    REQUIRE(fields[6]->As<ArrayObjectField>()->InnerType->As<ObjectObjectField>()->InnerType == StaticClass<Object>());
    Array<Object*>* otherObjects = (Array<Object*>*) fields[6]->GetUntypedValuePtr(object);
    REQUIRE(otherObjects->size() == 1);
    REQUIRE((*otherObjects)[0] == otherObject);
    REQUIRE(otherObjects == &object->SomeOtherObjects);

    // String field
    REQUIRE(fields[7]->Type == ObjectFieldType::String);
    REQUIRE(fields[7]->Name == "SomeString");
    REQUIRE(*fields[7]->As<StringObjectField>()->GetValuePtr(object) == "Test string");

    // Enum field
    REQUIRE(fields[8]->Type == ObjectFieldType::Enum);
    REQUIRE(fields[8]->Name == "SomeEnum");
    REQUIRE(fields[8]->As<EnumObjectField>()->EnumClass == StaticEnum<TestEnum>());
    REQUIRE(*static_cast<TestEnum*>(fields[8]->GetUntypedValuePtr(object)) == TestEnum::SecondEnumerator);

    REQUIRE(fields[9]->Type == ObjectFieldType::Struct);
    REQUIRE(fields[9]->Name == "SomeStruct");
    REQUIRE(fields[9]->As<StructObjectField>()->StructType == StaticClass<TestStruct>());
}

TEST_CASE("Struct fields should be correct", "[object]") {
    const Array<UniquePtr<ObjectField>>& fields = StaticClass<TestStruct>()->Fields();

    TestObject* object = NewObject<TestObject>();

    TestStruct testStruct;
    testStruct.SomeProperty = 123;
    testStruct.SomeOtherObject = object;
    testStruct.SomeOtherObjects.push_back(object);
    testStruct.SomeOtherStructs.emplace_back().SomeOtherObject = object;

    REQUIRE(fields.size() == 4);

    REQUIRE(fields[0]->Type == ObjectFieldType::Int32);
    REQUIRE(fields[0]->Name == "SomeProperty");
    REQUIRE(*fields[0]->As<I32ObjectField>()->GetValuePtr(&testStruct) == 123);

    REQUIRE(fields[1]->Type == ObjectFieldType::Object);
    REQUIRE(fields[1]->Name == "SomeOtherObject");
    REQUIRE(*fields[1]->As<ObjectObjectField>()->GetValuePtr(&testStruct) == object);

    REQUIRE(fields[2]->Type == ObjectFieldType::Array);
    REQUIRE(fields[2]->Name == "SomeOtherObjects");
    REQUIRE(fields[2]->As<ArrayObjectField>()->InnerType->Type == ObjectFieldType::Object);
    REQUIRE(fields[2]->As<ArrayObjectField>()->InnerType->As<ObjectObjectField>()->InnerType == StaticClass<Object>());
    Array<Object*>* otherObjects = (Array<Object*>*) fields[2]->GetUntypedValuePtr(&testStruct);
    REQUIRE(otherObjects->size() == 1);
    REQUIRE((*otherObjects)[0] == object);

    REQUIRE(fields[3]->Type == ObjectFieldType::Array);
    REQUIRE(fields[3]->Name == "SomeOtherStructs");
    REQUIRE(fields[3]->As<ArrayObjectField>()->InnerType->Type == ObjectFieldType::Struct);
    REQUIRE(fields[3]->As<ArrayObjectField>()->InnerType->As<StructObjectField>()->StructType == StaticClass<TestReferenceStruct>());
    Array<TestReferenceStruct>* otherStructs = (Array<TestReferenceStruct>*) fields[3]->GetUntypedValuePtr(&testStruct);
    REQUIRE(otherStructs->size() == 1);
    REQUIRE((*otherStructs)[0].SomeOtherObject == object);
}

TEST_CASE("Object class info should be correct", "[object]") {
    REQUIRE(StaticClass<TestObject>()->Parent() == StaticClass<Object>());
    REQUIRE(StaticClass<TestObject>()->IsDerivedFrom<Object>());
    REQUIRE(StaticClass<TestObject>()->Size() == sizeof(TestObject));
    REQUIRE(StaticClass<TestObject>()->Name() == "TestObject");
    REQUIRE(StaticClass<TestObject>()->Parent()->Name() == "Object");
    REQUIRE(StaticClass<TestObject>()->StaticInstance() == StaticInstance<TestObject>());
    REQUIRE_FALSE(IsValid(StaticClass<Object>()->Parent()));
}

TEST_CASE("Object class info for derived classes should be correct", "[object]") {
    REQUIRE(StaticClass<TestDerivedObject>()->Name() == "TestDerivedObject");
    REQUIRE(StaticClass<TestDerivedObject>()->Parent() == StaticClass<TestReferencingObject>());
    REQUIRE(StaticClass<TestDerivedObject>()->IsDerivedFrom<TestReferencingObject>());
    REQUIRE(StaticClass<TestDerivedObject>()->IsDerivedFrom<Object>());
    REQUIRE(StaticClass<TestDerivedObject>()->Size() == sizeof(TestDerivedObject));
    REQUIRE(StaticClass<TestDerivedObject>()->StaticInstance() == StaticInstance<TestDerivedObject>());
    REQUIRE_FALSE(StaticClass<TestDerivedObject>()->IsDerivedFrom<TestReferencingArrayObject>());
}

TEST_CASE("Enum info should be correct", "[object]") {
    REQUIRE(StaticEnum<TestEnum>()->Name() == "TestEnum");
    REQUIRE(StaticEnum<TestEnum>()->IsEnumFlags());
    REQUIRE(StaticEnum<TestEnum>()->Values().size() == 2);
    REQUIRE(StaticEnum<TestEnum>()->Values()[0] == static_cast<i32>(TestEnum::FirstEnumerator));
    REQUIRE(StaticEnum<TestEnum>()->Values()[1] == static_cast<i32>(TestEnum::SecondEnumerator));
    REQUIRE(StaticEnum<TestEnum>()->Enumerators().size() == 2);
    REQUIRE(StaticEnum<TestEnum>()->Enumerators()[0] == "FirstEnumerator");
    REQUIRE(StaticEnum<TestEnum>()->Enumerators()[1] == "SecondEnumerator");

    REQUIRE(StaticEnum<TestEnum>()->ToString(static_cast<i32>(TestEnum::SecondEnumerator)) == "SecondEnumerator");
    REQUIRE(StaticEnum<TestEnum>()->FromString("SECONDENUMERATOR") == static_cast<i32>(TestEnum::SecondEnumerator));
}

TEST_CASE("Struct info should be correct", "[object]") {
    REQUIRE(StaticClass<TestStruct>()->Parent() == StaticClass<Struct>());
    REQUIRE(StaticClass<TestStruct>()->IsDerivedFrom<Struct>());
    REQUIRE(StaticClass<TestStruct>()->Name() == "TestStruct");
    REQUIRE(StaticClass<TestStruct>()->Size() == sizeof(TestStruct));
}

TEST_CASE("Object class info should be able to find derived classes", "[object]") {
    Array<Class*> derivedClasses = StaticClass<TestReferencingObject>()->GetDerivedClasses();
    REQUIRE(derivedClasses.size() == 1);
    if (derivedClasses.size() > 0) {
        REQUIRE(derivedClasses[0] == StaticClass<TestDerivedObject>());
    }
}

TEST_CASE("Object fields should maintain tags", "[object]") {
    const Array<UniquePtr<ObjectField>>& fields = StaticClass<TestObject>()->Fields();

    REQUIRE(fields.size() > 3);

    REQUIRE(fields[0]->Name == "SomeBoolean");
    REQUIRE(fields[0]->HasParam("TestTag"));
    REQUIRE(fields[0]->GetParam("TestTag") == "TestTagValue");
    REQUIRE(fields[0]->HasParam("OtherTag"));
    REQUIRE(fields[0]->GetParam("OtherTag") == "OtherTagValue");

    REQUIRE(fields[1]->Name == "SomeInt32");
    REQUIRE(fields[1]->HasParam("TestTag"));
    REQUIRE(fields[1]->GetParam("TestTag") == "AnotherTestTagValue");
    REQUIRE_FALSE(fields[1]->HasParam("OtherTag"));

    REQUIRE(fields[2]->Name == "SomeInt64");
    REQUIRE(fields[2]->HasFlag("TestFlag"));
}
