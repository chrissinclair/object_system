#include "Object/TypeTraits.h"

#include "TestObjects.h"

#include <catch2/catch_test_macros.hpp>

enum class TestBitFlags {
    None = 0,
    Bit1 = 1,
    Bit2 = 2,
    Bit3 = 4,
};
DEFINE_ENUM_CLASS_FLAGS(TestBitFlags)

TEST_CASE("HasFlag should be true if the flag is set, and false if unset", "[TypeTraits]") {
    TestBitFlags value = TestBitFlags::None;
    REQUIRE_FALSE(HasAnyFlags(value, TestBitFlags::Bit1));
    REQUIRE_FALSE(HasAnyFlags(value, TestBitFlags::Bit2));

    SetFlag(value, TestBitFlags::Bit1);
    REQUIRE(HasAnyFlags(value, TestBitFlags::Bit1));
    REQUIRE_FALSE(HasAnyFlags(value, TestBitFlags::Bit2));

    UnsetFlag(value, TestBitFlags::Bit1);
    REQUIRE_FALSE(HasAnyFlags(value, TestBitFlags::Bit1));
    REQUIRE_FALSE(HasAnyFlags(value, TestBitFlags::Bit2));
}

TEST_CASE("IsTypeComplete should be true for defined structs, and false otherwise", "[TypeTraits]") {
    REQUIRE(IsTypeComplete<TestStruct>);
    REQUIRE_FALSE(IsTypeComplete<struct ForwardDeclaredStruct>);
}
