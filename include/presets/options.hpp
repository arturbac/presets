// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#pragma once

#include <presets/error_info.hpp>
#include <concepts>
#include <vector>
#include <ranges>
#include <memory>

namespace presets::inline v2::po
  {

// Forward declarations
template<typename T>
struct value_t;

// Generic value template
template<typename T>
struct value_t final
  {
  };

template<>
struct value_t<std::string> final
  {
  std::string * storage_;

  explicit value_t(std::string & storage) : storage_{&storage} {}

  value_t(value_t &&) noexcept = default;
  value_t(value_t const &) noexcept = default;
  value_t & operator=(value_t &&) noexcept = default;
  value_t & operator=(value_t const &) noexcept = default;
  auto parse(std::string_view str) const -> void;
  };

template<std::integral T>
struct value_t<T> final
  {
  T * storage_;

  explicit value_t(T & storage) : storage_{&storage} {}

  value_t(value_t &&) noexcept = default;
  value_t(value_t const &) noexcept = default;
  value_t & operator=(value_t &&) noexcept = default;
  value_t & operator=(value_t const &) noexcept = default;

  auto parse(std::string_view str) const -> void
    {
    if constexpr(std::is_signed_v<T>)
      *storage_ = T(std::stoll(std::string{str}));
    else
      *storage_ = T(std::stoull(std::string{str}));
    }
  };

template<>
struct value_t<double> final
  {
  double * storage_;

  explicit value_t(double & storage) : storage_{&storage} {}

  value_t(value_t &&) noexcept = default;
  value_t(value_t const &) noexcept = default;
  value_t & operator=(value_t &&) noexcept = default;
  value_t & operator=(value_t const &) noexcept = default;
  auto parse(std::string_view str) const -> void;
  };

// Specialization for vector
template<typename T>
struct value_t<std::vector<T>> final
  {
  std::vector<T> * storage_;

  explicit value_t(std::vector<T> & storage) : storage_{&storage} {}

  value_t(value_t &&) noexcept = default;
  value_t(value_t const &) noexcept = default;
  value_t & operator=(value_t &&) noexcept = default;
  value_t & operator=(value_t const &) noexcept = default;

  auto parse(std::string_view str) const -> void
    {
    T value;
    value_t<T> parser{value};
    parser.parse(str);
    storage_->emplace_back(std::move(value));
    }
  };

namespace concepts
  {

  template<typename T>
  concept parseable = requires(value_t<T> const & t, std::string_view sv) {
    { t.parse(sv) } -> std::same_as<void>;
  };
  }  // namespace concepts

class value_semantic_t final
  {
  struct value_interface_t
    {
    virtual ~value_interface_t() noexcept;
    virtual auto parse(std::string_view str) const -> void = 0;
    };

  template<concepts::parseable T>
  struct walue_wrapper_t final : public value_interface_t
    {
    value_t<T> obj;

    walue_wrapper_t() noexcept = default;

    template<concepts::parseable V>
      requires std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<V>>
    walue_wrapper_t(value_t<V> && value) : obj{std::forward<value_t<V>>(value)}
      {
      }

    auto parse(std::string_view str) const -> void override { obj.parse(str); }
    };

  std::unique_ptr<value_interface_t> stored_value_;

public:
  template<typename T>
  explicit value_semantic_t(value_t<T> && value) : stored_value_(std::make_unique<walue_wrapper_t<T>>(std::move(value)))
    {
    }

  auto parse(std::string_view str) -> void;
  };

struct option_t
  {
  std::string key;
  std::string value;

  [[nodiscard]]
  constexpr auto operator==(option_t const &) const noexcept -> bool
    = default;
  };

namespace detail
  {
  [[nodiscard]]
  constexpr auto is_option(std::string_view arg) noexcept -> bool
    {
    return arg.starts_with(std::string_view{"-"}) || arg.starts_with(std::string_view{"\\"});
    }

  [[nodiscard]]
  constexpr auto is_value(std::string_view arg) noexcept -> bool
    {
    return !is_option(arg);
    }
  }  // namespace detail

[[nodiscard]]
auto process_args(std::span<char const * const> args) noexcept -> expected_ec<std::vector<option_t>>;

struct option_description
  {
  std::string option_;
  std::string description_;
  };

struct option_descriptions;

struct option_constructor
  {
  option_descriptions * parent_;
  auto operator()(std::string_view option, std::string_view description) -> option_constructor;
  };

struct option_descriptions
  {
  std::string caption_;
  std::vector<option_description> options_;

  template<typename T>
  explicit option_descriptions(T && caption) : caption_(std::forward<T>(caption))
    {
    }

  auto add_options() -> option_constructor { return option_constructor{this}; }
  };
  }  // namespace presets::inline v2::po
