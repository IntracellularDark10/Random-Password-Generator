#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "password_generator.h"

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::fprintf(stderr, "FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            std::exit(1);                                                 \
        }                                                                 \
    } while (0)

static bool has(const std::string &s, const std::string &set) {
    return s.find_first_of(set) != std::string::npos;
}
static bool only(const std::string &s, const std::string &set) {
    return s.find_first_not_of(set) == std::string::npos;
}

template <class F>
static bool throwsInvalid(F f) {
    try { f(); } catch (const std::invalid_argument &) { return true; }
    return false;
}

int main() {
    const std::string U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ", L = "abcdefghijklmnopqrstuvwxyz",
                      D = "0123456789", S = "!@#$%^&*()-_=+[]{}|;:',.<>?/`~";

    // Length is exact for every size in range.
    for (int n = 4; n <= 300; ++n) {
        pwgen::Options o; o.length = n;
        CHECK(static_cast<int>(pwgen::generate(o).size()) == n);
    }
    { pwgen::Options o; o.length = pwgen::kMaxLength; CHECK(static_cast<int>(pwgen::generate(o).size()) == pwgen::kMaxLength); }

    // Minimum length still contains every selected set.
    for (int i = 0; i < 5000; ++i) {
        pwgen::Options o; o.length = 4;
        std::string p = pwgen::generate(o);
        CHECK(has(p, U) && has(p, L) && has(p, D) && has(p, S));
    }

    // Only selected sets are used.
    for (int i = 0; i < 500; ++i) {
        pwgen::Options o; o.length = 40; o.upper = o.lower = o.special = false;
        CHECK(only(pwgen::generate(o), D));
        pwgen::Options p; p.length = 40; p.digits = p.special = false;
        CHECK(only(pwgen::generate(p), U + L));
    }

    // Ambiguous characters are excluded.
    for (int i = 0; i < 2000; ++i) {
        pwgen::Options o; o.length = 64; o.excludeAmbiguous = true;
        CHECK(!has(pwgen::generate(o), "Il1O0"));
    }

    // Invalid input throws.
    { pwgen::Options o; o.length = 0; CHECK(throwsInvalid([&] { pwgen::generate(o); })); }
    { pwgen::Options o; o.length = -5; CHECK(throwsInvalid([&] { pwgen::generate(o); })); }
    { pwgen::Options o; o.length = pwgen::kMaxLength + 1; CHECK(throwsInvalid([&] { pwgen::generate(o); })); }
    { pwgen::Options o; o.upper = o.lower = o.digits = o.special = false; CHECK(throwsInvalid([&] { pwgen::generate(o); })); }
    { pwgen::Options o; o.length = 3; CHECK(throwsInvalid([&] { pwgen::generate(o); })); }

    // Two draws differ (collision odds at 32 chars are negligible).
    { pwgen::Options o; o.length = 32; CHECK(pwgen::generate(o) != pwgen::generate(o)); }

    std::puts("all generator tests passed");
    return 0;
}
