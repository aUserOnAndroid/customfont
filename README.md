 CustomFont

A C++ library for loading and using custom TTF and OTF fonts to replace default system fonts.

## Features

- **TTF/OTF Support**: Load both TrueType and OpenType font files
- **Easy Font Replacement**: Simple API to import and apply custom fonts
- **FreeType Integration**: Built on top of the industry-standard FreeType library
- **Font Information Access**: Query font family, style, size, and glyph information

## Requirements

- C++17 or later
- CMake 3.10+
- FreeType library (development files)

### Install FreeType

**Ubuntu/Debian:**
```bash
sudo apt-get install libfreetype6-dev
```

**macOS (with Homebrew):**
```bash
brew install freetype
```

**Windows (with vcpkg):**
```bash
vcpkg install freetype:x64-windows
```

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

### Basic Example

```cpp
#include "font_loader.h"

int main() {
    FontLoader loader;
    
    // Load a custom font (TTF or OTF)
    if (loader.load_font("path/to/your/font.ttf", 24)) {
        // Font loaded successfully!
        auto face = loader.get_face();
        // Use the font for rendering...
    } else {
        // Handle error
    }
    
    return 0;
}
```

### Command Line Demo

```bash
./customfont_demo path/to/font.ttf [size]

# Example:
./customfont_demo /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf 14
```

## API Reference

### FontLoader Class

#### Constructor
```cpp
FontLoader();
```
Initializes the FreeType library.

#### `bool load_font(const std::string& font_path, int size = 12)`
Loads a TTF or OTF font file.
- **Parameters:**
  - `font_path`: Path to the font file
  - `size`: Font size in pixels (default: 12)
- **Returns:** `true` on success, `false` on failure

#### `FT_Face get_face() const`
Returns the FreeType face object for the loaded font.

#### `int get_font_size() const`
Returns the current font size in pixels.

#### `std::string get_font_path() const`
Returns the path of the currently loaded font file.

#### `void unload_font()`
Releases the currently loaded font and frees resources.

## Supported Font Formats

- **.ttf** - TrueType Font
- **.otf** - OpenType Font

## Error Handling

The library provides detailed error messages for common issues:
- Font file not found
- Invalid file format
- FreeType initialization failures
- Font size setting errors

Check console output for detailed error information.

