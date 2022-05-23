#include <gtest/gtest.h>

#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{ 
  tracker_manager manager;
  std::cout << "start ebpf...\n";

  manager.start_container_tracker();

  std::this_thread::sleep_for(10s);
  return 0;
}


/*
TEST(TmpAddTest, CheckValues)
{
  ASSERT_EQ(tmp::add(1, 2), 3);
  EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
*/