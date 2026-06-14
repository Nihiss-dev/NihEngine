#pragma once

template <typename... Deps> 
struct RunAfter
{
	static constexpr int kNumDeps = sizeof...(Deps);
};

template <typename... Ts>
struct Reads {};

template <typename... Ts>
struct Writes {};

template <typename ...Ts>
struct TaskList
{
	static constexpr int kCount = sizeof...(Ts);
};
