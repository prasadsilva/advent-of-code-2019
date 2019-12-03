#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <numeric>
#include <sstream>
#include <regex>
#include <math.h>

namespace day3 {

  using point_t = std::pair<int, int>;
  using line_seg_t = std::pair<point_t, point_t>;

  std::ostream& operator << (std::ostream &out, const point_t &pt) {
    out << "(" << pt.first << "," << pt.second << ")";
    return out;
  }
  std::ostream& operator << (std::ostream &out, const line_seg_t &seg) {
    out << seg.first << "," << seg.second;
    return out;
  }

  struct wire_t {
    std::vector<line_seg_t> segments;

    void initialize(const std::string& path) {
      // E.g.: R8,U5,L5,D3
      std::regex pattern("(U|D|L|R)(\\d+)");
      std::smatch matches;

      point_t curr_point = {0, 0};

      std::stringstream input_stream(path);
      while( input_stream.good() )
      {
        std::string substr;
        getline( input_stream, substr, ',' );
        if (std::regex_match(substr, matches, pattern)) {
          auto next_point = curr_point;
//          std::cout << "> " << matches[1].str() << " -> " << matches[2].str() << std::endl;
          switch (matches[1].str().at(0)) {
            case 'U': next_point.second -= stoi(matches[2]); break;
            case 'D': next_point.second += stoi(matches[2]); break;
            case 'L': next_point.first -= stoi(matches[2]); break;
            case 'R': next_point.first += stoi(matches[2]); break;
            default: {
              assert(0);
            }
          }
          segments.emplace_back(curr_point, next_point);
          curr_point = next_point;
        } else {
          assert(0);
        }
      }
    }
  };

  int get_distance_between(const point_t &p0, const point_t &p1) {
    auto x1 = p0.first;
    auto y1 = p0.second;
    auto x2 = p1.first;
    auto y2 = p1.second;
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
  }

  int get_length(const line_seg_t &segment) {
    return get_distance_between(segment.first, segment.second);
  }

  int get_manhattan_dist_from_origin(const point_t &point) {
    return std::abs(point.first) + std::abs(point.second);
  }

  // https://stackoverflow.com/a/328122/2847817
  bool is_between(const point_t &a, const point_t &b, const point_t &c) {
    auto crossproduct = (c.second - a.second) * (b.first - a.first) - (c.first - a.first) * (b.second - a.second);

    // compare versus epsilon for floating point values, or != 0 if using integers
    if (std::abs(crossproduct) != 0) return false;

    auto dotproduct = (c.first - a.first) * (b.first - a.first) + (c.second - a.second)*(b.second - a.second);
    if (dotproduct < 0) return false;

    auto squaredlengthba = (b.first - a.first)*(b.first - a.first) + (b.second - a.second)*(b.second - a.second);
    return dotproduct <= squaredlengthba;

  }

  // https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
  constexpr point_t find_intersection(const line_seg_t &segment1, const line_seg_t &segment2) {
    auto A = segment1.first;
    auto B = segment1.second;
    auto C = segment2.first;
    auto D = segment2.second;

    // Line AB represented as a1x + b1y = c1
    double a1 = B.second - A.second;
    double b1 = A.first - B.first;
    double c1 = a1*(A.first) + b1*(A.second);

    // Line CD represented as a2x + b2y = c2
    double a2 = D.second - C.second;
    double b2 = C.first - D.first;
    double c2 = a2*(C.first)+ b2*(C.second);

    double determinant = a1*b2 - a2*b1;

    if (determinant == 0)
    {
      // The lines are parallel. This is simplified
      // by returning a pair of FLT_MAX
      return point_t{0, 0};
    }
    else
    {
      double x = (b2*c1 - b1*c2)/determinant;
      double y = (a1*c2 - a2*c1)/determinant;

      point_t intersection{x, y};
      // PPS EDIT: But wait, we have line intersection. Ensure that the intersection point is on
      //           the line SEGMENT
      if (is_between(A, B, intersection) && is_between(C, D, intersection))
        return intersection;
      else
        return point_t{0, 0};
    }
  }

  int find_closest_intersection_point_distance(const wire_t &wire1, const wire_t &wire2) {
    // For each wire1 x wire2 segment intersections, keep track of closest to origin
    // Compute manhattan distance for closest pair
    int closest_distance_to_intersection = std::numeric_limits<int>::max();
    for (auto& wire1_segment : wire1.segments) {
      for (auto& wire2_segment : wire2.segments) {
        auto intersection_pt = find_intersection(wire1_segment, wire2_segment);
        auto distance = get_manhattan_dist_from_origin(intersection_pt);
//        if (distance > 0) {
//          std::cout << "Found intersection at " << intersection_pt;
//          std::cout << "    " << wire1_segment << " <-> " << wire2_segment << std::endl;
//        }
        if (distance > 0 && distance < closest_distance_to_intersection) {
          closest_distance_to_intersection = distance;
        }
      }
    }
    return closest_distance_to_intersection;
  }

  int find_minimum_intersection_steps(const wire_t &wire1, const wire_t &wire2) {
    int minimum_steps_to_intersection = std::numeric_limits<int>::max();
    int wire1_steps = 0;
    for (auto& wire1_segment : wire1.segments) {
      int wire2_steps = 0;
      for (auto &wire2_segment : wire2.segments) {
        auto intersection_pt = find_intersection(wire1_segment, wire2_segment);
        auto distance = get_manhattan_dist_from_origin(intersection_pt);
        if (distance > 0) {
          auto wire1_step_remainder = get_distance_between(wire1_segment.first, intersection_pt);
          auto wire2_step_remainder = get_distance_between(wire2_segment.first, intersection_pt);
          auto steps_to_intersection = (wire1_steps + wire1_step_remainder) + (wire2_steps + wire2_step_remainder);
          if (steps_to_intersection < minimum_steps_to_intersection) {
            minimum_steps_to_intersection = steps_to_intersection;
          }
        }
        wire2_steps += get_length(wire2_segment);
      }
      wire1_steps += get_length(wire1_segment);
    }
    return minimum_steps_to_intersection;
  }

  void read_data(std::vector<wire_t> &outdata, const char *filepath) {
    std::ifstream input_stream(filepath);
    std::string str;
    while (std::getline(input_stream, str))
    {
      wire_t wire;
      wire.initialize(str);
      outdata.push_back(wire);
    }
  }

  void problem1() {
    {
      wire_t test1, test2;
      test1.initialize("R8,U5,L5,D3");
      test2.initialize("U7,R6,D4,L4");
      assert(find_closest_intersection_point_distance(test1, test2) == 6);
    }
    {
      wire_t test1, test2;
      test1.initialize("R75,D30,R83,U83,L12,D49,R71,U7,L72");
      test2.initialize("U62,R66,U55,R34,D71,R55,D58,R83");
      assert(find_closest_intersection_point_distance(test1, test2) == 159);
    }
    {
      wire_t test1, test2;
      test1.initialize("R98,U47,R26,D63,R33,U87,L62,D20,R33,U53,R51");
      test2.initialize("U98,R91,D20,R16,D67,R40,U7,R15,U6,R7");
      assert(find_closest_intersection_point_distance(test1, test2) == 135);
    }

    std::vector<wire_t> wires;
    read_data(wires, "data/day3/problem1/input.txt");
    std::cout << "Result : " << find_closest_intersection_point_distance(wires[0], wires[1]) << std::endl;
  }

  void problem2() {
    {
      wire_t test1, test2;
      test1.initialize("R8,U5,L5,D3");
      test2.initialize("U7,R6,D4,L4");
      assert(find_minimum_intersection_steps(test1, test2) == 30);
    }

    std::vector<wire_t> wires;
    read_data(wires, "data/day3/problem2/input.txt");
    std::cout << "Result : " << find_minimum_intersection_steps(wires[0], wires[1]) << std::endl;


  }

} // namespace day3
