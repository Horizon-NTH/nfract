#include <gtest/gtest.h>

#include <stdexcept>

#include "app/ArgumentsParser.hpp"
#include "../support/TestUtils.hpp"

using nfract::Arguments;
using nfract::ArgumentsParser;
using nfract::test::ArgvBuilder;

TEST(ArgumentsParserTest, UsesDefaultsWhenNoOverrides)
{
    const ArgvBuilder argv{"nfract"};

    const Arguments args = ArgumentsParser::parse(argv.span());

    EXPECT_EQ(args.degree, 5);
    EXPECT_EQ(args.width, 1920);
    EXPECT_EQ(args.height, 1080);
    EXPECT_EQ(args.maxIter, 50);
    EXPECT_FLOAT_EQ(args.xmin, -2.0f);
    EXPECT_FLOAT_EQ(args.xmax, 2.0f);
    EXPECT_FLOAT_EQ(args.ymin, -2.0f);
    EXPECT_FLOAT_EQ(args.ymax, 2.0f);
    EXPECT_FLOAT_EQ(args.tolerance, 1e-6f);
    EXPECT_EQ(args.outputPath, "nfract.png");
}

TEST(ArgumentsParserTest, ParsesAllSupportedOptions)
{
    const ArgvBuilder argv{
        "nfract",
        "--degree", "7",
        "--width", "800",
        "--height", "600",
        "--max-iter", "123",
        "--xmin", "-1.5",
        "--xmax", "1.5",
        "--ymin", "-1.0",
        "--ymax", "1.0",
        "--tol", "1e-5",
        "--out", "output.png"
    };

    const Arguments args = ArgumentsParser::parse(argv.span());

    EXPECT_EQ(args.degree, 7);
    EXPECT_EQ(args.width, 800);
    EXPECT_EQ(args.height, 600);
    EXPECT_EQ(args.maxIter, 123);
    EXPECT_FLOAT_EQ(args.xmin, -1.5f);
    EXPECT_FLOAT_EQ(args.xmax, 1.5f);
    EXPECT_FLOAT_EQ(args.ymin, -1.0f);
    EXPECT_FLOAT_EQ(args.ymax, 1.0f);
    EXPECT_FLOAT_EQ(args.tolerance, 1e-5f);
    EXPECT_EQ(args.outputPath, "output.png");
}

TEST(ArgumentsParserTest, AllowsPartialOverrides)
{
    const ArgvBuilder argv{
        "nfract",
        "--degree", "3",
        "--width", "1024",
        "--out", "custom.png"
    };

    const Arguments args = ArgumentsParser::parse(argv.span());

    EXPECT_EQ(args.degree, 3);
    EXPECT_EQ(args.width, 1024);
    EXPECT_EQ(args.outputPath, "custom.png");

    EXPECT_EQ(args.height, 1080);
    EXPECT_EQ(args.maxIter, 50);
    EXPECT_FLOAT_EQ(args.xmin, -2.0f);
    EXPECT_FLOAT_EQ(args.xmax, 2.0f);
    EXPECT_FLOAT_EQ(args.ymin, -2.0f);
    EXPECT_FLOAT_EQ(args.ymax, 2.0f);
    EXPECT_FLOAT_EQ(args.tolerance, 1e-6f);
}

TEST(ArgumentsParserTest, ThrowsWhenXminIsNotLessThanXmax)
{
    const ArgvBuilder argv{
        "nfract",
        "--xmin", "1.0",
        "--xmax", "0.0"
    };

    EXPECT_THROW(static_cast<void>(ArgumentsParser::parse(argv.span())), std::invalid_argument);
}

TEST(ArgumentsParserTest, ThrowsWhenYminIsNotLessThanYmax)
{
    const ArgvBuilder argv{
        "nfract",
        "--ymin", "2.0",
        "--ymax", "2.0"
    };

    EXPECT_THROW(static_cast<void>(ArgumentsParser::parse(argv.span())), std::invalid_argument);
}
