#include "aoc/helpers.h"
#include <queue>
#include <vector>
#include <cmath>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.
Blueprint 2: Each ore robot costs 2 ore. Each clay robot costs 3 ore. Each obsidian robot costs 3 ore and 8 clay. Each geode robot costs 3 ore and 12 obsidian.)");
  constexpr int SR_Part1 = 33;
  constexpr int SR_Part2 = 0;

  struct MaterialSet {
    int ore;
    int clay;
    int obsidian;
    int geode;

    MaterialSet()
      : ore(0)
      , clay(0)
      , obsidian(0)
      , geode(0)
    { }
  };

  struct Blueprint {
    MaterialSet oreset;
    MaterialSet clayset;
    MaterialSet obsidianset;
    MaterialSet geodeset;

    MaterialSet max;
  };

  struct State {
    int64_t t;
    MaterialSet bag;
    MaterialSet bots;

    bool valid() const {
      return bag.clay >= 0 && bag.geode >= 0 && bag.obsidian >= 0 && bag.geode >= 0;
    }
  };

  std::ostream& operator<<(std::ostream& os, const MaterialSet& m) {
    os << "{" << " ore: " << m.ore << " clay: " << m.clay << " obsidian: " << m.obsidian << " geode: " << m.geode << " }";
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const State& m) {
    os << "{" << " t: " << m.t << " bag: " << m.bag << " bots: " << m.bots << " }";
    return os;
  }

  const auto canBuild = [](const MaterialSet& mineral, const MaterialSet& bag) {
    return bag.ore >= mineral.ore &&
      bag.clay >= mineral.clay &&
      bag.obsidian >= mineral.obsidian;
  };

  const auto build = [](const MaterialSet& mineral, MaterialSet& bag) {
    if (!canBuild(mineral, bag)) {
      return false;
    }

    bag.ore -= mineral.ore;
    bag.clay -= mineral.clay;
    bag.obsidian -= mineral.obsidian;
    bag.geode -= mineral.geode;

    return true;
  };

  const auto mine = [](State& state, int ticks = 1) {
    state.t += ticks;
    state.bag.ore += (ticks * state.bots.ore);
    state.bag.clay += (ticks * state.bots.clay);
    state.bag.obsidian += (ticks * state.bots.obsidian);
    state.bag.geode += (ticks * state.bots.geode);
  };

  const auto getOutput = [](const Blueprint& b, const State& s, int64_t deadline) {
    std::vector<State>out;
    State next{s};

    int64_t td = 0;
    while (!canBuild(b.geodeset, next.bag)) {
      mine(next);

      if (next.t >= deadline) {
        break;
      }
      td++;
    }

    if (next.t < deadline && canBuild(b.geodeset, next.bag)) {
      mine(next);
      build(b.geodeset, next.bag);
      next.bots.geode++;
      out.emplace_back(std::move(next));

      if (!td) { return out; }
    }

    if (s.bots.obsidian < b.max.obsidian) {
      td = 0;
      next = s;

      while (!canBuild(b.obsidianset, next.bag)) {
        mine(next);

        if (next.t >= deadline) {
          break;
        }
        td++;
      }

      if (next.t < deadline && canBuild(b.obsidianset, next.bag)) {
        mine(next);
        build(b.obsidianset, next.bag);
        next.bots.obsidian++;
        out.emplace_back(std::move(next));

        if (!td) { return out; }
      }
    }

    if (s.bots.clay < b.max.clay) {
      td = 0;
      next = s;

      while (!canBuild(b.clayset, next.bag)) {
        mine(next);

        if (next.t >= deadline) {
          break;
        }
        td++;
      }

      if (next.t < deadline && canBuild(b.clayset, next.bag)) {
        mine(next);
        build(b.clayset, next.bag);
        next.bots.clay++;
        out.emplace_back(std::move(next));

        if (!td) { return out; }
      }
    }

    if (s.bots.ore < b.max.ore) {
      next = s;

      if (next.bag.ore < b.oreset.ore) {
        const auto ticks = std::ceil(static_cast<float>(b.oreset.ore - next.bag.ore) / static_cast<float>(next.bots.ore));
        mine(next, ticks);
      }

      if (next.t < deadline) {
        mine(next);
        build(b.oreset, next.bag);

        next.bots.ore++;
        out.emplace_back(std::move(next));
      }
    }

    return out;
  };

  const auto runBlueprint = [](const Blueprint& b, int64_t maxtime) {
    State t0;
    t0.bots.ore = 1;
    t0.t = 0;
    std::queue<State>q;
    q.emplace(t0);

    int64_t max = 0;
    int64_t maxAt = 0;

    while (!q.empty()) {
      auto state = q.front(); q.pop();
      DEBUG_LOG(state);

      if (state.bag.geode + 1 < max && state.t >= maxAt) {
        // we can't possibly improve on our current best, so prune it
        continue;
      }

      if (state.bag.geode > max) {
        maxAt = state.t;
        max = state.bag.geode;
      }
      const auto out = getOutput(b, state, maxtime);
      for (const auto& s : out) {
        assert(s.valid());
        if (s.valid()) {
          q.emplace(s);
        }
      }
    }

    return max;
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::vector<Blueprint> blueprints;
    std::string_view line;
    while (aoc::getline(f, line)) {
      std::string_view part;
      std::vector<int> minerals;
      while (aoc::getline(line, part, ":. ")) {
        if (!aoc::is_numeric(part[0])) {
          continue;
        }
        minerals.push_back(aoc::stoi(part));
      }

      assert(minerals.size() == 7);
      Blueprint b;
      size_t i = 0;
      const auto id = minerals[i++];
      b.oreset.ore = minerals[i++];
      b.clayset.ore = minerals[i++];
      b.obsidianset.ore = minerals[i++];
      b.obsidianset.clay = minerals[i++];
      b.geodeset.ore = minerals[i++];
      b.geodeset.obsidian = minerals[i++];

      b.max.obsidian = b.geodeset.obsidian;
      b.max.clay = b.obsidianset.clay;
      b.max.ore = std::max(b.oreset.ore, std::max(b.clayset.ore, std::max(b.obsidianset.ore, b.geodeset.obsidian)));

      assert(i == minerals.size());
      r.first += id * runBlueprint(b, 24);
      blueprints.push_back(b);
    }
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
  int part2 = 0;  //21840

  std::tie(part1, part2) = r;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
