#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <array>

namespace day7 {

  const bool DEEP_TRACE = false;

  using int_code_program_t = std::vector<long>;
  using input_handler_t = std::function<long(void)>;
  using output_handler_t = std::function<void(long)>;

  struct int_code_program_state_t {
    int_code_program_t _program_code;
    int _instruction_pointer = 0;
    bool _halted = false;

    void reset(const int_code_program_t &program_code) {
      _program_code = program_code;
      _instruction_pointer = 0;
      _halted = false;
    }

    void print_program_code() {
      std::cout << "> ";
      for (auto value : _program_code) {
        std::cout << value << ",";
      }
      std::cout << std::endl;
    }

    struct instruction_t {
      long _opcode;
      long _param_mode_0;
      long _param_mode_1;
      long _param_mode_2;

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
        if (trace)
          std::cout << "\tOPCODE: " << _opcode << " [" << _param_mode_0 << "," << _param_mode_1 << "," << _param_mode_2
                    << "]" << std::endl;
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

    void step(
        const input_handler_t &input_handler,
        const output_handler_t &output_handler,
        bool trace = false
    ) {
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
          auto input = input_handler();
          _program_code[write_address] = input;
          if (trace) std::cout << "WROTE " << input << " to ADDR " << write_address << std::endl;
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
          std::cerr << "ERROR: Unknown opcode (" << instruction._opcode << ") [IP=" << _instruction_pointer << "]"
                    << std::endl;
          assert(0);
        }
      }
    }

    void run(const input_handler_t &input_handler, const output_handler_t &output_handler, bool trace = false) {
      if (trace) {
        std::cout << "\nRunning program.." << std::endl;
        print_program_code();
      }
      while (!_halted) {
        step(input_handler, output_handler, trace);
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

  long get_amplifier_output(long phase_setting, long prev_amplifier_output, const int_code_program_t &program, bool trace = false) {
    std::vector<long> input = {phase_setting, prev_amplifier_output};
    long input_idx = 0;
    long output = -1;

    int_code_program_state_t program_state;
    program_state._program_code = program;
    program_state.run(
        [&]() -> long {
          if (trace) std::cout << "Requesting " << input_idx << std::endl;
          assert(input_idx < input.size());
          return input[input_idx++];
        },
        [&](long value) { output = value; },
        trace
    );

    return output;
  }

  constexpr long num_amplifiers = 5;

  // NOT IMPLEMENTED: Check whether each phase setting is used exactly once
  struct phase_setting_sequence_t {
  private:
    std::vector<long> _phase_settings;
  public:
    explicit phase_setting_sequence_t(long num_phase_settings) {
      for (long idx = 0; idx < num_phase_settings; idx++) _phase_settings.push_back(idx);
    }
    explicit phase_setting_sequence_t(const std::vector<long> &phase_settings) : _phase_settings(phase_settings) {}
    friend std::ostream& operator << (std::ostream& out, const phase_setting_sequence_t &phase_setting_seq) {
      std::cout << "[";
      for (auto phase_setting : phase_setting_seq._phase_settings) std::cout << phase_setting;
      std::cout << "]";
      return out;
    }
    long operator[] (size_t index) const {
      if (index >= _phase_settings.size()) return -1;
      return _phase_settings[index];
    }
    bool next() {
      return std::next_permutation(_phase_settings.begin(), _phase_settings.end());
    }
  };

  long get_thruster_signal(const phase_setting_sequence_t &phase_setting_seq, const int_code_program_t &program, bool trace = false) {
    long prev_amplifier_output = 0;
    for (long phase_setting_idx = 0; phase_setting_idx < num_amplifiers; phase_setting_idx++) {
      prev_amplifier_output = get_amplifier_output(phase_setting_seq[phase_setting_idx], prev_amplifier_output, program, trace);
    }
    return prev_amplifier_output;
  }

  long get_highest_possible_thruster_signal(const int_code_program_t &program, bool trace = false) {
    long max_thruster_signal = -1;
    phase_setting_sequence_t phase_setting_seq{num_amplifiers};
    do {
      std::cout << "Testing Phase Seq: " << phase_setting_seq << std::endl;
      auto thruster_signal = get_thruster_signal(phase_setting_seq, program, trace);
      if (thruster_signal > max_thruster_signal) max_thruster_signal = thruster_signal;
    } while (phase_setting_seq.next());
    return max_thruster_signal;
  }

  void problem1() {
    {
      int_code_program_t program = {3,15,3,16,1002,16,10,16,1,16,15,15,4,15,99,0,0};
      phase_setting_sequence_t phase_setting_seq({4,3,2,1,0});
      std::cout << "Max thruster signal: " << get_thruster_signal(phase_setting_seq, program) << std::endl;
    }

    {
      int_code_program_t program = {3,23,3,24,1002,24,10,24,1002,23,-1,23,
                                    101,5,23,23,1,24,23,23,4,23,99,0,0};
      phase_setting_sequence_t phase_setting_seq({0,1,2,3,4});
      std::cout << "Max thruster signal: " << get_thruster_signal(phase_setting_seq, program) << std::endl;
    }

    int_code_program_t program;
    read_data(program, "data/day7/problem1/input.txt");
    std::cout << "Result : " << get_highest_possible_thruster_signal(program, false) << std::endl;
  }

  void problem2() {
//    assert(get_fuel_required_recursive(14) == 2);

//    std::vector<int> input;
//    read_data(input, "data/day1/problem2/input.txt");
//    std::cout << "Result : " << get_total_fuel_required(input, get_fuel_required_recursive) << std::endl;
  }

} // namespace day1
