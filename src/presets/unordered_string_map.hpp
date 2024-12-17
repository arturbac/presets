// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#pragma once

#include <unordered_map>

namespace presets::inline v2
  {
template<typename CharType>
struct string_comparator_t
  {
  using is_transparent = void;  // Enable heterogeneous lookups
  using string_view_type = std::basic_string_view<CharType>;
  using string_type = std::basic_string<CharType>;

  [[nodiscard]]
  bool operator()(string_view_type lhs, string_view_type rhs) const noexcept
    {
    return lhs < rhs;
    }

  [[nodiscard]]
  bool operator()(string_type const & lhs, string_type const & rhs) const noexcept
    {
    return lhs < rhs;
    }

  [[nodiscard]]
  bool operator()(string_type const & lhs, string_view_type rhs) const noexcept
    {
    return lhs < rhs;
    }

  [[nodiscard]]
  bool operator()(string_view_type lhs, string_type const & rhs) const noexcept
    {
    return lhs < rhs;
    }
  };

template<typename CharType>
struct string_hash_t
  {
  using is_transparent = void;  // Enables heterogeneous lookup
  using string_type = std::basic_string<CharType>;
  using string_view_type = std::basic_string_view<CharType>;

  [[nodiscard]]
  auto operator()(string_type const & key) const noexcept -> size_t
    {
    return std::hash<string_view_type>{}(key);
    }

  [[nodiscard]]
  auto operator()(string_view_type key) const noexcept -> size_t
    {
    return std::hash<string_view_type>{}(key);
    }
  };

template<typename CharType>
struct string_equal_t
  {
  using is_transparent = void;  // Enables heterogeneous lookup
  using string_type = std::basic_string<CharType>;
  using string_view_type = std::basic_string_view<CharType>;

  [[nodiscard]]
  auto operator()(string_type const & lhs, string_type const & rhs) const noexcept -> bool
    {
    return lhs == rhs;
    }

  [[nodiscard]]
  auto operator()(string_view_type lhs, string_view_type rhs) const noexcept -> bool
    {
    return lhs == rhs;
    }
  };

template<typename value_type>
using unordered_string_map = std::unordered_map<std::string, value_type, string_hash_t<char>, string_equal_t<char>>;

  }  // namespace presets::inline v2

