#pragma once

#include "Core/Containers/Vector.h"

class Task;

class TaskManager
{
public:
	TaskManager();
	~TaskManager();

	TaskManager(const TaskManager&) = delete;
	TaskManager& operator=(const TaskManager&) = delete;

	void Init();
	void BeginSimulation();
	void BeginFrame();
	void Update(float deltaTime);
	void EndFrame();
	void EndSimulation();
	void AddTask(Task* task);
private:

	// every task should be in a separate thread
	Vector<Task*> m_Tasks;

	bool m_IsRunning;
};

