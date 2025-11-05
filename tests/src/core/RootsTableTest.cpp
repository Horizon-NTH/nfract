#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "core/RootsTable.hpp"

using nfract::RootsTable;

TEST(RootsTableTest, ConstructorRejectsNonPositiveSize)
{
    EXPECT_THROW(RootsTable(0), std::invalid_argument);
    EXPECT_THROW(RootsTable(-5), std::invalid_argument);
}

TEST(RootsTableTest, RootOutOfRangeThrows)
{
    const RootsTable table{4};

    EXPECT_THROW(static_cast<void>(table.root(-1)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(table.root(table.size())), std::out_of_range);
}

TEST(RootsTableTest, RootComputesExpectedValue)
{
    const RootsTable table{4};

    const auto root = table.root(1);
    EXPECT_NEAR(root.real(), 0.0f, 1e-5f);
    EXPECT_NEAR(root.imag(), 1.0f, 1e-5f);
}

TEST(RootsTableTest, RealAndImagSpansExposeRoots)
{
    const RootsTable table{3};

    EXPECT_EQ(table.size(), 3);
    EXPECT_FALSE(table.empty());

    const auto real = table.re();
    const auto imag = table.im();

    ASSERT_EQ(real.size(), 3);
    ASSERT_EQ(imag.size(), 3);

    EXPECT_NEAR(real[0], 1.0f, 1e-5f);
    EXPECT_NEAR(imag[0], 0.0f, 1e-5f);

    constexpr float half = -0.5f;
    const float sqrt3_over_2 = static_cast<float>(std::sqrt(3.0)) / 2.0f;
    EXPECT_NEAR(real[1], half, 1e-5f);
    EXPECT_NEAR(imag[1], sqrt3_over_2, 1e-5f);

    EXPECT_NEAR(real[2], half, 1e-5f);
    EXPECT_NEAR(imag[2], -sqrt3_over_2, 1e-5f);
}
