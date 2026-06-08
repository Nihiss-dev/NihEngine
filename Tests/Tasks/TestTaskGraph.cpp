#include <gtest/gtest.h>
#include "Tasks/Task.h"
#include "Tasks/TaskGraph.h"

// ---------------------------------------------------------------------------
// Dummy data tags for Requires/Mutates
// ---------------------------------------------------------------------------
struct PhysicsData {};
struct TransformData {};
struct AnimationData {};

// ---------------------------------------------------------------------------
// Test fixtures — tasks with various phase/dependency combinations
// ---------------------------------------------------------------------------

// Single task, no dependencies
struct StandaloneTask : public Task<TaskPhase::Gameplay, RunAfter<>>
{
    void Init() override {}
    void Update(float) override {}
};

// Two tasks, B depends on A
struct TaskA : public Task<TaskPhase::Gameplay, RunAfter<>>
{
    void Init() override {}
    void Update(float) override {}
};

struct TaskB : public Task<TaskPhase::Gameplay, RunAfter<TaskA>>
{
    void Init() override {}
    void Update(float) override {}
};

// Three tasks: C depends on B, B depends on A
struct TaskC : public Task<TaskPhase::Gameplay, RunAfter<TaskB>>
{
    void Init() override {}
    void Update(float) override {}
};

// Diamond: D and E both depend on F, G depends on both D and E
struct TaskF : public Task<TaskPhase::Animation, RunAfter<>>
{
    void Init() override {}
    void Update(float) override {}
};

struct TaskD : public Task<TaskPhase::Animation, RunAfter<TaskF>>
{
    void Init() override {}
    void Update(float) override {}
};

struct TaskE : public Task<TaskPhase::Animation, RunAfter<TaskF>>
{
    void Init() override {}
    void Update(float) override {}
};

struct TaskG : public Task<TaskPhase::Animation, RunAfter<TaskD, TaskE>>
{
    void Init() override {}
    void Update(float) override {}
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Returns true if indexof(a) < indexof(b) in the sorted order
template<typename TaskList, typename A, typename B>
constexpr bool ComesBeforeInSort(const std::array<int, TaskList::kCount>& order)
{
    constexpr int idxA = IndexOf<A, TaskList>::kValue;
    constexpr int idxB = IndexOf<B, TaskList>::kValue;
    int posA = -1, posB = -1;
    for (int i = 0; i < static_cast<int>(order.size()); ++i)
    {
        if (order[i] == idxA) posA = i;
        if (order[i] == idxB) posB = i;
    }
    return posA < posB;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

// Single task — graph should produce a 1-element sorted array
TEST(TaskGraph, SingleTask)
{
    using List = TaskList<StandaloneTask>;
    constexpr auto order = TaskGraph<List>::kSortedOrder;

    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0], 0);
}

// Linear chain A -> B: A must come before B
TEST(TaskGraph, LinearChain_ABeforeB)
{
    using List = TaskList<TaskA, TaskB>;
    constexpr auto order = TaskGraph<List>::kSortedOrder;

    EXPECT_TRUE((ComesBeforeInSort<List, TaskA, TaskB>(order)));
}

// Linear chain A -> B -> C: A before B before C
TEST(TaskGraph, LinearChain_ABC)
{
    using List = TaskList<TaskA, TaskB, TaskC>;
    constexpr auto order = TaskGraph<List>::kSortedOrder;

    EXPECT_TRUE((ComesBeforeInSort<List, TaskA, TaskB>(order)));
    EXPECT_TRUE((ComesBeforeInSort<List, TaskB, TaskC>(order)));
    EXPECT_TRUE((ComesBeforeInSort<List, TaskA, TaskC>(order)));
}

// Registration order shouldn't matter — B and A registered as B, A
TEST(TaskGraph, LinearChain_ReverseRegistration)
{
    using List = TaskList<TaskB, TaskA>;
    constexpr auto order = TaskGraph<List>::kSortedOrder;

    EXPECT_TRUE((ComesBeforeInSort<List, TaskA, TaskB>(order)));
}

// Diamond: F before D, F before E, D and E before G
TEST(TaskGraph, DiamondDependency)
{
    using List = TaskList<TaskG, TaskD, TaskE, TaskF>;
    constexpr auto order = TaskGraph<List>::kSortedOrder;

    EXPECT_TRUE((ComesBeforeInSort<List, TaskF, TaskD>(order)));
    EXPECT_TRUE((ComesBeforeInSort<List, TaskF, TaskE>(order)));
    EXPECT_TRUE((ComesBeforeInSort<List, TaskD, TaskG>(order)));
    EXPECT_TRUE((ComesBeforeInSort<List, TaskE, TaskG>(order)));
}

// All tasks present — sorted array has no duplicates and covers all indices
TEST(TaskGraph, SortedOrderIsComplete)
{
    using List = TaskList<TaskA, TaskB, TaskC>;
    constexpr auto order = TaskGraph<List>::kSortedOrder;

    std::array<bool, 3> seen = { false, false, false };
    for (int idx : order)
    {
        ASSERT_GE(idx, 0);
        ASSERT_LT(idx, 3);
        EXPECT_FALSE(seen[idx]) << "Duplicate index " << idx << " in sorted order";
        seen[idx] = true;
    }
}

// ---------------------------------------------------------------------------
// Cycle detection — these must produce a compile error when uncommented.
// Verify manually by uncommenting and checking the static_assert fires.
// ---------------------------------------------------------------------------

// Cycle: X depends on Y, Y depends on X
// struct CycleX;
// struct CycleY : public Task<TaskPhase::Gameplay, RunAfter<CycleX>> { void Execute(float) override {} };
// struct CycleX : public Task<TaskPhase::Gameplay, RunAfter<CycleY>> { void Execute(float) override {} };
// TEST(TaskGraph, CycleDetected)
// {
//     using List = TaskList<CycleX, CycleY>;
//     constexpr auto order = TaskGraph<List>::kSortedOrder; // should static_assert
// }