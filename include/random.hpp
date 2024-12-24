#ifndef RT_RANDOM_HPP
#define RT_RANDOM_HPP

#include <glm/vec3.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <random>
#include <chrono>
#include <cmath>

namespace prng {
  thread_local auto rd = std::random_device{};
  thread_local auto time = std::chrono::steady_clock::now().time_since_epoch().count();
  thread_local auto seed_seq = std::seed_seq{
    static_cast<std::seed_seq::result_type>(time), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
  thread_local auto mt = std::mt19937{seed_seq};

  template <typename T>
	T get_int(T min, T max) {
		return std::uniform_int_distribution<T>{min, max}(mt);
	}

  template <typename T>
  T get_real(T min, T max) {
    return std::uniform_real_distribution<T>{min, max}(mt);
  }

  auto get_unit_vector() -> glm::vec3 {
    auto a = get_real(0.0f, 2.0f * glm::pi<float>());
    auto z = get_real(-1.0f, 1.0f);
    auto r = std::sqrt(1.0f - z * z);
    return glm::vec3{r * std::cos(a), r * std::sin(a), z};
  }

  auto get_hemisphere_vector(const glm::vec3& normal) -> glm::vec3 {
    auto unit_vector = get_unit_vector();
    return glm::dot(unit_vector, normal) > 0.0f ? unit_vector : -unit_vector;
  }
}
#endif