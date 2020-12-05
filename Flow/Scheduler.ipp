/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Scheduler
 */

#pragma once

inline void Flow::Scheduler::schedule(Graph &graph)
{
    if (graph.running())
        throw std::logic_error("Flow::Scheduler::schedule: Can't schedule a graph if it is already running");
    graph.setRunning(true);
    for (auto &child : graph) {
        if (child->linkedFrom.empty())
            schedule(Task(child.node()));
    }
}

inline void Flow::Scheduler::schedule(const Task task) noexcept
{
    const auto count = workerCount();
    auto id = _lastWorkerId.load(std::memory_order_relaxed);
    std::size_t targetId;

    while (true) {
        while (true) {
            targetId = id + 1;
            if (targetId == count) [[unlikely]]
                targetId = 0;
            if (_lastWorkerId.compare_exchange_weak(id, targetId, std::memory_order_relaxed)) [[likely]]
                break;
        }
        if (_cache.workers[targetId].push(task)) [[likely]]
            break;
    }
}