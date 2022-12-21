#include "aoc/helpers.h"
#include <map>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(root: pppw + sjmn
dbpl: 5
cczh: sllz + lgvd
zczc: 2
ptdq: humn - dvpt
dvpt: 3
lfqf: 4
humn: 5
ljgn: 2
sjmn: drzm * dbpl
sllz: 4
pppw: cczh / lfqf
lgvd: ljgn * ptdq
drzm: hmdt - zczc
hmdt: 32
)");
  constexpr int SR_Part1 = 152;
  constexpr int SR_Part2 = 301;

  enum class Job {
    Shout = 0,
    Add = '+',
    Sub = '-',
    Mul = '*',
    Div = '/',
  };

  struct Monkey {
    Job job;
    std::string name;
    std::string mlhs;
    std::string mrhs;
    int64_t result;
    bool done;

    void solve(int64_t lhs, int64_t rhs) {
      assert(!done);

      switch (job) {
        case Job::Add:
          result = lhs + rhs;
          break;
        case Job::Mul:
          result = lhs * rhs;
          break;
        case Job::Div:
          result = lhs / rhs;
          break;
        case Job::Sub:
          result = lhs - rhs;
          break;
        case Job::Shout:
          throw std::runtime_error("Bad Input");
      }

      done = true;
    }
  };

  const auto parse = [](std::string_view line) {
    Monkey m;

    auto sep = line.find(':');
    assert(sep != std::string_view::npos);

    m.name = line.substr(0, sep);
    while ((line[sep] == ':' || line[sep] == ' ') && sep < line.size()) {
      sep++;
    }
    line.remove_prefix(sep);

    if (aoc::is_numeric(line[0])) {
      m.done = true;
      m.result = aoc::stoi(line);
      m.job = Job::Shout;

      return m;
    }

    m.result = 0;
    m.done = false;

    sep = line.find(' ');
    m.mlhs = line.substr(0, sep);
    sep++;
    switch (line[sep]) {
      case '+':
        m.job = Job::Add;
        break;
      case '-':
        m.job = Job::Sub;
        break;
      case '/':
        m.job = Job::Div;
        break;
      case '*':
        m.job = Job::Mul;
        break;
      default:
        assert(false);
    }
    sep+=2;

    m.mrhs = line.substr(sep);
    return m;
  };

  using MonkeyMap = std::map<std::string, Monkey, std::less<>>;

  const auto LoadInput = [](auto f) {
    MonkeyMap r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      Monkey m = parse(line);
      r.emplace(m.name, m);
    }
    return r;
  };

  int64_t resolveMonkey(MonkeyMap& monkeys, const std::string_view name) {
    auto mit = monkeys.find(name);
    assert(mit != monkeys.end());

    if (mit->second.done) {
      DEBUG_LOG(name, mit->second.result);
      return mit->second.result;
    }
    DEBUG_LOG(name, mit->second.mlhs, (char)mit->second.job, mit->second.mrhs);

    const auto lhs = resolveMonkey(monkeys, mit->second.mlhs);
    const auto rhs = resolveMonkey(monkeys, mit->second.mrhs);

    mit->second.solve(lhs, rhs);

    DEBUG_LOG(name, lhs, (char)mit->second.job, rhs, mit->second.result);

    return mit->second.result;
  }

}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  MonkeyMap r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int64_t part1 = resolveMonkey(r, "root");
  int part2 = 0;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
