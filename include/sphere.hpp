#ifndef RT_SPHERE_HPP
#define RT_SPHERE_HPP

#include "hittable.hpp"
#include "ray.hpp"

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <stdexcept>
#include <optional>

class Sphere : public Hittable {
public:
  Sphere(const glm::vec3& center1, const glm::vec3& center2, float radius, std::shared_ptr<Material> material)
    : m_center{center1, center2 - center1}
    , m_radius{radius}
    , m_material{material}
  {
    if (m_radius <= 0.0f) {
      throw std::invalid_argument{"Sphere radius must be positive"};
    }

    auto radius_vec = glm::vec3{m_radius};
    auto aabb1 = Aabb{center1 - radius_vec, center1 + radius_vec};
    auto aabb2 = Aabb{center2 - radius_vec, center2 + radius_vec};
    m_bounding_box = Aabb{aabb1, aabb2};
  }

  Sphere(const glm::vec3& center, float radius, std::shared_ptr<Material> material)
    : Sphere{center, center, radius, material}
  {}

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    auto center = m_center.at(ray.time());
    auto oc = center - ray.origin();
    auto a = glm::dot(ray.direction(), ray.direction());
    auto h = glm::dot(oc, ray.direction());
    auto c = glm::dot(oc, oc) - m_radius * m_radius;
    auto discriminant = h * h - a * c;
    
    if (discriminant < 0.0f) {
      return {};
    }
    
    auto sqrt_discriminant = std::sqrt(discriminant);

    auto front_face = true;
    auto root = (h - sqrt_discriminant) / a;
    if (root < min_distance || max_distance < root) {
      front_face = false;
      root = (h + sqrt_discriminant) / a;
      if (root < min_distance || max_distance < root) {
        return {};
      }
    }

    auto point = ray.at(root);
    auto out_normal = (point - center) / m_radius;
    auto texture_coords = get_texture_coords(out_normal);

    return HitRecord{root, front_face, point, front_face ? out_normal : -out_normal, m_material, texture_coords};
  }

  auto get_texture_coords(const glm::vec3& normal) const -> glm::vec2 {
    auto theta = std::acos(-normal.y);
    auto phi = std::atan2(-normal.z, normal.x) + glm::pi<float>();
    auto u = phi / (2.0f * glm::pi<float>());
    auto v = theta / glm::pi<float>();
    return {u, v};
  }

  auto bounding_box() const -> Aabb override {
    return m_bounding_box;
  }

private:
  Ray m_center{};
  float m_radius{};
  std::shared_ptr<Material> m_material{};
  Aabb m_bounding_box{};
};


#endif