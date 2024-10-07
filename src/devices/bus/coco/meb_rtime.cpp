// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    meb_rtime.cpp

    Real time clock for Disto mini expansion bus

    Includes a Centronics parallel port interface

***************************************************************************/

#include "emu.h"
#include "meb_rtime.h"
#include "machine/msm6242.h"
#include "bus/centronics/ctronics.h"

#define VERBOSE (LOG_GENERAL)
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

		protected:
			// device-level overrides
			virtual void device_start() override ATTR_COLD;
			virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
			virtual u8 meb_read(offs_t offset) override;
			virtual void meb_write(offs_t offset, u8 data) override;

		private:
			void busy_w(int state);

			required_device<msm6242_device> m_rtc;
			u8 m_rtc_address;
			u8 m_double_write;
			required_device<centronics_device> m_centronics;
			required_device<output_latch_device> m_latch;
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
		, m_rtc_address(0)
		, m_double_write(0)
		, m_centronics(*this, "centronics")
		, m_latch(*this, "latch")
		, m_centronics_busy(0)
	{
	}

	//-------------------------------------------------
	//  device_start - device-specific startup
	//-------------------------------------------------

	void disto_rtime_device::device_start()
	{
		// save state
		save_item(NAME(m_rtc_address));
		save_item(NAME(m_double_write));
		save_item(NAME(m_centronics_busy));
	}

	//-------------------------------------------------
	//  device_add_mconfig - add device configuration
	//-------------------------------------------------

	void disto_rtime_device::device_add_mconfig(machine_config &config)
	{
		MSM6242(config, m_rtc, XTAL(32'768));

		CENTRONICS(config, m_centronics, centronics_devices, "printer");
		m_centronics->busy_handler().set(FUNC(disto_rtime_device::busy_w));

		OUTPUT_LATCH(config, m_latch);
		m_centronics->set_output_latch(*m_latch);
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
				result = m_rtc->read(m_rtc_address & 0x0f);
				break;

			case 0x02:  /* FF52 */
			case 0x03:  /* FF53 */
				result = m_centronics_busy << 7;
		}

		return result;
	}


	//-------------------------------------------------
	//    meb_write
	//-------------------------------------------------

	void disto_rtime_device::meb_write(offs_t offset, u8 data)
	{
		LOG("meb write: %02x %02x\n", offset, data);

		switch(offset)
		{
			case 0x00: /* FF50 */
				m_rtc->write(m_rtc_address & 0x0f, data);
				break;

			case 0x01: /* FF51 */
				m_rtc_address = data;
				break;

			case 0x02: /* FF52 */
			case 0x03: /* FF53 */
				if ((m_rtc_address == data) && !m_double_write)
				{
					m_double_write = 1;
					m_latch->write(data);
					m_centronics->write_strobe(1);
					m_centronics->write_strobe(0);
				}

				m_rtc_address = data;
				break;
		}

		m_double_write = 0;
	}


	//-------------------------------------------------
	//    busy_w - centronics busy call back
	//-------------------------------------------------

	void disto_rtime_device::busy_w(int state)
	{
		m_centronics_busy = state;
	}
} // Anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DISTOMEB_RTIME, device_distomeb_interface, disto_rtime_device, "distomeb_rtime", "Disto Real Time Clock Card")
