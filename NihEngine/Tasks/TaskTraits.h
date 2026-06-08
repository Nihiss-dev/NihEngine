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
	static constexpr int kSize = sizeof...(Ts);
};

template <typename T, typename List, int Index = 0>
struct IndexOf {};

template <typename T, typename... Rest, int Index>
struct IndexOf<T, TaskList<T, Rest...>, Index>
{
	static constexpr int kValue = Index;
};

template <typename T, typename Head, typename... Rest, int Index>
struct IndexOf<T, TaskList<Head, Rest...>, Index>
{
	static constexpr int kValue = IndexOf<T, TaskList<Rest...>, Index + 1>::kValue;
};

template <typename T, int Index>
struct IndexOf<T, TaskList<>, Index>
{
	static_assert(sizeof(T) == 0, "Type T not found in TaskList");
};

template <typename TList, typename TDependencies>
struct GetDependencyIndices {};

template <typename TList, typename... Deps>
struct GetDependencyIndices<TList, RunAfter<Deps...>>
{
	static constexpr std::array<int, sizeof...(Deps)> kIndices = []()
		{
			std::array<int, sizeof...(Deps)> indices = {};
			int i = 0;
			(indices[i++] = IndexOf<Deps, TList>::kValue, ...);
			return indices;
		}
};

template <typename TTask, typename TList>
using TaskDependencyIndices = GetDependencyIndices<TList, typename TTask::Dependencies>;