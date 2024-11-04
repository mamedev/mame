// license:BSD-3-Clause
// copyright-holders:David Shah

/*

Monkey King SoCs (currently only 3B is supported)

Presumably-custom ARM-based system-on-chips by Digital Media Cartridge (DMC).
Intended to run NES and Genesis emulators, primarily for ATgames systems.

Sometimes abbreviated MK. It is a successor of the Titan SoC used in previous
emulation based ATgames systems.

Monkey King and Monkey 2: Presumed custom. Used in some ATgames/Blaze
Genesis systems and the Atari Flashback Portable.

Monkey King 3 and Monkey King 3B: Presumed custom. Used in the ATgames
BLAST system and the RS-70 648-in-1 "PS1 form factor" clone. Supports
HDMI output.

Monkey King 3.6: not a custom part but a rebranded RK3036, usually
running a cut-down Android based OS. Used in newer ATgames systems.

The typical configuration of the Monkey King SoCs (other than the
3.6) is with 8/16MB of SDRAM, NOR flash for the firmware and
built-in games, and a SD card for additional games.

The RS-70 is notable for having a debug UART on the USB port
(serial TX on D+, 115200). It prints the following messages on boot:

    EXEC: Executing 'boot' with 0 args (ZLib ON)...
    EXEC: Loading 'boot' at 0x18000000...
    EXEC: Loaded 372272 bytes of 2097152 available.

This is different from the serial output that this emulation model
currently produces. Perhaps one of the unimplemented IO is causing
it to go into some kind of debug mode. The log output produced by
this machine is:

    Modes:0x00000000
    PUT: Setting joystick to mode 0x0, timer to 250us

    ******************************************************
     MK FIRMWARE INFORMATION
     Mode:       0xB4
     Build Time: May  8 2019 14:09:21
     CPU Clock:  240MHz
     TFS Start:  0x8070000
     Video Buf:  0x6000000
     Stack Top:  0x3001EE8
     IWRAM Size: 32kB
     EVRAM Size: 16384kB
     Heap Size:  6144kB at 0x18200000
     Video Mode: 0
     Video Size: 1280x720x16bpp
    ******************************************************

There are other strings in the ROM that imply there may be more serial
debug possibilities.

TODO:
    implement everything
    add dumps of more Monkey King systems
*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "emupal.h"
#include "screen.h"


namespace {

class mk3b_soc_state : public driver_device
{
public:
	mk3b_soc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_iram0(*this, "iram0"),
		m_iram3(*this, "iram3"),
		m_sdram(*this, "sdram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void mk3b_soc(machine_config &config);

	void init_rs70();

private:
	required_shared_ptr<uint32_t> m_iram0, m_iram3;
	required_shared_ptr<uint32_t> m_sdram;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t io4_r(offs_t offset, uint32_t mem_mask = ~0);
	void io4_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t io7_r(offs_t offset, uint32_t mem_mask = ~0);
	void io7_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t io10_r(offs_t offset, uint32_t mem_mask = ~0);
	void io10_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update_mk3b_soc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void map(address_map &map) ATTR_COLD;
	std::string debug_buf;
};


void mk3b_soc_state::map(address_map &map)
{
	// 64MB external NOR flash
	map(0x08000000, 0x0BFFFFFF).rom().share("norflash");
	// unknown amount and configuration of internal RAM
	map(0x00000000, 0x0000FFFF).ram().share("iram0");
	// This section of RAM seems to contain the stack
	map(0x03000000, 0x0300FFFF).ram().share("iram3");

	// 16MB of external SDRAM
	map(0x18000000, 0x18FFFFFF).ram().share("sdram");
	// IO is totally unknown for now
	map(0x04000000, 0x0400FFFF).rw(FUNC(mk3b_soc_state::io4_r), FUNC(mk3b_soc_state::io4_w));
	map(0x07000000, 0x0700FFFF).rw(FUNC(mk3b_soc_state::io7_r), FUNC(mk3b_soc_state::io7_w));
	map(0x10000000, 0x1000FFFF).rw(FUNC(mk3b_soc_state::io10_r), FUNC(mk3b_soc_state::io10_w));
}

static INPUT_PORTS_START( mk3b_soc )

INPUT_PORTS_END

void mk3b_soc_state::video_start()
{
}

void mk3b_soc_state::machine_reset()
{
	// In practice, this will probably be done by a small
	// internal boot ROM.
	m_iram0[0] = 0xe59f0000; // ldr r0, [pc]
	m_iram0[1] = 0xe12fff10; // bx, r0
	m_iram0[2] = 0x08000000; // target address
}

uint32_t mk3b_soc_state::screen_update_mk3b_soc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void mk3b_soc_state::mk3b_soc(machine_config &config)
{
	// type unknown (should actually have VFP?)
	// debug output suggests 240MHz clock
	ARM920T(config, m_maincpu, 240000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mk3b_soc_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(1280, 720);
	m_screen->set_visarea(0, 1280-1, 0, 720-1);
	m_screen->set_screen_update(FUNC(mk3b_soc_state::screen_update_mk3b_soc));
}

uint32_t mk3b_soc_state::io4_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset) {
		case 0x01:
			return (m_screen->vblank() << 27) | m_screen->vblank(); // who knows? seems to need to toggle between 0 and 1
		default:
			logerror("%s: IO 0x04 read 0x%04X\n", machine().describe_context(), offset);
			return 0x00;
	}
}

void mk3b_soc_state::io4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s: IO 0x04 write 0x%04X 0x%08X & 0x%08X\n", machine().describe_context(), offset, data, mem_mask);
}


uint32_t mk3b_soc_state::io7_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset) {
		case 0x21: // video size
			return (1280 << 16) | (720);
		default:
			logerror("%s: IO 0x07 read 0x%04X\n", machine().describe_context(), offset);
			return 0x00;
	}
}

void mk3b_soc_state::io7_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s: IO 0x07 write 0x%04X 0x%08X & 0x%08X\n", machine().describe_context(), offset, data, mem_mask);
}

uint32_t mk3b_soc_state::io10_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset) {
		// Definitely not correct, but toggling somehow keeps things moving
		case 0x148:
		case 0x149:
			return m_screen->vblank() ? 0x00000000 : 0xFFFFFFFF;
		default:
			logerror("%s: IO 0x10 read 0x%04X\n", machine().describe_context(), offset);
			return 0x00;
	}
}

void mk3b_soc_state::io10_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset) {
		case 0x148: { // debug UART
			char c =  data & 0xFF;
			logerror("%s: UART W: %c\n", machine().describe_context(), c);
			if (c == '\n') {
				logerror("%s: [DEBUG] %s\n", machine().describe_context(), debug_buf.c_str());
				debug_buf.clear();
			} else if (c != '\r') {
				debug_buf += c;
			}
			break;
		}
		default:
			logerror("%s: IO 0x10 write 0x%04X 0x%08X & 0x%08X\n", machine().describe_context(), offset, data, mem_mask);
	}
}

void mk3b_soc_state::init_rs70()
{
	// Uppermost address bit seems to be inverted
	uint8_t *ROM = memregion("norflash")->base();
	int size = memregion("norflash")->bytes();

	for (int i = 0; i < (size / 2); i++) {
		std::swap(ROM[i], ROM[i + (size / 2)]);
	}
	// FIXME: Work around missing FPU for now
	ROM[0x32f24] = 0x00;
	ROM[0x32f25] = 0x00;
	ROM[0x32f26] = 0x00;
	ROM[0x32f27] = 0x00;
}


ROM_START( rs70_648 )
	ROM_REGION(0x04000000, "norflash", 0)
	ROM_LOAD("s29gl512p.bin", 0x000000, 0x04000000, CRC(cb452bd7) SHA1(0b19a13a3d0b829725c10d64d7ff852ff5202ed0) )
ROM_END

} // anonymous namespace


CONS( 2019, rs70_648,  0,        0, mk3b_soc, mk3b_soc, mk3b_soc_state, init_rs70, "CoolBoy", "RS-70 648-in-1",      MACHINE_IS_SKELETON )
