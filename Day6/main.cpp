#include "aoc/helpers.h"
#include <vector>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  const auto LookbackUnique = [](const auto& l) {
    DEBUG_LOG(l);
    std::vector<bool>char_set(255);

    for (const auto c : l) {
      if (char_set[static_cast<int>(c)]) return false;
      char_set[static_cast<int>(c)] = 1;
    }
    return true;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  size_t part1 = 0;
  size_t part2 = 0;

  assert(f.size() > 4);
  for (size_t i = 0; (!part1 || !part2) && i < f.size() - 4; i++) {
    std::string_view packet{f.data() + i, 4};
    std::string_view message{f.data() + i, std::min(14LU, f.size() - i)};
    if (!part1 && LookbackUnique(packet)) {
      part1 = i + 4;
    }
    if (!part2 && message.size() == 14 && LookbackUnique(message)) {
      part2 = i + 14;
    }
  }
  aoc::print_results(part1, part2);

  return 0;
}
