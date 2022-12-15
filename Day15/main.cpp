#include "aoc/helpers.h"
#include <map>
#include <unordered_set>
#include <vector>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Sensor at x=2, y=18: closest beacon is at x=-2, y=15
Sensor at x=9, y=16: closest beacon is at x=10, y=16
Sensor at x=13, y=2: closest beacon is at x=15, y=3
Sensor at x=12, y=14: closest beacon is at x=10, y=16
Sensor at x=10, y=20: closest beacon is at x=10, y=16
Sensor at x=14, y=17: closest beacon is at x=10, y=16
Sensor at x=8, y=7: closest beacon is at x=2, y=10
Sensor at x=2, y=0: closest beacon is at x=2, y=10
Sensor at x=0, y=11: closest beacon is at x=2, y=10
Sensor at x=20, y=14: closest beacon is at x=25, y=17
Sensor at x=17, y=20: closest beacon is at x=21, y=22
Sensor at x=16, y=7: closest beacon is at x=15, y=3
Sensor at x=14, y=3: closest beacon is at x=15, y=3
Sensor at x=20, y=1: closest beacon is at x=15, y=3)");
  constexpr int SR_Part1 = 26;
  constexpr int64_t SR_Part2 = 56000011;

  // Sensor -> Radius
  using Report = std::map<aoc::Point, int64_t>;

  const auto LoadInput = [](auto f) {
    Report r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      std::vector<int>points;
      aoc::parse_as_integers(line, "=,:",
        [&points](const int64_t v) { points.push_back(v); });
      assert(points.size() == 4);
      aoc::Point sensor{points[0], points[1]};
      aoc::Point beacon{points[2], points[3]};
      const auto radius = aoc::manhattan(sensor, beacon);
      r.emplace(sensor, radius);
    }
    return r;
  };

  using Ranges = std::vector<aoc::Point>;

  const auto reduceCoveredRanges = [](Ranges& r) {
    Ranges out;
    std::sort(r.begin(), r.end());
    auto last = r.front();
    for (const auto& span : r) {
      // completely encompassed
      if (span.first >= last.first && span.second <= last.second) {
        DEBUG_LOG("encompassed range", span.first, span.second, last.first, last.second);
        continue;
      } else if ((span.first <= last.first && span.second > last.first) ||
                (span.first <= last.second && span.second > last.second))
      { // overlap, so extend
        last.first = std::min(span.first, last.first);
        last.second = std::max(span.second, last.second);
        DEBUG_LOG("extended range", span.first, span.second, last.first, last.second);
      } else { // no overlap
        DEBUG_LOG("distinct range", span.first, span.second, last.first, last.second);
        out.emplace_back(last);
        last = span;
      }
    }
    out.emplace_back(last);
    return out;
  };

  const auto getCoveredRanges = [](const Report& r, int64_t row) {
    Ranges covered_range;
    for (const auto& [sensor, radius] : r) {

      // if the target row is not in the signal area, skip
      if (row > sensor.second + radius ||
          row < sensor.second - radius) {
        continue;
      }

      // reduce the radius to account for the row to signal offset
      const int distance = (row > sensor.second) ?
        ((sensor.second + radius) - row) :
        row - (sensor.second - radius);

      if (!distance) { continue; }

      // all these points are covered
      DEBUG_LOG(row, sensor.first - distance, sensor.first + distance);
      covered_range.emplace_back(sensor.first - distance, sensor.first + distance);
    }
    return reduceCoveredRanges(covered_range);
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Report r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int part1 = 0;
  int64_t part2 = 0;

  {
    const int64_t row = inTest ? 10 : 2000000;
    const auto ranges = getCoveredRanges(r, row);

    for (const auto& last : ranges) {
      part1 += std::abs(last.second - last.first);
    }
  }

  
  const int64_t max = inTest ? 20 : 4000000;

  for (int64_t y = 0; y < max; y++) {
    const auto ranges = getCoveredRanges(r, y);
    DEBUG_LOG(y, ranges.size());
    if (ranges.size() == 1) { continue; }

    if (ranges[0].second < 0 || ranges[1].first > max) {
      continue;
    }
    const auto x = (ranges[0].second + 1);
    DEBUG_LOG(y, x);
    part2 = y + (x * 4000000);
    break;
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
