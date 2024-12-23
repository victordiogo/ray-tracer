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

class Material {
public:
  virtual ~Material() = default;

  struct ScatterData {
    glm::vec3 attenuation{};
    Ray scattered{};
  };

  virtual auto scatter(const Ray& ray, const HitRecord& hit_record) const -> std::optional<ScatterData> = 0;
};

auto near_zero(const glm::vec3& vec) -> bool {
  const float s = 1e-6f;
  return (std::fabs(vec.x) < s) && (std::fabs(vec.y) < s) && (std::fabs(vec.z) < s);
}

auto random_hemisphere_vector(const glm::vec3& normal) -> glm::vec3 {
  auto unit_vector = prng::get_unit_vector();
  return glm::dot(unit_vector, normal) > 0.0f ? unit_vector : -unit_vector;
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

    auto point = hit_record.point + hit_record.normal * 0.0001f;

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

    auto point = hit_record.point + hit_record.normal * 0.0001f;

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

    auto point = hit_record.point - hit_record.normal * 0.0001f;

    auto scattered_direction = glm::vec3{};
    if (ri * sin_theta > 1.0f || reflectance(cos_theta, ri) > prng::get_real(0.0f, 1.0f)) {
      scattered_direction = glm::reflect(direction, hit_record.normal);
    } else {
      scattered_direction = glm::refract(direction, hit_record.normal, ri);
    }

    return ScatterData{glm::vec3{1.0f}, Ray{point, scattered_direction, ray.time()}};
  }

private:
  float m_refraction_index{};
};

#endif