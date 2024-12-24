#ifndef RT_BVH_HPP
#define RT_BVH_HPP

#include "aabb.hpp"
#include "hittable.hpp"
#include "random.hpp"

#include <memory>
#include <optional>
#include <algorithm>

class BvhNode : public Hittable {
public:
  BvhNode(Hittables& hittables, unsigned start, unsigned end) {
    auto bounding_box = hittables[start]->bounding_box();
    for (auto i = start + 1; i < end; ++i) {
      bounding_box = Aabb{bounding_box, hittables[i]->bounding_box()};
    }

    auto axis = bounding_box.longest_axis();

    auto compare = [axis](const auto& a, const auto& b) {
      return a->bounding_box().axes()[axis].min < b->bounding_box().axes()[axis].min;
    };

    auto span = end - start;

    if (span == 1) {
      m_left = m_right = hittables[start];
    } 
    else if (span == 2) {
      m_left = hittables[start];
      m_right = hittables[start + 1];
    } 
    else {
      std::sort(hittables.begin() + start, hittables.begin() + end, compare);

      auto mid = start + span / 2;
      m_left = std::make_shared<BvhNode>(hittables, start, mid);
      m_right = std::make_shared<BvhNode>(hittables, mid, end);
    }

    m_bounding_box = Aabb{m_left->bounding_box(), m_right->bounding_box()};
  }

  BvhNode(Hittables& hittables) 
    : BvhNode{hittables, 0, static_cast<unsigned>(hittables.size())} 
  {}

  auto hit(const Ray& ray, float min_distance, float max_distance) const -> std::optional<HitRecord> override {
    if (!m_bounding_box.hit(ray, min_distance, max_distance)) {
      return {};
    }

    auto left_hit = m_left->hit(ray, min_distance, max_distance);
    auto right_hit = m_right->hit(ray, min_distance, left_hit ? left_hit->distance : max_distance);

    if (right_hit) {
      return right_hit;
    }

    return left_hit;
  }

  auto bounding_box() const -> Aabb override {
    return m_bounding_box;
  }

private:
  std::shared_ptr<Hittable> m_left{};
  std::shared_ptr<Hittable> m_right{};
  Aabb m_bounding_box{};
};

#endif