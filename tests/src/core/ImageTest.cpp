#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "core/Image.hpp"
#include "../support/TestUtils.hpp"

using nfract::Image;
using nfract::test::TempFileGuard;

namespace test_utils = nfract::test;

TEST(ImageTest, DefaultConstructedImageIsEmpty)
{
    const Image image;

    EXPECT_EQ(image.width(), 0);
    EXPECT_EQ(image.height(), 0);
    EXPECT_TRUE(image.empty());
    EXPECT_TRUE(image.pixels().empty());
}

TEST(ImageTest, ConstructedImageAllocatesPixelBuffer)
{
    constexpr int width = 3;
    constexpr int height = 2;

    const Image image{width, height};

    EXPECT_EQ(image.width(), width);
    EXPECT_EQ(image.height(), height);
    EXPECT_FALSE(image.empty());
    EXPECT_EQ(image.pixels().size(), static_cast<std::size_t>(width * height * 4));
    EXPECT_TRUE(std::all_of(image.pixels().begin(), image.pixels().end(), [](const Image::pixel_type p) { return p == 0; }));
}

TEST(ImageTest, RowProvidesWritableSpan)
{
    Image image{2, 3};

    auto row = image.row(1);
    ASSERT_EQ(row.size(), static_cast<std::size_t>(image.width() * 4));
    std::iota(row.begin(), row.end(), Image::pixel_type{1});

    const auto reread = std::as_const(image).row(1);
    EXPECT_TRUE(std::ranges::equal(row, reread));
}

TEST(ImageTest, ConstructingImageWithNegativeDimensionsThrows)
{
    EXPECT_THROW(Image(-1, 0), std::invalid_argument);
    EXPECT_THROW(Image(0, -1), std::invalid_argument);
    EXPECT_THROW(Image(-4, -4), std::invalid_argument);
}

TEST(ImageTest, RowOutOfRangeThrows)
{
    Image image{3, 3};

    EXPECT_THROW(static_cast<void>(image.row(-1)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(image.row(image.height())), std::out_of_range);
    EXPECT_THROW(static_cast<void>(std::as_const(image).row(-1)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(std::as_const(image).row(image.height())), std::out_of_range);
}

TEST(ImageTest, PixelAccessReturnsChannelsForCoordinate)
{
    Image image{3, 2};

    auto* pixel = image.pixel(1, 0);
    pixel[0] = 10;
    pixel[1] = 20;
    pixel[2] = 30;
    pixel[3] = 255;

    const auto row = image.row(0);
    constexpr std::size_t base_index = 1 * 4;
    EXPECT_EQ(row[base_index + 0], 10);
    EXPECT_EQ(row[base_index + 1], 20);
    EXPECT_EQ(row[base_index + 2], 30);
    EXPECT_EQ(row[base_index + 3], 255);

    const Image& const_image = image;
    const auto* const_pixel = const_image.pixel(1, 0);
    EXPECT_EQ(const_pixel[0], 10);
}

TEST(ImageTest, PixelOutOfRangeThrows)
{
    Image image{4, 4};

    EXPECT_THROW(static_cast<void>(image.pixel(-1, 0)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(image.pixel(0, -1)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(image.pixel(image.width(), 0)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(image.pixel(0, image.height())), std::out_of_range);

    const Image& const_image = image;
    EXPECT_THROW(static_cast<void>(const_image.pixel(-1, 0)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(const_image.pixel(0, -1)), std::out_of_range);
}

TEST(ImageTest, SavePngFailsForEmptyImage)
{
    const Image image;
    const auto path = test_utils::make_unique_path("nfract-empty", ".png");
    TempFileGuard guard{path};

    EXPECT_FALSE(image.save_png(path));
    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(ImageTest, SavePngWritesFile)
{
    Image image{2, 2};

    for (int y = 0; y < image.height(); ++y)
    {
        auto row = image.row(y);
        std::iota(row.begin(), row.end(), static_cast<Image::pixel_type>(y * 10));
    }

    const auto path = test_utils::make_unique_path("nfract-image", ".png");
    TempFileGuard guard{path};

    ASSERT_TRUE(image.save_png(path));
    ASSERT_TRUE(std::filesystem::exists(path));
    EXPECT_GT(std::filesystem::file_size(path), 0);
}
