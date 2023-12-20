#include "gtest/gtest.h"
#include "../clockesp32/linetracker.h"

TEST(LineTracker, integer_line_left_no_overlap) 
{
    LineTracker lt(1, 2, 0, 4, 2, 6, false);
    scanpoint p;

    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1, 2, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(0.5, 3, 127, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(0, 4, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1, 5, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(2, 6, 0, 0));
    EXPECT_EQ(lt.next(&p), false);
}

TEST(LineTracker, integer_line_left_adjacenty_p1p2) 
{
    LineTracker lt(1, 1, 0, 2, 2, 3, false);
    scanpoint p;

    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1, 1, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(0, 2, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(2, 3, 0, 0));
    EXPECT_EQ(lt.next(&p), false);
}

TEST(LineTracker, integer_line_left_adjacenty_p2p3)
{
    LineTracker lt(2, 1, 1, 3, 2, 4, false);
    scanpoint p;

    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(2, 1, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1.5, 2, 127, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1, 3, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(2, 4, 0, 0));
    EXPECT_EQ(lt.next(&p), false);
}

TEST(LineTracker, integer_line_left_adjacenty_p1p2p3)
{
    LineTracker lt(1, 1, 2, 2, 3, 3, false);
    scanpoint p;

    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1, 1, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(2, 2, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(3, 3, 0, 0));
    EXPECT_EQ(lt.next(&p), false);
}

TEST(LineTracker, integer_line_left_samey_p1p2)
{
    LineTracker lt(1, 1, 2, 1, 3, 3, false);
    scanpoint p;

    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(1, 1, 255, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(2.5, 2, 127, 255));
    EXPECT_EQ(lt.next(&p), true);
    EXPECT_EQ(p, scanpoint(3, 3, 0, 0));
    EXPECT_EQ(lt.next(&p), false);
}
