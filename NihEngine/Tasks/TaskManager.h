#pragma once

#include <thread>
#include "Core/Types/Vector.h"
#include "Singleton/Singleton.h"

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
	//std::vector<Task*> m_Tasks;
	Vector<Task*> m_Tasks;

	bool m_IsRunning;
};

