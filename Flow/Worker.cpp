/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Worker thread
 */

#include <iostream>

#include "Scheduler.hpp"

Flow::Worker::Worker(Scheduler * const parent, const std::size_t queueSize)
    : _cache(Cache {
        parent: parent,
        thd: std::thread()
    }),
    _queue(queueSize)
{
}

void Flow::Worker::run(void)
{
    while (state() == State::Running) {
        if (Task task; _queue.pop(task) || _cache.parent->steal(task))
            work(task);
        else
            std::this_thread::yield();
    }
    _state = State::Stopped;
}

void Flow::Worker::work(Task &task)
{
    try {
        switch (task.type()) {
        case NodeType::Static:
            dispatchStaticNode(task.node());
            break;
        case NodeType::Dynamic:
            dispatchDynamicNode(task.node());
            break;
        case NodeType::Switch:
            dispatchSwitchNode(task.node());
            break;
        case NodeType::Graph:
            dispatchGraphNode(task.node());
            break;
        default:
            throw std::logic_error("Flow::Worker::Work: Undefined node");
        }
        if (!task.hasNotification()) [[likely]]
            return;
        // If the task has notification, loop until parent scheduler receive it
        while (!_cache.parent->notify(task) && state() == State::Running) {
            if (Task task; _queue.pop(task) || _cache.parent->steal(task))
                work(task);
            else
                std::this_thread::yield();
        }
    } catch (const std::exception &e) {
        std::cout << "Flow::Worker::work: Exception thrown in task '" << task.name() << "': " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Flow::Worker::work: Unknown exception thrown in task '" << task.name() << '\'' << std::endl;
    }
}