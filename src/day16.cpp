#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>

namespace day16 {

  using unit_t = int64_t;
  using sequence_t = std::vector<unit_t>;

  void read_data(sequence_t &outdata, const char *filepath) {
    std::ifstream input_stream(filepath);
    char c;
    while(input_stream >> c) {
      outdata.push_back(c - '0');
    }
  }

  struct fft_t {
    sequence_t _input;

    static unit_t get_pattern_value(const sequence_t &pattern, unit_t repeat_count, unit_t lookup_idx) {
      return pattern[(lookup_idx / repeat_count) % pattern.size()];
    }

    friend std::ostream& operator << (std::ostream& out, const fft_t& fft) {
      for (auto input_value : fft._input) {
        out << input_value;
      }
      return out;
    }

    void next(const sequence_t &pattern, bool trace = false) {
      sequence_t output;

      // For each output
      for (auto output_idx = 0; output_idx < _input.size(); output_idx++) {
        unit_t accum = 0;
        unit_t pattern_repeat_count = output_idx + 1;

        for (auto input_idx = 0; input_idx < _input.size(); input_idx++) {
          unit_t pattern_lookup_idx = input_idx + 1;
          auto input = _input[input_idx];
          auto pattern_value = get_pattern_value(pattern, pattern_repeat_count, pattern_lookup_idx);
          if (trace && input_idx != 0) std::cout << " + ";
          if (trace) std::cout << input << '*' << pattern_value;
          accum += (input * pattern_value);
          pattern_lookup_idx++;
        }
        unit_t digit = std::abs(accum) % 10;
        if (trace) std::cout << " = " << digit << std::endl;

        output.push_back(digit); // Store only the last digit
      }

      assert(output.size() == _input.size());
      _input = output;
    }
  };

  void problem1() {
    {
      for (unit_t repeat_count = 1; repeat_count < 9; repeat_count++) {
        std::cout << repeat_count << " => ";
        for (unit_t i = 1; i < 9; i++) {
          std::cout << std::setw(2) << fft_t::get_pattern_value({0, 1, 0, -1}, repeat_count, i) << " ";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
    }
    {
      sequence_t sequence{1,2,3,4,5,6,7,8};
      fft_t fft{sequence};
      for (unit_t i = 0; i < 4; i++) {
        fft.next({0, 1, 0, -1}, true);
        std::cout << "After " << (i + 1) << " phases: " << fft << std::endl << std::endl;
      }
    }

    fft_t fft;
    read_data(fft._input, "data/day16/problem1/input.txt");
    for (unit_t i = 0; i < 100; i++) {
      fft.next({0, 1, 0, -1});
    }
    std::cout << "Result : ";
    for (unit_t i = 0; i < 8; i++) {
      std::cout << fft._input[i];
    }
    std::cout << std::endl;
  }

  void problem2() {
    // NOTEThe code below will take a while
    {
//      sequence_t input;
//      read_data(input, "data/day16/problem1/input.txt");
//      unit_t copies = 10;
//      for (unit_t copy_idx = 0; copy_idx < copies; copy_idx++) {
//        fft_t fft;
//        for (unit_t i = 0; i < copy_idx + 1; i++) {
//          fft._input.insert(fft._input.end(), input.begin(), input.end());
//        }
//        std::cout << "Real input: " << fft._input.size() << " digits" << std::endl;
//        for (unit_t i = 0; i < 100; i++) {
//          fft.next({0, 1, 0, -1});
//          std::cout << "Result : ";
//          for (unit_t offset_idx = 0; offset_idx <= copy_idx; offset_idx++) {
//            for (unit_t i = 0; i < 8; i++) {
//              std::cout << fft._input[i + (offset_idx * input.size())];
//            }
//            std::cout << " ";
//          }
//          std::cout << std::endl;
//        }
//      }
    }
    // Output:
    // Real input: 650 digits
    // Result : 88323090
    // Real input: 1300 digits
    // Result : 40087651 32359658
    // Real input: 1950 digits
    // Result : 51433420 77254713 32359658
    // Real input: 2600 digits
    // Result : 74688369 50549634 01764174 32359658
    // Real input: 3250 digits
    // Result : 38426032 68225863 91524232 01764174 32359658
    // Real input: 3900 digits
    // Result : 20020428 11259719 14916630 25034693 01764174 32359658
    // Real input: 4550 digits
    // Result : 48404919 43452760 22293126 50361868 25034693 01764174 32359658
    // Real input: 5200 digits
    // Result : 89824428 16364723 81777380 85190961 84871229 25034693 01764174 32359658
    // Real input: 5850 digits
    // Result : 86423324 58729912 46251237 57039645 21545199 84871229 25034693 01764174 32359658
    // Real input: 6500 digits
    // Result : 24815166 98259876 23159603 01576393 17944014 55055550 84871229 25034693 01764174 32359658
    //
    // Notice trailing digits are duplicated regardless of number of copies
    //
    // Figure out trailing position
    //    (650 * 10000) - 5979187 = 520813
    //    520813 / 650 = 801 copy offset
    //    520813 % 650 = 151
    //

    sequence_t input;
    read_data(input, "data/day16/problem1/input.txt");
    fft_t fft;
    for (unit_t i = 0; i < 1600 + 1; i++) {
      fft._input.insert(fft._input.end(), input.begin(), input.end());
    }
    std::cout << "Real input: " << fft._input.size() << " digits" << std::endl;

    for (unit_t i = 0; i < 100; i++) {
      std::cout << "Step: " << i << std::endl;
      fft.next({0, 1, 0, -1});
    }
    std::cout << "Result : ";
    for (unit_t i = 0; i < 8; i++) {
      std::cout << fft._input[(fft._input.size() - 520813) + i];
    }
    std::cout << std::endl;
  }

} // namespace day1
