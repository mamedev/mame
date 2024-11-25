// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

QT Computer Systems SBC +2/4

This looks like the same system as the Compu/Time SBC-880 Processor Board.

Currently it crashes. There's a memory move routine at 50A4, and after
a few turns it is told to move E603 bytes which corrupts everything.

Chips: P8251, D8253C, MK3880N-4 (Z80). 3x 6-sw dips. Unmarked crystal.
       EPROM: D2716-2; RAM: 2x MM2114N-3L.

A blue jumper marked 4M and 2M (between U11 and U12) selects the CPU clock.

The RS232 port uses a 26-pin right-angle header (J1) rather than the
conventional DB25 connector. The second 26-pin header (J2) mostly carries
data and handshake signals for two unidirectional parallel ports, but also
includes a few timer outputs.

Feature list from QT ad:
- 1K RAM (which can be located at any 1K boundary) plus one each
  Parallel and Serial I/O ports on board
- Power on jump to onboard EPROM (2708 or 2716)
- EPROM addressable on any 1K or 2K boundary
- Full 64K use of RAM allowed in shadow mode
- Programmable Baud rate selection, 110-9600
- 2 or 4MHz switch selectable
- DMA capability allows MWRT signal generation on CPU board or elsewhere
  in system under DMA logic or front panel control
- Two programmable timers available for use by programs run with the
  SBC+2/4 (timer output and controls available at parallel I/O connector;
  parallel input and output ports available for use on CPU board).

List of signals on pin headers (from CompuTime manual):

        J1                                  J2

     2  RS232 Transmit Data              1  Output Port Data Bit 0
     3  RS232 Receive Data               2  Output Port Data Bit 1
     4  Request to Send                  3  Output Port Data Bit 2
     5  Clear to Send                    4  Output Port Data Bit 3
     6  Data Set Ready                   5  Output Port Data Bit 4
     7  Signal Ground                    6  Output Port Data Bit 5
     8  Carrier Detect                   7  Output Port Data Bit 6
    11  Reverse Channel Transmit         8  Output Port Data Bit 7
    20  Data Terminal Ready              9  Signal Ground
                                        10  Output Port Clock
                                        11  Counter 1 Gate Input
                                        12  Counter 2 Gate Input
                                        14  Input Port Data Bit 0
                                        15  Input Port Data Bit 1
                                        16  Input Port Data Bit 2
                                        17  Input Port Data Bit 3
                                        18  Input Port Data Bit 4
                                        19  Input Port Data Bit 5
                                        20  Input Port Data Bit 6
                                        21  Input Port Data Bit 7
                                        22  Signal Ground
                                        23  Input Port Strobe
                                        24  Counter 1 Output
                                        25  Counter 2 Output

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"


namespace {

class qtsbc_state : public driver_device
{
public:
	qtsbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_usart(*this, "usart")
		, m_rs232(*this, "rs232")
		, m_dsw(*this, "DSW%u", 1)
		, m_jumpers(*this, "JUMPERS")
		, m_cpu_speed(*this, "SPEED")
		, m_eprom(*this, "maincpu")
		, m_p_ram(*this, "ram")
	{ }

	void qtsbc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 memory_r(offs_t offset);
	void memory_w(offs_t offset, u8 data);
	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);
	void rts_loopback_w(int state);
	void dtr_loopback_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<i8251_device> m_usart;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<3> m_dsw;
	required_ioport m_jumpers;
	required_ioport m_cpu_speed;
	required_region_ptr<u8> m_eprom;
	required_shared_ptr<u8> m_p_ram;
	bool m_power_on = false;
	s32 m_rts = 1;
	s32 m_dtr = 1;
};


u8 qtsbc_state::memory_r(offs_t offset)
{
	ioport_value jumpers = m_jumpers->read();
	ioport_value dsw3 = m_dsw[2]->read();

	bool eprom_addressed = (offset & (BIT(jumpers, 2) ? 0xf800 : 0xfc00)) >> 10 == dsw3;
	if (eprom_addressed && !machine().side_effects_disabled())
		m_power_on = false;

	if ((m_power_on && !BIT(jumpers, 1)) || (eprom_addressed && !BIT(jumpers, 0)))
	{
		offset &= BIT(jumpers, 2) ? 0x07ff : 0x03ff;
		return m_eprom[offset];
	}
#ifdef NOT_IMPLEMENTED_CURRENTLY
	else if ((offset & 0xfc00) >> 10 == m_dsw[1]->read())
	{
		offset &= 0x03ff;
		return m_ram[offset];
	}
#endif
	else
	{
		if (offset == 0xe377)
			return 0x80;

		// TODO: S-100 bus
		return m_p_ram[offset];
	}
}

void qtsbc_state::memory_w(offs_t offset, u8 data)
{
#ifdef NOT_IMPLEMENTED_CURRENTLY
	if ((offset & 0xfc00) >> 10 == m_dsw[1]->read())
	{
		offset &= 0x03ff;
		m_ram[offset] = data;
	}
	else
#endif
	{
		// TODO: S-100 bus
		m_p_ram[offset] = data;
	}
}

u8 qtsbc_state::io_r(offs_t offset)
{
	if ((offset & 0xf8) >> 3 == (m_dsw[0]->read() & 0x1f))
	{
		switch (offset & 7)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		default:
			return m_pit->read(offset & 3);

		case 4:
		case 5:
			// TODO: 8-bit parallel input port
			return 0xff;

		case 6:
		case 7:
			return m_usart->read(offset & 1);
		}
	}
	else
	{
		// TODO: S-100 bus (no address mirroring)
		logerror("Input from %04X\n", offset);

		// Ports 40 and 43 are for some sort of off-board RAM
		// Bit 0 of 43 is some sort of busy signal; bits 1-3 select some memory size factor
		if ((offset & 0xff) == 0x43)
			return 0x00;

		// Ports 08, 10 and 80 also used for read access
		return 0xff;
	}
}

void qtsbc_state::io_w(offs_t offset, u8 data)
{
	if ((offset & 0x00f8) >> 3 == (m_dsw[0]->read() & 0x1f))
	{
		switch (offset & 7)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		default:
			m_pit->write(offset & 3, data);
			break;

		case 4:
		case 5:
			// TODO: 8-bit parallel output port
			break;

		case 6:
		case 7:
			m_usart->write(offset & 1, data);
			break;
		}
	}
	else
	{
		// TODO: S-100 bus (no address mirroring)
		logerror("Output %02X to %04X\n", data, offset);
	}
}

void qtsbc_state::rts_loopback_w(int state)
{
	// Filtered through this routine to avoid infinite loops
	if (state != m_rts)
	{
		m_rts = state;
		m_rs232->write_rts(m_rts);
	}
}

void qtsbc_state::dtr_loopback_w(int state)
{
	// Filtered through this routine to avoid infinite loops
	if (state != m_dtr)
	{
		m_dtr = state;
		m_rs232->write_dtr(m_dtr);
	}
}

void qtsbc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("ram");
	map(0x0000, 0xffff).rw(FUNC(qtsbc_state::memory_r), FUNC(qtsbc_state::memory_w));
}

void qtsbc_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(qtsbc_state::io_r), FUNC(qtsbc_state::io_w));
}

/* Input ports */
static INPUT_PORTS_START( qtsbc )
	PORT_START("DSW1")
	PORT_DIPNAME(0x1f, 0x00, "On-Board I/O Ports") PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(0x00, "00-07")
	PORT_DIPSETTING(0x01, "08-0F")
	PORT_DIPSETTING(0x02, "10-17")
	PORT_DIPSETTING(0x03, "18-1F")
	PORT_DIPSETTING(0x04, "20-27")
	PORT_DIPSETTING(0x05, "28-2F")
	PORT_DIPSETTING(0x06, "30-37")
	PORT_DIPSETTING(0x07, "38-3F")
	PORT_DIPSETTING(0x08, "40-47")
	PORT_DIPSETTING(0x09, "48-4F")
	PORT_DIPSETTING(0x0a, "50-57")
	PORT_DIPSETTING(0x0b, "58-5F")
	PORT_DIPSETTING(0x0c, "60-67")
	PORT_DIPSETTING(0x0d, "68-6F")
	PORT_DIPSETTING(0x0e, "70-77")
	PORT_DIPSETTING(0x0f, "78-7F")
	PORT_DIPSETTING(0x10, "80-87")
	PORT_DIPSETTING(0x11, "88-8F")
	PORT_DIPSETTING(0x12, "90-87")
	PORT_DIPSETTING(0x13, "98-9F")
	PORT_DIPSETTING(0x14, "A0-A7")
	PORT_DIPSETTING(0x15, "A8-AF")
	PORT_DIPSETTING(0x16, "B0-B7")
	PORT_DIPSETTING(0x17, "B8-BF")
	PORT_DIPSETTING(0x18, "C0-C7")
	PORT_DIPSETTING(0x19, "C8-CF")
	PORT_DIPSETTING(0x1a, "D0-D7")
	PORT_DIPSETTING(0x1b, "D8-DF")
	PORT_DIPSETTING(0x1c, "E0-E7")
	PORT_DIPSETTING(0x1d, "E8-EF")
	PORT_DIPSETTING(0x1e, "F0-F7")
	PORT_DIPSETTING(0x1f, "F8-FF")
	PORT_DIPUNUSED(0x20, 0x20) PORT_DIPLOCATION("SW1:6")

	PORT_START("DSW2")
	PORT_DIPNAME(0x3f, 0x37, "On-Board RAM Address") PORT_DIPLOCATION("SW2:6,5,4,3,2,1")
	PORT_DIPSETTING(0x00, "0000-03FF")
	PORT_DIPSETTING(0x01, "0400-07FF")
	PORT_DIPSETTING(0x02, "0800-0BFF")
	PORT_DIPSETTING(0x03, "0C00-0FFF")
	PORT_DIPSETTING(0x04, "1000-13FF")
	PORT_DIPSETTING(0x05, "1400-17FF")
	PORT_DIPSETTING(0x06, "1800-1BFF")
	PORT_DIPSETTING(0x07, "1C00-1FFF")
	PORT_DIPSETTING(0x08, "2000-23FF")
	PORT_DIPSETTING(0x09, "2400-27FF")
	PORT_DIPSETTING(0x0a, "2800-2BFF")
	PORT_DIPSETTING(0x0b, "2C00-2FFF")
	PORT_DIPSETTING(0x0c, "3000-33FF")
	PORT_DIPSETTING(0x0d, "3400-37FF")
	PORT_DIPSETTING(0x0e, "3800-3BFF")
	PORT_DIPSETTING(0x0f, "3C00-3FFF")
	PORT_DIPSETTING(0x10, "4000-43FF")
	PORT_DIPSETTING(0x11, "4400-47FF")
	PORT_DIPSETTING(0x12, "4800-4BFF")
	PORT_DIPSETTING(0x13, "4C00-4FFF")
	PORT_DIPSETTING(0x14, "5000-53FF")
	PORT_DIPSETTING(0x15, "5400-57FF")
	PORT_DIPSETTING(0x16, "5800-5BFF")
	PORT_DIPSETTING(0x17, "5C00-5FFF")
	PORT_DIPSETTING(0x18, "6000-63FF")
	PORT_DIPSETTING(0x19, "6400-67FF")
	PORT_DIPSETTING(0x1a, "6800-6BFF")
	PORT_DIPSETTING(0x1b, "6C00-6FFF")
	PORT_DIPSETTING(0x1c, "7000-73FF")
	PORT_DIPSETTING(0x1d, "7400-77FF")
	PORT_DIPSETTING(0x1e, "7800-7BFF")
	PORT_DIPSETTING(0x1f, "7C00-7FFF")
	PORT_DIPSETTING(0x20, "8000-83FF")
	PORT_DIPSETTING(0x21, "8400-87FF")
	PORT_DIPSETTING(0x22, "8800-8BFF")
	PORT_DIPSETTING(0x23, "8C00-8FFF")
	PORT_DIPSETTING(0x24, "9000-93FF")
	PORT_DIPSETTING(0x25, "9400-97FF")
	PORT_DIPSETTING(0x26, "9800-9BFF")
	PORT_DIPSETTING(0x27, "9C00-9FFF")
	PORT_DIPSETTING(0x28, "A000-A3FF")
	PORT_DIPSETTING(0x29, "A400-A7FF")
	PORT_DIPSETTING(0x2a, "A800-ABFF")
	PORT_DIPSETTING(0x2b, "AC00-AFFF")
	PORT_DIPSETTING(0x2c, "B000-B3FF")
	PORT_DIPSETTING(0x2d, "B400-B7FF")
	PORT_DIPSETTING(0x2e, "B800-BBFF")
	PORT_DIPSETTING(0x2f, "BC00-BFFF")
	PORT_DIPSETTING(0x30, "C000-C3FF")
	PORT_DIPSETTING(0x31, "C400-C7FF")
	PORT_DIPSETTING(0x32, "C800-CBFF")
	PORT_DIPSETTING(0x33, "CC00-CFFF")
	PORT_DIPSETTING(0x34, "D000-D3FF")
	PORT_DIPSETTING(0x35, "D400-D7FF")
	PORT_DIPSETTING(0x36, "D800-DBFF")
	PORT_DIPSETTING(0x37, "DC00-DFFF")
	PORT_DIPSETTING(0x38, "E000-E3FF")
	PORT_DIPSETTING(0x39, "E400-E7FF")
	PORT_DIPSETTING(0x3a, "E800-EBFF")
	PORT_DIPSETTING(0x3b, "EC00-EFFF")
	PORT_DIPSETTING(0x3c, "F000-F3FF")
	PORT_DIPSETTING(0x3d, "F400-F7FF")
	PORT_DIPSETTING(0x3e, "F800-FBFF")
	PORT_DIPSETTING(0x3f, "FC00-FFFF")

	PORT_START("DSW3")
	PORT_DIPNAME(0x3f, 0x14, "On-Board EPROM Address") PORT_DIPLOCATION("SW3:6,5,4,3,2,1")
	PORT_DIPSETTING(0x00, "0000-03FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x01, "0400-07FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x02, "0800-0BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x03, "0C00-0FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x04, "1000-13FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x05, "1400-17FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x06, "1800-1BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x07, "1C00-1FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x08, "2000-23FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x09, "2400-27FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x0a, "2800-2BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x0b, "2C00-2FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x0c, "3000-33FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x0d, "3400-37FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x0e, "3800-3BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x0f, "3C00-3FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x10, "4000-43FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x11, "4400-47FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x12, "4800-4BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x13, "4C00-4FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x14, "5000-53FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x15, "5400-57FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x16, "5800-5BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x17, "5C00-5FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x18, "6000-63FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x19, "6400-67FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x1a, "6800-6BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x1b, "6C00-6FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x1c, "7000-73FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x1d, "7400-77FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x1e, "7800-7BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x1f, "7C00-7FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x20, "8000-83FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x21, "8400-87FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x22, "8800-8BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x23, "8C00-8FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x24, "9000-93FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x25, "9400-97FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x26, "9800-9BFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x27, "9C00-9FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x28, "A000-A3FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x29, "A400-A7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x2a, "A800-ABFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x2b, "AC00-AFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x2c, "B000-B3FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x2d, "B400-B7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x2e, "B800-BBFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x2f, "BC00-BFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x30, "C000-C3FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x31, "C400-C7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x32, "C800-CBFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x33, "CC00-CFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x34, "D000-D3FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x35, "D400-D7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x36, "D800-DBFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x37, "DC00-DFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x38, "E000-E3FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x39, "E400-E7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x3a, "E800-EBFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x3b, "EC00-EFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x3c, "F000-F3FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x3d, "F400-F7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x3e, "F800-FBFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x3f, "FC00-FFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(0x00, "0000-07FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x02, "0800-0FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x04, "1000-17FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x06, "1800-1FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x08, "2000-27FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x0a, "2800-2FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x0c, "3000-37FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x0e, "3800-3FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x10, "4000-47FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x12, "4800-4FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x14, "5000-57FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x16, "5800-5FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x18, "6000-67FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x1a, "6800-6FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x1c, "7000-77FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x1e, "7800-7FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x20, "8000-87FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x22, "8800-8FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x24, "9000-97FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x26, "9800-9FFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x28, "A000-A7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x2a, "A800-AFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x2c, "B000-B7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x2e, "B800-BFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x30, "C000-C7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x32, "C800-CFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x34, "D000-D7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x36, "D800-DFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x38, "E000-E7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x3a, "E800-EFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x3c, "F000-F7FF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(0x3e, "F800-FFFF") PORT_CONDITION("JUMPERS", 0x04, EQUALS, 0x04)

	PORT_START("JUMPERS")
	PORT_DIPNAME(0x02, 0x00, "Power-On Jump") PORT_DIPLOCATION("T to U:1")
	PORT_DIPSETTING(0x02, DEF_STR(Off)) // T to U open
	PORT_DIPSETTING(0x00, DEF_STR(On)) // T to U shorted
	PORT_DIPNAME(0x01, 0x01, "EPROM Phantom Mode") PORT_DIPLOCATION("Q to R:1") PORT_CONDITION("JUMPERS", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(0x00, DEF_STR(Off)) // Q to R shorted
	PORT_DIPSETTING(0x01, DEF_STR(On)) // Q to R open
	PORT_DIPNAME(0x01, 0x00, "On-Board EPROM") PORT_DIPLOCATION("Q to R:1") PORT_CONDITION("JUMPERS", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(0x01, "Disabled") // Q to R open
	PORT_DIPSETTING(0x00, "Enabled") // Q to R shorted
	PORT_DIPNAME(0x04, 0x04, "On-Board EPROM Size") PORT_DIPLOCATION("P to O:1")
	PORT_DIPSETTING(0x00, "1K x 8 (2708)") // P to O shorted
	PORT_DIPSETTING(0x04, "2K x 8 (2716)") // P to O open

	PORT_START("SPEED")
	PORT_CONFNAME(0x01, 0x01, "CPU Speed")
	PORT_CONFSETTING(0x00, "2 MHz")
	PORT_CONFSETTING(0x01, "4 MHz")
INPUT_PORTS_END


void qtsbc_state::machine_start()
{
	m_usart->write_cts(0);

	save_item(NAME(m_power_on));
	save_item(NAME(m_rts));
	save_item(NAME(m_dtr));
}

void qtsbc_state::machine_reset()
{
	m_power_on = true;
	m_maincpu->set_unscaled_clock(BIT(m_cpu_speed->read(), 0) ? 4_MHz_XTAL : 4_MHz_XTAL / 2);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void qtsbc_state::qtsbc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL); // Mostek MK3880
	m_maincpu->set_addrmap(AS_PROGRAM, &qtsbc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &qtsbc_state::io_map);

	/* video hardware */
	PIT8253(config, m_pit); // U9
	m_pit->set_clk<0>(4_MHz_XTAL / 2); /* Timer 0: baud rate gen for 8251 */
	m_pit->out_handler<0>().set(m_usart, FUNC(i8251_device::write_txc));
	m_pit->out_handler<0>().append(m_usart, FUNC(i8251_device::write_rxc));
	m_pit->set_clk<1>(4_MHz_XTAL / 2);
	m_pit->out_handler<1>().set(m_pit, FUNC(pit8253_device::write_clk2));

	I8251(config, m_usart, 0); // U8
	m_usart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr)); // actually from pin 11, "Reverse Channel Transmit"
	m_rs232->cts_handler().set(FUNC(qtsbc_state::rts_loopback_w));
	m_rs232->dcd_handler().set(FUNC(qtsbc_state::dtr_loopback_w));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

/* ROM definition */
ROM_START( qtsbc )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "qtsbc.u23", 0x0000, 0x0800, CRC(823fd942) SHA1(64c4f74dd069ae4d43d301f5e279185f32a1efa0))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                     FULLNAME     FLAGS
COMP( 198?, qtsbc, 0,      0,      qtsbc,   qtsbc, qtsbc_state, empty_init, "QT Computer Systems Inc.", "SBC + 2/4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
