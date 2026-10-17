#pragma once

// Minimal test harness. Deliberately tiny: the point of these tests is to
// verify deterministic simulation behaviour, not to introduce a dependency.
//
// Usage:
//   TEST(suite, name) { CHECK(cond); CHECK_EQ(a, b); }

#include <cmath>
#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace tt::test {

struct TestCase {
    const char* suite;
    const char* name;
    void (*fn)();
};

std::vector<TestCase>& registry();
int& failureCount();
const char*& currentTest();

struct Registrar {
    Registrar(const char* suite, const char* name, void (*fn)());
};

int runAll();

// Record a failure and keep going so one broken assertion does not hide others.
void reportFailure(const char* file, int line, const std::string& message);

// Render a value for CHECK_EQ failure messages. Uses toString(v) when the type
// provides one (e.g. AgentState), which keeps failure output readable.
template <typename T, typename = void>
struct HasToString : std::false_type {};

template <typename T>
struct HasToString<T, std::void_t<decltype(toString(std::declval<const T&>()))>>
    : std::true_type {};

template <typename T>
std::string describe(const T& v) {
    using D = std::decay_t<T>;
    if constexpr (HasToString<D>::value) {
        return std::string(toString(v));
    } else if constexpr (std::is_same_v<D, bool>) {
        return v ? "true" : "false";
    } else if constexpr (std::is_arithmetic_v<D>) {
        return std::to_string(v);
    } else {
        return std::string(v);
    }
}

}  // namespace tt::test

#define TEST(suite, name)                                                        \
    static void suite##_##name##_impl();                                         \
    static ::tt::test::Registrar suite##_##name##_reg(#suite, #name,             \
                                                      &suite##_##name##_impl);   \
    static void suite##_##name##_impl()

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            ::tt::test::reportFailure(__FILE__, __LINE__,                        \
                                      "CHECK failed: " #cond);                   \
        }                                                                        \
    } while (false)

#define CHECK_MSG(cond, msg)                                                     \
    do {                                                                         \
        if (!(cond)) {                                                           \
            ::tt::test::reportFailure(__FILE__, __LINE__,                        \
                                      std::string("CHECK failed: " #cond " — ") + (msg)); \
        }                                                                        \
    } while (false)

#define CHECK_EQ(a, b)                                                           \
    do {                                                                         \
        const auto _va = (a);                                                    \
        const auto _vb = (b);                                                    \
        if (!(_va == _vb)) {                                                     \
            ::tt::test::reportFailure(                                           \
                __FILE__, __LINE__,                                              \
                std::string("CHECK_EQ failed: " #a " == " #b " (got ") +         \
                    ::tt::test::describe(_va) + " vs " + ::tt::test::describe(_vb) + ")"); \
        }                                                                        \
    } while (false)

#define CHECK_NEAR(a, b, eps)                                                    \
    do {                                                                         \
        const double _va = static_cast<double>(a);                               \
        const double _vb = static_cast<double>(b);                               \
        if (std::fabs(_va - _vb) > (eps)) {                                      \
            ::tt::test::reportFailure(                                           \
                __FILE__, __LINE__,                                              \
                std::string("CHECK_NEAR failed: |" #a " - " #b "| = ") +         \
                    std::to_string(std::fabs(_va - _vb)) + " > " + #eps);        \
        }                                                                        \
    } while (false)
