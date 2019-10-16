// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        TIM-011

        04/09/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "emupal.h"
#include "screen.h"

#define FDC9266_TAG "u43"

class tim011_state : public driver_device
{
public:
	tim011_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, FDC9266_TAG)
		, m_floppy0(*this, FDC9266_TAG ":0:35dd")
		, m_floppy1(*this, FDC9266_TAG ":1:35dd")
		, m_floppy2(*this, FDC9266_TAG ":2:35dd")
		, m_floppy3(*this, FDC9266_TAG ":3:35dd")
	{ }

	void tim011(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_tim011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(print_w);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE8_MEMBER(fdc_dma_w);
	DECLARE_READ8_MEMBER(print_r);
	DECLARE_READ8_MEMBER(scroll_r);
	uint8_t m_scroll;

	required_device<cpu_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<floppy_image_device> m_floppy2;
	required_device<floppy_image_device> m_floppy3;
	void tim011_io(address_map &map);
	void tim011_mem(address_map &map);
};


void tim011_state::tim011_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x01fff).rom().mirror(0x3e000);
	map(0x40000, 0x7ffff).ram(); // 256KB RAM  8 * 41256 DRAM
}

void tim011_state::tim011_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram(); /* Z180 internal registers */
	map(0x0080, 0x009f).m(m_fdc, FUNC(upd765a_device::map));
	//map(0x00a0, 0x00a0).mirror(0x001f).w(FUNC(tim011_state::fdc_dma_w));
	//map(0x00c0, 0x00c1).mirror(0x000e).rw(FUNC(tim011_state::print_r), FUNC(tim011_state::print_w));
	//map(0x00d0, 0x00d0).mirror(0x000f).rw(FUNC(tim011_state::scroll_r), FUNC(tim011_state::scroll_w));
	map(0x8000, 0xffff).ram(); // Video RAM 43256 SRAM  (32KB)
}

/* Input ports */
static INPUT_PORTS_START( tim011 )
INPUT_PORTS_END

void tim011_state::machine_reset()
{
	// motor is actually connected on TXS pin of CPU
	m_floppy0->mon_w(0);
	m_floppy1->mon_w(0);
	m_floppy2->mon_w(0);
	m_floppy3->mon_w(0);
}

void tim011_state::video_start()
{
}

uint32_t tim011_state::screen_update_tim011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE8_MEMBER(tim011_state::fdc_dma_w)
{
	printf("fdc_dma_w :%02x\n",data);
}

WRITE8_MEMBER(tim011_state::print_w)
{
	//printf("print_w :%02x\n",data);
}

READ8_MEMBER(tim011_state::print_r)
{
	//printf("print_r\n");
	return 0;
}

WRITE8_MEMBER(tim011_state::scroll_w)
{
	m_scroll = data;
}

READ8_MEMBER(tim011_state::scroll_r)
{
	return m_scroll;
}

static void tim011_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static const floppy_format_type tim011_floppy_formats[] = {
	FLOPPY_IMD_FORMAT,
	FLOPPY_MFI_FORMAT,
	FLOPPY_MFM_FORMAT,
	nullptr
};

void tim011_state::tim011(machine_config &config)
{
	/* basic machine hardware */
	HD64180RP(config, m_maincpu, XTAL(12'288'000)); // location U17 HD64180
	m_maincpu->set_addrmap(AS_PROGRAM, &tim011_state::tim011_mem);
	m_maincpu->set_addrmap(AS_IO, &tim011_state::tim011_io);
	m_maincpu->set_vblank_int("screen", FUNC(tim011_state::irq0_line_hold));

//  CDP1802(config, "keyboard", XTAL(1'750'000)); // CDP1802, unknown clock

	// FDC9266 location U43
	UPD765A(config, m_fdc, XTAL(8'000'000), true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);

	/* floppy drives */
	FLOPPY_CONNECTOR(config, FDC9266_TAG ":0", tim011_floppies, "35dd", tim011_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9266_TAG ":1", tim011_floppies, "35dd", tim011_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9266_TAG ":2", tim011_floppies, "35dd", tim011_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9266_TAG ":3", tim011_floppies, "35dd", tim011_floppy_formats);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(tim011_state::screen_update_tim011));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( tim011 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sys_tim011.u16", 0x0000, 0x2000, CRC(5b4f1300) SHA1(d324991c4292d7dcde8b8d183a57458be8a2be7b))
	ROM_REGION( 0x10000, "keyboard", ROMREGION_ERASEFF )
	ROM_LOAD( "keyb_tim011.bin", 0x0000, 0x1000, CRC(a99c40a6) SHA1(d6d505271d91df4e079ec3c0a4abbe75ae9d649b))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                    FULLNAME   FLAGS */
COMP( 1987, tim011, 0,      0,      tim011,  tim011, tim011_state, empty_init, "Mihajlo Pupin Institute", "TIM-011", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
