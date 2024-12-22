#ifndef RT_RANDOM_HPP
#define RT_RANDOM_HPP

#include <random>
#include <chrono>

namespace prng {
  auto rd = std::random_device{};
  auto seed_seq = std::seed_seq{
    static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
    rd(), rd(), rd(), rd(), rd(), rd(), rd()};
  auto mt = std::mt19937{seed_seq};

  template <typename T>
	T get_int(T min, T max) {
		return std::uniform_int_distribution<T>{min, max}(mt);
	}

  template <typename T>
  T get_real(T min, T max) {
    return std::uniform_real_distribution<T>{min, max}(mt);
  }
}

#endif