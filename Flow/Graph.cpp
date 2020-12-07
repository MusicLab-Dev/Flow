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