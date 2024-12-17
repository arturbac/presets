// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#pragma once
#include <presets/presets.hpp>
#include <simple_enum/generic_error_category.hpp>
#include <format>

namespace presets::inline v2
  {
using cxx23::unexpected;

enum struct error_code
  {
  cannot_load_file,
  invalid_json,
  invalid_preset_version,
  invalid_config,
  invalid_variable_reference,
  undefined_variable_reference,
  undefined_inherited_preset,
  circular_inclusion,
  invalid_type_expected_variable_key_value_pairs,
  invalid_type_expected_value_definition,
  variable_redeclaration,
  preset_redefinition,
  empty_data,
  invalid_preset_name
  };
constexpr int expected_version = 2;

consteval auto adl_enum_bounds(error_code)
  {
  using enum error_code;
  return simple_enum::adl_info{cannot_load_file, invalid_preset_name, true};
  }

template<simple_enum::concepts::error_enum en, typename... Args>
  requires(std::formattable<Args, char> && ...)
static auto make_unexpected_error(en error, std::format_string<Args...> fmt, Args &&... args) noexcept
  -> unexpected<error_info>
  {
  return unexpected{error_info{simple_enum::make_error_code(error), std::format(fmt, std::forward<Args>(args)...)}};
  }

template<simple_enum::concepts::error_enum en>
static auto make_unexpected_error(en error) noexcept -> unexpected<error_info>
  {
  return unexpected{error_info{simple_enum::make_error_code(error)}};
  }

  }  // namespace presets::inline v2
