#pragma once

class Engine
{
public:
    Engine();
    ~Engine();

    Engine(Engine&&) = default;
    Engine& operator= (Engine&&) = default;

    Engine(const Engine&) = delete;
    Engine& operator= (const Engine&) = delete;
};
