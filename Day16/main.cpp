#include "aoc/helpers.h"
#include <vector>
#include <map>
#include <functional>

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
    std::vector<std::string> exits;
  };

  using CompressedExit = std::pair<std::string_view, int64_t>;
  // Zero nodes removed, with cost to next nodes
  struct CompressedValve {
    std::string name;
    int64_t flow;
    std::vector<CompressedExit> exits;
  };

  using ValveList = std::vector<Valve>;
  using ValveRef = std::reference_wrapper<Valve>;
  using CompressedValveList = std::map<std::string_view, ValveRef>;

#if 0
  const auto NameToInt = [](const std::string_view& name) {
    uint32_t v = 0;
    assert(name.size() < 4);
    for (const auto c : name) {
      assert(c >= 'A' && c <= 'Z');
      v |= (c - 'A');
      v <<= 5;
    }
    DEBUG_LOG(name, v);
    return v;
  };

  const auto IntToName = [](uint32_t v) {
    std::string s;
    while (v > 0) {
      char c = static_cast<char>(v & 0xff) + 'A';
      s.push_back(c);
      v >>= 5;
    }

    if (s.size() == 0) {
      s.push_back('A');
    }
    if (s.size() % 2) {
      s.push_back('A');
    }

    return std::reverse(s.begin(), s.end());
  };

  const auto CompressValves = [](const ValveList& vl) {
    std::for_each(vl.begin(), vl.end(), [](const auto& f) {
      if (f.flow == 0) { return; }

      //dijkstra_full(f, f.exits)
    });
    for (const auto& v : vl) {
      (void)v;
      DEBUG_LOG(v.name);
    }
  };
#endif

  const auto LoadInput = [](auto f) {
    std::string_view line;
    ValveList vl;
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
      v.name = parts[i++]; //NameToInt(parts[i++]);
      while (i < parts.size() && !aoc::is_numeric(parts[i][0])) { i++; }
      v.flow = aoc::stoi(parts[i++]);
      while (i < parts.size() && parts[i] != STR_VALVES && parts[i] != STR_VALVE_) { i++; }
      i++;
      assert(i < parts.size());
      while (i < parts.size()) {
        v.exits.emplace_back(parts[i++]);
      }

      vl.emplace_back(std::move(v));
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

  int part1 = 0;
  int part2 = 0;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
