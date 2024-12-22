#ifndef RT_AABB_HPP
#define RT_AABB_HPP

#include "ray.hpp"

#include <glm/vec3.hpp>

#include <stdexcept>
#include <cmath>

auto min_vec(const glm::vec3& a, const glm::vec3& b) -> glm::vec3 {
  return glm::vec3{std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z)};
}

auto max_vec(const glm::vec3& a, const glm::vec3& b) -> glm::vec3 {
  return glm::vec3{std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z)};
}

class Aabb {
public:
  Aabb() = default;

  Aabb(const glm::vec3& min, const glm::vec3& max)
    : m_min{min}
    , m_max{max}
  {
    if (m_min.x > m_max.x || m_min.y > m_max.y || m_min.z > m_max.z) {
      throw std::invalid_argument{"Invalid AABB"};
    }
  }

  Aabb(const Aabb& aabb1, const Aabb& aabb2)
    : Aabb{min_vec(aabb1.min(), aabb2.min()), max_vec(aabb1.max(), aabb2.max())}
  {}

  auto min() const -> const glm::vec3& {
    return m_min;
  }

  auto max() const -> const glm::vec3& {
    return m_max;
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> bool {
    for (int axis = 0; axis < 3; ++axis) {
      auto inv_d = 1.0f / ray.direction()[axis];
      auto t0 = (m_min[axis] - ray.origin()[axis]) * inv_d;
      auto t1 = (m_max[axis] - ray.origin()[axis]) * inv_d;
      if (inv_d < 0.0f) {
        std::swap(t0, t1);
      }
      min_distance = t0 > min_distance ? t0 : min_distance;
      max_distance = t1 < max_distance ? t1 : max_distance;
      if (max_distance <= min_distance) {
        return false;
      }
    }
    return true;
  }

  auto longest_axis() const -> int {
    auto extent = m_max - m_min;
    if (extent.x > extent.y && extent.x > extent.z) {
      return 0;
    }
    if (extent.y > extent.z) {
      return 1;
    }
    return 2;
  }

private:
  glm::vec3 m_min{};
  glm::vec3 m_max{};
};

#endif