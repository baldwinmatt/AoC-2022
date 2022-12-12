#include "aoc/helpers.h"
#include <vector>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(30373
25512
65332
33549
35390)");
  constexpr int SR_Part1 = 21;
  constexpr int SR_Part2 = 8;

  using Row = std::vector<char>;
  using Grid = std::vector<Row>;

  const auto LoadInput = [](auto f) {
    std::string_view line;
    Grid grid;
    while (aoc::getline(f, line)) {
      grid.emplace_back();
      auto& r = grid.back();
      for (const auto c: line) {
        assert(aoc::is_numeric(c));
        r.push_back(c);
      }
      assert(grid.size() == 1 || r.size() == grid[0].size());
    }
    return grid;
  };

  class TreeFinder {
    public:
      TreeFinder(const Grid& grid)
        : grid_(grid)
        , width_(grid[0].size())
        , height_(grid.size())
        , max_score_(INT64_MIN)
        , visible_trees_(0)
      {
        visible_trees_ = countVisibleTrees();
      }

      int64_t getMaxScore() const {
        return max_score_;
      }

      size_t getVisibleTrees() const {
        return visible_trees_;
      }

    protected:
      size_t countVisibleTrees() {
        size_t v = (2 * width_) + (2 * (height_ - 2));
        for (size_t y = 1; y < height_ - 1; y++) {
          for (size_t x = 1; x < width_ - 1; x++) {
            int64_t s;
            v += isTreeVisible(x, y, s);
            max_score_ = std::max(s, max_score_);
          }
        }
        return v;
      }

      bool isTreeVisible(size_t x, size_t y, int64_t& score) const {
        score = 0;
        if (x == 0 || x == width_ - 1) { return true; }
        if (y == 0 || y == height_ - 1) { return true; }
        const auto& row = grid_[y];
        char height = row[x];
        if (height == 0) { return false; }

        // From left
        bool visibleL = true;
        int32_t scoreL = 0;
        for (size_t i = x; visibleL && i > 0; i--) {
          scoreL += visibleL;
          if (row[i - 1] >= height) { visibleL = false; }
        }
        // from right
        bool visibleR = true;
        int32_t scoreR = 0;
        for (size_t i = x + 1; visibleR && i < width_; i++) {
          scoreR += visibleR;
          if (row[i] >= height) { visibleR = false; }
        }
        // from top
        bool visibleT = true;
        int32_t scoreT = 0;
        for (size_t j = y; visibleT && j > 0; j--) {
          scoreT += visibleT;
          if (grid_[j - 1][x] >= height ) { visibleT = false; }
        }
        // from bottom
        bool visibleD = true;
        int32_t scoreD = 0;
        for (size_t j = y + 1; visibleD && j < height_; j++) {
          scoreD += visibleD;
          if (grid_[j][x] >= height ) { visibleD = false; }
        }
        score = scoreL * scoreR * scoreT * scoreD;
        DEBUG_LOG(x, y, height, score, scoreL, scoreR, scoreT, scoreD);
        return visibleL || visibleR || visibleD || visibleT;
      }

    private:
      const Grid& grid_;
      const size_t width_;
      const size_t height_;
      int64_t max_score_;
      int64_t visible_trees_;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Grid grid;
  if (inTest) {
    grid = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    grid = LoadInput(f);
  }

  const TreeFinder tf(grid);

  int part1 = tf.getVisibleTrees();
  int part2 = tf.getMaxScore();

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
