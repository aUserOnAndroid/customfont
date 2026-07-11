#ifndef FONT_LOADER_H
#define FONT_LOADER_H

#include <string>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H

class FontLoader {
public:
    FontLoader();
    ~FontLoader();

    /**
     * Load a TTF or OTF font file
     * @param font_path Path to the TTF or OTF file
     * @param size Font size in pixels
     * @return true if loading was successful, false otherwise
     */
    bool load_font(const std::string& font_path, int size = 12);

    /**
     * Get the loaded FreeType face object
     * @return Pointer to the FT_Face object, or nullptr if no font is loaded
     */
    FT_Face get_face() const;

    /**
     * Get the current font size
     * @return Font size in pixels
     */
    int get_font_size() const;

    /**
     * Get the loaded font path
     * @return Path to the currently loaded font file
     */
    std::string get_font_path() const;

    /**
     * Release the currently loaded font
     */
    void unload_font();

private:
    FT_Library ft_library;
    FT_Face current_face;
    int current_size;
    std::string current_path;
    bool is_initialized;

    /**
     * Validate if a file is a valid TTF or OTF file
     * @param font_path Path to check
     * @return true if valid font file, false otherwise
     */
    bool is_valid_font_file(const std::string& font_path) const;
};

#endif // FONT_LOADER_H
