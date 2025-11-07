#include <core/RenderNewton.hpp>

#include <limits>
#include <cmath>
#include <algorithm>
#ifndef RUN_ON_CPU
#include <Newton_ispc.h>
#endif

namespace nfract
{
    namespace
    {
        struct Complex
        {
            float re;
            float im;
        };

        [[nodiscard]] float clamp01(const float x) noexcept
        {
            return std::clamp(x, 0.0f, 1.0f);
        }

        [[nodiscard]] std::uint8_t to_byte01(const float x) noexcept
        {
            return static_cast<std::uint8_t>(clamp01(x) * 255.0f + 0.5f);
        }

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

        [[nodiscard]] Complex pow_int(const Complex z, const int k) noexcept
        {
            Complex res{1.0f, 0.0f};
            for (int i = 0; i < k; ++i)
            {
                res = mul(res, z);
            }
            return res;
        }

        void hsv_to_rgb_f(float h, const float s, const float v, float& rf, float& gf, float& bf) noexcept
        {
            if (s <= 0.0f)
            {
                const float val = clamp01(v);
                rf = gf = bf = val;
                return;
            }

            h = (h - std::floor(h)) * 6.0f;
            const int i = static_cast<int>(std::floor(h));
            const float f = h - static_cast<float>(i);
            const float p = v * (1.0f - s);
            const float q = v * (1.0f - s * f);
            const float t = v * (1.0f - s * (1.0f - f));

            rf = 0.0f;
            gf = 0.0f;
            bf = 0.0f;
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

            rf = clamp01(rf);
            gf = clamp01(gf);
            bf = clamp01(bf);
        }

        void hsv_to_rgb(float h, const float s, const float v, std::uint8_t& R, std::uint8_t& G, std::uint8_t& B) noexcept
        {
            if (s <= 0.0f)
            {
                const auto val = static_cast<std::uint8_t>(clamp01(v) * 255.0f);
                R = G = B = val;
                return;
            }

            float rf{}, gf{}, bf{};
            hsv_to_rgb_f(h, s, v, rf, gf, bf);

            auto to_u8 = [](const float x) noexcept
            {
                return static_cast<std::uint8_t>(clamp01(x) * 255.0f);
            };

            R = to_u8(rf);
            G = to_u8(gf);
            B = to_u8(bf);
        }

        [[nodiscard]] float compute_continuous_iteration(const int iter, const float bestDist2) noexcept
        {
            constexpr float SMOOTH = 1.0e-4f;

            float d = std::sqrt(bestDist2);
            d = std::max(d, 1.0e-12f);

            float ratio = std::log(d) / std::log(SMOOTH);
            ratio = std::max(ratio, 1.0e-12f);

            return static_cast<float>(iter) - std::log(ratio) / std::log(2.0f);
        }

        void shade_jewelry(const int iter, const int maxIter, const int bestIdx, const int numRoots, const float bestDist2, std::uint8_t& R, std::uint8_t& G, std::uint8_t& B) noexcept
        {
            if (maxIter <= 0 || numRoots <= 0)
            {
                R = G = B = 0;
                return;
            }

            const float ci = compute_continuous_iteration(iter, bestDist2);
            const float color_value = 0.7f + 0.3f * std::cos(0.18f * ci);

            if (iter == maxIter)
            {
                R = G = B = 0;
                return;
            }

            const float h_base = static_cast<float>(bestIdx) / static_cast<float>(numRoots);
            const float h_highlight = h_base + 2.0f / 3.0f;

            float br{}, bg{}, bb{};
            float hr{}, hg{}, hb{};

            hsv_to_rgb_f(h_base, 1.0f, 1.0f, br, bg, bb);
            hsv_to_rgb_f(h_highlight, 1.0f, 1.0f, hr, hg, hb);

            const float rf = (br + 0.3f * hr) * color_value;
            const float gf = (bg + 0.3f * hg) * color_value;
            const float bf = (bb + 0.3f * hb) * color_value;

            R = to_byte01(rf);
            G = to_byte01(gf);
            B = to_byte01(bf);
        }

        void shade_neon(const int iter, [[maybe_unused]] const int maxIter, const float bestDist2, std::uint8_t& R, std::uint8_t& G, std::uint8_t& B) noexcept
        {
            const float ci = compute_continuous_iteration(iter, bestDist2);

            const float rf = (-std::cos(0.025f * ci) + 1.0f) * 0.5f;
            const float gf = (-std::cos(0.08f * ci) + 1.0f) * 0.5f;
            const float bf = (-std::cos(0.12f * ci) + 1.0f) * 0.5f;

            R = to_byte01(rf);
            G = to_byte01(gf);
            B = to_byte01(bf);
        }

        void shade_classic(const int iter, const int maxIter, const int bestIdx, const int numRoots, std::uint8_t& R, std::uint8_t& G, std::uint8_t& B) noexcept
        {
            const float hue = numRoots > 0 ? static_cast<float>(bestIdx) / static_cast<float>(numRoots) : 0.0f;
            const float t = maxIter > 1
                                ? 1.0f - static_cast<float>(iter) / static_cast<float>(maxIter)
                                : 1.0f;
            const float value = std::clamp(t, 0.0f, 1.0f);
            constexpr float sat = 1.0f;

            hsv_to_rgb(hue, sat, value, R, G, B);
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
        const float tol2 = p.tolerance * p.tolerance;

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
                    // z^(n-1)
                    const Complex zn1 = pow_int(z, p.degree - 1);

                    // f(z) = z^n - 1
                    const Complex zn = mul(zn1, z);
                    const Complex fz{zn.re - 1.0f, zn.im};

                    if (abs2(fz) < tol2)
                    {
                        break;
                    }

                    // f'(z) = n * z^(n-1)
                    const Complex fpz{
                        static_cast<float>(p.degree) * zn1.re,
                        static_cast<float>(p.degree) * zn1.im
                    };

                    const float denom2 = abs2(fpz);
                    if (denom2 < 1e-12f)
                    {
                        break;
                    }

                    // f / f' = (a+ib)/(c+id) = ((ac+bd) + i(bc-ad)) / (c^2+d^2)
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
                std::uint8_t R{}, G{}, B{};
                const int numRoots = static_cast<int>(roots.size());
                switch (p.colorMode)
                {
                case ColorMode::JEWELRY:
                    shade_jewelry(iter, p.maxIter, bestIdx, numRoots, bestDist2, R, G, B);
                    break;
                case ColorMode::NEON:
                    shade_neon(iter, p.maxIter, bestDist2, R, G, B);
                    break;
                case ColorMode::CLASSIC:
                default:
                    shade_classic(iter, p.maxIter, bestIdx, numRoots, R, G, B);
                    break;
                }

                auto* pix = image.pixel(px, py);
                pix[0] = R;
                pix[1] = G;
                pix[2] = B;
                pix[3] = 255;
            }
        }
    }

#ifndef RUN_ON_CPU
    void render_newton_ispc(const Arguments& p, const RootsTable& roots, Image& image)
    {
        if (p.width <= 0 || p.height <= 0 || image.width() != p.width || image.height() != p.height || roots.empty())
        {
            return;
        }

        const auto roots_re = roots.re();
        const auto roots_im = roots.im();

        ispc::newton_fractal(
            p.width,
            p.height,
            p.xmin,
            p.xmax,
            p.ymin,
            p.ymax,
            p.degree,
            p.maxIter,
            p.tolerance,
            roots_re.data(),
            roots_im.data(),
            roots.size(),
            static_cast<int>(p.colorMode),
            image.data()
        );
    }

#endif
}
