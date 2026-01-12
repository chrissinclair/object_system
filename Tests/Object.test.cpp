#include "TestObjects.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Static type info should be correct", "[object]") {
    REQUIRE(StaticTypeId<TestObject>() == 'TEST');
    REQUIRE(StaticTypeName<TestObject>() == "TestObject");
}

TEST_CASE("Object fields should be correct", "[object]") {
    Array<UniquePtr<ObjectField>> fields;
    StaticInstance<TestObject>()->GetObjectFields(fields);

    TestObject* object = NewObject<TestObject>();
    TestObject* otherObject = NewObject<TestObject>();
    REQUIRE(object);
    object->SomeBoolean = true;
    object->SomeInt32 = 123;
    object->SomeInt64 = 456;
    object->SomeReal32 = 1.0;
    object->SomeReal64 = 4.56;
    object->SomeOtherObject = otherObject;
    object->SomeOtherObjects.push_back(otherObject);

    REQUIRE(fields.size() == 7);
    // Bool field
    REQUIRE(fields[0]->Type == ObjectFieldType::Boolean);
    REQUIRE(fields[0]->Name == "SomeBoolean");
    REQUIRE(*static_cast<BoolObjectField&>(*fields[0]).GetValuePtr(object) == true);

    // Int32 field
    REQUIRE(fields[1]->Type == ObjectFieldType::Int32);
    REQUIRE(fields[1]->Name == "SomeInt32");
    REQUIRE(*static_cast<I32ObjectField&>(*fields[1]).GetValuePtr(object) == 123);

    // Int64 field
    REQUIRE(fields[2]->Type == ObjectFieldType::Int64);
    REQUIRE(fields[2]->Name == "SomeInt64");
    REQUIRE(*static_cast<I32ObjectField&>(*fields[2]).GetValuePtr(object) == 456);

    // Int32 field
    REQUIRE(fields[3]->Type == ObjectFieldType::Real32);
    REQUIRE(fields[3]->Name == "SomeReal32");
    REQUIRE(*static_cast<R32ObjectField&>(*fields[3]).GetValuePtr(object) == 1.0);

    // Int32 field
    REQUIRE(fields[4]->Type == ObjectFieldType::Real64);
    REQUIRE(fields[4]->Name == "SomeReal64");
    REQUIRE(*static_cast<R64ObjectField&>(*fields[4]).GetValuePtr(object) == 4.56);

    // Int32 field
    REQUIRE(fields[5]->Type == ObjectFieldType::Object);
    REQUIRE(fields[5]->Name == "SomeOtherObject");
    REQUIRE(*static_cast<ObjectObjectField&>(*fields[5]).GetValuePtr(object) == otherObject);

    // Int32 field
    REQUIRE(fields[6]->Type == ObjectFieldType::Array);
    REQUIRE(fields[6]->Name == "SomeOtherObjects");
    REQUIRE(static_cast<ArrayObjectField&>(*fields[6]).ItemType == ObjectFieldType::Object);
    Array<Object*>* otherObjects = (Array<Object*>*) fields[6]->GetUntypedValuePtr(object);
    REQUIRE(otherObjects->size() == 1);
    REQUIRE((*otherObjects)[0] == otherObject);
    REQUIRE(otherObjects == &object->SomeOtherObjects);
}

TEST_CASE("Object class info should be correct", "[object]") {
    REQUIRE(StaticClass<TestObject>()->Parent() == StaticClass<Object>());
    REQUIRE(StaticClass<TestObject>()->IsDerivedFrom<Object>());
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
    REQUIRE(StaticClass<TestDerivedObject>()->StaticInstance() == StaticInstance<TestDerivedObject>());
    REQUIRE_FALSE(StaticClass<TestDerivedObject>()->IsDerivedFrom<TestReferencingArrayObject>());
}

TEST_CASE("Object class info should be able to find derived classes", "[object]") {
    Array<Class*> derivedClasses = StaticClass<TestReferencingObject>()->GetDerivedClasses();
    REQUIRE(derivedClasses.size() == 1);
    if (derivedClasses.size() > 0) {
        REQUIRE(derivedClasses[0] == StaticClass<TestDerivedObject>());
    }
}
