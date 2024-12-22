#ifndef RT_HITTABLE_HPP
#define RT_HITTABLE_HPP

#include "ray.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <optional>
#include <memory>

class Material;

struct HitRecord {
  float distance{};
  bool front_face{};
  glm::vec3 point{};
  glm::vec3 normal{};
  std::shared_ptr<Material> material{};
};

class Hittable {
public:
  virtual ~Hittable() = default;
  virtual auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> = 0;
};

#endif