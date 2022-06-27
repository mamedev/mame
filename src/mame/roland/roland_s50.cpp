// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland S-50 and related samplers.

****************************************************************************/

#include "emu.h"
#include "bu3905.h"
#include "sa16.h"
//#include "bus/midi/midi.h"
#include "cpu/mcs96/i8x9x.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "mb63h149.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "video/tms3556.h"
#include "video/t6963c.h"
#include "emupal.h"
#include "screen.h"

class roland_s50_state : public driver_device
{
public:
	roland_s50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sram(*this, "sram")
		, m_io(*this, "io")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_vdp(*this, "vdp")
		, m_wave(*this, "wave")
		, m_keyscan(*this, "keyscan")
	{
	}

	void s50(machine_config &config);

protected:
	virtual void machine_start() override;

	TIMER_DEVICE_CALLBACK_MEMBER(vdp_timer);

	void p2_w(u8 data);
	u8 floppy_status_r();
	u8 floppy_unknown_r();
	u16 key_r(offs_t offset);
	void key_w(offs_t offset, u16 data);

	void sram_map(address_map &map);
	void vram_map(address_map &map);

private:
	void ioga_out_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

protected:
	required_device<i8x9x_device> m_maincpu;
	optional_device<address_map_bank_device> m_sram;
	optional_device<address_map_bank_device> m_io;
	required_device<wd_fdc_digital_device_base> m_fdc;
	required_device<floppy_connector> m_floppy;
	optional_device<tms3556_device> m_vdp;
	required_device<sa16_base_device> m_wave;
	optional_device<mb63h149_device> m_keyscan;
};

class roland_s550_state : public roland_s50_state
{
public:
	roland_s550_state(const machine_config &mconfig, device_type type, const char *tag)
		: roland_s50_state(mconfig, type, tag)
		, m_lowmem(*this, "lowmem")
	{
	}

	void s550(machine_config &config);

private:
	void sram_bank_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void lowmem_map(address_map &map);

	required_device<address_map_bank_device> m_lowmem;
};

class roland_w30_state : public roland_s50_state
{
public:
	roland_w30_state(const machine_config &mconfig, device_type type, const char *tag)
		: roland_s50_state(mconfig, type, tag)
		, m_psram(*this, "psram%u", 1U)
		, m_psram_bank(0)
	{
	}

	void w30(machine_config &config);
#ifdef UNUSED_DEFINITION
	void s330(machine_config &config);
#endif

protected:
	virtual void machine_start() override;

private:
	u8 psram_bank_r();
	void psram_bank_w(u8 data);
	u8 unknown_status_r();

	void w30_mem_map(address_map &map);
#ifdef UNUSED_DEFINITION
	void s330_mem_map(address_map &map);
#endif
	void psram1_map(address_map &map);
	void psram2_map(address_map &map);

	required_device_array<address_map_bank_device, 2> m_psram;

	u8 m_psram_bank;
};

void roland_s50_state::machine_start()
{
	m_fdc->set_floppy(m_floppy->get_device());
	m_fdc->dden_w(0);
}

void roland_w30_state::machine_start()
{
	save_item(NAME(m_psram_bank));
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_s50_state::vdp_timer)
{
	// FIXME: internalize this ridiculousness
	m_vdp->interrupt();
}


void roland_s50_state::p2_w(u8 data)
{
	m_io->set_bank(BIT(data, 5));
}

void roland_s50_state::ioga_out_w(u8 data)
{
	m_sram->set_bank(BIT(data, 6, 2));
}

void roland_s550_state::sram_bank_w(u8 data)
{
	m_sram->set_bank(BIT(data, 0, 2));
	m_lowmem->set_bank(BIT(data, 2, 3));
}

u8 roland_w30_state::psram_bank_r()
{
	return m_psram_bank;
}

void roland_w30_state::psram_bank_w(u8 data)
{
	m_psram_bank = data;
	m_psram[1]->set_bank(BIT(data, 0, 3));
	m_psram[0]->set_bank(BIT(data, 3, 4));
}

u8 roland_s50_state::floppy_status_r()
{
	return 1 | m_fdc->intrq_r() << 2 | m_fdc->drq_r() << 3;
}

u8 roland_s50_state::floppy_unknown_r()
{
	return 1;
}

u16 roland_s50_state::key_r(offs_t offset)
{
	return m_keyscan->read(offset) << 1;
}

void roland_s50_state::key_w(offs_t offset, u16 data)
{
	m_keyscan->write(offset, data >> 1);
}

u8 roland_w30_state::unknown_status_r()
{
	return 0x1c;
}


void roland_s50_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0x7fff).ram().share("common");
	map(0x8000, 0xbfff).m(m_sram, FUNC(address_map_bank_device::amap16));
	map(0xc000, 0xffff).m(m_io, FUNC(address_map_bank_device::amap16));
}

void roland_s50_state::io_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(roland_s50_state::ioga_out_w));
	map(0x0200, 0x0200).r(FUNC(roland_s50_state::floppy_status_r));
	map(0x0300, 0x0300).r(FUNC(roland_s50_state::floppy_unknown_r));
	map(0x0800, 0x0807).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0x1200, 0x1200).r(m_vdp, FUNC(tms3556_device::vram_r));
	map(0x1202, 0x1202).rw(m_vdp, FUNC(tms3556_device::initptr_r), FUNC(tms3556_device::vram_w));
	map(0x1204, 0x1204).rw(m_vdp, FUNC(tms3556_device::reg_r), FUNC(tms3556_device::reg_w));
	map(0x0000, 0x3fff).rw(m_wave, FUNC(rf5c36_device::read), FUNC(rf5c36_device::write)).umask16(0xff00);
	map(0x4000, 0x4fff).mirror(0x3000).rw(FUNC(roland_s50_state::key_r), FUNC(roland_s50_state::key_w));
}

void roland_s550_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_lowmem, FUNC(address_map_bank_device::amap16));
	map(0x2000, 0x3fff).rom().region("program", 0x2000);
	map(0x4000, 0x7fff).ram().share("common");
	map(0x8000, 0xbfff).m(m_sram, FUNC(address_map_bank_device::amap16));
	map(0xc000, 0xffff).m(m_io, FUNC(address_map_bank_device::amap16));
}

void roland_s550_state::io_map(address_map &map)
{
	map(0x0200, 0x0200).r(FUNC(roland_s550_state::floppy_status_r));
	map(0x0300, 0x0300).r(FUNC(roland_s550_state::floppy_unknown_r));
	map(0x0800, 0x0807).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0x1000, 0x1000).r(m_vdp, FUNC(tms3556_device::vram_r));
	map(0x1002, 0x1002).rw(m_vdp, FUNC(tms3556_device::initptr_r), FUNC(tms3556_device::vram_w));
	map(0x1004, 0x1004).rw(m_vdp, FUNC(tms3556_device::reg_r), FUNC(tms3556_device::reg_w));
	//map(0x1800, 0x181f).rw(m_tvf, FUNC(mb654419u_device::read), FUNC(mb654419u_device::write)).umask16(0x00ff);
	map(0x2000, 0x2000).w(FUNC(roland_s550_state::sram_bank_w));
	map(0x2800, 0x281f).w("outas", FUNC(bu3905_device::write)).umask16(0x00ff);
	//map(0x3800, 0x381f).rw(m_scsic, FUNC(mb89352_device::read), FUNC(mb89352_device::write)).umask16(0x00ff);
	map(0x0000, 0x3fff).rw(m_wave, FUNC(rf5c36_device::read), FUNC(rf5c36_device::write)).umask16(0xff00);
}

void roland_w30_state::w30_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_psram[0], FUNC(address_map_bank_device::amap16));
	map(0x2000, 0x3fff).rom().region("program", 0x2000);
	map(0x4000, 0x7fff).ram().share("common");
	map(0x8000, 0xbfff).m(m_psram[1], FUNC(address_map_bank_device::amap16));
	map(0xc200, 0xc200).r(FUNC(roland_w30_state::floppy_status_r));
	map(0xc600, 0xc600).rw(FUNC(roland_w30_state::psram_bank_r), FUNC(roland_w30_state::psram_bank_w));
	map(0xc800, 0xc807).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0xd806, 0xd806).r(FUNC(roland_w30_state::unknown_status_r));
	//map(0xe000, 0xe01f).rw(m_scsic, FUNC(mb89352_device::read), FUNC(mb89352_device::write)).umask16(0x00ff);
	map(0xe400, 0xe403).rw("lcd", FUNC(lm24014h_device::read), FUNC(lm24014h_device::write)).umask16(0x00ff);
	//map(0xe800, 0xe83f).w("output", FUNC(upd65006gf_376_3b8_device::write)).umask16(0x00ff);
	//map(0xf000, 0xf01f).rw(m_tvf, FUNC(mb654419u_device::read), FUNC(mb654419u_device::write)).umask16(0x00ff);
	map(0xc000, 0xffff).rw(m_wave, FUNC(sa16_device::read), FUNC(sa16_device::write)).umask16(0xff00);
}

#ifdef UNUSED_DEFINITION
void roland_w30_state::s330_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_psram[0], FUNC(address_map_bank_device::amap16));
	map(0x2000, 0x3fff).rom().region("program", 0x2000);
	map(0x4000, 0x7fff).ram().share("common");
	map(0x8000, 0xbfff).m(m_psram[1], FUNC(address_map_bank_device::amap16));
	map(0xc000, 0xffff).rw(m_wave, FUNC(sa16_device::read), FUNC(sa16_device::write)).umask16(0xff00);
}
#endif

void roland_s50_state::sram_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("common");
	map(0x4000, 0xffff).ram();
}

void roland_s550_state::lowmem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("program", 0);
	map(0x2000, 0xffff).ram();
}

void roland_w30_state::psram1_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("program", 0);
	map(0x2000, 0xffff).ram();
}

void roland_w30_state::psram2_map(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("program", 0);
	map(0x04000, 0x0ffff).mirror(0x10000).ram();
	map(0x10000, 0x13fff).ram().share("common");
}

void roland_s50_state::vram_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}

static INPUT_PORTS_START(s50)
INPUT_PORTS_END

static INPUT_PORTS_START(s550)
INPUT_PORTS_END

static INPUT_PORTS_START(w30)
INPUT_PORTS_END

#ifdef UNUSED_DEFINITION
static INPUT_PORTS_START(s330)
INPUT_PORTS_END
#endif

static void s50_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void roland_s50_state::s50(machine_config &config)
{
	C8095_90(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_s50_state::mem_map);
	m_maincpu->out_p2_cb().set(FUNC(roland_s50_state::p2_w));

	ADDRESS_MAP_BANK(config, m_sram);
	m_sram->set_endianness(ENDIANNESS_LITTLE);
	m_sram->set_data_width(16);
	m_sram->set_addr_width(16);
	m_sram->set_stride(0x4000);
	m_sram->set_addrmap(0, &roland_s50_state::sram_map);

	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_endianness(ENDIANNESS_LITTLE);
	m_io->set_data_width(16);
	m_io->set_addr_width(15);
	m_io->set_stride(0x4000);
	m_io->set_addrmap(0, &roland_s50_state::io_map);

	MB63H149(config, m_keyscan, 24_MHz_XTAL / 2);
	m_keyscan->int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);

	WD1772(config, m_fdc, 8_MHz_XTAL); // WD1770-00 or WD1772-02
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, i8x9x_device::HSI2_LINE);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, i8x9x_device::HSI3_LINE);

	// Floppy unit: FDD4261A0K or FDD4251G0K
	FLOPPY_CONNECTOR(config, m_floppy, s50_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	//UPD7538A(config, "fipcpu", 600_kHz_XTAL);

	TMS3556(config, m_vdp, 14.3496_MHz_XTAL); // TMS3556NL
	m_vdp->set_addrmap(0, &roland_s50_state::vram_map);
	//m_vdp->vsync_callback().set_inputline(m_maincpu, i8x9x_device::HSI1_LINE).invert();
	m_vdp->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_screen_update(m_vdp, FUNC(tms3556_device::screen_update));
	screen.set_size(tms3556_device::TOTAL_WIDTH, tms3556_device::TOTAL_HEIGHT*2);
	screen.set_visarea(0, tms3556_device::TOTAL_WIDTH-1, 0, tms3556_device::TOTAL_HEIGHT-1);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	TIMER(config, "vdp_timer").configure_scanline(FUNC(roland_s50_state::vdp_timer), "screen", 0, 1);

	RF5C36(config, m_wave, 26.88_MHz_XTAL);
	m_wave->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
}

void roland_s550_state::s550(machine_config &config)
{
	s50(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &roland_s550_state::mem_map);
	m_io->set_addrmap(0, &roland_s550_state::io_map);

	ADDRESS_MAP_BANK(config, m_lowmem);
	m_lowmem->set_endianness(ENDIANNESS_LITTLE);
	m_lowmem->set_data_width(16);
	m_lowmem->set_addr_width(16);
	m_lowmem->set_stride(0x2000);
	m_lowmem->set_addrmap(0, &roland_s550_state::lowmem_map);

	//UPD7537(config.device_replace(), "fipcpu", 400_kHz_XTAL);

	config.device_remove("keyscan");

	//MB89352(config, m_scsic, 8_MHz_XTAL); // on Option Board
	//m_scsic->intr_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);

	BU3905(config, "outas");

	//MB654419U(config, m_tvf, 20_MHz_XTAL);

	m_wave->sh_callback().set("outas", FUNC(bu3905_device::axi_w));
}

void roland_w30_state::w30(machine_config &config)
{
	N8097BH(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_w30_state::w30_mem_map);

	ADDRESS_MAP_BANK(config, m_psram[0]);
	m_psram[0]->set_endianness(ENDIANNESS_LITTLE);
	m_psram[0]->set_data_width(16);
	m_psram[0]->set_addr_width(16);
	m_psram[0]->set_stride(0x2000);
	m_psram[0]->set_addrmap(0, &roland_w30_state::psram1_map);

	ADDRESS_MAP_BANK(config, m_psram[1]);
	m_psram[1]->set_endianness(ENDIANNESS_LITTLE);
	m_psram[1]->set_data_width(16);
	m_psram[1]->set_addr_width(17);
	m_psram[1]->set_stride(0x4000);
	m_psram[1]->set_addrmap(0, &roland_w30_state::psram2_map);

	MB63H149(config, m_keyscan, 24_MHz_XTAL / 2);
	m_keyscan->int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);

	WD1772(config, m_fdc, 8_MHz_XTAL); // WD1772-02

	// Floppy unit: FX-354 (307F1JC)
	FLOPPY_CONNECTOR(config, m_floppy, s50_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	//MB89352(config, m_scsic, 8_MHz_XTAL); // by option

	LM24014H(config, "lcd"); // LCD unit: LM240142

	SA16(config, m_wave, 26.88_MHz_XTAL);
	m_wave->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);

	//UPD65006GF_376_3B8(config, "output", 26.88_MHz_XTAL);

	//MB654419U(config, m_tvf, 20_MHz_XTAL);
}

#ifdef UNUSED_DEFINITION
void roland_w30_state::s330(machine_config &config)
{
	P8097(config, m_maincpu, 24_MHz_XTAL / 2); // P8097-90
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_w30_state::s330_mem_map);

	ADDRESS_MAP_BANK(config, m_psram[0]);
	m_psram[0]->set_endianness(ENDIANNESS_LITTLE);
	m_psram[0]->set_data_width(16);
	m_psram[0]->set_addr_width(16);
	m_psram[0]->set_stride(0x2000);
	m_psram[0]->set_addrmap(0, &roland_w30_state::psram1_map);

	ADDRESS_MAP_BANK(config, m_psram[1]);
	m_psram[1]->set_endianness(ENDIANNESS_LITTLE);
	m_psram[1]->set_data_width(16);
	m_psram[1]->set_addr_width(17);
	m_psram[1]->set_stride(0x4000);
	m_psram[1]->set_addrmap(0, &roland_w30_state::psram2_map);

	WD1772(config, m_fdc, 8_MHz_XTAL); // WD1772-02

	// Floppy unit: ND-362S-A
	FLOPPY_CONNECTOR(config, m_floppy, s50_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// LCD unit: DM1620-5BL7 (MW-5F)

	TMS3556(config, m_vdp, 14.3496_MHz_XTAL); // TMS3556NL
	m_vdp->set_addrmap(0, &roland_w30_state::vram_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_screen_update(m_vdp, FUNC(tms3556_device::screen_update));
	screen.set_size(tms3556_device::TOTAL_WIDTH, tms3556_device::TOTAL_HEIGHT*2);
	screen.set_visarea(0, tms3556_device::TOTAL_WIDTH-1, 0, tms3556_device::TOTAL_HEIGHT-1);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	TIMER(config, "vdp_timer").configure_scanline(FUNC(roland_w30_state::vdp_timer), "screen", 0, 1);

	SA16(config, m_wave, 26.88_MHz_XTAL);
	m_wave->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
	m_wave->sh_callback().set("outas", FUNC(bu3905_device::axi_w));

	BU3905(config, "outas");

	//MB654419U(config, m_tvf, 20_MHz_XTAL);
}
#endif

ROM_START(s50)
	ROM_REGION16_LE(0x4000, "program", 0)
	// PROMs contain identical data but are mapped with even bytes loaded from IC64 and odd bytes from IC65
	ROM_LOAD("s-50.ic64", 0x0000, 0x4000, CRC(9a911016) SHA1(00a829d7921556c41d872c10b7bbb82b62b6c5cf))
	ROM_LOAD("s-50.ic65", 0x0000, 0x4000, CRC(9a911016) SHA1(00a829d7921556c41d872c10b7bbb82b62b6c5cf))

	ROM_REGION(0x1000, "fipcpu", 0) // on panel board
	ROM_LOAD("upd7538a-013_15179240.ic1", 0x0000, 0x1000, NO_DUMP)
ROM_END

ROM_START(s550)
	ROM_REGION16_LE(0x4000, "program", 0)
	// PROMs contain identical data but are mapped with even bytes loaded from IC6 and odd bytes from IC3
	ROM_LOAD("s-550_2-0-0.ic6", 0x0000, 0x4000, CRC(9dbc93b7) SHA1(bd9219772773f51e5ad7872daa1eaf03ec23f2c5))
	ROM_LOAD("s-550_2-0-0.ic3", 0x0000, 0x4000, CRC(9dbc93b7) SHA1(bd9219772773f51e5ad7872daa1eaf03ec23f2c5))

	ROM_REGION(0x800, "fipcpu", 0)
	ROM_LOAD("upd7537c-014_15179201.ic35", 0x000, 0x800, NO_DUMP)
ROM_END

ROM_START(w30)
	ROM_REGION16_LE(0x4000, "program", 0)
	ROM_LOAD16_BYTE("w-30_1-0-3.ic19", 0x0000, 0x2000, CRC(4aa83074) SHA1(6d6f3f9dc58a4aed7cbc5d8cfce4a8b3bc2a276a))
	ROM_LOAD16_BYTE("w-30_1-0-3.ic20", 0x0001, 0x2000, CRC(9c5e3c7f) SHA1(42a0463322be5f965967d531d3636376785c9820))

	ROM_REGION16_LE(0x100000, "wave", 0)
	ROM_LOAD16_BYTE("lh534146_15179935.ic30", 0x00000, 0x80000, NO_DUMP) // D0-D3 not connected
	ROM_LOAD16_BYTE("lh534145_15179936.ic29", 0x00001, 0x80000, NO_DUMP)
ROM_END

#ifdef UNUSED_DEFINITION
ROM_START(s330)
	ROM_REGION16_LE(0x4000, "program", 0)
	ROM_LOAD16_BYTE("s-330.ic15", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD16_BYTE("s-330.ic14", 0x0001, 0x2000, NO_DUMP)
ROM_END
#endif

SYST(1987, s50,  0,   0, s50,  s50,  roland_s50_state,  empty_init, "Roland", "S-50 Digital Sampling Keyboard", MACHINE_IS_SKELETON)
SYST(1987, s550, s50, 0, s550, s550, roland_s550_state, empty_init, "Roland", "S-550 Digital Sampler", MACHINE_IS_SKELETON)
SYST(1988, w30,  0,   0, w30,  w30,  roland_w30_state,  empty_init, "Roland", "W-30 Music Workstation", MACHINE_IS_SKELETON)
//SYST(1988, s330, w30, 0, s330, s330, roland_w30_state, empty_init, "Roland", "S-330 Digital Sampler", MACHINE_IS_SKELETON)
