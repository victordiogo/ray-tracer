#ifndef RT_TRIANGLE_HPP
#define RT_TRIANGLE_HPP

#include "hittable.hpp"
#include "material.hpp"
#include "aabb.hpp"
#include "quad.hpp"

#include <glm/vec3.hpp>

#include <memory>

class Triangle : public Hittable {
public:
  Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, 
           const glm::vec3& na, const glm::vec3& nb, const glm::vec3& nc,
           const glm::vec2& ta, const glm::vec2& tb, const glm::vec2& tc,
          const std::shared_ptr<Material>& material)
    : m_a(a), m_b(b), m_c(c)
    , m_na(na), m_nb(nb), m_nc(nc)
    , m_ta(ta), m_tb(tb), m_tc(tc)
    , m_material(material) {
    auto min = glm::vec3{
      std::fmin(std::fmin(a.x, b.x), c.x),
      std::fmin(std::fmin(a.y, b.y), c.y),
      std::fmin(std::fmin(a.z, b.z), c.z)
    };

    auto max = glm::vec3{
      std::fmax(std::fmax(a.x, b.x), c.x),
      std::fmax(std::fmax(a.y, b.y), c.y),
      std::fmax(std::fmax(a.z, b.z), c.z)
    };

    m_bounding_box = Aabb{min, max};
  }

  auto bounding_box() const -> Aabb override {
    return m_bounding_box;
  }

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    // o + dt = p + u(b - a) + v(c - a)
    // PO = matrix(b - a, c - a, -d) * vector(u, v, t)

    auto ab = m_b - m_a;
    auto ac = m_c - m_a;

    auto det = determinant(ab, ac, -ray.direction());
    if (std::fabs(det) < 1e-6f) {
      return {};
    }

    auto inv_det = 1.0f / det;
    auto po = ray.origin() - m_a;

    auto det_u = determinant(po, ac, -ray.direction());
    auto u = det_u * inv_det;
    if (u < 0.0f || u > 1.0f) {
      return {};
    }

    auto det_v = determinant(ab, po, -ray.direction());
    auto v = det_v * inv_det;
    if (v < 0.0f || v + u > 1.0f) {
      return {};
    }

    auto det_t = determinant(ab, ac, po);
    auto t = det_t * inv_det;
    if (t < min_distance || max_distance < t) {
      return {};
    }

    auto w = 1.0f - u - v;

    auto normal = glm::normalize(w * m_na + u * m_nb + v * m_nc);
    auto front_face = glm::dot(ray.direction(), normal) < 0.0f;

    auto tex = w * m_ta + u * m_tb + v * m_tc;

    return HitRecord{t, front_face, ray.at(t), front_face ? normal : -normal, m_material, tex};
  }

  auto a() const -> glm::vec3 {
    return m_a;
  }

  auto b() const -> glm::vec3 {
    return m_b;
  }

  auto c() const -> glm::vec3 {
    return m_c;
  }

  auto na() const -> glm::vec3 {
    return m_na;
  }

  auto nb() const -> glm::vec3 {
    return m_nb;
  }

  auto nc() const -> glm::vec3 {
    return m_nc;
  }

  auto ta() const -> glm::vec2 {
    return m_ta;
  }

  auto tb() const -> glm::vec2 {
    return m_tb;
  }

  auto tc() const -> glm::vec2 {
    return m_tc;
  }

  auto material() const -> std::shared_ptr<Material> {
    return m_material;
  }

private:
  glm::vec3 m_a{};
  glm::vec3 m_b{};
  glm::vec3 m_c{};
  glm::vec3 m_na{};
  glm::vec3 m_nb{};
  glm::vec3 m_nc{};
  glm::vec2 m_ta{};
  glm::vec2 m_tb{};
  glm::vec2 m_tc{};
  std::shared_ptr<Material> m_material{};
  Aabb m_bounding_box{};
};

#endif