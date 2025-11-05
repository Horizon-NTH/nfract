#pragma once

#include <span>

#include "ArgumentsParser.hpp"

namespace nfract
{
    class Application
    {
    public:
        explicit Application(const std::span<const char* const>& args);
        Application(const Application&) = delete;
        Application(Application&&) = delete;

        int execute() const;

    private:
        Arguments m_arguments;
    };
}
