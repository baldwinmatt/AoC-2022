#include "aoc/helpers.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  using Range = std::pair<int64_t, int64_t>;

  const auto IsFullyEnclosed = [](const Range r1, const Range r2) {
    return (r1.first <= r2.first && r1.second >= r2.second);
  };

  const auto IsOverlapping = [](const Range r1, const Range r2) {
    return (r1.first <= r2.first && r1.second >= r2.first) ||
      (r1.first <= r2.second && r1.second >= r2.second);
  };

  const auto ParseRange = [](const std::string_view s) {
    Range r;

    const auto sep = s.find('-');
    std::string_view p = s.substr(0, sep);
    r.first = aoc::stoi(p);
    p = s.substr(sep + 1);
    r.second = aoc::stoi(p);

    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  size_t part1 = 0;
  size_t part2 = 0;

  std::string_view line;
  while (aoc::getline(f, line, "\r\n")) {
    DEBUG_LOG(line);
    std::string_view part;
    Range r1, r2;
    int idx = 0;
    while (aoc::getline(line, part, ",")) {
      idx++;
      if (idx == 1) {
        r1 = ParseRange(part);
      } else if (idx == 2) {
        r2 = ParseRange(part);
      } else {
        throw std::runtime_error("Bad input");
      }
    }
    if (idx != 2) {
      throw std::runtime_error("Bad input");
    }
    part1 += (IsFullyEnclosed(r1, r2) || IsFullyEnclosed(r2, r1));
    part2 += (IsOverlapping(r1, r2) || IsOverlapping(r2, r1));
  }

  aoc::print_results(part1, part2);

  return 0;
}
