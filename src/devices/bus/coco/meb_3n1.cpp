// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    meb_3n1.cpp

    RS232 serial port for Disto mini expansion bus

    Includes an RTC and Centronics parallel port interface

***************************************************************************/

#include "emu.h"
#include "meb_3n1.h"
#include "machine/mos6551.h"
#include "machine/msm6242.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define ACIA_PORT_TAG "rs232"

// ======================> disto_3n1_device

namespace
{
	class disto_3n1_device
		: public device_t
		, public device_distomeb_interface
	{
		public:
			// construction/destruction
			disto_3n1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		protected:
			// device-level overrides
			virtual void device_start() override ATTR_COLD;
			virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
			virtual u8 meb_read(offs_t offset) override;
			virtual void meb_write(offs_t offset, u8 data) override;

		private:
			void busy_w(int state);

			required_device<mos6551_device> m_acia;
			required_device<msm6242_device> m_rtc;
			u8 m_rtc_address;
			required_device<centronics_device> m_centronics;
			required_device<output_latch_device> m_latch;
			u8 m_centronics_busy;
	};


	/***************************************************************************
	    IMPLEMENTATION
	***************************************************************************/

	//-------------------------------------------------
	//  disto_3n1_device - constructor
	//-------------------------------------------------

	disto_3n1_device::disto_3n1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, DISTOMEB_3N1, tag, owner, clock)
		, device_distomeb_interface(mconfig, *this)
		, m_acia(*this, "acia")
		, m_rtc(*this, "rtc")
		, m_rtc_address(0)
		, m_centronics(*this, "centronics")
		, m_latch(*this, "latch")
		, m_centronics_busy(0)
	{
	}

	//-------------------------------------------------
	//  device_start - device-specific startup
	//-------------------------------------------------

	void disto_3n1_device::device_start()
	{
		// save state
		save_item(NAME(m_rtc_address));
		save_item(NAME(m_centronics_busy));
	}

	//-------------------------------------------------
	//  device_add_mconfig - add device configuration
	//-------------------------------------------------

	void disto_3n1_device::device_add_mconfig(machine_config &config)
	{
		MOS6551(config, m_acia);
		m_acia->set_xtal(1.8432_MHz_XTAL);
		m_acia->irq_handler().set(*this, FUNC(disto_3n1_device::set_cart_value));
		m_acia->txd_handler().set(ACIA_PORT_TAG, FUNC(rs232_port_device::write_txd));

		rs232_port_device &rs232(RS232_PORT(config, ACIA_PORT_TAG, default_rs232_devices, nullptr));
		rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
		rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
		rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
		rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

 		MSM6242(config, m_rtc,  XTAL(32'768));
		m_rtc->set_default_24h(false);

		CENTRONICS(config, m_centronics, centronics_devices, "printer");
		m_centronics->busy_handler().set(FUNC(disto_3n1_device::busy_w));

		OUTPUT_LATCH(config, m_latch);
		m_centronics->set_output_latch(*m_latch);
	}

	//-------------------------------------------------
	//  meb_read
	//-------------------------------------------------

	u8 disto_3n1_device::meb_read(offs_t offset)
	{
		u8 result = 0;

		switch(offset)
		{
			case 0x00:  /* FF50 */
				result = m_rtc->read(m_rtc_address);
				break;

			case 0x02:  /* FF52 */
				result = m_centronics_busy << 7;
				break;

			case 0x03:  /* FF53 */
				break;

			case 0x04: /* FF54 */
			case 0x05: /* FF55 */
			case 0x06: /* FF56 */
			case 0x07: /* FF57 */
				result = m_acia->read(offset & 0x03);
				break;
		}

		LOG("%s read:  %02x %02x\n", machine().describe_context(), offset, result);

		return result;
	}


	//-------------------------------------------------
	//    meb_write
	//-------------------------------------------------

	void disto_3n1_device::meb_write(offs_t offset, u8 data)
	{
		LOG("%s write: %02x %02x\n", machine().describe_context(), offset, data);

		switch(offset)
		{
			case 0x00: /* FF50 */
				m_rtc->write(m_rtc_address & 0x0f, data);
				break;

			case 0x01: /* FF51 */
				m_rtc_address = data;
				break;

 			case 0x02: /* FF52 */
				m_latch->write(data);
				m_centronics->write_strobe(1);
				m_centronics->write_strobe(0);
				break;

			case 0x03: /* FF53 */
				break;

			case 0x04: /* FF54 */
			case 0x05: /* FF55 */
			case 0x06: /* FF56 */
			case 0x07: /* FF57 */
				m_acia->write(offset & 0x03, data);
				break;
		}
	}


	//-------------------------------------------------
	//    busy_w - centronics busy call back
	//-------------------------------------------------

	void disto_3n1_device::busy_w(int state)
	{
		m_centronics_busy = state;
	}
} // Anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DISTOMEB_3N1, device_distomeb_interface, disto_3n1_device, "distomeb_3n1", "Disto 3 in 1 Card")
