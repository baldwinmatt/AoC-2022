#include "aoc/helpers.h"
#include <unordered_map>
#include <stack>

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

    int64_t solve(int64_t lhs, int64_t rhs) const {
      switch (job) {
        case Job::Add:
          return lhs + rhs;
          break;
        case Job::Mul:
          return lhs * rhs;
          break;
        case Job::Div:
          return lhs / rhs;
          break;
        case Job::Sub:
          return lhs - rhs;
          break;
        case Job::Shout:
          assert(false);
      }
      assert(false);

      return lhs;
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
      m.result = aoc::stoi(line);
      m.job = Job::Shout;

      return m;
    }

    m.result = 0;

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

  using MonkeyMap = std::unordered_map<std::string, Monkey>;
  using MonkeyStack = std::stack<Monkey>;

  const auto LoadInput = [](auto f) {
    MonkeyMap r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      Monkey m = parse(line);
      r.emplace(m.name, m);
    }
    return r;
  };

  int64_t resolveMonkey(const MonkeyMap& monkeys, const std::string& name) {
    auto mit = monkeys.find(name);
    assert(mit != monkeys.end());

    if (mit->second.job == Job::Shout) {
      return mit->second.result;
    }
    const auto lhs = resolveMonkey(monkeys, mit->second.mlhs);
    const auto rhs = resolveMonkey(monkeys, mit->second.mrhs);

    return mit->second.solve(lhs, rhs);
  }

  const std::string ROOT{"root"};
  const std::string HUMN{"humn"};

  bool find_human(const MonkeyMap& monkeys, MonkeyStack& stack, const Monkey& node) {
    if (HUMN.compare(node.name) == 0) {
      return true;
    }

    if (node.job == Job::Shout) {
      return false;
    }

    const auto lhs = monkeys.find(node.mlhs);
    const auto rhs = monkeys.find(node.mrhs);

    if (find_human(monkeys, stack, lhs->second)) {
      DEBUG_LOG(lhs->second.name);
      stack.push(lhs->second);
      return true;
    } else if (find_human(monkeys, stack, rhs->second)) {
      DEBUG_LOG(rhs->second.name);
      stack.push(rhs->second);
      return true;
    }

    return false;
  }

  int64_t solve(const MonkeyMap& monkeys, MonkeyStack& stack, const std::string& name, int64_t equal_to) {
    if (name == HUMN) {
      return equal_to;
    }

    const auto mit = monkeys.find(name);

    const auto op = mit->second.job;
    const auto& left = mit->second.mlhs;
    const auto& right = mit->second.mrhs;

    assert(op != Job::Shout);

    const auto top = stack.top(); stack.pop();

    const bool is_left = top.name.compare(left) == 0;
    const auto& to_solve = is_left ? left : right;
    DEBUG_LOG(top.name, top.mlhs, top.mrhs, to_solve, mit->second.name, left, right);
    const auto other = is_left ? resolveMonkey(monkeys, right) : resolveMonkey(monkeys, left);

    switch (op) {
      case Job::Add:
        return solve(monkeys, stack, to_solve, equal_to - other);
      case Job::Sub:
        {
          const auto rem = is_left ? equal_to + other : other - equal_to;
          return solve(monkeys, stack, to_solve, rem);
        }
      case Job::Div:
        {
          const auto rem = is_left ? equal_to * other : other / equal_to;
          return solve(monkeys, stack, to_solve, rem);
        }
      case Job::Mul:
        return solve(monkeys, stack, to_solve, equal_to / other);
      default:
        throw std::runtime_error("Bad Path");
    }
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

  int64_t part1 = resolveMonkey(r, ROOT);

  MonkeyStack hs;
  const auto rit = r.find(ROOT);
  find_human(r, hs, rit->second);

  const auto hp = hs.top().name; hs.pop();
  const auto is_left = rit->second.mlhs == hp;

  const auto target = is_left ?
    resolveMonkey(r, rit->second.mrhs) :
    resolveMonkey(r, rit->second.mlhs);

  const auto& to_solve = is_left ? rit->second.mlhs : rit->second.mrhs;

  const int64_t part2 = solve(r, hs, to_solve, target);
  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
