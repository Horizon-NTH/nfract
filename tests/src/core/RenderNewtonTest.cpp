#include <gtest/gtest.h>

#include <algorithm>
#include <ranges>

#include "app/ArgumentsParser.hpp"
#include "core/Image.hpp"
#include "core/RenderNewton.hpp"
#include "core/RootsTable.hpp"

using nfract::Arguments;
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
        return args;
    }
}

TEST(RenderNewtonTest, CpuRendererFillsImage)
{
    const Arguments args = make_default_args();
    const RootsTable roots{args.degree};
    Image img{args.width, args.height};

    // Pre-fill with sentinel values to ensure the renderer writes all pixels.
    std::fill(img.pixels().begin(), img.pixels().end(), std::uint8_t{17});

    nfract::render_newton_cpu(args, roots, img);

    const auto pixels = img.pixels();
    ASSERT_FALSE(pixels.empty());
    for (std::size_t i = 0; i < pixels.size(); i += 4)
    {
        EXPECT_NE(pixels[i + 0], 17) << "Pixel R channel not updated at index " << i;
        EXPECT_NE(pixels[i + 1], 17) << "Pixel G channel not updated at index " << i;
        EXPECT_NE(pixels[i + 2], 17) << "Pixel B channel not updated at index " << i;
        EXPECT_EQ(pixels[i + 3], 255) << "Alpha channel expected to remain opaque";
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
    const Arguments args = make_default_args();
    const RootsTable roots{args.degree};

    Image cpu_img{args.width, args.height};
    Image ispc_img{args.width, args.height};

    std::fill(cpu_img.pixels().begin(), cpu_img.pixels().end(), std::uint8_t{0});
    std::fill(ispc_img.pixels().begin(), ispc_img.pixels().end(), std::uint8_t{0});

    nfract::render_newton_cpu(args, roots, cpu_img);
    nfract::render_newton_ispc(args, roots, ispc_img);

    EXPECT_TRUE(std::ranges::equal(cpu_img.pixels(), ispc_img.pixels()))
        << "CPU and ISPC renderers should produce identical RGBA output for the same parameters";
}
#endif
