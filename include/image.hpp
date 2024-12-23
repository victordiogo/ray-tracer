#ifndef RT_IMAGE_HPP
#define RT_IMAGE_HPP

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/vec3.hpp>

#include <string>
#include <vector>
#include <optional>

struct Image {
  std::vector<glm::vec3> pixels{};
  unsigned width{};
  unsigned height{};
};

auto load_image(const std::string& filepath) -> std::optional<Image> {
  auto image = Image{};
  stbi_set_flip_vertically_on_load(true);
  auto fdata = stbi_loadf(filepath.c_str(), reinterpret_cast<int*>(&image.width), reinterpret_cast<int*>(&image.height), nullptr, 3);
  if (fdata == nullptr) {
    return {};
  }

  image.pixels.reserve(image.width * image.height);
  for (auto i = 0u; i < image.width * image.height; ++i) {
    image.pixels.push_back(glm::vec3{fdata[i * 3], fdata[i * 3 + 1], fdata[i * 3 + 2]});
  }

  stbi_image_free(fdata);

  return image; 
}

#endif