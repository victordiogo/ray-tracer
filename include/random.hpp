#ifndef RT_RANDOM_HPP
#define RT_RANDOM_HPP

#include <glm/vec3.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <random>
#include <chrono>

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
    auto theta = prng::get_real(0.0f, 2.0f * glm::pi<float>());
    auto phi = prng::get_real(0.0f, glm::pi<float>());
    return glm::vec3{
      std::sin(phi) * std::cos(theta),
      std::cos(phi),
      std::sin(phi) * std::sin(theta)};
  }
}
#endif