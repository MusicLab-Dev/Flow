/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Scheduler
 */

#include <gtest/gtest.h>

#include <Flow/Scheduler.hpp>

TEST(Scheduler, InitDestroy)
{
    for (auto i = 1ul; i <= 16; ++i) {
        Flow::Scheduler scheduler(i);
        ASSERT_EQ(scheduler.workerCount(), i);
    }
}

TEST(Scheduler, BasicTask)
{
    Flow::Scheduler scheduler(1);
    Flow::Graph graph;
    bool trigger = false;

    graph.emplace([&trigger] { trigger = true; });
    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, true);
}

TEST(Scheduler, SequenceTask)
{
    Flow::Scheduler scheduler(1);
    Flow::Graph graph;
    int trigger = false;
    auto func = [&trigger] {
        ++trigger;
    };
    auto a = graph.emplace(func);
    auto b = graph.emplace(func);
    auto c = graph.emplace(func);
    a.precede(b);
    b.precede(c);
    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 3);
}

TEST(Scheduler, NotifyTask)
{
    Flow::Scheduler scheduler(1);
    int trigger = false;
    auto func = [&trigger] {
        ++trigger;
    };
    Flow::Graph graph;
    graph.emplace(func, func);

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 1);
    scheduler.processNotifications();
    ASSERT_EQ(trigger, 2);
    auto func2 = [&trigger] {
        trigger += 2;
    };
    graph.clear();
    graph.emplace(func2, func2);
    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 4);
    scheduler.processNotifications();
    ASSERT_EQ(trigger, 6);
}

TEST(Scheduler, MergeTask)
{
    Flow::Scheduler scheduler;
    std::atomic<int> trigger = 0;
    Flow::Graph graph;
    auto a = graph.emplace([&trigger]{ trigger = 1; });
    auto b = graph.emplace([&trigger]{ trigger = 2; });
    auto c = graph.emplace([&trigger]{ trigger = 3; });
    a.precede(c);
    b.precede(c);

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 3);
}

TEST(Scheduler, ConditionTask)
{
    Flow::Scheduler scheduler;
    int trigger = 0;
    Flow::Graph graph;

    auto a = graph.emplace([&trigger]() -> bool { return trigger % 2; });
    auto b = graph.emplace([&trigger] { trigger = 1; });
    auto c = graph.emplace([&trigger] { trigger = 2; });
    a.precede(b); // False returned
    a.precede(c); // True returned

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 1);

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 2);

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 1);
}

TEST(Scheduler, SwitchTask)
{
    Flow::Scheduler scheduler;
    int trigger = 0;
    Flow::Graph graph;

    auto a = graph.emplace([&trigger]() -> int { return trigger; });
    auto b = graph.emplace([&trigger] { trigger = 1; });
    auto c = graph.emplace([&trigger] { trigger = 2; });
    auto d = graph.emplace([&trigger] { trigger = 3; });
    auto e = graph.emplace([&trigger] { trigger = 4; });
    b.succeed(a); // 0 returned
    c.succeed(a); // 1 returned
    d.succeed(a); // 2 returned
    e.succeed(a); // 3 returned

    for (auto i = 1; i <= 4; ++i) {
        scheduler.schedule(graph);
        graph.wait();
        ASSERT_EQ(trigger, i);
    }
}

TEST(Scheduler, GraphTask)
{
    Flow::Scheduler scheduler;
    int trigger = 0;
    auto func = [&trigger] { trigger += 1; };
    auto func2 = [&trigger] { trigger += 2; };

    // Construct sub graph
    Flow::Graph subGraph;
    auto a = subGraph.emplace(func);
    auto b = subGraph.emplace(func2);
    a.precede(b);

    // Construct graph
    Flow::Graph graph;
    auto before = graph.emplace(Flow::EmptyWork, func);
    auto sub = graph.emplace(subGraph, "SubGraph A");
    before.precede(sub);
    auto sub2 = graph.emplace(subGraph, "SubGraph B");
    sub.precede(sub2);
    auto after = graph.emplace(Flow::EmptyWork, func2);
    after.succeed(sub2);

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 6);
    scheduler.processNotifications();
    ASSERT_EQ(trigger, 9);
}

TEST(Scheduler, DynamicTask)
{
    Flow::Scheduler scheduler;
    Flow::Graph graph;
    int trigger = 0;
    bool clear = false;

    graph.emplace([&trigger, &clear](Flow::Graph &sub) {
        if (clear)
            sub.clear();
        sub.emplace([&trigger] { ++trigger; });
    });

    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 1);
    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 3);
    clear = true;
    scheduler.schedule(graph);
    graph.wait();
    ASSERT_EQ(trigger, 4);
}