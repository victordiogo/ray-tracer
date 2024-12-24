#include "sphere.hpp"
#include "quad.hpp"
#include "renderer.hpp"
#include "material.hpp"
#include "bvh.hpp"
#include "texture.hpp"
#include "image.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>

#include <memory>

auto random_color(float min = 0.0f, float max = 1.0f) -> glm::vec3 {
  return glm::vec3{prng::get_real(min, max), prng::get_real(min, max), prng::get_real(min, max)};
}

auto bouncing_spheres() {
  auto ppm = Ppm{"output.ppm", 800, 500};
  constexpr auto fov = 20.0f * glm::pi<float>() / 180.0f;
  constexpr auto num_samples = 30u;
  constexpr auto max_depth = 10u;
  constexpr auto look_from = glm::vec3{13.0f, 2.0f, 3.0f};
  constexpr auto look_at = glm::vec3{0.0f, 0.0f, 0.0f};
  constexpr auto defocus_angle = 0.6f * glm::pi<float>() / 180.0f;
  auto focus_distance = 10.0f;

  auto hittables = Hittables{};

  auto checker = std::make_shared<CheckerTexture>(0.6, glm::vec3{0.2f, 0.4f, 0.1f}, glm::vec3{0.1f, 0.2f, 0.5f});
  auto ground_material = std::make_shared<Lambertian>(checker);
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
        auto center2 = center;

        if (choose_material < 0.8f) {
          auto albedo = random_color() * random_color();
          center2 += glm::vec3{0.0f, prng::get_real(0.0f, 0.5f), 0.0f};
          sphere_material = std::make_shared<Lambertian>(albedo);
        } else if (choose_material < 0.95f) {
          auto albedo = random_color(0.5f, 1.0f);
          auto fuzz = prng::get_real(0.0f, 0.5f);
          sphere_material = std::make_shared<Metal>(albedo, fuzz);
        } else {
          sphere_material = std::make_shared<Dielectric>(1.5f);
        }

        auto sphere = std::make_shared<Sphere>(center, center2, 0.2f, sphere_material);
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

  hittables = {std::make_shared<BvhNode>(hittables)};

  auto options = RenderOptions{fov, num_samples, max_depth, look_from, look_at, focus_distance, defocus_angle};
  render(ppm, options, hittables);
}

auto checkered_spheres() {
  auto ppm = Ppm{"output.ppm", 700, 450};
  constexpr auto fov = 20.0f * glm::pi<float>() / 180.0f;
  constexpr auto num_samples = 50u;
  constexpr auto max_depth = 8u;
  constexpr auto look_from = glm::vec3{13.0f, 2.0f, 3.0f};
  constexpr auto look_at = glm::vec3{0.0f, 0.0f, 0.0f};
  constexpr auto defocus_angle = 0.0f;
  auto focus_distance = 10.0f;

  auto hittables = Hittables{};

  auto checker = std::make_shared<CheckerTexture>(0.32f, glm::vec3{0.2f, 0.3f, 0.1f}, glm::vec3{0.9f, 0.9f, 0.9f});
  auto material = std::make_shared<Lambertian>(checker);

  auto sphere1 = std::make_shared<Sphere>(glm::vec3{0.0f, -10.0f, 0.0f}, 10.0f, material);
  hittables.push_back(sphere1);

  auto sphere2 = std::make_shared<Sphere>(glm::vec3{0.0f, 10.0f, 0.0f}, 10.0f, material);
  hittables.push_back(sphere2);

  hittables = {std::make_shared<BvhNode>(hittables)};

  auto options = RenderOptions{fov, num_samples, max_depth, look_from, look_at, focus_distance, defocus_angle};
  render(ppm, options, hittables);
}

auto world() {
  auto ppm = Ppm{"output.ppm", 1000, 600};
  
  auto options = RenderOptions{};
  options.fov = 20.0f * glm::pi<float>() / 180.0f;
  options.num_samples = 100u;
  options.max_depth = 8u;
  options.look_from = glm::vec3{0.0f, 0.0f, 12.0f};
  options.look_at = glm::vec3{0.0f, 0.0f, 0.0f};
  options.defocus_angle = 0.0f;
  options.focus_distance = 10.0f;

  auto hittables = Hittables{};

  auto texture = load_image("./assets/textures/earthmap.jpg");
  if (!texture) {
    std::cerr << "Failed to load texture\n";
    return;
  }

  auto earth_material = std::make_shared<Lambertian>(std::make_shared<ImageTexture>(*texture));
  auto earth = std::make_shared<Sphere>(glm::vec3{0.0f, 0.0f, 0.0f}, 2.0f, earth_material);
  hittables.push_back(earth);

  hittables = {std::make_shared<BvhNode>(hittables)};

  render(ppm, options, hittables);
}

auto perlin_spheres() {
  auto ppm = Ppm{"output.ppm", 1280, 720};
  
  auto options = RenderOptions{};
  options.fov = 20.0f * glm::pi<float>() / 180.0f;
  options.num_samples = 25u;
  options.max_depth = 8u;
  options.look_from = glm::vec3{13.0f, 2.0f, 3.0f};
  options.look_at = glm::vec3{0.0f, 0.0f, 0.0f};
  options.defocus_angle = 0.0f;
  options.focus_distance = 10.0f;

  auto hittables = Hittables{};

  auto texture = std::make_shared<NoiseTexture>(2.0f);
  auto material = std::make_shared<Lambertian>(texture);

  auto sphere1 = std::make_shared<Sphere>(glm::vec3{0.0f, -1000.0f, 0.0f}, 1000.0f, material);
  hittables.push_back(sphere1);

  auto sphere2 = std::make_shared<Sphere>(glm::vec3{0.0f, 2.0f, 0.0f}, 2.0f, material);
  hittables.push_back(sphere2);

  hittables = {std::make_shared<BvhNode>(hittables)};

  render(ppm, options, hittables);
}

auto quads() {
  auto ppm = Ppm{"output.ppm", 500, 500};
  
  auto options = RenderOptions{};
  options.fov = 80.0f * glm::pi<float>() / 180.0f;
  options.num_samples = 50u;
  options.max_depth = 20u;
  options.look_from = glm::vec3{0.0f, 0.0f, 9.0f};
  options.look_at = glm::vec3{0.0f, 0.0f, 0.0f};
  options.defocus_angle = 0.0f;
  options.focus_distance = 10.0f;

  auto hittables = Hittables{};

  auto back_material = std::make_shared<Lambertian>(glm::vec3{1.0f, 0.2f, 0.2f});
  auto back_quad = std::make_shared<Quad>(glm::vec3{-2.0f, -2.0f, 0.0f}, glm::vec3{4.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 4.0f, 0.0f}, back_material);
  hittables.push_back(back_quad);

  auto left_material = std::make_shared<Lambertian>(glm::vec3{0.2f, 0.2f, 1.0f});
  auto left_quad = std::make_shared<Quad>(glm::vec3{-2.0f, -2.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 4.0f}, glm::vec3{0.0f, 4.0f, 0.0f}, left_material);
  hittables.push_back(left_quad);

  auto right_material = std::make_shared<Lambertian>(glm::vec3{0.2f, 1.0f, 0.2f});
  auto right_quad = std::make_shared<Quad>(glm::vec3{2.0f, -2.0f, 0.0f}, glm::vec3{0.0f, 4.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 4.0f}, right_material);
  hittables.push_back(right_quad);

  auto bottom_material = std::make_shared<Lambertian>(glm::vec3{1.0f, 1.0f, 1.0f});
  auto bottom_quad = std::make_shared<Quad>(glm::vec3{-2.0f, -2.0f, 0.0f}, glm::vec3{4.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 4.0f}, bottom_material);
  hittables.push_back(bottom_quad);

  auto top_material = std::make_shared<Lambertian>(glm::vec3{0.5f, 0.0f, 0.5f});
  auto top_quad = std::make_shared<Quad>(glm::vec3{-2.0f, 2.0f, 0.0f}, glm::vec3{4.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 4.0f}, top_material);
  hittables.push_back(top_quad);

  hittables = {std::make_shared<BvhNode>(hittables)};

  render(ppm, options, hittables);
}

auto simple_light() {
  auto hittables = Hittables{};

  auto texture = std::make_shared<NoiseTexture>(2.0f);
  auto material = std::make_shared<Lambertian>(texture);
  auto sphere = std::make_shared<Sphere>(glm::vec3{0.0f, -1000.0f, 0.0f}, 1000.0f, material);
  hittables.push_back(sphere);

  auto material2 = std::make_shared<Metal>(glm::vec3{0.7f, 0.6f, 0.5f}, 0.2f);
  auto sphere2 = std::make_shared<Sphere>(glm::vec3{0.0f, 2.0f, 0.0f}, 2.0f, material2);
  hittables.push_back(sphere2);

  auto checker = std::make_shared<CheckerTexture>(1.0f, glm::vec3{0.2f, 0.3f, 0.1f}, glm::vec3{0.9f, 0.9f, 0.9f});
  auto material3 = std::make_shared<Lambertian>(checker);
  auto sphere3 = std::make_shared<Sphere>(glm::vec3{-3.0f, 2.0f, 3.0f}, 2.0f, material3); 
  hittables.push_back(sphere3);

  auto light = std::make_shared<DiffuseLight>(glm::vec3{4.0f, 4.0f, 4.0f});
  auto sphere4 = std::make_shared<Sphere>(glm::vec3{0.0f, 7.0f, 0.0f}, 2.0f, light);
  hittables.push_back(sphere4);

  auto quad = std::make_shared<Quad>(glm::vec3{3.0f, 1.0f, -2.0f}, glm::vec3{2.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 2.0f, 0.0f}, light);
  hittables.push_back(quad);

  hittables = {std::make_shared<BvhNode>(hittables)};

  auto ppm = Ppm{"output.ppm", 800, 400};
  auto options = RenderOptions{};
  options.num_samples = 5000u;
  options.max_depth = 10u;
  options.fov = 20.0f * glm::pi<float>() / 180.0f;
  options.look_from = glm::vec3{20.0f, 6.0f, 13.0f};
  options.look_at = glm::vec3{0.0f, 2.0f, 0.0f};
  options.background_color = glm::vec3{0.001f};

  render(ppm, options, hittables);
}

auto cornell_box() {
  auto hittables = Hittables{};

  auto red = std::make_shared<Lambertian>(glm::vec3{0.65f, 0.05f, 0.05f});
  auto white = std::make_shared<Lambertian>(glm::vec3{0.73f});
  auto green = std::make_shared<Lambertian>(glm::vec3{0.12f, 0.45f, 0.15f});
  auto light = std::make_shared<DiffuseLight>(glm::vec3{15.0f});

  auto left_wall = std::make_shared<Quad>(glm::vec3{555.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 555.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 555.0f}, green);
  hittables.push_back(left_wall);

  auto right_wall = std::make_shared<Quad>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 555.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 555.0f}, red);
  hittables.push_back(right_wall);

  auto light_quad = std::make_shared<Quad>(glm::vec3{343.0f, 554.0f, 332.0f}, glm::vec3{-130.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -105.0f}, light);
  hittables.push_back(light_quad);

  auto floor = std::make_shared<Quad>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{555.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 555.0f}, white);
  hittables.push_back(floor);

  auto ceiling = std::make_shared<Quad>(glm::vec3{555.0f, 555.0f, 555.0f}, glm::vec3{-555.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -555.0f}, white);
  hittables.push_back(ceiling);

  auto back_wall = std::make_shared<Quad>(glm::vec3{0.0f, 0.0f, 555.0f}, glm::vec3{555.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 555.0f, 0.0f}, white);
  hittables.push_back(back_wall);

  auto box1 = get_box(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{165.0f, 330.0f, 165.0f}, white);
  for (auto& hittable : box1) {
    hittable = {std::make_shared<RotateY>(hittable, 15.0f * glm::pi<float>() / 180.0f)};
    hittable = {std::make_shared<Translate>(hittable, glm::vec3{265.0f, 0.0f, 295.0f})};
    hittables.push_back(hittable);
  }

  auto box2 = get_box(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{165.0f, 165.0f, 165.0f}, white);
  for (auto& hittable : box2) {
    hittable = {std::make_shared<RotateY>(hittable, -18.0f * glm::pi<float>() / 180.0f)};
    hittable = {std::make_shared<Translate>(hittable, glm::vec3{130.0f, 0.0f, 65.0f})};
    hittables.push_back(hittable);
  }

  hittables = {std::make_shared<BvhNode>(hittables)};

  auto ppm = Ppm{"output.ppm", 1000, 1000};
  auto options = RenderOptions{};
  options.num_samples = 5000u;
  options.max_depth = 15u;
  options.fov = 40.0f * glm::pi<float>() / 180.0f;
  options.look_from = glm::vec3{278.0f, 278.0f, -800.0f};
  options.look_at = glm::vec3{278.0f, 278.0f, 0.0f};
  options.background_color = glm::vec3{0.0f};

  render(ppm, options, hittables);
}

auto main() -> int {
  // bouncing_spheres();
  // checkered_spheres();
  // world();
  // perlin_spheres();
  // quads();
  // simple_light();
  cornell_box();

  return 0;
}
