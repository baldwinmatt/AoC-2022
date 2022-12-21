#include "aoc/helpers.h"
#include <memory>
#include <vector>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(1
2
-3
3
-2
0
4)");
  constexpr int SR_Part1 = 3;
  constexpr int SR_Part2 = 1623178306;

  struct Node {
    int64_t val;
    Node *next;
    Node *prev;

    Node()
      : val(0)
      , next(nullptr)
      , prev(nullptr)
    { }
  };

  struct MixList {
    Node *list;
    Node *zero;
    size_t size;

    MixList()
      : list(nullptr)
      , zero(nullptr)
      , size(0)
    {
    }

    MixList(const std::vector<int64_t>& in)
      : list(nullptr)
      , zero(nullptr)
      , size(in.size())
    {
      list = new Node[size];

      size_t last = 0;

      for (const auto v : in) {
        list[last].val = v;
        DEBUG_LOG(last, v);
        if (!v) {
          zero = &list[last];
        }

        const size_t prev = (last > 0) ? (last - 1) : (size - 1);
        const size_t next = (last < size - 1) ? last + 1 : 0;

        list[last].prev = &list[prev];
        list[last].next = &list[next];

        last++;
      }
    }

    MixList(const MixList& o)
      : list(nullptr)
      , zero(nullptr)
      , size(o.size)
    {
      list = new Node[size];

      // update pointers
      for (size_t i = 0; i < size; i++) {
        list[i].val = o.list[i].val;
        if (!list[i].val) {
          zero = &list[i];
        }
        const size_t prev = (i > 0) ? (i - 1) : (size - 1);
        const size_t next = (i < size - 1) ? i + 1 : 0;

        list[i].prev = &list[prev];
        list[i].next = &list[next];
      }
    }

    ~MixList() {
      delete [] list;

      list = nullptr;
      zero = nullptr;
      size = 0;
    }

    MixList& operator=(const MixList& o) {
      delete [] list;

      zero = nullptr;
      size = o.size;
      list = new Node[size];

      // update pointers
      for (size_t i = 0; i < size; i++) {
        list[i].val = o.list[i].val;
        if (!list[i].val) {
          zero = &list[i];
        }
        const size_t prev = (i > 0) ? (i - 1) : (size - 1);
        const size_t next = (i < size - 1) ? i + 1 : 0;

        list[i].prev = &list[prev];
        list[i].next = &list[next];
      }

      return *this;
    }
  };

  const auto LoadInput = [](auto f) {
    std::vector<int64_t> in;
    aoc::parse_as_integers(f, "\r\n", [&in](const int64_t n) {
      in.push_back(n);
    });
    return MixList{in};
  };

  const auto mix = [](MixList& r, size_t rounds, int64_t key) {
    aoc::AutoTimer mixing{"Mixing"};

    key %= (r.size - 1);

    for (size_t n = 0; n < rounds; n++) {
      for (size_t i = 0; i < r.size; i++) {
        Node *cur = &r.list[i];

        if (!cur->val) {
          continue;
        }

        Node *p = cur->prev;

        // remove
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;

        const int64_t val = cur->val * key;
        DEBUG_LOG(i, val);
        int64_t dist = std::abs(val) % (r.size - 1);

        // walk to new location
        if (val > 0) {
          while (dist--) {
            p = p->next;
          }
        } else {
          while (dist--) {
            p = p->prev;
          }
        }
        // insert
        cur->prev = p;
        cur->next = p->next;

        // update pointers
        cur->prev->next = cur;
        cur->next->prev = cur;
      }
    }
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  MixList r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  assert(r.zero);
  assert(r.list);
  assert(r.size);

  int64_t part1 = 0;
  int64_t part2 = 0;

  {
    MixList r1 = r;
    mix(r1, 1, 1);

    Node *p = r1.zero;
    int n = 0;
    size_t i = 0;
    while (n < 3) {
      if (i && (i % 1000 == 0)) {
        n++;
        DEBUG_LOG(i, p->val);
        part1 += p->val;
      }
      p = p->next;
      i++;
    }
  }

  {
    MixList r2 = r;
    
    mix(r2, 10, 811589153);

    Node *p = r2.zero;
    int n = 0;
    size_t i = 0;
    while (n < 3) {
      if (i && (i % 1000 == 0)) {
        n++;
        DEBUG_LOG(i, p->val);
        part2 += p->val;
      }
      p = p->next;
      i++;
    }
    part2 *= 811589153;
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
