#include "TestObjects.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Unreferenced objects should be collected", "[GC]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    TestReferencingObject* object2 = NewObject<TestReferencingObject>();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(object->GetGeneration() == 1);
    REQUIRE(object2->GetGeneration() == 1);

    Object::CollectGarbage();

    REQUIRE_FALSE(IsValid(object));
    REQUIRE_FALSE(IsValid(object2));
}

TEST_CASE("RootSet objects should not be collected", "[GC]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    TestReferencingObject* object2 = NewObject<TestReferencingObject>();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));

    object->AddToRootSet();

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE_FALSE(IsValid(object2));

    object->RemoveFromRootSet();

    Object::CollectGarbage();

    REQUIRE_FALSE(IsValid(object));
}

TEST_CASE("Objects referenced from the root set should not be collected", "[GC]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    TestReferencingObject* object2 = NewObject<TestReferencingObject>();
    TestReferencingObject* object3 = NewObject<TestReferencingObject>();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(IsValid(object3));

    object->AddToRootSet();
    object->Next = object2;
    object2->Next = object3;

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(IsValid(object3));

    // Stop referencing object 3
    object2->Next = nullptr;

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE_FALSE(IsValid(object3));
}

TEST_CASE("Objects referenced through arrays should not be collected", "[GC]") {
    TestReferencingArrayObject* object = NewObject<TestReferencingArrayObject>();
    TestReferencingArrayObject* object2 = NewObject<TestReferencingArrayObject>();
    TestReferencingObject* object3 = NewObject<TestReferencingObject>();
    TestReferencingObject* object4 = NewObject<TestReferencingObject>();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(IsValid(object3));
    REQUIRE(IsValid(object4));

    object->AddToRootSet();
    object->Others.push_back(object2);
    object->Others.push_back(object3);
    object2->Others.push_back(object4);

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(IsValid(object3));
    REQUIRE(IsValid(object4));

    object2->Others.clear();

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(IsValid(object3));
    REQUIRE_FALSE(IsValid(object4));

    // Remove the reference to object 3
    object->Others.erase(object->Others.begin() + 1, object->Others.end());

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE_FALSE(IsValid(object3));
    REQUIRE_FALSE(IsValid(object4));
}

TEST_CASE("Objects reference through structs should not be collected", "[GC]") {
    TestObject* object = NewObject<TestObject>();
    TestObject* object2 = NewObject<TestObject>();
    TestObject* object3 = NewObject<TestObject>();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE(IsValid(object3));

    object->AddToRootSet();
    object->SomeStruct.SomeOtherObject = object2;

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE_FALSE(IsValid(object3));

    object->SomeStruct.SomeOtherObject = nullptr;
    object->SomeStruct.SomeOtherObjects.push_back(object2);

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE_FALSE(IsValid(object3));

    object->SomeStruct.SomeOtherObjects.clear();
    object->SomeStruct.SomeOtherStructs.emplace_back().SomeOtherObject = object2;

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object2));
    REQUIRE_FALSE(IsValid(object3));

    object->SomeStruct.SomeOtherStructs.clear();

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE_FALSE(IsValid(object2));
    REQUIRE_FALSE(IsValid(object3));
}

TEST_CASE("Pointers to destroyed objects should be nulled out", "[GC]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    TestReferencingObject* objectToDestroy = NewObject<TestReferencingObject>();

    object->AddToRootSet();
    object->Next = objectToDestroy;

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object->Next));

    objectToDestroy->Destroy();

    REQUIRE(IsValid(object));
    REQUIRE_FALSE(IsValid(object->Next));
    REQUIRE(object->Next == objectToDestroy);

    Object::CollectGarbage();

    REQUIRE(IsValid(object));
    REQUIRE_FALSE(IsValid(object->Next));
    REQUIRE(object->Next == nullptr);
}

TEST_CASE("Pointers to destroyed objects should be nulled during destruction", "[GC]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    TestDelayedDestroyObject* objectToDestroy = NewObject<TestDelayedDestroyObject>();

    object->AddToRootSet();
    object->Next = objectToDestroy;

    REQUIRE(IsValid(object));
    REQUIRE(IsValid(object->Next));

    objectToDestroy->Destroy();

    REQUIRE(IsValid(object));
    REQUIRE_FALSE(IsValid(object->Next));
    REQUIRE(object->Next == objectToDestroy);
    REQUIRE(HasAnyFlags(objectToDestroy->GetFlags(), ObjectFlags::IsBeingDestroyed));
    REQUIRE_FALSE(HasAnyFlags(objectToDestroy->GetFlags(), ObjectFlags::IsDestroyed));

    Object::CollectGarbage();

    REQUIRE_FALSE(IsValid(object->Next));
    REQUIRE(object->Next == nullptr);
    REQUIRE(HasAnyFlags(objectToDestroy->GetFlags(), ObjectFlags::IsBeingDestroyed));
    REQUIRE_FALSE(HasAnyFlags(objectToDestroy->GetFlags(), ObjectFlags::IsDestroyed));

    objectToDestroy->FinishedDestruction = true;

    Object::CollectGarbage();

    REQUIRE_FALSE(HasAnyFlags(objectToDestroy->GetFlags(), ObjectFlags::IsBeingDestroyed));
    REQUIRE(HasAnyFlags(objectToDestroy->GetFlags(), ObjectFlags::IsDestroyed));
}
