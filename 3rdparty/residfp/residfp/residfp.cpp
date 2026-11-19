/*
 * This file is part of residfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
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

#include "residfp.h"

#include "SID.h"
#include "State.h"

#include <cstring>

using namespace reSIDfp;

residfp::residfp() :
    sid(*(new SID)) {}

residfp::~residfp()
{
    delete &sid;
}

bool residfp::setChipModel(ChipModel model)
{
    try
    {
        sid.setChipModel(model);
        return true;
    }
    catch(SIDError&)
    {
        return false;
    }
}

ChipModel residfp::getChipModel() const
{
    return sid.getChipModel();
}

bool residfp::setCombinedWaveforms(CombinedWaveforms cws)
{
    try
    {
        sid.setCombinedWaveforms(cws);
        return true;
    }
    catch(SIDError&)
    {
        return false;
    }
}

void residfp::reset()
{
    sid.reset();
}

void residfp::input(int value)
{
    sid.input(value);
}

unsigned char residfp::peek(int offset) const
{
    return sid.peek(offset);
}

unsigned char residfp::read(int offset)
{
    return sid.read(offset);
}

void residfp::write(int offset, unsigned char value)
{
    sid.write(offset, value);
}

bool residfp::setSamplingParameters(
        double clockFrequency,
        SamplingMethod method,
        double samplingFrequency
    )
{
    try
    {
        sid.setSamplingParameters(clockFrequency, method, samplingFrequency);
        return true;
    }
    catch(SIDError&)
    {
        return false;
    }
}

int residfp::clock(unsigned int cycles, short* buf)
{
    return sid.clock(cycles, buf);
}

int residfp::clock( short* buf, int bufSize)
{
    return sid.clock(buf, bufSize);
}

void residfp::clockDigital(unsigned int cycles)
{
    sid.clockDigital(cycles);
}

void residfp::clockSilent(unsigned int cycles)
{
    sid.clockSilent(cycles);
}

void residfp::setFilter6581Curve(double filterCurve)
{
    sid.setFilter6581Curve(filterCurve);
}

void residfp::setFilter6581Range(double adjustment)
{
    sid.setFilter6581Range(adjustment);
}

void residfp::setFilter8580Curve(double filterCurve)
{
    sid.setFilter8580Curve(filterCurve);
}

void residfp::enableFilter(bool enable)
{
    sid.enableFilter(enable);
}

void residfp::enableOld6581caps(bool enable)
{
    sid.enableOld6581caps(enable);
}

void residfp::setOffset6581(double offset)
{
    sid.setOffset6581(offset);
}

void residfp::setPaddle(unsigned char x, unsigned char y)
{
    sid.setPaddle(x, y);
}

int residfp::stateSize() const
{
    return State::size(sid);
}

int residfp::saveState(char* buffer, int size) const
{
    return State::saveState(sid, buffer, size);
}

void residfp::restoreState(char* buffer, int size)
{
    State::restoreState(sid, buffer, size);
}

void residfp::setDacLeakage(double level)
{
    sid.setDacLeakage(level);
}

void residfp::setDCBRes(double res)
{
    sid.setDCBRes(res);
}
