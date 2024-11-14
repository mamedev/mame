// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for XBase 09 drum machine by JoMoX GmbH.

****************************************************************************/

#include "emu.h"
#include "cpu/pic17/pic17c4x.h"
#include "machine/nvram.h"


namespace {

class xbase09_state : public driver_device
{
public:
	xbase09_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_analog_ports(*this, "ANALOG%02X", 0U)
		, m_port_select(0)
	{
	}

	void xbase09(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void portb_w(u8 data);
	u16 ram_r(offs_t offset);
	void ram_w(offs_t offset, u16 data);
	void mem_map(address_map &map) ATTR_COLD;

	required_device<pic17c4x_device> m_maincpu;
	required_ioport_array<0x16> m_analog_ports;

	std::unique_ptr<u8[]> m_nvram_data;
	u8 m_port_select;
};

void xbase09_state::machine_start()
{
	m_nvram_data = std::make_unique<u8[]>(0x8000);
	subdevice<nvram_device>("nvram")->set_base(m_nvram_data.get(), 0x8000);

	save_pointer(NAME(m_nvram_data), 0x8000);
	save_item(NAME(m_port_select));
}


void xbase09_state::portb_w(u8 data)
{
	m_port_select = data >> 3;
}

u16 xbase09_state::ram_r(offs_t offset)
{
	return m_nvram_data[offset] | 0xff00;
}

void xbase09_state::ram_w(offs_t offset, u16 data)
{
	switch (offset)
	{
	case 0x0007:
		// Successive-approximation ADC implemented using software algorithm and LM393 comparator
		if (m_port_select < 0x16 && m_analog_ports[m_port_select]->read() < (data >> 8))
			m_maincpu->set_input_line(pic17c4x_device::RA0_LINE, ASSERT_LINE);
		else
			m_maincpu->set_input_line(pic17c4x_device::RA0_LINE, CLEAR_LINE);
		break;

	default:
		if ((data & 0xff00) != 0)
			logerror("%s: Writing %04X to %04X\n", machine().describe_context(), data, offset + 0x8000);
		break;
	}

	m_nvram_data[offset] = data & 0x00ff;
}


void xbase09_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("firmware", 0);
	map(0x8000, 0xffff).rw(FUNC(xbase09_state::ram_r), FUNC(xbase09_state::ram_w));
}


static INPUT_PORTS_START(xbase09)
	PORT_START("ANALOG00")
	PORT_BIT(0xff, 0x00, IPT_UNKNOWN)

	PORT_START("ANALOG01")
	PORT_BIT(0xff, 0x01, IPT_UNKNOWN)

	PORT_START("ANALOG02")
	PORT_BIT(0xff, 0x02, IPT_UNKNOWN)

	PORT_START("ANALOG03")
	PORT_BIT(0xff, 0x03, IPT_UNKNOWN)

	PORT_START("ANALOG04")
	PORT_BIT(0xff, 0x04, IPT_UNKNOWN)

	PORT_START("ANALOG05")
	PORT_BIT(0xff, 0x05, IPT_UNKNOWN)

	PORT_START("ANALOG06")
	PORT_BIT(0xff, 0x06, IPT_UNKNOWN)

	PORT_START("ANALOG07")
	PORT_BIT(0xff, 0x07, IPT_UNKNOWN)

	PORT_START("ANALOG08")
	PORT_BIT(0xff, 0x08, IPT_UNKNOWN)

	PORT_START("ANALOG09")
	PORT_BIT(0xff, 0x09, IPT_UNKNOWN)

	PORT_START("ANALOG0A")
	PORT_BIT(0xff, 0x0a, IPT_UNKNOWN)

	PORT_START("ANALOG0B")
	PORT_BIT(0xff, 0x14, IPT_UNKNOWN)

	PORT_START("ANALOG0C")
	PORT_BIT(0xff, 0x1e, IPT_UNKNOWN)

	PORT_START("ANALOG0D")
	PORT_BIT(0xff, 0x28, IPT_UNKNOWN)

	PORT_START("ANALOG0E")
	PORT_BIT(0xff, 0x32, IPT_UNKNOWN)

	PORT_START("ANALOG0F")
	PORT_BIT(0xff, 0x3c, IPT_UNKNOWN)

	PORT_START("ANALOG10")
	PORT_BIT(0xff, 0x46, IPT_UNKNOWN)

	PORT_START("ANALOG11")
	PORT_BIT(0xff, 0x50, IPT_UNKNOWN)

	PORT_START("ANALOG12")
	PORT_BIT(0xff, 0x5a, IPT_UNKNOWN)

	PORT_START("ANALOG13")
	PORT_BIT(0xff, 0x64, IPT_UNKNOWN)

	PORT_START("ANALOG14")
	PORT_BIT(0xff, 0xc8, IPT_UNKNOWN)

	PORT_START("ANALOG15")
	PORT_BIT(0xff, 0xff, IPT_UNKNOWN)
INPUT_PORTS_END

void xbase09_state::xbase09(machine_config &config)
{
	PIC17C43(config, m_maincpu, 16_MHz_XTAL); // PIC17C43-16/P
	m_maincpu->set_mode(pic17c43_device::mode::MICROPROCESSOR);
	m_maincpu->set_addrmap(AS_PROGRAM, &xbase09_state::mem_map);
	m_maincpu->rb_out_cb().set(FUNC(xbase09_state::portb_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM2B256SLMX10, NEC D43256BGU-70LL or equivalent + battery

	//MAX509(config, "dac"); // MAX509BWCP (near top of PCB)
}

// IC positions are not labeled on PCB except for stickered PLDs
ROM_START(xbase09)
	ROM_REGION16_LE(0x10000, "firmware", 0)
	ROM_DEFAULT_BIOS("v209")
	ROM_SYSTEM_BIOS(0, "v132", "Version 1.32")
	ROMX_LOAD("xbase_09__1.32l.bin", 0x0000, 0x8000, CRC(30dc47c6) SHA1(ee79f9e98c06edd8a5963fcd325da3490e8e5b76), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("xbase_09__1.32h.bin", 0x0001, 0x8000, CRC(ce2df930) SHA1(56922d069b53013f2a3646dcd63fbbc9b609cf5b), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v208", "Version 2.08 (without Tempo lock)")
	ROMX_LOAD("xbase_09__2.08l.bin", 0x0000, 0x8000, CRC(c0f06ef6) SHA1(912f0b01cf5cfdc37ce61b3eacf933174f80171d), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("xbase_09__2.08h.bin", 0x0001, 0x8000, CRC(f1287888) SHA1(ee162887af1cfe968004b8c2e1c5417be42b2e9f), ROM_BIOS(1) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(2, "v209", "Version 2.09 (with Tempo lock)")
	ROMX_LOAD("xbase_09__2.09l.bin", 0x0000, 0x8000, CRC(60667060) SHA1(6a004934dd3f5f729f59bcbcb79e86e0e5f97fe9), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("xbase_09__2.09h.bin", 0x0001, 0x8000, CRC(64645c77) SHA1(35cc18774d9e4cc61af0469144c49dddfabc92b7), ROM_BIOS(2) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(3, "sx209", "SX Version 2.09") // "requires hardware update," so may need to be a separate system
	ROMX_LOAD("xbase09sx_2.09l.bin", 0x0000, 0x8000, CRC(7e87c30d) SHA1(2237f235749ae583327f1f8d893ceb3bb6502ae9), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("xbase09sx_2.09h.bin", 0x0001, 0x8000, CRC(ac9ad2e3) SHA1(8977e4520072b4ef6b603bcb0574663f10ba2629), ROM_BIOS(3) | ROM_SKIP(1))

	ROM_REGION(0x10000, "hhrom", 0)
	ROM_LOAD("xbase_09__rom_0.bin", 0x00000, 0x10000, NO_DUMP) // size unknown

	ROM_REGION(0x3000, "plds", 0)
	ROM_LOAD("xbase.ic17", 0x0000, 0x0a92, NO_DUMP) // next to CPU
	ROM_LOAD("xbase.ic59", 0x1000, 0x0a92, NO_DUMP) // right side of HH ROM
	ROM_LOAD("xbase.ic60", 0x2000, 0x0a92, NO_DUMP) // left side of HH ROM; confirmed to be a PALCE20V8H-15PC/4
ROM_END

} // anonymous namespace


SYST(1997, xbase09, 0, 0, xbase09, xbase09, xbase09_state, empty_init, "JoMoX", "XBase 09 Midi Controlled Analogue Drum Module", MACHINE_IS_SKELETON)
