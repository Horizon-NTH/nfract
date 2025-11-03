#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <utility>

#include "core/Image.hpp"

using nfract::Image;

namespace
{
    [[nodiscard]] std::filesystem::path make_temp_png(const std::string_view prefix)
    {
        thread_local std::mt19937_64 rng{std::random_device{}()};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto random_value = rng();

        std::string filename(prefix);
        filename.append("-");
        filename.append(std::to_string(timestamp));
        filename.append("-");
        filename.append(std::to_string(random_value));
        filename.append(".png");

        return std::filesystem::temp_directory_path() / filename;
    }

    class TempFileGuard
    {
    public:
        explicit TempFileGuard(std::filesystem::path path) noexcept :
            m_path(std::move(path))
        {
        }

        TempFileGuard(const TempFileGuard&) = delete;
        TempFileGuard& operator=(const TempFileGuard&) = delete;

        TempFileGuard(TempFileGuard&& other) noexcept :
            m_path(std::exchange(other.m_path, {}))
        {
        }

        TempFileGuard& operator=(TempFileGuard&& other) noexcept
        {
            if (this != &other)
            {
                remove();
                m_path = std::exchange(other.m_path, {});
            }
            return *this;
        }

        ~TempFileGuard()
        {
            remove();
        }

    private:
        void remove() const noexcept
        {
            if (!m_path.empty())
            {
                std::error_code ec;
                std::filesystem::remove(m_path, ec);
            }
        }

        std::filesystem::path m_path;
    };
}

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
    EXPECT_TRUE(std::all_of(image.pixels().begin(), image.pixels().end(), [](const Image::Pixel p) { return p == 0; }));
}

TEST(ImageTest, RowProvidesWritableSpan)
{
    Image image{2, 3};

    auto row = image.row(1);
    ASSERT_EQ(row.size(), static_cast<std::size_t>(image.width() * 4));
    std::iota(row.begin(), row.end(), Image::Pixel{1});

    const auto reread = std::as_const(image).row(1);
    EXPECT_TRUE(std::ranges::equal(row, reread));
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

TEST(ImageTest, SavePngFailsForEmptyImage)
{
    const Image image;
    const auto path = make_temp_png("nfract-empty");
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
        std::iota(row.begin(), row.end(), static_cast<Image::Pixel>(y * 10));
    }

    const auto path = make_temp_png("nfract-image");
    TempFileGuard guard{path};

    ASSERT_TRUE(image.save_png(path));
    ASSERT_TRUE(std::filesystem::exists(path));
    EXPECT_GT(std::filesystem::file_size(path), 0);
}
