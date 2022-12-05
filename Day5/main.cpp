#include "aoc/helpers.h"
#include <vector>
#include <stack>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  using Stack = std::stack<char>;
  using Stacks = std::vector<Stack>;

  enum class Context {
    None,
    Move,
    From,
    To
  };

  const auto MoveCrates = [](Stacks& stacks, int64_t move, int64_t from, int64_t to) {
    assert(from > 0 && from <= (int64_t)stacks.size());
    assert(to > 0 && to <= (int64_t)stacks.size());
    Stack& src = stacks.at(from - 1);
    Stack& dst = stacks.at(to - 1);

    assert((int64_t)src.size() >= move);
    for ( ; move > 0; move--) {
      const auto c = src.top();
      src.pop();
      dst.push(c);
    }
  };
  
  const auto MoveCrates_part2 = [](Stacks& stacks, int64_t move, int64_t from, int64_t to) {
    assert(from > 0 && from <= (int64_t)stacks.size());
    assert(to > 0 && to <= (int64_t)stacks.size());
    Stack& src = stacks.at(from - 1);
    Stack& dst = stacks.at(to - 1);

    assert((int64_t)src.size() >= move);
    Stack tmp;
    for ( ; move > 0; move--) {
      const auto c = src.top();
      src.pop();
      tmp.push(c);
    }

    while (!tmp.empty()) {
      dst.push(tmp.top());
      tmp.pop();
    }
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  std::string_view line;
  Stacks stacks_in;
  // Parse stacks
  while (aoc::getline(f, line, "\r\n", true)) {
    if (line.empty()) { break; }
    DEBUG_LOG(line);
    if (line.find('[') == std::string_view::npos) { continue; }
    assert((line.size() + 1) % 4 == 0);
    const auto stack_count = (line.size() + 1) / 4;
    if (stacks_in.size() < stack_count) {
      stacks_in.resize(stack_count);
    }

    for (size_t idx = 0; idx < stack_count; idx++) {
      const auto offset = (idx * 4) + 1;
      if (offset >= line.size()) { continue;}

      const auto c = line[offset];
      assert((c >= 'A' && c <= 'Z') || c == ' ');
      if (c == ' ') { continue; }

      stacks_in[idx].push(c);
    }
  }

  // stacks are created upside down, so flip them
  Stacks stacks;
  size_t i = 0;
  stacks.resize(stacks_in.size());
  for (auto &s : stacks_in) {
    while(!s.empty()) {
      stacks[i].push(s.top());
      s.pop();
    }
    i++;
  }
  Stacks stacks_part2 = stacks;
  // Move the crates
  while (aoc::getline(f, line, "\r\n")) {
    DEBUG_LOG(line);
    std::string_view part;
    Context ctx = Context::None;
    std::pair<int64_t, int64_t>move_from;
    while (aoc::getline(line, part, " ")) {
      if (part == std::string_view("move")) {
        ctx = Context::Move;
        continue;
      } else if (part == std::string_view("from")) {
        ctx = Context::From;
        continue;
      } else if (part == std::string_view("to")) {
        ctx = Context::To;
        continue;
      }

      const auto i = aoc::stoi(part);
      switch (ctx) {
        case Context::Move:
          move_from.first = i;
          continue;
        case Context::From:
          move_from.second = i;
          continue;
        case Context::To:
          MoveCrates(stacks, move_from.first, move_from.second, i);
          MoveCrates_part2(stacks_part2, move_from.first, move_from.second, i);
          continue;
        case Context::None:
          break;
      }

      throw std::runtime_error("Bad input");
    }
  }

  std::string part1;
  for (const auto& s : stacks) {
    if (s.empty()) { continue; }
    part1.append(1, s.top());
  }
  std::string part2;
  for (const auto& s : stacks_part2) {
    if (s.empty()) { continue; }
    part2.append(1, s.top());
  }
  aoc::print_results(part1, part2);

  return 0;
}
