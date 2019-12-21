#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <set>

namespace day19 {

  const bool DEEP_TRACE = false;

  using unit_t = int64_t;

  using int_code_program_t = std::vector<unit_t>;
  using input_handler_t = std::function<unit_t(void)>;
  using output_handler_t = std::function<void(unit_t)>;
  using exit_handler_t = std::function<bool(void)>;

  struct int_code_program_state_t {
    int_code_program_t _program_code;
    unit_t _instruction_pointer = 0;
    unit_t _relative_base_pointer = 0;
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

    unit_t read_param_value(unit_t param_idx, unit_t mode) {
      auto address = 0;
      // Compute address
      switch (mode) {
        case 0: {
          // Position mode
          address = _program_code[_instruction_pointer + param_idx + 1];
          break;
        }
        case 1: {
          // Immediate mode
          address = _instruction_pointer + param_idx + 1;
          break;
        }
        case 2: {
          // Relative mode
          address = _relative_base_pointer + _program_code[_instruction_pointer + param_idx + 1];
          break;
        }
        default: assert(0);
      }
      // Handle out of bounds
      if (address >= _program_code.size()) _program_code.resize(address + 1, 0);
      return _program_code[address];
    }

    unit_t write_value2(unit_t param_idx, unit_t mode, unit_t value) {
      unit_t location = read_param_value(param_idx, 1);  // Read value
      unit_t address = 0;
      switch (mode) {
        case 0:
        case 1: {
          address = location;
          break;
        }
        case 2: {
          // Relative mode
          address = _relative_base_pointer + location;
          break;
        }
        default: assert(0);
      }
      // Handle out of bounds
      if (address >= _program_code.size()) _program_code.resize(address + 1, 0);
      _program_code[address] = value;

      return address;
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
          auto val0 = read_param_value(0, instruction._param_mode_0);
          auto val1 = read_param_value(1, instruction._param_mode_1);
          auto result = val0 + val1;
          auto write_address = write_value2(2, instruction._param_mode_2, result);
          if (trace) std::cout << "\t\tADD: WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 2: {
          // MUL
          auto val0 = read_param_value(0, instruction._param_mode_0);
          auto val1 = read_param_value(1, instruction._param_mode_1);
          auto result = val0 * val1;
          auto write_address = write_value2(2, instruction._param_mode_2, result);
          if (trace) std::cout << "\t\tMUL: WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 3: {
          // INPUT
          auto input = input_handler();
          auto write_address = write_value2(0, instruction._param_mode_0, input);
          if (trace) std::cout << "\t\tINPUT: WROTE " << input << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 2;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 4: {
          // OUTPUT
          auto val0 = read_param_value(0, instruction._param_mode_0);
          output_handler(val0);
          _instruction_pointer += 2;
          if (trace) std::cout << "\t\tOUTPUT => " << val0 << std::endl;
          if (trace && DEEP_TRACE) print_program_code();
          return true;
          break;
        }
        case 5: {
          // JUMP IF TRUE
          auto val0 = read_param_value(0, instruction._param_mode_0);
          auto val1 = read_param_value(1, instruction._param_mode_1);
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
          auto val0 = read_param_value(0, instruction._param_mode_0);
          auto val1 = read_param_value(1, instruction._param_mode_1);
          if (val0 == 0) {
            if (trace) std::cout << "\t\tJNE: SET IP from " << _instruction_pointer << " to " << val1 << std::endl;
            _instruction_pointer = val1;
          } else {
            if (trace) std::cout << "\t\tJNE: NO CHANGE" << std::endl;
            _instruction_pointer += 3;
          }
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 7: {
          // LESS THAN
          auto val0 = read_param_value(0, instruction._param_mode_0);
          auto val1 = read_param_value(1, instruction._param_mode_1);
          auto result = (val0 < val1) ? 1 : 0;
          auto write_address = write_value2(2, instruction._param_mode_2, result);
          if (trace) std::cout << "\t\tLT: WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 8: {
          // EQUALS
          auto val0 = read_param_value(0, instruction._param_mode_0);
          auto val1 = read_param_value(1, instruction._param_mode_1);
          auto result = (val0 == val1) ? 1 : 0;
          auto write_address = write_value2(2, instruction._param_mode_2, result);
          if (trace) std::cout << "\t\tEQ: WROTE " << result << " to ADDR " << write_address << std::endl;
          _instruction_pointer += 4;
          if (trace && DEEP_TRACE) print_program_code();
          break;
        }
        case 9: {
          // ADJ RELBASE
          auto val0 = read_param_value(0, instruction._param_mode_0);
          _relative_base_pointer = _relative_base_pointer + val0;
          if (trace) std::cout << "\t\tADJ RELBASE: CHANGED TO " << _relative_base_pointer << std::endl;
          _instruction_pointer += 2;
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
      return false;
    }

    void run(
        const input_handler_t &input_handler,
        const output_handler_t &output_handler,
        const exit_handler_t &exit_handler,
        bool trace = false
    ) {
      if (trace) {
        std::cout << "\nRunning program.." << std::endl;
        print_program_code();
      }
      while (!_halted && !exit_handler()) {
        step(input_handler, output_handler, trace);
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

  using point_t = std::pair<unit_t, unit_t>;

  struct drone_t {

    unit_t check_point(int_code_program_t &code, unit_t x, unit_t y) {
      int_code_program_state_t _program_state;
      unit_t output = 0;
      std::vector<unit_t> input;
      unit_t input_cursor_idx = 0;
      input.push_back(x);
      input.push_back(y);

      _program_state.reset(code);
      _program_state.run([&]() -> unit_t {
        unit_t value = -1;
        if (input_cursor_idx < input.size()) {
          value = input[input_cursor_idx++];
        }
        return value;
      }, [&](unit_t status) {
        output = status;
      }, [&]() -> bool {
        return false;
      });
      return output;
    }

    unit_t find_num_points_affected(int_code_program_t &code, unit_t width, unit_t height, bool trace = false) {
      unit_t affected_points = 0;

      for (unit_t y = 0; y < height; y++) {
        for (unit_t x = 0; x < width; x++) {
          auto status = check_point(code, x, y);
          if (trace) std::cout << (status ? '#' : '.');
          affected_points += status;
        }
        if (trace) std::cout << std::endl;
      }

      return affected_points;
    }

    point_t find_point_where_ship_fits(int_code_program_t &code, unit_t ship_width, unit_t ship_height, bool trace = false) {
      std::vector<point_t> ordered_affected_points;
      std::set<point_t> affected_points_lookup;
      unit_t check_width = 3000, check_height = 3000;

      std::cout << "Gather data.." << std::endl;
      for (unit_t y = 0; y < check_height; y++) {
        for (unit_t x = 0; x < check_width; x++) {
          auto status = check_point(code, x, y);
          if (trace) std::cout << (status ? '#' : '.');
          if (status) {
            ordered_affected_points.emplace_back(x, y);
            affected_points_lookup.insert({x, y});
          }
        }
        if (trace) std::cout << std::endl;
      }

      std::cout << "Checking data.." << std::endl;
      // For every affected point (points are ordered properly)
      for (auto&& [affected_x, affected_y] : ordered_affected_points) {
        if (trace) std::cout << "Checking " << affected_x << "," << affected_y << std::endl;
        bool ok = true;
        // Perform scan of perimeter
        for (unit_t x = 0; x < ship_width && ok; x++) {
          point_t pt = {affected_x + x, affected_y};
          if (trace) std::cout << "\t" << pt.first << "," << pt.second;
          ok = ok && (affected_points_lookup.find(pt) != affected_points_lookup.end());
          if (trace) std::cout << " " << ok << std::endl;
        }
        for (unit_t x = 0; x < ship_width && ok; x++) {
          point_t pt = {affected_x + x, affected_y + ship_height - 1};
          if (trace) std::cout << "\t" << pt.first << "," << pt.second;
          ok = ok && (affected_points_lookup.find(pt) != affected_points_lookup.end());
          if (trace) std::cout << " " << ok << std::endl;
        }
        for (unit_t y = 0; y < ship_height && ok; y++) {
          point_t pt = {affected_x, affected_y + y};
          if (trace) std::cout << "\t" << pt.first << "," << pt.second;
          ok = ok && (affected_points_lookup.find(pt) != affected_points_lookup.end());
          if (trace) std::cout << " " << ok << std::endl;
        }
        for (unit_t y = 0; y < ship_height && ok; y++) {
          point_t pt = {affected_x + ship_width - 1, affected_y + y};
          if (trace) std::cout << "\t" << pt.first << "," << pt.second;
          ok = ok && (affected_points_lookup.find(pt) != affected_points_lookup.end());
          if (trace) std::cout << " " << ok << std::endl;
        }
        if (ok) {
          return {affected_x, affected_y};
        }
      }

      return { -1, -1 };
    }
  };

  void problem1() {
    int_code_program_t code;
    read_data(code, "data/day19/problem1/input.txt");
    drone_t drone;
    unit_t num_points_affected = drone.find_num_points_affected(code, 50, 50, true);
    std::cout << "Result : " << num_points_affected << std::endl;
  }

  void problem2() {
    int_code_program_t code;
    read_data(code, "data/day19/problem2/input.txt");
    drone_t drone;
    point_t pt = drone.find_point_where_ship_fits(code, 100, 100, false);
    std::cout << "Result : " << (pt.first * 10000 + pt.second) << std::endl;
  }

} // namespace day1
