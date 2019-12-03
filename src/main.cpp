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
DECLARE_DAY(2);

void run_problem(int day, int problem, const std::function<void(void)>& logic) {
  std::cout << "Day " << day << " - Problem " << problem << std::endl;
  logic();
  std::cout << "..Done\n" << std::endl;
}

int main(int argc, char const *argv[]) {
  std::vector<std::vector<std::function<void(void)>>> days = {
    {day1::problem1,  day1::problem2},
    {day2::problem1,  day2::problem2},
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
    std::cout << "Running ONLY day " << day_to_run << std::endl << std::endl;
    auto problem_num = 1;
    for (auto &problem : day_problems) {
      run_problem(day_to_run, problem_num++, problem);
    }
  } else {
    // Run all days
    auto day_num = 1;
    for (auto &day_problems : days) {
      auto problem_num = 1;
      for (auto &problem : day_problems) {
        run_problem(day_num, problem_num++, problem);
      }
      day_num++;
    }
  }

  return 0;
}
