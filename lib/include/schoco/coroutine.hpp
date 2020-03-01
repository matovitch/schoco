#pragma once

#include "schoco/details/coroutine/coroutine.hpp"

#include "schoco/details/pool/pool.hpp"

#include <cstdint>

namespace schoco
{

namespace coroutine
{

namespace pool
{

template <std::size_t STACK_SIZE>
using TMake = schoco_details::pool::TMake<schoco_details::TCoroutine<STACK_SIZE>, 0>;

} // namespace pool

} // namespace coroutine

} // namespace schoco
