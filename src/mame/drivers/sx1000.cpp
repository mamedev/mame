// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Casio SX1010 and SX1050.
 *
 * Sources:
 *
 * TODO:
 *   - skeleton only
 */

/*
 * Preliminary parts list for 600-CPU-PCA CPU board:
 *
 * MC68010L10
 * D8237AC-5
 * HD63484-8
 * D8251AFC
 * D72065C
 * D8259AC-2
 * D8251AFC
 * D65021G031    gate array
 * D65030G024    gate array
 * HN62301AP AA1 128k mask ROM
 * HN62301AP AA2 128k mask ROM
 * MB89321A      CMOS Programmable CRT Controller
 * MB4108        ASSP Floppy Disk VFO
 * RP5C15        RTC
 * 48.8MHz XTAL
 *
 * D41256C-15    262144x1 DRAM, x36 == 1M with parity main RAM?
 * MB81464-12    262144x1 DRAM, x16 == 512K video RAM?
 * HM6264ALSP-12 8192-word 8-bit High Speed CMOS Static RAM, x2 == 16K non-volatile RAM?
 */
#include "emu.h"

#include "cpu/m68000/m68000.h"

// memory
#include "machine/ram.h"
#include "machine/nvram.h"

// various hardware
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"

// video
#include "screen.h"
#include "video/mc6845.h"
//#include "video/hd63484.h"

// busses and connectors
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class sx1000_state : public driver_device
{
public:
	sx1000_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_vram(*this, "vram")
		, m_eprom(*this, "eprom")
		, m_pic(*this, "pic")
		, m_screen(*this, "screen")
		, m_crtc(*this, "crtc")
	{
	}

	void sx1010(machine_config &config);
	void init_common();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void cpu_map(address_map &map);

	void common(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	// devices
	required_device<m68010_device> m_cpu;
	required_device<ram_device> m_ram;
	required_shared_ptr<u16> m_vram;
	required_memory_region m_eprom;

	required_device<pic8259_device> m_pic;

	required_device<screen_device> m_screen;
	required_device<hd6345_device> m_crtc;
};

void sx1000_state::machine_start()
{
}

void sx1000_state::machine_reset()
{
}

void sx1000_state::init_common()
{
}

void sx1000_state::cpu_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region(m_eprom, 0); // FIXME: probably mapped/unmapped during reset

	map(0xf00000, 0xf0ffff).rom().region(m_eprom, 0);
	map(0xf13801, 0xf13801).w(m_crtc, FUNC(hd6345_device::address_w));
	map(0xf13901, 0xf13901).rw(m_crtc, FUNC(hd6345_device::register_r), FUNC(hd6345_device::register_w));

	map(0xf14001, 0xf14001).lrw8([this]() { return m_pic->read(0); }, "pic_r0", [this](u8 data) { m_pic->write(0, data); }, "pic_w0");
	map(0xf14101, 0xf14101).lrw8([this]() { return m_pic->read(1); }, "pic_r1", [this](u8 data) { m_pic->write(1, data); }, "pic_w1");

	map(0xf20000, 0xf23fff).ram().share(m_vram);
}

MC6845_UPDATE_ROW( sx1000_state::crtc_update_row )
{
	//	logerror("ma=%x ra=%d y=%d x_count=%d cursor_x=%d de=%d hbp=%d vbp=%d\n", ma*2, ra, y, x_count, cursor_x, de, hbp, vbp);
	const u16 *charset = reinterpret_cast<const u16 *>(m_eprom->base() + 0x5c40);
	const u16 *vram = m_vram + ma;
	u32 *dest = &bitmap.pix(y);
	for(u32 x0 = 0; x0 != x_count; x0 ++) {
		u16 data = *vram++;
		u16 bitmap = charset[((data & 0xff) << 3) | (ra >> 1)];
		if(!(ra & 1))
			bitmap >>= 8;
		for(u32 x1 = 0; x1 != 8; x1 ++) {
			u32 color = BIT(bitmap, 7-x1) ? 0xffffff : 0x000000;
			*dest ++ = color;
			*dest ++ = color;
		}
	}
}

void sx1000_state::common(machine_config &config)
{
	M68010(config, m_cpu, 10'000'000);
	m_cpu->set_addrmap(AS_PROGRAM, &sx1000_state::cpu_map);

	// 36 x D41256C-15 (256Kb DRAM) on CPU board
	RAM(config, m_ram);
	m_ram->set_default_size("1M");

	//	NVRAM(config, "nvram");

	PIC8259(config, m_pic);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(48'800'000, 80*16, 0, 80*16, 25*16, 0, 25*16);
	m_screen->set_screen_update(m_crtc, FUNC(hd6345_device::screen_update));

	// htotal = 117
	// hdisp = 80
	// vtotal = 26
	// vdisp = 25
	// mrast = 15
	HD6345(config, m_crtc, 4'000'000);
	m_crtc->set_screen(m_screen);
	m_crtc->set_update_row_callback(FUNC(sx1000_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_crtc->set_hpixels_per_column(16);
}

void sx1000_state::sx1010(machine_config &config)
{
	common(config);
}

static INPUT_PORTS_START(sx1010)
INPUT_PORTS_END

ROM_START(sx1010)
	ROM_REGION16_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "sx1010", "sx1010")
	ROMX_LOAD("pc1a.u6l8", 0x8001, 0x4000, CRC(75d0f02c) SHA1(dfcc7efc1b5e7b43fc1ee030bef5c75c23d5e742), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_CONTINUE (         0x0001, 0x4000)
	ROMX_LOAD("pc1a.u6h8", 0x8000, 0x4000, CRC(a928c0c9) SHA1(712601ee889a0790a7579fe06df20ec7e0a4bb49), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_CONTINUE (         0x0000, 0x4000)
ROM_END

} // anonymous namespace

/*   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME  FLAGS */
COMP(1987, sx1010, 0,      0,      sx1010,  sx1010, sx1000_state, init_common, "Casio", "SX1010", MACHINE_IS_SKELETON)
