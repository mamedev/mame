// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nl_interface.h
///
/// This header contains objects for interfacing with the netlist core.
///

#ifndef NLINTERFACE_H_
#define NLINTERFACE_H_

#include "nl_base.h"
#include "nl_setup.h"
#include "core/setup.h"

#include <memory>
#include <array>

namespace netlist
{

	namespace interface
	{
		/// \brief analog_callback device
		///
		/// This device is used to call back into the application which
		/// is controlling the execution of the netlist.
		///
		/// The device will call the provided lambda with a reference to
		/// itself and the current value of the net it is connected to.
		///
		/// The following code is an example on how to add the device to
		/// the netlist factory.
		///
		///     const pstring pin(m_in);
		///     pstring dname = pstring("OUT_") + pin;
		///
		///     const auto lambda = [this](auto &in, netlist::nl_fptype val)
		///     {
		///         this->cpu()->update_icount(in.exec().time());
		///         this->m_delegate(val, this->cpu()->local_time());
		///         this->cpu()->check_mame_abort_slice();
		///     };
		///
		///     using lb_t = decltype(lambda);
		///     using cb_t = netlist::interface::NETLIB_NAME(analog_callback)<lb_t>;
		///
		///     parser.factory().add<cb_t, netlist::nl_fptype, lb_t>(dname,
		///         netlist::factory::properties("-", PSOURCELOC()), 1e-6, std::forward<lb_t>(lambda));
		///

		template <typename FUNC>
		class NETLIB_NAME(analog_callback) : public device_t
		{
		public:
			NETLIB_NAME(analog_callback)(netlist_state_t &anetlist,
				const pstring &name, nl_fptype threshold, FUNC &&func)
				: device_t(anetlist, name)
				, m_in(*this, "IN")
				, m_threshold(threshold)
				, m_last(*this, "m_last", 0)
				, m_func(func)
			{
			}

			NETLIB_RESETI()
			{
				m_last = 0.0;
			}

			NETLIB_UPDATEI()
			{
				const nl_fptype cur = m_in();
				if (plib::abs(cur - m_last) > m_threshold)
				{
					m_last = cur;
					m_func(*this, cur);
				}
			}

		private:
			analog_input_t m_in;
			nl_fptype      m_threshold;
			state_var<nl_fptype> m_last;
			FUNC m_func;
		};


		/// \brief logic_callback device
		///
		/// This device must be connected to a logic net. It has no power terminals
		/// and conversion with proxies will not work.
		///
		/// Background: This device may be inserted later into the driver and should
		/// not modify the resulting analog representation of the netlist.
		///
		/// If you get error messages on missing power terminals you have to use the
		/// analog callback device instead.

		template <typename FUNC>
		class NETLIB_NAME(logic_callback) : public device_t
		{
		public:
			NETLIB_NAME(logic_callback)(netlist_state_t &anetlist, const pstring &name, FUNC &&func)
				: device_t(anetlist, name)
				, m_in(*this, "IN")
				, m_func(func)
			{
			}

			NETLIB_UPDATEI()
			{
				const netlist_sig_t cur = m_in();
				m_func(*this, cur);
			}

		private:
			logic_input_t m_in;
			FUNC m_func;
		};

		/// \brief Set parameters to buffers contents at regular intervals
		///
		/// This devices will set up to N parameters from buffers passed to the device.
		/// It is the responsibility of the controlling application to ensure that
		/// buffers filled at regular intervals.
		///
		/// \tparam T The buffer type
		/// \tparam N Maximum number of supported buffers
		///
		template <typename T, std::size_t N>
		class NETLIB_NAME(buffered_param_setter) : public device_t
		{
		public:

			static const int MAX_INPUT_CHANNELS = N;

			NETLIB_NAME(buffered_param_setter)(netlist_state_t &anetlist, const pstring &name)
			: device_t(anetlist, name)
			, m_sample_time(netlist_time::zero())
			, m_feedback(*this, "FB") // clock part
			, m_Q(*this, "Q")
			, m_pos(0)
			, m_samples(0)
			, m_num_channels(0)
			, m_param_names(*this, 0, "CHAN{}", "")
			, m_param_mults(*this, 0, "MULT{}", 1.0)
			, m_param_offsets(*this, 0, "OFFSET{}", 0.0)
			{
				connect(m_feedback, m_Q);
			}

		protected:
			NETLIB_RESETI()
			{
				m_pos = 0;
				for (auto & elem : m_buffers)
					elem = nullptr;
			}

			NETLIB_UPDATEI()
			{
				if (m_pos < m_samples)
				{
					for (std::size_t i=0; i<m_num_channels; i++)
					{
						if (m_buffers[i] == nullptr)
							break; // stop, called outside of stream_update
						const nl_fptype v = m_buffers[i][m_pos];
						m_params[i]->set(v * m_param_mults[i]() + m_param_offsets[i]());
					}
				}
				else
				{
					// FIXME: The logic has a rounding issue because time resolution divided
					//        by 48,000 is not a natural number. The fractional part
					//        adds up to one samples every 13 seconds for 100 ps resolution.
					//        Fixing this is possible but complicated and expensive.
				}
				m_pos++;

				m_Q.net().toggle_and_push_to_queue(m_sample_time);
			}

		public:
			/// \brief resolve parameter names to pointers
			///
			/// This function must be called after all device were constructed but
			/// before reset is called.
			void resolve_params(netlist_time sample_time)
			{
				m_pos = 0;
				m_sample_time = sample_time;
				for (std::size_t i = 0; i < MAX_INPUT_CHANNELS; i++)
				{
					if (m_param_names[i]() != pstring(""))
					{
						if (i != m_num_channels)
							state().log().fatal("sound input numbering has to be sequential!");
						m_num_channels++;
						m_params[i] = dynamic_cast<param_fp_t *>(
							&state().setup().find_param(m_param_names[i]()).param()
						);
					}
				}
			}

			void buffer_reset(netlist_time sample_time, std::size_t num_samples, T **inputs)
			{
				m_samples = num_samples;
				m_sample_time = sample_time;
				m_pos = 0;
				for (std::size_t i=0; i < m_num_channels; i++)
				{
					m_buffers[i] = inputs[i];
				}
			}

			int num_channels() { return m_num_channels; }

		private:
			netlist_time m_sample_time;

			logic_input_t m_feedback;
			logic_output_t m_Q;

			std::size_t m_pos;
			std::size_t m_samples;
			std::size_t m_num_channels;

			object_array_t<param_str_t, MAX_INPUT_CHANNELS> m_param_names;
			object_array_t<param_fp_t, MAX_INPUT_CHANNELS>  m_param_mults;
			object_array_t<param_fp_t, MAX_INPUT_CHANNELS>  m_param_offsets;
			std::array<param_fp_t *, MAX_INPUT_CHANNELS>             m_params;
			std::array<T *, MAX_INPUT_CHANNELS>                               m_buffers;
		};

	} // namespace interface

} // namespace netlist


#endif // NLINTERFACE_H_
