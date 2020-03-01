#include "schoco/details/coroutine/context_switch.hpp"
#include "schoco/details/coroutine/coroutine.hpp"
#include "schoco/details/coroutine/register.hpp"

#include <cstdint>

namespace schoco_details
{

namespace coroutine
{

void* contextSwitch(schoco_details::coroutine::Abstract& srce,
                    schoco_details::coroutine::Abstract& dest)
{
    return schocoDetailsCoroutineContextSwitch(&srce, &dest);
}

void mmxFpuSave(schoco_details::coroutine::Register& mmxFpuState)
{
    schocoDetailsCoroutineMmxFpuSave(&mmxFpuState);
}

} // namespace coroutine

} // namespace schoco_details
