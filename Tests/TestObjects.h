#pragma once

#include "Object/Object.h"

struct TestObject : Object {
   virtual u32 TypeId() const override { return 'TEST'; }
    virtual String TypeName() const override { return "TestObject"; }
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(SomeBoolean);
        EXPOSE_FIELD(SomeInt32);
        EXPOSE_FIELD(SomeInt64);
        EXPOSE_FIELD(SomeReal32);
        EXPOSE_FIELD(SomeReal64);
        EXPOSE_FIELD(SomeOtherObject);
        EXPOSE_FIELD(SomeOtherObjects);
        EXPOSE_FIELD(SomeString);
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

    bool DestroyFinished = false;
};

DECLARE_OBJECT(TestObject);

struct TestReferencingObject : Object {
    virtual u32 TypeId() const override { return 'REFR'; }
    virtual String TypeName() const override { return "TestReferencingObject"; }
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(Next);
    }

    Object* Next;
};

DECLARE_OBJECT(TestReferencingObject);

struct TestReferencingArrayObject : Object {
    virtual u32 TypeId() const override { return 'REFA'; }
    virtual String TypeName() const override { return "TestReferencingArrayObject"; }
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        Object::GetObjectFields(fields);
        EXPOSE_FIELD(Others);
    }

    Array<Object*> Others;
};

DECLARE_OBJECT(TestReferencingArrayObject);

struct TestDelayedDestroyObject : Object {
    virtual u32 TypeId() const override { return 'DSRY'; }
    virtual String TypeName() const override { return "TestDelayedDestroyObject"; }
    virtual bool IsDestroyFinished() const override { return FinishedDestruction; }

    bool FinishedDestruction = false;
};

DECLARE_OBJECT(TestDelayedDestroyObject);

struct TestDerivedObject : TestReferencingObject {
    virtual u32 TypeId() const override { return 'DERV'; }
    virtual String TypeName() const override { return "TestDerivedObject"; }
    virtual void GetObjectFields(Array<UniquePtr<ObjectField>>& fields) const override {
        TestReferencingObject::GetObjectFields(fields);
        EXPOSE_FIELD(Other);
    }

    Object* Other;
};

DECLARE_OBJECT(TestDerivedObject);
