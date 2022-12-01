#include "aoc/helpers.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  std::string_view line;
  size_t max_cal = 0;
  size_t max_elf = 0;
  size_t elf_idx = 0;
  size_t cal = 0;
  while (aoc::getline(f, line, "\r\n", true)) {
    if (line.empty()) {
      elf_idx++;
      if (cal > max_cal) {
        max_elf = elf_idx;
        max_cal = cal;
      }
      cal = 0;

      continue;

    }
    cal += aoc::stoi(line);
    DEBUG_PRINT(max_elf << ":" << max_cal << ":" << line << ":" << cal) ;
  }

  elf_idx++;
  if (cal > max_cal) {
    max_elf = elf_idx;
    max_cal = cal;
  }
  cal = 0;

  aoc::print_result(1, max_cal);
  
  return 0;
}

