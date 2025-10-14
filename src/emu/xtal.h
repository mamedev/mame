// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    xtal.h

    Documentation for known existing crystals.
    See the .cpp file for the crystal list


Usage:

    When you're 100% sure there is a given crystal or resonator on a
    PCB, use XTAL(frequency) to document so.  That xtal object then
    collates multiplies or divides done to the base frequency to
    compute the final one.

    If you receive a XTAL object and want to turn it to a final
    frequency, use value() to get an integer or dvalue() to get a
    double.

    If you receive a XTAL object and want to check if the initial
    crystal value is sane, use check(context message).  It will
    fatalerror if the value is not in the authorized value list.  It
    has a (small) cost, so don't do it in a hot path.

    Remember that with PLLs it is perfectly normal to multiply
    frequencies by a rational.  For instance the 315-5746 in the Sega
    Saturn generates two dotclocks at 57.27MHz and 53.69MHz as needed
    from a single 13.32MHz crystal.  Banks of oscillators connected to
    a chip usually don't exist.  So if you're doing a switch to select
    a frequency between multiple XTAL() ones, you're probably doing it
    wrong.  If you're selecting multipliers on a single crystal otoh,
    that's perfectly normal.  I'm looking at you, VGA pixel clock
    generators.

***************************************************************************/

#ifndef MAME_EMU_XTAL_H
#define MAME_EMU_XTAL_H

#include "emucore.h"

#pragma once

class XTAL
{
public:
	constexpr explicit XTAL(double base_clock) : m_base_clock(base_clock), m_current_clock(base_clock) {}

	constexpr double dvalue() const noexcept { return m_current_clock; }
	constexpr u32    value()  const noexcept { return u32(m_current_clock + 1e-3); }
	constexpr double base()   const noexcept { return m_base_clock; }

	template <typename T> constexpr XTAL operator *(T &&mult) const noexcept { return XTAL(m_base_clock, m_current_clock * mult); }
	template <typename T> constexpr XTAL operator /(T &&div) const noexcept { return XTAL(m_base_clock, m_current_clock / div); }

	friend constexpr XTAL operator *(int          mult, const XTAL &xtal);
	friend constexpr XTAL operator *(unsigned int mult, const XTAL &xtal);
	friend constexpr XTAL operator *(double       mult, const XTAL &xtal);

	void validate(const char *message) const;
	void validate(const std::string &message) const;

private:
	double m_base_clock, m_current_clock;

	constexpr XTAL(double base_clock, double current_clock) noexcept : m_base_clock(base_clock), m_current_clock(current_clock) {}

	static const double known_xtals[];
	static double last_correct_value, xtal_error_low, xtal_error_high;
	static void fail(double base_clock, const std::string &message);
	static bool validate(double base_clock);
	static void check_ordering();
};

template <typename T> constexpr auto operator /(T &&div, const XTAL &xtal) { return div / xtal.dvalue(); }

constexpr XTAL operator *(int          mult, const XTAL &xtal) { return XTAL(xtal.base(), mult * xtal.dvalue()); }
constexpr XTAL operator *(unsigned int mult, const XTAL &xtal) { return XTAL(xtal.base(), mult * xtal.dvalue()); }
constexpr XTAL operator *(double       mult, const XTAL &xtal) { return XTAL(xtal.base(), mult * xtal.dvalue()); }

constexpr XTAL operator ""_Hz_XTAL(long double clock) { return XTAL(double(clock)); }
constexpr XTAL operator ""_kHz_XTAL(long double clock) { return XTAL(double(clock * 1e3)); }
constexpr XTAL operator ""_MHz_XTAL(long double clock) { return XTAL(double(clock * 1e6)); }

constexpr XTAL operator ""_Hz_XTAL(unsigned long long clock) { return XTAL(double(clock)); }
constexpr XTAL operator ""_kHz_XTAL(unsigned long long clock) { return XTAL(double(clock) * 1e3); }
constexpr XTAL operator ""_MHz_XTAL(unsigned long long clock) { return XTAL(double(clock) * 1e6); }

#endif // MAME_EMU_XTAL_H
