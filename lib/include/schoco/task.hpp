#pragma once

#include "schoco/coroutine.hpp"
#include "schoco/work.hpp"

#include "schoco/details/graph/graph.hpp"

#include <optional>
#include <future>

namespace schoco
{

template <std::size_t>
class TTask;

namespace task
{

template <class ReturnType>
struct TNoReturn
{
    static const std::optional<ReturnType> _value;
};

template <class ReturnType>
const std::optional<ReturnType> TNoReturn<ReturnType>::_value;

template <std::size_t SIZE>
struct TEntryPoint
{
    static void run()
    {
        _next->_value.run();
    } 

    using TaskHandle = typename schoco_details::graph::TMake<TTask<SIZE>, SIZE>::NodeListIt;

    static void* _value;

    static thread_local TaskHandle _next;
};



template <std::size_t SIZE>
thread_local typename TEntryPoint<SIZE>::TaskHandle TEntryPoint<SIZE>::_next;

template <std::size_t SIZE>
void* TEntryPoint<SIZE>::_value = reinterpret_cast<void*>(&TEntryPoint<SIZE>::run);

template <std::size_t>
class THandleWrapper;

} // namespace task

template <std::size_t>
class TScheduler;



template <std::size_t SIZE>
class TTask
{
    friend TScheduler<SIZE>;

    using TaskHandle = typename schoco_details::graph::TMake<TTask<SIZE>, SIZE>::NodeListIt;
    using Dependency = typename schoco_details::graph::TMake<TTask<SIZE>, SIZE>::EdgeListIt;

    TTask(const TTask<SIZE>&  ) = delete;
    TTask(      TTask<SIZE>&& ) = delete;

public:

    template <std::size_t STACK_SIZE>
    TTask(TScheduler<SIZE>& scheduler,
         schoco::coroutine::pool::TMake<STACK_SIZE>& coroutinePool) :
        _scheduler{scheduler},
        _coroutine{coroutinePool.make(task::TEntryPoint<SIZE>::_value)}
    {}

    void attach(TaskHandle task, Dependency& dependency)
    {
        _scheduler.attach(_me.value(), task, dependency);

        _scheduler.releaseThread(_me.value());
    }

    template <class Type>
    void attach(const std::future<Type>& future, Dependency& dependency)
    {
        _scheduler.attach(_me.value(), future, dependency);

        _scheduler.releaseThread(_me.value());
    }

    template <class Type, class Rep, class Period>
    void attach(const std::future<Type>& future, Dependency& dependency,
                const std::chrono::duration<Rep, Period>& timeoutDuration)
    {
        _scheduler.attach(_me.value(), future, dependency, timeoutDuration);

        _scheduler.releaseThread(_me.value());
    }

    /* Due to http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1591,
       template deduction doesn't work with std::array. */
    template <std::size_t BATCH_SIZE>
    void attachBatch(std::reference_wrapper<task::THandleWrapper<SIZE> > const (&tasks        )[BATCH_SIZE],
                     std::reference_wrapper<Dependency                 > const (&dependencies )[BATCH_SIZE])
    {
        _scheduler.attachBatch(_me.value(), tasks, dependencies);

        _scheduler.releaseThread(_me.value());
    }

    template <class ReturnType, std::size_t POOL_SIZE, class Function>
    void assignWork(work::pool::TMake<ReturnType, POOL_SIZE>& workPool, Function&& function)
    {
        _workOpt = workPool.make(function);
    }

    void detachAll()
    {
        _scheduler.detachAll(_me.value());
    }

    template <class ReturnType>
    const std::optional<ReturnType>& returnOpt() const
    {
        return _workOpt ? static_cast<const TWork<ReturnType>&>(_workOpt.value().get()).returnOpt()
                        : task::TNoReturn<ReturnType>::_value;
    }

    void run()
    {
        _isRunning = true;

        if (_workOpt)
        {
            _workOpt.value().get().run();
        }

        _isRunning = false;

        _scheduler.pop(_me.value());
    }

private:

    std::optional<std::reference_wrapper<work::Abstract>> _workOpt;
    TScheduler<SIZE>&                                     _scheduler;
    std::optional<TaskHandle>                             _me;
    schoco_details::coroutine::Abstract&                  _coroutine;
    bool                                                  _isRunning = false;
};

namespace task
{

template <std::size_t SIZE>
class THandleWrapper
{
    friend TScheduler<SIZE>;

    using TaskHandle = typename schoco_details::graph::TMake<TTask<SIZE>, SIZE>::NodeListIt;
    using Dependency = typename schoco_details::graph::TMake<TTask<SIZE>, SIZE>::EdgeListIt;

public:

    THandleWrapper(const TaskHandle taskHandle) : _taskHandle{taskHandle} {}

    template <class ReturnType, std::size_t POOL_SIZE, class Function>
    void assignWork(work::pool::TMake<ReturnType, POOL_SIZE>& workPool, Function&& function)
    {
        _taskHandle->_value.assignWork(workPool, function);
    }

    void attach(THandleWrapper task, Dependency& dependency)
    {
        _taskHandle->_value.attach(task._taskHandle, dependency);
    }

    template <class Type>
    void attach(const std::future<Type>& future, Dependency& dependency)
    {
        _taskHandle->_value.attach(future, dependency);
    }

    template <class Type, class Rep, class Period>
    void attach(const std::future<Type>& future, Dependency& dependency,
                const std::chrono::duration<Rep, Period>& timeoutDuration)
    {
        _taskHandle->_value.attach(future, dependency, timeoutDuration);
    }

    /* Due to http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1591,
       template deduction doesn't work with std::array. */
    template <std::size_t BATCH_SIZE>
    void attachBatch(std::reference_wrapper<THandleWrapper > const (&tasks        )[BATCH_SIZE],
                     std::reference_wrapper<Dependency     > const (&dependencies )[BATCH_SIZE])
    {
        _taskHandle->_value.attachBatch(tasks, dependencies);
    }

    void detachAll()
    {
        _taskHandle->_value.detachAll();
    }

    template <class ReturnType>
    const std::optional<ReturnType>& returnOpt() const
    {
        return _taskHandle->_value.returnOpt();
    }

private:

    TaskHandle _taskHandle;
};

} // namespace task

} // namespace schoco
