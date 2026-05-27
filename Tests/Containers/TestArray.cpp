#include <gtest/gtest.h>
#include "Core/Containers/Array.h"

namespace Containers
{
	TEST(InplaceArray, Construct)
	{
		// These are here to make sure we will always be able to compile
		{
			InplaceArray<int, 4> array{ 0,1,2,3 };
		}
		{
			InplaceArray array{ 0,1,2,3 };
		}
	}
	TEST(InplaceArray, GetSize)
	{
		InplaceArray<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.GetSize() == 4);
	}

	TEST(InplaceArray, At)
	{
		InplaceArray<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.At(2) == 2);
	}

	TEST(InplaceArray, Front)
	{
		InplaceArray<int, 4> array{ 3,2,1,0 };
		EXPECT_TRUE(array.Front() == 3);
	}

	TEST(InplaceArray, Back)
	{
		InplaceArray<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.Back() == 3);
	}

	TEST(InplaceArray, AtOperator)
	{
		InplaceArray<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array[2] == 2);
	}
}