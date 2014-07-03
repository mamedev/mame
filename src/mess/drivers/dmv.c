// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        NCR Decision Mate V

        04/01/2012 Skeleton driver.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"
#include "machine/pit8253.h"
#include "machine/dmv_keyb.h"
#include "sound/speaker.h"
#include "video/upd7220.h"
#include "dmv.lh"

class dmv_state : public driver_device
{
public:
	dmv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_hgdc(*this, "upd7220"),
			m_dmac(*this, "dma8237"),
			m_pit(*this, "pit8253"),
			m_fdc(*this, "i8272"),
			m_floppy0(*this, "i8272:0"),
			m_floppy1(*this, "i8272:1"),
			m_keyboard(*this, "keyboard"),
			m_speaker(*this, "speaker"),
			m_video_ram(*this, "video_ram"),
			m_palette(*this, "palette")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<dmv_keyboard_device> m_keyboard;
	required_device<speaker_sound_device> m_speaker;

	virtual void video_start();
	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(dmac_eop);
	DECLARE_WRITE_LINE_MEMBER(dmac_dack3);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(pit_out0);
	DECLARE_WRITE8_MEMBER(fdd_motor_w);
	DECLARE_READ8_MEMBER(sys_status_r);
	DECLARE_WRITE8_MEMBER(tc_set_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_WRITE8_MEMBER(ramsel_w);
	DECLARE_WRITE8_MEMBER(romsel_w);
	DECLARE_READ8_MEMBER(kb_mcu_port1_r);
	DECLARE_WRITE8_MEMBER(kb_mcu_port1_w);
	DECLARE_WRITE8_MEMBER(kb_mcu_port2_w);

	required_shared_ptr<UINT8> m_video_ram;
	required_device<palette_device> m_palette;

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	int         m_eop_line;
	int         m_dack3_line;
	int         m_sd_poll_state;
	int         m_floppy_motor;
	UINT8       m_ram[0x2000];
};

WRITE8_MEMBER(dmv_state::tc_set_w)
{
	m_fdc->tc_w(true);
}

WRITE8_MEMBER(dmv_state::leds_w)
{
	/*
	    LEDs    Value       Significance
	    ---------------------------------------
	    None    0xFF        Check complete
	    1+8     0x7E        Sumcheck error
	    2+8     0xBE        GDC error
	    3+8     0xDE        Disk drive error
	    4+8     0xEE        16-bit processor error
	    5+8     0xF6        Keyboard error
	    6+8     0xFA        DMA error
	    7+8     0xFC        Memory error
	    All     0x00        Processor error
	*/

	for(int i=0; i<8; i++)
		output_set_led_value(8-i, BIT(data, i));
}

WRITE8_MEMBER(dmv_state::ramsel_w)
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x1fff, m_ram);
}

WRITE8_MEMBER(dmv_state::romsel_w)
{
	m_maincpu->space(AS_PROGRAM).install_rom(0x0000, 0x1fff, memregion("maincpu")->base());
}

WRITE8_MEMBER(dmv_state::fdd_motor_w)
{
	m_pit->write_gate0(1);
	m_pit->write_gate0(0);

	m_floppy_motor = 0;
	m_floppy0->get_device()->mon_w(m_floppy_motor);
	m_floppy1->get_device()->mon_w(m_floppy_motor);
}

READ8_MEMBER(dmv_state::sys_status_r)
{
	/*
	    Main system status
	    x--- ---- FDD index
	    -x--- --- IRQ 2
	    --x--- -- IRQ 3
	    ---x--- - IRQ 4
	    ---- x--- FDC interrupt
	    ---- -x-- FDD ready
	    ---- --x- 16-bit CPU available (active low)
	    ---- ---x FDD motor (active low)
	*/
	UINT8 data = 0x00;

	if (m_floppy_motor)
		data |= 0x01;

	// 16-bit CPU not available
	data |= 0x02;

	if (!m_floppy0->get_device()->ready_r())
		data |= 0x04;

	if (m_fdc->get_irq())
		data |= 0x08;

	return data;
}

UPD7220_DISPLAY_PIXELS_MEMBER( dmv_state::hgdc_display_pixels )
{
	//TODO
}

UPD7220_DRAW_TEXT_LINE_MEMBER( dmv_state::hgdc_draw_text )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 * chargen = memregion("maincpu")->base() + 0x1000;

	for( int x = 0; x < pitch; x++ )
	{
		UINT8 tile = m_video_ram[((addr+x)*2) & 0x1ffff] & 0xff;

		for( int yi = 0; yi < lr; yi++)
		{
			UINT8 tile_data = chargen[(tile*16+yi) & 0x7ff];

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			for( int xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;
				int pen = (tile_data >> xi) & 1 ? 1 : 0;

				res_x = x * 8 + xi;
				res_y = y * lr + yi;

				if(!machine().first_screen()->visible_area().contains(res_x, res_y))
					continue;

				if(yi >= 16) { pen = 0; }

				bitmap.pix32(res_y, res_x) = palette[pen];
			}
		}
	}
}

static SLOT_INTERFACE_START( dmv_floppies )
		SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static ADDRESS_MAP_START(dmv_mem, AS_PROGRAM, 8, dmv_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_ROM
	AM_RANGE( 0x2000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dmv_io , AS_IO, 8, dmv_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(leds_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(ramsel_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(romsel_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(tc_set_w)
	AM_RANGE(0x13, 0x13) AM_READ(sys_status_r)
	AM_RANGE(0x14, 0x14) AM_WRITE(fdd_motor_w)
	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("kb_ctrl_mcu", upi41_cpu_device, upi41_master_r, upi41_master_w)
	AM_RANGE(0x50, 0x51) AM_DEVICE("i8272", i8272a_device, map)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0xa0, 0xa1) AM_DEVREADWRITE("upd7220", upd7220_device, read, write)

	//AM_RANGE(0xe0, 0xe7) RAM bankswitch
ADDRESS_MAP_END

READ8_MEMBER(dmv_state::kb_mcu_port1_r)
{
	return !(m_keyboard->sd_poll_r() & !m_sd_poll_state);
}

WRITE8_MEMBER(dmv_state::kb_mcu_port1_w)
{
	m_sd_poll_state = BIT(data, 1);
	m_keyboard->sd_poll_w(!m_sd_poll_state);
}

WRITE8_MEMBER(dmv_state::kb_mcu_port2_w)
{
	m_speaker->level_w(BIT(data, 0));
}

static ADDRESS_MAP_START( dmv_kb_ctrl_io, AS_IO, 8, dmv_state )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(kb_mcu_port1_r, kb_mcu_port1_w) // bit 0 data from kb, bit 1 data to kb
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(kb_mcu_port2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_map, AS_0, 8, dmv_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM  AM_SHARE("video_ram")
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( dmv )
INPUT_PORTS_END

void dmv_state::machine_start()
{
}

void dmv_state::machine_reset()
{
	m_eop_line = 0;
	m_dack3_line = 0;
	m_sd_poll_state = 0;
	m_floppy_motor = 1;
	m_maincpu->space(AS_PROGRAM).install_rom(0x0000, 0x1fff, memregion("maincpu")->base());
}

void dmv_state::video_start()
{
}

/* F4 Character Displayer */
static const gfx_layout dmv_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ STEP16(0,8) },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( dmv )
	GFXDECODE_ENTRY("maincpu", 0x1000, dmv_charlayout, 0, 1)
GFXDECODE_END


//------------------------------------------------------------------------------------
//   I8237
//------------------------------------------------------------------------------------

WRITE_LINE_MEMBER( dmv_state::dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	m_dmac->hack_w(state);
}

WRITE_LINE_MEMBER( dmv_state::dmac_eop )
{
	if (!(m_dack3_line || m_eop_line) && (m_dack3_line || state))
		m_fdc->tc_w(true);

	m_eop_line = state;
}

WRITE_LINE_MEMBER( dmv_state::dmac_dack3 )
{
	if (!(m_dack3_line || m_eop_line) && (state || m_eop_line))
		m_fdc->tc_w(true);

	m_dack3_line = state;
}

WRITE_LINE_MEMBER( dmv_state::pit_out0 )
{
	if (!state)
	{
		m_floppy_motor = 1;
		m_floppy0->get_device()->mon_w(m_floppy_motor);
		m_floppy1->get_device()->mon_w(m_floppy_motor);
	}
}

WRITE_LINE_MEMBER( dmv_state::fdc_irq )
{
	if (state)
		m_fdc->tc_w(false);
}

READ8_MEMBER(dmv_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(dmv_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}


static MACHINE_CONFIG_START( dmv, dmv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_24MHz / 6)
	MCFG_CPU_PROGRAM_MAP(dmv_mem)
	MCFG_CPU_IO_MAP(dmv_io)

	MCFG_CPU_ADD("kb_ctrl_mcu", I8741, XTAL_6MHz)
	MCFG_CPU_IO_MAP(dmv_kb_ctrl_io)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_DMV_KEYBOARD_ADD("keyboard")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dmv)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
	MCFG_DEFAULT_LAYOUT(layout_dmv)

	// devices
	MCFG_DEVICE_ADD("upd7220", UPD7220, XTAL_5MHz/2) // unk clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(dmv_state, hgdc_display_pixels)
	MCFG_UPD7220_DRAW_TEXT_CALLBACK_OWNER(dmv_state, hgdc_draw_text)

	MCFG_DEVICE_ADD( "dma8237", AM9517A, XTAL_4MHz )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(dmv_state, dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(dmv_state, dmac_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(dmv_state, memory_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(dmv_state, memory_write_byte))
	MCFG_I8237_IN_IOR_3_CB(DEVREAD8("i8272", i8272a_device, mdma_r))
	MCFG_I8237_OUT_IOW_3_CB(DEVWRITE8("i8272", i8272a_device, mdma_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(dmv_state, dmac_dack3))

	MCFG_I8272A_ADD( "i8272", true )
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(dmv_state, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("dma8237", am9517a_device, dreq3_w))
	MCFG_FLOPPY_DRIVE_ADD("i8272:0", dmv_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("i8272:1", dmv_floppies, "525dd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(50)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(dmv_state, pit_out0))
	//MCFG_PIT8253_CLK2(XTAL_24MHz / 3 / 16)
	//MCFG_PIT8253_OUT2_HANDLER(WRITELINE(dmv_state, timint_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dmv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dmv_norm.bin", 0x0000, 0x2000, CRC(bf25f3f0) SHA1(0c7dd37704db4799e340cc836f887cd543e5c964))

	ROM_REGION(0x400, "kb_ctrl_mcu", 0)
	ROM_LOAD( "dmv_kb_ctrl_mcu.bin", 0x0000, 0x0400, CRC(a03af298) SHA1(144cba41294c46f5ca79b7ad8ced0e4408168775))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME             FLAGS */
COMP( 1984, dmv,    0,       0,         dmv,    dmv, driver_device,  0,      "NCR",   "Decision Mate V",    GAME_NOT_WORKING | GAME_NO_SOUND)
