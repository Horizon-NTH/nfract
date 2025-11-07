#include "app/ArgumentsParser.hpp"

#include <stdexcept>

#include <CLI11.hpp>

namespace nfract
{
    Arguments ArgumentsParser::parse(const std::span<const char* const>& args)
    {
        Arguments arguments;

        CLI::App app{"nfract - Newton fractal renderer"};

        app.set_help_all_flag("--help-all", "Show all help");
        app.allow_extras(false);
        app.set_version_flag("-v,--version", PROJECT_VERSION, "Print version information and exit");

        app.add_option("-n,--degree", arguments.degree,
                       "Degree n in z^n - 1 = 0")
           ->check(CLI::Range(2, 64))
           ->default_val(arguments.degree);

        app.add_option("--width", arguments.width,
                       "Image width in pixels")
           ->check(CLI::PositiveNumber)
           ->default_val(arguments.width);

        app.add_option("--height", arguments.height,
                       "Image height in pixels")
           ->check(CLI::PositiveNumber)
           ->default_val(arguments.height);

        app.add_option("--xmin", arguments.xmin,
                       "Minimum real value (left)")
           ->default_val(arguments.xmin);

        app.add_option("--xmax", arguments.xmax,
                       "Maximum real value (right)")
           ->default_val(arguments.xmax);

        app.add_option("--ymin", arguments.ymin,
                       "Minimum imaginary value (bottom)")
           ->default_val(arguments.ymin);

        app.add_option("--ymax", arguments.ymax,
                       "Maximum imaginary value (top)")
           ->default_val(arguments.ymax);

        app.add_option("--max-iter", arguments.maxIter,
                       "Maximum number of Newton iterations")
           ->check(CLI::Range(1, 10'000))
           ->default_val(arguments.maxIter);

        app.add_option("--tol", arguments.tolerance,
                       "Convergence tolerance on |f(z)|")
           ->check(CLI::Range(1e-9, 1e-2))
           ->default_val(arguments.tolerance);

        app.add_option("-o,--out", arguments.outputPath,
                       "Output PNG file path")
           ->default_val(arguments.outputPath);

        bool use_neon = false;
        bool use_jewelry = false;
        auto* neon_flag = app.add_flag("--neon", use_neon, "Render using the neon color palette");
        auto* jewelry_flag = app.add_flag("--jewelry", use_jewelry, "Render using the jewelry color palette");
        neon_flag->excludes(jewelry_flag);
        jewelry_flag->excludes(neon_flag);

        try
        {
            app.parse(args.size(), args.data());
        }
        catch (const CLI::ParseError& e)
        {
            std::exit(app.exit(e));
        }

        if (arguments.xmin >= arguments.xmax)
        {
            throw std::invalid_argument("xmin must be < xmax");
        }
        if (arguments.ymin >= arguments.ymax)
        {
            throw std::invalid_argument("ymin must be < ymax");
        }

        if (use_jewelry)
        {
            arguments.colorMode = ColorMode::JEWELRY;
        }
        else if (use_neon)
        {
            arguments.colorMode = ColorMode::NEON;
        }

        return arguments;
    }
}
