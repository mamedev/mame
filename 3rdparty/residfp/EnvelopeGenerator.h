/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2022 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef ENVELOPEGENERATOR_H
#define ENVELOPEGENERATOR_H

#include "siddefs-fp.h"

#include <cstdint>

namespace reSIDfp
{

/**
 * A 15 bit [LFSR] is used to implement the envelope rates, in effect dividing
 * the clock to the envelope counter by the currently selected rate period.
 *
 * In addition, another 5 bit counter is used to implement the exponential envelope decay,
 * in effect further dividing the clock to the envelope counter.
 * The period of this counter is set to 1, 2, 4, 8, 16, 30 at the envelope counter
 * values 255, 93, 54, 26, 14, 6, respectively.
 *
 * [LFSR]: https://en.wikipedia.org/wiki/Linear_feedback_shift_register
 */
class EnvelopeGenerator
{
    friend class State;

public:
    /**
     * The envelope state machine's distinct states. In addition to this,
     * envelope has a hold mode, which freezes envelope counter to zero.
     */
    enum class State
    {
        ATTACK, DECAY_SUSTAIN, RELEASE
    };

private:
    /// XOR shift register for ADSR prescaling.
    uint16_t lfsr = 0x7fff;

    /// Comparison value (period) of the rate counter before next event.
    uint16_t rate = 0;

    /**
     * During release mode, the SID approximates envelope decay via piecewise
     * linear decay rate.
     */
    unsigned int exponential_counter = 0;

    /**
     * Comparison value (period) of the exponential decay counter before next
     * decrement.
     */
    unsigned int exponential_counter_period = 1;
    unsigned int new_exponential_counter_period = 0;

    unsigned int state_pipeline = 0;

    ///
    unsigned int envelope_pipeline = 0;

    unsigned int exponential_pipeline = 0;

    /// Current envelope state
    State state = State::RELEASE;
    State next_state = State::RELEASE;

    /// Whether counter is enabled. Only switching to ATTACK can release envelope.
    bool counter_enabled = true;

    /// Gate bit
    bool gate = false;

    ///
    bool resetLfsr = false;

    /// The current digital value of envelope output.
    uint8_t envelope_counter = 0xaa;

    /// Attack register
    uint8_t attack = 0;

    /// Decay register
    uint8_t decay = 0;

    /// Sustain register
    uint8_t sustain = 0;

    /// Release register
    uint8_t release = 0;

    /// The ENV3 value, sampled at the first phase of the clock
    uint8_t env3 = 0;

private:
    static const uint16_t adsrtable[16];

private:
    void set_exponential_counter();

    void state_change();

public:
    /**
     * SID clocking.
     */
    void clock();

    /**
     * Get the Envelope Generator digital output.
     */
    uint8_t output() const { return envelope_counter; }

    /**
     * SID reset.
     */
    void reset();

    /**
     * Write control register.
     *
     * @param control
     *            control register value
     */
    void writeCONTROL_REG(uint8_t control);

    /**
     * Write Attack/Decay register.
     *
     * @param attack_decay
     *            attack/decay value
     */
    void writeATTACK_DECAY(uint8_t attack_decay);

    /**
     * Write Sustain/Release register.
     *
     * @param sustain_release
     *            sustain/release value
     */
    void writeSUSTAIN_RELEASE(uint8_t sustain_release);

    /**
     * Return the envelope current value.
     *
     * @return envelope counter value
     */
    uint8_t readENV() const { return env3; }
};

} // namespace reSIDfp

#if RESIDFP_INLINING || defined(ENVELOPEGENERATOR_CPP)

namespace reSIDfp
{

RESIDFP_INLINE
void EnvelopeGenerator::clock()
{
    env3 = envelope_counter;

    if (unlikely(new_exponential_counter_period > 0))
    {
        exponential_counter_period = new_exponential_counter_period;
        new_exponential_counter_period = 0;
    }

    if (unlikely(state_pipeline))
    {
        state_change();
    }

    if (unlikely(envelope_pipeline != 0) && (--envelope_pipeline == 0))
    {
        if (likely(counter_enabled))
        {
            if (state == State::ATTACK)
            {
                if (++envelope_counter==0xff)
                {
                    next_state = State::DECAY_SUSTAIN;
                    state_pipeline = 3;
                }
            }
            else if ((state == State::DECAY_SUSTAIN) || (state == State::RELEASE))
            {
                if (--envelope_counter==0x00)
                {
                    counter_enabled = false;
                }
            }

            set_exponential_counter();
        }
    }
    else if (unlikely(exponential_pipeline != 0) && (--exponential_pipeline == 0))
    {
        exponential_counter = 0;

        if (((state == State::DECAY_SUSTAIN) && (envelope_counter != sustain))
            || (state == State::RELEASE))
        {
            // The envelope counter can flip from 0x00 to 0xff by changing state to
            // attack, then to release. The envelope counter will then continue
            // counting down in the release state.
            // This has been verified by sampling ENV3.

            envelope_pipeline = 1;
        }
    }
    else if (unlikely(resetLfsr))
    {
        lfsr = 0x7fff;
        resetLfsr = false;

        if (state == State::ATTACK)
        {
            // The first envelope step in the attack state also resets the exponential
            // counter. This has been verified by sampling ENV3.
            exponential_counter = 0; // NOTE this is actually delayed one cycle, not modeled

            // The envelope counter can flip from 0xff to 0x00 by changing state to
            // release, then to attack. The envelope counter is then frozen at
            // zero; to unlock this situation the state must be changed to release,
            // then to attack. This has been verified by sampling ENV3.

            envelope_pipeline = 2;
        }
        else
        {
            if (counter_enabled && (++exponential_counter == exponential_counter_period))
                exponential_pipeline = exponential_counter_period != 1 ? 2 : 1;
        }
    }

    // ADSR delay bug.
    // If the rate counter comparison value is set below the current value of the
    // rate counter, the counter will continue counting up until it wraps around
    // to zero at 2^15 = 0x8000, and then count rate_period - 1 before the
    // envelope can constly be stepped.
    // This has been verified by sampling ENV3.

    // check to see if LFSR matches table value
    if (likely(lfsr != rate))
    {
        // it wasn't a match, clock the LFSR once
        // by performing XOR on last 2 bits
        const unsigned int feedback = ((lfsr << 14) ^ (lfsr << 13)) & 0x4000;
        lfsr = (lfsr >> 1) | feedback;
    }
    else
    {
        resetLfsr = true;
    }
}

/**
 * This is what happens on chip during state switching,
 * based on die reverse engineering and transistor level
 * emulation.
 *
 * Attack
 *
 *  0 - Gate on
 *  1 - Counting direction changes
 *      During this cycle the decay rate is "accidentally" activated
 *  2 - Counter is being inverted
 *      Now the attack rate is correctly activated
 *      Counter is enabled
 *  3 - Counter will be counting upward from now on
 *
 * Decay
 *
 *  0 - Counter == $ff
 *  1 - Counting direction changes
 *      The attack state is still active
 *  2 - Counter is being inverted
 *      During this cycle the decay state is activated
 *  3 - Counter will be counting downward from now on
 *
 * Release
 *
 *  0 - Gate off
 *  1 - During this cycle the release state is activated if coming from sustain/decay
 * *2 - Counter is being inverted, the release state is activated
 * *3 - Counter will be counting downward from now on
 *
 *  (* only if coming directly from Attack state)
 *
 * Freeze
 *
 *  0 - Counter == $00
 *  1 - Nothing
 *  2 - Counter is disabled
 */
RESIDFP_INLINE
void EnvelopeGenerator::state_change()
{
    state_pipeline--;

    switch (next_state)
    {
    case State::ATTACK:
        if (state_pipeline == 1)
        {
            // The decay rate is "accidentally" enabled during first cycle of attack phase
            rate = adsrtable[decay];
        }
        else if (state_pipeline == 0)
        {
            state = State::ATTACK;
            // The attack rate is correctly enabled during second cycle of attack phase
            rate = adsrtable[attack];
            counter_enabled = true;
        }
        break;
    case State::DECAY_SUSTAIN:
        if (state_pipeline == 0)
        {
            state = State::DECAY_SUSTAIN;
            rate = adsrtable[decay];
        }
        break;
    case State::RELEASE:
        if (((state == State::ATTACK) && (state_pipeline == 0))
            || ((state == State::DECAY_SUSTAIN) && (state_pipeline == 1)))
        {
            state = State::RELEASE;
            rate = adsrtable[release];
        }
        break;
    }
}

RESIDFP_INLINE
void EnvelopeGenerator::set_exponential_counter()
{
    // Check for change of exponential counter period.
    //
    // For a detailed description see:
    // http://ploguechipsounds.blogspot.it/2010/03/sid-6581r3-adsr-tables-up-close.html
    switch (envelope_counter)
    {
    case 0xff:
    case 0x00:
        new_exponential_counter_period = 1;
        break;

    case 0x5d:
        new_exponential_counter_period = 2;
        break;

    case 0x36:
        new_exponential_counter_period = 4;
        break;

    case 0x1a:
        new_exponential_counter_period = 8;
        break;

    case 0x0e:
        new_exponential_counter_period = 16;
        break;

    case 0x06:
        new_exponential_counter_period = 30;
        break;
    }
}

} // namespace reSIDfp

#endif

#endif
