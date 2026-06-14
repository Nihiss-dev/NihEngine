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
	for (ITask* task : m_Tasks)
	{
		task->Init();
	}
}

void TaskManager::BeginSimulation()
{
	NIH_LOG_INFO(LogCategory::TaskManager, "BeginSimulation");
}

void TaskManager::BeginFrame()
{
	NIH_LOG_INFO(LogCategory::TaskManager, "BeginFrame");
}

void TaskManager::Update(float deltaTime)
{
	std::string test = std::string("Update: ") + std::to_string(deltaTime);
	NIH_LOG_INFO(LogCategory::TaskManager, test.c_str());
	for (ITask* task : m_Tasks)
	{
		task->Update(deltaTime);
	}
}

void TaskManager::EndFrame()
{
	NIH_LOG_INFO(LogCategory::TaskManager, "EndFrame");
}

void TaskManager::EndSimulation()
{
	NIH_LOG_INFO(LogCategory::TaskManager, "EndSimulation");
}
