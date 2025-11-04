#include "core/RootsTable.hpp"

#include <stdexcept>
#include <numbers>

namespace nfract
{
    RootsTable::RootsTable(const int n)
    {
        if (n <= 0)
        {
            throw std::invalid_argument("RootsTable size must be positive");
        }

        m_real.resize(static_cast<std::size_t>(n));
        m_imag.resize(static_cast<std::size_t>(n));

        constexpr value_type two_pi = 2.0f * std::numbers::pi_v<value_type>;

        for (int k = 0; k < n; k++)
        {
            const value_type theta = two_pi * static_cast<value_type>(k) / static_cast<value_type>(n);
            m_real[static_cast<std::size_t>(k)] = std::cos(theta);
            m_imag[static_cast<std::size_t>(k)] = std::sin(theta);
        }
    }

    int RootsTable::size() const noexcept
    {
        return static_cast<int>(m_real.size());
    }

    bool RootsTable::empty() const noexcept
    {
        return m_real.empty();
    }

    std::span<const RootsTable::value_type> RootsTable::real() const noexcept
    {
        return std::span{m_real};
    }

    std::span<const RootsTable::value_type> RootsTable::imag() const noexcept
    {
        return std::span{m_imag};
    }

    std::complex<RootsTable::value_type> RootsTable::root(const int index) const
    {
        if (index < 0 || index >= size())
        {
            throw std::out_of_range("RootsTable::root index out of range");
        }
        const auto idx = static_cast<std::size_t>(index);
        return std::complex{m_real[idx], m_imag[idx]};
    }
}
