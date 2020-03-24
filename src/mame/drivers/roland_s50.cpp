// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland S-50 and related samplers.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/mcs96/i8x9x.h"
#include "imagedev/floppy.h"
#include "machine/mb63h149.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "video/tms3556.h"
#include "emupal.h"
#include "screen.h"

class roland_s50_state : public driver_device
{
public:
	roland_s50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_vdp(*this, "vdp")
	{
	}

	void s50(machine_config &config);
	void s550(machine_config &config);

protected:
	virtual void machine_start() override;

	TIMER_DEVICE_CALLBACK_MEMBER(vdp_timer);

	u8 floppy_status_r();
	u8 floppy_unknown_r();
	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	void vram_map(address_map &map);

private:
	void s50_mem_map(address_map &map);
	void s550_mem_map(address_map &map);

protected:
	required_device<i8x9x_device> m_maincpu;
	required_device<wd_fdc_digital_device_base> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<tms3556_device> m_vdp;
	//required_device<sa16_device> m_wave;
};

class roland_w30_state : public roland_s50_state
{
public:
	roland_w30_state(const machine_config &mconfig, device_type type, const char *tag)
		: roland_s50_state(mconfig, type, tag)
	{
	}

	void w30(machine_config &config);
#ifdef UNUSED_DEFINITION
	void s330(machine_config &config);
#endif

private:
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	u8 unknown_status_r();
	u8 unknown2_status_r();

	void w30_mem_map(address_map &map);
#ifdef UNUSED_DEFINITION
	void s330_mem_map(address_map &map);
#endif
};

void roland_s50_state::machine_start()
{
	m_fdc->set_floppy(m_floppy->get_device());
	m_fdc->dden_w(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_s50_state::vdp_timer)
{
	// FIXME: internalize this ridiculousness
	m_vdp->interrupt();
}


u8 roland_s50_state::floppy_status_r()
{
	return m_fdc->intrq_r() << 2;
}

u8 roland_s50_state::floppy_unknown_r()
{
	return 1;
}

u8 roland_s50_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset >> 1);
}

void roland_s50_state::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset >> 1, data);
}

WRITE_LINE_MEMBER(roland_w30_state::fdc_irq_w)
{
}

WRITE_LINE_MEMBER(roland_w30_state::fdc_drq_w)
{
}

u8 roland_w30_state::unknown_status_r()
{
	return 0x1c;
}

u8 roland_w30_state::unknown2_status_r()
{
	return 0x23;
}


void roland_s50_state::s50_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0xbfff).ram(); // TODO: banking
	map(0xc200, 0xc200).r(FUNC(roland_s50_state::floppy_status_r));
	map(0xc300, 0xc300).r(FUNC(roland_s50_state::floppy_unknown_r));
	//map(0xc800, 0xc807).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0xc800, 0xc800).select(6).rw(FUNC(roland_s50_state::fdc_r), FUNC(roland_s50_state::fdc_w));
	map(0xd200, 0xd200).r(m_vdp, FUNC(tms3556_device::vram_r));
	map(0xd202, 0xd202).nopr().w(m_vdp, FUNC(tms3556_device::vram_w));
	map(0xd204, 0xd204).rw(m_vdp, FUNC(tms3556_device::reg_r), FUNC(tms3556_device::reg_w));
}

void roland_s50_state::s550_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0xbfff).ram(); // TODO: banking
	map(0xc200, 0xc200).r(FUNC(roland_s50_state::floppy_status_r));
	map(0xc300, 0xc300).r(FUNC(roland_s50_state::floppy_unknown_r));
	//map(0xc800, 0xc807).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)).umask16(0x00ff);
	map(0xc800, 0xc800).select(6).rw(FUNC(roland_s50_state::fdc_r), FUNC(roland_s50_state::fdc_w));
	map(0xd000, 0xd000).r(m_vdp, FUNC(tms3556_device::vram_r));
	map(0xd002, 0xd002).nopr().w(m_vdp, FUNC(tms3556_device::vram_w));
	map(0xd004, 0xd004).rw(m_vdp, FUNC(tms3556_device::reg_r), FUNC(tms3556_device::reg_w));
}

void roland_w30_state::w30_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0x7fff).ram(); // TODO: banking
	map(0xd806, 0xd806).r(FUNC(roland_w30_state::unknown_status_r));
	map(0xe402, 0xe402).r(FUNC(roland_w30_state::unknown2_status_r));
}

#ifdef UNUSED_DEFINITION
void roland_w30_state::s330_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0xbfff).ram(); // TODO: banking
}
#endif

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
	C8095(config, m_maincpu, 24_MHz_XTAL / 2); // C8095-90
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_s50_state::s50_mem_map);

	mb63h149_device &keyscan(MB63H149(config, "keyscan", 24_MHz_XTAL / 2));
	(void)keyscan; //keyscan.int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);

	WD1772(config, m_fdc, 8_MHz_XTAL); // WD1770-00 or WD1772-02
	//m_fdc->drq_wr_callback().set_inputline(m_maincpu, i8x9x_device::HSI2_LINE);
	//m_fdc->intrq_wr_callback().set_inputline(m_maincpu, i8x9x_device::HSI3_LINE);

	// Floppy unit: FDD4261A0K or FDD4251G0K
	FLOPPY_CONNECTOR(config, m_floppy, s50_floppies, "35dd", floppy_image_device::default_floppy_formats).enable_sound(true);

	//UPD7538A(config, "fipc", 600_kHz_XTAL);

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

	//RF5C16(config, m_wave, 26.88_MHz_XTAL);
	//m_wave->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
}

void roland_s50_state::s550(machine_config &config)
{
	s50(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &roland_s50_state::s550_mem_map);

	//UPD7537(config.device_replace(), "fipc", 400_kHz_XTAL);

	config.device_remove("keyscan");
}

void roland_w30_state::w30(machine_config &config)
{
	P8098(config, m_maincpu, 24_MHz_XTAL / 2); // 8097BH
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_w30_state::w30_mem_map);

	mb63h149_device &keyscan(MB63H149(config, "keyscan", 24_MHz_XTAL / 2));
	(void)keyscan; //keyscan.int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);

	WD1772(config, m_fdc, 8_MHz_XTAL); // WD1772-02
	m_fdc->intrq_wr_callback().set(FUNC(roland_w30_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(roland_w30_state::fdc_drq_w));

	// Floppy unit: FX-354 (307F1JC)
	FLOPPY_CONNECTOR(config, m_floppy, s50_floppies, "35dd", floppy_image_device::default_floppy_formats).enable_sound(true);

	//MB89352(config, "scsic", 8_MHz_XTAL); // by option

	// LCD unit: LM240142

	TMS3556(config, m_vdp, 14.3496_MHz_XTAL); // TMS3556NL
	m_vdp->set_addrmap(0, &roland_w30_state::vram_map);
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

	TIMER(config, "vdp_timer").configure_scanline(FUNC(roland_w30_state::vdp_timer), "screen", 0, 1);

	//R15229874(config, m_wave, 26.88_MHz_XTAL);
	//m_wave->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
}

#ifdef UNUSED_DEFINITION
void roland_w30_state::s330(machine_config &config)
{
	P8097(config, m_maincpu, 24_MHz_XTAL / 2); // P8097-90
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_w30_state::s330_mem_map);

	WD1772(config, m_fdc, 8_MHz_XTAL); // WD1772-02
	m_fdc->intrq_wr_callback().set(FUNC(roland_w30_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(roland_w30_state::fdc_drq_w));

	// Floppy unit: ND-362S-A
	FLOPPY_CONNECTOR(config, m_floppy, s50_floppies, "35dd", floppy_image_device::default_floppy_formats).enable_sound(true);

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

	//R15229874(config, m_wave, 26.88_MHz_XTAL);
	//m_wave->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
}
#endif

ROM_START(s50)
	ROM_REGION(0x4000, "program", 0)
	// PROMs contain identical data but are mapped with even bytes loaded from IC64 and odd bytes from IC65
	ROM_LOAD("s-50.ic64", 0x0000, 0x4000, CRC(9a911016) SHA1(00a829d7921556c41d872c10b7bbb82b62b6c5cf))
	ROM_LOAD("s-50.ic65", 0x0000, 0x4000, CRC(9a911016) SHA1(00a829d7921556c41d872c10b7bbb82b62b6c5cf))

	ROM_REGION(0x1000, "fipc", 0) // on panel board
	ROM_LOAD("upd7538a-013_15179240.ic1", 0x0000, 0x1000, NO_DUMP)
ROM_END

ROM_START(s550)
	ROM_REGION(0x4000, "program", 0)
	// PROMs contain identical data but are mapped with even bytes loaded from IC6 and odd bytes from IC3
	ROM_LOAD("s-550_2-0-0.ic6", 0x0000, 0x4000, CRC(9dbc93b7) SHA1(bd9219772773f51e5ad7872daa1eaf03ec23f2c5))
	ROM_LOAD("s-550_2-0-0.ic3", 0x0000, 0x4000, CRC(9dbc93b7) SHA1(bd9219772773f51e5ad7872daa1eaf03ec23f2c5))

	ROM_REGION(0x800, "fipc", 0)
	ROM_LOAD("upd7537c-014_15179201.ic35", 0x000, 0x800, NO_DUMP)
ROM_END

ROM_START(w30)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD16_BYTE("w-30_1-0-3.ic19", 0x0000, 0x2000, CRC(4aa83074) SHA1(6d6f3f9dc58a4aed7cbc5d8cfce4a8b3bc2a276a))
	ROM_LOAD16_BYTE("w-30_1-0-3.ic20", 0x0001, 0x2000, CRC(9c5e3c7f) SHA1(42a0463322be5f965967d531d3636376785c9820))
ROM_END

#ifdef UNUSED_DEFINITION
ROM_START(s330)
	ROM_REGION16_LE(0x4000, "program", 0)
	ROM_LOAD16_BYTE("s-330.ic15", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD16_BYTE("s-330.ic14", 0x0001, 0x2000, NO_DUMP)
ROM_END
#endif

SYST(1987, s50,  0,   0, s50,  s50,  roland_s50_state, empty_init, "Roland", "S-50 Digital Sampling Keyboard", MACHINE_IS_SKELETON)
SYST(1987, s550, s50, 0, s550, s550, roland_s50_state, empty_init, "Roland", "S-550 Digital Sampler", MACHINE_IS_SKELETON)
SYST(1988, w30,  0,   0, w30,  w30,  roland_w30_state, empty_init, "Roland", "W-30 Music Workstation", MACHINE_IS_SKELETON)
//SYST(1988, s330, w30, 0, s330, s330, roland_w30_state, empty_init, "Roland", "S-330 Digital Sampler", MACHINE_IS_SKELETON)
