#include "aoc/helpers.h"
#include <vector>
#include <set>
#include <unordered_set>
#include <thread>
#include <algorithm>
#include <functional>
#include <queue>

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
  constexpr int SR_Part2 = 0;

  using Blizzards = std::vector<aoc::Point>;

  const auto blizzard_pos = [](aoc::CardinalDirection dir, const aoc::Point p, const aoc::Point dims, const size_t n) {
    const auto grid_width = dims.first - 2;
    const auto grid_height = dims.second - 2;
    const auto modx = n % grid_width;
    const auto mody = n % grid_height;

    aoc::Point out(p);

    switch(dir) {
      case aoc::CardinalDirection::North:
        // pos.y = (init.y - 1 + grid_height - mody) % grid_height + 1
        out.second = (p.second - 1 + grid_height - mody) % grid_height + 1;
        break;
      case aoc::CardinalDirection::South:
        //(init.y - 1 + mody) % grid_height + 1
        out.second = (p.second - 1 + mody) % grid_height + 1;
        break;
      case aoc::CardinalDirection::East:
        //(init.x - 1 + modx) % grid_width + 1
        out.first = (p.first - 1 + modx) % grid_width + 1;
        break;
      case aoc::CardinalDirection::West:
        // pos.x = (init.x - 1 + grid_width - modx) % grid_width + 1
        out.first = (p.first - 1 + grid_width - modx) % grid_width + 1;
        break;
      default:
        assert(false);
    }

    return out;
  };

  struct Map {
    size_t width;
    size_t height;

    Blizzards north;
    Blizzards east;
    Blizzards south;
    Blizzards west;

    aoc::Point start;
    aoc::Point end;
    aoc::Point elves;

    Map()
      : width(0)
      , height(0)
      , start(1, 0)
      , end(-1, -1)
      , elves(start) {
    }

    // determine if a given point is empty at this point in time
    bool empty(aoc::Point pt, size_t n) {
      // entrance and exit are always empty
      if (pt == start ||
          pt == end) {
        return true;
      }

      const auto grid_width = width - 2;
      const auto grid_height = height - 2;
      const auto modx = n % grid_width;
      const auto mody = n % grid_height;

      // point is out of bounds
      if (pt.first < 1 || pt.first > static_cast<int64_t>(grid_width) ||
          pt.second < 1 || pt.second > static_cast<int64_t>(grid_height)) {
        return false;
      }

      // east stepped n times.
      // pos.x = (init.x - 1 + modx) % grid_width + 1
      // => init.x = (pos.x - 1 + grid_width - modx) % grid_width + 1
      aoc::Point s{ (pt.first - 1 + grid_width - modx) % grid_width + 1, pt.second };
      auto it = std::find(east.begin(), east.end(), s);
      if (it != east.end()) { return false; }

      // west stepped -n times
      // pos.x = (init.x - 1 + grid_width - modx) % grid_width + 1
      // => init.x = (pos.x - 1 + modx) % grid_width + 1
      s.first = (pt.first - 1 + modx) % grid_width + 1;
      it = std::find(west.begin(), west.end(), s);
      if (it != west.end()) { return false; }

      s.first = pt.first;

      // south stepped n times
      // pos.y = (init.y - 1 + mody) % grid_height + 1
      // => init.y = (pos.y - 1 + grid_height - modx) % grid_height + 1
      s.second = (pt.second - 1 + grid_height - modx) % grid_height + 1;
      it = std::find(south.begin(), south.end(), s);
      if (it != south.end()) { return false; }

      // north stepped -n times
      // pos.y = (init.y - 1 + grid_height - mody) % grid_height + 1
      // => init.y = (pos.y - 1 + mody) % grid_height + 1
      s.second = (pt.second - 1 + mody) % grid_height + 1;
      it = std::find(north.begin(), north.end(), s);
      if (it != north.end()) { return false; }

      return true;
    }

    size_t to_offset(const aoc::Point& p) const {
      return p.first + (p.second * width);
    }

    size_t to_offset(size_t x, size_t y) const {
      return to_offset(aoc::Point{x, y});
    }

    std::vector<aoc::Point> generatePossibleMoves(aoc::Point pos, int64_t n) {
      std::vector<aoc::Point> moves;

      // only move is to go to the target
      if (pos == (end - aoc::Point{0, 1})) {
        moves.emplace_back(end);
        return moves;
      }

      // try N, S, E, W and stationary
      const std::vector<aoc::Point>DIRECTIONS{
        { 0, 0 },
        { 0, -1 },
        { 1, 0 },
        { 0, 1 },
        { -1, 0 },
      };
      for (const auto& dir : DIRECTIONS) {
        const auto pt = pos + dir;
        if (empty(pt, n)) {
          moves.emplace_back(std::move(pt));
        }
      }

      return moves;
    }

    std::ostream& print(std::ostream& os, size_t n) const {
      std::vector<char> out;
      out.resize(width * height, '.');

      const auto render_point = [&](const aoc::Point p, char c) {
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

      aoc::Point dims{width, height};
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

      for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
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

    int64_t step(size_t iteration) {
      using PointStepPair = std::pair<aoc::Point, int64_t>;
      struct PointStepPairHash {
        std::size_t operator() (const PointStepPair& pair) const {
          aoc::PointHash hasher{};
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
      std::unordered_set<PointStepPair, PointStepPairHash>visited;

      if (end == aoc::Point{-1, -1}) {
        end = aoc::Point{width - 2, height - 1};
      }

      queue.emplace(PointStepPairPriority{PointStepPair{start, 0}, 0});
      while (!queue.empty()) {
        auto& top = queue.top();
        auto item = top.first;
        DEBUG(elves = item.first; print(std::cout, item.second); std::cout << std::endl);
        DEBUG_LOG(top.second);
        DEBUG(std::this_thread::sleep_for(100ms));
        queue.pop();

        if (item.first == end) {
          return item.second;
        }

        const auto moves = generatePossibleMoves(item.first, iteration + item.second + 1);
        for (const auto& move : moves) {
          PointStepPair psp{move, item.second + 1};
          const auto it = visited.emplace(std::move(psp));
          DEBUG_LOG(move, it.second);
          if (it.second && item.second + 1 < 25) {
            psp.first = move;
            psp.second = item.second + 1;
            const auto pri = aoc::manhattan(move, end) + item.second;
            DEBUG_LOG(psp.first, psp.second, pri);
            queue.emplace(std::move(psp), pri);
          }
        }
      }
      assert(false);
      return 0;
    }
  };

  using State = std::pair<Map, size_t>;

  std::ostream& operator<<(std::ostream& os, const State& in) {
    const auto& m = in.first;
    return m.print(os, in.second);
  }
  
  const auto LoadInput = [](auto f) {
    Map r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      assert(r.width == 0 || r.width == line.size());
      r.width = line.size();
      int64_t x = 0;
      for (const auto c : line) {
        switch (c) {
          case '<':
            r.west.emplace_back(aoc::Point{x, r.height});
            break;
          case '>':
            r.east.emplace_back(aoc::Point{x, r.height});
            break;
          case 'v':
            r.south.emplace_back(aoc::Point{x, r.height});
            break;
          case '^':
            r.north.emplace_back(aoc::Point{x, r.height});
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

  int64_t part1 = r.first.step(0);

#if 0
  for (r.second = 0; r.second < 10; r.second++) {
    std::cout << aoc::cls << r << std::endl;
    std::this_thread::sleep_for(1s);
  }
#endif

  int part2 = 0;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
