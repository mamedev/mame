/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2020 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2018 VICE Project
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem <resid@nimrod.no>
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

#define ENVELOPEGENERATOR_CPP

#include "EnvelopeGenerator.h"

namespace reSIDfp
{

/**
 * Lookup table to convert from attack, decay, or release value to rate
 * counter period.
 *
 * The rate counter is a 15 bit register which is left shifted each cycle.
 * When the counter reaches a specific comparison value,
 * the envelope counter is incremented (attack) or decremented
 * (decay/release) and the rate counter is resetted.
 *
 * see [kevtris.org](http://blog.kevtris.org/?p=13)
 */
const uint16_t EnvelopeGenerator::adsrtable[16] =
{
    0x007f,
    0x3000,
    0x1e00,
    0x0660,
    0x0182,
    0x5573,
    0x000e,
    0x3805,
    0x2424,
    0x2220,
    0x090c,
    0x0ecd,
    0x010e,
    0x23f7,
    0x5237,
    0x64a8
};

void EnvelopeGenerator::reset()
{
    // counter is not changed on reset
    envelope_pipeline = 0;

    state_pipeline = 0;

    attack = 0;
    decay = 0;
    sustain = 0;
    release = 0;

    gate = false;

    resetLfsr = true;

    exponential_counter = 0;
    exponential_counter_period = 1;
    new_exponential_counter_period = 0;

    state = State::RELEASE;
    counter_enabled = true;
    rate = adsrtable[release];
}

void EnvelopeGenerator::writeCONTROL_REG(uint8_t control)
{
    const bool gate_next = (control & 0x01) != 0;

    if (gate_next != gate)
    {
        gate = gate_next;

        // The rate counter is never reset, thus there will be a delay before the
        // envelope counter starts counting up (attack) or down (release).

        if (gate_next)
        {
            // Gate bit on:  Start attack, decay, sustain.
            next_state = State::ATTACK;
            state_pipeline = 2;

            if (resetLfsr || (exponential_pipeline == 2))
            {
                envelope_pipeline = (exponential_counter_period == 1) || (exponential_pipeline == 2) ? 2 : 4;
            }
            else if (exponential_pipeline == 1)
            {
                state_pipeline = 3;
            }
        }
        else
        {
            // Gate bit off: Start release.
            next_state = State::RELEASE;
            state_pipeline = envelope_pipeline > 0 ? 3 : 2;
        }
    }
}

void EnvelopeGenerator::writeATTACK_DECAY(uint8_t attack_decay)
{
    attack = (attack_decay >> 4) & 0x0f;
    decay = attack_decay & 0x0f;

    if (state == State::ATTACK)
    {
        rate = adsrtable[attack];
    }
    else if (state == State::DECAY_SUSTAIN)
    {
        rate = adsrtable[decay];
    }
}

void EnvelopeGenerator::writeSUSTAIN_RELEASE(uint8_t sustain_release)
{
    // From the sustain levels it follows that both the low and high 4 bits
    // of the envelope counter are compared to the 4-bit sustain value.
    // This has been verified by sampling ENV3.
    //
    // For a detailed description see:
    // http://ploguechipsounds.blogspot.it/2010/11/new-research-on-sid-adsr.html
    sustain = (sustain_release & 0xf0) | ((sustain_release >> 4) & 0x0f);

    release = sustain_release & 0x0f;

    if (state == State::RELEASE)
    {
        rate = adsrtable[release];
    }
}

} // namespace reSIDfp
