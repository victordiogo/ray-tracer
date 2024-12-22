#include "sphere.hpp"
#include "renderer.hpp"
#include "material.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>

#include <memory>

auto random_color(float min = 0.0f, float max = 1.0f) -> glm::vec3 {
  return glm::vec3{prng::get_real(min, max), prng::get_real(min, max), prng::get_real(min, max)};
}

auto main() -> int {
  auto ppm = PPM{"output.ppm", 1280, 720};
  constexpr auto fov = 20.0f * glm::pi<float>() / 180.0f;
  constexpr auto num_samples = 50u;
  constexpr auto max_depth = 15u;
  constexpr auto look_from = glm::vec3{13.0f, 2.0f, 3.0f};
  constexpr auto look_at = glm::vec3{0.0f, 0.0f, 0.0f};
  constexpr auto defocus_angle = 0.6f * glm::pi<float>() / 180.0f;
  auto focus_distance = 10.0f;

  auto hittables = Hittables{};

  auto ground_material = std::make_shared<Lambertian>(glm::vec3{0.5f, 0.5f, 0.3f});
  auto ground = std::make_shared<Sphere>(glm::vec3{0.0f, -1000.0f, 0.0f}, 1000.0f, ground_material);
  hittables.push_back(ground);

  for (auto a = -11; a < 11; ++a) {
    for (auto b = -11; b < 11; ++b) {
      auto choose_material = prng::get_real(0.0f, 1.0f);
      auto af = static_cast<float>(a);
      auto bf = static_cast<float>(b);
      auto center = glm::vec3{af + 0.9f * prng::get_real(0.0f, 1.0f), 0.2f, bf + 0.9f * prng::get_real(0.0f, 1.0f)};

      if (glm::length(center - glm::vec3{4.0f, 0.2f, 0.0f}) > 0.9f) {
        std::shared_ptr<Material> sphere_material;

        if (choose_material < 0.8f) {
          auto albedo = random_color() * random_color();
          sphere_material = std::make_shared<Lambertian>(albedo);
        } else if (choose_material < 0.95f) {
          auto albedo = random_color(0.5f, 1.0f);
          auto fuzz = prng::get_real(0.0f, 0.5f);
          sphere_material = std::make_shared<Metal>(albedo, fuzz);
        } else {
          sphere_material = std::make_shared<Dielectric>(1.5f);
        }

        auto sphere = std::make_shared<Sphere>(center, 0.2f, sphere_material);
        hittables.push_back(sphere);
      }
    }
  }

  auto material1 = std::make_shared<Dielectric>(1.5f);
  auto sphere1 = std::make_shared<Sphere>(glm::vec3{0.0f, 1.0f, 0.0f}, 1.0f, material1);
  hittables.push_back(sphere1);

  auto material2 = std::make_shared<Lambertian>(glm::vec3{0.4f, 0.2f, 0.1f});
  auto sphere2 = std::make_shared<Sphere>(glm::vec3{-4.0f, 1.0f, 0.0f}, 1.0f, material2);
  hittables.push_back(sphere2);

  auto material3 = std::make_shared<Metal>(glm::vec3{0.7f, 0.6f, 0.5f}, 0.0f);
  auto sphere3 = std::make_shared<Sphere>(glm::vec3{4.0f, 1.0f, 0.0f}, 1.0f, material3);
  hittables.push_back(sphere3);

  auto options = RenderOptions{fov, num_samples, max_depth, look_from, look_at, focus_distance, defocus_angle};
  render(ppm, options, hittables);

  return 0;
}
