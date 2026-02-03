#include "Object/Object.h"

#include <algorithm>

IMPL_OBJECT(Enum, Object);

const String& Enum::ToString(const i32 value) const {
    for (i32 index = 0; index < values.size(); ++index) {
        if (values[index] == value) {
            return enumerators[index];
        }
    }

    static String unknown = "UNKNOWN_ENUMERATOR_VALUE";
    return unknown;
}

i32 Enum::FromString(const String& value) const {
    auto tolower = [](const String& v) {
        String lowerValue = v;
        std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), [](char c) { return std::tolower(c); });
        return lowerValue;
    };

    const String lowerValue = tolower(value);
    for (i32 index = 0; index < enumerators.size(); ++index) {
        if (lowerValue == tolower(enumerators[index])) {
            return values[index];
        }
    }

    return -1;
}

Array<Class*>& GetAllClasses() {
    static Array<Class*> allClasses;
    return allClasses;
}

Array<Class*> Class::GetDerivedClasses() const {
    Array<Class*> derivedClasses;
    for (Class* maybeDerivedClass : GetAllClasses()) {
        if (maybeDerivedClass != this && maybeDerivedClass->IsDerivedFrom(this)) {
            derivedClasses.emplace_back(maybeDerivedClass);
        }
    }
    return derivedClasses;
}

bool Class::IsDerivedFrom(const Class* parentClass) const {
    if (!IsValid(parentClass)) {
        return false;
    }

    if (this == parentClass) {
        return true;
    }

    if (!IsValid(Parent())) {
        return false;
    }

    return Parent()->IsDerivedFrom(parentClass);
}

void Class::Construct(Object* object) {
    constructor(object);
}

void Class::Register() {
    GetAllClasses().emplace_back(this);
}

template<>
void Detail::ConfigureClass<Object>(Class* classInstance) {
    classInstance->parent = nullptr;
    classInstance->name = "Object";
    classInstance->size = sizeof(Object);
    classInstance->constructor = [](Object* object) {
        new (object) Object{};
    };
    StaticInstance<Object>()->GetObjectFields(classInstance->fields);
    StaticInstance<Object>()->classInstance = classInstance;
    classInstance->staticInstance = StaticInstance<Object>();
    classInstance->Register();
}

template<>
void Detail::ConfigureClass<Class>(Class* classInstance) {
    classInstance->parent = StaticClass<Object>();
    classInstance->name = "Class";
    classInstance->size = sizeof(Class);
    classInstance->constructor = [](Object* object) {
        new (object) Class{};
    };
    StaticInstance<Class>()->GetObjectFields(classInstance->fields);
    StaticInstance<Class>()->classInstance = classInstance;
    classInstance->staticInstance = StaticInstance<Class>();
    classInstance->Register();
}

static bool configuredObjectClassInstance_Object = []{
    StaticClass<Object>();
    return true;
}();

static bool configuredObjectClassInstance_Class = []{
    StaticClass<Class>();
    return true;
}();

