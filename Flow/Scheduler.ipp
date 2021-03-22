/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Scheduler
 */

template<bool IsRepeating>
inline void Flow::Scheduler::schedule(Graph &graph)
{
    if constexpr (!IsRepeating) {
        graph.preprocess();
        if (graph.running())
            throw std::logic_error("Flow::Scheduler::schedule: Can't schedule a graph if it is already running");
        graph.setRunning(true);
        graph.setScheduler(this);
    }
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
            if (targetId == count)
                targetId = 0;
            if (_lastWorkerId.compare_exchange_weak(id, targetId, std::memory_order_relaxed))
                break;
        }
        auto &worker = _cache.workers[targetId];
        if (worker.push(task)) {
            if (worker.state() == Worker::State::IDLE)
                worker.wakeUp(Worker::State::Running);
            break;
        }
    }
}