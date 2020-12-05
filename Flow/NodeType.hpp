/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Types of nodes
 */

#pragma once

#include <functional>

#include <Core/Functor.hpp>

namespace Flow
{
    class Graph;

    /** @brief Static functor */
    using StaticFunc = Core::Functor<void(void)>;

    /** @brief Switch functor */
    using SwitchFunc = Core::Functor<std::size_t(void)>;

    /** @brief Dynamic functor */
    using DynamicFunc = Core::Functor<void(Graph &)>;

    /** @brief Notify functor to be called on the event thread */
    using NotifyFunc = Core::Functor<void(void)>;

    /** @brief Different types of nodes */
    enum class NodeType : std::size_t {
        Static = 0ul,
        Dynamic,
        Switch,
        Graph
    };

    /** @brief Empty work placeholder */
    constexpr auto EmptyWork = []{};
}