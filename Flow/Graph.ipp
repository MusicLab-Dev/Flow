/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Graph
 */

inline void Flow::Graph::acquire(const Graph &other) noexcept
{
    if (other._data) [[likely]] {
        _data = other._data;
        ++_data->sharedCount;
    }
}

inline void Flow::Graph::release(void)
{
    if (_data && --_data->sharedCount == 0u) [[unlikely]] {
        wait();
        _data->~Data();
        _Pool.deallocate(_data, sizeof(Data), alignof(Data));
    }
}

inline void Flow::Graph::construct(void) noexcept
{
    if (!_data) [[unlikely]]
        _data = new (_Pool.allocate(sizeof(Data), alignof(Data))) Data {};
}

template<typename ...Args>
inline Flow::Task Flow::Graph::emplace(Args &&...args)
{
    construct();
    const auto node = _data->children.push(std::forward<Args>(args)...).node();
    node->root = this;
    return Task(node);
}

inline void Flow::Graph::wait(void) noexcept_ndebug
{
    while (running())
        std::this_thread::yield();
}

inline void Flow::Graph::clearLinks(void) noexcept
{
    for (auto &child : *this) {
        child->linkedFrom.clear();
        child->linkedTo.clear();
    }
}

inline void Flow::Graph::clear(void)
{
    if (_data) [[likely]] {
        wait();
        _data->children.clear();
    }
}