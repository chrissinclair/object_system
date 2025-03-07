#include "TestObjects.h"

IMPL_OBJECT(TestObject, Object);
IMPL_OBJECT(TestReferencingObject, Object);
IMPL_OBJECT(TestReferencingArrayObject, Object);
IMPL_OBJECT(TestDelayedDestroyObject, Object);
IMPL_OBJECT(TestDerivedObject, TestReferencingObject);
