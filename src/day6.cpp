#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <regex>
#include <map>
#include <list>
#include <memory>
#include <array>

namespace day6 {

  constexpr bool trace_read = false;
  constexpr bool trace_test = true;

  struct orbit_defn_t {
    std::string parent, child;

    friend std::istream &operator>>(std::istream &in, orbit_defn_t &orbit_defn) {
      // E.g.: AAA)BBB
      std::string line;
      getline(in, line);

      std::regex pattern(R"((.+)\)(.+))");
      std::smatch matches;

      if (!std::regex_search(line, matches, pattern)) {
        throw new std::invalid_argument("Cannot parse step rule!");
      }

      orbit_defn.parent = matches[1].str();
      orbit_defn.child = matches[2].str();

      if (trace_read) std::cout << "Read " << orbit_defn.child << ")" << orbit_defn.parent << std::endl;
      return in;
    }
  };

  struct orbiting_object_t {
    std::string                                     _name;
    orbiting_object_t*                              _parent = nullptr;
    std::list<std::shared_ptr<orbiting_object_t>>   _children;

    explicit orbiting_object_t(std::string name) : _name(std::move(name)) {}
  };

  struct orbital_map_t {
    std::map<std::string, std::shared_ptr<orbiting_object_t>> orbiting_objects;

    static void output_helper(std::ostream &out, long orbit_depth, const std::shared_ptr<orbiting_object_t>& orbiting_object) {
      for (long i = 0; i < orbit_depth; i++) out << "  ";
      if (orbit_depth > 0) out << "\\_ ";
      out << orbiting_object->_name << std::endl;
      for (auto &child : orbiting_object->_children) output_helper(out, orbit_depth + 1, child);
    }

    friend std::ostream &operator<<(std::ostream &out, orbital_map_t &map) {
      auto universal_center_of_mass = map.orbiting_objects["COM"];
      assert(universal_center_of_mass);
      output_helper(out, 0, universal_center_of_mass);
      return out;
    }

    std::shared_ptr<orbiting_object_t> find_or_create_orbiting_object(std::string name, bool trace = false) {
      auto orbit_iter = orbiting_objects.find(name);
      if (orbit_iter == orbiting_objects.end()) {
        if (trace) std::cout << "Creating " << name << std::endl;
        auto new_object = std::make_shared<orbiting_object_t>(name);
        orbiting_objects[name] = new_object;
        return new_object;
      } else {
        return orbit_iter->second;
      }
    }

    void add_orbit(const orbit_defn_t &orbit_defn, bool trace = false) {
      auto parent_object = find_or_create_orbiting_object(orbit_defn.parent, trace);
      auto child_object = find_or_create_orbiting_object(orbit_defn.child, trace);
      parent_object->_children.push_back(child_object);
      child_object->_parent = parent_object.get();
    }

    static long get_num_total_orbits_helper(
        long prev_orbit_depth,
        const std::shared_ptr<orbiting_object_t>& orbiting_object,
        bool trace = false
    ) {
      long curr_orbit_depth = prev_orbit_depth + 1;
      long num_total_orbits = curr_orbit_depth;
      if (!orbiting_object->_children.empty()) {
        for (auto &child : orbiting_object->_children) {
          num_total_orbits += get_num_total_orbits_helper(curr_orbit_depth, child, trace);
        }
      }
      if (trace) std::cout << "Num orbits for " << orbiting_object->_name << " = " << curr_orbit_depth << std::endl;
      return num_total_orbits - 1;
    }

    long get_num_total_orbits(bool trace = false) {
      auto universal_center_of_mass = find_or_create_orbiting_object("COM");
      assert(universal_center_of_mass);
      auto total_orbits = get_num_total_orbits_helper(0, universal_center_of_mass, trace);
      if (trace) std::cout << "Total orbits: " << total_orbits << std::endl;
      return total_orbits;
    }

    std::vector<orbiting_object_t*> get_ancestor_orbits_for(const std::string &object_name) {
      auto orbiting_object = orbiting_objects[object_name];
      assert(orbiting_object);
      std::vector<orbiting_object_t*> ancestor_orbits;
      auto ancestor_orbit = orbiting_object->_parent;
      while (ancestor_orbit) {
        ancestor_orbits.push_back(ancestor_orbit);
        ancestor_orbit = ancestor_orbit->_parent;
      }
      return ancestor_orbits;
    }

    long get_num_orbital_transfer_steps(const std::string& source_name, const std::string& destination_name, bool trace = false) {
      // Get ancestor list for source, dest
      auto source_ancestor_orbits = get_ancestor_orbits_for(source_name);
      if (trace) {
        std::cout << "Ancestors for " << source_name << " => ";
        for (auto ancestor_orbit : source_ancestor_orbits) std::cout << ancestor_orbit->_name << " ";
        std::cout << std::endl;
      }
      auto destination_ancestor_orbits = get_ancestor_orbits_for(destination_name);
      if (trace) {
        std::cout << "Ancestors for " << destination_name << " => ";
        for (auto ancestor_orbit : destination_ancestor_orbits) std::cout << ancestor_orbit->_name << " ";
        std::cout << std::endl;
      }
      // Back to front, search for common ancestor and return sum of steps to that common ancestor
      long source_transfer_steps = 0;
      long destination_transfer_steps = 0;
      for (auto source_ancestor : source_ancestor_orbits) {
        for (auto destination_ancestor : destination_ancestor_orbits) {
          if (source_ancestor == destination_ancestor) {
            return source_transfer_steps + destination_transfer_steps;
          }
          destination_transfer_steps++;
        }
        source_transfer_steps++;
        destination_transfer_steps = 0;
      }
      return -1;
    }
  };

  void read_data(orbital_map_t &outdata, const char *filepath, bool trace = false) {
    std::ifstream input_stream(filepath);
    orbit_defn_t orbit_defn;
    while (!input_stream.eof()) {
      input_stream >> orbit_defn;
      outdata.add_orbit(orbit_defn, trace);
    }
  }

  void problem1() {
    {
      orbital_map_t test;
      std::vector<orbit_defn_t> defns = {
          {"COM", "B"},
          {"B",   "C"},
          {"C",   "D"},
          {"D",   "E"},
          {"E",   "F"},
          {"B",   "G"},
          {"G",   "H"},
          {"D",   "I"},
          {"E",   "J"},
          {"J",   "K"},
          {"K",   "L"},
      };
      for (auto &defn : defns) test.add_orbit(defn, trace_test);
      std::cout << test << std::endl;
      assert(test.get_num_total_orbits(trace_test) == 42);
      std::cout << std::endl;
    }

    orbital_map_t input;
    read_data(input, "data/day6/problem1/input.txt");
    std::cout << "Result : " << input.get_num_total_orbits() << std::endl;
  }

  void problem2() {
    {
      orbital_map_t test;
      std::vector<orbit_defn_t> defns = {
          {"COM", "B"},
          {"B",   "C"},
          {"C",   "D"},
          {"D",   "E"},
          {"E",   "F"},
          {"B",   "G"},
          {"G",   "H"},
          {"D",   "I"},
          {"E",   "J"},
          {"J",   "K"},
          {"K",   "L"},
          {"K",   "YOU"},
          {"I",   "SAN"},
      };
      for (auto &defn : defns) test.add_orbit(defn, trace_test);
      std::cout << test << std::endl;
      assert(test.get_num_orbital_transfer_steps("YOU", "SAN", trace_test) == 4);
      std::cout << std::endl;
    }

    orbital_map_t input;
    read_data(input, "data/day6/problem2/input.txt");
    std::cout << "Result : " << input.get_num_orbital_transfer_steps("YOU", "SAN") << std::endl;
  }

} // namespace day1
