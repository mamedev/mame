/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2024 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "OpAmp.h"

#include <cmath>

#include "siddefs-fp.h"

namespace reSIDfp
{

constexpr double EPSILON = 1e-8;

double OpAmp::solve(double n, double vi) const
{
    // Start off with an estimate of x and a root bracket [ak, bk].
    // f is decreasing, so that f(ak) > 0 and f(bk) < 0.
    double ak = vmin;
    double bk = vmax;

    const double a = n + 1.;
    const double b = Vddt;
    const double b_vi = (b > vi) ? (b - vi) : 0.;
    const double c = n * (b_vi * b_vi);

    for (;;)
    {
        const double xk = x;

        // Calculate f and df.

        Spline::Point out = opamp.evaluate(x);
        const double vo = out.x;
        const double dvo = out.y;

        const double b_vx = (b > x) ? b - x : 0.;
        const double b_vo = (b > vo) ? b - vo : 0.;

        // f = a*(b - vx)^2 - c - (b - vo)^2
        const double f = a * (b_vx * b_vx) - c - (b_vo * b_vo);

        // df = 2*((b - vo)*dvo - a*(b - vx))
        const double df = 2. * (b_vo * dvo - a * b_vx);

        // Newton-Raphson step: xk1 = xk - f(xk)/f'(xk)
        x -= f / df;

        if (unlikely(std::fabs(x - xk) < EPSILON))
        {
            out = opamp.evaluate(x);
            return out.x;
        }

        // Narrow down root bracket.
        (f < 0. ? bk : ak) = xk;

        if (unlikely(x <= ak) || unlikely(x >= bk))
        {
            // Bisection step (ala Dekker's method).
            x = (ak + bk) * 0.5;
        }
    }
}

} // namespace reSIDfp
