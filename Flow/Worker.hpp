/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Worker thread
 */

#pragma once

// This header must no be directly included, include 'Scheduler' instead

#ifndef WIN32
# include <atomic_wait>
#endif

#include <Core/MPMCQueue.hpp>

#include "Graph.hpp"

namespace Flow
{
    class Worker;
    class Scheduler;
}

class alignas_double_cacheline Flow::Worker
{
public:
    /** @brief Current state of the worker */
#ifdef __APPLE__ 
    enum State { // Apple does not allow the enum class as a valid integral for atomic operations
#else
    enum class State {
#endif
        Running,    // Worker is running
        IDLE,       // Worker is waiting for a new task
        Stopping,   // Worker is stopping
        Stopped,    // Worker is stopped
    };

    /** @brief Construct and start the worker */
    Worker(Scheduler * const parent, const std::size_t queueSize);

    /** @brief Destroy the worker without stopping it ! */
    ~Worker(void) = default;

    /** @brief Start the worker */
    void start(void);

    /** @brief Stop the worker */
    void stop(void) noexcept;

    /** @brief Join the worker */
    void join(void) noexcept;

    /** @brief Get internal state of worker */
    [[nodiscard]] State state(void) noexcept { return _state.load(std::memory_order_relaxed); }

    /** @brief Push a task to be processed on the worker thread */
    [[nodiscard]] bool push(const Task task) noexcept { return _queue.push(task); }

    /** @brief Try to steal a task from worker */
    [[nodiscard]] bool steal(Task &task) noexcept { return _queue.pop(task); }

    /** @brief Get the task count of the queue */
    [[nodiscard]] std::size_t taskCount(void) const noexcept { return _queue.size(); }

    /** @brief Notify that the worker should work right now */
    void wakeUp(const State state) noexcept;

private:
    struct Cache
    {
        Scheduler *parent { nullptr };
        std::thread thd {};
    };

    alignas_cacheline std::atomic<State> _state { State::Stopped };
    alignas_cacheline Cache _cache {};
    Core::MPMCQueue<Task> _queue;

    /** @brief Busy loop */
    void run(void);

    /** @brief Execute a task */
    void work(Task &task);

private:
    /** @brief Work untile given graph finished */
    void blockingGraphSchedule(Graph &graph);

    /** @brief Tries to schedule a single node */
    void scheduleNode(Node * const node);

    /** @brief Helper used to process a Static node */
    [[nodiscard]] std::uint32_t dispatchStaticNode(Node * const node);

    /** @brief Helper used to process a Dynamic node */
    [[nodiscard]] std::uint32_t dispatchDynamicNode(Node * const node);

    /** @brief Helper used to process a Switch node */
    [[nodiscard]] std::uint32_t dispatchSwitchNode(Node * const node);

    /** @brief Helper used to process a Graph node */
    [[nodiscard]] std::uint32_t dispatchGraphNode(Node * const node);
};

static_assert_sizeof(Flow::Worker, 6 * Core::CacheLineSize);
static_assert_alignof_double_cacheline(Flow::Worker);
