#pragma once

#include "schoco/details/coroutine/register.hpp"
#include "schoco/details/coroutine/stack.hpp"

#include <cstdint>

namespace schoco_details
{

namespace coroutine
{

struct Abstract {};

} // namespace coroutine

template <std::size_t STACK_SIZE>
class TCoroutine : public coroutine::Abstract
{
    using Register = coroutine::Register;

public:

    TCoroutine(void* entryPoint)
    {
        using namespace coroutine;

        _registers[RegisterMap::RDX_RETURN_ADDRESS ] = reinterpret_cast<Register>(entryPoint    );
        _registers[RegisterMap::RCX_STACK_POINTER  ] = reinterpret_cast<Register>(_stack.base() );
        _registers[RegisterMap::MMX_FPU_STATE      ] = reinterpret_cast<Register>(MMX_FPU_STATE );
    }

private:

    Register _registers[coroutine::RegisterMap::SIZE];

    coroutine::TStack<STACK_SIZE> _stack;
};

} // namespace schoco_details
