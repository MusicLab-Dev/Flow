/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Task graph
 */

#pragma once

#include <thread>
#include <memory>
#include <memory_resource>

#include <Core/Assert.hpp>
#include <Core/Vector.hpp>

#include "Task.hpp"

namespace Flow
{
    class NodeInstance;
    class Graph;

    class Scheduler;
}

class alignas_eighth_cacheline Flow::Graph
{
public:
    /** @brief Data of the task graph */
    struct alignas_cacheline Data
    {
        Core::TinyVector<NodeInstance> children; // Children instances
        std::atomic<std::uint32_t> joined { 0 }; // Number of joined nodes
        std::atomic<std::uint16_t> sharedCount { 1 }; // Number of shared graph instances
        std::atomic<bool> running { false }; // True if the graph is already processing
        bool isPreprocessed { false }; // True if the graph is already preprocessed and safe to schedule
        Scheduler *scheduler { nullptr }; // The scheduler that ran the graph
        Core::Functor<bool(void)> repeatCallback {}; // On true returned, it will immediatly repeat the graph after it succeeded
    };

    static_assert_fit_cacheline(Data);

    /** @brief Shared pointer to data structure */
    using DataPtr = std::shared_ptr<Data>;

    /** @brief Children iterator */
    using Iterator = decltype(Data::children)::Iterator;
    using ConstIterator = decltype(Data::children)::ConstIterator;

    /** @brief Default construtor */
    Graph(void) noexcept = default;

    /** @brief Copy constructor */
    Graph(const Graph &other) noexcept { acquire(other); }

    /** @brief Move constructor */
    Graph(Graph &&other) noexcept { swap(other); };

    /** @brief Destructor */
    ~Graph(void) { release(); }

    /** @brief Copy assignment */
    Graph &operator=(const Graph &other) noexcept { acquire(other); return *this; }

    /** @brief Move assignment */
    Graph &operator=(Graph &&other) noexcept { swap(other); return *this; }

    /** @brief Fast check */
    operator bool(void) const noexcept { return _data != nullptr; }

    /** @brief Swap two graph */
    void swap(Graph &other) noexcept { std::swap(_data, other._data); }


    /** @brief Acquire a reference to another graph */
    void acquire(const Graph &other) noexcept;

    /** @brief Release the instance */
    void release(void);

    /** @brief Construct an instance if not already done */
    void construct(void) noexcept;


    /** @brief Get the running property */
    [[nodiscard]] bool running(void) const noexcept { return _data->running.load(std::memory_order_seq_cst); }


    /** @brief Check if the graph has a repeat callback */
    [[nodiscard]] bool hasRepeatCallback(void) const noexcept { return _data->repeatCallback; }

    /** @brief Set the notify functor */
    template<typename Callback>
    void setRepeatCallback(Callback &&callback) noexcept
        { construct(); _data->repeatCallback = std::forward<Callback>(callback); }


    /** @brief Emplace a node in the graph */
    template<typename ...Args>
    Task emplace(Args &&...args);


    /** @brief Wait for the graph to be executed */
    void wait(void) noexcept_ndebug;


    /** @brief Clear every node link (node are still valid) */
    void clearLinks(void) noexcept;

    /** @brief Clear the graph children */
    void clear(void);

    /** @brief Ensure that the graph is ready to be scheduled (called by the Scheduler on schedule) */
    void preprocess(void) noexcept;


    /** @brief Get the number of owned nodes */
    [[nodiscard]] auto size(void) const noexcept { return _data->children.size(); }

    /** @brief Begin / end iterators to iterate over children nodes */
    [[nodiscard]] Iterator begin(void) noexcept { return _data->children.begin(); }
    [[nodiscard]] Iterator begin(void) const noexcept { return _data->children.begin(); }
    [[nodiscard]] ConstIterator end(void) noexcept { return _data->children.end(); }
    [[nodiscard]] ConstIterator end(void) const noexcept { return _data->children.end(); }

public:
    /** @brief Set the running property
     *  Reserved for internal use ! */
    void setRunning(const bool running) noexcept { _data->running.store(running, std::memory_order_seq_cst); }

    /** @brief Get the joined property
     *  Reserved for internal use ! */
    [[nodiscard]] std::uint32_t joined(void) const noexcept { return _data->joined.load(std::memory_order_seq_cst); }

    /** @brief Callback that increment join count (to know when graph is done)
     *  Reserved for internal use ! */
    void childJoined(void) noexcept { childrenJoined(1); }
    void childrenJoined(const std::uint32_t childrenJoined) noexcept;

    /** @brief Set the scheduler property
     *  Reserved for internal use ! */
    void setScheduler(Scheduler * const scheduler) noexcept { _data->scheduler = scheduler; }

private:
    Data *_data { nullptr };

    inline static std::pmr::synchronized_pool_resource _Pool {};

    /** @brief Implementation of the preprocess algorithm */
    void preprocessImpl(void) noexcept;

    /** @brief Count the numbr of ssubchildren of a node */
    void countSubChildren(const Node &node, std::size_t &count, Core::TinyVector<const Node *> &cache) noexcept;
};

static_assert_fit_eighth_cacheline(Flow::Graph);

#include "Node.hpp" // Include the node to compile Task.ipp and Graph.ipp
#include "Task.ipp"
#include "Graph.ipp"
