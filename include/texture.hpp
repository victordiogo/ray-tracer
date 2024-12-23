#ifndef RT_TEXTURE_HPP
#define RT_TEXTURE_HPP

#include "image.hpp"
#include "perlin.hpp"

#include <glm/vec3.hpp>

#include <memory>
#include <cmath>
#include <algorithm>

class Texture {
public:
  virtual ~Texture() = default;

  virtual auto value(float u, float v, const glm::vec3& point) const -> glm::vec3 = 0;
};

class SolidColor : public Texture {
public:
  explicit SolidColor(const glm::vec3& color) : m_color{color} {}

  auto value(float, float, const glm::vec3&) const -> glm::vec3 override {
    return m_color;
  }

private:
  glm::vec3 m_color{};
};

class CheckerTexture : public Texture {
public:
  CheckerTexture(float scale, std::shared_ptr<Texture> even, std::shared_ptr<Texture> odd) 
    : m_inv_scale{1.0f / scale}
    , m_even{std::move(even)}
    , m_odd{std::move(odd)} {}

  CheckerTexture(float scale, const glm::vec3& even, const glm::vec3& odd) 
    : CheckerTexture(scale, std::make_shared<SolidColor>(even), std::make_shared<SolidColor>(odd)) {}

  auto value(float u, float v, const glm::vec3& point) const -> glm::vec3 override {
    auto x = static_cast<int>(std::floor(m_inv_scale * point.x));
    auto y = static_cast<int>(std::floor(m_inv_scale * point.y));
    auto z = static_cast<int>(std::floor(m_inv_scale * point.z));

    auto is_even = (x + y + z) % 2 == 0;

    return is_even ? m_even->value(u, v, point) : m_odd->value(u, v, point);
  }

private:
  float m_inv_scale{};
  std::shared_ptr<Texture> m_even{};
  std::shared_ptr<Texture> m_odd{};
};

class ImageTexture : public Texture {
public:
  ImageTexture(const Image& image) : m_image{image} {}

  auto value(float u, float v, const glm::vec3&) const -> glm::vec3 override {
    auto widthf = static_cast<float>(m_image.width);
    auto heightf = static_cast<float>(m_image.height);
    auto i = static_cast<unsigned>(u * widthf);
    auto j = static_cast<unsigned>(v * heightf);

    i = std::clamp(i, 0u, m_image.width - 1u);
    j = std::clamp(j, 0u, m_image.height - 1u);

    return m_image.pixels[i + m_image.width * j];
  }

private:
  Image m_image{};
};

class NoiseTexture : public Texture {
public:
  NoiseTexture(float scale = 1.0f) : m_perlin{}, m_scale{scale} {}

  auto value(float, float, const glm::vec3& point) const -> glm::vec3 override {
    return glm::vec3{0.5f} * (1.0f + std::sin(m_scale * point.z + 10.0f * m_perlin.turb(m_scale * point)));
  }

private:
  Perlin m_perlin{};
  float m_scale{};
};

#endif