#pragma once

#include "schoco/details/coroutine/coroutine.hpp"
#include "schoco/details/coroutine/register.hpp"

extern "C" void* schocoDetailsCoroutineContextSwitch(schoco_details::coroutine::Abstract* srce,
                                                     schoco_details::coroutine::Abstract* dest);

extern "C" void schocoDetailsCoroutineMmxFpuSave(schoco_details::coroutine::Register*);

namespace schoco_details
{

namespace coroutine
{

void* contextSwitch(coroutine::Abstract& srce,
                    coroutine::Abstract& dest);

void mmxFpuSave(coroutine::Register&);

} // namespace coroutine

} // namespace schoco_details
