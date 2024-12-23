#ifndef RT_PERLIN_HPP
#define RT_PERLIN_HPP

#include "random.hpp"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <array>

class Perlin final {
public:
  Perlin() {
    for (auto i = 0u; i < s_num_points; ++i) {
      m_rand_vec[i] = prng::get_unit_vector() * prng::get_real(-1.0f, 1.0f);
    }

    m_perm_x = perlin_generate_perm();
    m_perm_y = perlin_generate_perm();
    m_perm_z = perlin_generate_perm();
  }

  auto noise(const glm::vec3& p) const -> float {
    auto u = p.x - std::floor(p.x);
    auto v = p.y - std::floor(p.y);
    auto w = p.z - std::floor(p.z);

    auto i = static_cast<unsigned>(std::floor(p.x));
    auto j = static_cast<unsigned>(std::floor(p.y));
    auto k = static_cast<unsigned>(std::floor(p.z));

    auto c = std::array<std::array<std::array<glm::vec3, 2>, 2>, 2>{};

    for (auto di = 0u; di < 2u; ++di) {
      for (auto dj = 0u; dj < 2u; ++dj) {
        for (auto dk = 0u; dk < 2u; ++dk) {
          c[di][dj][dk] = m_rand_vec[
            m_perm_x[(i + di) & 255] ^ 
            m_perm_y[(j + dj) & 255] ^ 
            m_perm_z[(k + dk) & 255]];
        }
      }
    }

    return perlin_interp(c, u, v, w);
  }

  auto turb(const glm::vec3& p, unsigned depth = 7u) const -> float {
    auto accum = 0.0f;
    auto temp_p = p;
    auto weight = 1.0f;

    for (auto i = 0u; i < depth; ++i) {
      accum += weight * noise(temp_p);
      weight *= 0.5f;
      temp_p *= 2.0f;
    }

    return std::fabs(accum);
  }

private:
  static constexpr auto s_num_points = 256;
  using PerlinNoise = std::array<unsigned, s_num_points>;

  std::array<glm::vec3, s_num_points> m_rand_vec{};
  PerlinNoise m_perm_x{};
  PerlinNoise m_perm_y{};
  PerlinNoise m_perm_z{};

  static auto perlin_generate_perm() -> PerlinNoise {
    auto perm = PerlinNoise{};
    for (auto i = 0u; i < s_num_points; ++i) {
      perm[i] = i;
    }
    permute(perm);
    return perm;
  }

  static auto permute(PerlinNoise& perm) -> void {    
    for (auto i = perm.size() - 1; i > 0; --i) {
      auto target = prng::get_int(0uz, i);
      std::swap(perm[i], perm[target]);
    }
  }

  static auto perlin_interp(const std::array<std::array<std::array<glm::vec3, 2>, 2>, 2>& c, float u, float v, float w) -> float{
    auto uu = u * u * (3.0f - 2.0f * u);
    auto vv = v * v * (3.0f - 2.0f * v);
    auto ww = w * w * (3.0f - 2.0f * w);
    auto accum = 0.0f;

    for (auto i = 0u; i < 2u; ++i) {
      for (auto j = 0u; j < 2u; ++j) {
        for (auto k = 0u; k < 2u; ++k) {
          auto fi = static_cast<float>(i);
          auto fj = static_cast<float>(j);
          auto fk = static_cast<float>(k);
          auto weight_v = glm::vec3{u - fi, v - fj, w - fk};
          accum += (fi * uu + (1.0f - fi) * (1.0f - uu)) *
                   (fj * vv + (1.0f - fj) * (1.0f - vv)) *
                   (fk * ww + (1.0f - fk) * (1.0f - ww)) *
                   glm::dot(c[i][j][k], weight_v);
        }
      }
    }

    return accum;
  }
};

#endif