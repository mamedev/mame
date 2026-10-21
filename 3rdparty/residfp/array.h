/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright (C) 2011-2026 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ARRAY_H
#define ARRAY_H

#include <cstdint>

#include <atomic>
#include <memory>

template<typename T>
class matrix
{
private:
    T* data;
    const unsigned int x, y;

private:
    matrix& operator=(const matrix&) = delete;

public:
    matrix(unsigned int new_x, unsigned int new_y) :
        data(new T[new_x * new_y]),
        x(new_x),
        y(new_y) {}

    matrix(const matrix& p) :
        data(p.data),
        x(p.x),
        y(p.y) {}

    ~matrix() { delete [] data; }

    unsigned int length() const { return x * y; }

    T* operator[](unsigned int a) { return &data[a * y]; }

    T const* operator[](unsigned int a) const { return &data[a * y]; }
};

using matrix_t = matrix<int16_t>;
using rc_matrix_t = std::shared_ptr<matrix_t>;

#endif
