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

        m_re.resize(static_cast<std::size_t>(n));
        m_im.resize(static_cast<std::size_t>(n));

        constexpr value_type two_pi = 2.0f * std::numbers::pi_v<value_type>;

        for (int k = 0; k < n; k++)
        {
            const value_type theta = two_pi * static_cast<value_type>(k) / static_cast<value_type>(n);
            m_re[static_cast<std::size_t>(k)] = std::cos(theta);
            m_im[static_cast<std::size_t>(k)] = std::sin(theta);
        }
    }

    int RootsTable::size() const noexcept
    {
        return static_cast<int>(m_re.size());
    }

    bool RootsTable::empty() const noexcept
    {
        return m_re.empty();
    }

    std::span<const RootsTable::value_type> RootsTable::re() const noexcept
    {
        return std::span{m_re};
    }

    std::span<const RootsTable::value_type> RootsTable::im() const noexcept
    {
        return std::span{m_im};
    }

    std::complex<RootsTable::value_type> RootsTable::root(const int index) const
    {
        if (index < 0 || index >= size())
        {
            throw std::out_of_range("RootsTable::root index out of range");
        }
        const auto idx = static_cast<std::size_t>(index);
        return std::complex{m_re[idx], m_im[idx]};
    }
}
