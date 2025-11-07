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

static inline void hsv_to_rgb_f(float h, float s, float v, float &rf, float &gf, float &bf)
{
    if (s <= 0.0f)
    {
        float val = clamp01(v);
        rf = val;
        gf = val;
        bf = val;
        return;
    }

    h = h - floor(h);
    h = h * 6.0f;

    int i = (int)floor(h);
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

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

    rf = clamp01(rf);
    gf = clamp01(gf);
    bf = clamp01(bf);
}

static inline uint8 to_byte01(float x)
{
    x = clamp01(x);
    return (uint8)(x * 255.0f + 0.5f);
}

static inline float compute_continuous_iteration(int iter, float bestDist2)
{
    const float SMOOTH = 1.0e-4f;

    float d = sqrt(bestDist2);
    d = max(d, 1.0e-12f);

    float ratio = log(d) / log(SMOOTH);
    ratio = max(ratio, 1.0e-12f);

    float continuous = (float)iter - log(ratio) / log(2.0f);
    return continuous;
}

static inline void shade_jewel(int iter, int maxIter, int bestIdx, int numRoots, float bestDist2, uint8 &R, uint8 &G, uint8 &B)
{
    if (maxIter <= 0 || numRoots <= 0)
    {
        R = G = B = 0;
        return;
    }

    float ci = compute_continuous_iteration(iter, bestDist2);
    float color_value = 0.7f + 0.3f * cos(0.18f * ci);

    if (iter == maxIter)
    {
        R = G = B = 0;
        return;
    }

    float h_base = (float)bestIdx / (float)numRoots;
    float h_highlight = h_base + 2.0f / 3.0f;

    float br, bg, bb;
    float hr, hg, hb;

    hsv_to_rgb_f(h_base, 1.0f, 1.0f, br, bg, bb);
    hsv_to_rgb_f(h_highlight, 1.0f, 1.0f, hr, hg, hb);

    float rf = (br + 0.3f * hr) * color_value;
    float gf = (bg + 0.3f * hg) * color_value;
    float bf = (bb + 0.3f * hb) * color_value;

    R = to_byte01(rf);
    G = to_byte01(gf);
    B = to_byte01(bf);
}

static inline void shade_neon(int iter, int maxIter, float bestDist2, uint8 &R, uint8 &G, uint8 &B)
{
    float ci = compute_continuous_iteration(iter, bestDist2);

    float rf = (-cos(0.025f * ci) + 1.0f) * 0.5f;
    float gf = (-cos(0.08f  * ci) + 1.0f) * 0.5f;
    float bf = (-cos(0.12f * ci) + 1.0f) * 0.5f;

    R = to_byte01(rf);
    G = to_byte01(gf);
    B = to_byte01(bf);
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
                           uniform int colorMode,
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

    uniform float invMaxIter = (maxIter  > 0) ? 1.0f / (float)maxIter  : 0.0f;
    uniform float invNumRoots = (numRoots > 0) ? 1.0f / (float)numRoots : 0.0f;
    uniform float tol2 = tolerance * tolerance;

    foreach_tiled (px = 0 ... width, py = 0 ... height)
    {
        float cx = xmin + dx * (float)px;
        float cy = ymin + dy * (float)py;

        Complex z;
        z.re = cx;
        z.im = cy;

        int iter = 0;
        for (; iter < maxIter; ++iter)
        {
            // z^(degree-1)
            Complex zn1 = pow_int(z, degree - 1);

            // f(z) = z^degree - 1
            Complex zn = mul(zn1, z);
            Complex fz;
            fz.re = zn.re - 1.0f;
            fz.im = zn.im;

            if (abs2(fz) < tol2)
            {
                break;
            }

            // f'(z) = degree * z^(degree-1)
            Complex fpz;
            fpz.re = (float)degree * zn1.re;
            fpz.im = (float)degree * zn1.im;

            float denom2 = abs2(fpz);
            if (denom2 < 1.0e-12f)
            {
                break;
            }

            // f / f' = (a+ib)/(c+id) = ((ac+bd) + i(bc-ad)) / (c^2+d^2)
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

        // Next we search the closest root
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

        // Then we can color the pixel based on the root reached and the iteration count
        // We also support a few different color modes
        uint8 R = 0, G = 0, B = 0;
        if (iter != maxIter && bestDist2 < tol2)
        {
            if (colorMode == 0)
            {
                shade_jewel(iter, maxIter, bestIdx, numRoots, bestDist2, R, G, B);
            }
            else if (colorMode == 1)
            {
                shade_neon(iter, maxIter, bestDist2, R, G, B);
            }
            else
            {
                float hue = (numRoots > 0) ? (float)bestIdx * invNumRoots : 0.0f;
                float t   = (maxIter > 1) ? 1.0f - (float)iter * invMaxIter : 1.0f;
                float val = clamp01(t);
                float sat = 1.0f;
                hsv_to_rgb(hue, sat, val, R, G, B);
            }
        }

        int idx = (py * width + px) * 4;
        out[idx + 0] = R;
        out[idx + 1] = G;
        out[idx + 2] = B;
        out[idx + 3] = 255;
    }
}
