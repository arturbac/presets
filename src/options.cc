#include <presets/options.hpp>
#include <presets/error_handling.hpp>

namespace presets::inline v2::po
  {
value_semantic_t::value_interface_t::~value_interface_t() noexcept {}

auto value_semantic_t::parse(std::string_view str) -> void
  {
  dynamic_cast<value_interface_t *>(stored_value_.get())->parse(str);
  }

auto value_t<std::string>::parse(std::string_view str) const -> void { *storage_ = std::string{str}; }

auto value_t<double>::parse(std::string_view str) const -> void { *storage_ = std::stod(std::string{str}); }
using enum error_code;

auto process_args(std::span<char const * const> args) noexcept -> expected_ec<std::vector<option_t>>
  {
  using namespace std::string_view_literals;
  if(args.empty())
    return std::vector<option_t>{};

  std::vector<option_t> options;

  for(auto it = args.begin() + 1; it != args.end(); ++it)
    {
    std::string_view current{*it};

    if(current.starts_with("--"sv))
      {
      // Handle long option
      std::string_view key_value{current.substr(2)};
      if(auto pos = key_value.find('='); pos != std::string_view::npos)
        {
        options.emplace_back(
          option_t{.key = std::string{key_value.substr(0, pos)}, .value = std::string{key_value.substr(pos + 1)}}
        );
        }
      else
        {
        auto next = std::next(it);
        options.emplace_back(
          option_t{
            .key = std::string{key_value},
            .value = next != args.end() && detail::is_value(*next) ? std::string{*next} : std::string{}
          }
        );
        if(next != args.end() && detail::is_value(*next))
          ++it;
        }
      }
    else if(current.starts_with("-"sv) or current.starts_with("\\"sv))
      {
      // Handle short option
      auto key = current.substr(1);
      auto next = std::next(it);
      options.emplace_back(
        option_t{
          .key = std::string{key},
          .value = next != args.end() && detail::is_value(*next) ? std::string{*next} : std::string{}
        }
      );
      if(next != args.end() && detail::is_value(*next))
        ++it;
      }
    else
      return make_unexpected_error(invalid_option, "argument {} should start with `-` or `--`", current);
    }

  return options;
  }

auto option_constructor::operator()(std::string_view option, std::string_view description) -> option_constructor
  {
  parent_->options_.emplace_back(std::string{option}, std::string{description});
  return option_constructor{parent_};
  }
  }  // namespace presets::inline v2::po
