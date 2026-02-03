#pragma once

#include <type_traits>

template<typename T>
using RemoveReference = std::remove_reference_t<T>;

template<typename T>
constexpr RemoveReference<T>&& Move(T&& value) {
    return static_cast<RemoveReference<T>&&>(value);
}

template<typename, typename = void>
constexpr bool IsTypeComplete = false;

template<typename T>
constexpr bool IsTypeComplete<T, std::void_t<decltype(sizeof(T))>> = true;

template<typename Child, typename Parent>
constexpr bool IsDerivedFrom = std::is_base_of_v<Parent, Child>;

template<typename T>
constexpr bool IsEnumType = std::is_enum_v<T>;

template<typename T>
constexpr bool IsEnumClassType = IsEnumType<T> && !std::is_convertible_v<T, int>;

template<typename T>
using UnderlyingType = std::underlying_type_t<T>;

template<typename T, std::enable_if_t<IsEnumType<T>, void*> = nullptr>
struct EnumTraits {
    static constexpr bool IsFlags = false;
};

#define IMPL_ENUM_FLAG_OPERATOR(opToImpl) \
template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr> \
constexpr T operator opToImpl(const T x, const T y) { \
    return static_cast<T>( \
        static_cast<UnderlyingType<T>>(x) opToImpl static_cast<UnderlyingType<T>>(y) \
    ); \
}

IMPL_ENUM_FLAG_OPERATOR(|)
IMPL_ENUM_FLAG_OPERATOR(&)
IMPL_ENUM_FLAG_OPERATOR(^)

#undef IMPL_ENUM_FLAG_OPERATOR

#define IMPL_ENUM_FLAG_OPERATOR(opToImpl) \
template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr> \
T& operator opToImpl(T& x, const T y) { \
    (UnderlyingType<T>&)(x) opToImpl static_cast<UnderlyingType<T>>(y); \
    return x; \
}

IMPL_ENUM_FLAG_OPERATOR(|=)
IMPL_ENUM_FLAG_OPERATOR(&=)
IMPL_ENUM_FLAG_OPERATOR(^=)

#undef IMPL_ENUM_FLAG_OPERATOR

template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr>
constexpr T operator~(const T value) {
    return static_cast<T>(~static_cast<UnderlyingType<T>>(value));
}

#define IMPL_ENUM_FLAG_OPERATOR(opToImpl) \
template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr> \
T operator opToImpl(const T x, const size_t bitshiftBy) { \
    return static_cast<T>( \
        static_cast<UnderlyingType<T>>(x) opToImpl bitshiftBy \
    ); \
}

IMPL_ENUM_FLAG_OPERATOR(<<)
IMPL_ENUM_FLAG_OPERATOR(>>)

#undef IMPL_ENUM_FLAG_OPERATOR

#define IMPL_ENUM_FLAG_OPERATOR(opToImpl) \
template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr> \
T& operator opToImpl(T& x, const size_t bitshiftBy) { \
    static_cast<UnderlyingType<T>&>(x) opToImpl bitshiftBy; \
    return x; \
}

IMPL_ENUM_FLAG_OPERATOR(<<=)
IMPL_ENUM_FLAG_OPERATOR(>>=)

#undef IMPL_ENUM_FLAG_OPERATOR

template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr>
bool HasAnyFlags(const T value, const T flag) {
    return static_cast<UnderlyingType<T>>(value & flag) != 0;
}

template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr>
bool HasAllFlags(const T value, const T flag) {
    return static_cast<UnderlyingType<T>>(value & flag) == flag;
}

template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr>
void SetFlag(T& value, const T flag) {
    value |= flag;
}

template<typename T, std::enable_if_t<EnumTraits<T>::IsFlags, void*> = nullptr>
void UnsetFlag(T& value, const T flag) {
    value &= ~flag;
}

#define DEFINE_ENUM_CLASS_FLAGS(EnumClass) \
    template<> \
    struct EnumTraits<EnumClass> { \
        static constexpr bool IsFlags = true; \
    };

#define MACRO_CONCAT_(a, b) a ## b
#define MACRO_CONCAT(a, b) MACRO_CONCAT_(a, b)
#define MACRO_QUOTE(a) #a
#define MACRO_PARENTHESES ()
#define MACRO_EXPAND(...) MACRO_EXPAND4(MACRO_EXPAND4(MACRO_EXPAND4(MACRO_EXPAND4(__VA_ARGS__))))
#define MACRO_EXPAND4(...) MACRO_EXPAND3(MACRO_EXPAND3(MACRO_EXPAND3(MACRO_EXPAND3(__VA_ARGS__))))
#define MACRO_EXPAND3(...) MACRO_EXPAND2(MACRO_EXPAND2(MACRO_EXPAND2(MACRO_EXPAND2(__VA_ARGS__))))
#define MACRO_EXPAND2(...) MACRO_EXPAND1(MACRO_EXPAND1(MACRO_EXPAND1(MACRO_EXPAND1(__VA_ARGS__))))
#define MACRO_EXPAND1(...) __VA_ARGS__

#define MACRO_FOR_EACH(macro, ...) __VA_OPT__(MACRO_EXPAND(MACRO_FOR_EACH_(macro, __VA_ARGS__)))
#define MACRO_FOR_EACH_AGAIN_() MACRO_FOR_EACH_
#define MACRO_FOR_EACH_(macro, arg1, ...) macro(arg1) __VA_OPT__(, MACRO_FOR_EACH_AGAIN_ MACRO_PARENTHESES (macro, __VA_ARGS__))
