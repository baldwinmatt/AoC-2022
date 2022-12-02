#include "aoc/helpers.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  enum class Shape {
    Rock = 1,
    Paper = 2,
    Scissors = 3,
  };

  std::ostream& operator<<(std::ostream& os, const Shape c) {
    switch (c) {
      case Shape::Rock:
        os << "Rock";
        break;
      case Shape::Paper:
        os << "Paper";
        break;
      case Shape::Scissors:
        os << "Scissors";
        break;
      default:
        throw std::runtime_error("oops");
    }
    return os;
  }

  enum class Outcome {
    Lose = 0,
    Draw = 3,
    Win = 6,
  };

  std::ostream& operator<<(std::ostream& os, const Outcome c) {
    switch (c) {
      case Outcome::Lose:
        os << "Lose";
        break;
      case Outcome::Draw:
        os << "Draw";
        break;
      case Outcome::Win:
        os << "Win";
        break;
      default:
        throw std::runtime_error("oops");
    }
    return os;
  }

  const auto parseShape = [](char c) {
    switch (c) {
      case 'A':
      case 'X':
        return Shape::Rock;
      case 'B':
      case 'Y':
        return Shape::Paper;
      case 'C':
      case 'Z':
        return Shape::Scissors;
      default:
        throw std::invalid_argument(std::string("Bad input ") + c);
    }
  };

  const auto parseOutcome = [](char c) {
    switch (c) {
      case 'X':
        return Outcome::Lose;
      case 'Y':
        return Outcome::Draw;
      case 'Z':
        return Outcome::Win;
      default:
        throw std::invalid_argument(std::string("Bad input ") + c);
    }
  };

  const auto getScore = [](auto s) {
    return static_cast<int>(s);
  };

  const auto getShapeForOutcome = [](Shape p1, Outcome o) {
    switch (o) {
      case Outcome::Lose:
        switch (p1) {
          case Shape::Rock:
            return Shape::Scissors;
          case Shape::Paper:
            return Shape::Rock;
          case Shape::Scissors:
            return Shape::Paper;
          default:
            throw std::runtime_error("oops");
        }
      case Outcome::Draw:
        return p1;
      case Outcome::Win:
        switch (p1) {
          case Shape::Rock:
            return Shape::Paper;
          case Shape::Paper:
            return Shape::Scissors;
          case Shape::Scissors:
            return Shape::Rock;
          default:
            throw std::runtime_error("oops");
        }
        default:
          throw std::runtime_error("oops");
    }
  };

  const auto getOutcome = [](Shape p1, Shape p2) {
    switch (p2) {
      case Shape::Rock:
        switch (p1) {
          case Shape::Rock:
            return Outcome::Draw;
          case Shape::Paper:
            return Outcome::Lose;
          case Shape::Scissors:
            return Outcome::Win;
          default:
            throw std::runtime_error("oops");
        }
      case Shape::Paper:
        switch (p1) {
          case Shape::Rock:
            return Outcome::Win;
          case Shape::Paper:
            return Outcome::Draw;
          case Shape::Scissors:
            return Outcome::Lose;
          default:
            throw std::runtime_error("oops");
        }
      case Shape::Scissors:
        switch (p1) {
          case Shape::Rock:
            return Outcome::Lose;
          case Shape::Paper:
            return Outcome::Win;
          case Shape::Scissors:
            return Outcome::Draw;
          default:
            throw std::runtime_error("oops");
        }
        default:
          throw std::runtime_error("oops");
    }
  };

  const auto getGameScore = [](Shape p1, Shape p2) {
    const auto o = getOutcome(p1, p2);
    return getScore(o) + getScore(p2);
  };

  const auto getExpectedScore = [](Shape p1, Outcome o) {
    const auto p2 = getShapeForOutcome(p1, o);
    DEBUG_PRINT(p1 << " vs " << p2 << " = " << o);
    return getScore(o) + getScore(p2);
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;

  std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
  std::string_view f(m->data(), m->size());

  std::string_view line;
  size_t part1 = 0;
  size_t part2 = 0;
  while (aoc::getline(f, line, "\r\n")) {
    if (line.size() != 3) { throw std::invalid_argument(std::string("Bad input line: ") + std::string(line)); }

    const auto p1 = parseShape(line[0]);
    const auto p2 = parseShape(line[2]);
    const auto o = parseOutcome(line[2]);

    DEBUG_PRINT("p1: " << line[0] << "(" << getScore(p1) << ")");
    DEBUG_PRINT("p2: " << line[2] << "(" << getScore(p2) << ")");
    DEBUG_PRINT("o : " << line[2] << "(" << getScore(o) << ")");
    DEBUG_PRINT("getGameScore : " << getGameScore(p1, p2));
    DEBUG_PRINT("getExpectedScore : " << getExpectedScore(p1, o));

    part1 += getGameScore(p1, p2);
    part2 += getExpectedScore(p1, o);
  }

  aoc::print_results(part1, part2);

  return 0;
}

