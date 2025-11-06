struct Complex
{
    float re;
    float im;
};

static inline Complex mul(Complex a, Complex b)
{
    Complex r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
}

static inline float abs2(Complex z)
{
    return z.re * z.re + z.im * z.im;
}

static inline float cabs(Complex z)
{
    return sqrt(abs2(z));
}

static inline Complex pow_int(Complex z, uniform int k)
{
    Complex res;
    res.re = 1.0f;
    res.im = 0.0f;
    for (uniform int i = 0; i < k; ++i)
    {
        res = mul(res, z);
    }
    return res;
}

static inline float clamp01(float x)
{
    return max(0.0f, min(1.0f, x));
}

static inline void hsv_to_rgb(float h, float s, float v, uint8 &R, uint8 &G, uint8 &B)
{
    if (s <= 0.0f)
    {
        float val = clamp01(v);
        uint8 u = (uint8)(val * 255.0f);
        R = u;
        G = u;
        B = u;
        return;
    }

    h = h - floor(h);
    h = h * 6.0f;

    int i = (int)floor(h);
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    float rf, gf, bf;

    switch (i % 6)
    {
    case 0: rf = v; gf = t; bf = p; break;
    case 1: rf = q; gf = v; bf = p; break;
    case 2: rf = p; gf = v; bf = t; break;
    case 3: rf = p; gf = q; bf = v; break;
    case 4: rf = t; gf = p; bf = v; break;
    case 5: rf = v; gf = p; bf = q; break;
    default: rf = v; gf = v; bf = v; break;
    }

    float rcf = clamp01(rf);
    float gcf = clamp01(gf);
    float bcf = clamp01(bf);

    R = (uint8)(rcf * 255.0f);
    G = (uint8)(gcf * 255.0f);
    B = (uint8)(bcf * 255.0f);
}

export void newton_fractal(uniform int width,
                           uniform int height,
                           uniform float xmin,
                           uniform float xmax,
                           uniform float ymin,
                           uniform float ymax,
                           uniform int degree,
                           uniform int maxIter,
                           uniform float tolerance,
                           uniform const float roots_re[],
                           uniform const float roots_im[],
                           uniform int numRoots,
                           uniform uint8 out[])
{
    if (width <= 0 || height <= 0 || numRoots <= 0)
    {
        return;
    }

    uniform int wDen = (width > 1) ? (width  - 1) : 1;
    uniform int hDen = (height > 1) ? (height - 1) : 1;

    uniform float dx = (xmax - xmin) / (float)wDen;
    uniform float dy = (ymax - ymin) / (float)hDen;

    for (uniform int py = 0; py < height; ++py) {
        float cy = ymin + dy * (float)py;

        foreach (px = 0 ... width) {
            float cx = xmin + dx * (float)px;

            Complex z;
            z.re = cx;
            z.im = cy;

            int iter = 0;
            for (; iter < maxIter; ++iter)
            {
                // f(z) = z^degree - 1
                Complex zn = pow_int(z, degree);
                Complex fz;
                fz.re = zn.re - 1.0f;
                fz.im = zn.im;

                if (cabs(fz) < tolerance)
                {
                    break;
                }

                // f'(z) = degree * z^(degree-1)
                Complex zn1 = pow_int(z, degree - 1);
                Complex fpz;
                fpz.re = (float)degree * zn1.re;
                fpz.im = (float)degree * zn1.im;

                float denom2 = abs2(fpz);
                if (denom2 < 1.0e-12f)
                {
                    break;
                }

                // f / f'
                float a = fz.re;
                float b = fz.im;
                float c = fpz.re;
                float d = fpz.im;

                float invDen = 1.0f / denom2;
                Complex ratio;
                ratio.re = (a * c + b * d) * invDen;
                ratio.im = (b * c - a * d) * invDen;

                // z = z - f/f'
                z.re -= ratio.re;
                z.im -= ratio.im;
            }

            int   bestIdx   = 0;
            float bestDist2 = 1.0e30f;

            for (uniform int k = 0; k < numRoots; ++k)
            {
                uniform float rx = roots_re[k];
                uniform float ry = roots_im[k];

                float dxr = z.re - rx;
                float dyr = z.im - ry;
                float d2  = dxr * dxr + dyr * dyr;

                if (d2 < bestDist2)
                {
                    bestDist2 = d2;
                    bestIdx   = k;
                }
            }

            // Color: hue = root index / n, value = based on iterations
            float denomRoots = (float)((numRoots > 0) ? numRoots : 1);
            float hue = (float)bestIdx / denomRoots;

            float t;
            if (maxIter > 1)
            {
                t = 1.0f - (float)iter / (float)maxIter;
            }
            else
            {
                t = 1.0f;
            }

            float value = clamp01(t);
            float sat   = 1.0f;

            uint8 R, G, B;
            hsv_to_rgb(hue, sat, value, R, G, B);

            int idx = (py * width + px) * 4;
            out[idx + 0] = R;
            out[idx + 1] = G;
            out[idx + 2] = B;
            out[idx + 3] = 255;
        }
    }
}
