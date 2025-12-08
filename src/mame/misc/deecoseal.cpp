// license:BSD-3-Clause
// copyright-holders:Carl
/***************************************************************************

    Lucas Deeco SealTouch

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs51/i80c51.h"
#include "screen.h"
#include "emupal.h"
#include "video/upd7220.h"
#include "bus/rs232/rs232.h"
#include "machine/mc68681.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"

namespace {

class deecoseal_state : public driver_device
{
public:
	deecoseal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcs51(*this, "mcs51"),
		m_crtc(*this, "crtc"),
		m_duart(*this, "duart"),
		m_palette(*this, "palette"),
		m_eeprom(*this, "eeprom"),
		m_vram(*this, "vram")
	{ }

	void deecoseal(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	void map(address_map &map);
	void io(address_map &map);
	void mcs51_map(address_map &map);
	void crtc_map(address_map &map);
	UPD7220_DISPLAY_PIXELS_MEMBER(draw);
	u8 i2c_r();
	void i2c_w(u8 data);

	bool m_port0;

	required_device<i80186_cpu_device> m_maincpu;
	required_device<cpu_device> m_mcs51;
	required_device<upd7220_device> m_crtc;
	required_device<scn2681_device> m_duart;
	required_device<palette_device> m_palette;
	required_device<i2c_x2404p_device> m_eeprom;
	required_shared_ptr<u16> m_vram;
};

void deecoseal_state::machine_start()
{
	u16 *rom = (u16 *)memregion("bios")->base();
	u16 swapbuf[128];

	for(int i = 0; i < 32768; i += 128)
	{
		u16 *cur = rom + i;
		memcpy(swapbuf, cur, 256);
		for(int j = 0; j < 128; j++)
		{
			int addr = 0;
			addr |= ((BIT(j, 3) & !BIT(j, 5)) | (BIT(j, 4) & BIT(j, 5)));
			addr |= ((BIT(j, 3) & BIT(j, 5)) | (BIT(j, 4) & !BIT(j, 5))) << 1;
			addr |= ((BIT(j, 0) & BIT(j, 6)) | (BIT(j, 5) & !BIT(j, 6))) << 2;
			addr |= ((BIT(j, 0) & !BIT(j, 6)) | (BIT(j, 5) & BIT(j, 6))) << 3;
			addr |= BIT(j, 6) << 4;
			addr |= ((BIT(j, 1) & !BIT(j, 4)) | (BIT(j, 2) & BIT(j, 4))) << 5;
			addr |= ((BIT(j, 1) & BIT(j, 4)) | (BIT(j, 2) & !BIT(j, 4))) << 6;
			cur[j] = swapbuf[addr];
		}
	}

	m_port0 = false;
}

void deecoseal_state::map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x40000, 0x4ffff).ram().share("nvram");
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void deecoseal_state::io(address_map &map)
{
	map(0x0000, 0x0001).lr16([this](){ m_port0 = !m_port0; return m_port0 ? 0x10 : 0; }, "port0");
	map(0x0110, 0x011f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x0180, 0x0183).rw(m_crtc, FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask16(0x00ff);
	map(0x0200, 0x0200).rw(FUNC(deecoseal_state::i2c_r), FUNC(deecoseal_state::i2c_w));
}

void deecoseal_state::mcs51_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("mcs51", 0);
}

void deecoseal_state::crtc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("vram");
}

UPD7220_DISPLAY_PIXELS_MEMBER( deecoseal_state::draw )
{
	u16 const gfx = m_vram[address];
	pen_t const *const pen = m_palette->pens();

	for(u16  i = 0; i < 16; i++)
		bitmap.pix(y, x + i) = pen[BIT(gfx, i)];
}

u8 deecoseal_state::i2c_r()
{
	return m_eeprom->read_sda() ? 0x10 : 0;
}

void deecoseal_state::i2c_w(u8 data)
{
	m_eeprom->write_scl(data & 0x40 ? 1 : 0);
	m_eeprom->write_sda(data & 0x10 ? 1 : 0);
}

void deecoseal_state::deecoseal(machine_config &config)
{
	I80186(config, m_maincpu, XTAL(24'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &deecoseal_state::map);
	m_maincpu->set_addrmap(AS_IO, &deecoseal_state::io);

	I80C31(config, m_mcs51, XTAL(24'000'000) / 4).set_disable(); // clock?
	m_mcs51->set_addrmap(AS_PROGRAM, &deecoseal_state::mcs51_map);

	I2C_X2404P(config, m_eeprom);

	NVRAM(config, "nvram",  nvram_device::DEFAULT_ALL_1); // 2x 28c256

	UPD7220(config, m_crtc, XTAL(16'257'000) / 2); // unknown clock
	m_crtc->set_addrmap(AS_PROGRAM, &deecoseal_state::crtc_map);
	m_crtc->set_screen("screen");
	m_crtc->set_display_pixels(FUNC(deecoseal_state::draw));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SCN2681(config, m_duart, XTAL(3'686'400)); // scn2692

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD, rgb_t::amber()));
	screen.set_raw(XTAL(16'257'000), 848, 0, 640, 440, 0, 400);
	screen.set_screen_update(m_crtc, FUNC(upd7220_device::screen_update));
}

ROM_START(deecoseal)
	ROM_REGION16_LE(0x10000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "bios", "bios")
	ROMX_LOAD("u28_27256_lo_rev_k_lucas_1990.bin", 0x0000, 0x8000, CRC(4f5e0d47) SHA1(646e369464b66705dbab888c8f351121ffc4bc37), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("u10_27256_hi_rev_k_lucas_1990.bin", 0x0001, 0x8000, CRC(20f7d631) SHA1(797482bc6b4427b8bf750608c5a83185fae02405), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_REGION(0x8000, "mcs51", 0)
	ROM_LOAD("u34_27256_rev_b_lucas_1989.bin", 0x0000, 0x8000, CRC(01574116) SHA1(75d8dc739bf4ef1db91341f2fb02e0c698e39536))
ROM_END

} // anonymous namespace


COMP(1990, deecoseal, 0, 0, deecoseal, 0, deecoseal_state, empty_init, "Lucas", "Deeco SealTouch ST3220", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
