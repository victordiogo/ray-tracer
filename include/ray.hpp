#ifndef RT_RAY_HPP
#define RT_RAY_HPP

#include <glm/vec3.hpp>

class Ray final {
public:
  Ray() = default;

  Ray(const glm::vec3& origin, const glm::vec3& direction, float time = 0.0f)
    : m_origin{origin}
    , m_direction{direction}
    , m_time{time} {
  }

  auto origin() const -> const glm::vec3& {
    return m_origin;
  }

  auto direction() const -> const glm::vec3& {
    return m_direction;
  }

  auto time() const -> float {
    return m_time;
  }

  auto at(float t) const -> glm::vec3 {
    return m_origin + t * m_direction;
  }

private:
  glm::vec3 m_origin{};
  glm::vec3 m_direction{};
  float m_time{};
};

#endif