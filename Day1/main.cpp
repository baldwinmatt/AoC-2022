#include "aoc/helpers.h"
#include <numeric>
#include <vector>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  std::string_view line;

  std::vector<int64_t>v;
  std::make_heap(v.begin(), v.end(), std::greater<>{});
  int64_t cal = 0;
  while (aoc::getline(f, line, "\r\n", true)) {
    if (line.empty()) {
      v.push_back(cal);
      std::push_heap(v.begin(), v.end(), std::greater<>{});
      cal = 0;
      continue;
    }
    cal += aoc::stoi(line);
  }

  v.push_back(cal);
  std::push_heap(v.begin(), v.end(), std::greater<>{});
  std::sort_heap(v.begin(), v.end(), std::greater<>{});

  size_t part1 = v.front();
  size_t idx = 0;
  const auto first_three = [&idx](size_t l, size_t r) {
    idx++;
    if (idx > 3) { return l; }
    return l + r;
  };
  size_t part2 = std::accumulate(v.begin(), v.end(), 0U, first_three);

  aoc::print_results(part1, part2);
  
  return 0;
}

