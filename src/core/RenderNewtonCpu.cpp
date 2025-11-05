#include <core/RenderNewtonCpu.hpp>

#include <limits>
#include <cmath>

namespace nfract
{
    namespace
    {
        struct Complex
        {
            float re;
            float im;
        };

        [[nodiscard]] Complex mul(const Complex a, const Complex b) noexcept
        {
            return {
                a.re * b.re - a.im * b.im,
                a.re * b.im + a.im * b.re
            };
        }

        [[nodiscard]] float abs2(const Complex z) noexcept
        {
            return z.re * z.re + z.im * z.im;
        }

        [[nodiscard]] float abs(const Complex z) noexcept
        {
            return std::sqrt(abs2(z));
        }

        [[nodiscard]] Complex pow_int(const Complex z, const int k) noexcept
        {
            Complex res{1.0f, 0.0f};
            for (int i = 0; i < k; ++i)
            {
                res = mul(res, z);
            }
            return res;
        }

        void hsv_to_rgb(float h, const float s, const float v, std::uint8_t& R, std::uint8_t& G, std::uint8_t& B) noexcept
        {
            if (s <= 0.0f)
            {
                const auto val = static_cast<std::uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f);
                R = G = B = val;
                return;
            }

            h = std::fmod(h, 1.0f) * 6.0f;
            const int i = static_cast<int>(std::floor(h));
            const float f = h - static_cast<float>(i);
            const float p = v * (1.0f - s);
            const float q = v * (1.0f - s * f);
            const float t = v * (1.0f - s * (1.0f - f));

            auto to_u8 = [](const float x)
            {
                return static_cast<std::uint8_t>(std::clamp(x, 0.0f, 1.0f) * 255.0f);
            };

            float rf{}, gf{}, bf{};
            switch (i % 6)
            {
            case 0: rf = v;
                gf = t;
                bf = p;
                break;
            case 1: rf = q;
                gf = v;
                bf = p;
                break;
            case 2: rf = p;
                gf = v;
                bf = t;
                break;
            case 3: rf = p;
                gf = q;
                bf = v;
                break;
            case 4: rf = t;
                gf = p;
                bf = v;
                break;
            case 5: rf = v;
                gf = p;
                bf = q;
                break;
            default: break;
            }

            R = to_u8(rf);
            G = to_u8(gf);
            B = to_u8(bf);
        }
    }

    void render_newton_cpu(const Arguments& p, const RootsTable& roots, Image& image)
    {
        const int W = p.width;
        const int H = p.height;

        if (W <= 0 || H <= 0 || image.width() != W || image.height() != H)
            return;

        const float dx = (p.xmax - p.xmin) / static_cast<float>(std::max(1, W - 1));
        const float dy = (p.ymax - p.ymin) / static_cast<float>(std::max(1, H - 1));

        const auto roots_re = roots.re();
        const auto roots_im = roots.im();

        for (int py = 0; py < H; py++)
        {
            const float cy = p.ymin + dy * static_cast<float>(py);

            for (int px = 0; px < W; px++)
            {
                const float cx = p.xmin + dx * static_cast<float>(px);
                Complex z{cx, cy};

                int iter = 0;
                for (; iter < p.maxIter; ++iter)
                {
                    // f(z) = z^n - 1
                    auto [re, im] = pow_int(z, p.degree);
                    const Complex fz{re - 1.0f, im};

                    if (abs(fz) < p.tolerance)
                    {
                        break;
                    }

                    // f'(z) = n * z^(n-1)
                    const Complex zn1 = pow_int(z, p.degree - 1);
                    const Complex fpz{
                        static_cast<float>(p.degree) * zn1.re,
                        static_cast<float>(p.degree) * zn1.im
                    };

                    const float denom2 = abs2(fpz);
                    if (denom2 < 1e-12f)
                    {
                        break;
                    }

                    // f / f'
                    // (a+ib)/(c+id) = ((ac+bd) + i(bc-ad)) / (c^2+d^2)
                    const float a = fz.re;
                    const float b = fz.im;
                    const float c = fpz.re;
                    const float d = fpz.im;

                    const float invDen = 1.0f / denom2;
                    const Complex ratio{
                        (a * c + b * d) * invDen,
                        (b * c - a * d) * invDen
                    };

                    // z = z - f/f'
                    z.re -= ratio.re;
                    z.im -= ratio.im;
                }

                // Next we search the closest root
                int bestIdx = 0;
                float bestDist2 = std::numeric_limits<float>::max();
                for (int k = 0; k < roots.size(); ++k)
                {
                    const float rx = roots_re[static_cast<std::size_t>(k)];
                    const float ry = roots_im[static_cast<std::size_t>(k)];
                    const float dxr = z.re - rx;
                    const float dyr = z.im - ry;
                    const float d2 = dxr * dxr + dyr * dyr;

                    if (d2 < bestDist2)
                    {
                        bestDist2 = d2;
                        bestIdx = k;
                    }
                }

                // Color: hue = root index / n, value = based on iterations
                const float hue = static_cast<float>(bestIdx) / static_cast<float>(std::max(1, roots.size()));
                const float t = p.maxIter > 1
                                    ? (1.0f - static_cast<float>(iter) / static_cast<float>(p.maxIter))
                                    : 1.0f;
                const float value = std::clamp(t, 0.0f, 1.0f);
                constexpr float sat = 1.0f;

                std::uint8_t R{}, G{}, B{};
                hsv_to_rgb(hue, sat, value, R, G, B);

                auto* pix = image.pixel(px, py);
                pix[0] = R;
                pix[1] = G;
                pix[2] = B;
                pix[3] = 255;
            }
        }
    }
}
