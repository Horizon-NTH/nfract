#include "app/Application.hpp"

int main(const int argc, char** argv)
{
    const nfract::Application app(std::span(argv, argc));
    return app.execute();
}
