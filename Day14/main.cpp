#include "aoc/helpers.h"
#include <vector>
#include <map>
#include <thread>

using namespace std::chrono_literals;

namespace {
  bool visualize =
#ifndef NDEBUG
    true;
#else
    false;
#endif

  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9)");
  constexpr int SR_Part1 = 24;
  constexpr int SR_Part2 = 93;

 enum class Tile {
    Air = 0,
    Rock = '#',
    Sand = 'O',
  };

  using SparseGrid = std::map<aoc::Point, Tile>;

  class Map {
  public:
    Map()
      : width(0)
      , depth(0)
      , ptmin(INT_MAX, INT_MAX)
      , ptmax(INT_MIN, INT_MIN)
    {
    }

    void add_rock_line(aoc::Point f, aoc::Point t) {
      auto diff = t - f;
      const auto xstep = aoc::sgn(diff.first);
      const auto ystep = aoc::sgn(diff.second);
      diff = aoc::abs(diff);

      const auto max = aoc::max(f, t);
      const auto min = aoc::min(f, t);

      resize_grid(min, max);
      ptmax = aoc::max(max, ptmax);

      if (diff.second) {
        for (auto y = 0; y <= diff.second; y++) {
          sg.emplace(f, Tile::Rock);
          f += { xstep, ystep };
        }
      } else if (diff.first) {
        for (auto x = 0; x <= diff.first; x++) {
          sg.emplace(f, Tile::Rock);
          f += { xstep, ystep };
        }
      }
    }

    // returns true if sand settles
    bool drop_sand(const aoc::Point& epoch, bool fill) {
      aoc::Point pt = epoch;
      const aoc::Point DOWN{0, 1};
      const aoc::Point DOWN_LEFT{-1, 1};
      const aoc::Point DOWN_RIGHT{1, 1};

      const auto floor = ptmax.second + (2 * fill);

      if (sg.find(pt) != sg.end()) {
        return false;
      }

      aoc::Point last = pt;
      while (pt.second < floor) {
        last = pt;
        auto cur = sg.find(last);

        aoc::Point down = pt + DOWN;
        aoc::Point down_left = pt + DOWN_LEFT;
        aoc::Point down_right = pt + DOWN_RIGHT;

        resize_grid(down_left, down_right);

        if (sg.find(down) == sg.end()) {
          pt = down;
        } else if (sg.find(down_left) == sg.end()) {
          pt = down_left;
        } else if (sg.find(down_right) == sg.end()) {
          pt = down_right;
        }
        
        if (pt == last) {
          DEBUG_LOG(last);
          sg.emplace(pt, Tile::Sand);
          return true;
        }

        if (cur != sg.end()) {
          sg.erase(cur);
        }

        if (visualize) {
          std::this_thread::sleep_for(5ms);
          aoc::cls(std::cout);
          std::cout << *this;
        }
      }
      // If fill, we emplace the sand at the point before we fell through the floor
      if (fill) {
        DEBUG_LOG(last);
        sg.emplace(last, Tile::Sand);
        return true;
      }

      return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const Map& m);

  protected:

    void resize_grid(const aoc::Point& min, const aoc::Point& max) {
      ptmin = aoc::min(min, ptmin);
      width = std::max(width, static_cast<size_t>(max.first) + 1);
      depth = std::max(depth, static_cast<size_t>(max.second) + 1);
    }

    size_t width;
    size_t depth;
    aoc::Point ptmin;
    aoc::Point ptmax;
    SparseGrid sg;
  };

  std::ostream& operator<<(std::ostream& os, const Map& m) {
    for (size_t y = m.ptmin.second; y < m.depth; y++) {
      for (size_t x = m.ptmin.first; x < m.width; x++) {
        const auto t = m.sg.find({x, y});
        if (t == m.sg.end()) {
          os << ' ';
        } else {
          os << static_cast<char>(t->second);
        }
      }
      os << std::endl;
    }
    return os;
  }

  const auto LoadInput = [](auto f) {
    std::string_view line;
    Map m;
    while (aoc::getline(f, line)) {
      std::vector<int>points;
      aoc::parse_as_integers(line, " ->,",
        [&points](const int64_t v) { points.push_back(v); });

      assert(points.size() % 2 == 0);
      assert(points.size() >= 4);
      aoc::Point last = {points[0],points[1]};
      for (size_t i = 0; i < points.size(); i += 2) {
        aoc::Point cur{points[i],points[i + 1]};
        m.add_rock_line(last, cur);
        last.swap(cur);
      }
    }
    return m;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Map r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int part1 = 0;
  int part2 = 0;
  {
    aoc::AutoTimer t("part 1");
    while (r.drop_sand({500, 0}, false)) {
      part1++;
    }
  }


  {
    part2 = part1;
    aoc::AutoTimer t("part 2");
    while (r.drop_sand({500, 0}, true)) {
      part2++;
    }
  }

  DEBUG(aoc::cls(std::cout));
  DEBUG(std::cout << r);

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
