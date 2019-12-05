#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>

namespace day5 {

  const bool DEEP_TRACE = false;

  using int_code_program_t = std::vector<long>;
  using output_handler_t = std::function<void(long)>;

  struct int_code_program_state_t {
    int_code_program_t  _program_code;
    int                 _instruction_pointer = 0;
    bool                _halted = false;

    long                _input = -1;

    void reset(const int_code_program_t &program_code) {
      _program_code = program_code;
      _instruction_pointer = 0;
      _halted = false;
    }

    void initialize(int noun, int verb) {
      _program_code[1] = noun;
      _program_code[2] = verb;
    }

    void set_1202_program_alarm() {
      initialize(12, 2);
    }

    void set_input(long input) {
      _input = input;
    }

    void print_program_code() {
      std::cout << "> ";
      for (auto value : _program_code) {
        std::cout << value << ",";
      }
      std::cout << std::endl;
    }

    struct instruction_t {
      long  _opcode;
      long  _param_mode_0;
      long  _param_mode_1;
      long  _param_mode_2;

      explicit instruction_t(long instruction_value, bool trace = false) {
        /*
          ABCDE
           1002

          DE - two-digit _opcode,      02 == _opcode 2
           C - mode of 1st parameter,  0 == position mode
           B - mode of 2nd parameter,  1 == immediate mode
           A - mode of 3rd parameter,  0 == position mode,
                                            omitted due to being a leading zero
         */
        _opcode = (instruction_value % 100);
        _param_mode_0 = (instruction_value / 100) % 10;
        _param_mode_1 = (instruction_value / 1000) % 10;
        _param_mode_2 = (instruction_value / 10000) % 10;
        if (trace) std::cout << "\tOPCODE: " << _opcode << " [" << _param_mode_0 << "," << _param_mode_1 << "," << _param_mode_2 << "]" << std::endl;
      }
    };

    long read_param_value(long instruction_pointer, long param_idx, long param_mode) {
      if (param_mode == 1) {
        return _program_code[_instruction_pointer + param_idx + 1];
      } else {
        auto address = _program_code[_instruction_pointer + param_idx + 1];
        return _program_code[address];
      }
    }

    void step(const output_handler_t& output_handler, bool trace = false) {
      instruction_t instruction(_program_code[_instruction_pointer], trace);
      switch (instruction._opcode) {
        case 1: {
          // ADD
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          auto result = val0 + val1;
          auto write_address = read_param_value(_instruction_pointer, 2, 1);
          _program_code[write_address] = result;
          if (trace) std::cout << "WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 2: {
          // MUL
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          auto result = val0 * val1;
          auto write_address = read_param_value(_instruction_pointer, 2, 1);
          _program_code[write_address] = result;
          if (trace) std::cout << "WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 3: {
          // INPUT
          auto write_address = read_param_value(_instruction_pointer, 0, 1);
          _program_code[write_address] = _input;
          if (trace) std::cout << "WROTE " << _input << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 2;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 4: {
          // OUTPUT
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          output_handler(val0);
          _instruction_pointer += 2;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 5: {
          // JUMP IF TRUE
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          if (val0 != 0) {
            if (trace) std::cout << "SET IP from " << _instruction_pointer << " to " << val1 << std::endl;
            _instruction_pointer = val1;
          } else {
            _instruction_pointer += 3;
          }
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 6: {
          // JUMP IF FALSE
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          if (val0 == 0) {
            if (trace) std::cout << "SET IP from " << _instruction_pointer << " to " << val1 << std::endl;
            _instruction_pointer = val1;
          } else {
            _instruction_pointer += 3;
          }
//          if (trace) print_program_code();
          break;
        }
        case 7: {
          // LESS THAN
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          auto result = (val0 < val1) ? 1 : 0;
          auto write_address = read_param_value(_instruction_pointer, 2, 1);
          _program_code[write_address] = result;
          if (trace) std::cout << "WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 8: {
          // EQUALS
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          auto result = (val0 == val1) ? 1 : 0;
          auto write_address = read_param_value(_instruction_pointer, 2, 1);
          _program_code[write_address] = result;
          if (trace) std::cout << "WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 99: {
          // HALT
          _halted = true;
          if (trace && DEEP_TRACE) print_program_code();
          return;
        }
        default: {
          std::cerr << "ERROR: Unknown opcode (" << instruction._opcode << ") [IP=" << _instruction_pointer << "]" << std::endl;
          assert(0);
        }
      }
    }

    void run(const output_handler_t& output_handler, bool trace = false) {
      if (trace) {
        std::cout << "\nRunning program.." << std::endl;
        print_program_code();
      }
      while (!_halted) {
        step(output_handler, trace);
      }
    }

  };

  void read_data(std::vector<long> &outdata, const char *filepath, bool trace = false) {
    std::ifstream input_stream(filepath);
    while( input_stream.good() )
    {
      std::string substr;
      getline( input_stream, substr, ',' );
      if (trace) std::cout << "READ: " << substr << std::endl;
      outdata.push_back( std::stol(substr) );
    }
  }

  void problem1() {
    int_code_program_state_t program_state;
    read_data(program_state._program_code, "data/day5/problem1/input.txt");
    program_state.set_input(1);
    program_state.run([](long value) {
      std::cout << "OUTPUT: " << value << std::endl;
    });
  }

  void problem2() {
    int_code_program_state_t program_state;
    read_data(program_state._program_code, "data/day5/problem2/input.txt");
    program_state.set_input(5);
    program_state.run([](long value) {
      std::cout << "OUTPUT: " << value << std::endl;
    });
  }

} // namespace day1
