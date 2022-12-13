#include "aoc/helpers.h"
#include <vector>
#include <variant>
#include <algorithm>
#include <charconv>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"([1,1,3,1,1]
[1,1,5,1,1]

[[1],[2,3,4]]
[[1],4]

[9]
[[8,7,6]]

[[4,4],4,4]
[[4,4],4,4,4]

[7,7,7,7]
[7,7,7]

[]
[3]

[[[]]]
[[]]

[1,[2,[3,[4,[5,6,7]]]],8,9]
[1,[2,[3,[4,[5,6,0]]]],8,9])");
  constexpr int SR_Part1 = 13;
  constexpr int SR_Part2 = 140;

  struct Value
    : public std::variant<int, std::vector<Value>>
  {
    Value()
      : variant(0)
    {}
    Value(int v)
      : variant(v)
    {}
    Value(const std::vector<Value>& v)
      : variant(v)
    {}

    bool operator<(const Value& rhs) const;
  };

  struct ValueComparator {
    bool operator()(int l, int r) const {
      return l < r;
    }
    bool operator()(const std::vector<Value>& l, const std::vector<Value>& r) const {
      return std::lexicographical_compare(l.cbegin(), l.cend(), r.cbegin(), r.cend());
    }
    bool operator()(int l, const std::vector<Value>& r) const {
      return operator()(std::vector<Value>{l}, r);
    }
    bool operator()(const std::vector<Value>& l, int r) const {
      return operator()(l, std::vector<Value>{r});
    }
  };

  bool Value::operator<(const Value& rhs) const {
    static ValueComparator vc;
    return std::visit(vc, *this, rhs);
  }

  Value ParseValue(std::string_view& line) {
    if (line[0] != '[') {
      // int
      int v;
      const auto end = line.data() + line.size();
      auto p = std::from_chars(line.data(), end, v);
      const auto prefix = p.ptr - line.data();
      line.remove_prefix(prefix);
      return v;
    } else {
      std::vector<Value> v;
      if (line[1] == ']') { // empty vector
        line.remove_prefix(2);
        return v;
      }

      // consume one list at a timme
      while (line[0] != ']') {
        line.remove_prefix(1);
        v.push_back(ParseValue(line));
      }
      // consume ']'
      line.remove_prefix(1);

      return v;
    }
  }

  STRING_CONSTANT(DIV_1, "[[2]]");
  STRING_CONSTANT(DIV_2, "[[6]]");

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    std::vector<Value>values;
    while (aoc::getline(f, line)) {
      values.push_back(ParseValue(line));

      if (!(values.size() % 2)) {
        const auto& lhs = values.at(values.size() - 2);
        const auto& rhs = values.back();
        r.first += (lhs < rhs ? (values.size() / 2) : 0);
      }
    }
    line = DIV_1;
    const auto div1 = ParseValue(line);
    line = DIV_2;
    const auto div2 = ParseValue(line);
    // add the two divider packets, sort and locate indicies
    values.push_back(div1);
    values.push_back(div2);

    std::sort(values.begin(), values.end());
    const auto idx1 = std::lower_bound(values.cbegin(), values.cend(), div1) - values.begin();
    const auto idx2 = std::lower_bound(values.cbegin(), values.cend(), div2) - values.begin();

    r.second = (idx1 + 1) * (idx2 + 1);
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
