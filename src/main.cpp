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
DECLARE_DAY(3);
DECLARE_DAY(4);
DECLARE_DAY(5);
DECLARE_DAY(6);
DECLARE_DAY(7);
DECLARE_DAY(8);
DECLARE_DAY(9);

void run_problem(int day, int problem, const std::function<void(void)>& logic) {
  std::cout << "Day " << day << " - Problem " << problem << std::endl;
  logic();
  std::cout << "..Done\n" << std::endl;
}

int main(int argc, char const *argv[]) {
  std::vector<std::vector<std::function<void(void)>>> days = {
    {day1::problem1,  day1::problem2},
    {day2::problem1,  day2::problem2},
    {day3::problem1,  day3::problem2},
    {day4::problem1,  day4::problem2},
    {day5::problem1,  day5::problem2},
    {day6::problem1,  day6::problem2},
    {day7::problem1,  day7::problem2},
    {day8::problem1,  day8::problem2},
    {day9::problem1,  day9::problem2},
  };

  if (argc > 3) {
    std::cerr << "ERROR: Specify no params OR a day # to run a specific day OR day and problem number" << std::endl;
    return -1;
  }

  int day_to_run = -1;
  if (argc >= 2) {
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

  int problem_to_run = -1;
  if (argc == 3 && day_to_run != -1) {
    try {
      problem_to_run = std::stoi(argv[2]);
    } catch (...) {
      std::cerr << "ERROR: Invalid parameter!" << std::endl;
      return -4;
    }
    if (problem_to_run < 1 || problem_to_run > days[day_to_run - 1].size()) {
      std::cerr << "ERROR: Problem parameter is invalid!" << std::endl;
      return -5;
    }
  }

  if (day_to_run != -1) {
    // Run a specific day
    auto &day_problems = days[day_to_run - 1];
    if (problem_to_run != -1) {
      std::cout << "Running ONLY day " << day_to_run << " ONLY problem " << problem_to_run << std::endl << std::endl;
      run_problem(day_to_run, problem_to_run, day_problems[problem_to_run - 1]);
    } else {
      std::cout << "Running ONLY day " << day_to_run << std::endl << std::endl;
      auto problem_num = 1;
      for (auto &problem : day_problems) {
        run_problem(day_to_run, problem_num++, problem);
      }
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
