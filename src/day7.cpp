#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>

namespace day7 {

  const bool DEEP_TRACE = false;

  using unit_t = int64_t;

  using int_code_program_t = std::vector<unit_t>;
  using input_handler_t = std::function<unit_t(void)>;
  using output_handler_t = std::function<void(unit_t)>;

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
      unit_t _opcode;
      unit_t _param_mode_0;
      unit_t _param_mode_1;
      unit_t _param_mode_2;

      explicit instruction_t(unit_t instruction_value, bool trace = false) {
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

    unit_t read_param_value(unit_t instruction_pointer, unit_t param_idx, unit_t param_mode) {
      if (param_mode == 1) {
        return _program_code[_instruction_pointer + param_idx + 1];
      } else {
        auto address = _program_code[_instruction_pointer + param_idx + 1];
        return _program_code[address];
      }
    }

    // Returns true if output occurred
    bool step(
        const input_handler_t &input_handler,
        const output_handler_t &output_handler,
        bool trace = false
    ) {
      if (trace) std::cout << "\tIP=" << _instruction_pointer;
      instruction_t instruction(_program_code[_instruction_pointer], trace);
      switch (instruction._opcode) {
        case 1: {
          // ADD
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          auto result = val0 + val1;
          auto write_address = read_param_value(_instruction_pointer, 2, 1);
          _program_code[write_address] = result;
          if (trace) std::cout << "\t\tADD: WROTE " << result << " to ADDR " << write_address << std::endl;
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
          if (trace) std::cout << "\t\tMUL: WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 3: {
          // INPUT
          auto write_address = read_param_value(_instruction_pointer, 0, 1);
          auto input = input_handler();
          _program_code[write_address] = input;
          if (trace) std::cout << "\t\tINPUT: WROTE " << input << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 2;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 4: {
          // OUTPUT
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          output_handler(val0);
          _instruction_pointer += 2;
          if (trace) std::cout << "\t\tOUTPUT => " << val0 << std::endl;
          if (trace && DEEP_TRACE) print_program_code();
          return false;
          break;
        }
        case 5: {
          // JUMP IF TRUE
          auto val0 = read_param_value(_instruction_pointer, 0, instruction._param_mode_0);
          auto val1 = read_param_value(_instruction_pointer, 1, instruction._param_mode_1);
          if (val0 != 0) {
            if (trace) std::cout << "\t\tJE: SET IP from " << _instruction_pointer << " to " << val1 << std::endl;
            _instruction_pointer = val1;
          } else {
            if (trace) std::cout << "\t\tJE: NO CHANGE" << std::endl;
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
            if (trace) std::cout << "\t\tJNE: SET IP from " << _instruction_pointer << " to " << val1 << std::endl;
            _instruction_pointer = val1;
          } else {
            if (trace) std::cout << "\t\tJNE: NO CHANGE" << std::endl;
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
          if (trace) std::cout << "\t\tLT: WROTE " << result << " to ADDR " << write_address << std::endl;
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
          if (trace) std::cout << "\t\tEQ: WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 99: {
          // HALT
          if (trace) std::cout << "\t\tHALTED" << std::endl;
          _halted = true;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        default: {
          std::cerr << "ERROR: Unknown opcode (" << instruction._opcode << ") [IP=" << _instruction_pointer << "]"
                    << std::endl;
          assert(0);
        }
      }
      return true;
    }

    void run(const input_handler_t &input_handler, const output_handler_t &output_handler, bool break_on_output, bool trace = false) {
      if (trace) {
        std::cout << "\nRunning program.." << std::endl;
        print_program_code();
      }
      while (!_halted) {
        auto output_occurred = step(input_handler, output_handler, trace);
        if (!output_occurred) return;
      }
    }
  };

  void read_data(std::vector<unit_t> &outdata, const char *filepath, bool trace = false) {
    std::ifstream input_stream(filepath);
    while( input_stream.good() )
    {
      std::string substr;
      getline( input_stream, substr, ',' );
      if (trace) std::cout << "READ: " << substr << std::endl;
      outdata.push_back( std::stol(substr) );
    }
  }

  struct amplifier_t {
    int_code_program_state_t  _program_state;
    unit_t                    _phase_setting = -1;
    unit_t                    _input_count = 0;
    unit_t                    _output = -1;

    unit_t process(unit_t input_value, bool break_on_output, bool trace = false) {
      assert(_phase_setting != -1);
      std::vector<unit_t> inputs = {_phase_setting, input_value};

      _program_state.run(
          [&]() -> unit_t {
            if (trace) std::cout << "Requesting [" << _input_count << "] => ";
            unit_t ret_value = (_input_count == 0) ? _phase_setting : input_value;
            if (trace) std::cout << ret_value << std::endl;
            _input_count++;
            return ret_value;
          },
          [&](unit_t value) { _output = value; },
          break_on_output,
          trace
      );

      return _output;
    }
  };

  struct phase_setting_sequence_t {
  private:
    std::vector<unit_t> _phase_settings;
  public:
    explicit phase_setting_sequence_t(std::vector<unit_t> phase_settings)
      : _phase_settings(std::move(phase_settings)) {}
    constexpr unit_t get_num_phase_settings() const { return _phase_settings.size(); }
    friend std::ostream& operator << (std::ostream& out, const phase_setting_sequence_t &phase_setting_seq) {
      std::cout << "[";
      for (auto phase_setting : phase_setting_seq._phase_settings) std::cout << phase_setting;
      std::cout << "]";
      return out;
    }
    unit_t operator[] (size_t index) const {
      if (index >= _phase_settings.size()) return -1;
      return _phase_settings[index];
    }
    bool next() {
      return std::next_permutation(_phase_settings.begin(), _phase_settings.end());
    }
  };

  unit_t get_thruster_signal(const phase_setting_sequence_t &phase_setting_seq, const int_code_program_t &program, bool trace = false) {
    std::vector<amplifier_t> amplifiers;
    for (unit_t idx = 0; idx < phase_setting_seq.get_num_phase_settings(); idx++) {
      amplifier_t amplifier;
      amplifier._program_state._program_code = program;
      amplifier._phase_setting = phase_setting_seq[idx];
      amplifiers.push_back(amplifier);
    }

    unit_t prev_amplifier_output = 0;
    for (unit_t amplifier_idx = 0; amplifier_idx < amplifiers.size(); amplifier_idx++) {
      if (trace) std::cout << "\n**** AMP " << amplifier_idx << std::endl;
      auto& amplifier = amplifiers[amplifier_idx];
      prev_amplifier_output = amplifier.process(prev_amplifier_output, true, trace);
    }
    return prev_amplifier_output;
  }

  unit_t get_highest_possible_thruster_signal(const int_code_program_t &program, bool trace = false) {
    unit_t max_thruster_signal = -1;
    phase_setting_sequence_t phase_setting_seq{{0, 1, 2, 3, 4}};
    do {
      std::cout << "Testing Phase Seq: " << phase_setting_seq << std::endl;
      auto thruster_signal = get_thruster_signal(phase_setting_seq, program, trace);
      if (thruster_signal > max_thruster_signal) max_thruster_signal = thruster_signal;
    } while (phase_setting_seq.next());
    return max_thruster_signal;
  }

  unit_t get_thruster_signal_mode2(const phase_setting_sequence_t &phase_setting_seq, const int_code_program_t &program, bool trace = false) {
    std::vector<amplifier_t> amplifiers;
    for (unit_t idx = 0; idx < phase_setting_seq.get_num_phase_settings(); idx++) {
      amplifier_t amplifier;
      amplifier._program_state._program_code = program;
      amplifier._phase_setting = phase_setting_seq[idx];
      amplifiers.push_back(amplifier);
    }

    unit_t step = 0;
    unit_t prev_amplifier_output = 0;
    unit_t amplifier_idx = 0;
    bool signal_ok = true;
    while (signal_ok) {
      auto &amplifier = amplifiers[amplifier_idx];
      if (trace) std::cout << "\n[" << step << "] ** AMP " << amplifier_idx;
      prev_amplifier_output = amplifier.process(prev_amplifier_output, true, trace);
      if (trace) std::cout << " (HALTED = " << (amplifier._program_state._halted) << ") OUTPUT: " << prev_amplifier_output << std::endl;
      amplifier_idx++;
      if (amplifier_idx == amplifiers.size()) {
        signal_ok = !amplifier._program_state._halted;
        amplifier_idx = 0;
      }
      step++;
    }

    return prev_amplifier_output;
  }

  unit_t get_highest_possible_thruster_signal_mode2(const int_code_program_t &program, bool trace = false) {
    unit_t max_thruster_signal = -1;
    phase_setting_sequence_t phase_setting_seq{{5, 6, 7, 8, 9}};
    do {
      std::cout << "Testing Phase Seq: " << phase_setting_seq << std::endl;
      auto thruster_signal = get_thruster_signal_mode2(phase_setting_seq, program, trace);
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
    std::cout << "Result : " << get_highest_possible_thruster_signal(program) << std::endl;
  }

  void problem2() {
    {
      int_code_program_t program = {3,26,1001,26,-4,26,3,27,1002,27,2,27,1,27,26,
                                    27,4,27,1001,28,-1,28,1005,28,6,99,0,0,5};
      phase_setting_sequence_t phase_setting_seq({9,8,7,6,5});
      std::cout << "Max thruster signal: " << get_thruster_signal_mode2(phase_setting_seq, program) << std::endl;
    }

    int_code_program_t program;
    read_data(program, "data/day7/problem2/input.txt");
    std::cout << "Result : " << get_highest_possible_thruster_signal_mode2(program) << std::endl;
  }

} // namespace day1
