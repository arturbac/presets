// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#pragma once

#include <simple_enum/expected.h>
#include <string>
#include <string_view>
#include <system_error>
#include <cstdint>

namespace presets::inline v2
  {
struct error_info
{
  std::error_code ec;
  std::string info;
};
template<typename T>
using expected_ec = cxx23::expected<T, error_info>;

  }
  
