#ifndef RT_AABB_HPP
#define RT_AABB_HPP

#include "ray.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <stdexcept>
#include <cmath>
#include <array>

class Aabb {
public:
  struct Interval {
    float min{};
    float max{};
  };

  Aabb() = default;

  Aabb(const glm::vec3& a, const glm::vec3& b)
    : m_axes{Interval{a.x, b.x}, Interval{a.y, b.y}, Interval{a.z, b.z}}
  {
    if (m_axes[0].min > m_axes[0].max) {
      std::swap(m_axes[0].min, m_axes[0].max);
    }
    if (m_axes[1].min > m_axes[1].max) {
      std::swap(m_axes[1].min, m_axes[1].max);
    }
    if (m_axes[2].min > m_axes[2].max) {
      std::swap(m_axes[2].min, m_axes[2].max);
    }

    pad_to_minimuns();
  }

  Aabb(const Aabb& aabb1, const Aabb& aabb2)
    : m_axes{Interval{std::min(aabb1.m_axes[0].min, aabb2.m_axes[0].min), std::max(aabb1.m_axes[0].max, aabb2.m_axes[0].max)},
             Interval{std::min(aabb1.m_axes[1].min, aabb2.m_axes[1].min), std::max(aabb1.m_axes[1].max, aabb2.m_axes[1].max)},
             Interval{std::min(aabb1.m_axes[2].min, aabb2.m_axes[2].min), std::max(aabb1.m_axes[2].max, aabb2.m_axes[2].max)}}
  {}

  auto operator+(const glm::vec3& offset) const -> Aabb {
    return Aabb{glm::vec3{m_axes[0].min + offset.x, m_axes[1].min + offset.y, m_axes[2].min + offset.z},
                glm::vec3{m_axes[0].max + offset.x, m_axes[1].max + offset.y, m_axes[2].max + offset.z}};
  }

  friend auto operator+(const glm::vec3& offset, const Aabb& aabb) -> Aabb {
    return aabb + offset;
  }

  auto axes() const -> const std::array<Interval, 3>& {
    return m_axes;
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> bool {
    for (auto axis_index = 0u; axis_index < 3u; ++axis_index) {
      auto axis = m_axes[axis_index];
      auto indexi = static_cast<int>(axis_index);
      auto inv_d = 1.0f / ray.direction()[indexi];
      auto t0 = (axis.min - ray.origin()[indexi]) * inv_d;
      auto t1 = (axis.max - ray.origin()[indexi]) * inv_d;
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

  auto longest_axis() const -> unsigned {
    auto extent_x = m_axes[0].max - m_axes[0].min;
    auto extent_y = m_axes[1].max - m_axes[1].min;
    auto extent_z = m_axes[2].max - m_axes[2].min;

    if (extent_x > extent_y && extent_x > extent_z) {
      return 0u;
    }
    if (extent_y > extent_z) {
      return 1u;
    }
    return 2u;
  }

private:
  std::array<Interval, 3> m_axes{};

  auto pad_to_minimuns() -> void {
    auto delta = 0.0002f;
    if (m_axes[0].max - m_axes[0].min < delta) {
      m_axes[0].max += 0.5f * delta;
      m_axes[0].min -= 0.5f * delta;
    }
    if (m_axes[1].max - m_axes[1].min < delta) {
      m_axes[1].max += 0.5f * delta;
      m_axes[1].min -= 0.5f * delta;
    }
    if (m_axes[2].max - m_axes[2].min < delta) {
      m_axes[2].max += 0.5f * delta;
      m_axes[2].min -= 0.5f * delta;
    }
  }
};

#endif