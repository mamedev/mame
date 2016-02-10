// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Killer Instinct hardware

    driver by Aaron Giles and Bryan McPhail

    Games supported:
        * Killer Instinct
        * Killer Instinct 2

    Known bugs:
        * the SRAM test fails in diagnostics; this is due to the fact that
          the test relies on executing out of the cache while it tromps
          over (and eventually restores) the instructions it is executing;
          this will likely never be fixed

****************************************************************************

Killer Instinct 1 / Killer Instinct 2
Rare/Nintendo, 1994/1995

PCB Layout
----------

This is a fighting game using a hard drive to hold the graphics + code, running on
what appears to be Williams Electronics manufactured hardware.

KILLER INSTINCT V4.0
5770-14397-03
(C)1994 Nintendo/Rare
(sticker - MIDWAY GAMES 44464 I457034  A-20333)
|---------------------------------------------------------------|
|       LED1  LED2       GAL          U10    U11    U12    U13  |
|  TDA7240   TL084  AD1851  10MHz                               |
|                      |--------|                               |
|                      |ANALOG  |                               |
|                      |DEVICES |                               |
|                      |ADSP2105|                               |
|                      |--------|                               |
|J3                                   U33    U34    U35    U36  |
|               71256 71256 71256 71256                         |
|               71256 71256 71256 71256      MT4C4001  MT4C4001 |
|J              71256 71256 71256 71256      MT4C4001  MT4C4001 |
|A              71256 71256 71256 71256      MT4C4001  MT4C4001 |
|M                                           MT4C4001  MT4C4001 |
|M    ULN2064B                               MT4C4001  MT4C4001 |
|A                                           MT4C4001  MT4C4001 |
|          *1                                MT4C4001  MT4C4001 |
|                        *5                  MT4C4001  MT4C4001 |
|                                                               |
|                               50MHz                       JP30|
|              *4                MAX705                         |
|                                 JP32  *2        *3        U98 |
|  DSW1  DSW2                                                   |
|       J7       J8           J6        S3                      |
|---------------------|---------------|-------------------------|
                      |     IDE44     |
                      |               |
                      |       *6      |
                      |               |
                      |     IDE44     |
                      |-|-----------|-|
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                  |-----|||||||||||||-----|
                  |     |-----------|     |
                  |                       |
                  |                       |
                  |                       |
                  |        Seagate        |
                  |                       |
                  |        ST9420AG       |
                  |                       |
                  |      2.5" H/Drive     |
                  |                       |
                  |                       |
                  |                       |
                  |                       |
                  |                       |
                  |                       |
                  |                       |
                  |                       |
                  |-----------------------|

Notes:
      GAL - GAL20V8 labelled 'KI-U1 A-19802' (DIP24)
      MT4C4001 - 1M x4 DRAM (SOJ28)
      71256 - IDT 71256 32k x8 SRAM (SOJ28)
      JP30 - 3 pin jumper to configure boot ROM. Set to 1-2. Settings are 1-2 = 4MBit. 2-3 = 8MBit.
      JP32 - 2 pin jumper to disable Watch Dog (Hard-wired on the PCB shorted 1-2)
      ADSP2105 - Analog Devices ADSP-2105 (PLCC68)
      J7 - 15 pin connector for player 3 controls
      J3 - 10 pin connector for extra controls
      J6 - 44 pin connector for 2.5" IDE hard drive
      H/drive - Seagate Marathon 2.5" IDE hard drive, model ST9420AG
        -for KI2, labelled 'L2.1 KILLER INSTINCT 2 DISK (C)1985 NINTENDO/RARE) CHS - 988/16/52 - 420.8MB
      For KI1 - H/drive - Seagate Marathon 2.5" IDE hard drive, model ST9150AG

      J8 - 8 pin connector for coin 3-4
      S3 - Reset push-button switch
      LED1 - H/Drive activity LED
      LED2 - Sound Active LED
      *1 Altera EPM7096LC68-10 labelled 'KI-U92 A-19488 (C)1994 NINTENDO/RARE' (PLCC68)
      *2 Altera MAX EPM7128ELC84-10 labelled 'KI-U103 A-19486 (C)1994 NINTENDO/RARE' (PLCC68)
      *3 Altera MAX EPM7128ELC84-10 labelled 'KI-U103 A-19486 (C)1994 NINTENDO/RARE' (PLCC68)
      *4 Altera EPM7032LC44 -15T labelled 'K12-U96 A-20351 (C)1996 NINTENDO/RARE' (PLCC44)
      *5 MIPS 4600-based CPU, heatsinked (QFP208)
      *6 Altera EPM7032LC44 -15T labelled 'K12-U1 A-20383 (C)1996 NINTENDO/RARE' (PLCC44)
         - This is an updated hard drive controller sub board used only with KI2

      ROMs
      ----
      U10 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U10 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U11 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U11 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U12 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U12 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U13 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U13 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U33 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U33 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U34 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U34 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U35 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U35 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U36 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U36 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U98 - ST 27C4001 EPROM labelled 'L1.4 KILLER INSTINCT U98 ROM 1 (C) 1994 Nintendo/Rare' (DIP32)

***************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/ataintf.h"
#include "machine/idehd.h"
#include "audio/dcs.h"


class kinst_state : public driver_device
{
public:
	enum
	{
		TIMER_IRQ0_STOP
	};

	kinst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_rambase2(*this, "rambase2"),
		m_control(*this, "control"),
		m_rombase(*this, "rombase"),
		m_maincpu(*this, "maincpu"),
		m_ata(*this, "ata"),
		m_dcs(*this, "dcs")
	{
	}

	required_shared_ptr<UINT32> m_rambase;
	required_shared_ptr<UINT32> m_rambase2;
	required_shared_ptr<UINT32> m_control;
	required_shared_ptr<UINT32> m_rombase;
	UINT32 *m_video_base;
	const UINT8 *m_control_map;
	DECLARE_READ32_MEMBER(kinst_control_r);
	DECLARE_WRITE32_MEMBER(kinst_control_w);
	DECLARE_READ32_MEMBER(kinst_ide_r);
	DECLARE_WRITE32_MEMBER(kinst_ide_w);
	DECLARE_READ32_MEMBER(kinst_ide_extra_r);
	DECLARE_WRITE32_MEMBER(kinst_ide_extra_w);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_DRIVER_INIT(kinst);
	DECLARE_DRIVER_INIT(kinst2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_kinst(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(irq0_start);
	required_device<mips3_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_device<dcs_audio_2k_device> m_dcs;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};



/* constants */
#define MASTER_CLOCK    XTAL_50MHz



/*************************************
 *
 *  Machine start
 *
 *************************************/

void kinst_state::machine_start()
{
	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);

	/* configure fast RAM regions */
	m_maincpu->add_fastram(0x08000000, 0x087fffff, FALSE, m_rambase2);
	m_maincpu->add_fastram(0x00000000, 0x0007ffff, FALSE, m_rambase);
	m_maincpu->add_fastram(0x1fc00000, 0x1fc7ffff, TRUE,  m_rombase);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void kinst_state::machine_reset()
{
	ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
	UINT16 *identify_device = hdd->identify_device_buffer();

	if (strncmp(machine().system().name, "kinst2", 6) != 0)
	{
		/* kinst: tweak the model number so we pass the check */
		identify_device[27] = ('S' << 8) | 'T';
		identify_device[28] = ('9' << 8) | '1';
		identify_device[29] = ('5' << 8) | '0';
		identify_device[30] = ('A' << 8) | 'G';
		identify_device[31] = (' ' << 8) | ' ';
	}
	else
	{
		/* kinst2: tweak the model number so we pass the check */
		identify_device[10] = ('0' << 8) | '0';
		identify_device[11] = ('S' << 8) | 'T';
		identify_device[12] = ('9' << 8) | '1';
		identify_device[13] = ('5' << 8) | '0';
		identify_device[14] = ('A' << 8) | 'G';
	}

	/* set a safe base location for video */
	m_video_base = &m_rambase[0x30000/4];
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

UINT32 kinst_state::screen_update_kinst(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	/* loop over rows and copy to the destination */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *src = &m_video_base[640/4 * y];
		UINT16 *dest = &bitmap.pix16(y, cliprect.min_x);
		int x;

		/* loop over columns */
		for (x = cliprect.min_x; x < cliprect.max_x; x += 2)
		{
			UINT32 data = *src++;

			/* store two pixels */
			*dest++ = (data >>  0) & 0x7fff;
			*dest++ = (data >> 16) & 0x7fff;
		}
	}
	return 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void kinst_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_IRQ0_STOP:
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in kinst_state::device_timer");
	}
}


INTERRUPT_GEN_MEMBER(kinst_state::irq0_start)
{
	device.execute().set_input_line(0, ASSERT_LINE);
	timer_set(attotime::from_usec(50), TIMER_IRQ0_STOP);
}


WRITE_LINE_MEMBER(kinst_state::ide_interrupt)
{
	m_maincpu->set_input_line(1, state);
}



/*************************************
 *
 *  IDE controller access
 *
 *************************************/

READ32_MEMBER(kinst_state::kinst_ide_r)
{
	return m_ata->read_cs0(space, offset / 2, mem_mask);
}


WRITE32_MEMBER(kinst_state::kinst_ide_w)
{
	m_ata->write_cs0(space, offset / 2, data, mem_mask);
}


READ32_MEMBER(kinst_state::kinst_ide_extra_r)
{
	return m_ata->read_cs1(space, 6, 0xff);
}


WRITE32_MEMBER(kinst_state::kinst_ide_extra_w)
{
	m_ata->write_cs1(space, 6, data, 0xff);
}



/*************************************
 *
 *  Control handling
 *
 *************************************/

READ32_MEMBER(kinst_state::kinst_control_r)
{
	UINT32 result;
	static const char *const portnames[] = { "P1", "P2", "VOLUME", "UNUSED", "DSW" };

	/* apply shuffling */
	offset = m_control_map[offset / 2];
	result = m_control[offset];

	switch (offset)
	{
		case 2:     /* $90 -- sound return */
			result = ioport(portnames[offset])->read();
			result &= ~0x0002;
			if (m_dcs->control_r() & 0x800)
				result |= 0x0002;
			break;

		case 0:     /* $80 */
		case 1:     /* $88 */
		case 3:     /* $98 */
			result = ioport(portnames[offset])->read();
			break;

		case 4:     /* $a0 */
			result = ioport(portnames[offset])->read();
			if (space.device().safe_pc() == 0x802d428)
				space.device().execute().spin_until_interrupt();
			break;
	}

	return result;
}


WRITE32_MEMBER(kinst_state::kinst_control_w)
{
	UINT32 olddata;

	/* apply shuffling */
	offset = m_control_map[offset / 2];
	olddata = m_control[offset];
	COMBINE_DATA(&m_control[offset]);

	switch (offset)
	{
		case 0:     /* $80 - VRAM buffer control */
			if (data & 4)
				m_video_base = &m_rambase[0x58000/4];
			else
				m_video_base = &m_rambase[0x30000/4];
			break;

		case 1:     /* $88 - sound reset */
			m_dcs->reset_w(~data & 0x01);
			break;

		case 2:     /* $90 - sound control */
			if (!(olddata & 0x02) && (m_control[offset] & 0x02))
				m_dcs->data_w(m_control[3]);
			break;

		case 3:     /* $98 - sound data */
			break;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, kinst_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x08000000, 0x087fffff) AM_RAM AM_SHARE("rambase2")
	AM_RANGE(0x10000080, 0x100000ff) AM_READWRITE(kinst_control_r, kinst_control_w) AM_SHARE("control")
	AM_RANGE(0x10000100, 0x1000013f) AM_READWRITE(kinst_ide_r, kinst_ide_w)
	AM_RANGE(0x10000170, 0x10000173) AM_READWRITE(kinst_ide_extra_r, kinst_ide_extra_w)
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("rombase")
ADDRESS_MAP_END




/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( kinst )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SPECIAL )  /* door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BILL1 )    /* bill */
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SPECIAL )  /* coin door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("VOLUME")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SPECIAL )  /* sound status */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0000fff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )   /* verify */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Blood Level" )
	PORT_DIPSETTING(          0x00000003, DEF_STR( High ))
	PORT_DIPSETTING(          0x00000002, DEF_STR( Medium ))
	PORT_DIPSETTING(          0x00000001, DEF_STR( Low ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( None ))
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ))
	PORT_DIPNAME( 0x00000008, 0x00000008, "Finishing Moves" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000008, DEF_STR( On ))
	PORT_DIPNAME( 0x00000010, 0x00000010, "Display Warning" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000010, DEF_STR( On ))
	PORT_DIPNAME( 0x00000020, 0x00000020, "Blood" )
	PORT_DIPSETTING(          0x00000020, "Red" )
	PORT_DIPSETTING(          0x00000000, "White" )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000100, 0x00000100, "Coinage Source" )
	PORT_DIPSETTING(          0x00000100, "Dipswitch" )
	PORT_DIPSETTING(          0x00000000, "Disk" )
	PORT_DIPNAME( 0x00003e00, 0x00003e00, DEF_STR( Coinage ))
	PORT_DIPSETTING(          0x00003e00, "USA-1" )
	PORT_DIPSETTING(          0x00003c00, "USA-2" )
	PORT_DIPSETTING(          0x00003a00, "USA-3" )
	PORT_DIPSETTING(          0x00003800, "USA-4" )
	PORT_DIPSETTING(          0x00003400, "USA-9" )
	PORT_DIPSETTING(          0x00003200, "USA-10" )
	PORT_DIPSETTING(          0x00003600, "USA-ECA" )
	PORT_DIPSETTING(          0x00003000, "USA-Free Play" )
	PORT_DIPSETTING(          0x00002e00, "German-1" )
	PORT_DIPSETTING(          0x00002c00, "German-2" )
	PORT_DIPSETTING(          0x00002a00, "German-3" )
	PORT_DIPSETTING(          0x00002800, "German-4" )
	PORT_DIPSETTING(          0x00002600, "German-ECA" )
	PORT_DIPSETTING(          0x00002000, "German-Free Play" )
	PORT_DIPSETTING(          0x00001e00, "French-1" )
	PORT_DIPSETTING(          0x00001c00, "French-2" )
	PORT_DIPSETTING(          0x00001a00, "French-3" )
	PORT_DIPSETTING(          0x00001800, "French-4" )
	PORT_DIPSETTING(          0x00001600, "French-ECA" )
	PORT_DIPSETTING(          0x00001000, "French-Free Play" )
	PORT_DIPNAME( 0x00004000, 0x00004000, "Coin Counters" )
	PORT_DIPSETTING(          0x00004000, "1" )
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPNAME( 0x00008000, 0x00008000, "Test Switch" )
	PORT_DIPSETTING(          0x00008000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( kinst2 )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SPECIAL )  /* door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BILL1 )    /* bill */
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SPECIAL )  /* coin door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("VOLUME")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SPECIAL )  /* sound status */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0000fff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )   /* verify */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Blood Level" )
	PORT_DIPSETTING(          0x00000003, DEF_STR( High ))
	PORT_DIPSETTING(          0x00000002, DEF_STR( Medium ))
	PORT_DIPSETTING(          0x00000001, DEF_STR( Low ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( None ))
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ))
	PORT_DIPNAME( 0x00000008, 0x00000008, "Finishing Moves" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000008, DEF_STR( On ))
	PORT_DIPNAME( 0x00000010, 0x00000010, "Display Warning" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000010, DEF_STR( On ))
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000100, 0x00000100, "Coinage Source" )
	PORT_DIPSETTING(          0x00000100, "Dipswitch" )
	PORT_DIPSETTING(          0x00000000, "Disk" )
	PORT_DIPNAME( 0x00003e00, 0x00003e00, DEF_STR( Coinage ))
	PORT_DIPSETTING(          0x00003e00, "USA-1" )
	PORT_DIPSETTING(          0x00003c00, "USA-2" )
	PORT_DIPSETTING(          0x00003a00, "USA-3" )
	PORT_DIPSETTING(          0x00003800, "USA-4" )
	PORT_DIPSETTING(          0x00003400, "USA-9" )
	PORT_DIPSETTING(          0x00003200, "USA-10" )
	PORT_DIPSETTING(          0x00003600, "USA-ECA" )
	PORT_DIPSETTING(          0x00003000, "USA-Free Play" )
	PORT_DIPSETTING(          0x00002e00, "German-1" )
	PORT_DIPSETTING(          0x00002c00, "German-2" )
	PORT_DIPSETTING(          0x00002a00, "German-3" )
	PORT_DIPSETTING(          0x00002800, "German-4" )
	PORT_DIPSETTING(          0x00002600, "German-ECA" )
	PORT_DIPSETTING(          0x00002000, "German-Free Play" )
	PORT_DIPSETTING(          0x00001e00, "French-1" )
	PORT_DIPSETTING(          0x00001c00, "French-2" )
	PORT_DIPSETTING(          0x00001a00, "French-3" )
	PORT_DIPSETTING(          0x00001800, "French-4" )
	PORT_DIPSETTING(          0x00001600, "French-ECA" )
	PORT_DIPSETTING(          0x00001000, "French-Free Play" )
	PORT_DIPNAME( 0x00004000, 0x00004000, "Coin Counters" )
	PORT_DIPSETTING(          0x00004000, "1" )
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPNAME( 0x00008000, 0x00008000, "Test Switch" )
	PORT_DIPSETTING(          0x00008000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END




/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( kinst, kinst_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R4600LE, MASTER_CLOCK*2)
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kinst_state,  irq0_start)


	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(kinst_state, ide_interrupt))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(kinst_state, screen_update_kinst)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BBBBBGGGGGRRRRR("palette")

	/* sound hardware */
	MCFG_DEVICE_ADD("dcs", DCS_AUDIO_2K, 0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( kinst )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki-l15d.u98", 0x00000, 0x80000, CRC(7b65ca3d) SHA1(607394d4ba1713f38c2cb5159303cace9cde991e) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "u10-l1", 0x000000, 0x80000, CRC(b6cc155f) SHA1(810d455df8f385d76143e9d7d048f2b555ff8bf0) )
	ROM_LOAD16_BYTE( "u11-l1", 0x200000, 0x80000, CRC(0b5e05df) SHA1(0595909cb667c38ac7c8c7bd0646b28899e27777) )
	ROM_LOAD16_BYTE( "u12-l1", 0x400000, 0x80000, CRC(d05ce6ad) SHA1(7a8ee405c118fd176b66353fa7bfab888cc63cd2) )
	ROM_LOAD16_BYTE( "u13-l1", 0x600000, 0x80000, CRC(7d0954ea) SHA1(ea4d1f153eb284f1bcfc5295fbce316bba6083f4) )
	ROM_LOAD16_BYTE( "u33-l1", 0x800000, 0x80000, CRC(8bbe4f0c) SHA1(b22e365bc8d58a80eaac226be14b4bb8d9a04844) )
	ROM_LOAD16_BYTE( "u34-l1", 0xa00000, 0x80000, CRC(b2e73603) SHA1(ee439f5162a2b3379d3f802328017bb3c68547d2) )
	ROM_LOAD16_BYTE( "u35-l1", 0xc00000, 0x80000, CRC(0aaef4fc) SHA1(48c4c954ac9db648f28ad64f9845e19ec432eec3) )
	ROM_LOAD16_BYTE( "u36-l1", 0xe00000, 0x80000, CRC(0577bb60) SHA1(cc78070cc41701e9a91fde5cfbdc7e1e83354854) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst", 0, SHA1(81d833236e994528d1482979261401b198d1ca53) )
ROM_END


ROM_START( kinst14 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki-l14.u98", 0x00000, 0x80000, CRC(afedb75f) SHA1(07254f20707377f7195e64675eb6458e663c1a9a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "u10-l1", 0x000000, 0x80000, CRC(b6cc155f) SHA1(810d455df8f385d76143e9d7d048f2b555ff8bf0) )
	ROM_LOAD16_BYTE( "u11-l1", 0x200000, 0x80000, CRC(0b5e05df) SHA1(0595909cb667c38ac7c8c7bd0646b28899e27777) )
	ROM_LOAD16_BYTE( "u12-l1", 0x400000, 0x80000, CRC(d05ce6ad) SHA1(7a8ee405c118fd176b66353fa7bfab888cc63cd2) )
	ROM_LOAD16_BYTE( "u13-l1", 0x600000, 0x80000, CRC(7d0954ea) SHA1(ea4d1f153eb284f1bcfc5295fbce316bba6083f4) )
	ROM_LOAD16_BYTE( "u33-l1", 0x800000, 0x80000, CRC(8bbe4f0c) SHA1(b22e365bc8d58a80eaac226be14b4bb8d9a04844) )
	ROM_LOAD16_BYTE( "u34-l1", 0xa00000, 0x80000, CRC(b2e73603) SHA1(ee439f5162a2b3379d3f802328017bb3c68547d2) )
	ROM_LOAD16_BYTE( "u35-l1", 0xc00000, 0x80000, CRC(0aaef4fc) SHA1(48c4c954ac9db648f28ad64f9845e19ec432eec3) )
	ROM_LOAD16_BYTE( "u36-l1", 0xe00000, 0x80000, CRC(0577bb60) SHA1(cc78070cc41701e9a91fde5cfbdc7e1e83354854) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst", 0, SHA1(81d833236e994528d1482979261401b198d1ca53) )
ROM_END


ROM_START( kinst13 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki-l13.u98", 0x00000, 0x80000, CRC(65f7ea31) SHA1(7f21620a512549db6821a0b4fa53681a767b7974) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "u10-l1", 0x000000, 0x80000, CRC(b6cc155f) SHA1(810d455df8f385d76143e9d7d048f2b555ff8bf0) )
	ROM_LOAD16_BYTE( "u11-l1", 0x200000, 0x80000, CRC(0b5e05df) SHA1(0595909cb667c38ac7c8c7bd0646b28899e27777) )
	ROM_LOAD16_BYTE( "u12-l1", 0x400000, 0x80000, CRC(d05ce6ad) SHA1(7a8ee405c118fd176b66353fa7bfab888cc63cd2) )
	ROM_LOAD16_BYTE( "u13-l1", 0x600000, 0x80000, CRC(7d0954ea) SHA1(ea4d1f153eb284f1bcfc5295fbce316bba6083f4) )
	ROM_LOAD16_BYTE( "u33-l1", 0x800000, 0x80000, CRC(8bbe4f0c) SHA1(b22e365bc8d58a80eaac226be14b4bb8d9a04844) )
	ROM_LOAD16_BYTE( "u34-l1", 0xa00000, 0x80000, CRC(b2e73603) SHA1(ee439f5162a2b3379d3f802328017bb3c68547d2) )
	ROM_LOAD16_BYTE( "u35-l1", 0xc00000, 0x80000, CRC(0aaef4fc) SHA1(48c4c954ac9db648f28ad64f9845e19ec432eec3) )
	ROM_LOAD16_BYTE( "u36-l1", 0xe00000, 0x80000, CRC(0577bb60) SHA1(cc78070cc41701e9a91fde5cfbdc7e1e83354854) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst", 0, SHA1(81d833236e994528d1482979261401b198d1ca53) )
ROM_END


ROM_START( kinstp )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki-p47.u98", 0x00000, 0x80000, CRC(05e67bcb) SHA1(501e69b3026394f69229a6e9866c1037502b86bb) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "u10-l1", 0x000000, 0x80000, CRC(b6cc155f) SHA1(810d455df8f385d76143e9d7d048f2b555ff8bf0) )
	ROM_LOAD16_BYTE( "u11-l1", 0x200000, 0x80000, CRC(0b5e05df) SHA1(0595909cb667c38ac7c8c7bd0646b28899e27777) )
	ROM_LOAD16_BYTE( "u12-l1", 0x400000, 0x80000, CRC(d05ce6ad) SHA1(7a8ee405c118fd176b66353fa7bfab888cc63cd2) )
	ROM_LOAD16_BYTE( "u13-l1", 0x600000, 0x80000, CRC(7d0954ea) SHA1(ea4d1f153eb284f1bcfc5295fbce316bba6083f4) )
	ROM_LOAD16_BYTE( "u33-l1", 0x800000, 0x80000, CRC(8bbe4f0c) SHA1(b22e365bc8d58a80eaac226be14b4bb8d9a04844) )
	ROM_LOAD16_BYTE( "u34-l1", 0xa00000, 0x80000, CRC(b2e73603) SHA1(ee439f5162a2b3379d3f802328017bb3c68547d2) )
	ROM_LOAD16_BYTE( "u35-l1", 0xc00000, 0x80000, CRC(0aaef4fc) SHA1(48c4c954ac9db648f28ad64f9845e19ec432eec3) )
	ROM_LOAD16_BYTE( "u36-l1", 0xe00000, 0x80000, CRC(0577bb60) SHA1(cc78070cc41701e9a91fde5cfbdc7e1e83354854) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst", 0, SHA1(81d833236e994528d1482979261401b198d1ca53) )
ROM_END


ROM_START( kinst2 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki2-l14.u98", 0x00000, 0x80000, CRC(27d0285e) SHA1(aa7a2a9d72a47dd0ea2ee7b2776b79288060b179) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END


ROM_START( kinst2k4 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki2-l14k.u98", 0x00000, 0x80000, CRC(9cbd00a8) SHA1(926dce4bb9016331ea40d3c337a9ace896f07493) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END


ROM_START( kinst213 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki2-l13.u98", 0x00000, 0x80000, CRC(25ebde3b) SHA1(771d150fb4de0a2ceb279954b9545458e93e2405) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END


ROM_START( kinst2k3 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki2-l13k.u98", 0x00000, 0x80000, CRC(3b4f16fc) SHA1(c28416f94453fd1f73ba01025276a04610569d12) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END


ROM_START( kinst211 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki2-l11.u98", 0x00000, 0x80000, CRC(0cb8de1e) SHA1(fe447f4b1d29b524f57c5ba1890652ef6afff88a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END


ROM_START( kinst210 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_LOAD( "ki2-l10.u98", 0x00000, 0x80000, CRC(b17b4b3d) SHA1(756629cd1b51ae50f2b9818765dd3d277c3019b3) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(kinst_state,kinst)
{
	static const UINT8 kinst_control_map[8] = { 0,1,2,3,4,5,6,7 };

	/* set up the control register mapping */
	m_control_map = kinst_control_map;
}


DRIVER_INIT_MEMBER(kinst_state,kinst2)
{
	static const UINT8 kinst2_control_map[8] = { 2,4,1,0,3,5,6,7 };

	// read: $80 on ki2 = $90 on ki
	// read: $88 on ki2 = $a0 on ki
	// write: $80 on ki2 = $90 on ki
	// write: $90 on ki2 = $88 on ki
	// write: $98 on ki2 = $80 on ki
	// write: $a0 on ki2 = $98 on ki

	/* set up the control register mapping */
	m_control_map = kinst2_control_map;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1994, kinst,    0,      kinst, kinst, kinst_state,  kinst,   ROT0, "Rare", "Killer Instinct (v1.5d)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, kinst14,  kinst,  kinst, kinst2, kinst_state, kinst,   ROT0, "Rare", "Killer Instinct (v1.4)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, kinst13,  kinst,  kinst, kinst2, kinst_state, kinst,   ROT0, "Rare", "Killer Instinct (v1.3)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, kinstp,   kinst,  kinst, kinst2, kinst_state, kinst,   ROT0, "Rare", "Killer Instinct (proto v4.7)", MACHINE_SUPPORTS_SAVE )

GAME( 1995, kinst2,   0,      kinst, kinst2, kinst_state, kinst2,  ROT0, "Rare", "Killer Instinct 2 (v1.4)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst2k4, kinst2, kinst, kinst2, kinst_state, kinst2,  ROT0, "Rare", "Killer Instinct 2 (v1.4k, upgrade kit)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst213, kinst2, kinst, kinst2, kinst_state, kinst2,  ROT0, "Rare", "Killer Instinct 2 (v1.3)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst2k3, kinst2, kinst, kinst2, kinst_state, kinst2,  ROT0, "Rare", "Killer Instinct 2 (v1.3k, upgrade kit)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst211, kinst2, kinst, kinst2, kinst_state, kinst2,  ROT0, "Rare", "Killer Instinct 2 (v1.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst210, kinst2, kinst, kinst2, kinst_state, kinst2,  ROT0, "Rare", "Killer Instinct 2 (v1.0)", MACHINE_SUPPORTS_SAVE )
