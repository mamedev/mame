/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2015 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SPLINE_H
#define SPLINE_H

#include <cstddef>
#include <vector>

namespace reSIDfp
{

/**
 * Fritsch-Carlson monotone cubic spline interpolation.
 *
 * Based on the implementation from the [Monotone cubic interpolation] wikipedia page.
 *
 * [Monotone cubic interpolation]: https://en.wikipedia.org/wiki/Monotone_cubic_interpolation
 */
class Spline
{
public:
    using Point = struct
    {
        double x;
        double y;
    };

private:
    using Param = struct
    {
        double x1;
        double x2;
        double a;
        double b;
        double c;
        double d;
    };

    using ParamVector = std::vector<Param>;

private:
    /// Interpolation parameters
    ParamVector params;

    /// Last used parameters, cached for speed up
    mutable ParamVector::const_pointer c;

public:
    explicit Spline(const std::vector<Point> &input);

    /**
     * Evaluate y and its derivative at given point x.
     */
    Point evaluate(double x) const;
};

} // namespace reSIDfp

#endif
