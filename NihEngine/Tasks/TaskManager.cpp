#include "TaskManager.h"

#include "Window/Window.h"

#include <iostream>

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

void TaskManager::BeginSimulation()
{
	OutputDebugStringA("BeginSimulation\n");
}

void TaskManager::BeginFrame()
{
	OutputDebugStringA("BeginFrame\n");
}

void TaskManager::Update(float deltaTime)
{
	OutputDebugStringA("Update\n");
	for (Task* task : m_Tasks)
	{
		task->Update(deltaTime);
	}
}

void TaskManager::EndFrame()
{
	OutputDebugStringA("EndFrame\n");
}

void TaskManager::EndSimulation()
{
	OutputDebugStringA("EndSimulation\n");
}

void TaskManager::AddTask(Task* task)
{
	m_Tasks.push_back(task);
}