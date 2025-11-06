#pragma once
#include "app/ArgumentsParser.hpp"
#include "core/RootsTable.hpp"
#include "core/Image.hpp"

namespace nfract
{
    void render_newton_cpu(const Arguments& params, const RootsTable& roots, Image& image);
#ifndef RUN_ON_CPU
    void render_newton_ispc(const Arguments& p, const RootsTable& roots, Image& image);
#endif
}
