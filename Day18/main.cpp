#include "aoc/helpers.h"
#include <vector>
#include <queue>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(2,2,2
1,2,2
3,2,2
2,1,2
2,3,2
2,2,1
2,2,3
2,2,4
2,2,6
1,2,5
3,2,5
2,1,5
2,3,5)");
  constexpr int SR_Part1 = 64;
  constexpr int SR_Part2 = 58;

  struct Point {
    int x;
    int y;
    int z;
  };

  constexpr Point OFFSET{2, 2, 2};

  Point operator+(const Point& lhs, const Point& rhs) {
      Point out{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
      return out;
  }

  std::ostream& operator<<(std::ostream& os, const Point& p) {
      os << "{ " << p.x << ", " << p.y << ", " << p.z << " }";
      return os;
  }
  Point& operator+=(Point& lhs, const Point& rhs) {
      lhs.x += rhs.x;
      lhs.y += rhs.y;
      lhs.z += rhs.z;
      return lhs;
  }


  using Row = std::vector<bool>;
  using Plane = std::vector<Row>;
  using Grid = std::vector<Plane>;

  const auto getGridRow = [](Grid &g, Point p) -> Row& {
      if (static_cast<int>(g.size()) < p.z + 2) {
        g.resize(p.z + 2);
      }
      auto& plane = g[p.z];
      if (static_cast<int>(plane.size()) < p.y + 2) {
        plane.resize(p.y + 2);
      }
      auto& row = plane[p.y];
      if (static_cast<int>(row.size()) < p.x + 2) {
        row.resize(p.x + 2);
      }
      return row;
  };

  const std::vector<Point>Dirs = {
    { -1, 0, 0 },
    { 1, 0, 0 },
    { 0, -1, 0 },
    { 0, 1, 0 },
    { 0, 0, -1 },
    { 0, 0, 1 },
  };

  const auto getAdjacent = [](Grid &g, Point p) {
    int count = 0;
    for (const auto& d : Dirs) {
      const auto c = p + d;
      const auto& row = getGridRow(g, c);
      count += row[c.x];
    }
    DEBUG_LOG(p, count);
    return count;
  };

  const auto getExternalSurfaceArea = [](Grid& g, const Point max) {
    int area = 0;

    Grid visited;

    std::queue<Point> next;
    next.push(max);
    while (!next.empty()) {
      Point p = next.front(); next.pop();

      auto& v = getGridRow(visited, p);
      if (v[p.x]) { continue; } // alredy been here
      v[p.x] = true;            // mark it
      DEBUG_LOG(p, v[p.x]);

      area += getAdjacent(g, p);

      for (const auto& d : Dirs) {
        const auto c = p + d;
        auto& r = getGridRow(g, c);
        auto& rv = getGridRow(visited, c);
        if (!rv[c.x] && !r[c.x] &&
              c.x > 0 && c.y > 0 && c.z > 0 &&
              c.x < max.x + 2 && c.y < max.y + 2 && c.z < max.z + 2) {
          next.push(c);
        }
      }
    }

    return area;
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;

    Grid g;
    Point max{0, 0, 0};
    while (aoc::getline(f, line)) {
      std::vector<int>parts;
      aoc::parse_as_integers(line, ",", [&parts](const auto part) {
        parts.push_back(part);
      });

      assert(parts.size() == 3);
      Point p{ parts[0], parts[1], parts[2] };
      p += OFFSET;
      auto& row = getGridRow(g, p);

      row[p.x] = true;

      int adj = getAdjacent(g, p);
      r.first += 6;
      r.first -= (2 * adj);

      max.x = std::max(max.x, p.x);
      max.y = std::max(max.y, p.y);
      max.z = std::max(max.z, p.z);
    }

    r.second = getExternalSurfaceArea(g, max);
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
