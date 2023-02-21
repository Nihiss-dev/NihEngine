#include "TaskManager.h"

#include "Window/Window.h"

TaskManager::TaskManager()
	: m_IsRunning(false)
{

}

TaskManager::~TaskManager()
{

}

void TaskManager::Init()
{
	for (Task* task : m_Tasks)
	{
		task->Init();
	}
}

void TaskManager::Run()
{
	m_IsRunning = true;

	while (m_IsRunning)
	{
		for (Task* task : m_Tasks)
		{
			task->Run();
		}
	}
}

void TaskManager::Stop()
{
	m_IsRunning = false;
}

void TaskManager::AddTask(Task* task)
{
	m_Tasks.push_back(task);
}