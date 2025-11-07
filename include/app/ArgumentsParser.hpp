#pragma once

#include <span>
#include <stdexcept>
#include <string>

namespace nfract
{
    enum class ColorMode
    {
        JEWELRY = 0,
        NEON = 1,
        CLASSIC = 2,
    };

    struct Arguments
    {
        int degree = 5; // n in z^n - 1 = 0
        int width = 1920;
        int height = 1080;
        int maxIter = 100;
        float xmin = -2.0f;
        float xmax = 2.0f;
        float ymin = -2.0f;
        float ymax = 2.0f;
        float tolerance = 1e-6f;
        std::string outputPath = "nfract.png";
        ColorMode colorMode = ColorMode::CLASSIC;
    };

    class ArgumentsParser
    {
    public:
        ArgumentsParser() = delete;

        [[nodiscard]] static Arguments parse(const std::span<const char* const>& args);
    };
}
