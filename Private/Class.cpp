#include "Object/Object.h"

bool Class::IsDerivedFrom(Class* parentClass) {
    if (!IsValid(parentClass)) {
        return false;
    }

    if (ClassTypeId() == parentClass->ClassTypeId()) {
        return true;
    }

    if (!IsValid(Parent())) {
        return false;
    }

    return Parent()->IsDerivedFrom(parentClass);
}

template<>
void Detail::ConfigureClass<Object>(Class* classInstance) {
    classInstance->parent = nullptr;
    classInstance->name = "Object";
    classInstance->typeId = StaticTypeId<Object>();
    StaticInstance<Object>()->GetObjectFields(classInstance->fields);
    StaticInstance<Object>()->classInstance = classInstance;
}

template<>
void Detail::ConfigureClass<Class>(Class* classInstance) {
    classInstance->parent = StaticClass<Object>();
    classInstance->name = "Class";
    classInstance->typeId = StaticTypeId<Class>();
    StaticInstance<Class>()->GetObjectFields(classInstance->fields);
    StaticInstance<Class>()->classInstance = classInstance;
}

static bool configuredObjectClassInstance_Object = []{
    StaticClass<Object>();
    return true;
}();

static bool configuredObjectClassInstance_Class = []{
    StaticClass<Class>();
    return true;
}();

