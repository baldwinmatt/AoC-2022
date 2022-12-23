#include "aoc/helpers.h"
#include <algorithm>
#include <vector>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(        ...#
        .#..
        #...
        ....
...#.......#
........#...
..#....#....
..........#.
        ...#....
        .....#..
        .#......
        ......#.

10R5L5R10L4R5L5)");
  constexpr int SR_Part1 = 6032;
  constexpr int SR_Part2 = 5031;

  enum class Tile {
    None = 0,
    Space,
    Wall,
  };

  using Row = std::vector<Tile>;
  using Grid = std::vector<Row>;

  enum class Direction {
    Right = 0,
    Down,
    Left,
    Up,
  };

  const auto turnLeft = [](const Direction d) {
    switch (d) {
      case Direction::Right:
        return Direction::Up;
      case Direction::Down:
        return Direction::Right;
      case Direction::Left:
        return Direction::Down;
      case Direction::Up:
        return Direction::Left;
    }
  };

  const auto turnRight = [](const Direction d) {
    switch (d) {
      case Direction::Right:
        return Direction::Down;
      case Direction::Down:
        return Direction::Left;
      case Direction::Left:
        return Direction::Up;
      case Direction::Up:
        return Direction::Right;
    }
  };

  const auto step = [](const aoc::Point pt, const Direction dir) {
    switch (dir) {
        case Direction::Up:
            return pt + aoc::Point{ 0, -1 };
        case Direction::Down:
            return pt + aoc::Point{ 0, 1 };
        case Direction::Right:
            return pt + aoc::Point{ 1, 0 };
        case Direction::Left:
            return pt + aoc::Point{ -1, 0 };
    }
  };

  std::ostream& operator<<(std::ostream& os, const Direction dir) {
    switch (dir) {
      case Direction::Up:
        os << "^"; break;
      case Direction::Down:
        os << "v"; break;
      case Direction::Right:
        os << ">"; break;
      case Direction::Left:
        os << "<"; break;
    }

    return os;
  }

  std::ostream& operator<<(std::ostream& os, const aoc::Point p) {
    os << "{ " << p.first << ", " << p.second << " }";
    return os;
  }

  class Map {
  public:
    Map(size_t tw)
      : pos (-1, -1)
      , dir (Direction::Right)
      , cube (true) // gets inverted in reset
      , width (0)
      , tw(tw)
    {
    }

    Map()
      : Map(0)
    { }

    void reset() {
      pos.second = 0;
      pos.first = 0;
      while (g.back()[pos.first] != Tile::Space) {
        assert(static_cast<size_t>(pos.first) < g.back().size());
        pos.first++;
      }

      dir = Direction::Right;
      cube = !cube;

      DEBUG_LOG(pos, dir, cube);
    }

    void addRow(const std::string_view row) {
      g.emplace_back();
      auto& r = g.back();
      r.reserve(row.size());

      for (const auto c: row) {
        switch (c) {
          case ' ':
            r.push_back(Tile::None);
            break;
          case '.':
            r.push_back(Tile::Space);
            break;
          case '#':
            r.push_back(Tile::Wall);
            break;
          default:
            assert(false);
            break;
        }
      }
      width = std::max(width, r.size());

      if (pos.first == -1) {
        // locate start
        reset();
      }
    }

    void moveForward(int steps) {
      while (steps) {
        steps--;

        auto new_pos = step(pos, dir);
        auto new_dir = dir;

        if (getAt(new_pos) == Tile::None) {
          auto np = !cube ?
            wrap_flat(new_pos) :
            wrap_cube(pos);
          new_pos = np.first;
          new_dir = np.second;
        }

        if (g[new_pos.second][new_pos.first] == Tile::Wall) {
          return;
        }

        pos = new_pos;
        dir = new_dir;
        DEBUG_LOG(pos, dir);
      }
    }

    void turn(char c) {
      switch (c) {
        case 'R':
          dir = turnRight(dir);
          break;
        case 'L':
          dir = turnLeft(dir);
          break;
        default:
          assert(false);
      }
      DEBUG_LOG(c, dir);
    }

    int64_t getCode() const {
      int64_t score = 1000 * (pos.second + 1) +
                      4 * (pos.first + 1) +
                      static_cast<int>(dir);
      return score;
    }

    friend std::ostream& operator<<(std::ostream& os, const Map& m);

  protected:
    aoc::Point pos;
    Direction dir;
    bool cube;
    size_t width;
    size_t tw;

    Grid g;

    bool inGrid(const aoc::Point p) const {
      return p.second >= 0 && static_cast<size_t>(p.second) < g.size() &&
        p.first >= 0 && static_cast<size_t>(p.first) < g[p.second].size();
    }

    Tile getAt(const aoc::Point p) const {
      if (inGrid(p)) {
        return g[p.second][p.first];
      }

      return Tile::None;
    }

    std::pair<aoc::Point, Direction> wrap_flat(aoc::Point new_pos) const {
      // walk to first non-air tile or end of grid in the given direction
      while (inGrid(new_pos) && getAt(new_pos) == Tile::None) {
        new_pos = step(new_pos, dir);
      }

      if (new_pos.second >= static_cast<int64_t>(g.size())) {
        // wrap around top
        new_pos.second = 0;
        while (getAt(new_pos) == Tile::None) {
          assert(static_cast<size_t>(new_pos.second) < g.size());
          new_pos.second++;
        }
      } else if (new_pos.second < 0) {
        // wrap around bottom
        new_pos.second = g.size() - 1;
        while (new_pos.first >= static_cast<int64_t>(g[new_pos.second].size()) ||
              getAt(new_pos) == Tile::None) {
          assert(new_pos.second >= 0);
          new_pos.second--;
        }
      } else if (new_pos.first >= static_cast<int64_t>(g[new_pos.second].size())) {
        // wrap around right
        new_pos.first = 0;
        while (getAt(new_pos) == Tile::None) {
          new_pos.first++;
        }
      } else if (new_pos.first < 0) {
        // wrap around left
        const auto& r = g[new_pos.second];
        new_pos.first = r.size() - 1;
        while (getAt(new_pos) == Tile::None) {
          assert(new_pos.first > 0);
          new_pos.first--;
        }
      }

      return {new_pos, dir};
    }

    std::pair<aoc::Point, Direction> wrap_cube(aoc::Point pos) const {
      // reduce to face index
      const aoc::Point face{pos.first / tw, pos.second / tw};

      constexpr aoc::Point TOP { 1, 0 };
      constexpr aoc::Point FRONT { 1, 1 };
      constexpr aoc::Point BOTTOM { 1, 2 };
      constexpr aoc::Point RIGHT { 2, 0 };
      constexpr aoc::Point LEFT { 0, 2 };
      constexpr aoc::Point BACK { 0, 3 };

      switch (dir) {
        case Direction::Up:
          if (LEFT == face) {
            // Front
            return { {tw, pos.first + tw}, Direction::Right };
          } else if (TOP == face) {
            // Back
            return { { 0, 3 * tw + pos.first }, Direction::Right };
          } else if (RIGHT == face) {
            // 
            return { { tw - 1, 3 * tw + pos.first }, Direction::Left };
          }
          assert(false);
          break;
        case Direction::Down:
          if (BACK == face) {
            return { { pos.first + 2 * tw, 0 }, Direction::Down };
          } else if (BOTTOM == face) {
            return { { tw - 1, 2 * tw + pos.first }, Direction::Left };
          } else if (RIGHT == face) {
            return { { 2 * tw - 1, pos.first - tw }, Direction::Left };
          }
          assert(false);
          break;
        case Direction::Left:
          if (FRONT == face) {
            return { { 0, pos.second - tw}, Direction::Down };
          } else if (TOP == face) {
            return { { 0, 3 * tw - pos.second }, Direction::Right };
          } else if (LEFT == pos) {
            return { { tw, 3 * tw - pos.second }, Direction::Right };
          } else if (BACK == pos) {
            return { { tw, 3 * tw - pos.second }, Direction::Right };
          }
          assert(false);
          break;
        case Direction::Right:
          if (FRONT == face || TOP == face) {
            return { {tw, pos.second}, Direction::Right };
          } else if (BOTTOM == face) {
            return { {tw -1, pos.second}, Direction::Right };
          }
          assert(false);
          break;
      }
      //assert(false);
      return {pos, dir};
    }
  };

  std::ostream& operator<<(std::ostream& os, const Map& m) {
    for (int64_t y = 0; static_cast<size_t>(y) < m.g.size(); y++) {
      const auto& r = m.g[y];
      for (int64_t x = 0; static_cast<size_t>(x) < r.size(); x++) {
        if (m.pos == aoc::Point{x, y}) {
          os << m.dir;
          continue;
        }
        switch (r[x]) {
          case Tile::None:
            os << " "; break;
          case Tile::Space:
            os << "."; break;
          case Tile::Wall:
            os << "#"; break;
        }
      }
      os << std::endl;
    }

    os << "{ " << m.pos.first << ", " << m.pos.second << " }";

    return os;
  }

  using Input = std::pair<Map, std::string>;

  const auto LoadInput = [](auto f, size_t tw) {
    Input r{tw, ""};
    std::string_view line;
    bool in_grid = true;
    while (aoc::getline(f, line, "\r\n", true)) {
      in_grid = in_grid && !line.empty();
      if (in_grid) {
        r.first.addRow(line);
      } else if (!line.empty()) {
        r.second = line;
      }
    }
    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Input r;
  if (inTest) {
    r = LoadInput(SampleInput, 4);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f, 50);
  }

  const char *pos = r.second.data();
  const char *end = r.second.data() + r.second.size();

  while (pos != end) {
    if (!aoc::is_numeric(*pos)) {
      r.first.turn(*pos);
      pos++;
    } else {
      int steps = 0;
      while (aoc::is_numeric(*pos)) {
        steps *= 10;
        steps += (*pos - '0');
        pos++;
      }
      r.first.moveForward(steps);
    }
  }
  aoc::cls(std::cout);
  std::cout << r.first << std::endl;

  int part1 = r.first.getCode();
  r.first.reset();
  pos = r.second.data();

  while (pos != end) {
    if (!aoc::is_numeric(*pos)) {
      r.first.turn(*pos);
      pos++;
    } else {
      int steps = 0;
      while (aoc::is_numeric(*pos)) {
        steps *= 10;
        steps += (*pos - '0');
        pos++;
      }
      r.first.moveForward(steps);
    }
  }
  int part2 = r.first.getCode();

  aoc::print_results(part1, part2);
  // 65368  77388
  // 156166
  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
