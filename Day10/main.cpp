#include "aoc/helpers.h"

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(addx 15
addx -11
addx 6
addx -3
addx 5
addx -1
addx -8
addx 13
addx 4
noop
addx -1
addx 5
addx -1
addx 5
addx -1
addx 5
addx -1
addx 5
addx -1
addx -35
addx 1
addx 24
addx -19
addx 1
addx 16
addx -11
noop
noop
addx 21
addx -15
noop
noop
addx -3
addx 9
addx 1
addx -3
addx 8
addx 1
addx 5
noop
noop
noop
noop
noop
addx -36
noop
addx 1
addx 7
noop
noop
noop
addx 2
addx 6
noop
noop
noop
noop
noop
addx 1
noop
noop
addx 7
addx 1
noop
addx -13
addx 13
addx 7
noop
addx 1
addx -33
noop
noop
noop
addx 2
noop
noop
noop
addx 8
noop
addx -1
addx 2
addx 1
noop
addx 17
addx -9
addx 1
addx 1
addx -3
addx 11
noop
noop
addx 1
noop
addx 1
noop
noop
addx -13
addx -19
addx 1
addx 3
addx 26
addx -30
addx 12
addx -1
addx 3
addx 1
noop
noop
noop
addx -9
addx 18
addx 1
addx 2
noop
noop
addx 9
noop
noop
noop
addx -1
addx 2
addx -37
addx 1
addx 3
noop
addx 15
addx -21
addx 22
addx -6
addx 1
noop
addx 2
addx 1
noop
addx -10
noop
noop
addx 20
addx 1
addx 2
addx 2
addx -6
addx -11
noop
noop
noop)");
  constexpr int SR_Part1 = 13140;
  constexpr int SR_Part2 = 0;

  class CPU {
    public:
      CPU()
        : cycle_(0)
        , running_sum_(0)
        , reg_x_(1)
      {
      }

      void addx(int64_t v) {
        step();
        step();
        reg_x_ += v;
      }

      void noop() {
        step();
      }

      int64_t getRunningSum() const {
        return running_sum_;
      }
    private:
      void step() {
        auto pos = cycle_;
        while (pos > 40) { pos -= 40; }
        if (pos >= (reg_x_ - 1) && pos <= (reg_x_ + 1)) {
          std::cout << "#";
        } else {
          std::cout << ".";
        }
        cycle_++;
        if (!(cycle_ % 40)) {
          std::cout << std::endl;
        }
        if (cycle_ < 20) { return; }
        
        const auto offset = cycle_ - 20;
        if ((offset % 40)) { return; }
        running_sum_ += (cycle_ * reg_x_);
      }

      int64_t cycle_;
      int64_t running_sum_;
      int64_t reg_x_;
  };

  STRING_CONSTANT(ADDX, "addx");
  STRING_CONSTANT(NOOP, "noop");

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    CPU cpu;
    while (aoc::getline(f, line)) {
      const auto sep = line.find(' ');
      const auto inst = sep == std::string_view::npos ? line : line.substr(0, sep);
      if (inst == NOOP) {
        cpu.noop();
      } else if (inst == ADDX) {
        assert(sep != std::string_view::npos);
        const auto arg = line.substr(sep + 1);
        cpu.addx(aoc::stoi(arg));
      } else {
        throw std::runtime_error("Bad instruction: " + std::string(line));
      }
    }
    r.first = cpu.getRunningSum();
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

  int part1 = 0;
  int part2 = 0;

  std::tie(part1, part2) = r;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
