#include "TestObjects.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Strong object pointers prevent garbage collection", "[StrongObjectPtr]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    WeakObjectPtr<TestReferencingObject> weakObject(object);
    REQUIRE(weakObject.IsValid());

    {
        StrongObjectPtr<TestReferencingObject> strongObject(object);

        REQUIRE(strongObject);
        REQUIRE(strongObject.IsValid());
        REQUIRE(strongObject.Get() == object);
        strongObject->Next = object; // This is just to validate that typed strong pointers do provide the typed value

        Object::CollectGarbage();

        REQUIRE(strongObject);
        REQUIRE(strongObject.IsValid());
        REQUIRE(strongObject.Get() == object);
    }

    // Ensure that once the strong pointer is destroyed, the object can be garbage collected
    Object::CollectGarbage();

    REQUIRE_FALSE(weakObject);
    REQUIRE_FALSE(weakObject.IsValid());
    REQUIRE(weakObject.Get() == nullptr);
}


TEST_CASE("Strong object pointers should only release when all destroyed", "[StrongObjectPtr]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    WeakObjectPtr<TestReferencingObject> weakObject(object);
    REQUIRE(weakObject.IsValid());

    {
        StrongObjectPtr<TestReferencingObject> strongObject(object);

        REQUIRE(strongObject);
        REQUIRE(strongObject.IsValid());
        REQUIRE(strongObject.Get() == object);

        {

            StrongObjectPtr<TestReferencingObject> strongObject2(object);
            REQUIRE(strongObject2);
            REQUIRE(strongObject2.IsValid());
            REQUIRE(strongObject2.Get() == object);

            Object::CollectGarbage();

            REQUIRE(strongObject);
            REQUIRE(strongObject.IsValid());
            REQUIRE(strongObject.Get() == object);
            REQUIRE(strongObject2);
            REQUIRE(strongObject2.IsValid());
            REQUIRE(strongObject2.Get() == object);
        }
        // Strong object 2 destroyed, but strongObject should keep the object alive

        Object::CollectGarbage();
        REQUIRE(strongObject);
        REQUIRE(strongObject.IsValid());
        REQUIRE(strongObject.Get() == object);
    }

    // Now both destroyed, should be valid for garbage collection again
    Object::CollectGarbage();
    REQUIRE_FALSE(weakObject);
    REQUIRE_FALSE(weakObject.IsValid());
    REQUIRE(weakObject.Get() == nullptr);
}

TEST_CASE("Strong object pointer should be valid until object is destroyed", "[StrongObjectPtr]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    StrongObjectPtr<TestReferencingObject> strongObject(object);

    REQUIRE(strongObject);
    REQUIRE(strongObject.IsValid());
    REQUIRE(strongObject.Get() == object);

    object->Destroy();

    REQUIRE_FALSE(strongObject);
    REQUIRE_FALSE(strongObject.IsValid());
    REQUIRE(strongObject.Get() == nullptr);

    TestDelayedDestroyObject* delayedDestroyObject = NewObject<TestDelayedDestroyObject>();
    StrongObjectPtr<TestDelayedDestroyObject> strongDelayedDestroyObject(delayedDestroyObject);

    REQUIRE(strongDelayedDestroyObject);
    REQUIRE(strongDelayedDestroyObject.IsValid());
    REQUIRE(strongDelayedDestroyObject.Get() == delayedDestroyObject);

    strongDelayedDestroyObject->Destroy();

    REQUIRE_FALSE(delayedDestroyObject->IsDestroyFinished());
    REQUIRE_FALSE(strongDelayedDestroyObject);
    REQUIRE_FALSE(strongDelayedDestroyObject.IsValid());
    REQUIRE(strongDelayedDestroyObject.Get() == nullptr);

    delayedDestroyObject->FinishedDestruction = true;
    Object::CollectGarbage();

    REQUIRE_FALSE(strongDelayedDestroyObject);
    REQUIRE_FALSE(strongDelayedDestroyObject.IsValid());
    REQUIRE(strongDelayedDestroyObject.Get() == nullptr);
}
