// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        TIM-011

        04/09/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "formats/tim011_dsk.h"
#include "machine/upd765.h"
#include "bus/rs232/rs232.h"
#include "bus/tim011/exp.h"

#include "emupal.h"
#include "screen.h"

#define FDC9266_TAG "u43"

namespace {

class tim011_state : public driver_device
{
public:
	tim011_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, FDC9266_TAG)
		, m_floppy(*this, FDC9266_TAG ":%u", 0)
		, m_vram(*this, "videoram")
		, m_palette(*this, "palette")
		, m_exp(*this, "exp")
	{ }

	void tim011(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_tim011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void print_w(uint8_t data);
	void scroll_w(uint8_t data);
	uint8_t print_r();
	uint8_t scroll_r();
	uint8_t m_scroll;

	required_device<z180_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_shared_ptr<u8> m_vram;
	required_device<palette_device> m_palette;
	required_device<bus::tim011::exp_slot_device> m_exp;

	void tim011_io(address_map &map) ATTR_COLD;
	void tim011_mem(address_map &map) ATTR_COLD;
	void tim011_palette(palette_device &palette) const;
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
	map(0x0080, 0x0081).mirror(0xff0e).m(m_fdc, FUNC(upd765a_device::map));
	map(0x00a0, 0x00a0).mirror(0xff0f).rw(m_fdc, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0x00c0, 0x00c1).mirror(0xff0e).rw(FUNC(tim011_state::print_r), FUNC(tim011_state::print_w));
	map(0x00d0, 0x00d0).mirror(0xff0f).rw(FUNC(tim011_state::scroll_r), FUNC(tim011_state::scroll_w));
	map(0x8000, 0xffff).ram().share(m_vram); // Video RAM 43256 SRAM  (32KB)
}

/* Input ports */
static INPUT_PORTS_START( tim011 )
INPUT_PORTS_END

void tim011_state::machine_reset()
{
	m_scroll = 0;
	// motor is actually connected on TXS pin of CPU
	for (auto &drive : m_floppy)
	{
		if (drive->get_device())
			drive->get_device()->mon_w(0);
	}
}

uint32_t tim011_state::screen_update_tim011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = 0; x < 512/4; x++)
	{
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			int horpos = x << 2;
			u8 code = m_vram[(x << 8) + ((y + m_scroll) & 0xff)];
			bitmap.pix(y, horpos++) =  (code >> 0) & 0x03;
			bitmap.pix(y, horpos++) =  (code >> 2) & 0x03;
			bitmap.pix(y, horpos++) =  (code >> 4) & 0x03;
			bitmap.pix(y, horpos++) =  (code >> 6) & 0x03;
		}
	}
	return 0;
}

void tim011_state::print_w(uint8_t data)
{
	//printf("print_w :%02x\n",data);
}

uint8_t tim011_state::print_r()
{
	//printf("print_r\n");
	return 0;
}

void tim011_state::scroll_w(uint8_t data)
{
	m_scroll = data;
}

uint8_t tim011_state::scroll_r()
{
	return m_scroll;
}

static void tim011_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static void tim011_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TIM011_FORMAT);
}

static DEVICE_INPUT_DEFAULTS_START( keyboard )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void tim011_state::tim011_palette(palette_device &palette) const
{
	static constexpr rgb_t tim011_pens[4] = {
		{ 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x55, 0x00 }, // 1
		{ 0x00, 0xaa, 0x00 }, // 2
		{ 0x00, 0xff, 0x00 }, // 3
	};

	palette.set_pen_colors(0, tim011_pens);
}

void tim011_state::tim011(machine_config &config)
{
	/* basic machine hardware */
	HD64180RP(config, m_maincpu, XTAL(12'288'000)); // location U17 HD64180
	m_maincpu->set_addrmap(AS_PROGRAM, &tim011_state::tim011_mem);
	m_maincpu->set_addrmap(AS_IO, &tim011_state::tim011_io);
	m_maincpu->tend1_wr_callback().set(m_fdc, FUNC(upd765a_device::tc_line_w));
	m_maincpu->txa1_wr_callback().set("rs232", FUNC(rs232_port_device::write_txd));

//  CDP1802(config, "keyboard", XTAL(1'750'000)); // CDP1802, unknown clock

	// FDC9266 location U43
	UPD765A(config, m_fdc, XTAL(8'000'000), true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, Z180_INPUT_LINE_DREQ1);

	/* floppy drives */
	FLOPPY_CONNECTOR(config, m_floppy[0], tim011_floppies, "35dd",  tim011_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], tim011_floppies, nullptr, tim011_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], tim011_floppies, nullptr, tim011_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], tim011_floppies, nullptr, tim011_floppy_formats);

	/* video hardware */
	PALETTE(config, m_palette, FUNC(tim011_state::tim011_palette), 4);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(tim011_state::screen_update_tim011));
	screen.set_palette(m_palette);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
	rs232.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa1_w));

	TIM011_EXPANSION_SLOT(config, m_exp, tim011_exp_devices, nullptr);
	m_exp->set_io_space(m_maincpu, AS_IO);
}

/* ROM definition */
ROM_START( tim011 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sys_tim011.u16", 0x0000, 0x2000, CRC(5b4f1300) SHA1(d324991c4292d7dcde8b8d183a57458be8a2be7b))
	ROM_REGION( 0x1000, "keyboard", ROMREGION_ERASEFF )
	ROM_LOAD( "keyb_tim011.bin", 0x0000, 0x1000, CRC(a99c40a6) SHA1(d6d505271d91df4e079ec3c0a4abbe75ae9d649b))
ROM_END

} // Anonymous namespace

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                    FULLNAME   FLAGS */
COMP( 1987, tim011, 0,      0,      tim011,  tim011, tim011_state, empty_init, "Mihajlo Pupin Institute", "TIM-011", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
