#include "aoc/helpers.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  const auto FindCommonElement = [](const auto l, const auto r) {
    for (auto c : l) {
      for (auto d : r) {
        if ( c == d) { return c; }
      }
    }
    return '\0';
  };
  const auto FindCommonElements = [](const auto l, const auto r) {
    std::vector<char> s;
    for (auto c : l) {
      for (auto d : r) {
        if ( c == d) { s.push_back(c); }
      }
    }
    return s;
  };

  const auto GetPriority = [](const auto c) {
    size_t offset = 0;
    char base = 0;
    if (c >= 'a' && c <= 'z') {
      offset = 1;
      base = 'a';
    } else if (c >= 'A' && c <= 'Z') {
      offset = 27;
      base = 'A';
    } else {
      throw std::runtime_error("Bad input");
    }
    return (c - base + offset);
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  std::string_view line;
  size_t part1 = 0;
  size_t part2 = 0;
  size_t i = 0;
  std::vector<char> reduce;
  std::string_view l1;
  while (aoc::getline(f, line, "\r\n")) {
    if (line.size() % 2) { throw std::invalid_argument("Bad line"); }

    const auto comp1 = line.substr(0, line.size() / 2);
    const auto comp2 = line.substr(line.size() / 2);

    const auto c = FindCommonElement(comp1, comp2);
    DEBUG_LOG(comp1, comp2, c);

    switch (i) {
      case 0:
        l1 = line;
        i++;
        break;
      case 1:
        reduce = FindCommonElements(line, l1);
        i++;
        break;
      case 2:
      {
        const auto d = FindCommonElement(reduce, line);
        part2 += GetPriority(d);
        i = 0;
      }
      default:
        break;
    }
    if (c) {

      part1 += GetPriority(c);
    }
  }

  aoc::print_results(part1, part2);

  return 0;
}

