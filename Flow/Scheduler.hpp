/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Task scheduler
 */

#pragma once

#include <vector>

#include <Core/HeapArray.hpp>

#include "Worker.hpp"

namespace Flow
{
    class Scheduler;
}

/** @brief Schedule graph of tasks using thread workers */
class alignas_double_cacheline Flow::Scheduler
{
public:
    /** @brief Use this variable to automatically detect hardware thread count */
    static constexpr std::size_t AutoWorkerCount { 0ul };

    /** @brief This variable is used on hardware thread detection failure */
    static constexpr std::size_t DefaultWorkerCount { 4ul };

    /** @brief Default queue size of tasks */
    static constexpr std::size_t DefaultTaskQueueSize { 4096ul };

    /** @brief Default queue size of notifications */
    static constexpr std::size_t DefaultNotificationQueueSize { 4096ul };

    /** @brief Construct a set of workers and start scheduler */
    Scheduler(const std::size_t workerCount = AutoWorkerCount, const std::size_t taskQueueSize = DefaultTaskQueueSize, const std::size_t notificationQueueSize = DefaultNotificationQueueSize);

    /** @brief Destroy and join all workers */
    ~Scheduler(void);

    /** @brief Schedule a graph of tasks
     *  IsRepeating is used internally to repeat graphs */
    template<bool IsRepeating = false>
    void schedule(Graph &task);

    /** @brief Schedule a task */
    void schedule(const Task task) noexcept;

    /** @brief Tries to steal a task from a busy worker (only used by workers) */
    [[nodiscard]] bool steal(Task &task) noexcept;

    /** @brief Tries to add a notification task to be executed on the event processing thread */
    [[nodiscard]] bool notify(const Task task) noexcept { return _notifications.push(task); }

    /** @brief Process all pending notifications on the current thread */
    void processNotifications(void) { for (Task task; _notifications.pop(task); task.node()->notifyFunc()); }

    /** @brief All job to be terminated */
    void wait(void) noexcept;

    /** @brief Get the count of worker */
    [[nodiscard]] std::size_t workerCount(void) const noexcept { return _cache.workers.size(); }

private:
    struct Cache
    {
        Core::HeapArray<Worker> workers {};
    };

    alignas_cacheline Cache _cache {};
    alignas_cacheline std::atomic<std::size_t> _lastWorkerId { 0 };
    Core::MPMCQueue<Task> _notifications;
};

#include "Scheduler.ipp"
#include "Worker.ipp"