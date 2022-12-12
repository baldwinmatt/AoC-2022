#include "aoc/helpers.h"
#include <set>
#include <vector>
#include <map>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Sabqponm
abcryxxl
accszExk
acctuvwj
abdefghi)");
  constexpr int SR_Part1 = 31;
  constexpr int SR_Part2 = 29;

  // Height (as character), steps to get there
  using MapPoint = std::pair<int8_t, int64_t>;
  using Map = std::vector<MapPoint>;

  using Point = std::pair<size_t, size_t>;
  struct Grid {
    Map map;
    size_t width;
    size_t height;

    Point start;
    Point dst;
  };

  const auto grid_off = [](const Grid& g, size_t x, size_t y) {
    assert(x < g.width);
    assert(y < g.height);
    return y * g.width + x;
  };

  const auto grid_add = [](Grid& g, size_t x, size_t y, char c) {
    const size_t off = grid_off(g, x, y);
    assert(off < g.map.size());
    auto& e = g.map.at(off);
    if (c == 'S') { c = 'a'; }
    else if (c == 'E') { c = 'z'; }
    e.first = c - 'a';
    e.second = -1;
  };

  const auto grid_set_width = [](Grid& g, size_t w) {
    assert(!g.width || g.width == w);
    if (g.width) { return; }
    g.width = w;
  };

  const auto grid_add_row = [](Grid& g) {
    assert(g.width);

    g.height ++;
    g.map.resize(g.width * g.height);
  };

  const auto LoadInput = [](auto f) {
    Grid g;
    std::string_view line;
    g.width = 0;
    g.height = 0;
    while (aoc::getline(f, line)) {
      grid_set_width(g, line.size());
      grid_add_row(g);

      size_t x = 0;
      const auto y = g.height - 1;
      for (const auto c : line) {
        assert((c >= 'a' && c <= 'z') || c == 'E' || c == 'S');
        DEBUG_LOG(x, y, c);
        grid_add(g, x, y, c);

        if (c == 'S') {
          g.start.first = x;
          g.start.second = y;
        } else if (c == 'E') {
          g.dst.first = x;
          g.dst.second = y;
        }
        x++;
      }
    }
    return g;
  };

  const auto print_grid = [](const Grid& g) {
    for (size_t y = 0; y < g.height; y++) {
      for (size_t x = 0; x < g.width; x++) {
        const auto off = grid_off(g, x, y);
        auto &p = g.map.at(off);
        if (p.second == -1) {
          std::cout << '.';
        } else {
          std::cout << p.second;
        }
      }
      std::cout << std::endl;
    }
  };

  const auto print_grid_heights = [](const Grid& g) {
    for (size_t y = 0; y < g.height; y++) {
      for (size_t x = 0; x < g.width; x++) {
        const auto off = grid_off(g, x, y);
        auto &p = g.map.at(off);
        std::cout << static_cast<char>(p.first + 'a');
      }
      std::cout << std::endl;
    }
  };

  void flood_fill(Grid& g, size_t x, size_t y, int32_t last, int64_t step) {
    auto& p = g.map.at(grid_off(g, x, y));
    // already colored
    DEBUG_LOG(x, y, last, p.first, p.second, step);
    if (p.second != -1 && step >= p.second) {
      return;
    }

    const auto diff = last - p.first;
    if (diff > 1) { return; }

    p.second = step++;
    if (x == g.start.first && y == g.start.second) {
      return;
    }
    if (x + 1 < g.width) {
      flood_fill(g, x + 1, y, p.first, step);
    }
    if (x > 0) {
      flood_fill(g, x - 1, y, p.first, step);
    }
    if (y + 1 < g.height) {
      flood_fill(g, x, y + 1, p.first, step);
    }
    if (y > 0) {
      flood_fill(g, x, y - 1, p.first, step);
    }
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Grid g;
  if (inTest) {
    g = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    g = LoadInput(f);
  }

  flood_fill(g, g.dst.first, g.dst.second, 26, 0);

  int64_t part1 = g.map.at(grid_off(g, g.start.first, g.start.second)).second;
  int64_t part2 = INT64_MAX;

  for (const auto& p : g.map) {
    if (p.first == 0 && p.second != -1) { part2 = std::min(p.second, part2); }
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
