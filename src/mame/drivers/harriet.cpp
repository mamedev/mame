// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Harriet (c) 1990 Quantel

    TODO:
    - PCB pics would be very useful
    - "Failed to read NVR" message.
    - hook-up keyboard/terminal, shouldn't be too hard by studying the code.

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class harriet_state : public driver_device
{
public:
	harriet_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 m_teletype_data;
	UINT8 m_teletype_status;

	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_WRITE8_MEMBER(serial_w);
	DECLARE_READ8_MEMBER(unk2_r);
	DECLARE_READ8_MEMBER(unk3_r);
	DECLARE_READ8_MEMBER(keyboard_status_r);
	DECLARE_WRITE8_MEMBER( kbd_put );
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
};

void harriet_state::video_start()
{
}

UINT32 harriet_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

/* tested at POST (PC=0x612), halts the CPU if cmp.b $f1000f.l, d0 for 0x4000 DBRAs */
READ8_MEMBER(harriet_state::unk_r)
{
	return machine().rand();
}

WRITE8_MEMBER(harriet_state::unk_w)
{
/*  if(offset)
        printf("%02x\n",data);
    else if(data != 0xcf)
        printf("$f1001d Control (offset 0) write %02x\n",data);*/
}

/* PC=0x676/0x694 */
READ8_MEMBER(harriet_state::unk2_r)
{
	return machine().rand();
}


WRITE8_MEMBER(harriet_state::serial_w)
{
	m_terminal->write(space, 0, data);
}

/* tested before putting data to serial port at PC=0x4c78 */
READ8_MEMBER(harriet_state::unk3_r)
{
	return 0xff;//machine().rand();
}

/* 0x314c bit 7: keyboard status */
READ8_MEMBER(harriet_state::keyboard_status_r)
{
	UINT8 res;

	res = m_teletype_status | m_teletype_data;
	m_teletype_status &= ~0x80;

	return res;
}

static ADDRESS_MAP_START( harriet_map, AS_PROGRAM, 16, harriet_state )
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x040000, 0x040fff) AM_RAM // NVRAM
	AM_RANGE(0x7f0000, 0x7fffff) AM_RAM // todo: boundaries, 0x7fe000 - 0x7fffff tested on boot
	AM_RANGE(0xf1000e, 0xf1000f) AM_READ8(unk_r,0x00ff)
	AM_RANGE(0xf1001c, 0xf1001f) AM_WRITE8(unk_w,0x00ff)
	AM_RANGE(0xf20022, 0xf20023) AM_READ8(unk2_r,0x00ff)
	AM_RANGE(0xf20024, 0xf20025) AM_READ8(unk2_r,0x00ff)
	AM_RANGE(0xf2002c, 0xf2002d) AM_READ8(unk3_r,0x00ff)
	AM_RANGE(0xf2002e, 0xf2002f) AM_WRITE8(serial_w,0x00ff)
//  AM_RANGE(0xf4001e, 0xf4001f) AM_READ8(keyboard_status_r,0x00ff)
//  AM_RANGE(0xf4002e, 0xf4002f) AM_READ8(keyboard_status_r,0x00ff)
	AM_RANGE(0xf4003e, 0xf4003f) AM_READ8(keyboard_status_r,0x00ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( harriet )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

WRITE8_MEMBER( harriet_state::kbd_put )
{
	m_teletype_data = data;
	m_teletype_status |= 0x80;
}

void harriet_state::machine_start()
{
}

void harriet_state::machine_reset()
{
}


static MACHINE_CONFIG_START( harriet, harriet_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68010,XTAL_8MHz) // TODO: clock
	MCFG_CPU_PROGRAM_MAP(harriet_map)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(harriet_state, kbd_put))

	MCFG_PALETTE_ADD("palette", 8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( harriet )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "harriet 36-74c.tfb v5.01 lobyte 533f.bin", 0x0001, 0x4000, CRC(f07fff76) SHA1(8288f7eaa8f4155e0e4746635f63ca2cc3da25d1) )
	ROM_LOAD16_BYTE( "harriet 36-74c.tdb v5.01 hibyte 2a0c.bin", 0x0000, 0x4000, CRC(a61f441d) SHA1(76af6eddd5c042f1b2eef590eb822379944b9b28) )
ROM_END

COMP( 1990, harriet,  0,  0, harriet,  harriet, driver_device,  0,    "Quantel",      "Harriet", MACHINE_IS_SKELETON )
