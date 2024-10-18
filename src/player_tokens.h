#pragma once
#include <iomanip>
#include <random>
#include <sstream>
#include "tagged.h"

namespace detail {
    struct TokenTag {};
}  // detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    Token GetToken() {
        Token t(GeneratePlayerToken());
        return t;
    }

private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{ [this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }() };
    std::mt19937_64 generator2_{ [this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }() };

    std::string GeneratePlayerToken() {

        std::uint64_t num1 = generator1_();
        std::uint64_t num2 = generator2_();

        std::ostringstream oss;
        oss << std::hex << std::setw(16) << std::setfill('0') << num1;
        oss << std::hex << std::setw(16) << std::setfill('0') << num2;

        return oss.str();
    }

};
