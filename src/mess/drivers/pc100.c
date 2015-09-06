// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-100

    preliminary driver by Angelo Salese
    Thanks to Carl for the i8259 tip;

    TODO:
    - floppy support (no images available right now);
    - i8259 works in edge triggering mode, kludged to work somehow.

    Notes:
    - First two POST checks are for the irqs, first one checks the timer irq:
    F8209: B8 FB 00                  mov     ax,0FBh
    F820C: E6 02                     out     2h,al
    F820E: B9 00 00                  mov     cx,0h
    F8211: FB                        sti
    F8212: 0A E4                     or      ah,ah <- irq fires here
    F8214: E1 FC                     loopz   0F8212h
    F8216: FA                        cli
    F8217: 0A E9                     or      ch,cl
    F8219: 74 15                     je      0F8230h
    - Second one is for the vblank irq timing:
        F8238: 8B D3                     mov     dx,bx
        F823A: 8B D9                     mov     bx,cx
        F823C: CF                        iret
    F824D: E4 02                     in      al,2h
    F824F: 8A E0                     mov     ah,al
    F8251: B0 EF                     mov     al,0EFh
    F8253: E6 02                     out     2h,al
    F8255: BB 00 00                  mov     bx,0h
    F8258: BA 00 00                  mov     dx,0h
    F825B: B9 20 4E                  mov     cx,4E20h
    F825E: FB                        sti
    F825F: E2 FE                     loop    0F825Fh ;calculates the vblank here
    F8261: FA                        cli
    F8262: 8A C4                     mov     al,ah
    F8264: E6 02                     out     2h,al
    F8266: 2B D3                     sub     dx,bx
    F8268: 81 FA 58 1B               cmp     dx,1B58h
    F826C: 78 06                     js      0F8274h ;error if DX is smaller than 0x1b58
    F826E: 81 FA 40 1F               cmp     dx,1F40h
    F8272: 78 0A                     js      0F827Eh ;error if DX is greater than 0x1f40
    F8274: B1 05                     mov     cl,5h
    F8276: E8 CB 03                  call    0F8644h
    F8279: E8 79 FF                  call    0F81F5h
    F827C: EB FE                     jmp     0F827Ch
    F827E: B0 FF                     mov     al,0FFh
    fwiw with current timings, we get DX=0x1f09, enough for passing the test;

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "imagedev/flopdrv.h"
#include "formats/mfi_dsk.h"
#include "formats/d88_dsk.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/msm58321.h"
#include "sound/beep.h"

class pc100_state : public driver_device
{
public:
	pc100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_rtc(*this, "rtc"),
		m_palette(*this, "palette"),
		m_kanji_rom(*this, "kanji"),
		m_vram(*this, "vram"),
		m_rtc_portc(0)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_device<msm58321_device> m_rtc;
	required_device<palette_device> m_palette;
	required_region_ptr<UINT16> m_kanji_rom;
	required_region_ptr<UINT16> m_vram;

	DECLARE_READ16_MEMBER(pc100_vram_r);
	DECLARE_WRITE16_MEMBER(pc100_vram_w);
	DECLARE_READ16_MEMBER(pc100_kanji_r);
	DECLARE_WRITE16_MEMBER(pc100_kanji_w);
	DECLARE_READ8_MEMBER(pc100_key_r);
	DECLARE_WRITE8_MEMBER(pc100_output_w);
	DECLARE_WRITE8_MEMBER(pc100_tc_w);
	DECLARE_READ8_MEMBER(pc100_shift_r);
	DECLARE_WRITE8_MEMBER(pc100_shift_w);
	DECLARE_READ8_MEMBER(pc100_vs_vreg_r);
	DECLARE_WRITE8_MEMBER(pc100_vs_vreg_w);
	DECLARE_WRITE8_MEMBER(pc100_crtc_addr_w);
	DECLARE_WRITE8_MEMBER(pc100_crtc_data_w);
	DECLARE_WRITE8_MEMBER(lower_mask_w);
	DECLARE_WRITE8_MEMBER(upper_mask_w);
	DECLARE_WRITE8_MEMBER(crtc_bank_w);
	DECLARE_WRITE8_MEMBER(rtc_porta_w);
	DECLARE_READ8_MEMBER(rtc_portc_r);
	DECLARE_WRITE8_MEMBER(rtc_portc_w);
	UINT16 m_kanji_addr;
	UINT8 m_timer_mode;

	UINT8 m_bank_r,m_bank_w;

	struct{
		UINT8 shift;
		UINT16 mask;
		UINT16 vstart;
		UINT8 addr;
		UINT8 reg[8];
	}m_crtc;
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_pc100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pc100_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_600hz_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_100hz_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_50hz_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(pc100_10hz_irq);


	WRITE_LINE_MEMBER(rtc_portc_0_w) { m_rtc_portc = (m_rtc_portc & ~(1 << 0)) | ((state & 1) << 0); }
	WRITE_LINE_MEMBER(rtc_portc_1_w) { m_rtc_portc = (m_rtc_portc & ~(1 << 1)) | ((state & 1) << 1); }
	WRITE_LINE_MEMBER(rtc_portc_2_w) { m_rtc_portc = (m_rtc_portc & ~(1 << 2)) | ((state & 1) << 2); }
	WRITE_LINE_MEMBER(rtc_portc_3_w) { m_rtc_portc = (m_rtc_portc & ~(1 << 3)) | ((state & 1) << 3); }
	UINT8 m_rtc_portc;
};

void pc100_state::video_start()
{
}

UINT32 pc100_state::screen_update_pc100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;
	int xi;
	int dot;
	int pen[4],pen_i;

	count = ((m_crtc.vstart + 0x20) * 0x40);

	for(y=0;y<512;y++)
	{
		count &= 0xffff;

		for(x=0;x<1024/16;x++)
		{
			for(xi=0;xi<16;xi++)
			{
				for(pen_i=0;pen_i<4;pen_i++)
					pen[pen_i] = (m_vram[count+pen_i*0x10000] >> xi) & 1;

				dot = 0;
				for(pen_i=0;pen_i<4;pen_i++)
					dot |= pen[pen_i]<<pen_i;

				if(y < 512 && x*16+xi < 768) /* TODO: safety check */
					bitmap.pix16(y, x*16+xi) = m_palette->pen(dot);
			}

			count++;
		}
	}

	return 0;
}

READ16_MEMBER( pc100_state::pc100_vram_r )
{
	return m_vram[offset+m_bank_r*0x10000];
}

WRITE16_MEMBER( pc100_state::pc100_vram_w )
{
	UINT16 old_vram;
	int i;

	for(i=0;i<4;i++)
	{
		if((m_bank_w >> i) & 1)
		{
			old_vram = m_vram[offset+i*0x10000];
			COMBINE_DATA(&m_vram[offset+i*0x10000]);
			m_vram[offset+i*0x10000] = ((m_vram[offset+i*0x10000]|(m_vram[offset+i*0x10000]<<16)) >> m_crtc.shift);
			if(ACCESSING_BITS_0_15)
				m_vram[offset+i*0x10000] = (m_vram[offset+i*0x10000] & ~m_crtc.mask) | (old_vram & m_crtc.mask);
			else if(ACCESSING_BITS_8_15)
				m_vram[offset+i*0x10000] = (m_vram[offset+i*0x10000] & ((~m_crtc.mask) & 0xff00)) | (old_vram & (m_crtc.mask|0xff));
			else if(ACCESSING_BITS_0_7)
				m_vram[offset+i*0x10000] = (m_vram[offset+i*0x10000] & ((~m_crtc.mask) & 0xff)) | (old_vram & (m_crtc.mask|0xff00));

		}
	}
}

static ADDRESS_MAP_START(pc100_map, AS_PROGRAM, 16, pc100_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0xbffff) AM_RAM // work ram
	AM_RANGE(0xc0000,0xdffff) AM_READWRITE(pc100_vram_r,pc100_vram_w) // vram, blitter based!
	AM_RANGE(0xf8000,0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

READ16_MEMBER( pc100_state::pc100_kanji_r )
{
	return m_kanji_rom[m_kanji_addr];
}


WRITE16_MEMBER( pc100_state::pc100_kanji_w )
{
	COMBINE_DATA(&m_kanji_addr);
}

READ8_MEMBER( pc100_state::pc100_key_r )
{
	if(offset)
		return ioport("DSW")->read(); // bit 5: horizontal/vertical monitor dsw

	return 0;
}

WRITE8_MEMBER( pc100_state::pc100_output_w )
{
	if(offset == 0)
	{
		m_timer_mode = (data & 0x18) >> 3;
		m_beeper->set_state(((data & 0x40) >> 6) ^ 1);
		printf("%02x\n",data & 0xc0);
	}
}

WRITE8_MEMBER( pc100_state::pc100_tc_w )
{
	machine().device<upd765a_device>("upd765")->tc_w(data & 0x40);
}

READ8_MEMBER( pc100_state::pc100_shift_r )
{
	return m_crtc.shift;
}

WRITE8_MEMBER( pc100_state::pc100_shift_w )
{
	m_crtc.shift = data & 0xf;
}

READ8_MEMBER( pc100_state::pc100_vs_vreg_r )
{
	if(offset)
		return m_crtc.vstart >> 8;

	return m_crtc.vstart & 0xff;
}

WRITE8_MEMBER( pc100_state::pc100_vs_vreg_w )
{
	if(offset)
		m_crtc.vstart = (m_crtc.vstart & 0xff) | (data << 8);
	else
		m_crtc.vstart = (m_crtc.vstart & 0xff00) | (data & 0xff);
}

WRITE8_MEMBER( pc100_state::pc100_crtc_addr_w )
{
	m_crtc.addr = data & 7;
}

WRITE8_MEMBER( pc100_state::pc100_crtc_data_w )
{
	m_crtc.reg[m_crtc.addr] = data;
	printf("%02x %02x\n",m_crtc.addr,data);
}


/* everything is 8-bit bus wide */
static ADDRESS_MAP_START(pc100_io, AS_IO, 16, pc100_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0x00ff) // i8259
//  AM_RANGE(0x04, 0x07) i8237?
	AM_RANGE(0x08, 0x0b) AM_DEVICE8("upd765", upd765a_device, map, 0x00ff ) // upd765
	AM_RANGE(0x10, 0x17) AM_DEVREADWRITE8("ppi8255_1", i8255_device, read, write,0x00ff) // i8255 #1
	AM_RANGE(0x18, 0x1f) AM_DEVREADWRITE8("ppi8255_2", i8255_device, read, write,0x00ff) // i8255 #2
	AM_RANGE(0x20, 0x23) AM_READ8(pc100_key_r,0x00ff) //i/o, keyboard, mouse
	AM_RANGE(0x22, 0x23) AM_WRITE8(pc100_output_w,0x00ff) //i/o, keyboard, mouse
	AM_RANGE(0x24, 0x25) AM_WRITE8(pc100_tc_w,0x00ff) //i/o, keyboard, mouse
//  AM_RANGE(0x28, 0x2b) i8251
	AM_RANGE(0x30, 0x31) AM_READWRITE8(pc100_shift_r,pc100_shift_w,0x00ff) // crtc shift
	AM_RANGE(0x38, 0x39) AM_WRITE8(pc100_crtc_addr_w,0x00ff) //crtc address reg
	AM_RANGE(0x3a, 0x3b) AM_WRITE8(pc100_crtc_data_w,0x00ff) //crtc data reg
	AM_RANGE(0x3c, 0x3f) AM_READWRITE8(pc100_vs_vreg_r,pc100_vs_vreg_w,0x00ff) //crtc vertical start position
	AM_RANGE(0x40, 0x5f) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
//  AM_RANGE(0x60, 0x61) crtc command (16-bit wide)
	AM_RANGE(0x80, 0x81) AM_READWRITE(pc100_kanji_r,pc100_kanji_w)
	AM_RANGE(0x82, 0x83) AM_WRITENOP //kanji-related?
	AM_RANGE(0x84, 0x87) AM_WRITENOP //kanji "strobe" signal 0/1
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( pc100 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Monitor" )
	PORT_DIPSETTING(    0x20, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout kanji_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8 },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( pc100 )
	GFXDECODE_ENTRY( "kanji", 0x0000, kanji_layout, 8, 1 )
GFXDECODE_END

/* TODO: untested */
WRITE8_MEMBER( pc100_state::rtc_porta_w )
{
/*
    ---- -x-- chip select
    ---- --x- read
    ---- ---x write
*/

	m_rtc->write_w((data >> 0) & 1);
	m_rtc->read_w((data >> 1) & 1);
	m_rtc->cs1_w((data >> 2) & 1);
}

WRITE8_MEMBER( pc100_state::rtc_portc_w )
{
	m_rtc->d0_w((data >> 0) & 1);
	m_rtc->d1_w((data >> 1) & 1);
	m_rtc->d2_w((data >> 2) & 1);
	m_rtc->d3_w((data >> 3) & 1);
}

READ8_MEMBER( pc100_state::rtc_portc_r )
{
	return m_rtc_portc;
}

WRITE8_MEMBER( pc100_state::lower_mask_w )
{
	m_crtc.mask = (m_crtc.mask & 0xff00) | data;
}

WRITE8_MEMBER( pc100_state::upper_mask_w )
{
	m_crtc.mask = (m_crtc.mask & 0xff) | (data << 8);
}

WRITE8_MEMBER( pc100_state::crtc_bank_w )
{
	m_bank_w = data & 0xf;
	m_bank_r = (data & 0x30) >> 4;
}

void pc100_state::machine_start()
{
}

void pc100_state::machine_reset()
{
	m_beeper->set_frequency(2400);
	m_beeper->set_state(0);
}

INTERRUPT_GEN_MEMBER(pc100_state::pc100_vblank_irq)
{
	machine().device<pic8259_device>("pic8259")->ir4_w(0);
	machine().device<pic8259_device>("pic8259")->ir4_w(1);
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_600hz_irq)
{
	if(m_timer_mode == 0)
	{
		machine().device<pic8259_device>("pic8259")->ir2_w(0);
		machine().device<pic8259_device>("pic8259")->ir2_w(1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_100hz_irq)
{
	if(m_timer_mode == 1)
	{
		machine().device<pic8259_device>("pic8259")->ir2_w(0);
		machine().device<pic8259_device>("pic8259")->ir2_w(1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_50hz_irq)
{
	if(m_timer_mode == 2)
	{
		machine().device<pic8259_device>("pic8259")->ir2_w(0);
		machine().device<pic8259_device>("pic8259")->ir2_w(1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc100_state::pc100_10hz_irq)
{
	if(m_timer_mode == 3)
	{
		machine().device<pic8259_device>("pic8259")->ir2_w(0);
		machine().device<pic8259_device>("pic8259")->ir2_w(1);
	}
}

static SLOT_INTERFACE_START( pc100_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

#define MASTER_CLOCK 6988800

static MACHINE_CONFIG_START( pc100, pc100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(pc100_map)
	MCFG_CPU_IO_MAP(pc100_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pc100_state, pc100_vblank_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("600hz", pc100_state, pc100_600hz_irq, attotime::from_hz(MASTER_CLOCK/600))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("100hz", pc100_state, pc100_100hz_irq, attotime::from_hz(MASTER_CLOCK/100))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("50hz", pc100_state, pc100_50hz_irq, attotime::from_hz(MASTER_CLOCK/50))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("10hz", pc100_state, pc100_10hz_irq, attotime::from_hz(MASTER_CLOCK/10))

	MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pc100_state, rtc_porta_w))
	MCFG_I8255_IN_PORTC_CB(READ8(pc100_state, rtc_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pc100_state, rtc_portc_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pc100_state, lower_mask_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(pc100_state, upper_mask_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pc100_state, crtc_bank_w))

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), GND, NULL )

	MCFG_UPD765A_ADD("upd765", true, true)

	MCFG_DEVICE_ADD("rtc", MSM58321, XTAL_32_768kHz)
	MCFG_MSM58321_D0_HANDLER(WRITELINE(pc100_state, rtc_portc_0_w))
	MCFG_MSM58321_D1_HANDLER(WRITELINE(pc100_state, rtc_portc_1_w))
	MCFG_MSM58321_D2_HANDLER(WRITELINE(pc100_state, rtc_portc_2_w))
	MCFG_MSM58321_D3_HANDLER(WRITELINE(pc100_state, rtc_portc_3_w))

	MCFG_FLOPPY_DRIVE_ADD("upd765:0", pc100_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", pc100_floppies, "525hd", floppy_image_device::default_floppy_formats)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	/* TODO: Unknown Pixel Clock and CRTC is dynamic */
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK*4, 1024, 0, 768, 264*2, 0, 512)
	MCFG_SCREEN_UPDATE_DRIVER(pc100_state, screen_update_pc100)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pc100)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_FORMAT(xxxxxxxBBBGGGRRR)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pc100 )
	ROM_REGION16_LE( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x8000, CRC(fd54a80e) SHA1(605a1b598e623ba2908a14a82454b9d32ea3c331))

	ROM_REGION16_LE( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, BAD_DUMP CRC(29298591) SHA1(d10174553ceea556fc53fc4e685d939524a4f64b))

	ROM_REGION16_LE( 0x20000*4, "vram", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY  FULLNAME       FLAGS */
COMP( 198?, pc100,  0,      0,       pc100,     pc100, driver_device,   0,      "NEC",   "PC-100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
