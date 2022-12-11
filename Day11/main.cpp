#include "aoc/helpers.h"
#include <queue>
#include <list>
#include <iostream>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Monkey 0:
  Starting items: 79, 98
  Operation: new = old * 19
  Test: divisible by 23
    If true: throw to monkey 2
    If false: throw to monkey 3

Monkey 1:
  Starting items: 54, 65, 75, 74
  Operation: new = old + 6
  Test: divisible by 19
    If true: throw to monkey 2
    If false: throw to monkey 0

Monkey 2:
  Starting items: 79, 60, 97
  Operation: new = old * old
  Test: divisible by 13
    If true: throw to monkey 1
    If false: throw to monkey 3

Monkey 3:
  Starting items: 74
  Operation: new = old + 3
  Test: divisible by 17
    If true: throw to monkey 0
    If false: throw to monkey 1)");
  constexpr size_t SR_Part1 = 10605;
  constexpr size_t SR_Part2 = 2713310158;

  using ItemList = std::list<int64_t>;

  enum class OpType {
    None = 0,
    Mul = '*',
    Add = '+',
    Pow = '^',
  };

  const auto ParseOptype = [](char c) {
    switch (c) {
      case '+':
        return OpType::Add;
      case '*':
        return OpType::Mul;
      default:
        throw std::runtime_error("Bad Input");
    }
  };

  class Monkey {
  public:
    Monkey()
      : op_(OpType::None)
      , arg_(0)
      , test_(0)
      , true_(0)
      , false_(0)
      , insepected_(0)
    {
    }

    void takeTurn(std::vector<Monkey>& ms, int64_t mod) {
      while (!items_.empty()) {
        auto in = items_.front(); items_.pop_front();
        auto item = performOperation(in); // increase worry
          // decrease worry
        if (!mod) { item /= 3; }
        else {item %= mod; }
        const auto test = !(item % test_); // test
        const auto dst = test ? true_ : false_;
        DEBUG_LOG(in, (char)op_, arg_, item, test_, test, dst);
        assert(dst < ms.size());
        ms[dst].addItem(item); // throw
        insepected_++;
      }
    }

    void addItem(int64_t i) {
      items_.push_back(i);
    }

    void setOperation(OpType o) {
      op_ = o;
    }

    void setArg(int64_t a) {
      arg_ = a;
    }

    void setTest(int64_t t) {
      test_ = t;
    }

    void setTrue(int64_t t) {
      true_ = t;
    }

    void setFalse(int64_t t) {
      false_ = t;
    }

    size_t getCount() const {
      return insepected_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Monkey& m);

    protected:

      int64_t performOperation(int64_t item) {
        switch (op_) {
          case OpType::Add:
            return item + arg_;
          case OpType::Mul:
            return item * arg_;
          case OpType::Pow:
            return item * item;
          case OpType::None:
            break;
        }
        throw std::runtime_error("Bad optype");
      }
      ItemList items_;
      OpType op_;
      int64_t arg_;
      int64_t test_;

      size_t true_;
      size_t false_;

      size_t insepected_;
  };
  using Monkeys = std::vector<Monkey>;

  STRING_CONSTANT(STR_MONKEY,    "Monkey ");
  STRING_CONSTANT(STR_ITEMS,     "  Starting items: ");
  STRING_CONSTANT(STR_OPERATION, "  Operation: new = old ");
  STRING_CONSTANT(STR_OLD, "old");
  STRING_CONSTANT(STR_TEST,      "  Test: divisible by ");
  STRING_CONSTANT(STR_TRUE,      "    If true: throw to monkey ");
  STRING_CONSTANT(STR_FALSE,     "    If false: throw to monkey ");

  std::ostream& operator<<(std::ostream& os, const Monkey& m) {
    os << "Monkey (" << m.insepected_ << "): " << std::endl;
    os << STR_ITEMS;
    for (const auto i : m.items_) {
      os << i << ' ';
    }
    os << std::endl;
    os << STR_OPERATION << (char)m.op_ << ' ' << m.arg_ << std::endl;;
    os << STR_TEST << m.test_ << std::endl;
    os << STR_TRUE << m.true_ << std::endl;
    os << STR_FALSE << m.false_ << std::endl;
    return os;
  }

  const auto LoadInput = [](auto f) {
    Monkeys ms;
    std::string_view line;
    ms.emplace_back();
    Monkey* m = &ms.back();
    int64_t mod = 1;
    while (aoc::getline(f, line, "\r\n", true)) {
      if (line.empty()) {
        ms.emplace_back();
        continue;
      }
      else if (aoc::starts_with(line, STR_MONKEY)) {
        m = &ms.back();
        continue;
      }
      else if (aoc::starts_with(line, STR_ITEMS)) {
        const auto r = line.substr(STR_ITEMS.size());
        aoc::parse_as_integers(r, ", ", [&m](int64_t i) { m->addItem(i); });
        continue;
      } else if (aoc::starts_with(line, STR_OPERATION)) {
        size_t i = STR_OPERATION.size();
        m->setOperation(ParseOptype(line.at(i++)));
        assert(line.at(i) == ' ');
        i++;
        const auto r = line.substr(i);
        if (r == STR_OLD) {
          m->setOperation(OpType::Pow);
          m->setArg(2);
        } else {
          m->setArg(aoc::stoi(line.substr(i)));
        }
        continue;
      } else if (aoc::starts_with(line, STR_TEST)) {
        const auto r = line.substr(STR_TEST.size());
        const auto t = aoc::stoi(r);
        m->setTest(t);
        mod *= t;
        continue;
      } else if (aoc::starts_with(line, STR_TRUE)) {
        const auto r = line.substr(STR_TRUE.size());
        m->setTrue(aoc::stoi(r));
        continue;
      } else if (aoc::starts_with(line, STR_FALSE)) {
        const auto r = line.substr(STR_FALSE.size());
        m->setFalse(aoc::stoi(r));
        continue;
      }
      throw std::runtime_error("Bad Input");
    }
    return std::pair{mod, ms};
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  std::pair<int64_t, Monkeys> r;

  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }
  Monkeys ms = r.second;
  for (size_t i = 0; i < 20; i++) {
    for (auto& m : ms) {
      m.takeTurn(ms, 0);
    }
  }

  const auto Solve = [&ms]() {
    size_t max1 = 0;
    size_t max2 = 0;
    for (const auto& m : ms) {
      DEBUG_LOG(m);
      if (m.getCount() > max1) {
        max2 = max1;
        max1 = m.getCount();
      } else if (m.getCount() > max2) {
        max2 = m.getCount();
      }
    }
    return max1 * max2;
  };
  size_t part1 = Solve();

  ms = r.second;
  for (size_t i = 0; i < 10000; i++) {
    for (auto& m : ms) {
      m.takeTurn(ms, r.first);
    }
  }

  size_t part2 = Solve();

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
