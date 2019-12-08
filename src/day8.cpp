#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>

namespace day8 {

  using image_layer_t = std::vector<long>;
  struct image_t {
    enum pixel_e {
      BLACK = 0,
      WHITE = 1,
      TRANSPARENT = 2,
    };

    std::vector<image_layer_t> _layers;
    image_layer_t _merged;
    long _width, _height;

    image_t(long width, long height) : _width(width), _height(height) {
      _merged.resize(_width * _height, pixel_e::TRANSPARENT);
    }

    void add_layer(const image_layer_t &layer) {
      _layers.push_back(layer);
      for (long pixel_idx = 0; pixel_idx < _merged.size(); pixel_idx++) {
        auto merged_pixel = _merged[pixel_idx];
        if (merged_pixel == pixel_e::TRANSPARENT) _merged[pixel_idx] = layer[pixel_idx];
      }
    }

    long calculate_checksum() {
      // Find layer with fewest 0 digits
      long min_0_digits = std::numeric_limits<long>::max();
      long min_layer_idx = -1;
      for (long layer_idx = 0; layer_idx < _layers.size(); layer_idx++) {
        auto& layer = _layers[layer_idx];
        long num_0_digits = std::count_if(layer.begin(), layer.end(), [](long val) { return val == 0; });
        if (num_0_digits < min_0_digits) {
          min_0_digits = num_0_digits;
          min_layer_idx = layer_idx;
        }
      }
      // Return number 1 digits multiplied by number of 2 digits
      auto& layer = _layers[min_layer_idx];
      long num_1_digits = std::count_if(layer.begin(), layer.end(), [](long val) { return val == 1; });
      long num_2_digits = std::count_if(layer.begin(), layer.end(), [](long val) { return val == 2; });
      return num_1_digits * num_2_digits;
    }

    friend std::ostream& operator << (std::ostream& out, image_t& image) {
      for (long pixel_idx = 0; pixel_idx < image._merged.size(); pixel_idx++) {
        auto pixel = image._merged[pixel_idx];
        switch (pixel) {
          case pixel_e::BLACK: std::cout << ' '; break;
          case pixel_e::WHITE: std::cout << "█"; break;
          case pixel_e::TRANSPARENT: std::cout << ' '; break;
          default: assert(0);
        }
        if ((pixel_idx % image._width) == (image._width - 1)) out << std::endl;
      }
      out << std::endl;
      return out;
    }
  };

  void read_data(image_t &image, const char *filepath) {
    std::ifstream input_stream(filepath);
    char c;
    image_layer_t temp;
    long max_layer_values = image._width * image._height;
    while (input_stream >> c) {
      temp.push_back(c - '0');
      if (temp.size() == max_layer_values) {
        image.add_layer(temp);
        temp.clear();
      }
    }
  }

  void problem1() {
    image_t input(25, 6);
    read_data(input, "data/day8/problem1/input.txt");
    std::cout << "Result : " << input.calculate_checksum() << std::endl;
  }

  void problem2() {
    image_t input(25, 6);
    read_data(input, "data/day8/problem2/input.txt");
    std::cout << "Result : " << std::endl;
    std::cout << input << std::endl;
  }

} // namespace day1
