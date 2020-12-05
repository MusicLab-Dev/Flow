/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Task
 */


inline Flow::NodeType Flow::Task::type(void) const noexcept
{
    return static_cast<NodeType>(_node->workData.index());
}

template<typename Work>
inline void Flow::Task::setWork(Work &&work) noexcept
{
    _node->workData = Node::ForwardWorkData(std::forward<Work>(work));
}

inline bool Flow::Task::hasNotification(void) const noexcept
{
    return _node->notifyFunc.operator bool();
}

inline void Flow::Task::notify(void)
{
    _node->notifyFunc();
}

inline void Flow::Task::setNotify(NotifyFunc &&notifyFunc) noexcept
{
    _node->notifyFunc = std::move(notifyFunc);
}

inline Flow::Graph *Flow::Task::root(void) noexcept
{
    return _node->root;
}

inline const Flow::Graph *Flow::Task::root(void) const noexcept
{
    return _node->root;
}

inline std::string_view Flow::Task::name(void) const noexcept
{
    return _node->name.toStdView();
}

inline void Flow::Task::setName(const std::string_view &name) noexcept
{
    _node->name = name;
}

inline bool Flow::Task::bypass(void) const noexcept
{
    return _node->bypass.load();
}

inline void Flow::Task::setBypass(const bool &bypass) noexcept
{
    _node->bypass.store(bypass);
}

inline Flow::Task &Flow::Task::precede(Task &task) noexcept
{
    _node->linkedTo.push(task._node);
    task._node->linkedFrom.push(_node);
    return *this;
}