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

class Translate : public Hittable {
public:
  Translate(std::shared_ptr<Hittable> hittable, const glm::vec3& offset)
    : m_hittable{hittable}
    , m_offset{offset}
  {
    m_bounding_box = m_hittable->bounding_box() + offset;
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    auto moved_ray = Ray{ray.origin() - m_offset, ray.direction(), ray.time()};
    auto hit_record = m_hittable->hit(moved_ray, min_distance, max_distance);
    if (hit_record) {
      hit_record->point += m_offset;
      return hit_record;
    }
    return {};
  }

  auto bounding_box() const -> Aabb override {
    return m_bounding_box;
  }

private:
  std::shared_ptr<Hittable> m_hittable{};
  glm::vec3 m_offset{};
  Aabb m_bounding_box{};
};

class RotateY : public Hittable {
public:
  RotateY(std::shared_ptr<Hittable> hittable, float angle)
    : m_hittable{hittable}
  {
    m_sin_theta = std::sin(angle);
    m_cos_theta = std::cos(angle);

    m_bounding_box = m_hittable->bounding_box();

    auto min = glm::vec3{std::numeric_limits<float>::max()};
    auto max = glm::vec3{-std::numeric_limits<float>::max()};

    for (auto i = 0; i < 2; ++i) {
      for (auto j = 0; j < 2; ++j) {
        for (auto k = 0; k < 2; ++k) {
          auto fi = static_cast<float>(i);
          auto fj = static_cast<float>(j);
          auto fk = static_cast<float>(k);
          auto x = fi * m_bounding_box.axes()[0].max + (1 - fi) * m_bounding_box.axes()[0].min;
          auto y = fj * m_bounding_box.axes()[1].max + (1 - fj) * m_bounding_box.axes()[1].min;
          auto z = fk * m_bounding_box.axes()[2].max + (1 - fk) * m_bounding_box.axes()[2].min;

          auto new_x = m_cos_theta * x + m_sin_theta * z;
          auto new_z = -m_sin_theta * x + m_cos_theta * z;

          auto tester = glm::vec3{new_x, y, new_z};

          for (auto c = 0; c < 3; ++c) {
            min[c] = std::fmin(min[c], tester[c]);
            max[c] = std::fmax(max[c], tester[c]);
          }
        }
      }
    }

    m_bounding_box = Aabb{min, max};
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    auto origin = glm::vec3{
      m_cos_theta * ray.origin().x - m_sin_theta * ray.origin().z,
      ray.origin().y,
      m_sin_theta * ray.origin().x + m_cos_theta * ray.origin().z
    };

    auto direction = glm::vec3{
      m_cos_theta * ray.direction().x - m_sin_theta * ray.direction().z,
      ray.direction().y,
      m_sin_theta * ray.direction().x + m_cos_theta * ray.direction().z
    };

    auto rotated_ray = Ray{origin, direction, ray.time()};

    auto hit_record = m_hittable->hit(rotated_ray, min_distance, max_distance);
    if (!hit_record) {
      return {};
    }

    auto point = glm::vec3{
      m_cos_theta * hit_record->point.x + m_sin_theta * hit_record->point.z,
      hit_record->point.y,
      -m_sin_theta * hit_record->point.x + m_cos_theta * hit_record->point.z
    };

    auto normal = glm::vec3{
      m_cos_theta * hit_record->normal.x + m_sin_theta * hit_record->normal.z,
      hit_record->normal.y,
      -m_sin_theta * hit_record->normal.x + m_cos_theta * hit_record->normal.z
    };

    hit_record->point = point;
    hit_record->normal = normal;

    return hit_record;
  }

  auto bounding_box() const -> Aabb override {
    return m_bounding_box;
  }
  
private:
  std::shared_ptr<Hittable> m_hittable{};
  float m_sin_theta{};
  float m_cos_theta{};
  Aabb m_bounding_box{};
};

#endif