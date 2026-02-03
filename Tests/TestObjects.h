#pragma once

#include "Object/Object.h"

enum class TestEnum {
    FirstEnumerator,
    SecondEnumerator = 3,
};
DEFINE_ENUM_CLASS_FLAGS(TestEnum)
DECLARE_ENUM(TestEnum)

struct TestReferenceStruct : Struct {
    OBJECT_BODY(TestReferenceStruct, Struct)

    Object* SomeOtherObject;
    OBJECT_PROPERTY(SomeOtherObject);
};
DECLARE_STRUCT(TestReferenceStruct);

struct TestStruct : Struct {
    OBJECT_BODY(TestStruct, Struct)

    i32 SomeProperty;
    OBJECT_PROPERTY(SomeProperty)

    Object* SomeOtherObject;
    OBJECT_PROPERTY(SomeOtherObject);

    Array<Object*> SomeOtherObjects;
    OBJECT_PROPERTY(SomeOtherObjects);

    Array<TestReferenceStruct> SomeOtherStructs;
    OBJECT_PROPERTY(SomeOtherStructs);
};
DECLARE_STRUCT(TestStruct);

struct TestObject : Object {
    OBJECT_BODY(TestObject, Object)

    virtual bool IsDestroyFinished() const override {
        return DestroyFinished;
    }

    bool SomeBoolean;
    OBJECT_PROPERTY(SomeBoolean, TestTag=TestTagValue, OtherTag=OtherTagValue)

    i32 SomeInt32;
    OBJECT_PROPERTY(SomeInt32, TestTag=AnotherTestTagValue)

    i64 SomeInt64;
    OBJECT_PROPERTY(SomeInt64, TestFlag)

    r32 SomeReal32;
    OBJECT_PROPERTY(SomeReal32)

    r64 SomeReal64;
    OBJECT_PROPERTY(SomeReal64)

    Object* SomeOtherObject;
    OBJECT_PROPERTY(SomeOtherObject)

    Array<Object*> SomeOtherObjects;
    OBJECT_PROPERTY(SomeOtherObjects)

    String SomeString;
    OBJECT_PROPERTY(SomeString)

    TestEnum SomeEnum;
    OBJECT_PROPERTY(SomeEnum)

    TestStruct SomeStruct;
    OBJECT_PROPERTY(SomeStruct)

    bool DestroyFinished = false;
};
DECLARE_OBJECT(TestObject);

struct TestReferencingObject : Object {
    OBJECT_BODY(TestReferencingObject, Object)

    Object* Next;
    OBJECT_PROPERTY(Next)
};
DECLARE_OBJECT(TestReferencingObject);

struct TestReferencingArrayObject : Object {
    OBJECT_BODY(TestReferencingArrayObject, Object)

    Array<Object*> Others;
    OBJECT_PROPERTY(Others)
};
DECLARE_OBJECT(TestReferencingArrayObject);

struct TestDelayedDestroyObject : Object {
    OBJECT_BODY(TestDelayedDestroyObject, Object)
    virtual bool IsDestroyFinished() const override { return FinishedDestruction; }

    bool FinishedDestruction = false;
};
DECLARE_OBJECT(TestDelayedDestroyObject);

struct TestDerivedObject : TestReferencingObject {
    OBJECT_BODY(TestDerivedObject, TestReferencingObject)

    Object* Other;
    OBJECT_PROPERTY(Other)
};
DECLARE_OBJECT(TestDerivedObject);
