#pragma once

// Deterministic, self-contained pseudo random generator (SplitMix64).
//
// A tiny hand-written generator is used instead of <random> because the
// standard library does not guarantee that a given engine/algorithm pair
// produces identical values across compilers and platforms. World generation
// and agent scheduling must be reproducible from a seed, so the generator is
// part of the product contract and is covered by tests.

#include <cstdint>

namespace tt {

class Rng {
public:
    explicit Rng(uint64_t seed = 0x2545F4914F6CDD1DULL) { reseed(seed); }

    void reseed(uint64_t seed) {
        // Avoid the all-zero state and spread the seed a little.
        state_ = seed ^ 0x9E3779B97F4A7C15ULL;
        if (state_ == 0) state_ = 0xD1B54A32D192ED03ULL;
    }

    uint64_t nextU64() {
        uint64_t z = (state_ += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

    uint32_t nextU32() { return static_cast<uint32_t>(nextU64() >> 32); }

    // Uniform in [0, 1). 24 explicit bits, so the result is identical on every
    // platform regardless of float rounding mode.
    float nextFloat() {
        return static_cast<float>(nextU32() >> 8) * (1.0f / 16777216.0f);
    }

    float range(float lo, float hi) { return lo + (hi - lo) * nextFloat(); }

    int rangeInt(int lo, int hi) {
        if (hi <= lo) return lo;
        const uint32_t span = static_cast<uint32_t>(hi - lo + 1);
        return lo + static_cast<int>(nextU32() % span);
    }

    bool chance(float p) { return nextFloat() < p; }

    uint64_t state() const { return state_; }

private:
    uint64_t state_ = 0;
};

}  // namespace tt
