#ifndef RT_SPHERE_HPP
#define RT_SPHERE_HPP

#include "hittable.hpp"

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include <stdexcept>
#include <optional>

class Sphere : public Hittable {
public:
  Sphere(const glm::vec3& center, float radius, std::shared_ptr<Material> material)
    : m_center{center}
    , m_radius{radius} 
    , m_material{material}
  {
    if (radius <= 0.0f) {
      throw std::invalid_argument{"Sphere radius must be positive"};
    }
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    auto oc = m_center - ray.origin();
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
    auto out_normal = (point - m_center) / m_radius;

    return HitRecord{root, front_face, point, front_face ? out_normal : -out_normal, m_material};
  }

private:
  glm::vec3 m_center{};
  float m_radius{};
  std::shared_ptr<Material> m_material{};
};


#endif