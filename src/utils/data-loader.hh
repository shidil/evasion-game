#ifndef RESOURCES_LOADER_H
#define RESOURCES_LOADER_H
#include <raylib.h>

#include <cstring>
#include <string>

#define ASSET_BASE_DIR "resources/";

namespace evs {

inline std::string get_real_path(std::string path) {
  std::string out = ASSET_BASE_DIR;
  return out.append(path);
}

inline std::string load_text_file(std::string file) {
  std::string contents = LoadFileText(get_real_path(file).data());
  return contents;
}

inline Sound load_sound(std::string file) {
  return LoadSound(get_real_path(file).data());
}

inline Music load_music(std::string file) {
  return LoadMusicStream(get_real_path(file).data());
}

inline Texture2D load_texture(std::string file) {
  return LoadTexture(get_real_path(file).data());
}

inline Shader load_fragment_shader(std::string name) {
  auto path = get_real_path(TextFormat("shaders/glsl%i/%s.fs", GLSL_VERSION, name.data()));
  return LoadShader(0, path.data());
}

inline Font load_sdf_font(std::string font_name, int base_size = 16,
                          int char_count = 95) {
  // Loading file to memory
  unsigned int fileSize = 0;
  unsigned char* fileData = LoadFileData(get_real_path(font_name).data(), &fileSize);

  // SDF font generation from TTF font
  Font fontSDF = {0};
  fontSDF.baseSize = base_size;
  fontSDF.charsCount = char_count;

  // Parameters > font size: 16, no chars array provided (0), chars count: 0
  // (defaults to 95)
  fontSDF.chars = LoadFontData(fileData, fileSize, base_size, 0, 0, FONT_SDF);
  // Parameters > chars count: 95, font size: 16, chars padding in image: 0 px,
  // pack method: 1 (Skyline algorithm)
  Image atlas =
      GenImageFontAtlas(fontSDF.chars, &fontSDF.recs, char_count, base_size, 0, 1);
  fontSDF.texture = LoadTextureFromImage(atlas);
  UnloadImage(atlas);

  UnloadFileData(fileData);  // Free memory from loaded file

  return fontSDF;
}

inline Font load_font(std::string font_name, int base_size = 16, int char_count = 95) {
  // Loading file to memory
  unsigned int fileSize = 0;
  unsigned char* fileData = LoadFileData(get_real_path(font_name).data(), &fileSize);

  // SDF font generation from TTF font
  Font font = {0};
  font.baseSize = base_size;
  font.charsCount = char_count;

  // Parameters > font size: 16, no chars array provided (0), chars count: 0
  // (defaults to 95)
  font.chars = LoadFontData(fileData, fileSize, base_size, 0, char_count, FONT_DEFAULT);
  // Parameters > chars count: 95, font size: 16, chars padding in image: 0 px,
  // pack method: 1 (Skyline algorithm)
  Image atlas = GenImageFontAtlas(font.chars, &font.recs, char_count, base_size, 4, 0);
  font.texture = LoadTextureFromImage(atlas);
  UnloadImage(atlas);

  UnloadFileData(fileData);  // Free memory from loaded file

  return font;
}
}  // namespace evs

#endif  // RESOURCES_LOADER_H
