// license:BSD-3-Clause
// copyright-holders:Antonio "Willy" Malara

/*
Korg TRITON skeleton driver

CPU: SH2 SH7043 / SH7045
FLASH: 2 * MX29F1610
LCD controller: M66271FP
Floppy disk controller: HD63266
Serial Interface: uPD71051
SCSI controller: MB86604L
Sound: 2 * "TGL96" MB87F1710-PFV-S (proprietary)

This driver runs the embedded firmware with skeleton devices
until the boot splash screen is shown.
*/


#include "emu.h"
#include "cpu/sh/sh7042.h"
#include "machine/i8251.h"
#include "screen.h"

#include <algorithm>

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class korgtriton_state : public driver_device
{
public:
	korgtriton_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_lcdcm(*this, "lcdcm")
		, m_screen(*this, "screen")
		, m_scu(*this, "scu")
	{ }

	void korgtriton(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static inline constexpr unsigned SCREEN_WIDTH = 320;
	static inline constexpr unsigned SCREEN_HEIGHT = 240;

	required_device<sh7043_device> m_maincpu;
	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_lcdcm;
	required_device<screen_device> m_screen;
	required_device<i8251_device> m_scu;

	u8 m_lcdcio[SCREEN_WIDTH / 8 * SCREEN_HEIGHT]; // 4 bytes per pixel

	// SCU: hack to provide data that makes the software proceed,
	//      instead of using the actual device, see also comment
	//      in map() below
	int m_scu_hack_status_idx;
	int m_scu_hack_data_idx;

	u16 m_tgl[0x1000];

	int m_moss_hack_idx;

	void map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 pa_r();
	void pa_w(u32 data);

	u32 pe_r();
	void pe_w(u32 data);

	void scu_txrdy_w(int state);
	void scu_rxrdy_w(int state);

	u8 scu_r(offs_t offs);
	void scu_w(offs_t offs, u8 data);

	u8 lcdcio_r(offs_t offs);
	void lcdcio_w(offs_t offs, u8 data);

	u8 tgl_r(offs_t offs);
	void tgl_w(offs_t offs, u8 data);

	u8 moss_r(offs_t offs);
	void moss_w(offs_t offs, u8 data);
};

static INPUT_PORTS_START(korgtriton)
INPUT_PORTS_END

void korgtriton_state::machine_start()
{
}

void korgtriton_state::machine_reset()
{
	m_scu_hack_status_idx = 0;
	m_scu_hack_data_idx = 0;

	std::fill(std::begin(m_tgl), std::end(m_tgl), 0);
	std::fill(std::begin(m_lcdcio), std::end(m_lcdcio), 0);
}

void korgtriton_state::map(address_map &map)
{
	// on chip rom
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);

	// cs0 space (32 bits access)
	// to SAMPLING interface

	// cs1 space (32 bits access)
	map(0x400000, 0x7fffff).rom(); // FLASH

	// cs2 space (16 bits access)
	// map(0x800000, 0x8fffff).ram(); // SPC
	map(0x900000, 0x9fffff).ram().share(m_lcdcm);
	map(0xa00000, 0xafffff).rw(FUNC(korgtriton_state::lcdcio_r), FUNC(korgtriton_state::lcdcio_w));
	map(0xb00000, 0xbfffff).rw(FUNC(korgtriton_state::tgl_r), FUNC(korgtriton_state::tgl_w));
	map(0xd00000, 0xdfffff).rw(FUNC(korgtriton_state::moss_r), FUNC(korgtriton_state::moss_w));

	// map(0xe00000, 0xefffff).ram(); // FDC

	// TODO: figure out why clocking the SCU device from port E is not working
	// map(0xf00000, 0xf00001).rw(m_scu, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf00000, 0xf00001).rw(FUNC(korgtriton_state::scu_r), FUNC(korgtriton_state::scu_w));

	// System DRAM
	map(0x01000000, 0x01ffffff).ram().share(m_ram);
}

void korgtriton_state::korgtriton(machine_config &config)
{
	SH7043(config, m_maincpu, 7_MHz_XTAL * 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &korgtriton_state::map);

	m_maincpu->read_porta().set(FUNC(korgtriton_state::pa_r));
	m_maincpu->write_porta().set(FUNC(korgtriton_state::pa_w));
	m_maincpu->read_porte().set(FUNC(korgtriton_state::pe_r));
	m_maincpu->write_porte().set(FUNC(korgtriton_state::pe_w));

	// 320x240 screen, should be a simple framebuffer at LCDCIO
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_screen->set_visarea(0, SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT - 1);
	m_screen->set_screen_update(FUNC(korgtriton_state::screen_update));

	I8251(config, m_scu, 1021800);
	m_scu->txrdy_handler().set(FUNC(korgtriton_state::scu_txrdy_w));
	m_scu->rxrdy_handler().set(FUNC(korgtriton_state::scu_rxrdy_w));
}

u32 korgtriton_state::pa_r()
{
	return 0;
}

void korgtriton_state::pa_w(u32 data)
{
	LOG("pa_w: %08x\n", data);
}

u32 korgtriton_state::pe_r()
{
	return 0;
}

void korgtriton_state::pe_w(u32 data)
{
	LOG("pe_w: %08x\n", data);
}

u32 korgtriton_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const int byte_width = SCREEN_WIDTH / 8;

	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x_byte = 0; x_byte < byte_width; ++x_byte)
		{
			const u8 byte = m_lcdcio[y * byte_width + x_byte];

			for (int bit = 0; bit < 8; ++bit)
			{
				const int x = x_byte * 8 + (7 - bit); // MSB first
				const u32 color = BIT(byte, bit) ? 0xffffffff : 0xff000000;
				dst[x] = color;
			}
		}
	}
	return 0;
}

u8 korgtriton_state::lcdcio_r(offs_t offs)
{
	return (offs < sizeof(m_lcdcio)) ? m_lcdcio[offs] : 0;
}

void korgtriton_state::lcdcio_w(offs_t offs, u8 data)
{
	if (offs < std::size(m_lcdcio))
		m_lcdcio[offs] = data;
}

void korgtriton_state::scu_txrdy_w(int state)
{
	LOG("upd71051_txrdy_w: %d\n", state);
}

void korgtriton_state::scu_rxrdy_w(int state)
{
	LOG("upd71051_rxrdy_w: %d\n", state);
}

u8 korgtriton_state::scu_r(offs_t offs)
{
	int res = 0;

	if (offs == 0)
	{
		const static u8 data[] = { 0x66, 0x31 };
		res = data[m_scu_hack_data_idx % 2];
		if (!machine().side_effects_disabled())
			m_scu_hack_data_idx++;
	}

	if (offs == 1)
	{
		res = m_scu_hack_status_idx;
		if (!machine().side_effects_disabled())
			m_scu_hack_status_idx++;
	}

	LOG("scu_read: %08x -> %02x\n", offs, res);
	return res;
}

void korgtriton_state::scu_w(offs_t offs, u8 data)
{
	LOG("scu_write: %08x %02x\n", offs, data);
}

u8 korgtriton_state::tgl_r(offs_t offs)
{
	int res = 0;

	if (offs < std::size(m_tgl))
		res = m_tgl[offs];

	if (offs == 0)
		res = 4;

	LOG("tgl_read: %08x -> %02x\n", offs, res);
	return res;
}

void korgtriton_state::tgl_w(offs_t offs, u8 data) {
	LOG("tgl_write: %08x %02x\n", offs, data);

	if (offs == 0x609 && data == 0x01)
		data = 0;

	if (offs == 0xe09 && data == 0x01)
		data = 0;

	if (offs < std::size(m_tgl))
		m_tgl[offs] = data;
}

u8 korgtriton_state::moss_r(offs_t offs)
{
	if (!machine().side_effects_disabled())
		m_moss_hack_idx++;

	return m_moss_hack_idx;
}

void korgtriton_state::moss_w(offs_t offs, u8 data)
{
}

// the following rom images are taken from the Triton OS 2.0.0 floppy disks
// the romfiles are missing the first two bytes (checksum) present in the
// files obtainable from floppy disks online.

ROM_START( korgtriton )
	ROM_REGION( 0x00800000, "maincpu", 0 )

	ROM_LOAD("int01080.710", 0x00000000, 0x00008000, CRC(ef0d7b3a) SHA1(5b20814dbd118df746437a5aa0b9bc371b8bada0))
	ROM_LOAD("int13080.710", 0x00008000, 0x00018000, CRC(295bd0f8) SHA1(5f21cbaa95d9622aa0cfcf8b453d8c98b5dc54cd))

	ROM_LOAD("ext03080.710", 0x00400000, 0x000c0000, CRC(12e681b1) SHA1(fb3299cffd8fc11ccbf5d7228e1d35f8a824579c))
	ROM_LOAD("ext34080.710", 0x004c0000, 0x00100000, CRC(a84ea53c) SHA1(c949bef347f0cf02a16a6a332aeedcff31c6b956))
	ROM_LOAD("ext74080.710", 0x005c0000, 0x00100000, CRC(281cef3b) SHA1(f348fb53f2ce954fda8004d310566bec663374f4))
ROM_END

} // anonymous namespace

SYST(1999, korgtriton, 0, 0, korgtriton,  korgtriton, korgtriton_state, empty_init, "Korg", "TRITON", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
