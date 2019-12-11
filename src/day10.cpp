#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <list>
#include <cmath>

namespace day10 {

  using unit_t = int64_t;

  struct asteroid_t {
    unit_t _x, _y;
    asteroid_t(unit_t x, unit_t y) : _x(x), _y(y) {}
  };

  using asteroid_ptr_t = std::shared_ptr<asteroid_t>;

  struct asteroid_location_t {
    asteroid_ptr_t  asteroid = nullptr;
    unit_t          num_other_asteroids_visible = -1;
  };

  struct map_t {
    std::vector<asteroid_ptr_t>               _asteroids;
    std::vector<std::vector<asteroid_ptr_t>>  _grid;

    map_t(const std::vector<std::string> &map) {
      unit_t x = 0, y = 0;
      for (auto &row : map) {
        x = 0;
        std::vector<asteroid_ptr_t> ptr_row;
        ptr_row.resize(row.size());
        for (auto &cell : row) {
          if (cell == '#') {
            auto asteroid = std::make_shared<asteroid_t>(x, y);
            _asteroids.push_back(asteroid);
            ptr_row[x] = asteroid;
          } else {
            ptr_row[x] = nullptr;
          }
          x++;
        }
        _grid.push_back(ptr_row);
        y++;
      }
    }

    bool in_bounds(unit_t x, unit_t y) {
      if (y < 0 || y >= _grid.size()) return false;
      return !(x < 0 || x >= _grid[0].size());
    }

    enum visibility_e {
      SOURCE = 'S',
      EMPTY = '.',
      OCCUPIED = '0',
      BLOCKED = 'x',
      BLOCKED_OCCUPIED = 'X'
    };

    using visibility_map_t = std::vector<std::vector<char>>;

    void compute_visibility(visibility_map_t &visibility_map, asteroid_ptr_t asteroid) {
      std::vector<char> visibility_row;
      visibility_row.resize(_grid[0].size(), visibility_e::EMPTY);
      visibility_map.resize(_grid.size(), visibility_row);

      visibility_map[asteroid->_y][asteroid->_x] = visibility_e::SOURCE;
      for (unit_t delta_y = -visibility_map.size(); delta_y < (unit_t)visibility_map.size(); delta_y++) {
        for (unit_t delta_x = -visibility_row.size(); delta_x < (unit_t)visibility_row.size(); delta_x++) {
          if (delta_x == 0 && delta_y == 0) continue;
          unit_t dest_x = asteroid->_x + delta_x, dest_y = asteroid->_y + delta_y;
          bool saw_other_asteroid = false;
          while(in_bounds(dest_x, dest_y)) {
            if (_grid[dest_y][dest_x]) {
              if (saw_other_asteroid) visibility_map[dest_y][dest_x] = visibility_e::BLOCKED_OCCUPIED;
              else if (visibility_map[dest_y][dest_x] != visibility_e::BLOCKED_OCCUPIED) visibility_map[dest_y][dest_x] = visibility_e::OCCUPIED;
              saw_other_asteroid = true;
            }
            else {
              if (saw_other_asteroid) visibility_map[dest_y][dest_x] = visibility_e::BLOCKED;
            }
            dest_x += delta_x;
            dest_y += delta_y;
          }
        }
      }
    }

    asteroid_location_t find_best_monitoring_station_location(bool trace = false) {
      asteroid_location_t retval;

      for (auto &asteroid : _asteroids) {
        if (trace) std::cout << "Checking asteroid " << asteroid->_x << "," << asteroid->_y << std::endl;
        visibility_map_t visibility_map;
        compute_visibility(visibility_map, asteroid);

        unit_t num_seen = 0;
        for (auto &row : visibility_map) {
          for (auto &cell : row) {
            if (trace) std::cout << cell;
            if (cell == visibility_e::OCCUPIED) num_seen++;
          }
          if (trace) std::cout << std::endl;
        }
        if (trace) std::cout << "Seen: " << num_seen << std::endl << std::endl;

        if (num_seen > retval.num_other_asteroids_visible) {
          retval.asteroid = asteroid;
          retval.num_other_asteroids_visible = num_seen;
        }
      }

      return retval;
    }

    std::list<asteroid_ptr_t> get_sorted_target_list_for_laser(const visibility_map_t &visibility_map, const asteroid_ptr_t &source) {
      std::list<asteroid_ptr_t> retval;
      for (auto y = 0; y < visibility_map.size(); y++) {
        for (auto x = 0; x < visibility_map[y].size(); x++) {
          if (visibility_map[y][x] == visibility_e::OCCUPIED) retval.push_back(_grid[y][x]);
        }
      }
      retval.sort([&source](asteroid_ptr_t a, asteroid_ptr_t b) {
        // https://math.stackexchange.com/questions/1596513/find-the-bearing-angle-between-two-points-in-a-2d-space
        double thetaA = atan2(a->_x - source->_x, source->_y - a->_y);
        if (thetaA < 0) thetaA += M_PI * 2;
        double thetaB = atan2(b->_x - source->_x, source->_y - b->_y);
        if (thetaB < 0) thetaB += M_PI * 2;
        return thetaA < thetaB;
      });
      return retval;
    }

    void print_grid(const asteroid_ptr_t &source = nullptr) {
      for (auto &row : _grid) {
        for (auto &cell : row) {
          if (cell == source) std::cout << 'S';
          else std::cout << (cell ? '#' : '.');
        }
        std::cout << std::endl;
      }
    }

    asteroid_ptr_t vaporize(const asteroid_ptr_t &source, unit_t iterations, bool trace = false) {
      unit_t iterations_completed = 0;
      asteroid_ptr_t last_target = nullptr;
      while (iterations_completed < iterations) {
        visibility_map_t visibility_map;
        compute_visibility(visibility_map, source);
        std::list<asteroid_ptr_t> targets = get_sorted_target_list_for_laser(visibility_map, source);
        while (iterations_completed < iterations && !targets.empty()) {
          last_target = targets.front();
          _grid[last_target->_y][last_target->_x] = nullptr;
          if (trace) {
            print_grid(source);
            std::cout << std::endl;
          }
          targets.pop_front();
          iterations_completed++;
        }

        if (trace) {
          std::cout << "Iterations left: " << (iterations - iterations_completed) << std::endl << std::endl;
        }
      }
      return last_target;
    }
  };

  void read_data(std::vector<std::string> &outdata, const char *filepath) {
    std::ifstream input_stream(filepath);
    std::copy(std::istream_iterator<std::string>(input_stream),
              std::istream_iterator<std::string>(),
              std::back_inserter(outdata));
  }

  void problem1() {
    {
      map_t map = {{
        ".#..#",
        ".....",
        "#####",
        "....#",
        "...##"
      }};
      auto location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << location.asteroid->_x << "," << location.asteroid->_y << " can see " << location.num_other_asteroids_visible << " other asteroids" << std::endl;
    }
    {
      map_t map = {{
        "......#.#.",
        "#..#.#....",
        "..#######.",
        ".#.#.###..",
        ".#..#.....",
        "..#....#.#",
        "#..#....#.",
        ".##.#..###",
        "##...#..#.",
        ".#....####"
      }};
      auto location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << location.asteroid->_x << "," << location.asteroid->_y << " can see " << location.num_other_asteroids_visible << " other asteroids" << std::endl;
    }
    {
      map_t map = {{
        "#.#...#.#.",
        ".###....#.",
        ".#....#...",
        "##.#.#.#.#",
        "....#.#.#.",
        ".##..###.#",
        "..#...##..",
        "..##....##",
        "......#...",
        ".####.###."
      }};
      auto location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << location.asteroid->_x << "," << location.asteroid->_y << " can see " << location.num_other_asteroids_visible << " other asteroids" << std::endl;
    }
    {
      map_t map = {{
        ".#..#..###",
        "####.###.#",
        "....###.#.",
        "..###.##.#",
        "##.##.#.#.",
        "....###..#",
        "..#.#..#.#",
        "#..#.#.###",
        ".##...##.#",
        ".....#.#.."
      }};
      auto location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << location.asteroid->_x << "," << location.asteroid->_y << " can see " << location.num_other_asteroids_visible << " other asteroids" << std::endl;
    }
    {
      map_t map = {{
        ".#..##.###...#######",
        "##.############..##.",
        ".#.######.########.#",
        ".###.#######.####.#.",
        "#####.##.#.##.###.##",
        "..#####..#.#########",
        "####################",
        "#.####....###.#.#.##",
        "##.#################",
        "#####.##.###..####..",
        "..######..##.#######",
        "####.##.####...##..#",
        ".#####..#.######.###",
        "##...#.##########...",
        "#.##########.#######",
        ".####.#.###.###.#.##",
        "....##.##.###..#####",
        ".#.#.###########.###",
        "#.#.#.#####.####.###",
        "###.##.####.##.#..##"
      }};
      auto location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << location.asteroid->_x << "," << location.asteroid->_y << " can see " << location.num_other_asteroids_visible << " other asteroids" << std::endl;
    }

    std::vector<std::string> input;
    read_data(input, "data/day10/problem1/input.txt");
    map_t map(input);
    std::cout << "Result : " << map.find_best_monitoring_station_location().num_other_asteroids_visible << std::endl;
  }

  void problem2() {
    {
      map_t map = {{
                       ".#..#",
                       ".....",
                       "#####",
                       "....#",
                       "...##"
                   }};
      auto station_location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << station_location.asteroid->_x << "," << station_location.asteroid->_y << " can see " << station_location.num_other_asteroids_visible << " other asteroids" << std::endl;
      auto last_vaporized_asteroid = map.vaporize(station_location.asteroid, 7);
      std::cout << "Last vaporized asteroid is at " << last_vaporized_asteroid->_x << "," << last_vaporized_asteroid->_y << std::endl;
    }
    {
      map_t map = {{
        ".#..##.###...#######",
        "##.############..##.",
        ".#.######.########.#",
        ".###.#######.####.#.",
        "#####.##.#.##.###.##",
        "..#####..#.#########",
        "####################",
        "#.####....###.#.#.##",
        "##.#################",
        "#####.##.###..####..",
        "..######..##.#######",
        "####.##.####...##..#",
        ".#####..#.######.###",
        "##...#.##########...",
        "#.##########.#######",
        ".####.#.###.###.#.##",
        "....##.##.###..#####",
        ".#.#.###########.###",
        "#.#.#.#####.####.###",
        "###.##.####.##.#..##"
      }};
      auto station_location = map.find_best_monitoring_station_location();
      std::cout << "Asteroid : " << station_location.asteroid->_x << "," << station_location.asteroid->_y << " can see " << station_location.num_other_asteroids_visible << " other asteroids" << std::endl;
      auto last_vaporized_asteroid = map.vaporize(station_location.asteroid, 200);
      std::cout << "200th vaporized asteroid is at " << last_vaporized_asteroid->_x << "," << last_vaporized_asteroid->_y << std::endl;
    }

    std::vector<std::string> input;
    read_data(input, "data/day10/problem2/input.txt");
    map_t map(input);
    auto station_location = map.find_best_monitoring_station_location();
    auto last_vaporized_asteroid = map.vaporize(station_location.asteroid, 200);
    auto value = last_vaporized_asteroid->_x * 100 + last_vaporized_asteroid->_y;
    std::cout << "Result : " << value << std::endl;
  }

} // namespace day1
