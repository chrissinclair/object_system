#pragma once

#include "Object/Object.h"

enum class TestEnum {
    FirstEnumerator,
    SecondEnumerator = 3,
};
DEFINE_ENUM_CLASS_FLAGS(TestEnum)
DECLARE_ENUM(TestEnum)

struct TestObject : Object {
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(SomeBoolean)
            ->WithTag("TestTag", "TestTagValue")
            ->WithTag("OtherTag", "OtherTagValue");
        EXPOSE_FIELD(SomeInt32)
            ->WithTag("TestTag", "AnotherTestTagValue");
        EXPOSE_FIELD(SomeInt64);
        EXPOSE_FIELD(SomeReal32);
        EXPOSE_FIELD(SomeReal64);
        EXPOSE_FIELD(SomeOtherObject);
        EXPOSE_FIELD(SomeOtherObjects);
        EXPOSE_FIELD(SomeString);
        EXPOSE_FIELD(SomeEnum);
    }

    virtual bool IsDestroyFinished() const override {
        return DestroyFinished;
    }

    bool SomeBoolean;
    i32 SomeInt32;
    i64 SomeInt64;
    r32 SomeReal32;
    r64 SomeReal64;
    Object* SomeOtherObject;
    Array<Object*> SomeOtherObjects;
    String SomeString;
    TestEnum SomeEnum;

    bool DestroyFinished = false;
};

DECLARE_OBJECT(TestObject);

struct TestReferencingObject : Object {
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(Next);
    }

    Object* Next;
};

DECLARE_OBJECT(TestReferencingObject);

struct TestReferencingArrayObject : Object {
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(Others);
    }

    Array<Object*> Others;
};

DECLARE_OBJECT(TestReferencingArrayObject);

struct TestDelayedDestroyObject : Object {
    virtual bool IsDestroyFinished() const override { return FinishedDestruction; }

    bool FinishedDestruction = false;
};

DECLARE_OBJECT(TestDelayedDestroyObject);

struct TestDerivedObject : TestReferencingObject {
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        TestReferencingObject::GetObjectFields(fields);
        EXPOSE_FIELD(Other);
    }

    Object* Other;
};

DECLARE_OBJECT(TestDerivedObject);
