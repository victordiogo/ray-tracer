#ifndef RT_MATERIAL_HPP
#define RT_MATERIAL_HPP

#include "ray.hpp"
#include "hittable.hpp"
#include "random.hpp"
#include "texture.hpp"
#include "random.hpp"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <optional>
#include <cmath>

constexpr auto g_bias = 0.0005f;

class Material {
public:
  virtual ~Material() = default;

  struct ScatterData {
    glm::vec3 attenuation{};
    Ray scattered{};
    glm::vec3 emission{};
  };

  virtual auto scatter(const Ray& ray, const HitRecord& hit_record) const -> std::optional<ScatterData> = 0;
};

auto near_zero(const glm::vec3& vec) -> bool {
  const float s = 1e-6f;
  return (std::abs(vec.x) < s) && (std::abs(vec.y) < s) && (std::abs(vec.z) < s);
}

class Lambertian : public Material {
public:
  Lambertian(const glm::vec3& albedo)
    : m_texture{std::make_shared<SolidColor>(albedo)}
  {}

  Lambertian(std::shared_ptr<Texture> texture)
    : m_texture{texture}
  {}  

  auto scatter(const Ray& ray, const HitRecord& hit_record) const -> std::optional<ScatterData> override {
    auto scatter_direction = hit_record.normal + prng::get_unit_vector();
    if (near_zero(scatter_direction)) {
      scatter_direction = hit_record.normal;
    }

    auto point = hit_record.point + hit_record.normal * g_bias;

    auto attenuation = m_texture->value(hit_record.texture_coords.x, hit_record.texture_coords.y, hit_record.point);
    return ScatterData{attenuation, Ray{point, scatter_direction, ray.time()}};
  }

private:
  std::shared_ptr<Texture> m_texture{};
};
 
class Metal : public Material {
public:
  Metal(const glm::vec3& albedo, float fuzz) 
  : m_albedo{albedo}
  , m_fuzz{ fuzz < 1.0f ? fuzz : 1.0f } {}

  auto scatter(const Ray& ray, const HitRecord& hit_record) const -> std::optional<ScatterData> override {
    auto reflected = glm::reflect(ray.direction(), hit_record.normal);
    reflected = glm::normalize(reflected) + (m_fuzz * prng::get_unit_vector());
    if (glm::dot(reflected, hit_record.normal) <= 0.0f) {
      return {};
    }

    auto point = hit_record.point + hit_record.normal * g_bias;

    return ScatterData{m_albedo, Ray{point, reflected, ray.time()}};
  }

private:
  glm::vec3 m_albedo{};
  float m_fuzz{};
};

auto reflectance(float cosine, float refraction_index) -> float {
  auto r0 = (1.0f - refraction_index) / (1.0f + refraction_index);
  r0 = r0 * r0;
  return r0 + (1.0f - r0) * std::pow(1.0f - cosine, 5.0f);
}

class Dielectric : public Material {
public:
  Dielectric(float refraction_index)
    : m_refraction_index{refraction_index}
  {}

  auto scatter(const Ray& ray, const HitRecord& hit_record) const -> std::optional<ScatterData> override {
    auto direction = glm::normalize(ray.direction());
    auto ri = hit_record.front_face ? (1.0f / m_refraction_index) : m_refraction_index;

    auto cos_theta = std::fmin(glm::dot(-direction, hit_record.normal), 1.0f);
    auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

    auto point = hit_record.point;

    auto scattered_direction = glm::vec3{};
    if (ri * sin_theta > 1.0f || reflectance(cos_theta, ri) > prng::get_real(0.0f, 1.0f)) {
      scattered_direction = glm::reflect(direction, hit_record.normal);
      point += hit_record.normal * g_bias;
    } else {
      scattered_direction = glm::refract(direction, hit_record.normal, ri);
      point -= hit_record.normal * g_bias;
    }

    return ScatterData{glm::vec3{1.0f}, Ray{point, scattered_direction, ray.time()}};
  }

private:
  float m_refraction_index{};
};

class DiffuseLight : public Material {
public:
  DiffuseLight(std::shared_ptr<Texture> texture)
    : m_texture{texture}
  {}

  DiffuseLight(const glm::vec3& color)
    : m_texture{std::make_shared<SolidColor>(color)}
  {}

  auto scatter(const Ray&, const HitRecord& hit_record) const -> std::optional<ScatterData> override {
    auto uv = hit_record.texture_coords;
    auto emission = m_texture->value(uv.x, uv.y, hit_record.point);
    return ScatterData{glm::vec3{}, Ray{}, emission};
  }

private:
  std::shared_ptr<Texture> m_texture{};
};

class Isotropic : public Material {
public:
  Isotropic(std::shared_ptr<Texture> texture)
    : m_texture{texture}
  {}

  Isotropic(const glm::vec3& color)
    : m_texture{std::make_shared<SolidColor>(color)}
  {}

  auto scatter(const Ray& ray, const HitRecord& hit_record) const -> std::optional<ScatterData> override {
    auto attenuation = m_texture->value(hit_record.texture_coords.x, hit_record.texture_coords.y, hit_record.point);
    return ScatterData{attenuation, Ray{hit_record.point, prng::get_unit_vector(), ray.time()}};
  }

private:
  std::shared_ptr<Texture> m_texture{};
};

#endif