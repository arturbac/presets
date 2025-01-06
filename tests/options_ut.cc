#include <presets/options.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/ut.hpp>
#pragma clang diagnostic pop

using namespace presets::po;

auto main(int argc, char ** argv, char ** env) -> int
  {
#pragma clang unsafe_buffer_usage begin
  std::span<char const * const> args{argv, std::size_t(argc)};
  std::span<char const * const> envs{env, std::size_t(argc)};
#pragma clang unsafe_buffer_usage end

  using namespace boost::ut;
  using namespace std::string_literals;

  suite value_string = []
  {
    "string parsing"_test = []
    {
      should("parse empty string") = []
      {
        std::string value;
        value_semantic_t parser{value_t<std::string>{value}};
        parser.parse("");
        expect(value.empty());
      };

      should("parse non-empty string") = []
      {
        std::string value;
        value_semantic_t parser{value_t<std::string>{value}};
        parser.parse("hello world");
        expect(eq(value, "hello world"s));
      };

      should("handle unicode strings") = []
      {
        std::string value;
        value_semantic_t parser{value_t<std::string>{value}};
        parser.parse("hello 🌍");
        expect(eq(value, "hello 🌍"s));
      };
    };
  };

  suite value_int = []
  {
    "integer parsing"_test = []
    {
      should("parse zero") = []
      {
        int value{};
        value_semantic_t parser{value_t<int>{value}};
        parser.parse("0");
        expect(eq(value, 0));
      };

      should("parse positive number") = []
      {
        int value{};
        value_semantic_t parser{value_t<int>{value}};
        parser.parse("42");
        expect(eq(value, 42));
      };

      should("parse negative number") = []
      {
        int value{};
        value_semantic_t parser{value_t<int>{value}};
        parser.parse("-42");
        expect(eq(value, -42));
      };

      should("throw on invalid input") = []
      {
        int value{};
        value_semantic_t parser{value_t<int>{value}};
        expect(throws([&] { parser.parse("not a number"); }));
      };
    };
  };

  suite value_double = []
  {
    "double parsing"_test = []
    {
      should("parse zero") = []
      {
        double value{};
        value_semantic_t parser{value_t<double>{value}};
        parser.parse("0.0");
        expect(eq(value, 0.0));
      };

      should("parse positive number") = []
      {
        double value{};
        value_semantic_t parser{value_t<double>{value}};
        parser.parse("3.14");
        expect(eq(value, 3.14));
      };

      should("parse negative number") = []
      {
        double value{};
        value_semantic_t parser{value_t<double>{value}};
        parser.parse("-3.14");
        expect(eq(value, -3.14));
      };

      should("parse scientific notation") = []
      {
        double value{};
        value_semantic_t parser{value_t<double>{value}};
        parser.parse("1.23e-4");
        expect(eq(value, 1.23e-4));
      };

      should("throw on invalid input") = []
      {
        double value{};
        value_semantic_t parser{value_t<double>{value}};
        expect(throws([&] { parser.parse("not a number"); }));
      };
    };
  };

  suite value_vector = []
  {
    "vector parsing"_test = []
    {
      should("parse single int") = []
      {
        std::vector<int> values;
        value_semantic_t parser{value_t<std::vector<int>>{values}};
        parser.parse("42");
        expect(eq(values.size(), 1u));
        expect(eq(values[0], 42));
      };

      should("parse single double") = []
      {
        std::vector<double> values;
        value_semantic_t parser{value_t<std::vector<double>>{values}};
        parser.parse("3.14");
        expect(eq(values.size(), 1u));
        expect(eq(values[0], 3.14));
      };

      should("parse single string") = []
      {
        std::vector<std::string> values;
        value_semantic_t parser{value_t<std::vector<std::string>>{values}};
        parser.parse("hello");
        expect(eq(values.size(), 1u));
        expect(eq(values[0], "hello"s));
      };

      should("build vector incrementally") = []
      {
        std::vector<int> values;
        value_semantic_t parser{value_t<std::vector<int>>{values}};
        parser.parse("1");
        parser.parse("2");
        parser.parse("3");
        expect(eq(values.size(), 3u));
        expect(eq(values[0], 1));
        expect(eq(values[1], 2));
        expect(eq(values[2], 3));
      };
    };
  };
  }
