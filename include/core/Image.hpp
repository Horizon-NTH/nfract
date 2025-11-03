#pragma once

#include <vector>
#include <span>
#include <filesystem>

namespace nfract
{
    class Image
    {
    public:
        using Pixel = std::uint8_t;

        Image() = default;
        explicit Image(int width, int height);

        [[nodiscard]] int width() const noexcept;
        [[nodiscard]] int height() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

        /// RGBA pixels for row y as a span of size width * 4
        [[nodiscard]] std::span<Pixel> row(int y) noexcept;
        [[nodiscard]] std::span<const Pixel> row(int y) const noexcept;

        /// Pointer to the first channel of pixel (x, y) (4 bytes: R,G,B,A)
        [[nodiscard]] Pixel* pixel(int x, int y) noexcept;
        [[nodiscard]] const Pixel* pixel(int x, int y) const noexcept;

        [[nodiscard]] Pixel* data() noexcept { return m_pixels.data(); }
        [[nodiscard]] const Pixel* data() const noexcept { return m_pixels.data(); }
        [[nodiscard]] std::span<Pixel> pixels() noexcept { return m_pixels; }
        [[nodiscard]] std::span<const Pixel> pixels() const noexcept { return m_pixels; }

        /// Save as PNG (RGBA8). Returns true on success.
        [[nodiscard]] bool save_png(const std::filesystem::path& path, std::optional<int> stride_bytes = std::nullopt) const noexcept;

    private:
        int m_width = 0;
        int m_height = 0;
        std::vector<Pixel> m_pixels;
    };
}
