#pragma once
class Task
{
public:
	Task();
	~Task();

	Task(Task&&) = default;
	Task& operator=(Task&&) = default;

	Task& operator=(const Task&) = delete;
	Task(const Task&) = delete;

	virtual void Init();
	virtual void Run();
};

