//
// Created by dev on 21.11.18.
//

#ifndef BALDA_BALDA_HPP
#define BALDA_BALDA_HPP

#include <chrono>

static constexpr std::chrono::seconds SESSIONS_TIMEOUT_LIMIT{ std::chrono::minutes{ 1 } };
static constexpr std::chrono::seconds GAMES_TIMEOUT_LIMIT{ std::chrono::minutes{ 1 } };

static constexpr std::uint64_t SESSIONS_COUNT_LIMIT{ 1000 };
static constexpr std::uint64_t GAMES_COUNT_LIMIT{ 1000 };


#endif //BALDA_BALDA_HPP
