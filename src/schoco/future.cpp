#include "schoco/scheduler.hpp"
#include "schoco/coroutine.hpp"
#include "schoco/task.hpp"
#include "schoco/work.hpp"

#include <iostream>
#include <thread>
#include <future>

using namespace std::chrono_literals;

int main()
{
    // Setup all the pools managing our allocations
    schoco::coroutine::pool::TMake<0x8000/*STACK_SIZE*/> poolCoroutine;
    schoco::scheduler::PoolSet                           poolScheduler;
    schoco::work::pool::TMake<void>                      poolWork;

    // Create our scheduler and tasks
    schoco::Scheduler scheduler{poolScheduler, 10us /*future's polling interval*/};
    auto ping = scheduler.makeTask(poolCoroutine);
    auto pong = scheduler.makeTask(poolCoroutine);

    // Create a dependency: pong depends on ping
    schoco::Scheduler::Dependency dependency;
    pong.attach(ping, dependency);

    // Assign work to ping
    ping.assignWork(poolWork,
        [&ping, &pong, &scheduler, &dependency]()
        {
            std::cout << "Ping!\n";
            scheduler.detach(dependency);
            ping.attach(pong, dependency);
            std::cout << "Ping!\n";
            scheduler.detach(dependency);
            ping.attach(pong, dependency);
            std::cout << "Ping!\n";
            auto&& future = std::async(
                []
                {
                    std::this_thread::sleep_for(2s);
                    std::cout << "Pong!\n";
                }
            );
            ping.attach(future, dependency);
            std::cout << "Ping!\n";
        }
    );

    // Assign work to pong
    pong.assignWork(poolWork,
        [&pong, &ping, &scheduler, &dependency]()
        {
            std::cout << "Pong!\n";
            scheduler.detach(dependency);
            pong.attach(ping, dependency);
            std::cout << "Pong!\n";
        }
    );

    // Start executing
    scheduler.run();

    return 0;
}