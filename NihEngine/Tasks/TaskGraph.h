#pragma once

#include <array>
#include <Tasks/TaskTraits.h>

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
			((indices[i++] = IndexOf<Deps, TList>::kValue), ...);
			return indices;
		}();
};

template <typename TTask, typename TList>
using TaskDependencyIndices = GetDependencyIndices<TList, typename TTask::Deps>;

template <typename TList>
struct InDegreeBuilder {};

template <typename... Tasks>
struct InDegreeBuilder<TaskList<Tasks...>>
{
	static constexpr std::array<int, sizeof...(Tasks)> Build()
	{
		std::array<int, sizeof...(Tasks)> inDegrees = {};

		auto processTask = [&]<typename TTask>()
		{
			constexpr int taskIndex = IndexOf<TTask, TaskList<Tasks...>>::kValue;
			constexpr auto deps = TaskDependencyIndices<TTask, TaskList<Tasks...>>::kIndices;

			for (int i = 0; i < static_cast<int>(deps.size()); ++i)
			{
				inDegrees[taskIndex]++;
			}
		};

		((processTask.template operator()<Tasks>()), ...);

		return inDegrees;
	}
};

template <typename TList>
struct BuildAdjacencyList {};

template <typename... Tasks>
struct BuildAdjacencyList<TaskList<Tasks...>>
{
	static constexpr int N = sizeof...(Tasks);
	static constexpr std::array<std::array<bool, N>, N> Build()
	{
		std::array<std::array<bool, N>, N> adjacencyList = {};

		auto processTask = [&]<typename TTask>()
		{
			constexpr int taskIndex = IndexOf<TTask, TaskList<Tasks...>>::kValue;
			constexpr auto deps = TaskDependencyIndices<TTask, TaskList<Tasks...>>::kIndices;

			for (int i = 0; i < static_cast<int>(deps.size()); ++i)
			{
				int depIndex = deps[i];
				adjacencyList[depIndex][taskIndex] = true;
			}
		};

		((processTask.template operator()<Tasks>()), ...);

		return adjacencyList;
	}
};

template <typename TList>
struct TaskGraph
{
public:
	static constexpr std::array<int, TList::kCount> kSortedOrder = []()
		{
			auto inDegrees = InDegreeBuilder<TList>::Build();
			auto adjacencyList = BuildAdjacencyList<TList>::Build();

			std::array<int, TList::kCount> queue = {};
			std::array<int, TList::kCount> result = {};

			int head = 0;
			int tail = 0;
			int count = 0;

			for (int i = 0; i < TList::kCount; ++i)
			{
				if (inDegrees[i] == 0)
				{
					queue[tail++] = i;
				}
			}

			while (head != tail)
			{
				int current = queue[head++];
				result[count++] = current;

				for (int i = 0; i < TList::kCount; ++i)
				{
					if (adjacencyList[current][i])
					{
						inDegrees[i]--;
						if (inDegrees[i] == 0)
						{
							queue[tail++] = i;
						}
					}
				}
			}

			if (count != TList::kCount)
			{
				return std::array<int, TList::kCount>{}; // Cycle detected, return empty order
			}

			return result;
		}();
};
