/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Task Scheduler
 */

#include "Scheduler.hpp"

Flow::Scheduler::Scheduler(const std::size_t workerCount, const std::size_t taskQueueSize, const std::size_t notificationQueueSize)
    : _notifications(notificationQueueSize)
{
    auto count = workerCount;
    if (count == AutoWorkerCount)
        count = std::thread::hardware_concurrency();
    if (!count)
        count = DefaultWorkerCount;
    _lastWorkerId = count - 1;
    _cache.workers.allocate(count, this, taskQueueSize);
    for (auto &worker : _cache.workers)
        worker.start();
}

Flow::Scheduler::~Scheduler(void)
{
    for (auto &worker: _cache.workers)
        worker.stop();
    for (auto &worker: _cache.workers)
        worker.join();
}

bool Flow::Scheduler::steal(Flow::Task &task) noexcept
{
    for (auto &worker : _cache.workers) {
        if (worker.steal(task))
            return true;
    }
    return false;
}

void Flow::Scheduler::wait(void) noexcept
{
    const auto count = workerCount();

    while (true) {
        {
            auto activeCount = count;
            for (auto &worker : _cache.workers) {
                if (!worker.taskCount())
                    --activeCount;
            }
            if (!activeCount)
                return;
        }
        std::this_thread::yield();
    }
}