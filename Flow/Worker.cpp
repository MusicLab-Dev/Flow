/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Worker thread
 */

#include <iostream>

#include "Scheduler.hpp"

Flow::Worker::Worker(Scheduler * const parent, const std::size_t queueSize)
    : _cache(Cache {
        parent,
        std::thread()
    }),
    _queue(queueSize)
{
}

void Flow::Worker::run(void)
{
    while (state() == State::Running) {
        if (Task task; _queue.pop(task) || _cache.parent->steal(task))
            work(task);
        else {
            auto s = State::Running;
            if (!_state.compare_exchange_weak(s, State::IDLE))
                continue;
            __cxx_atomic_wait(reinterpret_cast<State *>(&_state), State::IDLE, std::memory_order::memory_order_relaxed);
        }
    }
    _state = State::Stopped;
}

void Flow::Worker::work(Task &task)
{
    try {
        std::uint32_t joinCount;
        switch (task.type()) {
        case NodeType::Static:
            joinCount = dispatchStaticNode(task.node());
            break;
        case NodeType::Dynamic:
            joinCount = dispatchDynamicNode(task.node());
            break;
        case NodeType::Switch:
            joinCount = dispatchSwitchNode(task.node());
            break;
        case NodeType::Graph:
            joinCount = dispatchGraphNode(task.node());
            break;
        default:
            throw std::logic_error("Flow::Worker::Work: Undefined node");
        }
        if (!task.hasNotification()) {
            task.node()->root->childrenJoined(joinCount);
            return;
        }
        // If the task has notification, loop until parent scheduler accept it
        while (!_cache.parent->notify(task) && state() == State::Running) {
            if (Task task; _queue.pop(task) || _cache.parent->steal(task))
                work(task);
            else
                std::this_thread::yield();
        }
        task.node()->root->childrenJoined(joinCount);
    } catch (const std::exception &e) {
        std::cout << "Flow::Worker::work: Exception thrown in task '" << task.name() << "': " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Flow::Worker::work: Unknown exception thrown in task '" << task.name() << '\'' << std::endl;
    }
}