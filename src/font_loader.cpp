#include "font_loader.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

FontLoader::FontLoader() 
    : current_face(nullptr), current_size(12), is_initialized(false) {
    FT_Error error = FT_Init_FreeType(&ft_library);
    if (error) {
        std::cerr << "Failed to initialize FreeType library (error code: " 
                  << error << ")" << std::endl;
        return;
    }
    is_initialized = true;
}

FontLoader::~FontLoader() {
    unload_font();
    if (is_initialized) {
        FT_Done_FreeType(ft_library);
    }
}

bool FontLoader::is_valid_font_file(const std::string& font_path) const {
    if (!fs::exists(font_path)) {
        std::cerr << "Font file not found: " << font_path << std::endl;
        return false;
    }

    std::string extension = fs::path(font_path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension != ".ttf" && extension != ".otf") {
        std::cerr << "Invalid font file extension: " << extension 
                  << " (only .ttf and .otf are supported)" << std::endl;
        return false;
    }

    return true;
}

bool FontLoader::load_font(const std::string& font_path, int size) {
    if (!is_initialized) {
        std::cerr << "FreeType library is not initialized" << std::endl;
        return false;
    }

    if (!is_valid_font_file(font_path)) {
        return false;
    }

    // Unload any previously loaded font
    unload_font();

    // Load the new font face
    FT_Error error = FT_New_Face(ft_library, font_path.c_str(), 0, &current_face);
    if (error) {
        std::cerr << "Failed to load font from: " << font_path 
                  << " (error code: " << error << ")" << std::endl;
        current_face = nullptr;
        return false;
    }

    // Set the font size (width and height in 1/64th of a pixel)
    current_size = size;
    error = FT_Set_Pixel_Sizes(current_face, 0, size);
    if (error) {
        std::cerr << "Failed to set font size: " << size 
                  << " (error code: " << error << ")" << std::endl;
        FT_Done_Face(current_face);
        current_face = nullptr;
        return false;
    }

    current_path = font_path;
    std::cout << "Font loaded successfully: " << font_path 
              << " (size: " << size << "px)" << std::endl;
    return true;
}

FT_Face FontLoader::get_face() const {
    return current_face;
}

int FontLoader::get_font_size() const {
    return current_size;
}

std::string FontLoader::get_font_path() const {
    return current_path;
}

void FontLoader::unload_font() {
    if (current_face != nullptr) {
        FT_Done_Face(current_face);
        current_face = nullptr;
        current_path.clear();
        current_size = 12;
        std::cout << "Font unloaded" << std::endl;
    }
}
