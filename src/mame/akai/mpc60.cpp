// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Akai MPC60 MIDI Production Center.

***************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/i86/i186.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/upd765.h"
#include "video/hd61830.h"
#include "emupal.h"
#include "screen.h"

namespace {

class mpc60_state : public driver_device
{
public:
	mpc60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_panelcpu(*this, "panelcpu")
		, m_fdc(*this, "fdc")
	{
	}

	void mpc60(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	void fdc_tc_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void panel_map(address_map &map) ATTR_COLD;
	void lcd_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<upd7810_device> m_panelcpu;
	required_device<upd72065_device> m_fdc;

	std::unique_ptr<u8[]> m_nvram_data;
};

void mpc60_state::machine_start()
{
	m_nvram_data = make_unique_clear<u8[]>(0x800);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_data[0], 0x800);

	save_pointer(NAME(m_nvram_data), 0x800);
}

u8 mpc60_state::nvram_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void mpc60_state::nvram_w(offs_t offset, u8 data)
{
	m_nvram_data[offset] = data;
}

void mpc60_state::fdc_tc_w(u8 data)
{
	m_fdc->tc_w(0);
	m_fdc->tc_w(1);
}

void mpc60_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
	map(0xbf000, 0xbffff).rw(FUNC(mpc60_state::nvram_r), FUNC(mpc60_state::nvram_w)).umask16(0x00ff);
	map(0xc0000, 0xfffff).rom().region("program", 0);
}

void mpc60_state::io_map(address_map &map)
{
	map(0x0080, 0x0083).m(m_fdc, FUNC(upd72065_device::map)).umask16(0x00ff);
	map(0x0090, 0x0090).w(FUNC(mpc60_state::fdc_tc_w));
	map(0x00a0, 0x00a0).rw(m_fdc, FUNC(upd72065_device::dma_r), FUNC(upd72065_device::dma_w));
	map(0x00b0, 0x00b0).r("lcdc", FUNC(hd61830_device::status_r));
	map(0x00b2, 0x00b2).r("lcdc", FUNC(hd61830_device::data_r));
	map(0x00b4, 0x00b4).w("lcdc", FUNC(hd61830_device::control_w));
	map(0x00b6, 0x00b6).w("lcdc", FUNC(hd61830_device::data_w));
	map(0x0200, 0x0207).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

void mpc60_state::panel_map(address_map &map)
{
	map(0x4000, 0x5fff).rom().region("panel", 0);
}

void mpc60_state::lcd_map(address_map &map)
{
	map.global_mask(0x07ff);
	map(0x0000, 0x07ff).ram();
}


static INPUT_PORTS_START(mpc60)
INPUT_PORTS_END

void mpc60_state::mpc60(machine_config &config)
{
	I80186(config, m_maincpu, 20_MHz_XTAL); // MBL80186-10
	m_maincpu->set_addrmap(AS_PROGRAM, &mpc60_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mpc60_state::io_map);

	CLOCK(config, "tmrck", 20_MHz_XTAL / 10).signal_handler().set(m_maincpu, FUNC(i80186_cpu_device::tmrin1_w)); // CPUCK output divided by 74HC390

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK5816PN-15L + CL2020 battery

	UPD78C11(config, m_panelcpu, 12_MHz_XTAL);
	m_panelcpu->set_addrmap(AS_PROGRAM, &mpc60_state::panel_map);

	//MB89371(config, "sio01", 20_MHz_XTAL / 4);
	//MB89371(config, "sio23", 20_MHz_XTAL / 4);

	I8255(config, "ppi"); // MB89255A-P-C

	UPD72065(config, m_fdc, 16_MHz_XTAL / 4); // μPD72066C (clocked by SED9420CAC)
	m_fdc->set_ready_line_connected(false); // RDY tied to VDD (TODO: drive ready signal connected to PPI's PB2 instead)
	m_fdc->set_select_lines_connected(false);
	m_fdc->intrq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w)); // FIXME: delayed and combined with DRQAD

	hd61830_device &lcdc(HD61830(config, "lcdc", 0)); // LC7981
	lcdc.set_addrmap(0, &mpc60_state::lcd_map);
	lcdc.set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update("lcdc", FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	//L4003(config, "voicelsi", 35.84_MHz_XTAL);
}

ROM_START(mpc60)
	ROM_REGION16_LE(0x40000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v212", "v2.12") // V2.12 CPU ROMs (MBM27C512-20)
	ROMX_LOAD("mp6cpu2.ic2", 0x00000, 0x10000, CRC(e71b1acb) SHA1(b56ddfff1c546fc21341b1a614e18da9726312f4), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("mp6cpu3.ic3", 0x00001, 0x10000, CRC(f068838b) SHA1(42e815880d1c1a5b7d1c7933aad9c28410fc2627), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("mp6cpu1.ic4", 0x20000, 0x10000, CRC(1271bc73) SHA1(99fd6fa4c04e5bdf868e78072fec5b55c01350da), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("mpc6cpu4.ic5", 0x20001, 0x10000, CRC(d922a66d) SHA1(0f4bc0522b9826d617f4af72382d75853515d7f5), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v112", "v1.12")
	ROMX_LOAD("mpc60_v1-12_2.ic2", 0x00000, 0x10000, CRC(ddf26146) SHA1(987547198dc3984ab3dfa7f133ba7dca702cc269), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("mpc60_v1-12_4.ic3", 0x00001, 0x10000, CRC(9725d193) SHA1(6efda3d6760b3951c5036108106d446f6e128c59), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("mpc60_v1-12_1.ic4", 0x20000, 0x10000, CRC(f202dbb1) SHA1(6fd82224a99b52b6c414b88d5c920abda32ffa32), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("mpc60_v1-12_3.ic5", 0x20001, 0x10000, CRC(ba5a1640) SHA1(1f9f49c49a3682b9a44d614ac411a7c043df399e), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION16_LE(0x10000, "waves", 0)
	ROM_LOAD16_BYTE("mpc60_voice_1_v1-0.ic17", 0x00000, 0x08000, CRC(b8fdfe3e) SHA1(c2f0e1d8813d4178d2f883a3f3e461e036b56229)) // lowest nibble is unused
	ROM_LOAD16_BYTE("mpc60_voice_2_v1-0.ic18", 0x00001, 0x08000, CRC(42f8e0a6) SHA1(a22dbefb9dafbb0c4095fd0bf4e63e67b5ec3b95))

	ROM_REGION(0x1000, "panelcpu", 0)
	ROM_LOAD("upd78c11g-044-36.ic1", 0x0000, 0x1000, NO_DUMP)
	ROM_FILL(0x0000, 1, 0x54) // dummy reset vector
	ROM_FILL(0x0001, 1, 0x00)
	ROM_FILL(0x0002, 1, 0x40)
	ROM_FILL(0x0018, 1, 0x54) // dummy interrupt vector
	ROM_FILL(0x0019, 1, 0x18)
	ROM_FILL(0x001a, 1, 0x40)
	ROM_FILL(0x0090, 1, 0xca) // dummy CALT vectors
	ROM_FILL(0x0091, 1, 0x41)
	ROM_FILL(0x0092, 1, 0xca)
	ROM_FILL(0x0093, 1, 0x41)
	ROM_FILL(0x0094, 1, 0xca)
	ROM_FILL(0x0095, 1, 0x41)
	ROM_FILL(0x0096, 1, 0xca)
	ROM_FILL(0x0097, 1, 0x41)
	ROM_FILL(0x0098, 1, 0xca)
	ROM_FILL(0x0099, 1, 0x41)
	ROM_FILL(0x009a, 1, 0xca)
	ROM_FILL(0x009b, 1, 0x41)

	ROM_REGION(0x2000, "panel", 0)
	ROM_LOAD("akai mpc60 panel eprom op v1-1 2764.ic2", 0x0000, 0x2000, CRC(f1332f47) SHA1(dd5e917d16941fce3db4bfe21d37f722d6262561))
ROM_END

} // anonymous namespace

SYST(1987, mpc60, 0, 0, mpc60, mpc60, mpc60_state, empty_init, "Akai Electric", "MPC60 MIDI Production Center", MACHINE_IS_SKELETON)
