#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <numeric>

namespace day1 {

  long get_fuel_required(long mass) {
    return (mass / 3) - 2;
  }

  long get_fuel_required_recursive(long mass) {
    long total_fuel_required = 0;
    long current_mass = mass;
    while (current_mass > 0) {
      auto fuel_required = std::max(get_fuel_required(current_mass), 0l);
      total_fuel_required += fuel_required;
      current_mass = fuel_required;
    }
    return total_fuel_required;
  }

  long get_total_fuel_required(const std::vector<int> &masses, std::function<long(long)> compute_fn) {
    return std::accumulate(masses.begin(), masses.end(), 0, [compute_fn](long total, long mass) {
      return total + compute_fn(mass);
    });
  }

  void read_day1_data(std::vector<int> &outdata, const char *filepath) {
    std::ifstream inputStream(filepath);
    std::istream_iterator<int> inputIterator(inputStream);
    std::copy(inputIterator, std::istream_iterator<int>(), std::back_inserter(outdata));
  }

  void problem1() {
    std::cout << "Day 1 - Problem 1" << std::endl;
#if !defined(ONLY_ACTIVATE) || ONLY_ACTIVATE == 1

    assert(get_fuel_required(12) == 2);
    assert(get_fuel_required(14) == 2);
    assert(get_fuel_required(1969) == 654);
    assert(get_fuel_required(100756) == 33583);

    std::vector<int> input;
    read_day1_data(input, "data/day1/problem1/input.txt");
    std::cout << "Result : " << get_total_fuel_required(input, get_fuel_required) << std::endl;

#endif
  }

  void problem2() {
    std::cout << "Day 1 - Problem 2" << std::endl;
#if !defined(ONLY_ACTIVATE) || ONLY_ACTIVATE == 1

    assert(get_fuel_required_recursive(14) == 2);
    assert(get_fuel_required_recursive(1969) == 966);
    assert(get_fuel_required_recursive(100756) == 50346);

    std::vector<int> input;
    read_day1_data(input, "data/day1/problem2/input.txt");
    std::cout << "Result : " << get_total_fuel_required(input, get_fuel_required_recursive) << std::endl;

#endif
  }

}
