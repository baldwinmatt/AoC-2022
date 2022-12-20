#include "aoc/helpers.h"
#include <queue>
#include <vector>

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
  };

  struct State {
    int64_t t;
    MaterialSet bag;
    MaterialSet bots;
  };

  std::ostream& operator<<(std::ostream& os, const MaterialSet& m) {
    os << "{" << " ore: " << m.ore << " clay: " << m.clay << " obsidian: " << m.obsidian << " geode: " << m.geode << " }";
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const State& m) {
    os << "{" << " t: " << m.t << " bag: " << m.bag << " bots: " << m.bots << " }";
    return os;
  }

  const auto mine = [](const MaterialSet& mineral, MaterialSet& bag) {
    if (bag.ore < mineral.ore ||
      (mineral.clay && bag.clay <= mineral.clay) ||
      (mineral.obsidian && bag.obsidian <= mineral.obsidian))
    { return false; }

    bag.ore -= mineral.ore;
    bag.clay -= mineral.clay;
    bag.obsidian -= mineral.obsidian;
    bag.geode -= mineral.geode;

    return true;
  };

  const auto getOutput = [](const Blueprint& b, const State& s) {
    std::optional<State>out;

    State next{s};

    while (mine(b.geodeset, next.bag)) {
      next.bots.geode++;
      out.emplace(std::move(next));
    }

    while (mine(b.obsidianset, next.bag)) {
      next.bots.obsidian++;
      out.emplace(std::move(next));
    }
    while (mine(b.clayset, next.bag)) {
      next.bots.clay++;
      out.emplace(std::move(next));
    }
    while (mine(b.oreset, next.bag)) {
      next.bots.ore++;
      out.emplace(std::move(next));
    }
    return out;
  };

  const auto runBlueprint = [](const Blueprint& b, int64_t maxtime) {
    State t0;
    t0.bots.ore = 1;
    t0.t = 0;
    std::queue<State>q;
    q.emplace(t0);

    int max = 0;

    while (!q.empty()) {
      auto state = q.front(); q.pop();
      DEBUG_LOG(state);

      if (state.t == maxtime) {
        max = std::max(max, state.bag.geode);
        continue;
      }

      state.t++;
      auto out = getOutput(b, state);
      state.bag.ore += state.bots.ore;
      state.bag.clay += state.bots.clay;
      state.bag.obsidian += state.bots.obsidian;
      state.bag.geode += state.bots.geode;
      q.emplace(std::move(state));

      if (out.has_value()) {
        q.emplace(out.value());
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
      assert(i == minerals.size());
      r.first += id * runBlueprint(b, 4);
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
  int part2 = 0;

  std::tie(part1, part2) = r;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
