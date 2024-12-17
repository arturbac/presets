// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#include <presets/error_handling.hpp>
#include <presets/unordered_string_map.hpp>
#include <glaze/json/json_t.hpp>
#include <simple_enum/generic_error_category_impl.hpp>
#include <filesystem>
#include <variant>
namespace ranges = std::ranges;
namespace views = std::views;

namespace glz
  {
consteval auto adl_enum_bounds(error_code)
  {
  using enum error_code;
  return simple_enum::adl_info{none, includer_error, true};
  }
  }  // namespace glz
template class simple_enum::generic_error_category<glz::error_code>;
template class simple_enum::generic_error_category<presets::error_code>;

namespace presets::inline v2
  {
namespace
  {
  struct argument
    {
    std::variant<std::string, double, bool> value;
    };

  using argument_map = unordered_string_map<argument>;

  struct preset_t
    {
    std::string name;
    std::string description;
    bool hidden{};
    std::vector<preset_t const *> inherits;
    argument_map arguments;
    };

  using presets_map = unordered_string_map<std::unique_ptr<preset_t>>;
  }  // namespace

struct presets::data
  {
  argument_list arguments;
  presets_map presets;
  };

struct raw_preset_file_t
  {
  struct raw_preset_t
    {
    std::string name;
    std::string description;
    bool hidden{};
    std::vector<std::string> inherits;
    glz::json_t arguments;
    };

  int version{};
  glz::json_t variables;
  std::vector<std::string> includes;
  std::vector<raw_preset_t> presets;
  };

namespace
  {

  struct ctx_t
    {
    using include_map = unordered_string_map<raw_preset_file_t>;

    std::filesystem::path base_directory;
    include_map include{};
    argument_map variables;
    presets_map presets;  // built presets
    };

  struct variable_info
    {
    std::string_view name;
    std::string_view::size_type pos;
    };

  using enum error_code;

  [[nodiscard]]
  auto find_var_placeholder(std::string_view value) -> expected_ec<std::optional<variable_info>>
    {
    auto pos{value.find('$')};
    if(pos != std::string_view::npos) [[likely]]
      {
      if(value.size() <= (pos + 3u)) [[unlikely]]
        return make_unexpected_error(invalid_variable_reference, "invalid variable declaration in value {}", value);
      if(value[pos + 1u] != '{') [[unlikely]]
        return make_unexpected_error(
          invalid_variable_reference, "invalid variable declaration in value missing '{{' {}", value
        );

      auto pos2{value.find('}', pos + 2u)};
      if(pos2 == std::string_view::npos) [[unlikely]]
        return make_unexpected_error(
          invalid_variable_reference, "invalid variable declaration in value missing '}}' {}", value
        );
      auto var_name{value.substr(pos + 2u, pos2 - pos - 2u)};
      if(var_name.empty()) [[unlikely]]
        return make_unexpected_error(invalid_variable_reference, "invalid variable name {}", value);
      return variable_info{.name = var_name, .pos = pos};
      }
    return {};
    }

  [[nodiscard]]
  auto to_string(std::variant<std::string, double, bool> const & var) noexcept -> std::string
    {
    return std::visit(
      []<typename T>(T const & val) -> std::string
      {
        if constexpr(std::same_as<std::decay_t<T>, std::string>)
          return val;
        else
          return std::to_string(val);
      },
      var
    );
    }

  [[nodiscard]]
  auto find_inherited_argument(std::string_view key, preset_t const & current_preset) noexcept
    -> std::optional<argument>
    {
    auto it{current_preset.arguments.find(key)};
    if(it != current_preset.arguments.end())
      return it->second;
    for(preset_t const * inherited: current_preset.inherits)
      if(auto res{find_inherited_argument(key, *inherited)}; res)
        return res;
    return {};
    }

  [[nodiscard]]
  auto expand_value_definition(
    ctx_t & ctx,
    std::string_view preset_file_name,
    std::string_view key,
    glz::json_t const & var,
    preset_t const * current_preset = nullptr
  ) noexcept -> expected_ec<std::variant<std::string, double, bool>>
    {
    if(var.is_string())
      {
      std::string value{var.get_string()};
      do
        {
        auto varref{find_var_placeholder(value)};
        if(not varref) [[unlikely]]
          return unexpected{varref.error()};

        std::optional<variable_info> info{std::move(*varref)};
        if(not info) [[likely]]
          break;

        auto it{ctx.variables.find(info->name)};
        std::optional<std::string> replacment;
        if(it == ctx.variables.end()) [[unlikely]]
          {
          // find in inherited arguments
          if(current_preset != nullptr)
            {
            if(auto arg{find_inherited_argument(info->name, *current_preset)}; arg)
              replacment = to_string(arg->value);
            }
          if(not replacment) [[unlikely]]
            return make_unexpected_error(
              undefined_variable_reference,
              "referenced variable `{}` from `{}` while processing file {} is undefiend",
              info->name,
              value,
              preset_file_name
            );
          }
        else
          replacment = to_string(it->second.value);
        value.replace(info->pos, info->name.size() + 3, std::move(*replacment));
        } while(true);

      return value;
      }
    else if(var.is_boolean())
      return var.get_boolean();
    else if(var.is_number())
      return var.get_number();
    else [[unlikely]]
      return make_unexpected_error(invalid_type_expected_value_definition);
    }

  [[nodiscard]]
  auto process_preset_file(ctx_t & ctx, std::string_view preset_file_name, raw_preset_file_t const & preset)
    -> expected_ec<void>
    {
    // 1. variables with unfolding
    if(preset.variables.is_object())
      {
      glz::json_t::object_t const & variables{preset.variables.get_object()};
      for(auto const & [key, var]: variables)
        {
        if(ctx.variables.contains(key)) [[unlikely]]
          return make_unexpected_error(variable_redeclaration, "variable {} redeclared", key);

        auto res{expand_value_definition(ctx, preset_file_name, key, var)};
        if(not res) [[unlikely]]
          return unexpected{res.error()};
        ctx.variables.emplace(key, std::move(*res));
        }
      }
    else if(!preset.variables.is_null()) [[unlikely]]
      return make_unexpected_error(invalid_type_expected_variable_key_value_pairs);

    // 2. deep into inclusions
    for(std::string const & include_name: preset.includes)
      {
      ctx_t::include_map::iterator it;
        {
        std::filesystem::path include_path{ctx.base_directory / include_name};
        if(not std::filesystem::exists(include_path)) [[unlikely]]
          return make_unexpected_error(
            cannot_load_file, "included file `{}` from `{}` does not exists ", include_path.string(), preset_file_name
          );

        std::string file_name{include_path.string()};
        if(ctx.include.contains(file_name)) [[unlikely]]
          return make_unexpected_error(
            circular_inclusion, "included file `{}` from file `{}` was already included ", file_name, preset_file_name
          );

        raw_preset_file_t included_preset{};
        glz::error_ctx res{glz::read_file_json(included_preset, file_name, std::string{})};
        if(res) [[unlikely]]
          return make_unexpected_error(res.ec, "error parsing file `{}` at char {}", preset_file_name, res.location);

        it = ctx.include.emplace(std::move(file_name), std::move(included_preset)).first;
        }
        {
        raw_preset_file_t const & included_preset{it->second};
        std::string_view preset_file_name{it->first};
        if(auto res{process_preset_file(ctx, preset_file_name, included_preset)}; not res) [[unlikely]]
          return unexpected{res.error()};
        }
      }
    // .3 on return process presets
    for(raw_preset_file_t::raw_preset_t const & rp: preset.presets)
      {
      if(auto itp{ctx.presets.find(rp.name)}; ctx.presets.end() != itp) [[unlikely]]
        return make_unexpected_error(preset_redefinition, "preset `{}` is already defined", rp.name);
      std::unique_ptr<preset_t> preset{std::make_unique<preset_t>()};
      preset->name = rp.name;
      preset->hidden = rp.hidden;
      preset->description = rp.description;

      // process inherits
      for(std::string const & inherited: rp.inherits)
        {
        auto itinh{ctx.presets.find(inherited)};
        if(itinh == ctx.presets.end()) [[unlikely]]
          return make_unexpected_error(
            undefined_inherited_preset, "inherited preset `{}` in `{}` is undefined", inherited, rp.name
          );
        preset->inherits.emplace_back(itinh->second.get());
        }
      // process current arguments and unfold based on variables and inherits
      glz::json_t::object_t const & variables{rp.arguments.get_object()};
      for(auto const & [key, var]: variables)
        {
        if(ctx.variables.contains(key)) [[unlikely]]
          return make_unexpected_error(
            variable_redeclaration, "preset argument key {} is redeclared, previously used as variable", key
          );

        auto res{expand_value_definition(ctx, preset_file_name, key, var, preset.get())};
        if(not res)
          return unexpected{res.error()};
        preset->arguments.emplace(key, *res);
        }
      ctx.presets.emplace(rp.name, std::move(preset));
      }
    return {};
    }

  void expand_preset(preset_t const & preset, presets::argument_list & result) noexcept
    {
    ranges::transform(
      preset.arguments,
      std::back_inserter(result),
      [](auto const & arg) -> config_argument { return config_argument{arg.first, arg.second.value}; }
    );
    for(preset_t const * subpreset: preset.inherits)
      expand_preset(*subpreset, result);
    }
  }  // namespace

auto read_presets(std::string_view preset_file_path) noexcept -> expected_ec<presets>
  {
  if(not std::filesystem::exists(preset_file_path)) [[unlikely]]
    return make_unexpected_error(cannot_load_file);

  ctx_t ctx;
  ctx_t::include_map::iterator it;
    // read main file
    {
    raw_preset_file_t main_presets{};
    ctx.base_directory = preset_file_path;
    std::string file_name{ctx.base_directory.filename()};
    ctx.base_directory.remove_filename();

    glz::error_ctx res{glz::read_file_json(main_presets, preset_file_path, std::string{})};
    if(res) [[unlikely]]
      return make_unexpected_error(res.ec, "error parsing file `{}` at char {}", file_name, res.location);
    if(main_presets.version != expected_version) [[unlikely]]
      return make_unexpected_error(invalid_preset_version);
    it = ctx.include.emplace(std::move(file_name), std::move(main_presets)).first;
    }
    {
    raw_preset_file_t const & main_file{it->second};
    std::string_view preset_file_name{it->first};
    if(auto res{process_preset_file(ctx, preset_file_name, main_file)}; not res) [[unlikely]]
      return unexpected{res.error()};
    }

  auto data{std::make_unique<presets::data>()};
  ranges::transform(
    ctx.variables,
    std::back_inserter(data->arguments),
    [](auto const & kv) -> config_argument { return config_argument{kv.first, kv.second.value}; }
  );
  data->presets = std::move(ctx.presets);
  return presets{std::move(data)};
  }

presets::presets(std::unique_ptr<data> && data) noexcept : data_{std::move(data)} {}

presets::presets(presets && rh) noexcept : data_{std::move(rh.data_)} {}

auto presets::operator=(presets && rh) noexcept -> presets &
  {
  data_ = std::move(rh.data_);
  return *this;
  }

presets::~presets() noexcept {}

auto presets::variables() const noexcept -> std::span<config_argument const> { return data_->arguments; }

auto presets::preset(std::string_view preset_name) const noexcept -> expected_ec<argument_list>
  {
  auto it{data_->presets.find(preset_name)};
  if(data_->presets.end() == it) [[unlikely]]
    return make_unexpected_error(invalid_preset_name, "preset {} is not defiend", preset_name);

  preset_t const & preset{*it->second};
  presets::argument_list result;
  expand_preset(preset, result);
  return result;
  }

auto presets::list_presets() const noexcept -> std::vector<preset_info>
  {
  std::vector<preset_info> result;
  ranges::copy(
    data_->presets | views::filter([](auto const & kv) noexcept { return not kv.second->hidden; })
      | views::transform(
        [](auto const & kv) noexcept -> preset_info
        { return preset_info{.name = kv.first, .description = kv.second->description}; }
      ),
    std::back_inserter(result)
  );
  return result;
  }
  }  // namespace presets::inline v2
