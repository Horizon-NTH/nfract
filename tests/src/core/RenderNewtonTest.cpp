#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <ranges>

#include "app/ArgumentsParser.hpp"
#include "core/Image.hpp"
#include "core/RenderNewton.hpp"
#include "core/RootsTable.hpp"

using nfract::Arguments;
using nfract::ColorMode;
using nfract::Image;
using nfract::RootsTable;

namespace
{
    [[nodiscard]] Arguments make_default_args()
    {
        Arguments args;
        args.degree = 3;
        args.width = 8;
        args.height = 6;
        args.maxIter = 20;
        args.xmin = -1.5f;
        args.xmax = 1.5f;
        args.ymin = -1.0f;
        args.ymax = 1.0f;
        args.tolerance = 1e-4f;
        args.outputPath.clear();
        args.colorMode = ColorMode::CLASSIC;
        return args;
    }
}

TEST(RenderNewtonTest, CpuRendererFillsImageForAllColorModes)
{
    const Arguments base_args = make_default_args();
    const RootsTable roots{base_args.degree};
    constexpr std::array modes{
        ColorMode::CLASSIC, ColorMode::JEWELRY, ColorMode::NEON
    };

    for (const ColorMode mode : modes)
    {
        Arguments args = base_args;
        args.colorMode = mode;
        Image img{args.width, args.height};

        // Pre-fill with sentinel values to ensure the renderer writes all pixels.
        std::fill(img.pixels().begin(), img.pixels().end(), std::uint8_t{17});

        nfract::render_newton_cpu(args, roots, img);

        const auto pixels = img.pixels();
        ASSERT_FALSE(pixels.empty());
        for (std::size_t i = 0; i < pixels.size(); i += 4)
        {
            const bool rgbUnchanged = (pixels[i + 0] == 17) && (pixels[i + 1] == 17) && (pixels[i + 2] == 17);
            EXPECT_FALSE(rgbUnchanged) << "Pixel RGB channels not updated at index " << i;
            EXPECT_EQ(pixels[i + 3], 255) << "Alpha channel expected to remain opaque";
        }
    }
}

TEST(RenderNewtonTest, CpuRendererHandlesInvalidInputGracefully)
{
    Arguments args = make_default_args();
    args.width = 0;
    const RootsTable roots{args.degree};
    Image img{4, 4};

    const auto before = img.pixels();
    nfract::render_newton_cpu(args, roots, img);
    EXPECT_TRUE(std::ranges::equal(before, img.pixels()));
}

#ifndef RUN_ON_CPU
TEST(RenderNewtonTest, IspcRendererMatchesCpuOutput)
{
    const Arguments base_args = make_default_args();
    const RootsTable roots{base_args.degree};
    constexpr std::array modes{
        ColorMode::CLASSIC, ColorMode::JEWELRY, ColorMode::NEON
    };

    constexpr int kChannelTolerance = 1;

    for (const ColorMode mode : modes)
    {
        Arguments args = base_args;
        args.colorMode = mode;

        Image cpu_img{args.width, args.height};
        Image ispc_img{args.width, args.height};

        std::fill(cpu_img.pixels().begin(), cpu_img.pixels().end(), std::uint8_t{0});
        std::fill(ispc_img.pixels().begin(), ispc_img.pixels().end(), std::uint8_t{0});

        nfract::render_newton_cpu(args, roots, cpu_img);
        nfract::render_newton_ispc(args, roots, ispc_img);

        const auto cpu_pixels = cpu_img.pixels();
        const auto ispc_pixels = ispc_img.pixels();
        ASSERT_EQ(cpu_pixels.size(), ispc_pixels.size());

        bool mismatch_found = false;
        for (std::size_t i = 0; i < cpu_pixels.size(); ++i)
        {
            const int diff = std::abs(static_cast<int>(cpu_pixels[i]) - static_cast<int>(ispc_pixels[i]));
            if (diff > kChannelTolerance)
            {
                const std::size_t pixel_index = i / 4;
                const int channel = static_cast<int>(i % 4);
                ADD_FAILURE() << "Mode " << static_cast<int>(mode)
                    << " pixel " << pixel_index
                    << " channel " << channel
                    << " cpu=" << static_cast<int>(cpu_pixels[i])
                    << " ispc=" << static_cast<int>(ispc_pixels[i])
                    << " diff=" << diff
                    << " tolerance=" << kChannelTolerance;
                mismatch_found = true;
                break;
            }
        }

        EXPECT_FALSE(mismatch_found)
            << "CPU and ISPC renderers should produce identical RGBA output for the same parameters";
    }
}
#endif
