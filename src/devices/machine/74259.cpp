// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    74259/9334 8-Bit Addressable Latch

    The normal latch mode of this device provides a simple write-only
    interface to up to 8 output lines. Its 3-bit address, 1-bit data
    and active-low write enable inputs were commonly connected to the
    bus lines of popular 8-bit microprocessors in the 1970s and 1980s.

    The device may alternately serve as an active-high 1-of-8
    demultiplexer when the asynchronous parallel clear input is held
    low. The practical applications of this mode are more limited, and
    the clear input is usually asserted only during master reset.

    The A0-A2 (or A,B,C) and D inputs may be sourced in various
    permutations, but it seems reasonable to assume, as the emulation
    here requires, that these four inputs should always be written
    simultaneously, since modifying them independently of each other
    while the latch is enabled for writing would cause additional and
    probably undesired changes to outputs.

    The original TTL version of this device was introduced by Fairchild
    in the early 1970s as 9334 in their 9300 MSI series, and second-
    sourced by National Semiconductor as DM8334/DM9334 (the higher
    number being used for the military-rated version). TI copied and
    definitively renumbered many Fairchild MSI devices into their
    standard 5400/7400 series. While Schottky and later versions almost
    always use the 259 numbering (TI assigned SN74LS259 the alternative
    name of TIM9906 when promoting it as a CRU output interface for the
    TMS9900 family), older schematics sometimes suggest 9334 or DM8334
    as pin-compatible substitutes for 74LS259.

    Other addressable latch devices include:
    - CD4099B: Part of RCA's standard CMOS logic series, this device
      offers the same functions as 74259/9334, though its pins are
      arranged rather differently. Like many CMOS devices in the 4000B
      series and the CMOS addressable latches listed below, its reset
      input is active high rather than active low, as is normal for
      TTL devices.
    - SN74256: A dual 4-bit latch with common control inputs and a
      pinout similar to 74259, this device subtracts one of its
      address lines and replaces it with a second data input which is
      loaded in parallel.
    - F4723B/4724B: These dual 4-bit (4723B) and 8-bit (4724B) latches
      likely became part of the 4000B series when Fairchild introduced
      their isoplanar CMOS line around 1974. They are pin-compatible
      with 74256 and 74259 except for having active high clear inputs.
      4723B (originally 34723) might actually have preceded 74256,
      since Fairchild's 1975 MOS CCD Data Book denies the existence of
      a TTL counterpart.
    - MC14599B: Motorola's moderately successful expansion of the
      4000B CMOS series includes this enhanced version of the CD4099B,
      which has a bidirectional data pin and additional read/write and
      chip select (active high) control inputs, allowing output bits
      to be read back.
    - NE590/591 Addressable Peripheral Drivers: Part of Signetics'
      linear catalogue, these combine addressable latches with high
      current drivers. NE590 has open collector Darlington outputs
      (inverting logic) and is pin-compatible with 74LS259. NE591 uses
      an extra chip select and sources outputs from a common collector
      line independent of Vcc; it lacks the demultiplex mode.

***********************************************************************

           74259 Function Table        Latch Selection Table

            Inputs  |  Outputs          Inputs  |   Output
            /C  /E  |  Qa  Qn          A2 A1 A0 |  Addressed
           ---------+----------      -----------+-------------
             H   L  |  D   Qn          L  L  L  |     Q0
             H   H  |  Qa  Qn          L  L  H  |     Q1
             L   L  |  Qa  L           L  H  L  |     Q2
             L   H  |  L   L           L  H  H  |     Q3
                                       H  L  L  |     Q4
         Qa = output addressed         H  L  H  |     Q5
         Qn = all other outputs        H  H  L  |     Q6
                                       H  H  H  |     Q7

**********************************************************************/

#include "emu.h"
#include "74259.h"

#define LOG_ALL_WRITES          (0)
#define LOG_UNDEFINED_WRITES    (0)
#define LOG_MYSTERY_BITS        (0)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(LS259, ls259_device, "ls259", "74LS259 Addressable Latch")
DEFINE_DEVICE_TYPE(HC259, hc259_device, "hc259", "74HC259 Addressable Latch")
DEFINE_DEVICE_TYPE(HCT259, hct259_device, "hct259", "74HCT259 Addressable Latch")
DEFINE_DEVICE_TYPE(F9334, f9334_device, "f9334", "Fairchild 9334 Addressable Latch")
DEFINE_DEVICE_TYPE(CD4099, cd4099_device, "cd4099", "CD4099B Addressable Latch")

//**************************************************************************
//  ADDRESSABLE LATCH DEVICE
//**************************************************************************

addressable_latch_device::addressable_latch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool clear_active)
	: device_t(mconfig, type, tag, owner, clock),
		m_q_out_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}},
		m_parallel_out_cb(*this),
		m_clear_active(clear_active)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void addressable_latch_device::device_start()
{
	// resolve callbacks
	for (devcb_write_line &cb : m_q_out_cb)
		cb.resolve();
	m_parallel_out_cb.resolve();

	// initial input state
	m_address = 0;
	m_data = false;
	m_enable = false;
	m_clear = false;

	// arbitrary initial output state
	m_q = 0xff;

	save_item(NAME(m_address));
	save_item(NAME(m_data));
	save_item(NAME(m_enable));
	save_item(NAME(m_q));
	save_item(NAME(m_clear));
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void addressable_latch_device::device_reset()
{
	// assume clear upon reset
	clear_outputs(m_enable ? u8(m_data) << m_address : 0);
}

//-------------------------------------------------
//  write_bit - synchronously update one of the
//  eight output lines with a new data bit
//-------------------------------------------------

void addressable_latch_device::write_bit(offs_t offset, bool d)
{
	write_abcd(offset, d);
	enable_w(0);
	enable_w(1);
}

//-------------------------------------------------
//  write_abcd - update address select and data
//  inputs without changing enable state
//-------------------------------------------------

void addressable_latch_device::write_abcd(u8 a, bool d)
{
	m_address = a & 7;
	m_data = d;

	if (m_enable)
	{
		if (m_clear)
			clear_outputs(u8(m_data) << m_address);
		else
			update_bit();
	}
}

//-------------------------------------------------
//  enable_w - handle enable input (active low)
//-------------------------------------------------

WRITE_LINE_MEMBER(addressable_latch_device::enable_w)
{
	m_enable = !state;
	if (m_enable)
		update_bit();
	else if (m_clear)
		clear_outputs(0);
}

//-------------------------------------------------
//  update_bit - update one of the eight output
//  lines with a new data bit
//-------------------------------------------------

void addressable_latch_device::update_bit()
{
	// first verify that the selected bit is actually changing
	if (BIT(m_q, m_address) == m_data)
		return;

	if (!m_clear)
	{
		// update selected bit with new data
		m_q = (m_q & ~(1 << m_address)) | (u8(m_data) << m_address);
	}
	else
	{
		// clear any other bit that was formerly set
		clear_outputs(0);
		m_q = u8(m_data) << m_address;
	}

	// update output line via callback
	if (!m_q_out_cb[m_address].isnull())
		m_q_out_cb[m_address](m_data);

	// update parallel output
	if (!m_parallel_out_cb.isnull())
		m_parallel_out_cb(0, m_q, 1 << m_address);

	// do some logging
	if (LOG_ALL_WRITES || (LOG_UNDEFINED_WRITES && m_q_out_cb[m_address].isnull() && m_parallel_out_cb.isnull()))
		logerror("Q%d %s at %s\n", m_address, m_data ? "set" : "cleared", machine().describe_context());
}

//-------------------------------------------------
//  write_d0 - bus-triggered write handler using
//  LSB of data (or CRUOUT on TMS99xx)
//-------------------------------------------------

void addressable_latch_device::write_d0(offs_t offset, u8 data)
{
	if (LOG_MYSTERY_BITS && data != 0x00 && data != 0x01 && data != 0xff)
		logerror("Mystery bits written to Q%d:%s%s%s%s%s%s%s\n",
			offset,
			BIT(data, 7) ? " D7" : "",
			BIT(data, 6) ? " D6" : "",
			BIT(data, 5) ? " D5" : "",
			BIT(data, 4) ? " D4" : "",
			BIT(data, 3) ? " D3" : "",
			BIT(data, 2) ? " D2" : "",
			BIT(data, 1) ? " D1" : "");

	write_bit(offset, BIT(data, 0));
}

//-------------------------------------------------
//  write_d1 - bus-triggered write handler using
//  second-lowest data bit
//-------------------------------------------------

void addressable_latch_device::write_d1(offs_t offset, u8 data)
{
	if (LOG_MYSTERY_BITS && data != 0x00 && data != 0x02 && data != 0xff)
		logerror("Mystery bits written to Q%d:%s%s%s%s%s%s%s\n",
			offset,
			BIT(data, 7) ? " D7" : "",
			BIT(data, 6) ? " D6" : "",
			BIT(data, 5) ? " D5" : "",
			BIT(data, 4) ? " D4" : "",
			BIT(data, 3) ? " D3" : "",
			BIT(data, 2) ? " D2" : "",
			BIT(data, 0) ? " D0" : "");

	write_bit(offset, BIT(data, 1));
}

//-------------------------------------------------
//  write_d7 - bus-triggered write handler using
//  MSB of (8-bit) data
//-------------------------------------------------

void addressable_latch_device::write_d7(offs_t offset, u8 data)
{
	if (LOG_MYSTERY_BITS && data != 0x00 && data != 0x80 && data != 0xff)
		logerror("Mystery bits written to Q%d:%s%s%s%s%s%s%s\n",
			offset,
			BIT(data, 6) ? " D6" : "",
			BIT(data, 5) ? " D5" : "",
			BIT(data, 4) ? " D4" : "",
			BIT(data, 3) ? " D3" : "",
			BIT(data, 2) ? " D2" : "",
			BIT(data, 1) ? " D1" : "",
			BIT(data, 0) ? " D0" : "");

	write_bit(offset, BIT(data, 7));
}

//-------------------------------------------------
//  write_a0 - write handler that uses lowest bit
//  of address bus as data input (data on bus is
//  ignored)
//-------------------------------------------------

void addressable_latch_device::write_a0(offs_t offset, u8 data)
{
	write_bit(offset >> 1, offset & 1);
}

//-------------------------------------------------
//  write_a3 - write handler that uses three
//  lowest bits of address bus as address and
//  fourth lowest as data input
//-------------------------------------------------

void addressable_latch_device::write_a3(offs_t offset, u8 data)
{
	write_bit(offset & 7, (offset & 8) >> 3);
}

//-------------------------------------------------
//  write_nibble_d0 - write handler using LSB of
//  data as input and next three bits as address
//  (offset is ignored)
//-------------------------------------------------

void addressable_latch_device::write_nibble_d0(u8 data)
{
	write_bit((data & 0x0e) >> 1, data & 0x01);
}

//-------------------------------------------------
//  write_nibble_d3 - write handler using bit 3 of
//  data as input and lowest three bits as address
//  (offset is ignored)
//-------------------------------------------------

void addressable_latch_device::write_nibble_d3(u8 data)
{
	write_bit(data & 0x07, BIT(data, 3));
}

//-------------------------------------------------
//  clear - pulse clear line from bus write
//-------------------------------------------------

void addressable_latch_device::clear(u8 data)
{
	clear_outputs(m_enable ? u8(m_data) << m_address : 0);
}

//-------------------------------------------------
//  clear_w - handle clear/reset input
//-------------------------------------------------

WRITE_LINE_MEMBER(addressable_latch_device::clear_w)
{
	m_clear = bool(state) == m_clear_active;
	if (m_clear)
		clear_outputs(m_enable ? u8(m_data) << m_address : 0);
}

//-------------------------------------------------
//  clear_outputs - clear all output lines
//-------------------------------------------------

void addressable_latch_device::clear_outputs(u8 new_q)
{
	const u8 bits_changed = m_q ^ new_q;
	if (bits_changed == 0)
		return;

	m_q = new_q;

	// return any previously set output lines to clear state
	for (int bit = 0; bit < 8; bit++)
		if (BIT(bits_changed, bit) && !m_q_out_cb[bit].isnull())
			m_q_out_cb[bit](BIT(new_q, bit));

	// update parallel output
	if (!m_parallel_out_cb.isnull())
		m_parallel_out_cb(0, new_q, bits_changed);
}

//**************************************************************************
//  LS259 DEVICE
//**************************************************************************

ls259_device::ls259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: addressable_latch_device(mconfig, LS259, tag, owner, clock, false)
{
}

//**************************************************************************
//  HC259 DEVICE
//**************************************************************************

hc259_device::hc259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: addressable_latch_device(mconfig, HC259, tag, owner, clock, false)
{
}

//**************************************************************************
//  HCT259 DEVICE
//**************************************************************************

hct259_device::hct259_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: addressable_latch_device(mconfig, HCT259, tag, owner, clock, false)
{
}

//**************************************************************************
//  F9334 DEVICE
//**************************************************************************

f9334_device::f9334_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: addressable_latch_device(mconfig, F9334, tag, owner, clock, false)
{
}

//**************************************************************************
//  CD4099 DEVICE
//**************************************************************************

cd4099_device::cd4099_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: addressable_latch_device(mconfig, CD4099, tag, owner, clock, true)
{
}
