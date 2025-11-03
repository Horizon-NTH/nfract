#include "core/Image.hpp"

#include <cassert>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace nfract
{
    Image::Image(const int width, const int height) :
        m_width(width),
        m_height(height)
    {
        assert(width >= 0 && height >= 0);

        if (width > 0 && height > 0)
        {
            const auto count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u;
            m_pixels.resize(count, Pixel{0});
        }
    }

    int Image::width() const noexcept
    {
        return m_width;
    }

    int Image::height() const noexcept
    {
        return m_height;
    }

    bool Image::empty() const noexcept
    {
        return m_width <= 0 || m_height <= 0 || m_pixels.empty();
    }

    std::span<Image::Pixel> Image::row(const int y) noexcept
    {
        assert(y >= 0 && y < m_height);
        const auto offset = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) * 4u;
        return std::span{
            m_pixels.data() + offset,
            static_cast<std::size_t>(m_width) * 4u
        };
    }

    std::span<const Image::Pixel> Image::row(const int y) const noexcept
    {
        assert(y >= 0 && y < m_height);
        const auto offset = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) * 4u;
        return std::span{
            m_pixels.data() + offset,
            static_cast<std::size_t>(m_width) * 4u
        };
    }

    Image::Pixel* Image::pixel(const int x, const int y) noexcept
    {
        assert(x >= 0 && x < m_width);
        assert(y >= 0 && y < m_height);
        const auto idx = (static_cast<std::size_t>(y) *
            static_cast<std::size_t>(m_width) +
            static_cast<std::size_t>(x)) * 4u;
        return m_pixels.data() + idx;
    }

    const Image::Pixel* Image::pixel(const int x, const int y) const noexcept
    {
        assert(x >= 0 && x < m_width);
        assert(y >= 0 && y < m_height);
        const auto idx = (static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x)) * 4u;
        return m_pixels.data() + idx;
    }

    bool Image::save_png(const std::filesystem::path& path, std::optional<int> stride_bytes) const noexcept
    {
        if (empty())
        {
            return false;
        }

        const auto filename = path.string();
        const int result = stbi_write_png(filename.c_str(), m_width, m_height, 4, m_pixels.data(), stride_bytes.value_or(m_width * 4));

        return result != 0;
    }
}
