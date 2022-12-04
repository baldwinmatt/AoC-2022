#include "aoc/helpers.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;
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
  }

  aoc::print_results(part1, part2);

  return 0;
}
