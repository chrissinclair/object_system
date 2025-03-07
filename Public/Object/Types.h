#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using r32 = float;
using r64 = double;

using String = std::string;
template<typename T>
using Array = std::vector<T>;
template<typename K, typename V>
using Map = std::unordered_map<K, V>;
template<typename T>
using UniquePtr = std::unique_ptr<T>;
template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
