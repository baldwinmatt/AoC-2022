#include "aoc/helpers.h"
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <queue>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
Valve BB has flow rate=13; tunnels lead to valves CC, AA
Valve CC has flow rate=2; tunnels lead to valves DD, BB
Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE
Valve EE has flow rate=3; tunnels lead to valves FF, DD
Valve FF has flow rate=0; tunnels lead to valves EE, GG
Valve GG has flow rate=0; tunnels lead to valves FF, HH
Valve HH has flow rate=22; tunnel leads to valve GG
Valve II has flow rate=0; tunnels lead to valves AA, JJ
Valve JJ has flow rate=21; tunnel leads to valve II)");
  constexpr int SR_Part1 = 1651;
  constexpr int SR_Part2 = 1707; 

#ifndef NDEBUG
  STRING_CONSTANT(STR_VALVE, "Valve");
#endif
  STRING_CONSTANT(STR_VALVES, "valves");
  STRING_CONSTANT(STR_VALVE_, "valve");

  // Raw input
  struct Valve {
    std::string name;
    int64_t flow;
    int64_t id;

    Valve()
      : flow(0)
      , id(0)
    {
    }

    using ValveRefList = std::vector<Valve *>;
    using ValveCost = std::pair<Valve *, int64_t>;

    std::vector<ValveCost>working;
    ValveRefList exits;
  };

  using ValveCost = std::pair<Valve *, int64_t>;
  using ValveList = std::vector<Valve>;
  using ValveRefList = std::vector<Valve *>;

  const auto compressValves = [](ValveRefList& working) {

    std::map<std::pair<std::string, std::string>, int64_t> costs;

    const auto cost = [&costs](Valve* start, Valve* end) {
      struct comparator {
        bool operator()(const ValveCost& l, const ValveCost& r) {
          return l.second > r.second;
        }
      };

      std::pair<std::string, std::string>key{start->name, end->name};
      const auto rit = costs.find(key);
      if (rit != costs.end()) {
        DEBUG_LOG(start->name, end->name, rit->second);
        return rit->second;
      }

      std::priority_queue<ValveCost,
        std::vector<ValveCost>,
        comparator> queue;

      queue.emplace(start, 0);
      while (!queue.empty()) {
        auto item = queue.top(); queue.pop();

        if (item.first->name == end->name) {
          key.first = end->name;
          key.second = start->name;
          costs.emplace(std::move(key), item.second);
          DEBUG_LOG(start->name, end->name, item.second);
          return item.second;
        }

        for (auto conn : item.first->exits) {
          queue.emplace(conn, item.second + 1);
        }
      }
      assert(false);
      return static_cast<int64_t>(-1);
    };

    for (auto valve : working) {
      for (auto conn : working) {
        if (conn->name == valve->name || conn->name == "AA") {
          continue;
        }

        valve->working.emplace_back(conn, cost(valve, conn));
      }
    }
  };

  struct EnqueuedValve {
    Valve *valve;
    int64_t time;
    int64_t released;
    int64_t opened;

    EnqueuedValve(Valve *v, int64_t t, int64_t r, int64_t o)
      : valve(v)
      , time(t)
      , released(r)
      , opened(o)
    { }

    EnqueuedValve()
      : EnqueuedValve(nullptr, 0, 0, 0)
    { }

  };

  const auto solve = [](Valve* start) {
    int64_t max = 0;

    std::set<std::tuple<int64_t, int64_t, int64_t>>added;
    std::queue<EnqueuedValve>queue;
    
    for (auto v : start->working) {
      queue.emplace(v.first, 30 - v.second, 0, 0);
    }

    while (!queue.empty()) {
      auto node = queue.front(); queue.pop();

      if (!(node.opened & node.valve->id)) {
        node.time--;
        node.opened |= node.valve->id;
        node.released += node.valve->flow * node.time;
      }

      DEBUG_LOG(node.valve->name, node.released, max);
      max = std::max(max, node.released);

      if (node.time < 1) {
        continue;
      }

      for (auto v : node.valve->working) {
        if (node.time - v.second <= 0 ||
          (node.opened & v.first->id))
        {
          continue;
        }

        const auto rit = added.emplace(v.first->id, node.time, node.opened);
        if (rit.second) {
          DEBUG_LOG(v.first->name, node.time - v.second, node.released, node.opened);
          queue.emplace(v.first, node.time - v.second, node.released, node.opened);
        }
      }
    }

    assert(max);

    return max;
  };

  const auto LoadInput = [](auto f) {
    std::string_view line;
    ValveList vl;

    std::map<std::string, std::vector<std::string>> conns;
    int64_t id = 1;

    while (aoc::getline(f, line)) {
      std::string_view part;
      std::vector<std::string_view>parts;
      while (aoc::getline(line, part, " =;,")) {
        parts.push_back(part);
      }
      assert(parts.size() >= 11);
      assert(parts[0] == STR_VALVE);
      size_t i = 1;
      Valve v;
      v.name = parts[i++];
      v.id = id;
      id <<= 1;
      while (i < parts.size() && !aoc::is_numeric(parts[i][0])) { i++; }
      v.flow = aoc::stoi(parts[i++]);
      while (i < parts.size() && parts[i] != STR_VALVES && parts[i] != STR_VALVE_) { i++; }
      i++;
      assert(i < parts.size());
      const auto cit = conns.emplace(v.name, std::vector<std::string>());
      while (i < parts.size()) {
        cit.first->second.emplace_back(parts[i++]);
      }

      vl.emplace_back(std::move(v));
    }

    ValveRefList working;
    for (size_t i = 0; i < vl.size(); i++) {
      auto &v = vl[i];
      if (v.name == "AA" || v.flow > 0) {
        working.emplace_back(&vl[i]);
        DEBUG_LOG(v.name, v.flow);
      }
      auto vc = conns.at(v.name);
      for (const auto e : vc) {
        for (size_t j = 0; j < vl.size(); j++) {
          if (e == vl[j].name) {
            v.exits.emplace_back(&vl[j]);
          }
        }
      }
    }

    {
      aoc::AutoTimer compress("compress");
      compressValves(working);
    }
    return vl;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  ValveList vl;
  if (inTest) {
    vl = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    vl = LoadInput(f);
  }

  Valve *start = nullptr;
  for (size_t i = 0; i < vl.size(); i++) {
    if (vl[i].name == "AA") {
      start = &vl[i];
      break;
    }
  }
  assert(start);
  int64_t part1 = 0;
  {
    aoc::AutoTimer t("part1");
    part1 = solve(start);
  }
  int part2 = 0;

  aoc::print_results(part1, part2); // 1720, 2582

  if (inTest) {
    aoc::assert_result(part2, SR_Part2);
    aoc::assert_result(part1, SR_Part1);
  }

  return 0;
}
