#include "StarGuardedPtr.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(GuardedPtr, All) {
  GuardedPtr<int> ptr1;
  {
    GuardedPtr<int> ptr2;

    GuardedHolder<int> holder(new int (5), true);

    ptr1 = ptr2 = holder.guardedPtr();

    EXPECT_TRUE((bool)ptr1);
    EXPECT_TRUE((bool)ptr2);

    EXPECT_EQ(*ptr1, 5);
    EXPECT_EQ(*ptr2, 5);
  }

  EXPECT_FALSE((bool)ptr1);
}
