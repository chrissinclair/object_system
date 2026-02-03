#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define MAGIC_ENUM_NO_ASSERT
#include <magic_enum/magic_enum.hpp>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using isize = std::ptrdiff_t;
using r32 = float;
using r64 = double;

static constexpr i8 i8_min = std::numeric_limits<i8>::min();
static constexpr i8 i8_max = std::numeric_limits<i8>::max();
static constexpr i16 i16_min = std::numeric_limits<i16>::min();
static constexpr i16 i16_max = std::numeric_limits<i16>::max();
static constexpr i32 i32_min = std::numeric_limits<i32>::min();
static constexpr i32 i32_max = std::numeric_limits<i32>::max();
static constexpr i64 i64_min = std::numeric_limits<i64>::min();
static constexpr i64 i64_max = std::numeric_limits<i64>::max();
static constexpr isize isize_min = std::numeric_limits<isize>::min();
static constexpr isize isize_max = std::numeric_limits<isize>::max();
static constexpr u8 u8_min = std::numeric_limits<u8>::min();
static constexpr u8 u8_max = std::numeric_limits<u8>::max();
static constexpr u16 u16_min = std::numeric_limits<u16>::min();
static constexpr u16 u16_max = std::numeric_limits<u16>::max();
static constexpr u32 u32_min = std::numeric_limits<u32>::min();
static constexpr u32 u32_max = std::numeric_limits<u32>::max();
static constexpr u64 u64_min = std::numeric_limits<u64>::min();
static constexpr u64 u64_max = std::numeric_limits<u64>::max();
static constexpr usize usize_min = std::numeric_limits<usize>::min();
static constexpr usize usize_max = std::numeric_limits<usize>::max();
static constexpr r32 r32_min = std::numeric_limits<r32>::min();
static constexpr r32 r32_max = std::numeric_limits<r32>::max();
static constexpr r32 r32_small = 0.0001f;
static constexpr r32 r32_verysmall = 0.000001f;
static_assert(r32_verysmall >= std::numeric_limits<r32>::epsilon(), "r32_verysmall is too small");
static constexpr r64 r64_min = std::numeric_limits<r64>::min();
static constexpr r64 r64_max = std::numeric_limits<r64>::max();
static constexpr r64 r64_small = 0.0001;
static constexpr r64 r64_verysmall = 0.000001;
static_assert(r64_verysmall >= std::numeric_limits<r64>::epsilon(), "r64_verysmall is too small");

using String = std::string;
template<typename T>
using Array = std::vector<T>;
template<typename K, typename V>
using Map = std::unordered_map<K, V>;
template<typename T>
using Set = std::unordered_set<T>;
template<typename T>
using UniquePtr = std::unique_ptr<T>;
template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
