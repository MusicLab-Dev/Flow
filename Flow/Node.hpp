/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Node
 */

#pragma once

// This header must no be directly included, include 'Graph' instead

#include <memory_resource>
#include <variant>

#include <Core/FlatVector.hpp>
#include <Core/SmallVector.hpp>
#include <Core/FlatString.hpp>

#include "NodeType.hpp"

namespace Flow
{
    struct Node;
    class NodeInstance;

    class Graph;

    /** @brief Static node are used to execute independent jobs */
    using StaticNode = StaticFunc;

    /** @brief Dynamic node holds dynamic functor and a graph */
    struct DynamicNode
    {
        DynamicFunc func;
        Graph graph;
    };

    /** @brief Switch node is used to create branches */
    struct SwitchNode
    {
        SwitchFunc func;
        Core::FlatVector<std::size_t> joinCounts {};
    };

    /** @brief Graph node is used to construct nested graphs */
    using GraphNode = Graph;
}

/** @brief A node is a POD structure containing all data of a scheduled task in a graph */
struct alignas_double_cacheline Flow::Node
{
    /** @brief Work variant type */
    enum class WorkType {
        Static = 0,
        Dynamic,
        Switch,
        Graph
    };

    /** @brief Variant holding work struct */
    using WorkData = std::variant<StaticNode, DynamicNode, SwitchNode, GraphNode>;

    // Cacheline 1, frequently used members
    WorkData workData {}; // Work data variant
    Core::FlatVector<Node *> linkedTo {}; // List of forward tasks
    Core::FlatVector<Node *> linkedFrom {}; // List of children

    // Cacheline 2, rarely used members and frequently used atomic counter
    NotifyFunc notifyFunc {}; // Notify functor
    Core::FlatString name; // Node name
    std::atomic<std::uint32_t> joined { 0 }; // Joining
    alignas(4) std::atomic<bool> bypass { 0 }; // Bypass the node as if it was executed if true
    Graph *root { nullptr };

    /** @brief Construct a node with a work functor */
    template<typename Work>
    Node(Work &&work) noexcept
        : workData(ForwardWorkData(std::forward<Work>(work))) {}

    /** @brief Construct a node with a work functor and a name */
    template<typename Work, typename Literal, std::enable_if_t<std::is_constructible_v<decltype(Node::name), Literal>>* = nullptr>
    Node(Work &&work, Literal &&nodeName) noexcept
        : workData(ForwardWorkData(std::forward<Work>(work))), name(std::forward<Literal>(nodeName)) {}

    /** @brief Construct a node with a work and a notification functor */
    template<typename Work, typename Notify, std::enable_if_t<std::is_constructible_v<NotifyFunc, Notify>>* = nullptr>
    Node(Work &&work, Notify &&notify) noexcept
        : workData(ForwardWorkData(std::forward<Work>(work))), notifyFunc(std::forward<Notify>(notify)) {}

    /** @brief Construct a node with a work and a notification functor and a name */
    template<typename Work, typename Notify, typename Literal, std::enable_if_t<std::is_constructible_v<decltype(Node::name), Literal>>* = nullptr>
    Node(Work &&work, Notify &&notify, Literal &&nodeName) noexcept
        : workData(ForwardWorkData(std::forward<Work>(work))), notifyFunc(std::forward<Notify>(notify)), name(std::forward<Literal>(nodeName)) {}

    /** @brief Default destructor */
    ~Node(void) = default;

    /** @brief Helper to return the good workdata type from templated one */
    template<typename Work>
    inline static auto ForwardWorkData(Work &&work)
    {
        // Special rule for the dynamic node which can't specify its graph
        if constexpr (std::is_same_v<DynamicFunc, Work> || std::is_constructible_v<DynamicFunc, Work>) {
            return DynamicNode {
                std::forward<Work>(work),
                Graph()
            };
        // If we can't directly initialize a SwitchNode but we can convert it
        } else if constexpr (std::is_same_v<SwitchFunc, Work> || std::is_constructible_v<SwitchFunc, Work>) {
            return SwitchNode { std::forward<Work>(work) };
        // If we can't directly initialize a StaticNode but we can convert it
        } else if constexpr (!std::is_same_v<StaticNode, Work> && std::is_constructible_v<StaticNode, Work>) {
            return StaticNode { std::forward<Work>(work) };
        // Else, use default variant constructor
        } else
            return work;
    }
};

static_assert_fit_double_cacheline(Flow::Node);

class Flow::NodeInstance
{
public:
    /** @brief Default constructor */
    NodeInstance(void) = default;

    /** @brief Allocate constructor */
    template<typename ...Args>
    NodeInstance(Args &&...args)
        : _node(Allocate(std::forward<Args>(args)...)) {}

    /** @brief Move constructor */
    NodeInstance(NodeInstance &&other) noexcept { swap(other); }

    /** @brief Destroy the instance */
    ~NodeInstance(void) { if (_node) Deallocate(_node); }

    /** @brief Get node pointer */
    [[nodiscard]] Node *node(void) noexcept { return _node; }
    [[nodiscard]] const Node *node(void) const noexcept { return _node; }

    /** @brief Move assignment */
    NodeInstance &operator=(NodeInstance &&other) noexcept { swap(other); return *this; }

    /** @brief Swap two instances */
    void swap(NodeInstance &other) noexcept { std::swap(_node, other._node); }

    /** @brief Access operator */
    [[nodiscard]] Node *operator->(void) noexcept { return _node; }
    [[nodiscard]] const Node *operator->(void) const noexcept { return _node; }

private:
    Node *_node { nullptr };

    inline static std::pmr::synchronized_pool_resource _Pool {};

    template<typename ...Args>
    [[nodiscard]] inline static Node *Allocate(Args &&...args)
        { return new (_Pool.allocate(sizeof(Node), alignof(Node))) Node(std::forward<Args>(args)...); }

    inline static void Deallocate(Node *node) noexcept_destructible(Node)
        { node->~Node(); return _Pool.deallocate(node, sizeof(Node), alignof(Node)); }
};