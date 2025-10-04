# External Dependencies

This directory contains external dependencies that are bundled with the ASFMLogger project for self-contained distribution.

## Directory Structure

```
external/
├── nlohmann/           # JSON parsing library
│   └── json.hpp       # Single header file for JSON support
└── README.md          # This file
```

## Dependencies Included

### nlohmann/json
- **Version**: 3.11.2 (or compatible)
- **Purpose**: JSON configuration parsing and serialization
- **License**: MIT License
- **Source**: https://github.com/nlohmann/json

## Usage

These dependencies are automatically detected and used by the CMake build system when:

1. **Local Dependencies Mode** (recommended):
   ```bash
   cmake .. -DASFMLOGGER_USE_LOCAL_DEPS=ON
   ```

2. **Automatic Detection**:
   ```bash
   cmake ..  # Automatically finds local dependencies if vcpkg not available
   ```

## Adding New Dependencies

When adding new external dependencies:

1. Create a subdirectory under `external/`
2. Add the necessary header and/or library files
3. Update `CMakeLists.txt` to detect and use the local copy
4. Update this README with dependency information
5. Update `DEPENDENCIES.md` with setup instructions

## Dependency Information

### nlohmann/json
- **Files**: `json.hpp` (single header)
- **Features Enabled**:
  - JSON parsing and serialization
  - Configuration file support
  - Enhanced toolbox functionality

- **Build Integration**:
  ```cmake
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann/json.hpp")
      target_include_directories(ASFMLogger PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external")
      target_compile_definitions(ASFMLogger PRIVATE -DASFMLOGGER_JSON_SUPPORT)
  endif()
  ```

## Maintenance

- Keep dependencies up to date with security patches
- Test compatibility when updating versions
- Document any breaking changes in DEPENDENCIES.md
- Ensure licenses are compatible with project license

## Licenses

All included dependencies must maintain their original licenses:
- **nlohmann/json**: MIT License - Compatible with most projects
- **Future dependencies**: Verify license compatibility before inclusion