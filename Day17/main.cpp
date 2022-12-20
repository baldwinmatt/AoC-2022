#include "aoc/helpers.h"
#include <vector>
#include <array>
#include <set>
#include <map>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(>>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>)");
  constexpr int64_t SR_Part1 = 3068;
  constexpr int64_t SR_Part2 = 1514285714288;

  using Pattern = std::string;

  using Rock = std::set<aoc::Point>;
  using Rocks = std::vector<Rock>;

  const Rocks ROCKS = {
    {
      { 0, 0 }, { 1, 0 }, { 2, 0 }, {3, 0 },
    },
    {
                { 1, 2 },
      { 0, 1 }, { 1, 1 }, { 2, 1 },
                { 1, 0 },
    },
    {
                          { 2, 2 },
                          { 2, 1 },
      { 0, 0 }, { 1, 0 }, { 2, 0 },
    },
    {
      { 0, 3 },
      { 0, 2 },
      { 0, 1 },
      { 0, 0 },
    },
    {
      { 0, 1 }, { 1, 1 },
      { 0, 0 }, { 1, 0 },
    },
  };

  const auto LoadInput = [](auto f) {
    std::string_view line;

    aoc::getline(f, line);
    return Pattern{line.data(), line.size()};
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Pattern p;
  if (inTest) {
    p = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    p = LoadInput(f);
  }

  assert(p.find_first_not_of("<>") == std::string::npos);
  assert(ROCKS.size() == 5);

  int64_t part1 = 0;
  int64_t part2 = 0;

  using Cave = std::set<aoc::Point>;

  int64_t height = 0;
  size_t iteration = 0;

  Cave settled;

  const auto canMove = [&settled](const Rock& rock, const aoc::Point& pos, const aoc::Point& move) {
    // each point must be clear
    for (const auto pt : rock) {
      const auto pt2 = pos + pt + move;
      // Must stay in the bounds
      if (pt2.first < 0 || pt2.first > 6 || pt2.second < 0) { return false; }
      // Must not be settled
      if (settled.count(pt2)) { return false; }
    }
    return true;
  };

  const aoc::Point LEFT{ -1, 0 };
  const aoc::Point RIGHT{ 1, 0 };
  const aoc::Point DOWN{ 0, -1 };

  using Key = std::tuple<uint8_t, size_t, std::string>;
  using Cache = std::map<Key, std::pair<int64_t, int64_t>>;

  Cache cache;

  const auto cacheKey = [](const Cave& settled, int64_t height) {
    std::string key;
    for (int64_t x = 0; x < 7; x++) {
      int64_t y;
      for (y = height; y >= 0; y--) {
        aoc::Point pt{x, y};
        if (settled.count(pt)) {
          break;
        }
      }
      if (!key.empty()) {
        key += ',';
      }
      key.append(std::to_string(height - y));
    }
    return key;
  };

  int64_t dropped = 0;
  uint8_t rock_idx = 0;
  std::vector<int64_t>heights;
  heights.push_back(height); // initial state
  while (!part1 && !part2) {

    const auto& rock = ROCKS[rock_idx];
    aoc::Point rock_pos{2, height + 3};
    bool did_settle = false;

    do {
      // push one unit by jet-stream
      const auto dir = p[iteration];
      iteration = (iteration + 1) % p.size();
      const auto& move = (dir == '<') ? LEFT : RIGHT;

      // pushed by jet
      if (canMove(rock, rock_pos, move)) {
        rock_pos += move;
      }
      // fall by one
      if (canMove(rock, rock_pos, DOWN)) {
        rock_pos += DOWN;
      } else {
        // settle the rock
        for (const auto pt : rock) {
          const auto r = pt + rock_pos;
          settled.emplace(r);
          height = std::max(height, r.second + 1);
        }

        did_settle = true;
      }
    } while (!did_settle);

    heights.emplace_back(height);

    dropped ++;
    rock_idx = (rock_idx + 1) % ROCKS.size();

    Key cache_key = { rock_idx, iteration, cacheKey(settled, height) };
    DEBUG_LOG(height, dropped, (int32_t)rock_idx, iteration, std::get<2>(cache_key));
    std::pair<int64_t, int64_t> val{ dropped, height };
    const auto r = cache.emplace(std::move(cache_key), std::move(val));

    // We've seen this pattern before, so can consult our cache
    if (!r.second) {
      val = r.first->second;

      std::vector<int64_t> results;
      int64_t targets[] = { 2022, 1000000000000 };

      for ( int64_t n : targets) {
        int64_t x = (n - val.first) / (dropped - val.first);
        int64_t rx = (n - val.first) % (dropped - val.first);

        DEBUG_LOG(n, x, rx, val.first, val.second, height, heights[val.first + rx]);

        // scale
        int64_t hx = (x * (height - val.second)) + (heights[val.first + rx]);
        results.push_back(hx);
      }

      part1 = results[0];
      part2 = results[1];
    }
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
