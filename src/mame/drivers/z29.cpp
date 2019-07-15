// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Zenith Z-29 (alias Heathkit H-29) video terminal.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/x2212.h"
#include "video/i8275.h"
#include "screen.h"

class z29_state : public driver_device
{
public:
	z29_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc%u", 1U)
		, m_nvram(*this, "nvram")
		, m_charmem(*this, "charmem")
		, m_attrmem(*this, "attrmem")
		, m_chargen(*this, "chargen")
	{
	}

	void z29(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void p3_w(u8 data);
	DECLARE_READ8_MEMBER(bs_24k_r);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_WRITE8_MEMBER(latch_12k_w);

	void prg_map(address_map &map);
	void ext_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device_array<i8276_device, 2> m_crtc;
	required_device<x2210_device> m_nvram;

	required_shared_ptr<u8> m_charmem;
	required_shared_ptr<u8> m_attrmem;
	required_region_ptr<u8> m_chargen;

	bool m_dmatype;
};

void z29_state::machine_start()
{
	save_item(NAME(m_dmatype));
}

void z29_state::p3_w(u8 data)
{
	m_dmatype = BIT(data, 4);
}

READ8_MEMBER(z29_state::bs_24k_r)
{
	if (!machine().side_effects_disabled())
	{
		if (m_dmatype)
		{
			u8 chardata = m_charmem[offset];
			u8 attrdata = m_attrmem[offset] & 0xf;

			m_crtc[0]->dack_w(space, 0, chardata & 0x7f);
			m_crtc[1]->dack_w(space, 0, (chardata & 0x60) | (BIT(chardata, 7) ? 0x10 : 0) | attrdata);
		}
		else
		{
			m_charmem[offset] = 0x20;
			m_attrmem[offset] = 0;
		}
	}

	return m_dmatype ? 0x24 : 0x20;
}

WRITE8_MEMBER(z29_state::crtc_w)
{
	m_crtc[0]->write(space, offset, data);
	m_crtc[1]->write(space, offset, data);
}

WRITE8_MEMBER(z29_state::latch_12k_w)
{
	m_nvram->store(!BIT(data, 0));
	m_nvram->recall(!BIT(data, 3));
}

void z29_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x6000, 0x67ff).mirror(0x800).r(FUNC(z29_state::bs_24k_r));
}

void z29_state::ext_map(address_map &map)
{
	map(0x2000, 0x2001).mirror(0xffe).r("crtc1", FUNC(i8276_device::read)).w(FUNC(z29_state::crtc_w));
	map(0x3000, 0x3000).mirror(0xfff).w(FUNC(z29_state::latch_12k_w));
	map(0x4000, 0x47ff).mirror(0x800).ram().share("attrmem");
	map(0x5000, 0x57ff).mirror(0x800).ram().share("charmem");
	map(0x7000, 0x703f).mirror(0xfc0).rw("nvram", FUNC(x2210_device::read), FUNC(x2210_device::write));
}


static INPUT_PORTS_START(z29)
INPUT_PORTS_END


void z29_state::z29(machine_config &config)
{
	I8031(config, m_maincpu, 14.784_MHz_XTAL / 2); // Intel P8031AH
	m_maincpu->set_addrmap(AS_PROGRAM, &z29_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &z29_state::ext_map);
	m_maincpu->port_in_cb<1>().set_constant(0xfd); // hack around keyboard not working
	m_maincpu->port_out_cb<3>().set(FUNC(z29_state::p3_w));

	X2210(config, m_nvram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.784_MHz_XTAL, 880, 0, 640, 280, 0, 250);
	screen.set_screen_update("crtc1", FUNC(i8276_device::screen_update));

	I8276(config, m_crtc[0], 14.784_MHz_XTAL / 8).set_character_width(8);
	m_crtc[0]->drq_wr_callback().set_inputline(m_maincpu, MCS51_INT0_LINE);
	m_crtc[0]->irq_wr_callback().set_inputline(m_maincpu, MCS51_INT1_LINE);

	I8276(config, m_crtc[1], 14.784_MHz_XTAL / 8).set_character_width(8);

	I8021(config, "kbdmcu", 3.579545_MHz_XTAL).set_disable();
}


ROM_START(z29) // All EPROMs on 85-2835-1 terminal board are HN482732AG-30
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("u440.bin", 0x0000, 0x1000, CRC(169b9517) SHA1(c18b6a193655a64808e9ae8765d3e54d13e6669e)) // occupies pins 9-32 of 40-pin expansion socket
	ROM_LOAD("u407.bin", 0x1000, 0x1000, CRC(b5aae8e6) SHA1(692e521a85d7e07647c66a660faa2041d1bfd785))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("u429.bin", 0x0000, 0x1000, CRC(5e3bc5bf) SHA1(18d73e3d74a9768bee8b063ea45891f955558ae7))

	ROM_REGION(0x400, "kbdmcu", 0)
	ROM_LOAD("444-100.bin", 0x000, 0x400, NO_DUMP)
ROM_END


COMP(1983, z29, 0, 0, z29, z29, z29_state, empty_init, "Zenith Data Systems", "Z-29", MACHINE_IS_SKELETON)
