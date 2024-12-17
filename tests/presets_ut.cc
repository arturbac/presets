// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: BSL-1.0
// SPDX-PackageHomePage: https://github.com/arturbac/presets
#include <presets/presets.hpp>
#include <span>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/ut.hpp>
#pragma clang diagnostic pop
namespace ut = boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;
using namespace boost::ut::literals;
using namespace boost::ut::bdd;
using namespace std::string_view_literals;
using ut::approx;
using ut::eq;
using ut::expect;
using ut::fatal;
using ut::ge;
using ut::gt;
using ut::le;
using ut::lt;
using ut::neq;
using std::operator""sv;

static auto verify_argument(std::span<presets::config_argument const> args, presets::config_argument const expected_arg)
  {
  auto it{std::ranges::find(args, expected_arg.name, [](auto const & arg) -> std::string const & { return arg.name; })};
  when("contains argument");
  expect(it != args.end());
  then("has proper value");
  expect(it->value == expected_arg.value);
  }

auto main(int /*argc*/, char ** /*argv*/, char ** /*env*/) -> int
  {
  //   std::span<char const * const> args{ argv, std::size_t(argc) };
  //   std::span<char const * const> envs{ env, std::size_t(argc) };
  //
  "mutliple references in variables"_test = []
  {
    given("load var pereset") = []
    {
      auto presetres{presets::read_presets("presets/var_preset_top.json")};
      expect(presetres.has_value() >> ut::fatal) << "expected valid preset result";
      then("should be valid result with transformed arguments") = [&]
      {
        presets::presets value{std::move(*presetres)};
        auto variables{value.variables()};
        verify_argument(variables, presets::config_argument{.name = "variable_x", .value = true});
        verify_argument(
          variables,
          presets::config_argument{
            .name = "refering_var2_varn", .value = "refers `43.000000` from top and `1` from prev include"
          }
        );

        verify_argument(variables, presets::config_argument{.name = "variable_n", .value = true});
        verify_argument(variables, presets::config_argument{.name = "variable_3", .value = 67.15});
        verify_argument(
          variables,
          presets::config_argument{
            .name = "ref_refering_var", .value = "ref refering_var=`refers to variable_2=`43.000000`` from prev include"
          }
        );
        verify_argument(
          variables, presets::config_argument{.name = "refering_var", .value = "refers to variable_2=`43.000000`"}
        );
        verify_argument(variables, presets::config_argument{.name = "variable_2", .value = 43.});
        verify_argument(variables, presets::config_argument{.name = "variable_x", .value = true});
        verify_argument(variables, presets::config_argument{.name = "variable_1", .value = "22r3"});
      };
    };

    given("load invalid var pereset") = []
    {
      auto presetres{presets::read_presets("presets/var_invalid_ref_top.json")};
      then("should be invalid result");
      expect(not presetres.has_value() >> ut::fatal) << "expected error";
    };
    given("load not exisiting pereset") = []
    {
      auto presetres{presets::read_presets("presets/not_existing.json")};
      then("should be invalid result");
      expect(not presetres.has_value() >> ut::fatal) << "expected error";
    };
  };

  "preset definitions"_test = []
  {
    auto load_res{presets::read_presets("presets/cfg-presets.json")};
    expect(load_res.has_value() >> ut::fatal) << "expected valid preset result";
    presets::presets value{std::move(*load_res)};
    then("should be valid result with valid presets") = [&]
    {
      auto preset_res{value.preset("db-option2")};
      expect(preset_res.has_value() >> ut::fatal) << "expected valid argument list";
      auto arguments{std::move(*preset_res)};
      verify_argument(arguments, presets::config_argument{.name = "initdb", .value = ""});
      verify_argument(arguments, presets::config_argument{.name = "grid", .value = "grid-value"});
      verify_argument(arguments, presets::config_argument{.name = "db", .value = "grid-value_22r3"});
      verify_argument(arguments, presets::config_argument{.name = "dr", .value = "43.000000.00.00"});
    };
    then("should be valid result with listing presets") = [&]
    {
      std::vector<presets::preset_info> listed{value.list_presets()};
      ut::expect(ut::eq(listed.size(), 2u));
      
    };
  };

  // (void)presets::read_presets("presets_v2.json");
  }
