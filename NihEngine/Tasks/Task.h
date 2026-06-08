#pragma once

#include "Tasks/TaskPhases.h"
#include "Tasks/TaskTraits.h"

template <TaskPhase Phase, typename Dependencies = RunAfter<>>
class Task
{
public:
	static constexpr TaskPhase m_Phase = Phase;
	using Dependencies = Dependencies;

	Task() = default;
	~Task() = default;

	Task(Task&&) = default;
	Task& operator=(Task&&) = default;

	Task& operator=(const Task&) = delete;
	Task(const Task&) = delete;

	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
};
