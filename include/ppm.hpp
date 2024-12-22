#ifndef RT_PPM_HPP
#define RT_PPM_HPP

#include <glm/vec3.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <string>

auto linear_to_gamma(float linear) -> float {
  return std::pow(linear, 1.0f / 2.2f);
}

class PPM final {
public:
  PPM(const std::string& name, unsigned width, unsigned height) 
    : m_file{name, std::ios::binary}
    , m_width{width}
    , m_height{height} 
  {
    if (!m_file) {
      std::cerr << "[ERROR] Failed to open " << name << "\n";
      return;
    }

    m_file << "P6\n" << width << " " << height << "\n255\n"; 
  }

  auto write_color(const glm::vec3& color) {
    auto r = glm::clamp(linear_to_gamma(color.r), 0.0f, 0.999f);
    auto g = glm::clamp(linear_to_gamma(color.g), 0.0f, 0.999f);
    auto b = glm::clamp(linear_to_gamma(color.b), 0.0f, 0.999f);

    m_file << static_cast<char>(r * 256.0f)
           << static_cast<char>(g * 256.0f)
           << static_cast<char>(b * 256.0f);
  }

  auto width() const -> unsigned { return m_width; }
  auto height() const -> unsigned { return m_height; }

private:
  std::ofstream m_file{};
  unsigned m_width{};
  unsigned m_height{};
};

#endif