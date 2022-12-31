#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <chrono>
#include <cassert>
#include <functional>
#include <iomanip>
#include <vector>

#ifndef NDEBUG
#define DEBUG(x) do { \
    x; \
} while (0)
#else
#define DEBUG(x)
#endif

#define DEBUG_PRINT(x) do { DEBUG(std::cout << __func__ << ":" << __LINE__ << ": " << x << std::endl); } while (0)

#define STRING_CONSTANT(symbol, value) constexpr std::string_view symbol(value)

#ifndef INT_MIN
#define INT_MIN std::numeric_limits<int>::min()
#endif

#ifndef INT_MAX
#define INT_MAX std::numeric_limits<int>::max()
#endif


namespace aoc {

    using Point = std::pair<int64_t, int64_t>;

    enum class CardinalDirection {
        North = 0,
        NorthEast = 45,
        East = 90,
        SouthEast = 135,
        South = 180,
        SouthWest = 225,
        West = 270,
        NorthWest = 315,
    };

    const std::vector<CardinalDirection> DIRECTIONS {
        CardinalDirection::North,
        CardinalDirection::NorthEast, 
        CardinalDirection::East,
        CardinalDirection::SouthEast,
        CardinalDirection::South,
        CardinalDirection::SouthWest,
        CardinalDirection::West,
        CardinalDirection::NorthWest,
    };
}

std::ostream& operator<<(std::ostream& os, const aoc::Point p) {
    os << "{ " << p.first << ", " << p.second << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const aoc::CardinalDirection p) {
    switch (p) {
        case aoc::CardinalDirection::North: os << "North"; return os;
        case aoc::CardinalDirection::NorthEast: os << "NorthEast"; return os;
        case aoc::CardinalDirection::NorthWest: os << "NorthWest"; return os;
        case aoc::CardinalDirection::South: os << "South"; return os;
        case aoc::CardinalDirection::SouthEast: os << "SouthEast"; return os;
        case aoc::CardinalDirection::SouthWest: os << "SouthWest"; return os;
        case aoc::CardinalDirection::East: os << "East"; return os;
        case aoc::CardinalDirection::West: os << "West"; return os;
    }
    assert(false);
    os << static_cast<int>(p);
    return os;
}

aoc::Point operator+(const aoc::Point& lhs, const aoc::Point& rhs) {
    aoc::Point out{lhs.first + rhs.first, lhs.second + rhs.second};
    return out;
}

aoc::Point operator*(const aoc::Point& lhs, const int x) {
    aoc::Point out{lhs.first * x, lhs.second * x};
    return out;
}

aoc::Point& operator+=(aoc::Point& lhs, const aoc::Point& rhs) {
    lhs.first += rhs.first;
    lhs.second += rhs.second;
    return lhs;
}

aoc::Point& operator*=(aoc::Point& lhs, const int x) {
    lhs.first *= x;
    lhs.second *= x;
    return lhs;
}

aoc::Point operator-(const aoc::Point& lhs, const aoc::Point& rhs) {
    aoc::Point out{lhs.first - rhs.first, lhs.second - rhs.second};
    return out;
}
namespace aoc {
    // Taken from boost hash combine
    template <typename SizeT>
    inline void hash_combine_impl(SizeT& seed, SizeT value)
    {
        seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    inline void hash_combine_impl(uint32_t& h1, uint32_t k1)
    {
        const uint32_t c1 = 0xcc9e2d51;
        const uint32_t c2 = 0x1b873593;

        k1 *= c1;
        k1 =(k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;

        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> (32 - 13));
        h1 = h1*5+0xe6546b64;
    }

    template <class T>
    inline void hash_combine(std::size_t& seed, T const& v)
    {
        std::hash<T> hasher;
        return hash_combine_impl(seed, hasher(v));
    }

    // Needed if we want to store a point in a hash
    struct PointHash {
        std::size_t operator() (const Point& pair) const {
            std::size_t seed = 0;
            hash_combine(seed, pair.first);
            hash_combine(seed, pair.second);
            return seed;
        }
    };

    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

    aoc::Point abs(const aoc::Point& p) {
        return {std::abs(p.first), std::abs(p.second)};
    }

    int64_t manhattan(const aoc::Point& p, const aoc::Point& q) {
        const auto diff = aoc::abs(p - q);
        return diff.first + diff.second;
    }

    aoc::Point max(const aoc::Point& lhs, const aoc::Point& rhs) {
        return { std::max(lhs.first, rhs.first), std::max(lhs.second, rhs.second) };
    }

    aoc::Point min(const aoc::Point& lhs, const aoc::Point& rhs) {
        return { std::min(lhs.first, rhs.first), std::min(lhs.second, rhs.second) };
    }

    CardinalDirection fromBearing(int32_t bearing) {
        while (bearing < 0) {
            bearing += 360;
        }
        if (bearing >= 360) {
            bearing %= 360;
        }
        switch (bearing) {
            case 0:
                return CardinalDirection::North;
            case 45:
                return CardinalDirection::NorthEast;
            case 90:
                return CardinalDirection::East;
            case 135:
                return CardinalDirection::SouthEast;
            case 180:
                return CardinalDirection::South;
            case 225:
                return CardinalDirection::SouthWest;
            case 270:
                return CardinalDirection::West;
            case 325:
                return CardinalDirection::NorthWest;
            default:
                throw std::runtime_error("Bad bearing: " + std::to_string(bearing));
        }
    }

    CardinalDirection turnLeft(CardinalDirection dir) {
        int32_t bearing = static_cast<int32_t>(dir);
        bearing -= 90;
        return fromBearing(bearing);
    }

    CardinalDirection turnRight(CardinalDirection dir) {
        int32_t bearing = static_cast<int32_t>(dir);
        bearing += 90;
        return fromBearing(bearing);
    }

    aoc::Point stepFromCardinalDirection(CardinalDirection dir) {
        switch (dir) {
            case CardinalDirection::North:
                return { 0, -1 };
            case CardinalDirection::NorthEast:
                return { 1, -1 };
            case CardinalDirection::East:
                return { 1, 0 };
            case CardinalDirection::SouthEast:
                return { 1, 1 };
            case CardinalDirection::South:
                return { 0, 1 };
            case CardinalDirection::SouthWest:
                return { -1, 1 };
            case CardinalDirection::West:
                return { -1, 0 };
            case CardinalDirection::NorthWest:
                return { -1, -1 };
        }
        throw std::runtime_error("Bad direction: " + std::to_string(static_cast<int32_t>(dir)));
    }

    aoc::Point moveInDirection(const aoc::Point pt, CardinalDirection dir, int steps) {
      aoc::Point step = stepFromCardinalDirection(dir) * steps;
      return pt + step;
    }

    const auto print_result = [](int part, auto result) {
        std::cout << "Part " << part << ": " << result << std::endl;
    };

    const auto print_results = [](const auto& part1, const auto& part2) {
        print_result(1, part1);
        print_result(2, part2);
    };

    const auto assert_result = [](auto r, auto e) {
        std::cout << "Expected: " << e << " Got: " << r;
        if (e != r) {
            std::cout << " FAILED" << std::endl;
            exit(-1);
        }
        std::cout << " OK" << std::endl;
    };

    auto open_argv_1(int argc, char **argv) {
        if (argc < 2) {
            throw std::runtime_error("Insufficient arguments");
        }

        std::ifstream f;
        f.open(argv[1]);
        return f;
    }

    std::ostream& bold_on(std::ostream& os) {
        return os << "\e[1m";
    }

    std::ostream& bold_off(std::ostream& os) {
        return os << "\e[0m";
    }

    std::ostream& cls(std::ostream& os) {
        return os << "\033[2J\033[1;1H";
    }

    bool ends_with(const std::string_view s, const std::string_view p) {
        if (s.size() < p.size()) { return false; }
        const auto e = s.substr(s.size() - p.size());
        return e == p;
    }

    bool starts_with(const std::string_view s, const std::string_view p) {
        if (s.size() < p.size()) { return false; }
        const auto e = s.substr(0, p.size());
        return e == p;
    }

    bool is_numeric(const char c) {
        switch (c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return true;
            default:
                return false;
        }
    }

    bool is_numeric(const std::string_view sv) {
        if (sv.empty()) { return false; }
        bool first = true;
        for (const auto& c : sv) {
            if (first && c == '-') { first = false; continue; }
            first = false;
            if (!is_numeric(c)) {
                return false;
            }
        }
        return true;
    }

    int64_t stoi(std::string_view sv) {
        bool neg = false;
        bool first = true;
        int64_t out = 0;
        for (const auto& c : sv) {
            switch (c) {
                case '0':
                    out *= 10; break;
                case '1':
                    out *= 10; out += 1; break;
                case '2':
                    out *= 10; out += 2; break;
                case '3':
                    out *= 10; out += 3; break;
                case '4':
                    out *= 10; out += 4; break;
                case '5':
                    out *= 10; out += 5; break;
                case '6':
                    out *= 10; out += 6; break;
                case '7':
                    out *= 10; out += 7; break;
                case '8':
                    out *= 10; out += 8; break;
                case '9':
                    out *= 10; out += 9; break;
                case '-':
                    if (first) { neg = true; break; }
                    [[fallthrough]];
                default:
                    throw std::runtime_error("Not an integer: `" + std::string(sv) + "'");
            }
            first = false;
        }
        return out * (1 - 2 * neg);
    }

    bool getline(std::string_view& s, std::string_view& out, const std::string_view delims, bool return_empty = false) {
        out = std::string_view();
        if (s.empty()) { return false; }

        do {
            const size_t end = s.find_first_of(delims);

            if (end != std::string_view::npos) {
                out = s.substr(0, end);
                s = s.substr(end + 1);
            } else {
                out = s;
                s = std::string_view();
                break;
            }
        } while ((!return_empty && out.empty()) && !s.empty());

        return (return_empty || !out.empty());
    }

    bool getline(std::string_view& s, std::string_view& out, const char delim) {
        return getline(s, out, std::string_view(&delim, 1));
    }
    bool getline(std::string_view& s, std::string_view& out) {
        return getline(s, out, std::string_view("\r\n", 2));
    }

    bool getline(std::istream& s, std::string& out, const std::string_view delims) {
        char c;
        out.resize(0);
        while (s.good() && (c = s.get())) {
            if (delims.find(c) != std::string_view::npos) {
                if (out.empty()) {
                    continue;
                }
                return true;
            }
            if (c > 0) {
                out.append(&c, 1);
            }
        }
        return !out.empty() || s.good();
    }
    bool getline(std::istream& s, std::string& out, const char delim) {
        return getline(s, out, std::string_view(&delim, 1));
    }
    bool getline(std::istream& s, std::string& out) {
        char c;
        out.resize(0);
        while (s.good() && (c = s.get())) {
            switch (c) {
                case '\r':
                case '\n':
                    if (out.empty()) { continue; }
                    return true;
                default:
                    if (c > 0) {
                        out.append(&c, 1);
                    }
                    break;
            }
        }
        return !out.empty() || s.good();
    }

    using UnaryIntFunction = std::function<void(const int64_t)>;
    void parse_as_integers(std::istream& s, const char delim, UnaryIntFunction op) {
        std::string l;
        while (getline(s, l, delim)) {
            try {
                const auto n = aoc::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    void parse_as_integers(std::istream& s, const std::string_view delims, UnaryIntFunction op) {
        std::string l;
        while (getline(s, l, delims)) {
            try {
                const auto n = aoc::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    void parse_as_integers(std::istream& s, UnaryIntFunction op) {
        std::string l;
        while (getline(s, l)) {
            try {
                const auto n = aoc::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    void parse_as_integers(const std::string& s, const char delim, UnaryIntFunction op) {
        std::string_view ss(s);
        std::string_view l;
        while (getline(ss, l, delim)) {
            try {
                const auto n = aoc::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    void parse_as_integers(const std::string_view s, const std::string_view delims, UnaryIntFunction op) {
        std::string_view ss(s);
        std::string_view l;
        while (getline(ss, l, delims)) {
            try {
                const auto n = aoc::stoi(l);
                op(n);
            } catch (...) { }
        }
    }

    class AutoTimer {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
        std::string name_;

    public:
        AutoTimer()
            : start_(std::chrono::high_resolution_clock::now())
        { }

        AutoTimer(const char *name)
            : start_(std::chrono::high_resolution_clock::now())
            , name_(name)
        { }

        ~AutoTimer() {
            calculate_time();
        }

        void elapsed() const {
            calculate_time();
        }

        void reset() {
            start_ = std::chrono::high_resolution_clock::now();
        }

    private:
        // Needs to be a lambda due to use of auto
        void calculate_time () const {
            const auto end = std::chrono::high_resolution_clock::now();

            // Calculating total time taken by the program.
            double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
            time_taken *= 1e-9;

            std::cout << "Elapsed" << (name_.empty() ? "" : " " + name_) << ": " << std::fixed << time_taken << std::setprecision(9) << " sec" << std::endl;
        }

    };

    template<typename T>
    class MappedFileSource {
    public:

        MappedFileSource()
            : _fd(0)
            , _size(0)
            , _map(nullptr)
        {}

        MappedFileSource(const char *filename) 
            : MappedFileSource()
        {
            map_file(filename);
        }

        MappedFileSource(int argc, char **argv)
            : MappedFileSource()
        {
            if (argc < 2) {
                throw std::runtime_error("Insufficient arguments");
            }

            map_file(argv[1]);
        }

        ~MappedFileSource() {
            reset();
        }

        void reset() {
            if (_map) {
                ::munmap(_map, _size);
            }
            if (_fd) {
                ::close(_fd);
            }

            _map = nullptr;
            _fd = 0;
            _size = 0;
        }

        void reset(const char *filename) {
            this->reset();
            this->map_file(filename);
        }

        void map_file(const char *filename) {
            if (!filename) { throw std::runtime_error("map_file: nullptr"); }
            if (_fd || _map || _size) { throw std::runtime_error("map_file: already mapped"); }

            _fd = ::open(filename, O_RDONLY);
            if (_fd == -1) { throw std::runtime_error("map_file: open failed"); }

            struct stat fs;
            int r = ::fstat(_fd, &fs);
            if (r == -1) { reset(); throw std::runtime_error("map_file: fstat failed"); }
            _size = fs.st_size;

            _map = (T*)::mmap(0, _size, PROT_READ, MAP_SHARED, _fd, 0);
            if (!_map) { reset(); throw std::runtime_error("map_file: mmap failed"); }
        }

        const T* data() const { return _map; }
        size_t size() const { return _size; }

    private:

        int _fd;
        size_t _size;
        T* _map;
    };
};

#include "log.h"
#define DEBUG_LOG(...) do { DEBUG(LOG(__func__, __VA_ARGS__)); } while (0)
