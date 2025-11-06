#include "app/Application.hpp"

#include <iostream>

#include "core/RenderNewton.hpp"
#include "core/RootsTable.hpp"

namespace nfract
{
    Application::Application(const std::span<const char* const>& args) :
        m_arguments(ArgumentsParser::parse(args))
    {
    }

    int Application::execute() const
    {
        const RootsTable roots{m_arguments.degree};
        Image img{m_arguments.width, m_arguments.height};

#ifdef RUN_ON_CPU
        render_newton_cpu(m_arguments, roots, img);
#else
        render_newton_ispc(m_arguments, roots, img);
#endif

        if (!img.save_png(m_arguments.outputPath))
        {
            std::cerr << "Failed to write PNG" << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
}
