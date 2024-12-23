#ifndef RT_HITTABLE_HPP
#define RT_HITTABLE_HPP

#include "ray.hpp"
#include "aabb.hpp"

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
  glm::vec2 texture_coords{};
};

class Hittable {
public:
  virtual ~Hittable() = default;
  virtual auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> = 0;
  virtual auto bounding_box() const -> Aabb = 0;
};

using Hittables = std::vector<std::shared_ptr<Hittable>>;

#endif