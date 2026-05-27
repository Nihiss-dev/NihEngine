#include "TaskManager.h"

#include "Window/Window.h"
#include "Tasks/Task.h"

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
	//OutputDebugStringA("BeginSimulation\n");
	NIH_LOG_INFO(LogCategory::TaskManager, "BeginSimulation");
}

void TaskManager::BeginFrame()
{
	//OutputDebugStringA("BeginFrame\n");
	NIH_LOG_INFO(LogCategory::TaskManager, "BeginFrame");
}

void TaskManager::Update(float deltaTime)
{
	std::string test = std::string("Update: ") + std::to_string(deltaTime);
	//OutputDebugStringA(test.c_str());
	NIH_LOG_INFO(LogCategory::TaskManager, test.c_str());
	for (Task* task : m_Tasks)
	{
		task->Update(deltaTime);
	}
}

void TaskManager::EndFrame()
{
	//OutputDebugStringA("EndFrame\n");
	NIH_LOG_INFO(LogCategory::TaskManager, "EndFrame");
}

void TaskManager::EndSimulation()
{
	//OutputDebugStringA("EndSimulation\n");
	NIH_LOG_INFO(LogCategory::TaskManager, "EndSimulation");
}

void TaskManager::AddTask(Task* task)
{
	m_Tasks.push_back(task);
}