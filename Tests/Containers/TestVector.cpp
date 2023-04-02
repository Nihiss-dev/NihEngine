#include <gtest/gtest.h>
#include "Core/Containers/Vector.h"

namespace Containers
{
	TEST(Vector, GetSize)
	{
		Vector<int> vector{ 0,1,2,3 };
		EXPECT_TRUE(vector.size() == 4);
	}
	TEST(Vector, Add)
	{
		Vector<int> vector{ 0,1,2,3 };
		vector.push_back(4);
		EXPECT_TRUE(vector.size() == 5);
	}
}