#pragma once

#include "Tasks/TaskPhases.h"
#include "Tasks/TaskTraits.h"

class ITask
{
public:
	virtual ~ITask() = default;
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
};

template <TaskPhase Phase, typename Dependencies = RunAfter<>>
class Task : public ITask
{
public:
	static constexpr TaskPhase m_Phase = Phase;
	using Deps = Dependencies;

	Task() = default;
	~Task() = default;

	Task(Task&&) = default;
	Task& operator=(Task&&) = default;

	Task& operator=(const Task&) = delete;
	Task(const Task&) = delete;
};
