#pragma once

#include <vector>
#include <thread>
#include "Singleton/Singleton.h"
#include <deque>

class Task;

class TaskManager
{
public:
	TaskManager();
	~TaskManager();

	TaskManager(const TaskManager&) = delete;
	TaskManager& operator=(const TaskManager&) = delete;

	void Init();
	void Update(float deltaTime);
	void AddTask(Task* task);
private:

	// every task should be in a separate thread
	std::vector<Task*> m_Tasks;

	bool m_IsRunning;
};

