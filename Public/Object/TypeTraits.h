#pragma once

#include <type_traits>

template<typename T>
using RemoveReference = std::remove_reference_t<T>;

template<typename T>
constexpr RemoveReference<T>&& Move(T&& value) {
    return static_cast<RemoveReference<T>&&>(value);
}

template<typename Child, typename Parent>
constexpr bool IsDerivedFrom = std::is_base_of_v<Parent, Child>;

template<typename T>
using UnderlyingType = std::underlying_type_t<T>;

template<typename T, std::enable_if_t<std::is_enum_v<T>, void*> = nullptr>
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
