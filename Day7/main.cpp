#include "aoc/helpers.h"
#include <utility>
#include <optional>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"($ cd /
$ ls
dir a
14848514 b.txt
8504156 c.dat
dir d
$ cd a
$ ls
dir e
29116 f
2557 g
62596 h.lst
$ cd e
$ ls
584 i
$ cd ..
$ cd ..
$ cd d
$ ls
4060174 j
8033020 d.log
5626152 d.ext
7214296 k)");
  constexpr int SR_Part1 = 95437;
  constexpr int SR_Part2 = 24933642;

  class Directory {
    public:

      Directory(std::string_view name, Directory* parent)
        : name_(name.data(), name.size())
        , size_(0)
        , parent_(parent)
      { }

      Directory(std::string_view name)
        : Directory(name, nullptr)
      { }

      void add_file(size_t size) {
        size_ += size;
      }

      Directory *add_child(Directory&& child) {
        children_.emplace_back(std::move(child));
        return &children_.back();
      }

      Directory* get_parent() const {
        return parent_;
      }

      size_t get_this_size() const {
        return size_;
      }

      size_t get_size() const {
        size_t t = get_this_size();
        for (const auto& c : children_) {
          t += c.get_size();
        }
        return t;
      }

      const std::string& get_name() const {
        return name_;
      }

      size_t get_size_if_less_than(size_t limit) const {
        size_t t = 0;
        auto s = get_size();
        if (s <= limit) { t += s; }
        for (const auto& c : children_) {
          t += c.get_size_if_less_than(limit);
        }
        DEBUG_LOG(name_, t); 
        return t;
      }
      size_t get_smallest_over_limit(size_t limit) const {
        size_t t = SIZE_MAX;
        auto s = get_size();
        if (s > limit) {
          t = std::min(s, t);
        }
        for (const auto& c : children_) {
          s = c.get_smallest_over_limit(limit);
          t = std::min(s, t);
        }
        return t;
      }
    private:

      std::string name_;
      size_t size_;

      Directory* parent_;
      std::vector<Directory>children_;
  };

  STRING_CONSTANT(CD_COMMAND, "$ cd ");
  STRING_CONSTANT(LS_COMMAND, "$ ls");
  STRING_CONSTANT(DIR, "dir ");
  STRING_CONSTANT(ROOT, "/");
  STRING_CONSTANT(ELIPSES, "..");

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    Directory root("/");
    Directory* pwd = &root;
    while (aoc::getline(f, line)) {
      if (aoc::starts_with(line, CD_COMMAND)) {
        auto dir = line.substr(CD_COMMAND.size());
        if (dir == ROOT) {
          pwd = &root;
        } else if (dir == ELIPSES) {
          pwd = pwd->get_parent();
          assert(!!pwd);
        } else {
          Directory d{dir, pwd};
          pwd = pwd->add_child(std::move(d));
        }
        DEBUG_LOG(pwd->get_name());
      }
      else if (aoc::starts_with(line, DIR)) { continue; }
      else if (aoc::starts_with(line, LS_COMMAND)) {continue; }
      else {
        // file
        const auto sep = line.find(' ');
        assert(sep != std::string_view::npos);
        const auto s = line.substr(0, sep);
        pwd->add_file(aoc::stoi(s));
      }
    }
    r.first = root.get_size_if_less_than(100000);
    auto unused = 70000000 - root.get_size();
    auto limit = 30000000 - unused;
    r.second = root.get_smallest_over_limit(limit);
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