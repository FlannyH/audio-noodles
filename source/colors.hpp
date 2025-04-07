#pragma once
#include <glm/vec4.hpp>

#define MIN(a, b)          (a < b ? a : b)
#define MAX(a, b)          (a > b ? a : b)
#define MIN3(a, b, c)      MIN(a, MIN(b, c))
#define MAX3(a, b, c)      MAX(a, MAX(b, c))
#define MIN4(a, b, c, d)   MIN(MIN(a, b), MIN(c, d))
#define MAX4(a, b, c, d)   MAX(MAX(a, b), MAX(c, d))
#define CLAMP(v, min, max) MIN(max, MAX(min, v))

typedef glm::vec4 Color; // In RGBA, scale 0-1
namespace Colors {
    inline Color rgb_to_hsv(Color in) {
        // Check if values are correct
        assert(in.r >= 0.f && in.r <= 1.f);
        assert(in.g >= 0.f && in.g <= 1.f);
        assert(in.b >= 0.f && in.b <= 1.f);

        // Calculate common variables
        float cmin = MIN3(in.r, in.g, in.b);
        float cmax = MAX3(in.r, in.g, in.b);
        float diff = cmax - cmin;

        // Calculate hue
        float hue = -1.f;
        if (cmax == cmin) {
            hue = 0.f;
        } else if (cmax == in.r) {
            hue = (60 * (in.g - in.b) / diff);
            if (hue < 0.f) {
                hue += 360.f;
            }
        } else if (cmax == in.g) {
            hue = (60 * ((in.b - in.r) / diff) + 120.f);
        } else {
            hue = (60 * ((in.r - in.g) / diff) + 240.f);
        }
        hue /= 360.f;
        assert(hue != -1.f);

        // Calculate saturation
        float saturation = cmax;
        if (saturation != 0.f) {
            saturation = (diff / cmax);
        }

        // Combine to hsv
        return Color(hue, saturation, cmax, in.a);
    }

    inline Color hsv_to_rgb(Color in) {
        // Split the input, as glm::vec3 doesn't support .h/.s/.v, meaning we need to use .x/.y/.z or .r/.g/.b, which could make
        // it confusing
        float hue        = in.x;
        float saturation = in.y;
        float value      = in.z;

        // Check if values are correct
        assert(hue >= 0.f && hue <= 1.f);
        assert(saturation >= 0.f && saturation <= 1.f);
        assert(value >= 0.f && value <= 1.f);

        // Calculate common variables
        hue *= 6.f; // Doing this makes the calculations below easier, as we can now use round numbers. This is because hue is
                    // normally 0-360, but here 0-1, and we need to check in increments of 60.
        float chroma         = value * saturation;
        float second_largest = chroma * (1.f - abs(fmod(hue, 2.f) - 1.f));

        // Calculate rgb
        float red   = 0.f;
        float green = 0.f;
        float blue  = 0.f;
        if (hue >= 0.f && hue < 1.f) {
            red = chroma, green = second_largest, blue = 0.f;
        } else if (hue >= 1.f && hue < 2.f) {
            red = second_largest, green = chroma, blue = 0.f;
        } else if (hue >= 2.f && hue < 3.f) {
            red = 0.f, green = chroma, blue = second_largest;
        } else if (hue >= 3.f && hue < 4.f) {
            red = 0.f, green = second_largest, blue = chroma;
        } else if (hue >= 4.f && hue < 5.f) {
            red = second_largest, green = 0.f, blue = chroma;
        } else {
            red = chroma, green = 0.f, blue = second_largest;
        }

        // Combine to rgb
        float match = value - chroma;
        return Color(red + match, green + match, blue + match, in.a);
    }

    // Preset greyscale
    constexpr Color BLACK     = Color(0.00f, 0.00f, 0.00f, 1.f);
    constexpr Color DARK_GREY = Color(0.33f, 0.33f, 0.33f, 1.f);
    constexpr Color GREY      = Color(0.66f, 0.66f, 0.66f, 1.f);
    constexpr Color WHITE     = Color(1.00f, 1.00f, 1.00f, 1.f);

    // Preset colors
    constexpr Color RED     = Color(1.00f, 0.00f, 0.00f, 1.f);
    constexpr Color ORANGE  = Color(1.00f, 0.50f, 0.00f, 1.f);
    constexpr Color YELLOW  = Color(1.00f, 1.00f, 0.00f, 1.f);
    constexpr Color GREEN   = Color(0.00f, 1.00f, 0.00f, 1.f);
    constexpr Color CYAN    = Color(0.00f, 1.00f, 1.00f, 1.f);
    constexpr Color BLUE    = Color(0.00f, 0.00f, 1.00f, 1.f);
    constexpr Color PURPLE  = Color(0.50f, 0.00f, 1.00f, 1.f);
    constexpr Color MAGENTA = Color(1.00f, 0.00f, 1.00f, 1.f);

    // Preset dark colors
    constexpr Color DARK_RED     = Color(0.50f, 0.00f, 0.00f, 1.f);
    constexpr Color DARK_ORANGE  = Color(0.50f, 0.25f, 0.00f, 1.f);
    constexpr Color DARK_YELLOW  = Color(0.50f, 0.50f, 0.00f, 1.f);
    constexpr Color DARK_GREEN   = Color(0.00f, 0.50f, 0.00f, 1.f);
    constexpr Color DARK_CYAN    = Color(0.00f, 0.50f, 0.50f, 1.f);
    constexpr Color DARK_BLUE    = Color(0.00f, 0.00f, 0.50f, 1.f);
    constexpr Color DARK_PURPLE  = Color(0.25f, 0.00f, 0.50f, 1.f);
    constexpr Color DARK_MAGENTA = Color(0.50f, 0.00f, 0.50f, 1.f);
} // namespace Colors
