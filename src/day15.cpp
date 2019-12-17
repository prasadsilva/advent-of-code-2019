#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <queue>
#include <map>

namespace day15 {

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

  enum movement_command_e {
    MOVE_Invalid = 0,
    MOVE_North = 1,
    MOVE_South = 2,
    MOVE_West = 3,
    MOVE_East = 4
  };

  enum status_code_e {
    STATUS_Init = -1,
    STATUS_HitWall = 0,
    STATUS_Moved = 1,
    STATUS_MovedToOxygen = 2
  };

  enum position_type_e {
    TYPE_Wall = 0,
    TYPE_Moveable = 1,
    TYPE_Oxygen = 2,
  };

  using position_t = std::pair<unit_t, unit_t>;

  struct remote_control_t {
    int_code_program_state_t _program_state;
    std::map<position_t, unit_t> _position_types;
    position_t oxygen_position{-1, -1};
    unit_t run_limit = 3000; //std::numeric_limits<unit_t>::max();

    void render_map(const position_t &drone_position) {
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
          if (x == 0 && y == 0 ) {
            std::cout << 'S';
          } else if (drone_position == p) {
            std::cout << 'D';
          } else {
            if (_position_types.find(p) == _position_types.end()) {
              std::cout << ' ';
            } else {
              auto type = _position_types[p];
              if (type == TYPE_Wall) std::cout << '#';
              else if (type == TYPE_Moveable) std::cout << '.';
              else if (type == TYPE_Oxygen) std::cout << 'O';
            }
          }
        }
        std::cout << std::endl;
      }
      std::cout << std::endl << std::endl;
    }

    unit_t get_next_movement_command(const position_t &position) {
      // N, E, W, S
      auto [x, y] = position;
      if (_position_types.find({x, y - 1}) == _position_types.end()) {
        return MOVE_North;
      } else if (_position_types.find({x + 1, y}) == _position_types.end()) {
        return MOVE_East;
      } else if (_position_types.find({x, y + 1}) == _position_types.end()) {
        return MOVE_South;
      } else if (_position_types.find({x - 1, y}) == _position_types.end()) {
        return MOVE_West;
      }
      // We've explored all neighbors
      return MOVE_Invalid;
    }

    static unit_t get_position_movement_command(const position_t &source, const position_t &dest) {
      auto [x0, y0] = source;
      auto [x1, y1] = dest;
      auto xdelta = x1 - x0, ydelta = y1 - y0;
      if (xdelta < 0) return MOVE_West;
      if (xdelta > 0) return MOVE_East;
      if (ydelta < 0) return MOVE_North;
      if (ydelta > 0) return MOVE_South;
      assert(0);
      return MOVE_Invalid;
    }

    unit_t run_program(bool bail_when_oxygen_found = true, bool trace = false) {
      std::stack<position_t> current_path;
      position_t current_position{0, 0};
      unit_t next_movement_command = MOVE_North;
      unit_t _last_status = STATUS_Init;
      bool done = false;
      bool backtracking = false;
      unit_t moves = 0;

      // Initial position
      _position_types[current_position] = TYPE_Moveable;
      if (trace) std::cout << "Can move to " << current_position.first << "," << current_position.second << std::endl;

      _program_state.run([&]() -> unit_t {
        if (trace) {
          std::cout << "MOVES " << ++moves << std::endl;
          std::cout << "At (" << current_position.first << "," << current_position.second << ") - ";
          switch (next_movement_command) {
            case MOVE_North: std::cout << "Moving NORTH"; break;
            case MOVE_East: std::cout << "Moving EAST"; break;
            case MOVE_South: std::cout << "Moving SOUTH"; break;
            case MOVE_West: std::cout << "Moving WEST"; break;
            default: std::cout << "UNKNOWN";
          }
          std::cout << std::endl;
        }
        return next_movement_command;
      }, [&](unit_t status) {
        // Report on status
        auto [x, y] = current_position;
        switch (next_movement_command) {
          case MOVE_North: y--; break;
          case MOVE_East: x++; break;
          case MOVE_South: y++; break;
          case MOVE_West: x--; break;
          default: assert(0);
        }
        position_t next_position = {x, y};

        switch (status) {
          case STATUS_HitWall: {
            _position_types[next_position] = TYPE_Wall;
            if (trace) std::cout << "\tCannot move to " << next_position.first << "," << next_position.second << std::endl;
            break; // Do nothing
          }
          case STATUS_MovedToOxygen:
          case STATUS_Moved: {
            if (status == STATUS_MovedToOxygen) {
              done = bail_when_oxygen_found;
              oxygen_position = next_position;
              _position_types[next_position] = TYPE_Oxygen;
            } else {
              _position_types[next_position] = TYPE_Moveable;
            }
            if (!backtracking) {
              if (trace) std::cout << "\tCan move to " << next_position.first << "," << next_position.second << std::endl;
              current_path.push(current_position);
            } else {
              if (trace) std::cout << "\tBacktracking to " << next_position.first << "," << next_position.second << std::endl;
            }
            backtracking = false;
            current_position = next_position;
            break;
          }
        }
        next_movement_command = get_next_movement_command(current_position);
        if (next_movement_command == MOVE_Invalid) {
          // Backtrack
          if (trace) std::cout << "\tBacktracking.." << std::endl;
          done = current_path.empty();
          if (!done) {
            next_movement_command = get_position_movement_command(current_position, current_path.top());
            current_path.pop();
            backtracking = true;
          }
        }
        _last_status = status;
        render_map(current_position);
      }, [&]() -> bool {
        return done || moves >= run_limit;
      });

      if (trace) std::cout << "Found oxygen at " << oxygen_position.first << "," << oxygen_position.second << std::endl;
      return current_path.size();
    }
  };

  unit_t get_duration_for_oxygen_dissipation(const position_t &initial_position, const std::map<position_t, unit_t> &position_types) {
    auto position_map = position_types;
    unit_t elapsed_minutes = 0;
    std::queue<position_t> process_queue, next_process_queue;
    next_process_queue.push(initial_position);
    while (!next_process_queue.empty()) {
      process_queue = next_process_queue;
      next_process_queue = {};

      while (!process_queue.empty()) {
        auto&& [x, y] = process_queue.front();
        process_queue.pop();
        // Check 4 neighbors
        std::vector<position_t> neighbors = {{ x - 1, y }, { x + 1, y }, { x, y - 1 }, { x, y + 1 }};
        for (auto &neighbor : neighbors) {
          if (position_map[neighbor] == TYPE_Moveable) {
            position_map[neighbor] = TYPE_Oxygen;
            next_process_queue.push(neighbor);
          }
        }
      }

      if (!next_process_queue.empty()) elapsed_minutes++;
    }
    return elapsed_minutes;
  }

  void problem1() {
    remote_control_t remote_control;
    read_data(remote_control._program_state._program_code, "data/day15/problem1/input.txt");
    std::cout << "Result : " << remote_control.run_program() << std::endl;
  }

  void problem2() {
    remote_control_t remote_control;
    read_data(remote_control._program_state._program_code, "data/day15/problem2/input.txt");
    remote_control.run_program(false);
    std::cout << "Result : " << get_duration_for_oxygen_dissipation(remote_control.oxygen_position, remote_control._position_types) << std::endl;
  }

} // namespace day1
