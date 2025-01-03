#ifndef RT_RENDERER_HPP
#define RT_RENDERER_HPP

#include "ppm.hpp"
#include "hittable.hpp"
#include "timer.hpp"
#include "random.hpp"
#include "material.hpp"

#include <glm/geometric.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <limits>
#include <memory>
#include <vector>
#include <cmath>
#include <optional>

constexpr auto g_max_float = std::numeric_limits<float>::max();

auto trace(const Ray& ray, const Hittables& hittables) -> std::optional<HitRecord> {
  auto closest_hit_record = HitRecord{g_max_float};

  for (const auto& hittable : hittables) {
    auto hit_record = hittable->hit(ray, 0.0f, g_max_float);
    if (hit_record && hit_record->distance < closest_hit_record.distance) {
      closest_hit_record = *hit_record;
    }
  }

  if (closest_hit_record.distance == g_max_float) {
    return {};
  }

  return closest_hit_record;
}

auto ray_cast(const Ray& ray, unsigned depth, const glm::vec3& background_color, const Hittables& hittables) -> glm::vec3 {
  if (depth == 0) {
    return glm::vec3{0.0f};
  }

  auto hit_record = trace(ray, hittables);
  if (hit_record) {
    auto scatter_data = hit_record->material->scatter(ray, *hit_record);
    if (scatter_data) {
      if (!near_zero(scatter_data->emission)) {
        return scatter_data->emission;
      }
      return scatter_data->attenuation * ray_cast(scatter_data->scattered, depth - 1, background_color, hittables);
    }
    return glm::vec3{0.0f};
  }

  return background_color;
}

struct RenderOptions {
  float fov{0.9f};
  unsigned num_samples{};
  unsigned max_depth{};
  glm::vec3 look_from{};
  glm::vec3 look_at{};
  float focus_distance{1.0f};
  float defocus_angle{};
  glm::vec3 background_color{0.5f, 0.7f, 1.0f};
};

auto render(Ppm& ppm, const RenderOptions& options, const Hittables& hittables) -> void {
  auto widthf = static_cast<float>(ppm.width());
  auto heightf = static_cast<float>(ppm.height());
  auto aspect_ratio = widthf / heightf;

  auto w = glm::normalize(options.look_from - options.look_at);
  auto u = glm::normalize(glm::cross(glm::vec3{0.0f, 1.0f, 0.0f}, w));
  auto v = glm::cross(w, u);

  auto viewport_height = 2.0f * options.focus_distance * std::tan(options.fov / 2.0f);
  auto viewport_width = aspect_ratio * viewport_height;

  auto du = u * (viewport_width / widthf);
  auto dv = -v * (viewport_height / heightf);
  auto start = options.look_from - 0.5f * viewport_width * u + 0.5f * viewport_height * v - options.focus_distance * w + 0.5f * (du + dv);

  auto defocus_radius = options.focus_distance * std::tan(options.defocus_angle / 2);

  auto sqrt_samples = static_cast<unsigned>(std::sqrt(static_cast<float>(options.num_samples)));
  auto inv_sqrt_samples = 1.0f / static_cast<float>(sqrt_samples);

  auto color_scale = 1.0f / static_cast<float>(sqrt_samples * sqrt_samples); 

  auto framebuffer = std::vector<glm::vec3>(ppm.width() * ppm.height());
 
  auto timer = Timer{};

  #pragma omp parallel for schedule(dynamic, 1)
  for (auto y = 0u; y < ppm.height(); ++y) {
    for (auto x = 0u; x < ppm.width(); ++x) {
      auto color = glm::vec3{0.0f};
      auto direction = start + 
        static_cast<float>(x) * du + 
        static_cast<float>(y) * dv;
        
      for (auto sample_y = 0u; sample_y < sqrt_samples; ++sample_y) {
        for (auto sample_x = 0u; sample_x < sqrt_samples; ++sample_x) {
          auto theta = prng::get_real(0.0f, 2.0f * glm::pi<float>());
          auto r = prng::get_real(0.0f, 1.0f);
          auto lens_offset = defocus_radius * r * (std::cos(theta) * u + std::sin(theta) * v);

          auto sxf = static_cast<float>(sample_x);
          auto syf = static_cast<float>(sample_y);
          auto dir_offset = 
            ((sxf + prng::get_real(0.0f, 1.0f)) * inv_sqrt_samples - 0.5f) * du + 
            ((syf + prng::get_real(0.0f, 1.0f)) * inv_sqrt_samples - 0.5f) * dv;

          auto origin = options.look_from + lens_offset;
          auto ray = Ray{origin, direction + dir_offset - origin, prng::get_real(0.0f, 1.0f)};
          color += ray_cast(ray, options.max_depth, options.background_color, hittables);
        }
      }

      framebuffer[y * ppm.width() + x] = color * color_scale;
    }

    if (y % 50 == 0) {
      auto progress = static_cast<float>(y) / static_cast<float>(ppm.height());
      std::cout << "Progress: " << progress * 100 << "% (" << timer.elapsed() / 1000 << "s)\n";
    }
  }
  std::cout << "Render time: " << timer.elapsed() / 1000 << "s\n";

  for (auto i = 0u; i < ppm.width() * ppm.height(); ++i) {
    ppm.write_color(framebuffer[i]);
  }
}

#endif