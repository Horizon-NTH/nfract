#pragma once

#include <complex>
#include <span>
#include <vector>

namespace nfract
{
    class RootsTable
    {
    public:
        using value_type = float;

        RootsTable() = default;
        explicit RootsTable(int n);

        [[nodiscard]] int size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] std::span<const value_type> re() const noexcept;
        [[nodiscard]] std::span<const value_type> im() const noexcept;

        [[nodiscard]] std::complex<value_type> root(int index) const;

    private:
        std::vector<value_type> m_re;
        std::vector<value_type> m_im;
    };
}
