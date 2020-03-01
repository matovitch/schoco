#pragma once

namespace schoco_details
{

namespace buffer
{

template <class Type>
struct TAbstract
{
    virtual Type* allocate() = 0;

    virtual ~TAbstract() {}
};

} // namespace buffer

} // namespace schoco_details
