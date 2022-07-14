// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file param.h
///

#ifndef NL_CORE_ANALOG_H_
#define NL_CORE_ANALOG_H_

#include "../nltypes.h"
#include "base_objects.h"
#include "nets.h"

#include "../plib/plists.h"
#include "../plib/pstring.h"

#include <array>
#include <utility>

namespace netlist
{
	// -------------------------------------------------------------------------
	// analog_t
	// -------------------------------------------------------------------------

	class analog_t : public detail::core_terminal_t
	{
	public:
		analog_t(core_device_t &dev, const pstring &aname, state_e state,
			nl_delegate delegate);

		const analog_net_t &net() const noexcept
		{
			return plib::downcast<const analog_net_t &>(core_terminal_t::net());
		}

		analog_net_t &net() noexcept
		{
			return plib::downcast<analog_net_t &>(core_terminal_t::net());
		}

		solver::matrix_solver_t *solver() const noexcept;
	};

	/// \brief Base class for terminals.
	///
	/// Each \ref nld_two_terminal object consists of two terminals. Terminals
	/// are at the core of analog netlists and are connected to  \ref net_t
	/// objects.
	///
	class terminal_t : public analog_t
	{
	public:
		/// \brief constructor
		///
		/// \param dev object owning the terminal
		/// \param aname name of this terminal
		/// \param other_terminal pointer to the sibling terminal
		terminal_t(core_device_t &dev, const pstring &aname,
			terminal_t *other_terminal, nl_delegate delegate);

		terminal_t(core_device_t &dev, const pstring &aname,
			terminal_t *                       other_terminal,
			const std::array<terminal_t *, 2> &splitter_terms,
			nl_delegate                        delegate);

		/// \brief Returns voltage of connected net
		///
		/// \return voltage of net this terminal is connected to
		nl_fptype operator()() const noexcept { return net().Q_Analog(); }

		/// \brief sets conductivity value of this terminal
		///
		/// \param G Conductivity
		void set_conductivity(nl_fptype G) const noexcept
		{
			set_go_gt_I(-G, G, nlconst::zero());
		}

		void set_go_gt(nl_fptype GO, nl_fptype GT) const noexcept
		{
			set_go_gt_I(GO, GT, nlconst::zero());
		}

		void set_go_gt_I(nl_fptype GO, nl_fptype GT,
			nl_fptype I) const noexcept;

		void set_ptrs(nl_fptype *gt, nl_fptype *go, nl_fptype *Idr) noexcept(
			false);

	private:
		nl_fptype *m_Idr; //!< drive current
		nl_fptype *m_go;  //!< conductance for Voltage from other term
		nl_fptype *m_gt;  //!< conductance for total conductance
	};

	// -------------------------------------------------------------------------
	// analog_input_t
	// -------------------------------------------------------------------------

	/// \brief terminal providing analog input voltage.
	///
	/// This terminal class provides a voltage measurement. The conductance
	/// against ground is infinite.
	class analog_input_t : public analog_t
	{
	public:
		/// \brief Constructor
		analog_input_t(core_device_t &dev,     //!< owning device
			const pstring &           aname,   //!< name of terminal
			nl_delegate               delegate //!< delegate
		);

		/// \brief returns voltage at terminal.
		///  \returns voltage at terminal.
		nl_fptype operator()() const noexcept { return Q_Analog(); }

		/// \brief returns voltage at terminal.
		///  \returns voltage at terminal.
		nl_fptype Q_Analog() const noexcept { return net().Q_Analog(); }
	};

	// -------------------------------------------------------------------------
	// analog_output_t
	// -------------------------------------------------------------------------

	class analog_output_t : public analog_t
	{
	public:
		analog_output_t(core_device_t &dev, const pstring &aname);

		void push(nl_fptype val) noexcept;

		void initial(nl_fptype val) noexcept;

	private:
		analog_net_t m_my_net;
	};

	// -------------------------------------------------------------------------
	// out of class
	// -------------------------------------------------------------------------

	inline solver::matrix_solver_t *analog_t::solver() const noexcept
	{
		return (this->has_net() ? net().solver() : nullptr);
	}

	inline void terminal_t::set_go_gt_I(nl_fptype GO, nl_fptype GT,
		nl_fptype I) const noexcept
	{
		// Check for rail nets ...
		if (m_go != nullptr)
		{
			*m_Idr = I;
			*m_go = GO;
			*m_gt = GT;
		}
	}

	inline void analog_output_t::push(nl_fptype val) noexcept
	{
		if (val != m_my_net.Q_Analog())
		{
			m_my_net.set_Q_Analog(val);
			m_my_net.toggle_and_push_to_queue(netlist_time::quantum());
		}
	}

} // namespace netlist

#endif // NL_CORE_ANALOG_H_
