#pragma once

#include <vector>
#include <thread>
#include "Singleton/Singleton.h"
#include <deque>

class Task;

class TaskManager : public Singleton<TaskManager>
{
public:
	TaskManager();
	~TaskManager();

	void Init();
	void Run();
	void Stop();

	void AddTask(Task* task);
private:

	// every task should be in a separate thread
	std::vector<Task*> m_Tasks;

	bool m_IsRunning;
};

