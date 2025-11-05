#pragma once

#include <vector>
#include <span>
#include <filesystem>
#include <optional>

namespace nfract
{
    class Image
    {
    public:
        using pixel_type = std::uint8_t;

        Image() = default;
        explicit Image(int width, int height);

        [[nodiscard]] int width() const noexcept;
        [[nodiscard]] int height() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

        /// RGBA pixels for row y as a span of size width * 4
        [[nodiscard]] std::span<pixel_type> row(int y);
        [[nodiscard]] std::span<const pixel_type> row(int y) const;

        /// Pointer to the first channel of pixel (x, y) (4 bytes: R,G,B,A)
        [[nodiscard]] pixel_type* pixel(int x, int y);
        [[nodiscard]] const pixel_type* pixel(int x, int y) const;

        [[nodiscard]] pixel_type* data() noexcept;
        [[nodiscard]] const pixel_type* data() const noexcept;
        [[nodiscard]] std::span<pixel_type> pixels() noexcept;
        [[nodiscard]] std::span<const pixel_type> pixels() const noexcept;

        /// Save as PNG (RGBA8). Returns true on success.
        [[nodiscard]] bool save_png(const std::filesystem::path& path, std::optional<int> stride_bytes = std::nullopt) const noexcept;

    private:
        int m_width = 0;
        int m_height = 0;
        std::vector<pixel_type> m_pixels;
    };
}
