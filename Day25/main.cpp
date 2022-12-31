#include "aoc/helpers.h"

namespace {
  using Result = std::pair<int64_t, int64_t>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(1=-0-2
12111
2=0=
21
2=01
111
20012
112
1=-1=
1-12
12
1=
122)");
  STRING_CONSTANT(SR_Part1, "2=-1=0");
  constexpr int SR_Part2 = 0;


  const auto snafu_to_dec_digit = [](char in) {
    switch (in) {
      case 2:
      case '2':
        return 2;
      case 1:
      case '1':
        return 1;
      case '0':
      case 0:
        return 0;
      case '-':
        return -1;
      case '=':
        return -2;
      default:
        assert(false);
    }
    return 0;
  };

  const auto snafu_to_dec = [](std::string_view in) {
    int64_t out = 0;
    int64_t base = 1;
    for (auto c = in.crbegin(); c != in.crend(); c++) {
      out += base * snafu_to_dec_digit(*c);
      base *= 5;
    }
    return out;
  };

  const auto dec_to_snafu_rem = [](int64_t in) {
    int64_t mod = in % 5;
    int64_t rem = in / 5;
    char c = '\0';

    switch (mod) {
      case 0:
        c = '0';
        break;
      case 1:
        c = '1';
        break;
      case 2:
        c = '2';
        break;
      case 3:
        c = '=';
        rem ++;
        break;
      case 4:
        c = '-';
        rem ++;
        break;
      default:
        assert(false);
    }
    return std::pair{c, rem};
  };

  const std::string dec_to_snafu(int64_t in) {
    std::string out;
    const auto n_rem = dec_to_snafu_rem(in);
    if (n_rem.second) {
      out = dec_to_snafu(n_rem.second);
    }
    out += n_rem.first;
    return out;
  }

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    while (aoc::getline(f, line)) {
      const auto s = snafu_to_dec(line);
      DEBUG_LOG(s, line);
      r.first += s;
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

  int64_t part2 = 0;
  const auto part1 = dec_to_snafu(r.first);

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
