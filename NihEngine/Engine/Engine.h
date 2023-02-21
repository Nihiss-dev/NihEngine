#pragma once

#include "Tasks/Task.h"

class Engine : public Task
{
public:
    Engine();
    ~Engine();

    Engine(Engine&&) = default;
    Engine& operator= (Engine&&) = default;

    Engine(const Engine&) = delete;
    Engine& operator= (const Engine&) = delete;

    void Init() override;
    void Run() override;
};
