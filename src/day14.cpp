#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <regex>
#include <queue>
#include <map>
#include <set>

namespace day14 {

  using unit_t = int64_t;

  struct reaction_t {
    std::vector<std::pair<std::string, unit_t>> _inputs;
    std::string _output_chemical{"INVALID"};
    unit_t _output_amount{-1};

    void parse(const std::string &value) {
      // E.g.: 2 AB, 3 BC, 4 CA => 1 FUEL
      std::regex pattern(R"((.+) => (\d+) ([A-Z]+))");
      std::smatch matches;

      if (!std::regex_search(value, matches, pattern)) {
        throw new std::invalid_argument("Cannot parse reaction rule!");
      }

      _output_amount = std::stoi(matches[2].str());
      _output_chemical = matches[3].str();

      auto inputs = matches[1].str();
      std::regex input_pattern(R"((\d+) ([^,]+))");
      auto begin = std::sregex_iterator(inputs.begin(), inputs.end(), input_pattern);
      auto end = std::sregex_iterator();
      for (auto it = begin; it != end; it++) {
        matches = *it;
        _inputs.emplace_back( matches[2].str(), std::stoi(matches[1].str()));
      }
    }

    friend std::istream &operator>>(std::istream &in, reaction_t &reaction) {
      std::string line;
      getline(in, line);
      reaction.parse(line);
      return in;
    }
  };

  void read_data(std::vector<reaction_t> &outdata, const char *filepath) {
    std::ifstream input_stream(filepath);
    while (!input_stream.eof()) {
      reaction_t reaction;
      input_stream >> reaction;
      outdata.push_back(reaction);
    }
  }

  unit_t compute_ore_requirement(const std::vector<reaction_t> &reactions, bool trace = false) {
    std::queue<std::pair<std::string, unit_t>> process_queue{};
    std::map<std::string, unit_t> totals;
    std::map<std::string, reaction_t> chemical_reaction_lookup{};
    // Generate reaction lookup
    for (auto &reaction : reactions) {
      chemical_reaction_lookup[reaction._output_chemical] = reaction;
      if (reaction._output_chemical == "FUEL") {
        process_queue.push({ reaction._output_chemical, 1 });
      }
    }
    // Generate chemical ancestry values
    std::map<std::string, unit_t> ancestry_value;
    std::function<unit_t(const std::string &)> find_ancestry_value = [&](const std::string &chemical) -> unit_t {
      if (chemical == "ORE") return 1;
      else {
        unit_t max_value = 0;
        auto reaction = chemical_reaction_lookup[chemical];
        for (auto &input : reaction._inputs) {
          auto&& [input_chemical, _] = input;
          auto input_value = find_ancestry_value(input_chemical);
          if (input_value > max_value) max_value = input_value;
        }
        return max_value + 1;
      }
    };
    for (auto &reaction : reactions) {
      ancestry_value[reaction._output_chemical] = find_ancestry_value(reaction._output_chemical);
    }
    std::vector<std::string> chemicals_ancestry_sorted;
    for (auto &reaction : reactions)  chemicals_ancestry_sorted.push_back(reaction._output_chemical);
    std::sort(chemicals_ancestry_sorted.begin(), chemicals_ancestry_sorted.end(), [&](const std::string &a, const std::string &b) -> bool {
      return ancestry_value[a] > ancestry_value[b];
    });

    bool done = false;
    while (!done) {
      if (trace) std::cout << "Begin phase" << std::endl;
      auto old_totals = totals;
      while (!process_queue.empty()) {
        // Get the next item to process
        auto&& [output_chemical, output_amount] = process_queue.front();
        process_queue.pop();
        assert(output_chemical != "ORE");
        if (trace) {
          std::cout << "Processing " << output_amount << " " << output_chemical;
        }
        // Get current total for item
        auto current_total = totals.find(output_chemical) == totals.end() ? 0 : totals[output_chemical];
        // Get reaction for processing item
        auto &reaction = chemical_reaction_lookup[output_chemical];
        auto accum_output = output_amount + current_total;
        if (trace) {
          std::cout << " (+ " << current_total << ")" << std::endl;
        }
        if (accum_output < reaction._output_amount) {
          // We still can't process it. Absorb into totals
          totals[output_chemical] = accum_output;
          if (trace) {
            std::cout << "\tAbsorbing to totals" << std::endl;
          }
        } else {
          // We can process!
          unit_t multiplier = accum_output / reaction._output_amount;
          unit_t overflow_amount = accum_output % reaction._output_amount;
          if (overflow_amount > 0) {
            totals[output_chemical] = overflow_amount;
          } else {
            totals.erase(output_chemical);
          }
          for (auto&& [input_chemical, input_amount] : reaction._inputs) {
            std::pair<std::string, unit_t> process_rule{
                input_chemical,
                input_amount * multiplier
            };
            auto&& [new_output_chemical, new_output_amount] = process_rule;
            if (trace) {
              std::cout << "\tAdded " << new_output_amount << " " << new_output_chemical << std::endl;
            }
            if (new_output_chemical == "ORE") {
              totals[new_output_chemical] += new_output_amount;
            } else {
              process_queue.push(process_rule);
            }
          }
        }
      }
      done = totals.size() == 1 && totals.find("ORE") != totals.end();
      for (auto &chemical : chemicals_ancestry_sorted) {
        if (totals.find(chemical) != totals.end()) {
          // Expand and short circuit
          auto amount = totals[chemical];
          auto &reaction = chemical_reaction_lookup[chemical];
          assert(amount < reaction._output_amount);
          if (trace) {
            std::cout << "%% Adding " << reaction._output_amount << " " << chemical << " (+ " << (reaction._output_amount - amount) << ") for processing" << std::endl;
          }
          // Add extra amounts to enable processing
          process_queue.push({
            chemical,
            reaction._output_amount
          });
          totals.erase(chemical);
          break;
        }
      }
    }
    return totals["ORE"];
  }

  void problem1() {
    {
      std::vector<std::string> reaction_defns = {
          "10 ORE => 10 A",
          "1 ORE => 1 B",
          "7 A, 1 B => 1 C",
          "7 A, 1 C => 1 D",
          "7 A, 1 D => 1 E",
          "7 A, 1 E => 1 FUEL"
      };
      std::vector<reaction_t> reactions;
      for (auto &reaction_defn : reaction_defns) {
        reaction_t reaction;
        reaction.parse(reaction_defn);
        reactions.push_back(reaction);
      }
      auto num_ores = compute_ore_requirement(reactions);
      std::cout << "ORE needed for 1 FUEL: " << num_ores << std::endl << std::endl;
      assert(num_ores == 31);
    }
    {
      std::vector<std::string> reaction_defns = {
          "9 ORE => 2 A",
          "8 ORE => 3 B",
          "7 ORE => 5 C",
          "3 A, 4 B => 1 AB",
          "5 B, 7 C => 1 BC",
          "4 C, 1 A => 1 CA",
          "2 AB, 3 BC, 4 CA => 1 FUEL"
      };
      std::vector<reaction_t> reactions;
      for (auto &reaction_defn : reaction_defns) {
        reaction_t reaction;
        reaction.parse(reaction_defn);
        reactions.push_back(reaction);
      }
      auto num_ores = compute_ore_requirement(reactions);
      std::cout << "ORE needed for 1 FUEL: " << num_ores << std::endl << std::endl;
      assert(num_ores == 165);
    }
    {
      std::vector<std::string> reaction_defns = {
          "157 ORE => 5 NZVS",
          "165 ORE => 6 DCFZ",
          "44 XJWVT, 5 KHKGT, 1 QDVJ, 29 NZVS, 9 GPVTF, 48 HKGWZ => 1 FUEL",
          "12 HKGWZ, 1 GPVTF, 8 PSHF => 9 QDVJ",
          "179 ORE => 7 PSHF",
          "177 ORE => 5 HKGWZ",
          "7 DCFZ, 7 PSHF => 2 XJWVT",
          "165 ORE => 2 GPVTF",
          "3 DCFZ, 7 NZVS, 5 HKGWZ, 10 PSHF => 8 KHKGT"
      };
      std::vector<reaction_t> reactions;
      for (auto &reaction_defn : reaction_defns) {
        reaction_t reaction;
        reaction.parse(reaction_defn);
        reactions.push_back(reaction);
      }
      auto num_ores = compute_ore_requirement(reactions);
      std::cout << "ORE needed for 1 FUEL: " << num_ores << std::endl << std::endl;
      assert(num_ores == 13312);
    }
    {
      std::vector<std::string> reaction_defns = {
          "2 VPVL, 7 FWMGM, 2 CXFTF, 11 MNCFX => 1 STKFG",
          "17 NVRVD, 3 JNWZP => 8 VPVL",
          "53 STKFG, 6 MNCFX, 46 VJHF, 81 HVMC, 68 CXFTF, 25 GNMV => 1 FUEL",
          "22 VJHF, 37 MNCFX => 5 FWMGM",
          "139 ORE => 4 NVRVD",
          "144 ORE => 7 JNWZP",
          "5 MNCFX, 7 RFSQX, 2 FWMGM, 2 VPVL, 19 CXFTF => 3 HVMC",
          "5 VJHF, 7 MNCFX, 9 VPVL, 37 CXFTF => 6 GNMV",
          "145 ORE => 6 MNCFX",
          "1 NVRVD => 8 CXFTF",
          "1 VJHF, 6 MNCFX => 4 RFSQX",
          "176 ORE => 6 VJHF"
      };
      std::vector<reaction_t> reactions;
      for (auto &reaction_defn : reaction_defns) {
        reaction_t reaction;
        reaction.parse(reaction_defn);
        reactions.push_back(reaction);
      }
      auto num_ores = compute_ore_requirement(reactions);
      std::cout << "ORE needed for 1 FUEL: " << num_ores << std::endl << std::endl;
      assert(num_ores == 180697);
    }
    {
      std::vector<std::string> reaction_defns = {
          "171 ORE => 8 CNZTR",
          "7 ZLQW, 3 BMBT, 9 XCVML, 26 XMNCP, 1 WPTQ, 2 MZWV, 1 RJRHP => 4 PLWSL",
          "114 ORE => 4 BHXH",
          "14 VRPVC => 6 BMBT",
          "6 BHXH, 18 KTJDG, 12 WPTQ, 7 PLWSL, 31 FHTLT, 37 ZDVW => 1 FUEL",
          "6 WPTQ, 2 BMBT, 8 ZLQW, 18 KTJDG, 1 XMNCP, 6 MZWV, 1 RJRHP => 6 FHTLT",
          "15 XDBXC, 2 LTCX, 1 VRPVC => 6 ZLQW",
          "13 WPTQ, 10 LTCX, 3 RJRHP, 14 XMNCP, 2 MZWV, 1 ZLQW => 1 ZDVW",
          "5 BMBT => 4 WPTQ",
          "189 ORE => 9 KTJDG",
          "1 MZWV, 17 XDBXC, 3 XCVML => 2 XMNCP",
          "12 VRPVC, 27 CNZTR => 2 XDBXC",
          "15 KTJDG, 12 BHXH => 5 XCVML",
          "3 BHXH, 2 VRPVC => 7 MZWV",
          "121 ORE => 7 VRPVC",
          "7 XCVML => 6 RJRHP",
          "5 BHXH, 4 VRPVC => 5 LTCX"
      };
      std::vector<reaction_t> reactions;
      for (auto &reaction_defn : reaction_defns) {
        reaction_t reaction;
        reaction.parse(reaction_defn);
        reactions.push_back(reaction);
      }
      auto num_ores = compute_ore_requirement(reactions);
      std::cout << "ORE needed for 1 FUEL: " << num_ores << std::endl << std::endl;
      assert(num_ores == 2210736);
    }

    std::vector<reaction_t> input;
    read_data(input, "data/day14/problem1/input.txt");
    std::cout << "Result : " << compute_ore_requirement(input) << std::endl;
  }

  void problem2() {
//    assert(get_fuel_required_recursive(14) == 2);

//    std::vector<int> input;
//    read_data(input, "data/day1/problem2/input.txt");
//    std::cout << "Result : " << get_total_fuel_required(input, get_fuel_required_recursive) << std::endl;
  }

} // namespace day1
