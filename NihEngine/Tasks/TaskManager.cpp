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

void TaskManager::Update(float deltaTime)
{
	for (Task* task : m_Tasks)
	{
		task->Update(deltaTime);
	}
}

void TaskManager::AddTask(Task* task)
{
	m_Tasks.push_back(task);
}