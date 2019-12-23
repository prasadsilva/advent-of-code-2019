#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <queue>

namespace day23 {

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
      step(input_handler, output_handler, trace);
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

  using packet_t = std::pair<unit_t, unit_t>;
  struct request_t {
    unit_t _destination = -1;
    packet_t _data;

    bool is_valid() const { return _destination >= 0; }
  };

  struct computer_t {
    int_code_program_state_t _program_state;
    unit_t _address;
    std::queue<unit_t> _receive_queue;
    std::queue<unit_t> _send_queue;

    request_t step(bool trace = false) {
//      std::cout << "Stepping computer " << _address << " (" << _program_state._instruction_pointer << ")" << std::endl;
      request_t output;
      _program_state.run([&]() -> unit_t {
        if (trace) std::cout << _address << " read" << std::endl;
        unit_t data = -1;
        if (!_receive_queue.empty()) {
          data = _receive_queue.front();
          _receive_queue.pop();
          if (trace) std::cout << _address << " <- " << data << std::endl;
        }
        return data;
      }, [&](unit_t data) {
        if (trace) std::cout << _address << " write" << std::endl;
        _send_queue.push(data);
        if (_send_queue.size() == 3) {
          // Flush
          output._destination = _send_queue.front();
          _send_queue.pop();
          output._data.first = _send_queue.front();
          _send_queue.pop();
          output._data.second = _send_queue.front();
          _send_queue.pop();
          if (trace) std::cout << _address << "\t -> \t" << output._destination << " (" << output._data.first << "," << output._data.second << ")" << std::endl;
        }
      }, [&]() -> bool {
        return true;
      });
      return output;
    }
  };

  struct network_t {
    std::vector<computer_t> _computers;
    std::queue<packet_t> _nat_packets;

    void initialize(const int_code_program_t &code, unit_t num_computers) {
      for (unit_t i = 0; i < num_computers; i++) {
        computer_t computer;
        computer._address = i;
        computer._receive_queue.push(i);
        computer._program_state._program_code = code;
        _computers.push_back(computer);
      }
    }

    void step(bool trace = false) {
      std::vector<std::queue<packet_t>> outgoing_packets;
      for (auto &computer : _computers) {
        request_t request = computer.step(trace);
        if (request.is_valid()) {
          if (request._destination == 255) {
            if (!_nat_packets.empty()) _nat_packets.pop();
            _nat_packets.push(request._data);
          } else {
            _computers[request._destination]._receive_queue.push(request._data.first);
            _computers[request._destination]._receive_queue.push(request._data.second);
          }
        }
      }
    }

    bool is_idle() {
      for (auto &computer : _computers) {
        if (!computer._receive_queue.empty()) return false;
      }
      return true;
    }
  };

  void problem1() {
    int_code_program_t code;
    read_data(code, "data/day23/problem1/input.txt");
    network_t network;
    network.initialize(code, 50);
    while (network._nat_packets.empty()) {
      network.step();
    }
    std::cout << "Result: " << network._nat_packets.front().second << std::endl;
  }

  void problem2() {
    int_code_program_t code;
    read_data(code, "data/day23/problem2/input.txt");
    network_t network;
    network.initialize(code, 50);

    bool done = false;
    packet_t last_nat_flush_packet{-1, -1};
    while (!done) {
      network.step();
      if (network.is_idle() && !network._nat_packets.empty()) {
        auto current_nat_flush_packet = network._nat_packets.front();
        network._computers[0]._receive_queue.push(current_nat_flush_packet.first);
        network._computers[0]._receive_queue.push(current_nat_flush_packet.second);
        network._nat_packets.pop();
        if (current_nat_flush_packet == last_nat_flush_packet) {
          done = true;
        } else {
          last_nat_flush_packet = current_nat_flush_packet;
        }
      }
    }
    std::cout << "Result: " << last_nat_flush_packet.second << std::endl;
  }

} // namespace day1
