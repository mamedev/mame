// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    xtal.h

    Documentation and consistent naming for known existing crystals.
    See the .cpp file for details

***************************************************************************/


#ifndef MAME_EMU_DRIVERS_XTAL_H
#define MAME_EMU_DRIVERS_XTAL_H

#pragma once

class XTAL {
public:

	constexpr XTAL(double base_clock) : m_base_clock(base_clock), m_current_clock(base_clock) {}

	constexpr double dvalue() const noexcept { return m_current_clock; }
	constexpr u32    value()  const noexcept { return u32(m_current_clock); }
	constexpr double base()   const noexcept { return m_base_clock; }

	constexpr XTAL operator *(int          mult) const noexcept { return XTAL(m_base_clock, m_current_clock * mult); }
	constexpr XTAL operator *(unsigned int mult) const noexcept { return XTAL(m_base_clock, m_current_clock * mult); }
	constexpr XTAL operator *(double       mult) const noexcept { return XTAL(m_base_clock, m_current_clock * mult); }
	constexpr XTAL operator /(int          div)  const noexcept { return XTAL(m_base_clock, m_current_clock / div); }
	constexpr XTAL operator /(unsigned int div)  const noexcept { return XTAL(m_base_clock, m_current_clock / div); }
	constexpr XTAL operator /(double       div)  const noexcept { return XTAL(m_base_clock, m_current_clock / div); }

	friend constexpr XTAL operator /(int          div,  const XTAL &xtal);
	friend constexpr XTAL operator /(unsigned int div,  const XTAL &xtal);
	friend constexpr XTAL operator /(double       div,  const XTAL &xtal);
	friend constexpr XTAL operator *(int          mult, const XTAL &xtal);
	friend constexpr XTAL operator *(unsigned int mult, const XTAL &xtal);
	friend constexpr XTAL operator *(double       mult, const XTAL &xtal);

	void check(const char *message) const;
	void check(const std::string &message) const;

private:
	double m_base_clock, m_current_clock;

	constexpr XTAL(double base_clock, double current_clock) noexcept : m_base_clock(base_clock), m_current_clock(current_clock) {}

	static const double known_xtals[];
	static double last_correct_value, xtal_error_low, xtal_error_high;
	static void fail(double base_clock, std::string message);
	static bool check(double base_clock);
	static void check_ordering();
};

inline constexpr XTAL operator /(int          div,  const XTAL &xtal) { return XTAL(xtal.base(), div  / xtal.dvalue()); }
inline constexpr XTAL operator /(unsigned int div,  const XTAL &xtal) { return XTAL(xtal.base(), div  / xtal.dvalue()); }
inline constexpr XTAL operator /(double       div,  const XTAL &xtal) { return XTAL(xtal.base(), div  / xtal.dvalue()); }
inline constexpr XTAL operator *(int          mult, const XTAL &xtal) { return XTAL(xtal.base(), mult * xtal.dvalue()); }
inline constexpr XTAL operator *(unsigned int mult, const XTAL &xtal) { return XTAL(xtal.base(), mult * xtal.dvalue()); }
inline constexpr XTAL operator *(double       mult, const XTAL &xtal) { return XTAL(xtal.base(), mult * xtal.dvalue()); }

#endif // MAME_EMU_DRIVERS_XTAL_H
