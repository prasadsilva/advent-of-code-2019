#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>

namespace day4 {

  using password_t = char[7];

  void increment_password(password_t &password) {
    int idx = 5;
    while (idx >= 0) {
      auto digit = password[idx];
      if (digit == '9') {
        password[idx] = '0';
        idx--;
      } else {
        password[idx]++;
        idx = -1;
      }
    }
  }

  bool have_repeating_digits(const password_t &password) {
    // Two adjacent digits are the same (like 22 in 122345).
    int adjacent_pair_count = 0;
    for (int i = 1; i < 6; i++) {
      if (password[i-1] == password[i]) {
        adjacent_pair_count++;
      }
    }
    return adjacent_pair_count != 0;
  }

  bool has_a_two_digit_group(const password_t &password) {
    // Groupings must be two digits
    char group_value = password[0];
    int group_sz = 1;
    for (int i = 1; i < 6; i++) {
      if (group_value == password[i]) {
        group_sz++;
      } else {
        if (group_sz == 2) return true;
        group_value = password[i];
        group_sz = 1;
      }
    }
    return group_sz == 2;
  }

  bool have_increasing_digits(const password_t &password) {
    // Going from left to right, the digits never decrease; they only ever increase or
    // stay the same (like 111123 or 135679).
    for (int i = 1; i < 6; i++) {
      if (password[i-1] > password[i]) return false;
    }
    return true;
  }

  bool does_password_match(const password_t &password) {
    return have_repeating_digits(password) && have_increasing_digits(password);
  }

  bool does_password_match_2digit_group_only(const password_t &password) {
    return has_a_two_digit_group(password) && have_increasing_digits(password);
  }

  long get_num_password_matches(
      const password_t &start_value, const password_t &end_value,
      const std::function<bool(const password_t &)>& check_fn,
      bool trace = false
  ) {
    long num_matches = 0;
    password_t temp_value;
    strncpy(temp_value, start_value, 6);
    while (strncmp(temp_value, end_value, 6) <= 0) {
      if (trace) std::cout << "Checking " << temp_value;
      if (check_fn(temp_value)) {
        if (trace) std::cout << "\t YES";
        num_matches++;
      }
      if (trace) std::cout << std::endl;
      increment_password(temp_value);
    }
    return num_matches;
  };

  void problem1() {
    assert(get_num_password_matches("111110", "111112", does_password_match, true) == 2);

    std::cout
        << "Result : "
        << get_num_password_matches("235741", "706948", does_password_match)
        << std::endl;
  }

  void problem2() {
    assert(has_a_two_digit_group("112233"));
    assert(!has_a_two_digit_group("123444"));
    assert(has_a_two_digit_group("111122"));

    std::cout
        << "Result : "
        << get_num_password_matches("235741", "706948", does_password_match_2digit_group_only)
        << std::endl;
  }

} // namespace day1
