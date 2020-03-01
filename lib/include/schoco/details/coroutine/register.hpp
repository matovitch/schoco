#pragma once

#include <cstdint>

namespace schoco_details
{

namespace coroutine
{

using Register = uint64_t;

extern thread_local Register MMX_FPU_STATE;

namespace RegisterMap
{

enum
{
    R12,
    R13,
    R14,
    R15,
    RDX_RETURN_ADDRESS,
    RCX_STACK_POINTER,
    RBX,
    RBP,
    MMX_FPU_STATE,
    SIZE
};

} // namespace RegisterMap

} // namespace coroutine

} // namespace schoco_details
