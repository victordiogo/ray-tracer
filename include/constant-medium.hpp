#ifndef RT_CONSTANT_MEDIUM_HPP
#define RT_CONSTANT_MEDIUM_HPP

#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

#include <glm/geometric.hpp>

class ConstantMedium : public Hittable {
public:
  ConstantMedium(const std::shared_ptr<Hittable>& boundary, float density, const std::shared_ptr<Texture>& texture)
    : m_bounding_box(boundary), m_density(density), m_material(std::make_shared<Isotropic>(texture)) {}

  ConstantMedium(const std::shared_ptr<Hittable>& boundary, float density, const glm::vec3& color)
    : m_bounding_box(boundary), m_density(density), m_material(std::make_shared<Isotropic>(color)) {}

  auto bounding_box() const -> Aabb override {
    return m_bounding_box->bounding_box();
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    auto hit1 = m_bounding_box->hit(ray, -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
    if (!hit1) {
      return {};
    }

    auto hit2 = m_bounding_box->hit(ray, hit1->distance + 0.0005f, std::numeric_limits<float>::infinity());
    if (!hit2) {
      return {};
    }

    if (hit1->distance < min_distance) {
      hit1->distance = min_distance;
    }

    if (hit2->distance > max_distance) {
      hit2->distance = max_distance;
    }

    if (hit1->distance >= hit2->distance) {
      return {};
    }

    if (hit1->distance < 0.0f) {
      hit1->distance = 0.0f;
    }

    auto distance_inside_boundary = (hit2->distance - hit1->distance) * glm::length(ray.direction());
    auto hit_distance = -(1.0f / m_density) * std::log(prng::get_real(0.0f, 1.0f));

    if (hit_distance > distance_inside_boundary) {
      return {};
    }

    auto distance = hit1->distance + hit_distance / glm::length(ray.direction());
    auto point = ray.at(distance);

    return HitRecord{distance, true, point, glm::vec3{0.0f}, m_material, glm::vec2{}};
  }

private:
  std::shared_ptr<Hittable> m_bounding_box{};
  float m_density{};
  std::shared_ptr<Material> m_material{};
};

#endif