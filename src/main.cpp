#include "font_loader.h"
#include <iostream>

int main(int argc, char* argv[]) {
    FontLoader loader;

    // Example usage
    if (argc < 2) {
        std::cout << "Usage: customfont_demo <path_to_font_file> [font_size]" << std::endl;
        std::cout << "Supported formats: TTF, OTF" << std::endl;
        return 1;
    }

    std::string font_path = argv[1];
    int font_size = 12;

    if (argc >= 3) {
        try {
            font_size = std::stoi(argv[2]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid font size: " << argv[2] << std::endl;
            return 1;
        }
    }

    // Load the font
    if (!loader.load_font(font_path, font_size)) {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    // Get font information
    std::cout << "\nFont Information:" << std::endl;
    std::cout << "  Path: " << loader.get_font_path() << std::endl;
    std::cout << "  Size: " << loader.get_font_size() << "px" << std::endl;

    // You can access the FT_Face object for rendering
    auto face = loader.get_face();
    if (face) {
        std::cout << "  Family: " << face->family_name << std::endl;
        std::cout << "  Style: " << face->style_name << std::endl;
        std::cout << "  Glyphs: " << face->num_glyphs << std::endl;
    }

    return 0;
}
