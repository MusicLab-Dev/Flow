/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Worker
 */

inline void Flow::Worker::start(void)
{
    const auto state = _state.load();

    if (state != State::Stopped)
        throw std::logic_error("Flow::Worker::start: Worker already running");
    _state = State::Running;
    _cache.thd = std::thread([this] { run(); });
}

inline void Flow::Worker::stop(void) noexcept
{
    auto currentState = state();
    if (currentState == State::IDLE)
        wakeUp(State::Stopping);
    else if (currentState == State::Running) {
        while (!_state.compare_exchange_strong(currentState, State::Stopping)) {
            if (currentState == State::IDLE) {
                wakeUp(State::Stopping);
                break;
            } else if (currentState != State::Running)
                break;
        }
    }
}

inline void Flow::Worker::join(void) noexcept
{
    while (state() != State::Stopped) { // To be replaced by atomic wait when available
        std::this_thread::yield();
    }
    if (_cache.thd.joinable())
        _cache.thd.join();
}

inline void Flow::Worker::scheduleNode(Node * const node)
{
    if (const auto count = node->linkedFrom.size(); count && count == ++node->joined) {
        node->joined = 0;
        _cache.parent->schedule(node);
    }
}

inline void Flow::Worker::wakeUp(const State state) noexcept
{
    _state = state;
    __cxx_atomic_notify_one(reinterpret_cast<State *>(&_state));
}

inline void Flow::Worker::blockingGraphSchedule(Graph &graph)
{
    _cache.parent->schedule(graph);
    while (graph.running() && state() == State::Running) {
        if (Task task; _queue.pop(task) || _cache.parent->steal(task))
            work(task);
        else
            std::this_thread::yield();
    }
}

inline std::uint32_t Flow::Worker::dispatchStaticNode(Node * const node)
{
    if (!node->bypass.load())
        std::get<static_cast<std::size_t>(NodeType::Static)>(node->workData)();
    for (Node * const link : node->linkedTo)
        scheduleNode(link);
    return 1u;
}

inline std::uint32_t Flow::Worker::dispatchDynamicNode(Node * const node)
{
    if (!node->bypass.load()) {
        auto &dynamic = std::get<static_cast<std::size_t>(NodeType::Dynamic)>(node->workData);
        dynamic.func(dynamic.graph);
        blockingGraphSchedule(dynamic.graph);
    }
    return 1u;
}

inline std::uint32_t Flow::Worker::dispatchSwitchNode(Node * const node)
{
    auto &switchTask = std::get<static_cast<std::size_t>(NodeType::Switch)>(node->workData);
    const auto index = switchTask.func();
    const auto count = node->linkedTo.size();
    std::size_t joinCount = 1u;

    coreAssert(!node->bypass.load(),
        throw std::logic_error("A branch task can't be bypassed"));
    coreAssert(index >= 0ul && index < count,
        throw std::logic_error("Invalid switch task return index"));
    coreAssert(switchTask.joinCounts.size() == count,
        throw std::logic_error("Invalid switch task preprocessing, expected " + std::to_string(count) + " join counts but have " + std::to_string(switchTask.joinCounts.size())));
    scheduleNode(node->linkedTo[index]);
    for (std::size_t i = 0; i < count; ++i) {
        if (i != index) {
            joinCount += switchTask.joinCounts[i];
        } else
            continue;
    }
    return joinCount;
}

inline std::uint32_t Flow::Worker::dispatchGraphNode(Node * const node)
{
    if (!node->bypass.load()) {
        auto &graph = std::get<static_cast<std::size_t>(NodeType::Graph)>(node->workData);
        blockingGraphSchedule(graph);
    }
    for (const auto link : node->linkedTo)
        scheduleNode(link);
    return 1u;
}