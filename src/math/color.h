#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec3.h"

#include "SDL3/SDL.h"

using color = vec3;

inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0) return std::sqrt(linear_component);
    return 0;
}

void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Apply a linear to gamma transform for gamma 2
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    // Write out the pixel color components.
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

void write_color(uint32_t* buffer, int index, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Apply a linear to gamma transform for gamma 2
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    // Write out the pixel color components.
    const SDL_PixelFormatDetails* format_details = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888);
    uint32_t hex_color = SDL_MapRGBA(format_details, NULL, rbyte, gbyte, bbyte, 255);
    buffer[index] = hex_color;
}

#endif