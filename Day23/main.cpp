#include "aoc/helpers.h"
#include <map>
#include <set>
#include <vector>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(....#..
..###.#
#...#.#
.#...##
#.###..
##.#.##
.#..#..)");
  constexpr int SR_Part1 = 110;
  constexpr int SR_Part2 = 20;

  using Grid = std::set<aoc::Point>;

  struct Points {
    unsigned n:1;
    unsigned ne:1;
    unsigned e:1;
    unsigned se:1;
    unsigned s:1;
    unsigned sw:1;
    unsigned w:1;
    unsigned nw:1;
  };

  struct Elf {
    aoc::Point pos;

    Points points;

    Elf(aoc::Point p, const Grid& g)
      : pos(p) {

      points.n = g.count(p + aoc::Point{0, -1});
      points.ne = g.count(p + aoc::Point{1, -1});
      points.e = g.count(p + aoc::Point{1, 0});
      points.se = g.count(p + aoc::Point{1, 1});
      points.s = g.count(p + aoc::Point{0, 1});
      points.sw = g.count(p + aoc::Point{-1, 1});
      points.w = g.count(p + aoc::Point{-1, 0});
      points.nw = g.count(p + aoc::Point{-1, -1});
    }

    bool idle() const {
      constexpr Points idle_point{0};
      return *reinterpret_cast<const uint8_t*>(&idle_point) == *reinterpret_cast<const uint8_t*>(&points);
    }

    std::pair<bool, aoc::Point> check_north(int r) const {
      if (!points.n && !points.nw && !points.ne) {
        return {true, pos + aoc::Point{0, -1}};
      } else if (r) {
        return check_south(r - 1);
      }
      return { false, pos};
    }
    std::pair<bool, aoc::Point> check_south(int r) const {
      if (!points.s && !points.se && !points.sw) {
        return {true, pos + aoc::Point{0, 1}};
      } else if (r) {
        return check_west(r - 1);
      }
      return { false, pos};
    }
    std::pair<bool, aoc::Point> check_west(int r) const {
      if (!points.w && !points.nw && !points.sw) {
        return {true, pos + aoc::Point{-1, 0}};
      } else if (r) {
        return check_east(r - 1);
      }
      return { false, pos};
    }
    std::pair<bool, aoc::Point> check_east(int r) const {
      if (!points.e && !points.ne && !points.se) {
        return {true, pos + aoc::Point{1, 0}};
      } else if (r) {
        return check_north(r - 1);
      }
      return { false, pos};
    }
    std::pair<bool, aoc::Point> check_dir(aoc::CardinalDirection dir) const {

      switch (dir) {
        case aoc::CardinalDirection::North: return check_north(4);
        case aoc::CardinalDirection::East: return check_east(4);
        case aoc::CardinalDirection::South: return check_south(4);
        case aoc::CardinalDirection::West: return check_west(4);
        default: assert(false);
      }

      return { false, pos};
    }
  };

  struct Map {
    Grid g;
    aoc::Point tr;
    aoc::Point bl;

    Map()
      : tr(INT64_MIN, INT64_MIN)
      , bl(INT64_MAX, INT16_MAX)
    {}

    int64_t area() const {
      const auto bounds = tr - bl + aoc::Point{1, 1};
      DEBUG_LOG(tr, bl, bounds, g.size());
      return (bounds.first * bounds.second) - g.size();
    }

    void addElf(const aoc::Point p) {
      assert(!g.count(p));
      g.emplace(p);

      tr = aoc::max(tr, p);
      bl = aoc::min(bl, p);
    }
  };

  std::ostream& operator<<(std::ostream& os, const Map& m) {
    for (int64_t y = m.bl.second; y <= m.tr.second; y++) {
      for (int64_t x = m.bl.first; x <= m.tr.first; x++) {
        os << (m.g.count({x, y}) ? '#' : '.');
      }
      os << std::endl;
    }
    return os;
  }

  const auto LoadInput = [](auto f) {
    Map r;
    std::string_view line;
    aoc::Point pt{0, 0};
    while (aoc::getline(f, line)) {
      for (const auto c : line) {
        assert(c == '.' || c == '#');
        if (c == '#') {
          r.addElf(pt);
          DEBUG_LOG(pt.first, pt.second);
        }
        pt.first++;
      }
      pt.second++;
      pt.first = 0;
    }
    return r;
  };

  const auto next = [](aoc::CardinalDirection dir) {
    switch (dir) {
      case aoc::CardinalDirection::North:
        return aoc::CardinalDirection::South;
      case aoc::CardinalDirection::South:
        return aoc::CardinalDirection::West;
      case aoc::CardinalDirection::West:
        return aoc::CardinalDirection::East;
      case aoc::CardinalDirection::East:
        return aoc::CardinalDirection::North;
      default:
        assert(false);
    }
    return dir;
  };

  Map turn(const Map& elves, aoc::CardinalDirection dir) {
    std::vector<std::pair<aoc::Point, aoc::Point>> proposed;
    std::set<aoc::Point>new_pos;
    std::set<aoc::Point>blocked;

    for (const auto e : elves.g) {
      const Elf elf(e, elves.g);
      DEBUG_LOG(e.first, e.second, elf.idle());
      if (elf.idle()) {
        proposed.emplace_back(e, e);
        continue;
      }

      const auto np = elf.check_dir(dir);
      if (np.first) {
        proposed.emplace_back(e, np.second);
        auto r = new_pos.emplace(np.second);
        if (!r.second) {
          blocked.emplace(np.second);
        }
      } else {
        proposed.emplace_back(e, e);
      }
    }

    Map r;

    for (const auto &p : proposed) {
      if (blocked.count(p.second)) {
        r.addElf(p.first);
      } else {
        r.addElf(p.second);
      }
    }

    return r;
  }
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
  aoc::CardinalDirection dir = aoc::CardinalDirection::North;
  int64_t part1 = 0;
  int64_t part2 = 0;
  size_t i = 0;
  while (!part1 || !part2) {
    DEBUG(std::cout << aoc::cls << "i: " << i << std::endl << r << std::endl; std::this_thread::sleep_for(10ms));
    auto nr = turn(r, dir);
    dir = next(dir);
    i++;
    if (i == 10) {
      part1 = nr.area();
    }
    if (nr.g == r.g) {
      part2 = i;
    }
    r = nr;
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
