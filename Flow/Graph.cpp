/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Graph
 */

#include "Scheduler.hpp"

void Flow::Graph::childrenJoined(const std::uint32_t childrenJoined) noexcept
{
    if (const auto count = _data->children.size(); (_data->joined += childrenJoined) == count) {
        _data->joined = 0;
        if (hasRepeatCallback() && _data->repeatCallback())
            _data->scheduler->schedule<true>(*this);
        else {
            setRunning(false);
            setScheduler(nullptr);
        }
    }
}

void Flow::Graph::preprocessImpl(void)
{
    for (auto &node : *this) {
        if (node->workData.index() != static_cast<std::size_t>(Node::WorkType::Switch))
            continue;
        auto &switchTask = std::get<static_cast<std::size_t>(Node::WorkType::Switch)>(node->workData);
        switchTask.joinCounts.reserve(node->linkedTo.size());
        for (const auto childNode : node->linkedTo) {
            std::size_t count { 1u };
            countSubChildren(*childNode, count);
            switchTask.joinCounts.push(count);
        }
    }
    _data->isPreprocessed = true;
}

void Flow::Graph::countSubChildren(const Node &node, std::size_t &count)
{
    count += node.linkedTo.size();
    for (const auto childNode : node.linkedTo)
        countSubChildren(*childNode, count);
}