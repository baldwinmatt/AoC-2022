#include "aoc/helpers.h"
#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include <thread>
#include <algorithm>
#include <functional>
#include <queue>
#include "aoc/point.h"

using namespace std::chrono_literals;


namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(#.######
#>>.<^<#
#.<..<<#
#>v.><>#
#<^v^^>#
######.#)");
  constexpr int SR_Part1 = 18;
  constexpr int SR_Part2 = 54;

  using Blizzards = std::vector<aoc::point>;

  const auto blizzard_pos = [](aoc::CardinalDirection dir, const aoc::point p, const aoc::point dims, const int32_t n) {
    const auto grid_width = dims.x - 2;
    const auto grid_height = dims.y - 2;

    // first make our position 0-based
    aoc::point out{p - aoc::point{1, 1}};

    // Now, step n times in the given direction
    aoc::point step;
    switch(dir) {
      case aoc::CardinalDirection::North:
        step = aoc::point::up();
        break;
      case aoc::CardinalDirection::South:
        step = aoc::point::down();
        break;
      case aoc::CardinalDirection::East:
        step = aoc::point::right();
        break;
      case aoc::CardinalDirection::West:
        step = aoc::point::left();
        break;
      default:
        assert(false);
    }
    out += (step * n);

    // Now make it in bounds
    while (out.x < 0) {
      out.x += grid_width;
    }
    while (out.y < 0) {
      out.y += grid_height;
    }
    out.x %= grid_width;
    out.y %= grid_height;

    // and make it 1-based
    out += {1, 1};

    return out;
  };

  struct Map {
    int32_t width{0};
    int32_t height{0};

    aoc::point elves{1, 0};

    Blizzards north;
    Blizzards east;
    Blizzards south;
    Blizzards west;

    constexpr aoc::point entrance() const {
      return { 1, 0 };
    }
    const aoc::point exit() const {
      return { width - 2, height - 1 };
    }

    bool is_wall(aoc::point const& p) const {
      const auto start = entrance();
      const auto end = exit();
      
      return (p.x <= 0 || p.x >= width - 1 || p.y <= 0 || p.y >= height) &&
        !(p == start || p == end);
    }

    // determine if a given point is empty at this point in time
    bool empty(aoc::point pt, int32_t n) {
      // point is in a wall
      if (is_wall(pt)) {
        return false;
      }

      // for east (right) blizzards, go west from here:
      aoc::point s = blizzard_pos(aoc::CardinalDirection::West, pt, { width, height}, n);
      auto it = std::find(east.begin(), east.end(), s);
      if (it != east.end()) { return false; }

      s = blizzard_pos(aoc::CardinalDirection::East, pt, { width, height}, n);
      it = std::find(west.begin(), west.end(), s);
      if (it != west.end()) { return false; }

      s = blizzard_pos(aoc::CardinalDirection::North, pt, { width, height}, n);
      it = std::find(south.begin(), south.end(), s);
      if (it != south.end()) { return false; }

      s = blizzard_pos(aoc::CardinalDirection::South, pt, { width, height}, n);
      it = std::find(north.begin(), north.end(), s);
      if (it != north.end()) { return false; }

      return true;
    }

    size_t to_offset(const aoc::point& p) const {
      return p.x + (p.y * width);
    }

    size_t to_offset(int32_t x, int32_t y) const {
      return to_offset(aoc::point{x, y});
    }

    std::vector<aoc::point> generatePossibleMoves(aoc::point pos, int32_t n, const aoc::point& end) {
      std::vector<aoc::point> moves;

      // only move is to go to the target
      if ((end == exit() && pos + aoc::point::down() == end) ||
       (end == entrance() && pos + aoc::point::up() == end)) {
        moves.emplace_back(end);
        return moves;
      }

      constexpr std::array<aoc::point, 5>DIRECTIONS{
        aoc::point::origin(),
        aoc::point::up(),
        aoc::point::right(),
        aoc::point::down(),
        aoc::point::left(),
      };
      for (const auto& dir : DIRECTIONS) {
        const auto pt = pos + dir;
        if (empty(pt, n)) {
          moves.emplace_back(std::move(pt));
        }
      }

      return moves;
    }

    std::ostream& print(std::ostream& os, int32_t n) const {
      std::vector<char> out;
      out.resize(width * height, '.');

      const auto render_point = [&](const aoc::point p, char c) {
        const size_t off = to_offset(p);
        if (aoc::is_numeric(out[off])) {
          out[off]++;
        } else if (out[off] != '.') {
          out[off] = '2';
        } else {
          out[off] = c;
        }
      };
      render_point(elves, 'E');

      aoc::point dims{width, height};
      for (const auto& s : north) {
        const auto b = blizzard_pos(aoc::CardinalDirection::North, s, dims, n);
        render_point(b, '^');
      }
      for (const auto& s : east) {
        const auto b = blizzard_pos(aoc::CardinalDirection::East, s, dims, n);
        render_point(b, '>');
      }
      for (const auto& s : south) {
        const auto b = blizzard_pos(aoc::CardinalDirection::South, s, dims, n);
        render_point(b, 'v');
      }
      for (const auto& s : west) {
        const auto b = blizzard_pos(aoc::CardinalDirection::West, s, dims, n);
        render_point(b, '<');
      }

      for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
          const auto off = to_offset(x, y);
          if (out[off] == '.' && (y == 0 || x == 0 || y == height - 1 || x == width - 1)) {
            if ((x == 1 && y == 0) || (x == width - 2 && y == height - 1)) {
              os << ' ';
            } else {
              os << '#';
            }
          } else if (out[off] == 'E') {
            os << aoc::bold_on << out[off] << aoc::bold_off;
          } else {
            os << out[off];
          }
        }
        os << std::endl;
      }
      os << "Iteration: " << n;
      return os;
    }

    int64_t walk_to(int32_t iteration, aoc::point end) {
      using PointStepPair = std::pair<aoc::point, int32_t>;
      struct PointStepPairHash {
        std::size_t operator() (const PointStepPair& pair) const {
          aoc::point_hash hasher{};
          std::size_t seed = hasher(pair.first);
          aoc::hash_combine(seed, pair.second);
          return seed;
        }
      };
      using PointStepPairPriority = std::pair<PointStepPair, int64_t>;
      struct PriorityComparator {
        bool operator()(const PointStepPairPriority& l, const PointStepPairPriority& r) const {
          return l.second > r.second;
        }
      };

      std::priority_queue<PointStepPairPriority, std::vector<PointStepPairPriority>, PriorityComparator>queue;
      std::unordered_set<size_t>visited;

      queue.emplace(PointStepPairPriority{PointStepPair{elves, iteration}, 0});
      while (!queue.empty()) {
        auto& top = queue.top();
        auto item = top.first;
        DEBUG_LOG(item.first, item.second, top.second, end);
        queue.pop();

        if (item.first == end) {
          elves = item.first;
          return item.second;
        }

        const auto next_step = item.second + 1;

        const auto moves = generatePossibleMoves(item.first, next_step, end);
        for (const auto& move : moves) {
          PointStepPairHash hasher{};
          PointStepPair psp{move, next_step};
          const auto it = visited.emplace(hasher(psp));
          if (it.second) {
            psp.first = move;
            psp.second = next_step;
            const auto pri = move.manhattan(end) + item.second;
            queue.emplace(std::move(psp), pri);
          }
        }
      }
      assert(false);
      return 0;
    }
  };

  using State = std::pair<Map, int32_t>;

  std::ostream& operator<<(std::ostream& os, const State& in) {
    const auto& m = in.first;
    return m.print(os, in.second);
  }
  
  const auto LoadInput = [](auto f) {
    Map r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      assert(r.width == 0 || static_cast<size_t>(r.width) == line.size());
      r.width = line.size();
      int32_t x = 0;
      for (const auto c : line) {
        switch (c) {
          case '<':
            r.west.emplace_back(aoc::point{x, r.height});
            break;
          case '>':
            r.east.emplace_back(aoc::point{x, r.height});
            break;
          case 'v':
            r.south.emplace_back(aoc::point{x, r.height});
            break;
          case '^':
            r.north.emplace_back(aoc::point{x, r.height});
            break;
          default:
            assert(c == '.' || c == '#');
            break;
        }
        x++;
      }
      r.height++;
    }

    std::sort(r.north.begin(), r.north.end());
    std::sort(r.east.begin(), r.east.end());
    std::sort(r.south.begin(), r.south.end());
    std::sort(r.west.begin(), r.west.end());

    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  State r;
  if (inTest) {
    r.first = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r.first = LoadInput(f);
  }

  r.second = 0;
  r.second += r.first.walk_to(r.second, r.first.exit());
  DEBUG_LOG(r);
  int32_t part1 = r.second;
  r.second += r.first.walk_to(r.second, r.first.entrance());
  DEBUG_LOG(r);
  r.second += r.first.walk_to(r.second, r.first.exit());
  DEBUG_LOG(r);
  int32_t part2 = r.second;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
