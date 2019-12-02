#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <numeric>

namespace day2 {

  using int_code_program_t = std::vector<long>;

  struct int_code_program_state_t {
    int_code_program_t  program_code;
    int                 instruction_pointer = 0;
    bool                halted = false;

    void reset(const int_code_program_t &code) {
      program_code = code;
      instruction_pointer = 0;
      halted = false;
    }

    void initialize(int noun, int verb) {
      program_code[1] = noun;
      program_code[2] = verb;
    }

    void set_1202_program_alarm() {
      initialize(12, 2);
    }

    void print_program_code() {
      std::cout << "> ";
      for (auto value : program_code) {
        std::cout << value << ",";
      }
      std::cout << std::endl;
    }

    void step(bool trace) {
      auto opcode = program_code[instruction_pointer];
      switch (opcode) {
        case 1: {
          // ADD
          auto val1_idx = program_code[instruction_pointer + 1];
          auto val1 = program_code[val1_idx];
          auto val2_idx = program_code[instruction_pointer + 2];
          auto val2 = program_code[val2_idx];
          auto result = val1 + val2;
          auto write_idx = program_code[instruction_pointer + 3];
          program_code[write_idx] = result;
          instruction_pointer += 4;
          if (trace) print_program_code();
          break;
        }
        case 2: {
          // MUL
          auto val1_idx = program_code[instruction_pointer + 1];
          auto val1 = program_code[val1_idx];
          auto val2_idx = program_code[instruction_pointer + 2];
          auto val2 = program_code[val2_idx];
          auto result = val1 * val2;
          auto write_idx = program_code[instruction_pointer + 3];
          program_code[write_idx] = result;
          instruction_pointer += 4;
          if (trace) print_program_code();
          break;
        }
        case 99: {
          // HALT
          halted = true;
          if (trace) print_program_code();
          return;
        }
        default: {
          assert(0);
        }
      }
    }

    void run(bool trace = false) {
      if (trace) {
        std::cout << "\nRunning program.." << std::endl;
        print_program_code();
      }
      while (!halted) {
        step(trace);
      }
    }

  };

  void read_day2_data(std::vector<long> &outdata, const char *filepath) {
    std::ifstream input_stream(filepath);
    while( input_stream.good() )
    {
      std::string substr;
      getline( input_stream, substr, ',' );
      outdata.push_back( std::stol(substr) );
    }
  }

  void problem1() {
    std::cout << "Day 2 - Problem 1" << std::endl;
#if !defined(ONLY_ACTIVATE) || ONLY_ACTIVATE == 1

    {
      int_code_program_state_t test{{{1,9,10,3,2,3,11,0,99,30,40,50}}};
      test.run();
      assert(test.program_code[0] == 3500);
    }

    {
      int_code_program_state_t test{{{1,0,0,0,99}}};
      test.run();
      assert(test.program_code[0] == 2);
    }

    {
      int_code_program_state_t test{{{2,3,0,3,99}}};
      test.run();
      assert(test.program_code[3] == 6);
    }

    {
      int_code_program_state_t test{{{2,4,4,5,99,0}}};
      test.run();
      assert(test.program_code[5] == 9801);
    }

    {
      int_code_program_state_t test{{{1,1,1,4,99,5,6,0,99}}};
      test.run();
      assert(test.program_code[0] == 30);
    }

    int_code_program_state_t program_state;
    read_day2_data(program_state.program_code, "data/day2/problem1/input.txt");
    program_state.set_1202_program_alarm();
    program_state.run(true);
    std::cout << "Result : " << program_state.program_code[0] << std::endl;

#endif
  }

  void problem2() {
    std::cout << "Day 2 - Problem 2" << std::endl;
#if !defined(ONLY_ACTIVATE) || ONLY_ACTIVATE == 1

    int_code_program_t program_code;
    read_day2_data(program_code, "data/day2/problem2/input.txt");
    int_code_program_state_t program_state;
    for (int noun = 0; noun <= 99; noun++) {
      for (int verb = 0; verb <= 99; verb++) {
        program_state.reset(program_code);
        program_state.initialize(noun, verb);
        program_state.run();
        if (program_state.program_code[0] == 19690720) {
          std::cout << "Result: " << (100 * noun + verb) << std::endl;
          return;
        }
      }
    }
    std::cout << "Could not find value at address 0!" << std::endl;

#endif
  }

}
