#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <map>
#include <queue>
#include <thread>

namespace day13 {

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

  enum tile_type_e {
    TILE_EMPTY = 0,
    TILE_WALL = 1,
    TILE_BLOCK = 2,
    TILE_HORIZ_PADDLE = 3,
    TILE_BALL = 4,
  };

  enum joystick_state_t {
    JOY_NEUTRAL = 0,
    JOY_LEFT = -1,
    JOY_RIGHT = 1,
  };

  using position_t = std::pair<unit_t, unit_t>;
  using tile_map_t = std::map<position_t, unit_t>;

  struct arcade_cabinet_t {
    int_code_program_state_t _program_state;
    tile_map_t  _tile_map;
    unit_t _score = 0;
    unit_t _joystick_state = JOY_NEUTRAL;

    // For autoplay mode
    position_t _ball_position = {-1, -1};
    position_t _paddle_position = {-1, -1};

    void render_screen() {
      // Uncomment the following lines to see playback with a human friendly refresh rate
//      std::this_thread::sleep_for(std::chrono::milliseconds(150));
//      std::cout << "\033[2J" << std::flush;
      std::cout << std::endl << "SCORE: " << _score << std::endl;
      // Find dimensions of message
      unit_t min_x = std::numeric_limits<unit_t>::max(), max_x = std::numeric_limits<unit_t>::min();
      unit_t min_y = std::numeric_limits<unit_t>::max(), max_y = std::numeric_limits<unit_t>::min();
      for (const auto &[position, type] : _tile_map) {
        auto &&[x, y] = position;
        if (x < min_x) min_x = x;
        if (x > max_x) max_x = x;
        if (y < min_y) min_y = y;
        if (y > max_y) max_y = y;
      }
      unit_t width = max_x - min_x + 1, height = max_y - min_y + 1;
      std::vector<std::vector<unit_t>> painted_grid(height, std::vector<unit_t>(width, TILE_EMPTY));
      // Mark painted positions
      for (const auto &[position, type] : _tile_map) {
        auto &&[x, y] = position;
        painted_grid[y][x] = type;
      }
      // Print out grid
      for (auto &row : painted_grid) {
        for (auto &col : row) {
          char val = 'X';
          switch (col) {
            case TILE_EMPTY: val = ' '; break;
            case TILE_WALL: val = '#'; break;
            case TILE_BLOCK: val = '%'; break;
            case TILE_HORIZ_PADDLE: val = '='; break;
            case TILE_BALL: val = '*'; break;
            default: assert(0);
          }
          std::cout << val;
        }
        std::cout << std::endl;
      }
    }

    void run_program() {
      std::queue<unit_t> outputs;

      _program_state.run([&]() -> unit_t {
        render_screen();

        // Autoplay mode!
        auto&& [ball_x, ball_y] = _ball_position;
        auto&& [paddle_x, paddle_y] = _paddle_position;
        if (ball_x != -1 && paddle_x != -1) {
          if (paddle_x < ball_x) _joystick_state = JOY_RIGHT;
          else if (paddle_x > ball_x) _joystick_state = JOY_LEFT;
          else _joystick_state = JOY_NEUTRAL;
        }

        return _joystick_state;
      }, [&](unit_t value) {
        outputs.push(value);
        if (outputs.size() == 3) {
          auto x = outputs.front();
          outputs.pop();
          auto y = outputs.front();
          outputs.pop();
          if (x == -1 && y == 0) {
            _score = outputs.front();
            outputs.pop();
          } else {
            auto type = outputs.front();
            outputs.pop();
            position_t position{x, y};
            _tile_map[position] = type;
            if (type == TILE_BALL) _ball_position = position;
            else if (type == TILE_HORIZ_PADDLE) _paddle_position = position;
          }
        }
      }, [&]() -> bool {
        return false;
      });
    }
  };

  void problem1() {
    arcade_cabinet_t arcade_cabinet;
    read_data(arcade_cabinet._program_state._program_code, "data/day13/problem1/input.txt");
    arcade_cabinet.run_program();
    std::cout << "Result : " << std::count_if(arcade_cabinet._tile_map.begin(), arcade_cabinet._tile_map.end(), [](const auto &v) -> bool { return v.second == TILE_BLOCK; }) << std::endl;
  }

  void problem2() {
    arcade_cabinet_t arcade_cabinet;
    read_data(arcade_cabinet._program_state._program_code, "data/day13/problem2/input.txt");
    arcade_cabinet._program_state._program_code[0] = 2; // free play mode
    arcade_cabinet.run_program();
  }

} // namespace day1
