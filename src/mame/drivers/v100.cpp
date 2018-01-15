// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    The Visual 100 was the first of the second series of display terminals released by Visual Technology. (The Visual 200 and
    Visual 210 made up the first series.) It was followed by the Visual 110 (which emulated the Data General Dasher series rather
    than the DEC VT-100) and the "top of the line" Visual 400.

    The second 8251 and 8116T seem to have been typically unpopulated. However, the program does include routines to drive these
    components, which probably would be installed to provide the optional serial printer interface.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/er1400.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "video/tms9927.h"
#include "screen.h"

// character matrix is supposed to be only 7x7, but 15 produces correct timings
#define CHAR_WIDTH 15

class v100_state : public driver_device
{
public:
	v100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_vtac(*this, "vtac")
		, m_brg(*this, "brg%u", 1)
		, m_earom(*this, "earom")
		, m_picu(*this, "picu")
		, m_p_chargen(*this, "chargen")
		, m_videoram(*this, "videoram")
	{ }

	template<int n> DECLARE_WRITE8_MEMBER(brg_w);
	DECLARE_READ8_MEMBER(earom_r);
	DECLARE_WRITE8_MEMBER(picu_w);
	IRQ_CALLBACK_MEMBER(irq_ack);
	DECLARE_WRITE8_MEMBER(ppi_porta_w);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<crt5037_device> m_vtac;
	required_device_array<com8116_device, 2> m_brg;
	required_device<er1400_device> m_earom;
	required_device<i8214_device> m_picu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_videoram;
};

u32 v100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_vtac->screen_reset())
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	unsigned row = cliprect.top() / 10;
	unsigned scan = cliprect.top() % 10;
	unsigned x0 = cliprect.left();
	unsigned stride = (screen.visible_area().width() / CHAR_WIDTH) + 2;
	unsigned ch0 = (x0 / CHAR_WIDTH) + (row * stride) + 2;
	unsigned px0 = x0 % CHAR_WIDTH;

	for (unsigned y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		unsigned x = x0, ch = ch0, px = px0;
		u8 vchar = m_videoram[ch & m_videoram.mask()];
		u8 gfxdata = m_p_chargen[((vchar & 0x7f) << 4) | scan];
		while (x <= cliprect.right())
		{
			bitmap.pix(y, x) = BIT(gfxdata, 7) ? rgb_t::white() : rgb_t::black();
			x++;
			px++;
			if ((px & 1) == 0)
				gfxdata <<= 1;
			if (px >= CHAR_WIDTH)
			{
				vchar = m_videoram[++ch & m_videoram.mask()];
				gfxdata = m_p_chargen[((vchar & 0x7f) << 4) | scan];
				px = 0;
			}
		}
		scan++;
		if (scan >= 10)
		{
			row++;
			ch0 += stride;
			scan = 0;
		}
	}

	return 0;
}

void v100_state::machine_start()
{
	m_picu->inte_w(1);
	m_picu->etlg_w(1);
}

template<int n>
WRITE8_MEMBER(v100_state::brg_w)
{
	m_brg[n]->str_w(data & 0x0f);
	m_brg[n]->stt_w(data >> 4);
}

READ8_MEMBER(v100_state::earom_r)
{
	return m_earom->data_r();
}

WRITE8_MEMBER(v100_state::picu_w)
{
	m_picu->b_w((data & 0x0e) >> 1);
	m_picu->sgs_w(BIT(data, 4));
}

IRQ_CALLBACK_MEMBER(v100_state::irq_ack)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return (m_picu->a_r() << 1) | 0xf0;
}

WRITE8_MEMBER(v100_state::ppi_porta_w)
{
	m_vtac->set_clock_scale(BIT(data, 5) ? 0.5 : 1.0);
	m_screen->set_clock_scale(BIT(data, 5) ? 0.5 : 1.0);

	logerror("Writing %02X to PPI port A\n", data);
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, v100_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x4000, 0x4fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x5fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, v100_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_DEVREADWRITE("vtac", crt5037_device, read, write)
	AM_RANGE(0x10, 0x10) AM_WRITE(brg_w<0>)
	AM_RANGE(0x12, 0x12) AM_DEVREADWRITE("usart1", i8251_device, data_r, data_w)
	AM_RANGE(0x13, 0x13) AM_DEVREADWRITE("usart1", i8251_device, status_r, control_w)
	AM_RANGE(0x14, 0x14) AM_DEVREADWRITE("usart2", i8251_device, data_r, data_w)
	AM_RANGE(0x15, 0x15) AM_DEVREADWRITE("usart2", i8251_device, status_r, control_w)
	AM_RANGE(0x16, 0x16) AM_WRITE(brg_w<1>)
	AM_RANGE(0x20, 0x20) AM_READ(earom_r)
	// 0x30 - write ???
	AM_RANGE(0x40, 0x40) AM_NOP // read/write ???
	// 0x48 - write ???
	AM_RANGE(0x60, 0x60) AM_WRITE(picu_w)
	AM_RANGE(0x70, 0x73) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( v100 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( v100 )
	MCFG_CPU_ADD("maincpu", Z80, XTAL_47_736MHz / 12) // divider not verified
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(v100_state, irq_ack)

	MCFG_DEVICE_ADD("usart1", I8251, XTAL_47_736MHz / 12) // divider not verified

	MCFG_DEVICE_ADD("brg1", COM8116, 5068800) // TODO: clock and divisors for this customized variant
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("usart1", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("usart1", i8251_device, write_txc))

	MCFG_DEVICE_ADD("usart2", I8251, XTAL_47_736MHz / 12)

	MCFG_DEVICE_ADD("brg2", COM8116, 5068800)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("usart2", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("usart2", i8251_device, write_txc))

	MCFG_SCREEN_ADD("screen", RASTER)
	//MCFG_SCREEN_RAW_PARAMS(XTAL_47_736MHz / 2, 102 * CHAR_WIDTH, 0, 80 * CHAR_WIDTH, 260, 0, 240)
	MCFG_SCREEN_RAW_PARAMS(XTAL_47_736MHz, 170 * CHAR_WIDTH, 0, 132 * CHAR_WIDTH, 312, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(v100_state, screen_update)

	// FIXME: dot clock should be divided by char width
	MCFG_DEVICE_ADD("vtac", CRT5037, XTAL_47_736MHz)
	MCFG_TMS9927_CHAR_WIDTH(CHAR_WIDTH)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("picu", I8214, XTAL_47_736MHz / 12)
	MCFG_I8214_INT_CALLBACK(ASSERTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(v100_state, ppi_porta_w))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITELINE("earom", er1400_device, c3_w)) MCFG_DEVCB_BIT(6) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("earom", er1400_device, c2_w)) MCFG_DEVCB_BIT(5) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("earom", er1400_device, c1_w)) MCFG_DEVCB_BIT(4) MCFG_DEVCB_INVERT
	MCFG_I8255_OUT_PORTC_CB(DEVWRITELINE("earom", er1400_device, data_w)) MCFG_DEVCB_BIT(6) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("earom", er1400_device, clock_w)) MCFG_DEVCB_BIT(0) MCFG_DEVCB_INVERT

	MCFG_DEVICE_ADD("earom", ER1400, 0)
MACHINE_CONFIG_END



/**************************************************************************************************************

Visual 100. (VT-100 clone)
Chips: D780C-1 (Z80), CRT5037, D8255AC-5, uPB8214C, COM8116T-020, D8251AC, ER1400, 8-sw dip
Crystal: 47.736

***************************************************************************************************************/

ROM_START( v100 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "262-047.u108",  0x0000, 0x1000, CRC(e82f708c) SHA1(20ed83a41fd0703d72a20e170af971181cfbd575) )
	ROM_LOAD( "262-048.u110",  0x1000, 0x1000, CRC(830923d3) SHA1(108590234ff84b5856cc2784d738a2a625305953) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "241-001.u29",   0x0000, 0x0800, CRC(ef807141) SHA1(cbf3fed001811c5840b9a131d2d3133843cb3b6a) )
ROM_END

COMP( 1980, v100, 0, 0, v100, v100, v100_state, 0, "Visual Technology", "Visual 100", MACHINE_IS_SKELETON )
