#include <gtest/gtest.h>
#include "Core/Containers/Array.h"

namespace Containers
{
	TEST(Array, Construct)
	{
		// These are here to make sure we will always be able to compile
		{
			Array<int, 4> array{ 0,1,2,3 };
		}
		{
			Array array{ 0,1,2,3 };
		}
	}
	TEST(Array, GetSize)
	{
		Array<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.GetSize() == 4);
	}

	TEST(Array, At)
	{
		Array<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.At(2) == 2);
	}

	TEST(Array, Front)
	{
		Array<int, 4> array{ 3,2,1,0 };
		EXPECT_TRUE(array.Front() == 3);
	}

	TEST(Array, Back)
	{
		Array<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.Back() == 3);
	}

	TEST(Array, AtOperator)
	{
		Array<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array[2] == 2);
	}
}