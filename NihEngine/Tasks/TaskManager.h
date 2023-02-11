#pragma once

#include <vector>
#include <thread>
#include "Singleton/Singleton.h"

class Task;

class TaskManager : public Singleton<TaskManager>
{
public:
	TaskManager();
	~TaskManager();

	void Init();
	void Run();
	void Stop();

	void AddTask(Task&& task);
	void AddTask(Task* task);
private:

	// every task should be in a separate thread
	std::vector<Task*> m_Tasks;
	std::vector<std::thread> m_Threads;

	bool m_IsRunning;
};

