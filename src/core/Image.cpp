#include "core/Image.hpp"

#include <stdexcept>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace nfract
{
    Image::Image(const int width, const int height) :
        m_width(width),
        m_height(height)
    {
        if (width < 0 || height < 0)
        {
            throw std::invalid_argument("Image dimensions must be non-negative");
        }

        if (width > 0 && height > 0)
        {
            const auto count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u;
            m_pixels.resize(count, pixel_type{0});
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

    std::span<Image::pixel_type> Image::row(const int y)
    {
        if (y < 0 || y >= m_height)
        {
            throw std::out_of_range("Image::row index out of range");
        }
        const auto offset = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) * 4u;
        return std::span{
            m_pixels.data() + offset,
            static_cast<std::size_t>(m_width) * 4u
        };
    }

    std::span<const Image::pixel_type> Image::row(const int y) const
    {
        if (y < 0 || y >= m_height)
        {
            throw std::out_of_range("Image::row index out of range");
        }
        const auto offset = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) * 4u;
        return std::span{
            m_pixels.data() + offset,
            static_cast<std::size_t>(m_width) * 4u
        };
    }

    Image::pixel_type* Image::pixel(const int x, const int y)
    {
        if (x < 0 || x >= m_width)
        {
            throw std::out_of_range("Image::pixel x index out of range");
        }
        if (y < 0 || y >= m_height)
        {
            throw std::out_of_range("Image::pixel y index out of range");
        }
        const auto idx = (static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x)) * 4u;
        return m_pixels.data() + idx;
    }

    const Image::pixel_type* Image::pixel(const int x, const int y) const
    {
        if (x < 0 || x >= m_width)
        {
            throw std::out_of_range("Image::pixel x index out of range");
        }
        if (y < 0 || y >= m_height)
        {
            throw std::out_of_range("Image::pixel y index out of range");
        }
        const auto idx = (static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x)) * 4u;
        return m_pixels.data() + idx;
    }

    Image::pixel_type* Image::data() noexcept
    {
        return m_pixels.data();
    }

    const Image::pixel_type* Image::data() const noexcept
    {
        return m_pixels.data();
    }

    std::span<Image::pixel_type> Image::pixels() noexcept
    {
        return m_pixels;
    }

    std::span<const Image::pixel_type> Image::pixels() const noexcept
    {
        return m_pixels;
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
