#pragma once

#include "Core/Containers/Vector.h"
#include "Tasks/Task.h"

class TaskManager
{
public:
	TaskManager();
	~TaskManager();

	TaskManager(const TaskManager&) = delete;
	TaskManager& operator=(const TaskManager&) = delete;

    template<typename TTaskList>
    void Build();

	void Init();
	void BeginSimulation();
	void BeginFrame();
	void Update(float deltaTime);
	void EndFrame();
	void EndSimulation();
private:

	// every task should be in a separate thread
	Array<UniquePtr<ITask>> m_Phases[static_cast<int>(TaskPhase::Count)];

	bool m_IsRunning;
};

/* This won't work in the end because build will have to take a TaskList as a template parameter
* using MyTasks = TaskList<GameplayTask, PhysicsTask, AnimationTask>;
* m_TaskManager->Build<MyTasks>();
* We'll have to write something like this
* And this won't scale at all as we don't want to write a TaskList with 1000 tasks manually
*/
template<typename TTaskList>
void TaskManager::Build()
{
    // Compute the sorted order at compile time
    constexpr auto sortedOrder = TaskGraph<TTaskList>::kSortedOrder;

    // Instantiate tasks in sorted order, bucketed by phase
    [&] <int... Is>(std::integer_sequence<int, Is...>)
    {
        ([&]()
            {
                // Get the task type at sorted position Is
                using TTask = typename TTaskList::template Get<sortedOrder[Is]>;

                // Create the task and place it in the correct phase bucket
                constexpr int phaseIndex = static_cast<int>(TTask::kPhase);
                m_Phases[phaseIndex].push_back(std::make_unique<TTask>());
            }(), ...);
    }(std::make_integer_sequence<int, TTaskList::kCount>{});
}