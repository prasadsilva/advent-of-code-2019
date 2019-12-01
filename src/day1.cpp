#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <numeric>

namespace day1 {

  long fuel_required(long mass) {
    return (mass / 3) - 2;
  }

  long total_fuel_required(const std::vector<int> &masses) {
    return std::accumulate(masses.begin(), masses.end(), 0, [](long total, long mass) {
      return total + fuel_required(mass);
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

    assert(fuel_required(12) == 2);
    assert(fuel_required(14) == 2);
    assert(fuel_required(1969) == 654);
    assert(fuel_required(100756) == 33583);

    std::vector<int> input;
    read_day1_data(input, "data/day1/problem1/input.txt");
    std::cout << "Result : " << total_fuel_required(input) << std::endl;

#endif
  }

  void problem2() {
    std::cout << "Day 1 - Problem 2" << std::endl;
#if !defined(ONLY_ACTIVATE) || ONLY_ACTIVATE == 1

//    std::vector<int> test1;
//    read_day1_data(test1, "data/day1/problem2/test1.txt");
//    assert(get_first_repeating_frequency(test1) == 0);
//
//    std::vector<int> test2;
//    read_day1_data(test2, "data/day1/problem2/test2.txt");
//    assert(get_first_repeating_frequency(test2) == 10);
//
//    std::vector<int> test3;
//    read_day1_data(test3, "data/day1/problem2/test3.txt");
//    assert(get_first_repeating_frequency(test3) == 5);
//
//    std::vector<int> test4;
//    read_day1_data(test4, "data/day1/problem2/test4.txt");
//    assert(get_first_repeating_frequency(test4) == 14);
//
//    std::vector<int> input;
//    read_day1_data(input, "data/day1/problem2/input.txt");
//    std::cout << "Result : " << get_first_repeating_frequency(input) << std::endl;

#endif
  }

}
