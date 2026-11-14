#include "HUD.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>

HUD::HUD() {}

// =====================================================================
//  LANDSCAPE DRAWING HELPERS (top-down view, sunlight from top-left)
// =====================================================================
namespace {

const double kPi = 3.14159265358979;

// Deterministic tiny RNG so the scenery looks "random" but identical every run
struct Rng {
    unsigned s;
    explicit Rng(unsigned seed) : s(seed * 2654435761u + 1u) {}
    int next(int n) {
        if (n <= 1) return 0;
        s = s * 1103515245u + 12345u;
        return static_cast<int>((s >> 16) % static_cast<unsigned>(n));
    }
    int range(int lo, int hi) { return lo + next(hi - lo + 1); }
};

struct Foliage { int dark, mid, light, hi; };

// Shared palette
const int LAWN_A       = COLOR(78, 128, 58);
const int LAWN_B       = COLOR(70, 119, 52);
const int LAWN_SHADOW  = COLOR(46, 86, 38);
const int PATH_EDGE    = COLOR(142, 134, 112);
const int PATH_FILL    = COLOR(204, 194, 166);
const int STONE_LIGHT  = COLOR(198, 190, 172);
const int STONE_DARK   = COLOR(150, 142, 124);
const int WOOD_DARK    = COLOR(112, 74, 40);
const int WOOD_LIGHT   = COLOR(160, 112, 64);
const int METAL_DARK   = COLOR(52, 56, 62);

void disc(int x, int y, int rx, int ry, int col) {
    setcolor(col);
    setfillstyle(SOLID_FILL, col);
    fillellipse(x, y, rx, ry);
}

void box(int x1, int y1, int x2, int y2, int col) {
    setfillstyle(SOLID_FILL, col);
    bar(x1, y1, x2 + 1, y2 + 1);
}

void poly(int n, int* pts, int fill, int outline) {
    setcolor(outline);
    setfillstyle(SOLID_FILL, fill);
    fillpoly(n, pts);
}

void roundBox(int x1, int y1, int x2, int y2, int r, int col) {
    box(x1 + r, y1, x2 - r, y2, col);
    box(x1, y1 + r, x2, y2 - r, col);
    disc(x1 + r, y1 + r, r, r, col);
    disc(x2 - r, y1 + r, r, r, col);
    disc(x1 + r, y2 - r, r, r, col);
    disc(x2 - r, y2 - r, r, r, col);
}

void star(int cx, int cy, int spikes, double rOut, double rIn, double rot, int col) {
    int pts[64];
    int n = std::min(spikes * 2, 32);
    for (int i = 0; i < n; ++i) {
        double a = rot + i * kPi / spikes;
        double r = (i % 2 == 0) ? rOut : rIn;
        pts[2 * i]     = cx + static_cast<int>(std::cos(a) * r);
        pts[2 * i + 1] = cy + static_cast<int>(std::sin(a) * r);
    }
    poly(n, pts, col, col);
}

// Organic, slightly irregular blob (ponds, beds)
void blob(int cx, int cy, double rx, double ry, double scale, int col) {
    const int N = 40;
    int pts[2 * N];
    for (int i = 0; i < N; ++i) {
        double a = i * 2.0 * kPi / N;
        double f = 1.0 + 0.09 * std::sin(3 * a + 0.6) + 0.05 * std::cos(5 * a + 1.3);
        pts[2 * i]     = cx + static_cast<int>(std::cos(a) * rx * f * scale);
        pts[2 * i + 1] = cy + static_cast<int>(std::sin(a) * ry * f * scale);
    }
    poly(N, pts, col, col);
}

// ---------- Lawn with mowing stripes, grass tufts and a few wild flowers ----------
void drawLawn(int x1, int y1, int x2, int y2, bool verticalStripes, unsigned seed) {
    const int stripe = 18;
    if (verticalStripes) {
        for (int x = x1, i = 0; x <= x2; x += stripe, ++i)
            box(x, y1, std::min(x + stripe - 1, x2), y2, (i % 2) ? LAWN_A : LAWN_B);
    } else {
        for (int y = y1, i = 0; y <= y2; y += stripe, ++i)
            box(x1, y, x2, std::min(y + stripe - 1, y2), (i % 2) ? LAWN_A : LAWN_B);
    }

    Rng rng(seed);
    const int w = x2 - x1, h = y2 - y1;
    const int tufts = (w * h) / 70;
    const int tuftCols[4] = { COLOR(58, 102, 44), COLOR(92, 144, 66), COLOR(64, 110, 48), COLOR(108, 140, 66) };
    for (int i = 0; i < tufts; ++i) {
        int x = x1 + rng.next(w), y = y1 + rng.next(h);
        setcolor(tuftCols[rng.next(4)]);
        line(x, y, x + rng.range(-1, 1), y - rng.range(1, 3));
    }

    // Sparse daisies / dandelions
    for (int i = 0; i < (w * h) / 2600; ++i) {
        int x = x1 + 2 + rng.next(w - 4), y = y1 + 2 + rng.next(h - 4);
        int c = rng.next(3) == 0 ? COLOR(245, 210, 60) : COLOR(240, 240, 232);
        putpixel(x, y, c);
        putpixel(x + 1, y, c);
    }
}

// ---------- Trees & shrubs ----------
const Foliage OAK    = { COLOR(30, 78, 36),  COLOR(44, 104, 46), COLOR(66, 132, 58), COLOR(104, 164, 80) };
const Foliage MAPLE  = { COLOR(52, 88, 30),  COLOR(78, 120, 40), COLOR(108, 150, 56), COLOR(150, 182, 82) };
const Foliage LINDEN = { COLOR(22, 66, 42),  COLOR(32, 88, 54),  COLOR(50, 114, 68), COLOR(84, 146, 92) };
const Foliage HEDGE  = { COLOR(26, 70, 34),  COLOR(40, 94, 42),  COLOR(60, 120, 54), COLOR(90, 150, 72) };

void drawDeciduousTree(int x, int y, int r, const Foliage& f, unsigned seed) {
    Rng rng(seed);
    int off = r / 3 + 2;
    disc(x + off, y + off, r, r * 9 / 10, LAWN_SHADOW); // cast shadow

    // Bumpy outer canopy silhouette
    disc(x, y, r * 8 / 10, r * 8 / 10, f.dark);
    const int lobes = 8;
    for (int i = 0; i < lobes; ++i) {
        double a = i * 2.0 * kPi / lobes + rng.next(60) / 100.0;
        int lr = r * 45 / 100 + rng.next(r / 6 + 1);
        int d = r - lr;
        disc(x + static_cast<int>(std::cos(a) * d), y + static_cast<int>(std::sin(a) * d), lr, lr, f.dark);
    }
    // Mid canopy, shifted toward the sun
    for (int i = 0; i < 6; ++i) {
        double a = i * 2.0 * kPi / 6 + rng.next(80) / 100.0;
        int lr = r * 36 / 100 + rng.next(r / 8 + 1);
        int d = r * 33 / 100;
        disc(x - r / 8 + static_cast<int>(std::cos(a) * d), y - r / 8 + static_cast<int>(std::sin(a) * d), lr, lr, f.mid);
    }
    // Sun-lit leaf clusters
    for (int i = 0; i < 4; ++i) {
        int lr = r / 4 + rng.next(3);
        disc(x - r / 3 + rng.range(-r / 5, r / 5), y - r / 3 + rng.range(-r / 5, r / 5), lr, lr, f.light);
    }
    for (int i = 0; i < 3; ++i) {
        int lr = std::max(2, r / 9);
        disc(x - r / 2 + rng.next(r / 2 + 1), y - r / 2 + rng.next(r / 2 + 1), lr, lr, f.hi);
    }
    // Leaf texture gaps
    for (int i = 0; i < r * 2; ++i) {
        int px = rng.range(-r, r), py = rng.range(-r, r);
        if (px * px + py * py < (r * r * 6) / 10) putpixel(x + px, y + py, f.dark);
    }
}

void drawConifer(int x, int y, int r) {
    disc(x + r / 3 + 1, y + r / 3 + 1, r, r, LAWN_SHADOW);
    const int cols[4] = { COLOR(18, 58, 38), COLOR(26, 76, 48), COLOR(38, 96, 58), COLOR(60, 122, 74) };
    for (int k = 0; k < 4; ++k) {
        double ro = r * (1.0 - k * 0.22);
        star(x - k, y - k, 9, ro, ro * 0.72, k * 0.35, cols[k]);
    }
    disc(x - 3, y - 3, 2, 2, COLOR(96, 156, 100));
}

void drawShrub(int x, int y, int r, const Foliage& f, unsigned seed, int bloomCol = -1) {
    Rng rng(seed);
    disc(x + 2, y + 2, r, r * 8 / 10, LAWN_SHADOW);
    for (int i = 0; i < 4; ++i) {
        int lr = r * 6 / 10;
        disc(x + rng.range(-r / 2, r / 2), y + rng.range(-r / 2, r / 2), lr, lr, f.dark);
    }
    disc(x - 1, y - 1, r * 6 / 10, r * 6 / 10, f.mid);
    disc(x - r / 3, y - r / 3, r / 3 + 1, r / 3 + 1, f.light);
    if (bloomCol >= 0) {
        for (int i = 0; i < r + 3; ++i) {
            int px = rng.range(-r + 2, r - 2), py = rng.range(-r + 2, r - 2);
            if (px * px + py * py < (r - 1) * (r - 1)) {
                putpixel(x + px, y + py, bloomCol);
                putpixel(x + px + 1, y + py, bloomCol);
            }
        }
    }
}

// Trimmed boxwood hedge strip
void drawHedge(int x1, int y1, int x2, int y2, unsigned seed) {
    if (x2 <= x1 || y2 <= y1) return;
    Rng rng(seed);
    box(x1 + 2, y1 + 2, x2 + 2, y2 + 2, LAWN_SHADOW);
    box(x1, y1, x2, y2, HEDGE.dark);
    bool horiz = (x2 - x1) >= (y2 - y1);
    int t = horiz ? (y2 - y1) : (x2 - x1);
    int len = horiz ? (x2 - x1) : (y2 - y1);
    for (int s = 2; s <= len - 2; s += 5) {
        int cx = horiz ? x1 + s : (x1 + x2) / 2;
        int cy = horiz ? (y1 + y2) / 2 : y1 + s;
        int r = t / 2 - 1 + rng.next(2);
        disc(cx - 1, cy - 1, r, r, HEDGE.mid);
        if (rng.next(3) == 0) disc(cx - 2, cy - 2, 1, 1, HEDGE.light);
        if (rng.next(4) == 0) putpixel(cx + 1, cy + 1, HEDGE.hi);
    }
}

// ---------- Hardscape ----------
// Paved footpath along a quadratic Bezier curve
void drawFootpath(int x0, int y0, int cx, int cy, int x1, int y1, int hw, unsigned seed) {
    double len = std::hypot(cx - x0, cy - y0) + std::hypot(x1 - cx, y1 - cy);
    int steps = std::max(8, static_cast<int>(len / 2));
    for (int pass = 0; pass < 2; ++pass) {
        int r = (pass == 0) ? hw + 2 : hw;
        int col = (pass == 0) ? PATH_EDGE : PATH_FILL;
        for (int i = 0; i <= steps; ++i) {
            double t = static_cast<double>(i) / steps, u = 1.0 - t;
            int bx = static_cast<int>(u * u * x0 + 2 * u * t * cx + t * t * x1);
            int by = static_cast<int>(u * u * y0 + 2 * u * t * cy + t * t * y1);
            disc(bx, by, r, r, col);
        }
    }
    // Gravel / paver speckle
    Rng rng(seed);
    for (int i = 0; i < steps * 3; ++i) {
        double t = rng.next(1000) / 1000.0, u = 1.0 - t;
        int bx = static_cast<int>(u * u * x0 + 2 * u * t * cx + t * t * x1) + rng.range(-hw + 2, hw - 2);
        int by = static_cast<int>(u * u * y0 + 2 * u * t * cy + t * t * y1) + rng.range(-hw + 2, hw - 2);
        putpixel(bx, by, rng.next(2) ? COLOR(178, 168, 142) : COLOR(222, 214, 190));
    }
}

void drawBench(int cx, int cy, bool horizontal) {
    int hl = 12, hd = 4;
    int x1 = horizontal ? cx - hl : cx - hd, x2 = horizontal ? cx + hl : cx + hd;
    int y1 = horizontal ? cy - hd : cy - hl, y2 = horizontal ? cy + hd : cy + hl;
    box(x1 + 2, y1 + 2, x2 + 2, y2 + 2, LAWN_SHADOW);
    box(x1, y1, x2, y2, WOOD_DARK);
    setcolor(WOOD_LIGHT);
    if (horizontal) {
        for (int y = y1 + 1; y < y2; y += 2) line(x1 + 1, y, x2 - 1, y);
        box(x1 - 1, y1 - 1, x1 + 1, y2 + 1, METAL_DARK);
        box(x2 - 1, y1 - 1, x2 + 1, y2 + 1, METAL_DARK);
    } else {
        for (int x = x1 + 1; x < x2; x += 2) line(x, y1 + 1, x, y2 - 1);
        box(x1 - 1, y1 - 1, x2 + 1, y1 + 1, METAL_DARK);
        box(x1 - 1, y2 - 1, x2 + 1, y2 + 1, METAL_DARK);
    }
}

void drawPicnicTable(int cx, int cy) {
    box(cx - 16 + 3, cy - 13 + 3, cx + 16 + 3, cy + 13 + 3, LAWN_SHADOW);
    // Bench seats
    box(cx - 16, cy - 13, cx + 16, cy - 9, WOOD_DARK);
    box(cx - 16, cy + 9, cx + 16, cy + 13, WOOD_DARK);
    // Table top with planks
    box(cx - 16, cy - 6, cx + 16, cy + 6, WOOD_LIGHT);
    setcolor(WOOD_DARK);
    line(cx - 16, cy - 2, cx + 16, cy - 2);
    line(cx - 16, cy + 2, cx + 16, cy + 2);
    setcolor(COLOR(186, 138, 86));
    line(cx - 15, cy - 5, cx + 15, cy - 5);
}

void drawFlowerBed(int cx, int cy, int rx, int ry, unsigned seed, const int* blooms, int nBlooms) {
    Rng rng(seed);
    disc(cx, cy, rx + 3, ry + 3, COLOR(160, 150, 136));  // stone edging
    disc(cx, cy, rx, ry, COLOR(88, 60, 40));             // mulch / soil
    // Foliage tufts
    for (int i = 0; i < (rx * ry) / 6; ++i) {
        int px = rng.range(-rx + 2, rx - 2), py = rng.range(-ry + 2, ry - 2);
        if (px * px * ry * ry + py * py * rx * rx < rx * rx * ry * ry * 8 / 10)
            disc(cx + px, cy + py, 2, 2, rng.next(2) ? COLOR(52, 112, 48) : COLOR(70, 134, 58));
    }
    // Blossoms
    for (int i = 0; i < (rx * ry) / 4; ++i) {
        int px = rng.range(-rx + 2, rx - 2), py = rng.range(-ry + 2, ry - 2);
        if (px * px * ry * ry + py * py * rx * rx < rx * rx * ry * ry * 8 / 10) {
            int c = blooms[rng.next(nBlooms)];
            putpixel(cx + px, cy + py, c);
            putpixel(cx + px + 1, cy + py, c);
            putpixel(cx + px, cy + py + 1, c);
        }
    }
}

void drawPond(int cx, int cy, int rx, int ry, unsigned seed) {
    Rng rng(seed);
    blob(cx + 3, cy + 3, rx, ry, 1.14, LAWN_SHADOW);
    blob(cx, cy, rx, ry, 1.14, COLOR(132, 126, 112));   // stone rim
    blob(cx, cy, rx, ry, 1.06, COLOR(98, 88, 66));      // muddy bank
    blob(cx, cy, rx, ry, 1.00, COLOR(78, 146, 172));    // shallow water
    blob(cx, cy, rx, ry, 0.78, COLOR(56, 120, 158));
    blob(cx, cy, rx, ry, 0.50, COLOR(42, 100, 140));    // deep centre

    // Rim boulders
    for (int i = 0; i < 26; ++i) {
        double a = i * 2.0 * kPi / 26 + rng.next(20) / 100.0;
        double f = 1.0 + 0.09 * std::sin(3 * a + 0.6) + 0.05 * std::cos(5 * a + 1.3);
        int bx = cx + static_cast<int>(std::cos(a) * rx * f * 1.11);
        int by = cy + static_cast<int>(std::sin(a) * ry * f * 1.11);
        int br = rng.range(2, 4);
        disc(bx, by, br, br, rng.next(2) ? COLOR(160, 156, 146) : COLOR(118, 114, 104));
        putpixel(bx - 1, by - 1, COLOR(196, 192, 182));
    }

    // Reeds on the shaded side
    setcolor(COLOR(70, 110, 46));
    for (int i = 0; i < 18; ++i) {
        int bx = cx + rx / 2 + rng.range(0, rx / 2) - 4;
        int by = cy + ry / 3 + rng.range(-ry / 3, ry / 2);
        line(bx, by, bx + rng.range(-2, 2), by - rng.range(3, 6));
    }

    // Lily pads
    const int pads[4][3] = { { -rx / 2, -ry / 4, 5 }, { -rx / 3, ry / 3, 4 }, { rx / 5, -ry / 2, 4 }, { -rx / 5, 0, 3 } };
    for (int i = 0; i < 4; ++i) {
        int px = cx + pads[i][0], py = cy + pads[i][1], pr = pads[i][2];
        setcolor(COLOR(46, 110, 50));
        setfillstyle(SOLID_FILL, COLOR(66, 138, 62));
        sector(px, py, 30 + i * 70, 330 + i * 70, pr, pr);
    }
    disc(cx - rx / 2 + 1, cy - ry / 4 - 1, 1, 1, COLOR(250, 170, 200)); // lotus flower

    // Light ripples
    setcolor(COLOR(150, 205, 228));
    arc(cx + rx / 4, cy + ry / 5, 200, 340, 6);
    arc(cx + rx / 4, cy + ry / 5, 210, 330, 10);
    arc(cx - rx / 6, cy - ry / 2 + 6, 20, 160, 7);
}

void drawFountainPlaza(int cx, int cy, int r) {
    Rng rng(7);
    // Circular paved plaza with tile rings
    disc(cx + 3, cy + 3, r + 2, r + 2, LAWN_SHADOW);
    disc(cx, cy, r + 2, r + 2, STONE_DARK);
    disc(cx, cy, r, r, STONE_LIGHT);
    setcolor(COLOR(176, 168, 150));
    circle(cx, cy, r - 8);
    circle(cx, cy, r - 17);
    for (int a = 0; a < 360; a += 20) {
        double rad = a * kPi / 180.0;
        line(cx + static_cast<int>(std::cos(rad) * (r - 17)), cy + static_cast<int>(std::sin(rad) * (r - 17)),
             cx + static_cast<int>(std::cos(rad) * r),        cy + static_cast<int>(std::sin(rad) * r));
    }

    // Basin
    int br = r / 2 + 2;
    disc(cx + 2, cy + 2, br + 3, br + 3, COLOR(140, 134, 118));
    disc(cx, cy, br + 3, br + 3, COLOR(214, 212, 206));
    disc(cx, cy, br, br, COLOR(64, 138, 186));
    disc(cx - 2, cy - 2, br - 7, br - 7, COLOR(82, 158, 204));
    setcolor(COLOR(150, 206, 236));
    circle(cx, cy, br - 4);
    circle(cx, cy, br - 12);

    // Central pedestal and water jets
    disc(cx, cy, 7, 7, COLOR(200, 198, 192));
    disc(cx, cy, 4, 4, COLOR(150, 210, 240));
    disc(cx - 1, cy - 1, 2, 2, WHITE);
    for (int i = 0; i < 16; ++i) {
        double a = i * 2.0 * kPi / 16;
        int d = 10 + rng.next(4);
        putpixel(cx + static_cast<int>(std::cos(a) * d), cy + static_cast<int>(std::sin(a) * d), WHITE);
        putpixel(cx + static_cast<int>(std::cos(a) * (d + 3)), cy + static_cast<int>(std::sin(a) * (d + 3)), COLOR(200, 232, 248));
    }
}

// Octagonal gazebo roof seen from above, shaded per facet
void drawGazebo(int cx, int cy, int r) {
    int pts[16];
    auto octagon = [&](int ox, int oy, int rr) {
        for (int i = 0; i < 8; ++i) {
            double a = kPi / 8 + i * kPi / 4;
            pts[2 * i]     = ox + static_cast<int>(std::cos(a) * rr);
            pts[2 * i + 1] = oy + static_cast<int>(std::sin(a) * rr);
        }
    };
    octagon(cx + 6, cy + 6, r);
    poly(8, pts, LAWN_SHADOW, LAWN_SHADOW);
    octagon(cx, cy, r + 4);
    poly(8, pts, STONE_DARK, STONE_DARK); // deck edge peeking out under the eaves

    for (int i = 0; i < 8; ++i) {
        double a0 = kPi / 8 + i * kPi / 4, a1 = a0 + kPi / 4, am = (a0 + a1) / 2;
        double light = -(std::cos(am) + std::sin(am)) * 0.7071; // facing top-left = lit
        double k = (light + 1.0) / 2.0;
        int col = COLOR(static_cast<int>(104 + 80 * k), static_cast<int>(50 + 44 * k), static_cast<int>(36 + 30 * k));
        int tri[6] = { cx, cy,
                       cx + static_cast<int>(std::cos(a0) * r), cy + static_cast<int>(std::sin(a0) * r),
                       cx + static_cast<int>(std::cos(a1) * r), cy + static_cast<int>(std::sin(a1) * r) };
        poly(3, tri, col, col);
        // Shingle courses
        setcolor(COLOR(static_cast<int>(84 + 60 * k), static_cast<int>(40 + 34 * k), static_cast<int>(28 + 22 * k)));
        for (int s = r / 4; s < r; s += r / 4) {
            line(cx + static_cast<int>(std::cos(a0) * s), cy + static_cast<int>(std::sin(a0) * s),
                 cx + static_cast<int>(std::cos(a1) * s), cy + static_cast<int>(std::sin(a1) * s));
        }
    }
    // Ridge lines
    setcolor(COLOR(70, 34, 24));
    for (int i = 0; i < 8; ++i) {
        double a = kPi / 8 + i * kPi / 4;
        line(cx, cy, cx + static_cast<int>(std::cos(a) * r), cy + static_cast<int>(std::sin(a) * r));
    }
    disc(cx, cy, 3, 3, COLOR(196, 170, 90)); // finial
}

void drawPlayground(int x1, int y1, int x2, int y2) {
    Rng rng(99);
    const int RUBBER = COLOR(172, 84, 62);
    roundBox(x1 - 3 + 3, y1 - 3 + 3, x2 + 3 + 3, y2 + 3 + 3, 16, LAWN_SHADOW);
    roundBox(x1 - 3, y1 - 3, x2 + 3, y2 + 3, 16, COLOR(176, 172, 162)); // concrete edging
    roundBox(x1, y1, x2, y2, 13, RUBBER);                               // safety surface
    for (int i = 0; i < (x2 - x1) * (y2 - y1) / 18; ++i) {
        int px = x1 + 4 + rng.next(x2 - x1 - 8), py = y1 + 4 + rng.next(y2 - y1 - 8);
        putpixel(px, py, rng.next(2) ? COLOR(150, 70, 52) : COLOR(196, 108, 82));
    }

    // Sandbox with wooden frame
    int sx1 = x1 + 12, sy1 = y1 + 12, sx2 = x1 + 62, sy2 = y1 + 56;
    box(sx1 + 2, sy1 + 2, sx2 + 2, sy2 + 2, COLOR(130, 60, 44));
    box(sx1, sy1, sx2, sy2, WOOD_LIGHT);
    box(sx1 + 4, sy1 + 4, sx2 - 4, sy2 - 4, COLOR(228, 208, 154));
    setcolor(WOOD_DARK);
    rectangle(sx1, sy1, sx2, sy2);
    for (int i = 0; i < 70; ++i)
        putpixel(sx1 + 5 + rng.next(sx2 - sx1 - 10), sy1 + 5 + rng.next(sy2 - sy1 - 10),
                 rng.next(2) ? COLOR(206, 184, 128) : COLOR(242, 226, 182));
    disc(sx1 + 16, sy1 + 18, 4, 4, COLOR(220, 40, 40));   // toy bucket
    disc(sx1 + 16, sy1 + 18, 2, 2, COLOR(150, 20, 20));
    setcolor(COLOR(40, 110, 210));                         // toy spade
    line(sx1 + 30, sy1 + 30, sx1 + 38, sy1 + 24);
    disc(sx1 + 38, sy1 + 24, 2, 2, COLOR(40, 110, 210));

    // Slide: tower with pyramid roof + yellow chute
    int tx = x2 - 46, ty = y1 + 10;
    box(tx + 6, ty + 22, tx + 14, ty + 62, COLOR(150, 70, 52));              // chute shadow
    box(tx + 5, ty + 20, tx + 13, ty + 60, COLOR(244, 196, 40));            // chute
    setcolor(COLOR(196, 140, 20));
    line(tx + 5, ty + 20, tx + 5, ty + 60);
    line(tx + 13, ty + 20, tx + 13, ty + 60);
    box(tx + 3, ty + 58, tx + 15, ty + 63, COLOR(244, 196, 40));            // run-out
    int roof[4][6] = {
        { tx, ty, tx + 18, ty, tx + 9, ty + 9 },
        { tx + 18, ty, tx + 18, ty + 18, tx + 9, ty + 9 },
        { tx + 18, ty + 18, tx, ty + 18, tx + 9, ty + 9 },
        { tx, ty + 18, tx, ty, tx + 9, ty + 9 } };
    const int roofCols[4] = { COLOR(70, 140, 230), COLOR(40, 96, 190), COLOR(30, 74, 160), COLOR(56, 120, 214) };
    for (int i = 0; i < 4; ++i) poly(3, roof[i], roofCols[i], roofCols[i]);
    // Ladder
    setcolor(COLOR(90, 96, 104));
    line(tx + 24, ty + 4, tx + 36, ty + 4);
    line(tx + 24, ty + 14, tx + 36, ty + 14);
    for (int lx = tx + 26; lx <= tx + 36; lx += 3) line(lx, ty + 4, lx, ty + 14);

    // Swing set (A-frame top bar seen from above, two seats with worn patches)
    int swY = y2 - 28, swX1 = x1 + 20, swX2 = x1 + 92;
    disc(swX1 + 22, swY, 9, 5, COLOR(136, 64, 48));
    disc(swX1 + 50, swY, 9, 5, COLOR(136, 64, 48));
    setcolor(COLOR(60, 90, 140));
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(swX1, swY, swX2, swY);
    line(swX1, swY - 10, swX1, swY + 10);
    line(swX2, swY - 10, swX2, swY + 10);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
    box(swX1 + 17, swY - 2, swX1 + 27, swY + 2, COLOR(30, 30, 34));
    box(swX1 + 45, swY - 2, swX1 + 55, swY + 2, COLOR(30, 30, 34));

    // Merry-go-round
    int mx = x2 - 26, my = y2 - 30;
    disc(mx + 2, my + 2, 14, 14, COLOR(130, 60, 44));
    disc(mx, my, 14, 14, COLOR(214, 60, 52));
    setcolor(COLOR(250, 220, 60));
    for (int a = 0; a < 360; a += 60) {
        double rad = a * kPi / 180.0;
        line(mx, my, mx + static_cast<int>(std::cos(rad) * 13), my + static_cast<int>(std::sin(rad) * 13));
    }
    disc(mx, my, 3, 3, COLOR(200, 200, 205));

    // Spring rider
    disc(x1 + 112, y1 + 92, 6, 4, COLOR(60, 170, 90));
    disc(x1 + 116, y1 + 90, 2, 2, COLOR(250, 220, 60));
}

// Green street-name style board for the approach labels.
// (x, y) is the top-left corner, or the top-right corner when alignRight is set.
void drawSignBoard(int x, int y, const char* l1, const char* l2, bool alignRight = false) {
    const int bg = COLOR(0, 98, 60);
    const int pad = 8;
    int th = textheight(const_cast<char*>(l1));
    int w = std::max(textwidth(const_cast<char*>(l1)), textwidth(const_cast<char*>(l2))) + pad * 2;
    int h = th * 2 + pad + 4;
    if (alignRight) x -= w;
    box(x + 3, y + 3, x + w + 3, y + h + 3, COLOR(30, 50, 30));
    box(x, y, x + w, y + h, bg);
    setcolor(COLOR(235, 240, 235));
    rectangle(x + 2, y + 2, x + w - 2, y + h - 2);
    setbkcolor(bg);
    outtextxy(x + pad, y + pad / 2 + 1, const_cast<char*>(l1));
    outtextxy(x + pad, y + pad / 2 + 1 + th, const_cast<char*>(l2));
}

} // namespace

// =====================================================================
//  GREEN AREAS: four landscaped urban parks around the intersection
// =====================================================================
void HUD::drawGreenAreas() const {
    const int SW = 20;
    const int L_X2 = ROAD_LEFT - SW - 1;    // Right edge of left-side lawns   (254)
    const int R_X1 = ROAD_RIGHT + SW + 1;   // Left edge of right-side lawns   (456)
    const int R_X2 = HUD_X_START - 6;       // Right edge of right-side lawns  (704)
    const int T_Y2 = ROAD_TOP - SW - 1;     // Bottom edge of top lawns        (274)
    const int B_Y1 = ROAD_BOTTOM + SW + 1;  // Top edge of bottom lawns        (476)
    const int B_Y2 = SCREEN_HEIGHT - 1;

    const int BLOOM_WARM[4]  = { COLOR(230, 50, 60), COLOR(250, 200, 40), COLOR(250, 130, 40), COLOR(255, 240, 220) };
    const int BLOOM_COOL[4]  = { COLOR(170, 90, 210), COLOR(240, 120, 180), COLOR(255, 255, 255), COLOR(120, 140, 240) };
    const int BLOOM_MIXED[5] = { COLOR(230, 50, 60), COLOR(250, 200, 40), COLOR(170, 90, 210), COLOR(255, 255, 255), COLOR(240, 120, 180) };

    // ---------------- Lawns ----------------
    drawLawn(0, 0, L_X2, T_Y2, false, 11);
    drawLawn(R_X1, 0, R_X2, T_Y2, true, 23);
    drawLawn(0, B_Y1, L_X2, B_Y2, true, 37);
    drawLawn(R_X1, B_Y1, R_X2, B_Y2, false, 41);

    // ================= TOP-LEFT: City park with pond =================
    drawHedge(L_X2 - 12, 0, L_X2 - 3, 120, 101);            // Curb-side hedges (gaps = park entrances)
    drawHedge(L_X2 - 12, 150, L_X2 - 3, T_Y2 - 3, 102);
    drawHedge(0, T_Y2 - 12, 90, T_Y2 - 3, 103);
    drawHedge(120, T_Y2 - 12, L_X2 - 3, T_Y2 - 3, 104);

    drawFootpath(105, T_Y2 + 8, 150, 190, L_X2 + 8, 135, 9, 105);   // Main path between entrances
    drawFootpath(165, 199, 145, 178, 126, 166, 6, 106);              // Spur to the pond
    drawPond(78, 128, 52, 36, 107);
    drawBench(92, 186, true);
    drawBench(150, 152, false);
    drawFlowerBed(162, 92, 17, 11, 108, BLOOM_WARM, 4);

    drawDeciduousTree(38, 40, 22, OAK, 110);
    drawConifer(100, 34, 14);
    drawDeciduousTree(218, 78, 22, MAPLE, 111);
    drawDeciduousTree(40, 228, 20, LINDEN, 112);
    drawShrub(208, 226, 8, HEDGE, 113, COLOR(240, 120, 180));
    drawShrub(70, 258, 7, HEDGE, 114);
    drawShrub(18, 160, 8, HEDGE, 115, COLOR(250, 240, 240));

    // ================= TOP-RIGHT: Fountain plaza =================
    const int fx = 590, fy = 125;
    drawHedge(R_X1 + 3, 0, R_X1 + 12, 112, 201);
    drawHedge(R_X1 + 3, 140, R_X1 + 12, T_Y2 - 3, 202);
    drawHedge(R_X1 + 3, T_Y2 - 12, R_X2, T_Y2 - 3, 203);

    drawFootpath(R_X1 - 8, fy, (R_X1 + fx) / 2, fy, fx, fy, 10, 204);    // West entrance
    drawFootpath(fx, -8, fx, fy / 2, fx, fy, 10, 205);                    // North promenade
    drawFootpath(fx, fy, (fx + R_X2) / 2, fy, R_X2 + 8, fy, 10, 206);     // East promenade
    drawFountainPlaza(fx, fy, 58);
    drawBench(fx - 33, fy - 37, true);
    drawBench(fx + 33, fy - 37, true);
    drawBench(fx - 33, fy + 37, true);
    drawBench(fx + 33, fy + 37, true);

    drawFlowerBed(522, 208, 24, 15, 207, BLOOM_COOL, 4);
    drawFlowerBed(622, 212, 30, 13, 208, BLOOM_MIXED, 5);
    drawFlowerBed(520, 45, 18, 12, 209, BLOOM_WARM, 4);

    drawDeciduousTree(490, 92, 18, LINDEN, 210);
    drawDeciduousTree(676, 32, 22, OAK, 211);
    drawDeciduousTree(678, 196, 20, MAPLE, 212);
    drawConifer(648, 70, 12);
    drawShrub(492, 160, 8, HEDGE, 213, COLOR(250, 200, 40));

    // ================= BOTTOM-LEFT: Children's playground =================
    drawHedge(L_X2 - 12, B_Y1 + 3, L_X2 - 3, 690, 301);
    drawHedge(L_X2 - 12, 716, L_X2 - 3, B_Y2, 302);
    drawHedge(0, B_Y1 + 3, L_X2 - 3, B_Y1 + 12, 303);

    drawFootpath(L_X2 + 8, 703, 205, 706, 160, 686, 9, 304);
    drawPlayground(22, 548, 172, 684);
    drawBench(204, 640, false);
    drawBench(70, 706, true);

    drawDeciduousTree(214, 596, 18, OAK, 310);
    drawDeciduousTree(30, 726, 20, MAPLE, 311);
    drawDeciduousTree(128, 732, 15, LINDEN, 312);
    drawConifer(222, 734, 12);
    drawShrub(160, 532, 7, HEDGE, 313, COLOR(250, 240, 240));
    drawShrub(232, 680, 6, HEDGE, 314);
    drawShrub(190, 680, 6, HEDGE, 315, COLOR(230, 50, 60));

    // ================= BOTTOM-RIGHT: Picnic lawn with gazebo =================
    drawHedge(R_X1 + 3, B_Y1 + 3, R_X1 + 12, 600, 401);
    drawHedge(R_X1 + 3, 626, R_X1 + 12, B_Y2, 402);
    drawHedge(R_X1 + 3, B_Y1 + 3, R_X2, B_Y1 + 12, 403);

    drawFootpath(R_X1 - 8, 613, 530, 622, 600, 606, 9, 404);
    drawFlowerBed(620, 600, 40, 40, 405, BLOOM_MIXED, 5);   // Planting ring around the gazebo
    drawGazebo(620, 600, 30);
    drawPicnicTable(528, 670);
    drawPicnicTable(610, 682);
    drawFlowerBed(530, 560, 18, 11, 406, BLOOM_WARM, 4);

    drawDeciduousTree(572, 528, 18, MAPLE, 410);
    drawDeciduousTree(684, 512, 20, OAK, 411);
    drawConifer(688, 640, 14);
    drawDeciduousTree(684, 722, 20, LINDEN, 412);
    drawShrub(495, 650, 7, HEDGE, 413, COLOR(240, 120, 180));
    drawShrub(640, 728, 7, HEDGE, 414);
}

void HUD::drawRoadSigns() const {
    // Boards sit on the lawns, clear of the hedges, paths and signal posts
    const int boardH = textheight(const_cast<char*>("A")) * 2 + 12;
    drawSignBoard(ROAD_LEFT - 34, 8, "[TOP APPROACH]", "SOUTHBOUND v", true);
    drawSignBoard(ROAD_RIGHT + 30, SCREEN_HEIGHT - boardH - 6, "[BOT APPROACH]", "NORTHBOUND ^");
    drawSignBoard(10, ROAD_BOTTOM + 26, "[LEFT APPROACH]", "EASTBOUND >");
    drawSignBoard(HUD_X_START - 10, ROAD_TOP - 20 - boardH - 8, "[RIGHT APPROACH]", "WESTBOUND <", true);
}

void HUD::drawStreetLight(int x, int y, bool onLeft) const {
    // Base & Pole
    setcolor(COLOR(70, 75, 85));
    setfillstyle(SOLID_FILL, COLOR(50, 55, 65));
    fillellipse(x, y, 4, 4);

    // Lamp Head
    int headX = onLeft ? x + 10 : x - 10;
    line(x, y, headX, y - 6);
    setfillstyle(SOLID_FILL, COLOR(250, 230, 140));
    fillellipse(headX, y - 6, 3, 3);
}

void HUD::drawDirectionArrow(int x, int y, Direction dir) const {
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, WHITE);

    // Draw painted white lane arrows on asphalt
    switch (dir) {
        case DIR_SOUTH: { // Arrow pointing DOWN (Top approach)
            line(x, y - 14, x, y + 10);
            line(x - 1, y - 14, x - 1, y + 10);
            line(x + 1, y - 14, x + 1, y + 10);
            int pts[6] = { x, y + 16, x - 6, y + 6, x + 6, y + 6 };
            fillpoly(3, pts);
            break;
        }
        case DIR_NORTH: { // Arrow pointing UP (Bottom approach)
            line(x, y + 14, x, y - 10);
            line(x - 1, y + 14, x - 1, y - 10);
            line(x + 1, y + 14, x + 1, y - 10);
            int pts[6] = { x, y - 16, x - 6, y - 6, x + 6, y - 6 };
            fillpoly(3, pts);
            break;
        }
        case DIR_EAST: { // Arrow pointing RIGHT (Left approach)
            line(x - 14, y, x + 10, y);
            line(x - 14, y - 1, x + 10, y - 1);
            line(x - 14, y + 1, x + 10, y + 1);
            int pts[6] = { x + 16, y, x + 6, y - 6, x + 6, y + 6 };
            fillpoly(3, pts);
            break;
        }
        case DIR_WEST: { // Arrow pointing LEFT (Right approach)
            line(x + 14, y, x - 10, y);
            line(x + 14, y - 1, x - 10, y - 1);
            line(x + 14, y + 1, x - 10, y + 1);
            int pts[6] = { x - 16, y, x - 6, y - 6, x - 6, y + 6 };
            fillpoly(3, pts);
            break;
        }
    }
}

void HUD::drawScenery() const {
    // The background is static and detailed, so render it once and blit the
    // cached bitmap on every following frame.
    static void* cache = nullptr;
    static bool cacheTried = false;

    if (cache) {
        putimage(0, 0, cache, COPY_PUT);
        return;
    }

    renderStaticScenery();

    if (!cacheTried) {
        cacheTried = true;
        unsigned size = imagesize(0, 0, HUD_X_START - 5, SCREEN_HEIGHT - 1);
        cache = std::malloc(size);
        if (cache) getimage(0, 0, HUD_X_START - 5, SCREEN_HEIGHT - 1, cache);
    }
}

void HUD::renderStaticScenery() const {
    // ================= 1. LANDSCAPED GREEN AREAS =================
    drawGreenAreas();

    // ================= 2. CONCRETE SIDEWALKS & CURBS =================
    const int SW_WIDTH = 20;
    setfillstyle(SOLID_FILL, COLOR(105, 112, 122)); // Concrete sidewalk gray

    // Vertical sidewalks
    bar(ROAD_LEFT - SW_WIDTH, 0, ROAD_LEFT, SCREEN_HEIGHT);
    bar(ROAD_RIGHT, 0, ROAD_RIGHT + SW_WIDTH, SCREEN_HEIGHT);

    // Horizontal sidewalks
    bar(0, ROAD_TOP - SW_WIDTH, HUD_X_START - 5, ROAD_TOP);
    bar(0, ROAD_BOTTOM, HUD_X_START - 5, ROAD_BOTTOM + SW_WIDTH);

    // Curb edge borders
    setcolor(COLOR(175, 180, 190));
    line(ROAD_LEFT - SW_WIDTH, 0, ROAD_LEFT - SW_WIDTH, SCREEN_HEIGHT);
    line(ROAD_RIGHT + SW_WIDTH, 0, ROAD_RIGHT + SW_WIDTH, SCREEN_HEIGHT);
    line(0, ROAD_TOP - SW_WIDTH, HUD_X_START - 5, ROAD_TOP - SW_WIDTH);
    line(0, ROAD_BOTTOM + SW_WIDTH, HUD_X_START - 5, ROAD_BOTTOM + SW_WIDTH);

    // Sidewalk tiles
    setcolor(COLOR(85, 92, 100));
    for (int y = 15; y < SCREEN_HEIGHT; y += 30) {
        if (y < ROAD_TOP - SW_WIDTH || y > ROAD_BOTTOM + SW_WIDTH) {
            line(ROAD_LEFT - SW_WIDTH, y, ROAD_LEFT, y);
            line(ROAD_RIGHT, y, ROAD_RIGHT + SW_WIDTH, y);
        }
    }
    for (int x = 15; x < HUD_X_START - 5; x += 30) {
        if (x < ROAD_LEFT - SW_WIDTH || x > ROAD_RIGHT + SW_WIDTH) {
            line(x, ROAD_TOP - SW_WIDTH, x, ROAD_TOP);
            line(x, ROAD_BOTTOM, x, ROAD_BOTTOM + SW_WIDTH);
        }
    }

    // Street Lamps on sidewalk corners
    drawStreetLight(ROAD_LEFT - 8, ROAD_TOP - 8, true);
    drawStreetLight(ROAD_RIGHT + 8, ROAD_TOP - 8, false);
    drawStreetLight(ROAD_LEFT - 8, ROAD_BOTTOM + 8, true);
    drawStreetLight(ROAD_RIGHT + 8, ROAD_BOTTOM + 8, false);

    // ================= 3. ASPHALT ROADWAY =================
    setfillstyle(SOLID_FILL, COLOR(38, 44, 52)); // Deep asphalt slate
    bar(ROAD_LEFT, 0, ROAD_RIGHT, SCREEN_HEIGHT);
    bar(0, ROAD_TOP, HUD_X_START - 5, ROAD_BOTTOM);
    bar(ROAD_LEFT, ROAD_TOP, ROAD_RIGHT, ROAD_BOTTOM); // Intersection center

    // Road Edge White Boundary Lines
    setcolor(COLOR(230, 235, 240));
    line(ROAD_LEFT, 0, ROAD_LEFT, ROAD_TOP);
    line(ROAD_LEFT, ROAD_BOTTOM, ROAD_LEFT, SCREEN_HEIGHT);
    line(ROAD_RIGHT, 0, ROAD_RIGHT, ROAD_TOP);
    line(ROAD_RIGHT, ROAD_BOTTOM, ROAD_RIGHT, SCREEN_HEIGHT);

    line(0, ROAD_TOP, ROAD_LEFT, ROAD_TOP);
    line(ROAD_RIGHT, ROAD_TOP, HUD_X_START - 5, ROAD_TOP);
    line(0, ROAD_BOTTOM, ROAD_LEFT, ROAD_BOTTOM);
    line(ROAD_RIGHT, ROAD_BOTTOM, HUD_X_START - 5, ROAD_BOTTOM);

    // ================= 4. REAL-WORLD OPEN INTERSECTION CROSSROAD =================
    // Clean seamless dark asphalt junction (authentic real-world crossroad)
    setfillstyle(SOLID_FILL, COLOR(38, 44, 52));
    bar(ROAD_LEFT, ROAD_TOP, ROAD_RIGHT, ROAD_BOTTOM);

    // ================= 5. DOUBLE YELLOW CENTERLINES =================
    setcolor(COLOR(255, 205, 10));
    // Vertical Centerlines (Terminate cleanly before crosswalk)
    line(CENTER_X - 2, 0, CENTER_X - 2, STOP_LINE_SOUTH - 18);
    line(CENTER_X + 2, 0, CENTER_X + 2, STOP_LINE_SOUTH - 18);
    line(CENTER_X - 2, STOP_LINE_NORTH + 18, CENTER_X - 2, SCREEN_HEIGHT);
    line(CENTER_X + 2, STOP_LINE_NORTH + 18, CENTER_X + 2, SCREEN_HEIGHT);

    // Horizontal Centerlines (Terminate cleanly before crosswalk)
    line(0, CENTER_Y - 2, STOP_LINE_EAST - 18, CENTER_Y - 2);
    line(0, CENTER_Y + 2, STOP_LINE_EAST - 18, CENTER_Y + 2);
    line(STOP_LINE_WEST + 18, CENTER_Y - 2, HUD_X_START - 5, CENTER_Y - 2);
    line(STOP_LINE_WEST + 18, CENTER_Y + 2, HUD_X_START - 5, CENTER_Y + 2);

    // ================= 6. ZEBRA PEDESTRIAN CROSSINGS =================
    setfillstyle(SOLID_FILL, COLOR(245, 245, 245));
    // South Approach Zebra (Top)
    for (int x = ROAD_LEFT + 6; x < ROAD_RIGHT - 6; x += 18) {
        bar(x, STOP_LINE_SOUTH - 14, x + 10, STOP_LINE_SOUTH - 2);
    }
    // North Approach Zebra (Bottom)
    for (int x = ROAD_LEFT + 6; x < ROAD_RIGHT - 6; x += 18) {
        bar(x, STOP_LINE_NORTH + 2, x + 10, STOP_LINE_NORTH + 14);
    }
    // East Approach Zebra (Left)
    for (int y = ROAD_TOP + 6; y < ROAD_BOTTOM - 6; y += 18) {
        bar(STOP_LINE_EAST - 14, y, STOP_LINE_EAST - 2, y + 10);
    }
    // West Approach Zebra (Right)
    for (int y = ROAD_TOP + 6; y < ROAD_BOTTOM - 6; y += 18) {
        bar(STOP_LINE_WEST + 2, y, STOP_LINE_WEST + 14, y + 10);
    }

    // ================= 7. SOLID STOP LINES =================
    setcolor(WHITE);
    // South Stop Line
    for (int t = 0; t < 3; ++t) line(ROAD_LEFT, STOP_LINE_SOUTH - 15 - t, CENTER_X, STOP_LINE_SOUTH - 15 - t);
    // North Stop Line
    for (int t = 0; t < 3; ++t) line(CENTER_X, STOP_LINE_NORTH + 15 + t, ROAD_RIGHT, STOP_LINE_NORTH + 15 + t);
    // East Stop Line (Left road, Bottom lane)
    for (int t = 0; t < 3; ++t) line(STOP_LINE_EAST - 15 - t, CENTER_Y, STOP_LINE_EAST - 15 - t, ROAD_BOTTOM);
    // West Stop Line (Right road, Top lane)
    for (int t = 0; t < 3; ++t) line(STOP_LINE_WEST + 15 + t, ROAD_TOP, STOP_LINE_WEST + 15 + t, CENTER_Y);

    // ================= 8. LANE DIRECTIONAL ARROWS ON ALL ROADS =================
    // 1. Approach Lanes (Placed safely behind the induction sensor box)
    drawDirectionArrow(LANE_SOUTH_X, 55, DIR_SOUTH);  // Top approach (Behind sensor)
    drawDirectionArrow(LANE_NORTH_X, 695, DIR_NORTH); // Bottom approach (Behind sensor)
    drawDirectionArrow(45, LANE_EAST_Y, DIR_EAST);    // Left approach (Behind sensor)
    drawDirectionArrow(665, LANE_WEST_Y, DIR_WEST);   // Right approach (Behind sensor)

    // 2. Departure / Exit Lanes (Indicating outgoing traffic flow)
    drawDirectionArrow(LANE_NORTH_X, 130, DIR_NORTH); // Top exit lane
    drawDirectionArrow(LANE_SOUTH_X, 620, DIR_SOUTH); // Bottom exit lane
    drawDirectionArrow(130, LANE_WEST_Y, DIR_WEST);   // Left exit lane
    drawDirectionArrow(580, LANE_EAST_Y, DIR_EAST);   // Right exit lane

    // ================= 9. ROADWAY ORIENTATION SIGN BOARDS =================
    drawRoadSigns();
}

void HUD::drawDetectionZones(const IntersectionManager& manager) const {
    // ================= REALISTIC ROAD INDUCTION SENSOR LOOPS =================
    // Realistic saw-cut inductive loops embedded into the asphalt with chamfered corners
    
    char sTag[64];

    // Helper lambda to draw a realistic pavement-embedded induction loop
    auto drawLoop = [](int x1, int y1, int x2, int y2, int count) {
        int loopColor = (count > 0) ? COLOR(0, 225, 255) : COLOR(58, 68, 80);
        setcolor(loopColor);

        const int c = 8; // 45-degree chamfer cut at corners

        // Outer chamfered saw-cut rectangle
        line(x1 + c, y1, x2 - c, y1);
        line(x2, y1 + c, x2, y2 - c);
        line(x2 - c, y2, x1 + c, y2);
        line(x1, y2 - c, x1, y1 + c);

        // Corner diagonal cuts
        line(x1, y1 + c, x1 + c, y1);
        line(x2 - c, y1, x2, y1 + c);
        line(x2, y2 - c, x2 - c, y2);
        line(x1 + c, y2, x1, y2 - c);

        // Inner secondary coil wire
        const int m = 4;
        line(x1 + c + m, y1 + m, x2 - c - m, y1 + m);
        line(x2 - m, y1 + c + m, x2 - m, y2 - c - m);
        line(x2 - c - m, y2 - m, x1 + c + m, y2 - m);
        line(x1 + m, y2 - c - m, x1 + m, y1 + c + m);
    };

    // 1. Southbound Approach Sensor (Top Road, Left Lane)
    int s_x1 = ROAD_LEFT + 6, s_y1 = STOP_LINE_SOUTH - DETECTION_ZONE_LENGTH;
    int s_x2 = CENTER_X - 6,  s_y2 = STOP_LINE_SOUTH - 22;
    drawLoop(s_x1, s_y1, s_x2, s_y2, manager.countSouth);

    // 2. Northbound Approach Sensor (Bottom Road, Right Lane)
    int n_x1 = CENTER_X + 6,  n_y1 = STOP_LINE_NORTH + 22;
    int n_x2 = ROAD_RIGHT - 6, n_y2 = STOP_LINE_NORTH + DETECTION_ZONE_LENGTH;
    drawLoop(n_x1, n_y1, n_x2, n_y2, manager.countNorth);

    // 3. Eastbound Approach Sensor (Left Road, Bottom Lane)
    int e_x1 = STOP_LINE_EAST - DETECTION_ZONE_LENGTH, e_y1 = CENTER_Y + 6;
    int e_x2 = STOP_LINE_EAST - 22,                   e_y2 = ROAD_BOTTOM - 6;
    drawLoop(e_x1, e_y1, e_x2, e_y2, manager.countEast);

    // 4. Westbound Approach Sensor (Right Road, Top Lane)
    int w_x1 = STOP_LINE_WEST + 22,                   w_y1 = ROAD_TOP + 6;
    int w_x2 = STOP_LINE_WEST + DETECTION_ZONE_LENGTH, w_y2 = CENTER_Y - 6;
    drawLoop(w_x1, w_y1, w_x2, w_y2, manager.countWest);

    // Pavement-edge smart sensor status tags
    setbkcolor(COLOR(22, 34, 48));

    // South Tag
    setcolor(manager.countSouth > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countSouth);
    outtextxy(s_x1 + 4, s_y1 + 4, sTag);

    // North Tag
    setcolor(manager.countNorth > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countNorth);
    outtextxy(n_x1 + 4, n_y2 - 16, sTag);

    // East Tag
    setcolor(manager.countEast > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countEast);
    outtextxy(e_x1 + 4, e_y1 + 4, sTag);

    // West Tag
    setcolor(manager.countWest > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countWest);
    outtextxy(w_x2 - 76, w_y1 + 4, sTag);
}

void HUD::drawTrafficLightPost(int poleX, int poleY, int boxX, int boxY,
                              bool isRed, bool isYellow, bool isGreen,
                              const char* directionLabel, float timeRemaining, bool isHorizontal) const {
    // 1. Steel Support Pole & Cantilever Arm
    setcolor(COLOR(100, 108, 120));
    setfillstyle(SOLID_FILL, COLOR(70, 76, 85));
    fillellipse(poleX, poleY, 5, 5); // Pole Base

    int armTargetX = isHorizontal ? (boxX + 34) : (boxX + 13);
    int armTargetY = isHorizontal ? (boxY + 13) : (boxY + 34);
    line(poleX, poleY, armTargetX, armTargetY); // Arm

    // 2. High-Visibility Signal Housing Box
    const int boxW = isHorizontal ? 68 : 26;
    const int boxH = isHorizontal ? 26 : 68;

    // Outer Yellow Backplate / Border
    setfillstyle(SOLID_FILL, COLOR(255, 204, 0));
    bar(boxX - 3, boxY - 3, boxX + boxW + 3, boxY + boxH + 3);

    // Inner Dark Housing
    setfillstyle(SOLID_FILL, COLOR(18, 22, 28));
    bar(boxX, boxY, boxX + boxW, boxY + boxH);
    setcolor(COLOR(120, 130, 145));
    rectangle(boxX, boxY, boxX + boxW, boxY + boxH);

    // 3. Lamps (Red, Yellow, Green)
    int rX, rY, yX, yY, gX, gY;
    if (isHorizontal) {
        rX = boxX + 13; rY = boxY + 13;
        yX = boxX + 34; yY = boxY + 13;
        gX = boxX + 55; gY = boxY + 13;
    } else {
        rX = boxX + 13; rY = boxY + 13;
        yX = boxX + 13; yY = boxY + 34;
        gX = boxX + 13; gY = boxY + 55;
    }

    const int LAMP_RAD = 7;

    // --- RED LAMP ---
    if (isRed) {
        setfillstyle(SOLID_FILL, COLOR(120, 20, 20));
        fillellipse(rX, rY, LAMP_RAD + 2, LAMP_RAD + 2);
        setfillstyle(SOLID_FILL, COLOR(255, 30, 30));
        fillellipse(rX, rY, LAMP_RAD, LAMP_RAD);
        setfillstyle(SOLID_FILL, COLOR(255, 170, 170));
        fillellipse(rX - 2, rY - 2, 2, 2);
    } else {
        setfillstyle(SOLID_FILL, COLOR(65, 15, 15));
        fillellipse(rX, rY, LAMP_RAD, LAMP_RAD);
    }

    // --- YELLOW LAMP ---
    if (isYellow) {
        setfillstyle(SOLID_FILL, COLOR(130, 110, 15));
        fillellipse(yX, yY, LAMP_RAD + 2, LAMP_RAD + 2);
        setfillstyle(SOLID_FILL, COLOR(255, 215, 10));
        fillellipse(yX, yY, LAMP_RAD, LAMP_RAD);
        setfillstyle(SOLID_FILL, COLOR(255, 245, 170));
        fillellipse(yX - 2, yY - 2, 2, 2);
    } else {
        setfillstyle(SOLID_FILL, COLOR(65, 55, 10));
        fillellipse(yX, yY, LAMP_RAD, LAMP_RAD);
    }

    // --- GREEN LAMP ---
    if (isGreen) {
        setfillstyle(SOLID_FILL, COLOR(15, 120, 45));
        fillellipse(gX, gY, LAMP_RAD + 2, LAMP_RAD + 2);
        setfillstyle(SOLID_FILL, COLOR(30, 245, 90));
        fillellipse(gX, gY, LAMP_RAD, LAMP_RAD);
        setfillstyle(SOLID_FILL, COLOR(180, 255, 205));
        fillellipse(gX - 2, gY - 2, 2, 2);
    } else {
        setfillstyle(SOLID_FILL, COLOR(10, 60, 25));
        fillellipse(gX, gY, LAMP_RAD, LAMP_RAD);
    }

    // 4. Direction & Action Badge (Placed right below the housing)
    int badgeW = 86;
    int badgeH = 22;
    int badgeX = boxX + (boxW / 2) - (badgeW / 2);
    int badgeY = boxY + boxH + 4;
    if (badgeX < 5) badgeX = 5;

    int badgeBg = isRed ? COLOR(160, 20, 20) : (isYellow ? COLOR(160, 130, 10) : COLOR(20, 140, 50));
    setfillstyle(SOLID_FILL, badgeBg);
    bar(badgeX, badgeY, badgeX + badgeW, badgeY + badgeH);
    setcolor(WHITE);
    rectangle(badgeX, badgeY, badgeX + badgeW, badgeY + badgeH);

    setbkcolor(badgeBg);
    setcolor(WHITE);
    char bText[64];
    if (isRed) {
        std::snprintf(bText, sizeof(bText), "%s:STOP", directionLabel);
    } else if (isYellow) {
        std::snprintf(bText, sizeof(bText), "%s:WAIT", directionLabel);
    } else {
        std::snprintf(bText, sizeof(bText), "%s:GO %.0fs", directionLabel, std::max(0.0f, timeRemaining));
    }
    outtextxy(badgeX + 3, badgeY + 4, bText);
}

void HUD::drawTrafficLights(const TrafficLightController& lightController) const {
    bool nsGreen  = lightController.isNorthSouthGreen();
    bool nsYellow = lightController.isNorthSouthYellow();
    bool nsRed    = !nsGreen && !nsYellow;

    bool ewGreen  = lightController.isEastWestGreen();
    bool ewYellow = lightController.isEastWestYellow();
    bool ewRed    = !ewGreen && !ewYellow;

    float remTime = lightController.timeRemaining;

    // =========================================================================
    // 4 SEPARATE CORNERS - SPECIFIC HORIZONTAL / VERTICAL ORIENTATIONS:
    // South: Horizontal | North: Horizontal | East: Vertical | West: Vertical
    // =========================================================================

    // 1. TOP-LEFT CORNER: SOUTHBOUND Signal (Horizontal)
    drawTrafficLightPost(ROAD_LEFT - 12, STOP_LINE_SOUTH - 20,
                         ROAD_LEFT - 85, STOP_LINE_SOUTH - 65,
                         nsRed, nsYellow, nsGreen, "SOUTH", remTime, true);

    // 2. BOTTOM-LEFT CORNER: EASTBOUND Signal (Vertical)
    drawTrafficLightPost(STOP_LINE_EAST - 20, ROAD_BOTTOM + 12,
                         STOP_LINE_EAST - 70, ROAD_BOTTOM + 15,
                         ewRed, ewYellow, ewGreen, "EAST", remTime, false);

    // 3. BOTTOM-RIGHT CORNER: NORTHBOUND Signal (Horizontal)
    drawTrafficLightPost(ROAD_RIGHT + 12, STOP_LINE_NORTH + 20,
                         ROAD_RIGHT + 18, STOP_LINE_NORTH + 25,
                         nsRed, nsYellow, nsGreen, "NORTH", remTime, true);

    // 4. TOP-RIGHT CORNER: WESTBOUND Signal (Vertical)
    drawTrafficLightPost(STOP_LINE_WEST + 20, ROAD_TOP - 12,
                         STOP_LINE_WEST + 25, ROAD_TOP - 100,
                         ewRed, ewYellow, ewGreen, "WEST", remTime, false);
}

void HUD::drawDeadlockOverlay(const IntersectionManager& manager) const {
    if (manager.isDeadlocked) {
        // Flashing Warning Border around the intersection box
        setcolor(COLOR(255, 40, 40));
        rectangle(ROAD_LEFT - 2, ROAD_TOP - 2, ROAD_RIGHT + 2, ROAD_BOTTOM + 2);
        rectangle(ROAD_LEFT - 4, ROAD_TOP - 4, ROAD_RIGHT + 4, ROAD_BOTTOM + 4);
        setcolor(COLOR(255, 220, 0));
        rectangle(ROAD_LEFT - 6, ROAD_TOP - 6, ROAD_RIGHT + 6, ROAD_BOTTOM + 6);

        // Center Gridlock Alert Label
        setfillstyle(SOLID_FILL, COLOR(200, 20, 20));
        bar(CENTER_X - 65, CENTER_Y - 14, CENTER_X + 65, CENTER_Y + 14);
        setcolor(WHITE);
        rectangle(CENTER_X - 65, CENTER_Y - 14, CENTER_X + 65, CENTER_Y + 14);
        setbkcolor(COLOR(200, 20, 20));
        outtextxy(CENTER_X - 58, CENTER_Y - 6, (char*)"! GRIDLOCK DETECTED !");
        setbkcolor(COLOR(15, 22, 32));
    }
}

void HUD::drawDashboard(const IntersectionManager& manager, float fps, bool isPaused) const {
    int panelLeft   = HUD_X_START;
    int panelTop    = 10;
    int panelRight  = SCREEN_WIDTH - 10;
    int panelBottom = SCREEN_HEIGHT - 10;

    // Panel Outer Background (Dark modern glassmorphic card)
    setfillstyle(SOLID_FILL, COLOR(15, 22, 32));
    bar(panelLeft, panelTop, panelRight, panelBottom);

    // Border Accent
    int borderColor = manager.isDeadlocked ? COLOR(240, 60, 60) : COLOR(45, 130, 215);
    setcolor(borderColor);
    rectangle(panelLeft, panelTop, panelRight, panelBottom);
    rectangle(panelLeft + 2, panelTop + 2, panelRight - 2, panelBottom - 2);

    setbkcolor(COLOR(15, 22, 32));
    char buf[128];

    // ================= HEADER TITLE =================
    setcolor(COLOR(240, 245, 255));
    outtextxy(panelLeft + 14, panelTop + 14, (char*)"SMART TRAFFIC CONTROLLER");

    setcolor(COLOR(120, 165, 210));
    outtextxy(panelLeft + 14, panelTop + 32, (char*)"Adaptive Density & Deadlock AI");

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, panelTop + 54, panelRight - 10, panelTop + 54);

    // ================= 1. SYSTEM CONTROL MODE & DEADLOCK STATUS =================
    int modeY = panelTop + 62;
    if (manager.lightController.mode == MODE_ADAPTIVE) {
        setfillstyle(SOLID_FILL, COLOR(18, 100, 48));
        bar(panelLeft + 12, modeY, panelRight - 12, modeY + 26);
        setcolor(COLOR(110, 255, 160));
        outtextxy(panelLeft + 25, modeY + 5, (char*)"[ MODE: ADAPTIVE DENSITY AI ]");
    } else {
        setfillstyle(SOLID_FILL, COLOR(130, 75, 12));
        bar(panelLeft + 12, modeY, panelRight - 12, modeY + 26);
        setcolor(COLOR(255, 205, 110));
        outtextxy(panelLeft + 30, modeY + 5, (char*)"[ MODE: FIXED TIMER (8.0s) ]");
    }

    // Deadlock Status Banner
    int deadY = modeY + 34;
    if (manager.isDeadlocked) {
        setfillstyle(SOLID_FILL, COLOR(180, 25, 25));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(255, 255, 100));
        outtextxy(panelLeft + 20, deadY + 5, (char*)"[ ! DEADLOCK DETECTED ! ]");
    } else if (manager.resolvingDeadlock) {
        setfillstyle(SOLID_FILL, COLOR(25, 110, 165));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(160, 240, 255));
        outtextxy(panelLeft + 18, deadY + 5, (char*)"[ RESOLVING: PRIORITY WAVE ]");
    } else if (manager.starvationDetected) {
        setfillstyle(SOLID_FILL, COLOR(155, 95, 15));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(255, 220, 120));
        outtextxy(panelLeft + 18, deadY + 5, (char*)"[ WARNING: STARVATION QUEUE ]");
    } else {
        setfillstyle(SOLID_FILL, COLOR(24, 48, 75));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(120, 205, 255));
        outtextxy(panelLeft + 28, deadY + 5, (char*)"[ STATUS: TRAFFIC FLOWING ]");
    }

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, deadY + 36, panelRight - 10, deadY + 36);

    // ================= 2. ACTIVE PHASE & HIGH-VISIBILITY TIMER =================
    int phaseY = deadY + 44;
    setcolor(COLOR(200, 215, 230));
    outtextxy(panelLeft + 12, phaseY, (char*)"ACTIVE SIGNAL PHASE:");

    const char* phaseStr = "";
    int phaseColor = WHITE;
    if (manager.lightController.isNorthSouthGreen())  { phaseStr = "NORTH - SOUTH : [ GREEN ]"; phaseColor = COLOR(40, 245, 100); }
    if (manager.lightController.isNorthSouthYellow()) { phaseStr = "NORTH - SOUTH : [ YELLOW ]"; phaseColor = COLOR(255, 215, 0); }
    if (manager.lightController.isEastWestGreen())    { phaseStr = "EAST - WEST   : [ GREEN ]"; phaseColor = COLOR(40, 245, 100); }
    if (manager.lightController.isEastWestYellow())   { phaseStr = "EAST - WEST   : [ YELLOW ]"; phaseColor = COLOR(255, 215, 0); }

    setcolor(phaseColor);
    outtextxy(panelLeft + 12, phaseY + 18, (char*)phaseStr);

    // Animated Phase Countdown Bar
    float rem = manager.lightController.timeRemaining;
    float dur = manager.lightController.phaseDuration;
    float pct = std::max(0.0f, std::min(1.0f, rem / (dur > 0.0f ? dur : 1.0f)));

    int barX = panelLeft + 12;
    int barY = phaseY + 38;
    int barW = panelRight - panelLeft - 24;
    int barH = 15;

    // Background track
    setfillstyle(SOLID_FILL, COLOR(30, 42, 58));
    bar(barX, barY, barX + barW, barY + barH);
    setcolor(COLOR(65, 85, 110));
    rectangle(barX, barY, barX + barW, barY + barH);

    // Active fill
    setfillstyle(SOLID_FILL, phaseColor);
    bar(barX + 1, barY + 1, barX + static_cast<int>((barW - 2) * pct), barY + barH - 1);

    setcolor(COLOR(240, 240, 240));
    std::snprintf(buf, sizeof(buf), "%.1f sec remaining (Total %.1fs)", rem, dur);
    outtextxy(panelLeft + 12, barY + 18, buf);

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, barY + 40, panelRight - 10, barY + 40);

    // ================= 3. 4-WAY APPROACH REAL-TIME STATUS CARDS =================
    int countY = barY + 48;
    setcolor(COLOR(200, 215, 230));
    outtextxy(panelLeft + 12, countY, (char*)"ROAD APPROACH MONITOR:");

    struct ApproachRow {
        const char* name;
        int count;
        bool isGreen;
        bool isYellow;
    } approaches[4] = {
        { "South (Top):", manager.countSouth, manager.lightController.isNorthSouthGreen(), manager.lightController.isNorthSouthYellow() },
        { "North (Bot):", manager.countNorth, manager.lightController.isNorthSouthGreen(), manager.lightController.isNorthSouthYellow() },
        { "East  (Lft):", manager.countEast,  manager.lightController.isEastWestGreen(),   manager.lightController.isEastWestYellow() },
        { "West  (Rgt):", manager.countWest,  manager.lightController.isEastWestGreen(),   manager.lightController.isEastWestYellow() }
    };

    int rowY = countY + 22;
    for (int i = 0; i < 4; ++i) {
        // Signal pill
        int sigColor = approaches[i].isGreen ? COLOR(40, 240, 90) : (approaches[i].isYellow ? COLOR(255, 210, 10) : COLOR(255, 50, 50));
        const char* sigText = approaches[i].isGreen ? "GO " : (approaches[i].isYellow ? "YEL" : "RED");

        setfillstyle(SOLID_FILL, sigColor);
        bar(panelLeft + 12, rowY + 1, panelLeft + 44, rowY + 16);
        setcolor(BLACK);
        setbkcolor(sigColor);
        outtextxy(panelLeft + 15, rowY + 2, (char*)sigText);

        setbkcolor(COLOR(15, 22, 32));
        setcolor(COLOR(180, 195, 215));
        std::snprintf(buf, sizeof(buf), "%-12s %d cars", approaches[i].name, approaches[i].count);
        outtextxy(panelLeft + 50, rowY + 2, buf);

        // Density meter bar
        int dBarX = panelLeft + 195;
        int dBarW = 60;
        int dBarH = 12;
        int filledW = std::min(dBarW, approaches[i].count * 12);

        setfillstyle(SOLID_FILL, COLOR(28, 38, 50));
        bar(dBarX, rowY + 2, dBarX + dBarW, rowY + 2 + dBarH);

        int densityColor = (approaches[i].count >= 4) ? COLOR(255, 60, 60) : ((approaches[i].count >= 2) ? COLOR(255, 210, 20) : COLOR(40, 210, 100));
        setfillstyle(SOLID_FILL, densityColor);
        bar(dBarX, rowY + 2, dBarX + filledW, rowY + 2 + dBarH);

        rowY += 24;
    }

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, rowY + 6, panelRight - 10, rowY + 6);

    // ================= 4. PERFORMANCE & DEADLOCK METRICS =================
    int metricY = rowY + 14;
    setcolor(COLOR(200, 215, 230));
    outtextxy(panelLeft + 12, metricY, (char*)"SYSTEM PERFORMANCE & STATS:");

    std::snprintf(buf, sizeof(buf), "Cleared: %d | Avg Wait: %.1fs", manager.totalClearedVehicles, manager.getAverageWaitTime());
    setcolor(COLOR(175, 190, 210));
    outtextxy(panelLeft + 12, metricY + 20, buf);

    std::snprintf(buf, sizeof(buf), "Deadlocks: %d Det / %d Res", manager.deadlocksDetectedCount, manager.deadlocksResolvedCount);
    setcolor(manager.isDeadlocked ? COLOR(255, 100, 100) : COLOR(175, 190, 210));
    outtextxy(panelLeft + 12, metricY + 38, buf);

    std::snprintf(buf, sizeof(buf), "Box Occupants: %d  |  FPS: %.0f", manager.boxOccupantCount, fps);
    setcolor(COLOR(175, 190, 210));
    outtextxy(panelLeft + 12, metricY + 56, buf);

    // Anti-Deadlock Guard Indicator
    std::snprintf(buf, sizeof(buf), "Box Guard: %s", manager.antiDeadlockGuard ? "[ ACTIVE (ON) ]" : "[ DISABLED (OFF) ]");
    setcolor(manager.antiDeadlockGuard ? COLOR(100, 245, 155) : COLOR(255, 175, 50));
    outtextxy(panelLeft + 12, metricY + 74, buf);

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, metricY + 94, panelRight - 10, metricY + 94);

    // ================= 5. INTERACTIVE HOTKEYS LEGEND =================
    int ctrlY = metricY + 98;
    setcolor(COLOR(250, 210, 80));
    outtextxy(panelLeft + 12, ctrlY, (char*)"KEYBOARD CONTROLS:");

    setcolor(COLOR(145, 165, 190));
    outtextxy(panelLeft + 12, ctrlY + 20, (char*)"[N / S / E / W]  Spawn Vehicles");
    outtextxy(panelLeft + 12, ctrlY + 40, (char*)"[D]              Force Deadlock Demo");
    outtextxy(panelLeft + 12, ctrlY + 60, (char*)"[R]              Resolve Deadlock Now");
    outtextxy(panelLeft + 12, ctrlY + 80, (char*)"[X]              Toggle Anti-Deadlock");
    outtextxy(panelLeft + 12, ctrlY + 100, (char*)"[M]              Toggle Mode (Adap/Fix)");
    outtextxy(panelLeft + 12, ctrlY + 120, (char*)"[B]              Burst Spike Traffic");
    outtextxy(panelLeft + 12, ctrlY + 140, (char*)"[C]              Clear All Vehicles");
    outtextxy(panelLeft + 12, ctrlY + 160, (char*)"[P]              Pause / Resume Sim");

    if (isPaused) {
        setfillstyle(SOLID_FILL, COLOR(210, 30, 30));
        bar(panelLeft + 12, panelBottom - 30, panelRight - 12, panelBottom - 8);
        setcolor(WHITE);
        setbkcolor(COLOR(210, 30, 30));
        outtextxy(panelLeft + 60, panelBottom - 23, (char*)"*** SIMULATION PAUSED ***");
    }
}
