#ifndef RT_QUAD_HPP
#define RT_QUAD_HPP

#include "hittable.hpp"
#include "random.hpp"

#include <glm/geometric.hpp>

#include <memory>
#include <optional>
#include <iostream>

class Quad : public Hittable {
public:
  Quad(const glm::vec3& p, const glm::vec3& q, const glm::vec3& r, const std::shared_ptr<Material>& material)
    : m_p{p}
    , m_q{q}
    , m_r{r}
    , m_qxr{glm::cross(m_q, m_r)}
    , m_normal{glm::normalize(m_qxr)}
    , m_material{material}
  {
    set_bounding_box();
  }

  virtual auto set_bounding_box() -> void {
    auto diagonal1 = Aabb{m_p, m_p + m_q + m_r};
    auto diagonal2 = Aabb{m_p + m_q, m_p + m_r};
    m_bounding_box = Aabb{diagonal1, diagonal2};
  }

  virtual auto bounding_box() const -> Aabb override {
    return m_bounding_box;
  }

  virtual auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    // o + dt = p + uq + vr
    // PO = matrix(q, r, -d) * vector(u, v, t)

    auto det = glm::dot(m_qxr, -ray.direction());
    if (std::fabs(det) < 1e-6f) {
      return {};
    }

    auto inv_det = 1.0f / det;
    auto po = ray.origin() - m_p;

    auto dxpo = glm::cross(ray.direction(), po);
    auto det_u = glm::dot(-dxpo, m_r);
    auto u = det_u * inv_det;
    if (u < 0.0f || u > 1.0f) {
      return {};
    }

    auto det_v = glm::dot(dxpo, m_q);
    auto v = det_v * inv_det;
    if (v < 0.0f || v > 1.0f) {
      return {};
    }

    auto det_t = glm::dot(m_qxr, po);
    auto t = det_t * inv_det;
    if (t < min_distance || max_distance < t) {
      return {};
    }

    auto front_face = glm::dot(ray.direction(), m_normal) < 0.0f;

    return HitRecord{t, front_face, ray.at(t), front_face ? m_normal : -m_normal, m_material, glm::vec2{u, v}};
  }

private:
  glm::vec3 m_p{};
  glm::vec3 m_q{};
  glm::vec3 m_r{};
  glm::vec3 m_qxr{};
  glm::vec3 m_normal{};
  std::shared_ptr<Material> m_material{};
  Aabb m_bounding_box{};
};

auto get_box(const glm::vec3& a, const glm::vec3& b, std::shared_ptr<Material> material) -> Hittables {
  auto sides = Hittables{};

  auto min = glm::vec3{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
  auto max = glm::vec3{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};

  auto dx = glm::vec3{max.x - min.x, 0.0f, 0.0f};
  auto dy = glm::vec3{0.0f, max.y - min.y, 0.0f};
  auto dz = glm::vec3{0.0f, 0.0f, max.z - min.z};

  sides.push_back(std::make_shared<Quad>(glm::vec3{min.x, min.y, max.z}, dx, dy, material)); // front
  sides.push_back(std::make_shared<Quad>(glm::vec3{max.x, min.y, max.z}, -dz, dy, material)); // right
  sides.push_back(std::make_shared<Quad>(glm::vec3{max.x, min.y, min.z}, -dx, dy, material)); // back
  sides.push_back(std::make_shared<Quad>(glm::vec3{min.x, min.y, min.z}, dz, dy, material)); // left
  sides.push_back(std::make_shared<Quad>(glm::vec3{min.x, max.y, max.z}, dx, -dz, material)); // top
  sides.push_back(std::make_shared<Quad>(glm::vec3{min.x, min.y, min.z}, dx, dz, material)); // bottom

  return sides;
}

#endif