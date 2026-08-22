#ifndef MAIN_H
#define MAIN_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

// C++ Std Usings
using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double random_double()
{
    return std::rand() / (RAND_MAX + 1.0);
}


inline double random_double(double min, double max)
{
    // Return a random real in [min,max).
    return min + (max-min)*random_double();
}

inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}

// Common Headers

#include "color.h"
#include "ray.h"
#include "interval.h"
#include "vec3.h"

#endif