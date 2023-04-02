#include <gtest/gtest.h>
#include "Core/Containers/Array.h"

namespace Containers
{
	TEST(Array, GetSize)
	{
		Array<int, 4> array{ 0,1,2,3 };
		EXPECT_TRUE(array.size() == 4);
	}
}