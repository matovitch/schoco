#pragma once

#include "schoco/details/coroutine/context_switch.hpp"
#include "schoco/details/coroutine/register.hpp"
#include "schoco/coroutine.hpp"
#include "schoco/task.hpp"

#include <functional>
#include <optional>
#include <future>
#include <chrono>

namespace schoco
{

namespace scheduler
{

template <std::size_t SIZE> 
struct TPoolSet
{
    using Task       = TTask<SIZE>;
    using TaskGraph  = schoco_details::graph::TMake<Task, SIZE>;
    using TaskHandle = typename TaskGraph::NodeListIt;

    typename TaskGraph::PoolSet                                      _poolSetTaskGraph;
    typename schoco_details::list::TMake<TaskHandle, SIZE>::CellPool _poolFuture;
};

using PoolSet = TPoolSet<1>;

} // namespace scheduler

template <std::size_t SIZE>
class TScheduler
{
   using Task      = TTask<SIZE>;
   using TaskGraph = schoco_details::graph::TMake<Task, SIZE>;

   friend Task;

public:

    using TaskHandleWrapper = task::THandleWrapper<SIZE>;
    using TaskHandle        = typename TaskGraph::NodeListIt;
    using Dependency        = typename TaskGraph::EdgeListIt;

    template <class Rep, class Period>
    TScheduler(scheduler::TPoolSet<SIZE>& poolSetScheduler,
               const std::chrono::duration<Rep, Period>& futuresPollDelay) :
        _coroutine {_coroutinePool.make(nullptr)},
        _taskGraph {poolSetScheduler._poolSetTaskGraph},
        _futures   {poolSetScheduler._poolFuture},
        _futuresPollDelay{futuresPollDelay}
    {
        schoco_details::coroutine::mmxFpuSave(schoco_details::coroutine::MMX_FPU_STATE);
    }

    template <std::size_t STACK_SIZE>
    TaskHandleWrapper makeTask(schoco::coroutine::pool::TMake<STACK_SIZE>& coroutinePool)
    {
        TaskHandle taskHandle = _taskGraph.makeNode(*this, coroutinePool);
        taskHandle->_value._me = taskHandle;
        return taskHandle;
    }

    void run()
    {
        _isRunning = true;

        while (!_taskGraph .empty() ||
               !_futures   .empty())
        {
            checkFutures();

            if (_taskGraph.empty())
            {
                std::this_thread::sleep_for(_futuresPollDelay);
                continue;
            }

            schoco_details::coroutine::contextSwitch(_coroutine,
                                                     nextCoroutine());
        }

        _isRunning = false;
    }

    void detach(Dependency& dependency)
    {
        _taskGraph.detach(dependency);
    }

    void detachAll(TaskHandleWrapper task)
    {
        _taskGraph.detachAll(task);
    }

    void attach(TaskHandleWrapper lhs,
                TaskHandleWrapper rhs,
                Dependency& dependency)
    {
        dependency = _taskGraph.attach(lhs._taskHandle,
                                       rhs._taskHandle);
    }

    /* Due to http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1591,
       template deduction doesn't work with std::array. */
    template <std::size_t BATCH_SIZE>
    void attachBatch(TaskHandleWrapper lhs,
                     std::reference_wrapper<TaskHandleWrapper > const (&rhss         )[BATCH_SIZE],
                     std::reference_wrapper<Dependency        > const (&dependencies )[BATCH_SIZE])
    {
        for (std::size_t i = 0; i < BATCH_SIZE; i++)
        {
            attach(lhs, rhss[i], dependencies[i]);
        }
    }

    template <class Type>
    void attach(TaskHandleWrapper task,
                const std::future<Type>& future,
                Dependency& dependency)
    {
        auto futureTask = makeTask(_coroutinePool);

        futureTask.assignWork(_futurePool,
                              [&future]
                              {
                                  using namespace std::chrono_literals;
                                  return future.wait_for(0s) == std::future_status::ready;
                              });

        _taskGraph.attach(futureTask._taskHandle,
                          futureTask._taskHandle);

        attach(task, futureTask, dependency);

        _futures.emplace_front(futureTask._taskHandle);
    }

    template <class Type, class Rep, class Period>
    void attach(TaskHandleWrapper task,
                const std::future<Type>& future,
                Dependency& dependency,
                const std::chrono::duration<Rep, Period>& timeoutDuration)
    {
        auto futureTask = makeTask(_coroutinePool);

        const auto timeout = std::chrono::steady_clock::now() + timeoutDuration;

        futureTask.assignWork(_futurePool,
                              [&future, timeout]
                              {
                                  using namespace std::chrono_literals;
                                  return future.wait_for(0s) == std::future_status::ready ||
                                         std::chrono::steady_clock::now() > timeout;
                              });

        _taskGraph.attach(futureTask._taskHandle,
                          futureTask._taskHandle);

        attach(task, futureTask, dependency);

        _futures.emplace_front(futureTask._taskHandle);
    }

private:

    schoco_details::coroutine::Abstract& nextCoroutine()
    {
        if (_taskGraph.empty())
        {
            return _coroutine;
        }

        if (!_taskGraph.top()->_value._isRunning)
        {
            task::TEntryPoint<SIZE>::_next = _taskGraph.top();
        }

        return _taskGraph.top()->_value._coroutine;
    }

    void releaseThread(const TaskHandle taskHandle)
    {
        if (!_isRunning || (taskHandle->_value._isRunning && taskHandle->isPending()))
        {
            return;
        }

        schoco_details::coroutine::contextSwitch(taskHandle->_value._coroutine,
                                                 nextCoroutine());
    }

    void pop(const TaskHandle taskHandle)
    {
        // Note: The pop() bellow release the memory from 'task' to the allocator
        // but this is safe as it is destroyed only when the slot gets reallocated
        // (this is NOT thread-safe, as is everything here)
        _taskGraph.pop(taskHandle);

        releaseThread(taskHandle);
    }

    void checkFutures()
    {
        auto it = _futures.begin();

        while (it != _futures.end())
        {
            (*it)->_value._workOpt.value().get().run();

            const auto opt = (*it)->_value.template returnOpt<bool>();

            if (!opt.value())
            {
                it++;
                continue;
            }

            _taskGraph.pop(*it);
            it = _futures.erase(it);
        }
    }

    bool _isRunning = false;

    schoco_details::coroutine::Abstract&            _coroutine;
    schoco_details::graph ::TMake<Task, SIZE>       _taskGraph;
    schoco_details::list  ::TMake<TaskHandle, SIZE> _futures;

    std::chrono::nanoseconds _futuresPollDelay;

    static schoco::coroutine::pool::TMake<0x1000> _coroutinePool;
    static schoco::work::pool::TMake<bool>        _futurePool;
};

template <std::size_t SIZE>
schoco::coroutine::pool::TMake<0x1000> TScheduler<SIZE>::_coroutinePool;

template <std::size_t SIZE>
schoco::work::pool::TMake<bool> TScheduler<SIZE>::_futurePool;

using Scheduler = TScheduler<1>;

} // namespace schoco
