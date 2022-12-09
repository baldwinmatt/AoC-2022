#include "aoc/helpers.h"
#include <array>
#include <set>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(R 4
U 4
L 3
D 1
R 4
D 1
L 5
R 2)");
  constexpr int SR_Part1 = 13;
  constexpr int SR_Part2 = 1;

  using PointSet = std::set<aoc::Point>;
  using Elements = std::array<aoc::Point, 10>;

  constexpr aoc::Point Up{0, 1};
  constexpr aoc::Point Down{0, -1};
  constexpr aoc::Point Left{-1, 0};
  constexpr aoc::Point Right{1, 0};

  aoc::Point operator+(const aoc::Point& lhs, const aoc::Point& rhs) {
    aoc::Point out{lhs.first + rhs.first, lhs.second + rhs.second};
    return out;
  }

  aoc::Point& operator+=(aoc::Point& lhs, const aoc::Point& rhs) {
    lhs.first += rhs.first;
    lhs.second += rhs.second;
    return lhs;
  }

  aoc::Point operator-(const aoc::Point& lhs, const aoc::Point& rhs) {
    aoc::Point out{lhs.first - rhs.first, lhs.second - rhs.second};
    return out;
  }

  template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
  }

  const auto Step = [](PointSet& part1, PointSet& part2, Elements& elems, const aoc::Point dir, int step) {
    while (step) {
      // Move the head
      elems[0] += dir;
      DEBUG_LOG(elems[0].first, elems[0].second);

      for (size_t i = 1; i < 10; i++) {
        const auto& prev = elems[i - 1];
        auto& tail = elems[i];
        const auto diff = prev - tail;
        const auto diff_x = std::abs(diff.first);
        const auto diff_y = std::abs(diff.second);
        if (!diff_x || !diff_y) {
          // need to move in one plane
          if (diff_x > 1) {
            tail.first += sgn(diff.first);
          } 
          if (diff_y > 1) {
            tail.second += sgn(diff.second);
          }
        } else if (diff_x != 1 || diff_y != 1) {
          // need to move in both planes
          aoc::Point d{ sgn(diff.first), sgn(diff.second) };
          tail += d;
        }
      }
      if (part1.emplace(elems[1]).second) {
        DEBUG_LOG(elems[1].first, elems[1].second);
      }
      part2.emplace(elems[9]);
      step--;
    } 
  };

  const auto MoveUp = [](PointSet& part1, PointSet& part2, Elements& elems, int64_t d) {
    Step(part1, part2, elems, Up, d);
  };

  const auto MoveDown = [](PointSet& part1, PointSet& part2, Elements& elems, int64_t d) {
    Step(part1, part2, elems, Down, d);
  };

  const auto MoveLeft = [](PointSet& part1, PointSet& part2, Elements& elems, int64_t d) {
    Step(part1, part2, elems, Left, d);
  };

  const auto MoveRight = [](PointSet& part1, PointSet& part2, Elements& elems, int64_t d) {
    Step(part1, part2, elems, Right, d);
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    aoc::Point H{0, 0};
    Elements elems{ H, H, H, H, H, H, H, H, H, H };

    std::set<aoc::Point> part1;
    std::set<aoc::Point> part2;

    while (aoc::getline(f, line)) {
      DEBUG_LOG(line);
      assert(line.size() >= 3);
      assert(line[1] == ' ');
      const auto c = line[0];
      const auto d = aoc::stoi(line.substr(2));
      switch (c) {
        case 'U':
          MoveUp(part1, part2, elems, d);
          break;
        case 'D':
          MoveDown(part1, part2, elems, d);
          break;
        case 'R':
          MoveRight(part1, part2, elems, d);
          break;
        case 'L':
          MoveLeft(part1, part2, elems, d);
          break;
        default:
          assert(false);
      }
    }

    r.first = part1.size();
    r.second = part2.size();
    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Result r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int part1 = 0;
  int part2 = 0;

  std::tie(part1, part2) = r;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
