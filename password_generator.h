#pragma once

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pwgen {

constexpr int kMaxLength = 4096;

struct Options {
    int length = 16;
    bool upper = true;
    bool lower = true;
    bool digits = true;
    bool special = true;
    bool excludeAmbiguous = false;  // drops I, l, 1, O, 0
};

// Builds a password from std::random_device directly (no seeded PRNG).
// Every selected character set appears at least once, and the result is
// shuffled with a Fisher-Yates pass. Throws std::invalid_argument on bad input.
inline std::string generate(const Options &o) {
    if (o.length < 1 || o.length > kMaxLength) {
        throw std::invalid_argument("Length must be between 1 and " + std::to_string(kMaxLength) + ".");
    }

    std::vector<std::string> sets;
    auto addSet = [&](bool enabled, std::string chars) {
        if (!enabled) return;
        if (o.excludeAmbiguous) {
            chars.erase(std::remove_if(chars.begin(), chars.end(),
                                       [](char c) { return std::string("Il1O0").find(c) != std::string::npos; }),
                        chars.end());
        }
        if (!chars.empty()) sets.push_back(std::move(chars));
    };
    addSet(o.upper, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    addSet(o.lower, "abcdefghijklmnopqrstuvwxyz");
    addSet(o.digits, "0123456789");
    addSet(o.special, "!@#$%^&*()-_=+[]{}|;:',.<>?/`~");

    if (sets.empty()) {
        throw std::invalid_argument("No character sets selected.");
    }
    if (static_cast<std::size_t>(o.length) < sets.size()) {
        throw std::invalid_argument("Length must be at least " + std::to_string(sets.size()) +
                                    " to include every selected character set.");
    }

    std::string all;
    for (const auto &s : sets) all += s;

    std::random_device rd;
    auto pick = [&rd](std::size_t n) {
        std::uniform_int_distribution<std::size_t> dist(0, n - 1);
        return dist(rd);
    };

    std::string password;
    password.reserve(static_cast<std::size_t>(o.length));
    for (const auto &s : sets) password += s[pick(s.size())];
    while (password.size() < static_cast<std::size_t>(o.length)) password += all[pick(all.size())];

    for (std::size_t i = password.size() - 1; i > 0; --i) {
        std::swap(password[i], password[pick(i + 1)]);
    }
    return password;
}

}  // namespace pwgen
