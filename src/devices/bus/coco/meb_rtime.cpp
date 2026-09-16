// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    meb_rtime.cpp

    Real time clock for Disto mini expansion bus

    Includes a Centronics parallel port interface

***************************************************************************/

#include "emu.h"
#include "meb_rtime.h"
#include "machine/msm58321.h"
#include "bus/centronics/ctronics.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

// ======================> disto_rtime_device

namespace
{
	class disto_rtime_device
		: public device_t
		, public device_distomeb_interface
	{
		public:
			// construction/destruction
			disto_rtime_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

			void rtc_d0_w(int state);
			void rtc_d1_w(int state);
			void rtc_d2_w(int state);
			void rtc_d3_w(int state);

		protected:
			// device-level overrides
			virtual void device_start() override ATTR_COLD;
			virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
			virtual u8 meb_read(offs_t offset) override;
			virtual void meb_write(offs_t offset, u8 data) override;

		private:
			void busy_w(int state);

			required_device<msm58321_device> m_rtc;
			required_device<centronics_device> m_centronics;
			required_device<output_latch_device> m_parallel_latch;
			uint8_t m_rtc_data;
			u8 m_centronics_busy;
	};


	/***************************************************************************
	    IMPLEMENTATION
	***************************************************************************/

	//-------------------------------------------------
	//  disto_rtime_device - constructor
	//-------------------------------------------------

	disto_rtime_device::disto_rtime_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, DISTOMEB_RTIME, tag, owner, clock)
		, device_distomeb_interface(mconfig, *this)
		, m_rtc(*this, "rtc")
		, m_centronics(*this, "centronics")
		, m_parallel_latch(*this, "latch")
		, m_rtc_data(0)
		, m_centronics_busy(0)
	{
	}

	//-------------------------------------------------
	//  device_start - device-specific startup
	//-------------------------------------------------

	void disto_rtime_device::device_start()
	{
		// save state
		save_item(NAME(m_rtc_data));
		save_item(NAME(m_centronics_busy));
	}

	//-------------------------------------------------
	//  device_add_mconfig - add device configuration
	//-------------------------------------------------

	void disto_rtime_device::device_add_mconfig(machine_config &config)
	{
		MSM58321(config, m_rtc, 32.768_kHz_XTAL);
		m_rtc->d0_handler().set(FUNC(disto_rtime_device::rtc_d0_w));
		m_rtc->d1_handler().set(FUNC(disto_rtime_device::rtc_d1_w));
		m_rtc->d2_handler().set(FUNC(disto_rtime_device::rtc_d2_w));
		m_rtc->d3_handler().set(FUNC(disto_rtime_device::rtc_d3_w));
		m_rtc->set_default_24h(false);


		CENTRONICS(config, m_centronics, centronics_devices, "printer");
		m_centronics->busy_handler().set(FUNC(disto_rtime_device::busy_w));

		OUTPUT_LATCH(config, m_parallel_latch);
		m_centronics->set_output_latch(*m_parallel_latch);
	}

	//-------------------------------------------------
	//  meb_read
	//-------------------------------------------------

	u8 disto_rtime_device::meb_read(offs_t offset)
	{
		u8 result = 0;

		switch(offset)
		{
			case 0x00:  /* FF50 */
				m_rtc->cs1_w(1);
				m_rtc->cs2_w(1);
				m_rtc->read_w(1);
				result = m_rtc_data;
				m_rtc->read_w(0);
				m_rtc->cs2_w(0);
				m_rtc->cs1_w(0);
				break;

			case 0x02:  /* FF52 */
			case 0x03:  /* FF53 */
				// busy is d7
				result = m_centronics_busy << 7;
				break;

			default:
				break;
		}

		LOG("%s read: offset: %02x, result: %02x\n", machine().describe_context(), offset, result);

		return result;
	}


	//-------------------------------------------------
	//    meb_write
	//-------------------------------------------------

	void disto_rtime_device::meb_write(offs_t offset, u8 data)
	{
		LOG("%s write: offset: %02x, data: %02x\n", machine().describe_context(), offset, data);

		switch(offset)
		{
			case 0x00: /* FF50 */
				m_rtc->cs1_w(1);
				m_rtc->cs2_w(1);
				m_rtc->d0_w(BIT(data,0));
				m_rtc->d1_w(BIT(data,1));
				m_rtc->d2_w(BIT(data,2));
				m_rtc->d3_w(BIT(data,3));
				m_rtc->write_w(1);
				m_rtc->write_w(0);
				m_rtc->cs2_w(0);
				m_rtc->cs1_w(0);
				break;

			case 0x01: /* FF51 */
			case 0x02: /* FF52 */
				m_rtc->cs1_w(1);
				m_rtc->cs2_w(1);
				m_rtc->d0_w(BIT(data,0));
				m_rtc->d1_w(BIT(data,1));
				m_rtc->d2_w(BIT(data,2));
				m_rtc->d3_w(BIT(data,3));
				m_rtc->address_write_w(1);
				m_rtc->address_write_w(0);
				m_rtc->cs2_w(0);
				m_rtc->cs1_w(0);

				m_parallel_latch->write(data);
				break;

			case 0x03: /* FF53 */
				m_centronics->write_strobe(1);
				m_centronics->write_strobe(0);
				break;

			default:
				break;
		}
	}


	//-------------------------------------------------
	//    busy_w - centronics busy call back
	//-------------------------------------------------

	void disto_rtime_device::busy_w(int state)
	{
		m_centronics_busy = state;
	}

	void disto_rtime_device::rtc_d0_w(int state)
	{
		if (state)
			m_rtc_data |= 1;
		else
			m_rtc_data &= ~1;

	}

	void disto_rtime_device::rtc_d1_w(int state)
	{
		if (state)
			m_rtc_data |= 2;
		else
			m_rtc_data &= ~2;
	}

	void disto_rtime_device::rtc_d2_w(int state)
	{
		if (state)
			m_rtc_data |= 4;
		else
			m_rtc_data &= ~4;

	}

	void disto_rtime_device::rtc_d3_w(int state)
	{
		if (state)
			m_rtc_data |= 8;
		else
			m_rtc_data &= ~8;

	}

} // Anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DISTOMEB_RTIME, device_distomeb_interface, disto_rtime_device, "distomeb_rtime", "Disto Real Time Clock Card")
