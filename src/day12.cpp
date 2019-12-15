#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <regex>

namespace day12 {

  using unit_t = int64_t;

  struct triple_t {
    unit_t _x, _y, _z;

    bool operator == (const triple_t &other) const { return _x == other._x && _y == other._y && _z == other._z; }
  };

  struct moon_t {
    triple_t  _position{0, 0, 0};
    triple_t  _velocity{0, 0, 0};

    bool operator == (const moon_t &other) const { return _position == other._position && _velocity == other._velocity; }

    friend std::istream &operator>>(std::istream &in, moon_t &moon) {
      // E.g.: <x=9, y=-4, z=14>
      std::string line;
      getline(in, line);

      std::regex pattern(R"(<x=(.+), y=(.+), z=(.+)>)");
      std::smatch matches;

      if (!std::regex_search(line, matches, pattern)) {
        throw new std::invalid_argument("Cannot parse step rule!");
      }

      moon._position._x = std::stoi(matches[1].str());
      moon._position._y = std::stoi(matches[2].str());
      moon._position._z = std::stoi(matches[3].str());

      return in;
    }
  };

  struct system_t {
    std::vector<moon_t> _moons;

    void simulate() {
      // update the velocity of every moon by applying gravity
      for (unit_t idx_a = 0; idx_a < _moons.size(); idx_a++) {
        for (unit_t idx_b = idx_a + 1; idx_b < _moons.size(); idx_b++) {
          auto &moon_a = _moons[idx_a];
          auto &moon_b = _moons[idx_b];
          triple_t velocity_delta = {
              (moon_a._position._x > moon_b._position._x) ? -1 : ((moon_a._position._x == moon_b._position._x) ? 0 : 1),
              (moon_a._position._y > moon_b._position._y) ? -1 : ((moon_a._position._y == moon_b._position._y) ? 0 : 1),
              (moon_a._position._z > moon_b._position._z) ? -1 : ((moon_a._position._z == moon_b._position._z) ? 0 : 1)
          };
          moon_a._velocity._x += velocity_delta._x;
          moon_a._velocity._y += velocity_delta._y;
          moon_a._velocity._z += velocity_delta._z;
          moon_b._velocity._x += velocity_delta._x * -1;
          moon_b._velocity._y += velocity_delta._y * -1;
          moon_b._velocity._z += velocity_delta._z * -1;
        }
      }
      // update the position of every moon by applying velocity
      for (auto & moon : _moons) {
        moon._position._x += moon._velocity._x;
        moon._position._y += moon._velocity._y;
        moon._position._z += moon._velocity._z;
      }
    }

    [[nodiscard]] unit_t get_total_energy() const {
      return std::accumulate(_moons.begin(), _moons.end(), 0, [](unit_t total_energy, const moon_t &moon) -> unit_t {
        unit_t pot = std::abs(moon._position._x) + std::abs(moon._position._y) + std::abs(moon._position._z);
        unit_t kin = std::abs(moon._velocity._x) + std::abs(moon._velocity._y) + std::abs(moon._velocity._z);
        return total_energy + (pot * kin);
      });
    }
  };

  void read_data(std::vector<moon_t> &outdata, const char *filepath) {
    std::ifstream input_stream(filepath);
    moon_t moon;
    while (!input_stream.eof()) {
      input_stream >> moon;
      outdata.push_back(moon);
    }
  }

  void problem1() {
    {
      system_t system;
      system._moons = {
          {{-1, 0, 2}},
          {{2, -10, -7}},
          {{4, -8, 8}},
          {{3, 5, -1}}
      };
      for (unit_t i = 0; i < 10; i++) {
        system.simulate();
      }
      std::cout << "Test : " << system.get_total_energy() << std::endl;
    }

    system_t system;
    read_data(system._moons, "data/day12/problem1/input.txt");
    for (unit_t i = 0; i < 1000; i++) {
      system.simulate();
    }
    std::cout << "Result : " << system.get_total_energy() << std::endl;
  }

  void problem2() {
    {
      system_t system;
      std::vector<moon_t> moons = {
          {{-1, 0, 2}},
          {{2, -10, -7}},
          {{4, -8, 8}},
          {{3, 5, -1}}
      };
      system._moons = moons;
      unit_t last_initial_state_step[] = {0, 0, 0, 0};
      for (unit_t step = 0; step < 10000; step++) {
        system.simulate();
        for (unit_t moon_idx = 0; moon_idx < moons.size(); moon_idx++) {
          if (system._moons[moon_idx] == moons[moon_idx]) {
            auto delta = step - last_initial_state_step[moon_idx];
            std::cout << "Found initial state for moon " << moon_idx << " at step " << step << " (delta = " << delta << ")" << std::endl;
            last_initial_state_step[moon_idx] = step;
          }
        }
      }
    }
//    assert(get_fuel_required_recursive(14) == 2);

//    std::vector<int> input;
//    read_data(input, "data/day1/problem2/input.txt");
//    std::cout << "Result : " << get_total_fuel_required(input, get_fuel_required_recursive) << std::endl;
  }

} // namespace day1
