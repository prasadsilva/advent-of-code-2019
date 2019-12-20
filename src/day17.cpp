#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <numeric>
#include <map>

namespace day17 {

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

  enum position_type_e {
    TYPE_Scaffold = '#',   // #
    TYPE_OpenSpace = '.',  // .
    TYPE_RobotUp = '^',    // ^
    TYPE_RobotRight = '>', // >
    TYPE_RobotDown = 'v',  // v
    TYPE_RobotLeft = '<'   // <
  };

  using position_t = std::pair<unit_t, unit_t>;

  std::vector<position_t> get_intersections(const std::map<position_t, unit_t> &position_types) {
    std::vector<position_t> intersections;
    unit_t x0 = 1, x1 = 0, y0 = 1, y1 = 0;
    for (auto&& [position, type] : position_types) {
      auto&& [x, y] = position;
      if (x < x0) x0 = x;
      if (x > x1) x1 = x;
      if (y < y0) y0 = y;
      if (y > y1) y1 = y;
    }
    for (unit_t y = y0 + 1; y < y1; y++) {
      for (unit_t x = x0 + 1; x < x1; x++) {
        // Check for all 4 neighbors
        if (
            (position_types.at({x, y}) != TYPE_Scaffold) ||
            (position_types.at({x + 1, y}) == TYPE_OpenSpace) ||
            (position_types.at({x - 1, y}) == TYPE_OpenSpace) ||
            (position_types.at({x, y + 1}) == TYPE_OpenSpace) ||
            (position_types.at({x, y - 1}) == TYPE_OpenSpace)
            ) {
          continue;
        }
        intersections.emplace_back(x, y);
      }
    }
    return intersections;
  }

  struct ascii_program_t {
    int_code_program_state_t _program_state;
    std::map<position_t, unit_t> _position_types;

    void render_map() {
      unit_t x0 = 0, x1 = 0, y0 = 0, y1 = 0;
      for (auto&& [position, type] : _position_types) {
        auto&& [x, y] = position;
        if (x < x0) x0 = x;
        if (x > x1) x1 = x;
        if (y < y0) y0 = y;
        if (y > y1) y1 = y;
      }
      for (unit_t y = y0; y <= y1; y++) {
        for (unit_t x = x0; x <= x1; x++) {
          position_t p{x, y};
          if (_position_types.find(p) == _position_types.end()) {
            std::cout << ' ';
          } else {
            auto type = _position_types[p];
            if (type == TYPE_Scaffold) std::cout << '#';
            else if (type == TYPE_OpenSpace) std::cout << '.';
            else if (type == TYPE_RobotUp) std::cout << '^';
            else if (type == TYPE_RobotRight) std::cout << '>';
            else if (type == TYPE_RobotDown) std::cout << 'v';
            else if (type == TYPE_RobotLeft) std::cout << '<';
          }
        }
        std::cout << std::endl;
      }
      std::cout << std::endl << std::endl;
    }

    void run_scan_program(bool trace = false) {
      std::stack<position_t> current_path;
      position_t current_position{0, 0};

      _program_state.run([&]() -> unit_t {
        return -1;
      }, [&](unit_t status) {
        if (trace) std::cout << "Scanned " << current_position.first << ',' << current_position.second << std::endl;
        switch (status) {
          case TYPE_Scaffold:
          case TYPE_OpenSpace:
          case TYPE_RobotUp:
          case TYPE_RobotDown:
          case TYPE_RobotLeft:
          case TYPE_RobotRight: {
            _position_types[current_position] = status;
            current_position.first++;
            break;
          }
          case 10: {
            current_position.first = 0;
            current_position.second++;
            break;
          }
          default: {
            std::cout << "Unknown status : " << status << std::endl;
            assert(0);
          }
        }
      }, [&]() -> bool {
        return false;
      });

      render_map();
    }

    std::string add_comma_separators(const std::vector<std::string> &input) {
      std::string output;
      for (unit_t i = 0; i < input.size(); i++) {
        if (i > 0) output.append(",");
        output.append(input[i]);
      }
      return output;
    }

    unit_t run_work_program(
        const std::vector<std::string> &main_movement_routine,
        const std::vector<std::string> &function_a,
        const std::vector<std::string> &function_b,
        const std::vector<std::string> &function_c,
        bool continuous_video_feed,
        bool trace = false
    ) {
      unit_t output = 0;
      std::vector<std::string> inputs = {
          add_comma_separators(main_movement_routine),
          add_comma_separators(function_a),
          add_comma_separators(function_b),
          add_comma_separators(function_c),
          continuous_video_feed ? "y" : "n"
      };
      unit_t input_mode = 0;
      unit_t input_cursor_idx = 0;

      _program_state._program_code[0] = 2; // Wake up

      _program_state.run([&]() -> unit_t {
        char output = -1;
        if (input_mode < inputs.size()) {
          if (input_cursor_idx < inputs[input_mode].size()) {
            output = inputs[input_mode][input_cursor_idx];
            input_cursor_idx++;
          } else {
            input_mode++;
            input_cursor_idx = 0;
            output = 10;
          }
        }
        if (trace) std::cout << input_mode << "] Input : " << output << "(" << char(output) << ")" << std::endl;
        return output;
      }, [&](unit_t status) {
        switch (status) {
          case TYPE_Scaffold:
          case TYPE_OpenSpace:
          case TYPE_RobotUp:
          case TYPE_RobotDown:
          case TYPE_RobotLeft:
          case TYPE_RobotRight:
          case 10: {
            std::cout << char(status);
            break;
          }
          default: {
            if (status > 1000) {
              output = status;
            } else {
              std::cout << char(status);
            }
          }
        }
      }, [&]() -> bool {
        return false;
      });
      std::cout << std::endl;
      return output;
    }
  };

  void problem1() {
    int_code_program_t code;
    read_data(code, "data/day17/problem2/input.txt");
    ascii_program_t ascii_program;
    ascii_program._program_state.reset(code);

    ascii_program.run_scan_program(true);
    auto intersections = get_intersections(ascii_program._position_types);
    unit_t result = 0;
    for (auto &intersection : intersections) {
      std::cout << "Found intersection at (" << intersection.first << "," << intersection.second << ") = " << (intersection.first * intersection.second) << std::endl;
      result += (intersection.first * intersection.second);
    }
    std::cout << "Result : " << result << std::endl;
  }

  // https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
  void replaceAll(std::string& source, const std::string& from, const std::string& to)
  {
    std::string newString;
    newString.reserve(source.length());  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = source.find(from, lastPos)))
    {
      newString.append(source, lastPos, findPos - lastPos);
      newString += to;
      lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
  }

  void problem2() {
    int_code_program_t code;
    read_data(code, "data/day17/problem2/input.txt");
    ascii_program_t ascii_program;
    ascii_program._program_state.reset(code);

    // Manually traced instructions
    // R 12 L 8 L 4 L 4 L 8 R 6 L 6 R 12 L 8 L 4 L 4 L 8 R 6 L 6 L 8 L 4 R 12 L 6 L 4 R 12 L 8 L 4 L 4 L 8 L 4 R 12 L 6 L 4 R 12 L 8 L 4 L 4 L 8 L 4 R 12 L 6 L 4 L 8 R 6 L 6

//    std::vector<std::string> tests = {
//        "R12L8L4L4L8R6L6R12L8L4L4L8R6L6L8L4R12L6L4R12L8L4L4L8L4R12L6L4R12L6L4R12L8L4L4L8L4R12L6L4L8R6L6",
//    };
//    for (auto &search : tests) {
//      for (unit_t f0 = 1; f0 < 11; f0++) {
//        std::string temp = search;
//        std::string f0str = search.substr(0, f0);
//        replaceAll(temp, f0str, "_");
//        std::cout << temp << std::endl;
//        for (unit_t f1 = 1; f1 < 11; f1++) {
//          auto temp2 = temp;
//          std::string f1str = search.substr(f0, f1);
//          replaceAll(temp2, f1str, "_");
//          std::cout << "\t" << temp2 << std::endl;
//          for (unit_t f2 = 1; f2 < 11; f2++) {
//            auto temp3 = temp2;
//            std::string f2str = search.substr(f1, f2);
//            replaceAll(temp3, f2str, "_");
//            std::cout << "\t\t" << temp3 << std::endl;
//            replaceAll(temp3, "_", "");
//            if (temp3.empty()) {
//              std::cout << "############## FOUND IT" << std::endl;
//              exit(0);
//            }
//          }
//        }
//        std::cout << std::endl;
//      }
//    }
//    exit(0);

/*
    // #### METHOD 2
    //
    // Manually traced
    // R 12  L 8 L 4 L 4 L 8 R 6 L 6 R 12  L 8 L 4 L 4 L 8 R 6 L 6 L 8 L 4 R 12  L 6 L 4 R 12  L 8 L 4 L 4 L 8 L 4 R 12  L 6 L 4 R 12  L 6 L 4 R 12 L 8 L 4 L 4 L 8 L 4 R 12 L 6 L 4 L 8 R 6 L 6
    std::vector<unit_t> instructions = {
        'R', 12, 'L', 8, 'L', 4, 'L', 4, 'L', 8, 'R', 6, 'L', 6, 'R', 12, 'L', 8, 'L', 4, 'L', 4, 'L', 8, 'R', 6, 'L', 6, 'L', 8, 'L', 4, 'R', 12, 'L', 6, 'L', 4, 'R', 12, 'L', 8, 'L', 4, 'L', 4, 'L', 8, 'L', 4, 'R', 12, 'L', 6, 'L', 4, 'R', 12, 'L', 6, 'L', 4, 'R', 12, 'L', 8, 'L', 4, 'L', 4, 'L', 8, 'L', 4, 'R', 12, 'L', 6, 'L', 4, 'L', 8, 'R', 6, 'L', 6
    };
    // Figure out all possible values for each instruction
    std::vector<std::vector<std::vector<char>>> instruction_values;
    for (auto &instruction : instructions) {
      if (instruction == 'R' or instruction == 'L') instruction_values.push_back({{(char)instruction}});
      else if (instruction == 12) {
        instruction_values.push_back({
                                         {'6', '6'},
                                         {'4', '8'}, {'8', '4'},
                                         {'3', '9'}, {'9', '3'},
                                         {'5', '7'}, {'7', '5'}
        });
      }
      else if (instruction == 8){
        instruction_values.push_back({
                                         {'8'},
                                         {'4', '4'},
                                         {'3', '5'}, {'5', '3'},
                                         {'2', '6'}, {'6', '2'},
                                         {'1', '7'}, {'7', '1'}
        });
      }
      else if (instruction == 6){
        instruction_values.push_back({
                                         {'6'},
                                         {'3', '3'},
                                         {'2', '4'}, {'4', '2'},
                                         {'1', '5'}, {'5', '1'}
                                     });
      }
      else if (instruction == 4){
        instruction_values.push_back({
                                         {'4'},
                                         {'2', '2'},
                                         {'1', '3'}, {'3', '1'}
                                     });
      }
      else {
        std::cout << "Unknown instruction! " << instruction << std::endl;
        exit(0);
      }
    }
    // For every combination of all possible values, check if there are 3 patterns that cover the generated instruction
    std::function<void(unit_t, const std::string&)> process_combination = [&](unit_t instruction_idx, const std::string &accum) {
      if (instruction_idx >= instruction_values.size()) {
        std::cout << "Testing " << accum << std::endl;
        for (unit_t f0 = 1; f0 < 11; f0++) {
          std::string temp = accum;
          std::string f0str = accum.substr(0, f0);
          replaceAll(temp, f0str, "_");
//        std::cout << temp << std::endl;
          for (unit_t f1 = 1; f1 < 11; f1++) {
            auto temp2 = temp;
            std::string f1str = accum.substr(f0, f1);
            replaceAll(temp2, f1str, "_");
//          std::cout << temp2 << std::endl;
            for (unit_t f2 = 1; f2 < 11; f2++) {
              auto temp3 = temp2;
              std::string f2str = accum.substr(f1, f2);
              replaceAll(temp3, f2str, "_");
//              std::cout << temp3 << std::endl;
              replaceAll(temp3, "_", "");
              if (temp3.size() == 0) {
                std::cout << "############## FOUND IT" << std::endl;
                exit(0);
              }
            }
          }
        }
//        std::cout << std::endl;
        return;
      }
      for (unit_t i = 0; i < instruction_values[instruction_idx].size(); i++) {
        auto copy = accum;
        copy.append(instruction_values[instruction_idx][i].data(), instruction_values[instruction_idx][i].size());
        process_combination(instruction_idx + 1, copy);
      }
    };
    process_combination(0, "");
*/

    // A - R 12 L 8 L 4 L 4
    // B - L 8 R 6 L 6
    // C - 8 L 4 R 12 L 6 L 4
    //
    // R 12 L 8 L 4 L 4 L 8 R 6 L 6 R 12 L 8 L 4 L 4 L 8 R 6 L 6 L 8 L 4 R 12 L 6 L 4 R 12 L 8 L 4 L 4 L 8 L 4 R 12 L 6 L 4 R 12 L 8 L 4 L 4  L 8 L 4 R 12 L 6 L 4 L 8 R 6 L 6

    std::vector<std::string> main_movement_routine = {"A", "B", "A", "B", "C", "A", "C", "A", "C", "B"};
    std::vector<std::string> function_a = {"R", "12", "L", "8", "L", "4", "L", "4"};
    std::vector<std::string> function_b = {"L", "8", "R", "6", "L", "6"};
    std::vector<std::string> function_c = {"L", "8", "L", "4", "R", "12", "L", "6", "L", "4"};
    auto result = ascii_program.run_work_program(
        main_movement_routine,
        function_a,
        function_b,
        function_c,
        true,
        true
    );
    if (result != 'X') {
      std::cout << "Result : " << result << std::endl;
    } else {
      std::cout << "Went off into space!" << std::endl;
    }
}

} // namespace day1
