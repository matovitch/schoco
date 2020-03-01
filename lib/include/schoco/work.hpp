#pragma once

#include "schoco/details/pool/pool.hpp"

#include <functional>
#include <optional>

namespace schoco
{

namespace work
{

struct Abstract
{
    virtual void run() = 0;

    virtual ~Abstract() {}
};

} // namespace work

template <class ReturnType>
class TWork : public work::Abstract
{

public:

    template <class Function>
    TWork(Function&& function) :
        _function{std::move(function)}
    {}

    void run() override
    {
        _returnOpt = _function();
    }

    const std::optional<ReturnType>& returnOpt() const
    {
        return _returnOpt;
    }

private:
    const std::function<ReturnType()> _function;
    std::optional<ReturnType>         _returnOpt;
};

template <>
class TWork<void> : public work::Abstract
{

public:

    template <class Function>
    TWork(Function&& function) :
        _function{std::move(function)}
    {}

    void run() override
    {
        _function();
    }

private:
    const std::function<void()> _function;
};

namespace work
{

namespace pool
{

template <class ReturnType, std::size_t SIZE = 1>
using TMake = schoco_details::pool::TMake<TWork<ReturnType>, SIZE>;

} // namespace pool

} // namespace work

} // namespace schoco
