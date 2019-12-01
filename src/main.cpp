#include <iostream>
#include <vector>
#include <functional>

#define DECLARE_DAY(n) \
  namespace day##n { \
    void problem1(); \
    void problem2(); \
  }

// Forward declarations
DECLARE_DAY(1);

int main(int argc, char const *argv[]) {
  std::vector<std::vector<std::function<void(void)>>> days = {
    {day1::problem1,  day1::problem2},
  };

  if (argc > 2) {
    std::cerr << "ERROR: Specify no params or a day # to run a specific day" << std::endl;
    return -1;
  }

  int day_to_run = -1;
  if (argc == 2) {
    try {
      day_to_run = std::stoi(argv[1]);
    } catch (...) {
      std::cerr << "ERROR: Invalid parameter!" << std::endl;
      return -2;
    }
    if (day_to_run < 1 || day_to_run > days.size()) {
      std::cerr << "ERROR: Day parameter is invalid!" << std::endl;
      return -3;
    }
  }

  if (day_to_run != -1) {
    // Run a specific day
    auto &day_problems = days[day_to_run - 1];
    std::cout << "Running day " << day_to_run << std::endl;
    for (auto &problem : day_problems) {
      problem();
      std::cout << std::endl;
    }
  } else {
    // Run all days
    for (auto &day_problems : days) {
      for (auto &problem : day_problems) {
        problem();
        std::cout << std::endl;
      }
    }
  }

  return 0;
}
