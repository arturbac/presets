# Command Line Argument Preset Handling for any application

A modern C++23 library for managing command line arguments through preset configurations, similar to CMake's preset system. This library allows you to define preset argument configurations in JSON format, with support for inheritance and variable interpolation.
Early in development.

## Features

- Define command line argument presets in JSON, similar to CMake presets
- Inherit and combine argument sets from multiple presets
- Variable substitution with `${variable}` syntax
- Modern C++23 implementation leveraging ranges and views
- Fast JSON parsing with glaze
- Robust error handling using simple_enum's expected type

## Requirements

- C++23 compliant compiler (maybe donwgraded if requested to c++20)
- CMake 3.25 or higher
- [glaze](https://github.com/stephenberry/glaze) v4.1.0 for JSON handling
- [simple_enum](https://github.com/arturbac/simple_enum) v0.8.5 for expected type
- Boost.UT for unit testing


## Example Usage

Here's a practical example using database connection configurations:

### Base Configuration (`base_settings.json`):
```json
{
  "version": 2,
  "variables": {
    "default_port": 5432,
    "timeout": 30,
    "max_pool": 100
  },
  "presets": [
    {
      "name": "base-connection",
      "hidden": true,
      "arguments": {
        "port": "${default_port}",
        "connect_timeout": "${timeout}",
        "pool_size": "${max_pool}",
        "ssl": true
      }
    }
  ]
}
```

### Main Configuration (`db_config.json`):
```json
{
  "version": 2,
  "includes": ["base_settings.json"],
  "variables": {
    "prod_host": "db.production.example.com",
    "dev_host": "db.dev.example.com"
  },
  "presets": [
    {
      "name": "prod-db",
      "description": "Production database connection for ${prod_host}",
      "inherits": ["base-connection"],
      "arguments": {
        "host": "${prod_host}",
        "database": "main_db",
        "user": "prod_user",
        "pool_size": 50,
        "retry_writes": true
      }
    },
    {
      "name": "dev-db",
      "description": "Development database connection for ${dev_host}",
      "inherits": ["base-connection"],
      "arguments": {
        "host": "${dev_host}",
        "database": "dev_db",
        "user": "dev_user",
        "pool_size": 10,
        "log_level": "debug"
      }
    }
  ]
}
```

### Code Example

This exmaple show only presets use and may be comined with Your own command line handling
(such example will shortly come for boost::program_options)

```cpp
#include <presets/presets.hpp>
#include <iostream>

auto main() -> int {
    // Load database configuration presets
    auto preset_result = presets::read_presets("db_config.json");
    if (!preset_result) {
        std::cerr << "Failed to load presets: " 
                  << preset_result.error().info << '\n';
        return 1;
    }
    
    // Get production database configuration
    auto prod_config = preset_result->preset("prod-db");
    if (!prod_config) {
        std::cerr << "Failed to get production config: " 
                  << prod_config.error().info << '\n';
        return 1;
    }
    
    // Use the configuration
    for (auto const& arg : *prod_config) {
        std::visit([&](auto&& value) {
            std::cout << "--" << arg.name << "=" << value << '\n';
        }, arg.value);
    }
    
    return 0;
}
```

The above example demonstrates:
- Variable definitions and interpolation
- Preset inheritance
- Hidden base presets
- Configuration file inclusion
- Error handling

## Key Concepts

- **Presets**: Named collections of command line arguments
- **Inheritance**: Presets can inherit and override arguments from other presets
- **Variables**: Define values that can be reused across different presets
- **Includes**: Import presets and variables from other JSON files

## Features

The preset system supports:
- Three argument value types: string, double, and boolean
- Variable interpolation using `${variable}` syntax
- Multi-level preset inheritance
- External configuration file inclusion
- Hidden presets for creating base configurations
- Validation of preset definitions and references

## Error Handling

The library uses `expected<T, error_info>` from std or if not available from simple_enum for robust error handling, providing detailed information about any failures in loading or processing presets.

## Contributing

Contributions are welcome. Please ensure all tests pass and add new tests for new features using Boost.UT.
