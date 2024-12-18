// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#pragma once
#include <string>
#include <string_view>
#include <simple_enum/expected.h>
#include <system_error>
#include <vector>
#include <memory>
#include <span>
#include <variant>

namespace presets::inline v2
  {
struct error_info
{
  std::error_code ec;
  std::string info;
};
template<typename T>
using expected_ec = cxx23::expected<T, error_info>;

struct config_argument
  {
  std::string name;
  std::variant<std::string, double, bool> value;

  inline auto operator==(config_argument const & r) const noexcept -> bool = default;
  inline auto operator<=>(config_argument const & r) const noexcept = default;
  };

struct preset_info
  {
  std::string_view name;
  std::string_view description;
  };

struct presets
  {
  using argument_list = std::vector<config_argument>;
  struct data;
  std::unique_ptr<data> data_;

  explicit presets( std::unique_ptr<data> && data ) noexcept;
  presets(presets && r) noexcept;
  auto operator=(presets && r) noexcept -> presets &;
  ~presets() noexcept;

  [[nodiscard]]
  auto variables() const noexcept [[clang::lifetimebound]]-> std::span<config_argument const>;
  [[nodiscard]]
  auto preset(std::string_view preset_name) const noexcept -> expected_ec<argument_list>;
  
  [[nodiscard]]
  auto list_presets() const noexcept -> std::vector<preset_info>;
  };

[[nodiscard]]
auto read_presets(std::string_view preset_file_path) noexcept -> expected_ec<presets>;
  }  // namespace presets::inline v2
