// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Decision Data IS-48x/LM-48xC IBM-compatible coax workstation displays.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/bcp/dp8344.h"
//#include "machine/eeprompar.h"
//#include "video/mc6845.h"
//#include "screen.h"

class is48x_state : public driver_device
{
public:
	is48x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bcp(*this, "bcp")
		, m_bcpram(*this, "bcpram")
	{ }

	void is482(machine_config &config);

private:
	u8 bcpram_r(offs_t offset);
	void bcpram_w(offs_t offset, u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void bcp_inst_map(address_map &map);
	void bcp_data_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<dp8344_device> m_bcp;
	required_shared_ptr<u16> m_bcpram;
};

u8 is48x_state::bcpram_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_bcpram[offset >> 1] >> 8;
	else
		return m_bcpram[offset >> 1] & 0x00ff;
}

void is48x_state::bcpram_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_bcpram[offset >> 1] = (m_bcpram[offset >> 1] & 0x00ff) | data << 8;
	else
		m_bcpram[offset >> 1] = (m_bcpram[offset >> 1] & 0xff00) | data;
}

void is48x_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x40000, 0x47fff).rw(FUNC(is48x_state::bcpram_r), FUNC(is48x_state::bcpram_w));
	map(0x50000, 0x51fff).ram();
	map(0x54000, 0x55fff).ram();
	map(0x60022, 0x60022).nopr();
	map(0x61ffa, 0x61ffa).nopr();
	map(0x80000, 0xfffff).rom().region("program", 0);
}

void is48x_state::io_map(address_map &map)
{
	map(0x8005, 0x8005).nopw();
	//map(0x8080, 0x8080).w("crtc", FUNC(mc6845_device::address_w));
	//map(0x8081, 0x8081).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8101, 0x8101).nopr();
	map(0x8180, 0x8180).ram();
}

void is48x_state::bcp_inst_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("bcpram");
}

void is48x_state::bcp_data_map(address_map &map)
{
}

static INPUT_PORTS_START(is482)
INPUT_PORTS_END

void is48x_state::is482(machine_config &config)
{
	I80188(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &is48x_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &is48x_state::io_map);

	DP8344(config, m_bcp, 18.867_MHz_XTAL);
	m_bcp->set_addrmap(AS_PROGRAM, &is48x_state::bcp_inst_map);
	m_bcp->set_addrmap(AS_DATA, &is48x_state::bcp_data_map);
}

ROM_START(is482) // "IS-488-A" on case
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("is-482_u67_s008533243.bin", 0x00000, 0x80000, CRC(1e23ac17) SHA1(aadc73bc0454c5b1c33d440dc511009dc6b7f9e0))
ROM_END

COMP(199?, is482, 0, 0, is482, is482, is48x_state, empty_init, "Decision Data", "IS-482 Workstation", MACHINE_IS_SKELETON)
