#include "TestObjects.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Weak object pointer should be valid until object is garbage collected", "[WeakObjectPtr]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    WeakObjectPtr<TestReferencingObject> weakObject(object);

    REQUIRE(weakObject);
    REQUIRE(weakObject.IsValid());
    REQUIRE(weakObject.Get() == object);
    weakObject->Next = object; // This is just to validate that typed weak pointers do provide the typed value

    Object::CollectGarbage();

    REQUIRE_FALSE(weakObject);
    REQUIRE_FALSE(weakObject.IsValid());
    REQUIRE(weakObject.Get() == nullptr);
}

TEST_CASE("Weak object pointer should be valid until object is destroyed", "[WeakObjectPtr]") {
    TestReferencingObject* object = NewObject<TestReferencingObject>();
    WeakObjectPtr<TestReferencingObject> weakObject(object);

    REQUIRE(weakObject);
    REQUIRE(weakObject.IsValid());
    REQUIRE(weakObject.Get() == object);

    object->Destroy();

    REQUIRE_FALSE(weakObject);
    REQUIRE_FALSE(weakObject.IsValid());
    REQUIRE(weakObject.Get() == nullptr);

    TestDelayedDestroyObject* delayedDestroyObject = NewObject<TestDelayedDestroyObject>();
    WeakObjectPtr<TestDelayedDestroyObject> weakDelayedDestroyObject(delayedDestroyObject);

    REQUIRE(weakDelayedDestroyObject);
    REQUIRE(weakDelayedDestroyObject.IsValid());
    REQUIRE(weakDelayedDestroyObject.Get() == delayedDestroyObject);

    weakDelayedDestroyObject->Destroy();

    REQUIRE_FALSE(delayedDestroyObject->IsDestroyFinished());
    REQUIRE_FALSE(weakDelayedDestroyObject);
    REQUIRE_FALSE(weakDelayedDestroyObject.IsValid());
    REQUIRE(weakDelayedDestroyObject.Get() == nullptr);

    delayedDestroyObject->FinishedDestruction = true;
    Object::CollectGarbage();

    REQUIRE_FALSE(weakDelayedDestroyObject);
    REQUIRE_FALSE(weakDelayedDestroyObject.IsValid());
    REQUIRE(weakDelayedDestroyObject.Get() == nullptr);
}
