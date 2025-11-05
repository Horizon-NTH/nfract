#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/Application.hpp"
#include "../support/TestUtils.hpp"

using nfract::Application;
using nfract::test::ArgvBuilder;
using nfract::test::TempFileGuard;

namespace test_utils = nfract::test;

TEST(ApplicationTest, ConstructorPropagatesInvalidArguments)
{
    ArgvBuilder argv({
        "nfract",
        "--xmin", "1.0",
        "--xmax", "0.0"
    });

    EXPECT_THROW(Application app(argv.span()), std::invalid_argument);
}

TEST(ApplicationTest, ExecuteProducesImageFile)
{
    TempFileGuard guard{test_utils::make_unique_path("nfract-app", ".png")};

    ArgvBuilder argv({
        "nfract",
        "--degree", "3",
        "--width", "32",
        "--height", "24",
        "--max-iter", "50",
        "--xmin", "-1.0",
        "--xmax", "1.0",
        "--ymin", "-1.0",
        "--ymax", "1.0",
        "--tol", "1e-4",
        "--out", guard.path().string()
    });

    testing::internal::CaptureStderr();
    Application app(argv.span());
    const int result = app.execute();
    const std::string captured = testing::internal::GetCapturedStderr();

    EXPECT_EQ(result, EXIT_SUCCESS);

    ASSERT_TRUE(std::filesystem::exists(guard.path()));
    EXPECT_GT(std::filesystem::file_size(guard.path()), 0);
    EXPECT_TRUE(captured.empty());
}

TEST(ApplicationTest, ExecuteFailsWhenOutputPathIsInvalid)
{
    const auto bad_path = test_utils::make_unique_path("nfract-missing").parent_path() / "subdir-does-not-exist" / "image.png";

    std::vector<std::string> args = {
        "nfract",
        "--degree", "3",
        "--width", "32",
        "--height", "24",
        "--max-iter", "20",
        "--xmin", "-1.0",
        "--xmax", "1.0",
        "--ymin", "-1.0",
        "--ymax", "1.0",
        "--tol", "1e-4",
        "--out", bad_path.string()
    };

    ArgvBuilder argv(std::move(args));
    Application app(argv.span());

    testing::internal::CaptureStderr();
    const int result = app.execute();
    const std::string captured = testing::internal::GetCapturedStderr();

    EXPECT_EQ(result, EXIT_FAILURE);
    EXPECT_FALSE(std::filesystem::exists(bad_path));
    EXPECT_EQ(captured, "Failed to write PNG\n");
}
