#include "aoc/helpers.h"
#include <numeric>
#include <array>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  template<typename T, size_t SIZE = 3>
  class MaxHeap {
  public:
    MaxHeap() :
      size_(0)
    {
      data_.fill(0);
    }

    size_t size() const {
      return size_;
    }

   void push(T &d) {
      if (size_ == SIZE) {
        auto minItr = std::min_element(data_.begin() + size_ / 2, data_.end());
        auto minPos {minItr - data_.begin()};
        auto min { *minItr};

        if (d <= min) {
          return;
        }
        data_.at(minPos) = d;
        if (data_[parent(minPos)] > d) { 
          make_heap(minPos, true);
        } else {
          make_heap(parent(minPos));
        }

        return;
      }

      data_.at(size_++) = d;
      std::push_heap(data_.begin(), data_.begin() + size_);
    }

    T pop() {
      T d = data_.front();
      std::pop_heap(data_.begin(), data_.begin() + size_);
      size_--;
      return d;
    }

    T top() const {
      return data_.front();
    }

    typename std::array<T, SIZE>::value_type *begin() {
      return data_.begin();
    }

    typename std::array<T, SIZE>::value_type *end() {
      return data_.end();
    }

  private:

    size_t parent(size_t i) {
      return (i - 1)/2;
    }

    void make_heap(size_t i, bool downward = false) {
      size_t l = 2*i + 1;
      size_t r = 2*i + 2;
      size_t largest = 0;

      if (l < size() && data_[l] > data_[i]) {
        largest = l;
      } else {
        largest = i;
      }

      if (r < size() && data_[r] > data_[largest]) {
        largest = r;   
      }

      if (largest != i) {
        std::swap(data_[largest], data_[i]);
        if (!downward) {
          make_heap(parent(i));
        } else {
          make_heap(largest, true);
        }
      }
    }

    std::array<T, SIZE> data_;
    size_t size_;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  std::string_view line;

  MaxHeap<int64_t, 3>heap{};
  int64_t cal = 0;
  while (aoc::getline(f, line, "\r\n", true)) {
    if (line.empty()) {
      heap.push(cal);
      cal = 0;
      continue;
    }
    cal += aoc::stoi(line);
  }

  heap.push(cal);

  size_t part1 = heap.top();
  size_t part2 = std::accumulate(heap.begin(), heap.end(), 0U);

  aoc::print_results(part1, part2);
  
  return 0;
}

