// license:???
// copyright-holders:Naibo Zhang
/* Galaxian 3

  -- bootable driver --
  by Naibo Zhang, April 2009


  System Overview:

  This is a Scaleable, Multi-CPU and Multi-User System.
  The largest scale configuration known so far was capable of 28(!) players and 16 screens wraped around. (retaired in the early 2000's)

  System has one Master 68020 CPU Board for game play, and one or more Slave 68020 CPU Boards for graphics.

  The Master CPU communicates with (one or more)Slave CPUs via multi-port shared RAM, the "C-RAM board".
  Each C-RAM board is configured as 2 banks of Triple-Port Static RAM Units. The 2 Master-CPU-Port of the 2 RAM banks
  are joined together, leaving 4 Slave-CPU-Ports. More than one C-RAM boards can be chained together.

  The Master CPU controls a Sound Board and a RSO board. RSO board contains Namco 139 serial communication ICs (x9 pieces),
  connecting to several "Personal Boards" (Player-Terminals), which handle players' input and vibration/lamp feedback.
  Each the Sound board, the RSO board, and the Personal board has their own 68000 CPU.

  Slave CPU connects to a cluster of namcos21-type video board sets: DSP-PGN-OBJ. A DSP board has 5x TMS320C25 running at 40MHz.

  Every 68020 CPU board has 512KB of local Hi-Speed SRAM. Most code/data required by the Slave CPU sub-system are
  uploaded from the Master CPU's ROM. Among them are:
  Slave CPU program, DSP program, Calculation Tables, Vertex Array Index, Palette, 2D Sprite Data, etc.

  The current dumped system supports 6 players. It has 2x 68020, 5x 68000 and 10x TMS320C25.


  System Diagram:
  ---------------
                                                                    Serial Comm Cable
     Master 68020 -------- RSO 68000, 9x Namco 139 SCI ICs <==============================> Personal Board 68000
          |                     |                                       ........ more personal boards.........
          |                     |                                       ........ more personal boards.........
          |                     |
          |                     |-------- Sound 68000, 4x Namco 140 ICs
     |---------|
     |  C-RAM  |
     | #-# #-# |
     |---------|
       | | | |
       | | | |------- Slave 68020
       | | |                |-------- 1x master DSP, 4x slave DSPs, Polygon, 2D Sprite ------> V-MIX board -----> SCREEN
       | | |                |-------- ........ more video boards .........                          |
       | | |                |-------- ........ more video boards .........                      LD Player
       | | |
       | | |------- ........ more slave 68020s .........
       | |
       | |------- ........ more slave 68020s .........
       |
       |------- ........ more slave 68020s .........



  * Thanks DarthNuno for scaning hi-quality PCB pictures.
  http://www.dragonslairfans.com/

---------------------------------------------------------------------------------

  the hardware sits somewhere between Namco S21 and Namco S22
  multiple boards are used to drive multiple screens and multiple laserdiscs

  the version here is for a 2 screen system

 ------- info from Andy -------------


Namco 'Galaxian 3 - Theater 6 : Project Dragoon'
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Dumped by:
Andrew Welburn
http://www.andys-arcade.com
on a cold evening 21/11/08


This romset was obtained from a 'spares' set of pcbs, the pcbs were never
used in a game, and the set included one of every type of pcb in the machine.
This means that there were 11 pcbs of 17 needed for a full set. The
additional 6 pcbs are all duplicates of pcbs in the set (see, what i got
was a 'spares' set ;)

Anyway...

from looking at Darth_nuno's photos, i can see that a full PCB set comprises :

2x backplane pcbs
2x CPU pcbs (one with master MST roms, one with slave SLV roms)
1x CRAM pcb
1x RS (RS0) pcb
1x SOUND pcb
2x OBJ pcb
2x PGN pcb
2x DSP pcb
1x VMIX pcb
3x PERSONAL pcbs

This make 17 pcbs in total needed for a complete set running Zolgear.

I have 11 pcbs (one of each type) one assumes it might be possible to run
the game with what i have, but who knows.

Darth_nuno's machine (and pcbs) are actually 'Attack of the Zolgear',
the update 'kit' for 'Namco Galaxian 3 theater 6'.

The pcb set i have dumped here is for a plain Galaxian 3 : Project Dragoon,
all my rom labels differ from his, and they are different games.

Of the 11 pcbs, 7 of them contain unique game roms. In order to keep
things clear, i have kept the roms apart in separate folders, each
folder contains a photo of the pcb itself and another text file
containing location descriptions.

enjoy.

Andrew Welburn


--------------
better notes (complete chip lists) for each board still needed
--------------

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/namcos2.h"
#include "cpu/tms32025/tms32025.h"
#include "includes/namcoic.h"
#include "machine/nvram.h"
#include "sound/c140.h"
#include "rendlay.h"


class gal3_state : public namcos2_shared_state
{
public:
	gal3_state(const machine_config &mconfig, device_type type, const char *tag)
		: namcos2_shared_state(mconfig, type, tag) ,
		m_rso_shared_ram(*this, "rso_shared_ram"),
		m_generic_paletteram_16(*this, "paletteram") { }

	UINT32 *m_mpSharedRAM0;
	//UINT32 *m_mpSharedRAM1;
	UINT16 m_namcos21_video_enable;
	required_shared_ptr<UINT16> m_rso_shared_ram;
	optional_shared_ptr<UINT16> m_generic_paletteram_16;
	UINT32 m_led_mst;
	UINT32 m_led_slv;
	DECLARE_READ32_MEMBER(led_mst_r);
	DECLARE_WRITE32_MEMBER(led_mst_w);
	DECLARE_READ32_MEMBER(led_slv_r);
	DECLARE_WRITE32_MEMBER(led_slv_w);
	DECLARE_READ32_MEMBER(paletteram32_r);
	DECLARE_WRITE32_MEMBER(paletteram32_w);
	DECLARE_READ32_MEMBER(namcos21_video_enable_r);
	DECLARE_WRITE32_MEMBER(namcos21_video_enable_w);
	DECLARE_READ32_MEMBER(rso_r);
	DECLARE_WRITE32_MEMBER(rso_w);
	DECLARE_VIDEO_START(gal3);
	UINT32 screen_update_gal3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_palette(  );
};


VIDEO_START_MEMBER(gal3_state,gal3)
{
	m_generic_paletteram_16.allocate(0x10000);

	c355_obj_init(
		0,      /* gfx bank */
		0xf,    /* reverse palette mapping */
		namcos2_shared_state::c355_obj_code2tile_delegate() );

}

/* FIXME: this code has simply been copypasted from namcos21.c
   (which has subsequently been rewritten to use generic MAME
   palette handling) with a 32-bit CPU it's rather unlikely
   that the palette RAM is actually laid out this way */

void gal3_state::update_palette(  )
{
	int i;
	INT16 data1,data2;
	int r,g,b;

	for( i=0; i<NAMCOS21_NUM_COLORS; i++ )
	{
		data1 = m_generic_paletteram_16[0x00000/2+i];
		data2 = m_generic_paletteram_16[0x10000/2+i];

		r = data1>>8;
		g = data1&0xff;
		b = data2&0xff;

		m_palette->set_pen_color( i, rgb_t(r,g,b) );
	}
} /* update_palette */

UINT32 gal3_state::screen_update_gal3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i;
	char mst[18], slv[18];
	static int pivot = 15;
	int pri;

	update_palette();

	if( machine().input().code_pressed_once(KEYCODE_H)&&(pivot<15) )    pivot+=1;
	if( machine().input().code_pressed_once(KEYCODE_J)&&(pivot>0) ) pivot-=1;

	for( pri=0; pri<pivot; pri++ )
	{
		c355_obj_draw(screen, bitmap, cliprect, pri);
	}

/*  CopyVisiblePolyFrameBuffer( bitmap, cliprect,0,0x7fbf );

    for( pri=pivot; pri<15; pri++ )
    {
        c355_obj_draw(screen, bitmap, cliprect, pri);
    }*/

	// CPU Diag LEDs
	mst[17]='\0', slv[17]='\0';
/// printf("mst=0x%x\tslv=0x%x\n", m_led_mst, m_led_slv);
	for(i=16;i<32;i++)
	{
		int t;
		if(i<24)
			t=i;
		else
			t=i+1;
		mst[8]=' '; slv[8]=' ';

		if(m_led_mst&(1<<i))
			mst[t-16]='*';
		else
			mst[t-16]='O';

		if(m_led_slv&(1<<i))
			slv[t-16]='*';
		else
			slv[t-16]='O';
	}

	popmessage("LED_MST:  %s\nLED_SLV:  %s\n2D Layer: 0-%d (Press H for +, J for -)\n", mst, slv, pivot);

	return 0;
}


/***************************************************************************************/

READ32_MEMBER(gal3_state::led_mst_r)
{
	return m_led_mst;
}

WRITE32_MEMBER(gal3_state::led_mst_w)
{
	COMBINE_DATA(&m_led_mst);
}

READ32_MEMBER(gal3_state::led_slv_r)
{
	return m_led_slv;
}

WRITE32_MEMBER(gal3_state::led_slv_w)
{
	COMBINE_DATA(&m_led_slv);
}

/* palette memory handlers */

READ32_MEMBER(gal3_state::paletteram32_r)
{
	offset *= 2;
	return (m_generic_paletteram_16[offset]<<16)|m_generic_paletteram_16[offset+1];
}

WRITE32_MEMBER(gal3_state::paletteram32_w)
{
	UINT32 v;
	offset *= 2;
	v = (m_generic_paletteram_16[offset]<<16)|m_generic_paletteram_16[offset+1];
	COMBINE_DATA( &v );
	m_generic_paletteram_16[offset+0] = v>>16;
	m_generic_paletteram_16[offset+1] = v&0xffff;
}

READ32_MEMBER(gal3_state::namcos21_video_enable_r)
{
	return m_namcos21_video_enable<<16;
}

WRITE32_MEMBER(gal3_state::namcos21_video_enable_w)
{
	UINT32 v;
	v = m_namcos21_video_enable<<16;
	COMBINE_DATA( &v ); // 0xff53, instead of 0x40 in namcos21
	m_namcos21_video_enable = v>>16;
}

READ32_MEMBER(gal3_state::rso_r)
{
	/*store $5555 @$0046, and readback @$0000
	read @$0144 and store at A6_21e & A4_5c
	Check @$009a==1 to start DEMO
	HACK*/
	offset *= 2;
	return (m_rso_shared_ram[offset]<<16)|m_rso_shared_ram[offset+1];
}

WRITE32_MEMBER(gal3_state::rso_w)
{
	UINT32 v;
	offset *= 2;
	v = (m_rso_shared_ram[offset]<<16)|m_rso_shared_ram[offset+1];
	COMBINE_DATA( &v );
	m_rso_shared_ram[offset+0] = v>>16;
	m_rso_shared_ram[offset+1] = v&0xffff;
}


static ADDRESS_MAP_START( cpu_mst_map, AS_PROGRAM, 32, gal3_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x20000000, 0x20001fff) AM_RAM AM_SHARE("nvmem")   //NVRAM
/// AM_RANGE(0x40000000, 0x4000ffff) AM_WRITE() //
	AM_RANGE(0x44000000, 0x44000003) AM_READ_PORT("DSW_CPU_mst" )
	AM_RANGE(0x44800000, 0x44800003) AM_READ(led_mst_r) AM_WRITE(led_mst_w) //LEDs
	AM_RANGE(0x48000000, 0x48000003) AM_READNOP //irq1 v-blank ack
	AM_RANGE(0x4c000000, 0x4c000003) AM_READNOP //irq3 ack
	AM_RANGE(0x60000000, 0x60007fff) AM_RAM AM_SHARE("share1")  //CRAM
	AM_RANGE(0x60010000, 0x60017fff) AM_RAM AM_SHARE("share1")  //Mirror
	AM_RANGE(0x80000000, 0x8007ffff) AM_RAM //512K Local RAM
/// AM_RANGE(0xc0000000, 0xc000000b) AM_WRITENOP    //upload?
	AM_RANGE(0xc000000c, 0xc000000f) AM_READNOP //irq2 ack
/// AM_RANGE(0xd8000000, 0xd800000f) AM_RAM // protection or 68681?
	AM_RANGE(0xf2800000, 0xf2800fff) AM_READWRITE(rso_r, rso_w) //RSO PCB
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_slv_map, AS_PROGRAM, 32, gal3_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
/// AM_RANGE(0x40000000, 0x4000ffff) AM_WRITE() //
	AM_RANGE(0x44000000, 0x44000003) AM_READ_PORT("DSW_CPU_slv" )
	AM_RANGE(0x44800000, 0x44800003) AM_READ(led_slv_r) AM_WRITE(led_slv_w) //LEDs
	AM_RANGE(0x48000000, 0x48000003) AM_READNOP //irq1 ack
/// AM_RANGE(0x50000000, 0x50000003) AM_READ() AM_WRITE()
/// AM_RANGE(0x54000000, 0x54000003) AM_READ() AM_WRITE()
	AM_RANGE(0x60000000, 0x60007fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x60010000, 0x60017fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x80000000, 0x8007ffff) AM_RAM //512K Local RAM

	AM_RANGE(0xf1200000, 0xf120ffff) AM_RAM //DSP RAM
/// AM_RANGE(0xf1400000, 0xf1400003) AM_WRITE(pointram_control_w)
/// AM_RANGE(0xf1440000, 0xf1440003) AM_READWRITE(pointram_data_r,pointram_data_w)
/// AM_RANGE(0x440002, 0x47ffff) AM_WRITENOP /* (frame buffer?) */
/// AM_RANGE(0xf1480000, 0xf14807ff) AM_READWRITE(namcos21_depthcue_r,namcos21_depthcue_w)
	AM_RANGE(0xf1700000, 0xf170ffff) AM_READWRITE16(c355_obj_ram_r,c355_obj_ram_w,0xffffffff) AM_SHARE("objram")
	AM_RANGE(0xf1720000, 0xf1720007) AM_READWRITE16(c355_obj_position_r,c355_obj_position_w,0xffffffff)
	AM_RANGE(0xf1740000, 0xf175ffff) AM_READWRITE(paletteram32_r,paletteram32_w)
	AM_RANGE(0xf1760000, 0xf1760003) AM_READWRITE(namcos21_video_enable_r,namcos21_video_enable_w)

	AM_RANGE(0xf2200000, 0xf220ffff) AM_RAM
	AM_RANGE(0xf2700000, 0xf270ffff) AM_RAM //AM_READWRITE16(c355_obj_ram_r,c355_obj_ram_w,0xffffffff) AM_SHARE("objram")
	AM_RANGE(0xf2720000, 0xf2720007) AM_RAM //AM_READWRITE16(c355_obj_position_r,c355_obj_position_w,0xffffffff)
	AM_RANGE(0xf2740000, 0xf275ffff) AM_RAM //AM_READWRITE(paletteram16_r,paletteram16_w) AM_SHARE("paletteram")
	AM_RANGE(0xf2760000, 0xf2760003) AM_RAM //AM_READWRITE(namcos21_video_enable_r,namcos21_video_enable_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rs_cpu_map, AS_PROGRAM, 16, gal3_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM //64K working RAM

/// AM_RANGE(0x180000, 0x183fff) AM_RAM //Nvram

	AM_RANGE(0x1c0000, 0x1c0001) AM_RAM //148?
	AM_RANGE(0x1c2000, 0x1c2001) AM_RAM //?
	AM_RANGE(0x1c4000, 0x1c4001) AM_RAM //?
	AM_RANGE(0x1c6000, 0x1c6001) AM_RAM //?
	AM_RANGE(0x1c8000, 0x1c8001) AM_RAM //?
	AM_RANGE(0x1ca000, 0x1ca001) AM_RAM //?
	AM_RANGE(0x1cc000, 0x1cc001) AM_RAM //?
	AM_RANGE(0x1ce000, 0x1ce001) AM_RAM //?
	AM_RANGE(0x1d2000, 0x1d2001) AM_RAM //?
	AM_RANGE(0x1d4000, 0x1d4001) AM_RAM //?
	AM_RANGE(0x1d6000, 0x1d6001) AM_RAM //?
	AM_RANGE(0x1de000, 0x1de001) AM_RAM //?
	AM_RANGE(0x1e4000, 0x1e4001) AM_RAM //?
	AM_RANGE(0x1e6000, 0x1e6001) AM_RAM //?

	AM_RANGE(0x200000, 0x200001) AM_RAM //?

	AM_RANGE(0x2c0000, 0x2c0001) AM_RAM //?
	AM_RANGE(0x2c0800, 0x2c0801) AM_RAM //?
	AM_RANGE(0x2c1000, 0x2c1001) AM_RAM //?
	AM_RANGE(0x2c1800, 0x2c1801) AM_RAM //?
	AM_RANGE(0x2c2000, 0x2c2001) AM_RAM //?
	AM_RANGE(0x2c2800, 0x2c2801) AM_RAM //?
	AM_RANGE(0x2c3000, 0x2c3001) AM_RAM //?
	AM_RANGE(0x2c3800, 0x2c3801) AM_RAM //?
	AM_RANGE(0x2c4000, 0x2c4001) AM_RAM //?

	AM_RANGE(0x300000, 0x300fff) AM_RAM AM_SHARE("rso_shared_ram")  //shared RAM

	AM_RANGE(0x400000, 0x400017) AM_RAM //MC68681?
	AM_RANGE(0x480000, 0x480017) AM_RAM //?
	AM_RANGE(0x500000, 0x500017) AM_RAM //?
	AM_RANGE(0x580000, 0x580017) AM_RAM //?
	AM_RANGE(0x600000, 0x600017) AM_RAM //?
	AM_RANGE(0x680000, 0x680017) AM_RAM //?

	AM_RANGE(0x800000, 0x80000f) AM_RAM //?
	AM_RANGE(0x840000, 0x843fff) AM_RAM //8 bit, 139 SCI RAM?
	AM_RANGE(0x880000, 0x88000f) AM_RAM //?
	AM_RANGE(0x8c0000, 0x8c3fff) AM_RAM //8 bit
	AM_RANGE(0x900000, 0x90000f) AM_RAM //?
	AM_RANGE(0x940000, 0x943fff) AM_RAM //8 bit
	AM_RANGE(0x980000, 0x98000f) AM_RAM //?
	AM_RANGE(0x9c0000, 0x9c3fff) AM_RAM //8 bit
	AM_RANGE(0xa00000, 0xa0000f) AM_RAM //?
	AM_RANGE(0xa40000, 0xa43fff) AM_RAM //8 bit
	AM_RANGE(0xa80000, 0xa8000f) AM_RAM //?
	AM_RANGE(0xac0000, 0xac3fff) AM_RAM //8 bit
	AM_RANGE(0xb00000, 0xb0000f) AM_RAM //?
	AM_RANGE(0xb40000, 0xb43fff) AM_RAM //8 bit
	AM_RANGE(0xb80000, 0xb8000f) AM_RAM //?
	AM_RANGE(0xbc0000, 0xbc3fff) AM_RAM //8 bit
	AM_RANGE(0xc00000, 0xc0000f) AM_RAM //?
	AM_RANGE(0xc40000, 0xc43fff) AM_RAM //8 bit

/// AM_RANGE(0xc44000, 0xffffff) AM_RAM /////////////
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_map, AS_PROGRAM, 16, gal3_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM
/// AM_RANGE(0x0c0000, 0x0cffff) AM_RAM //00, 20, 30, 40, 50
/// AM_RANGE(0x100000, 0x10000f) AM_RAM
	AM_RANGE(0x110000, 0x113fff) AM_RAM
/// AM_RANGE(0x120000, 0x120003) AM_RAM //2ieme byte
/// AM_RANGE(0x200000, 0x20017f) AM_RAM //C140
	AM_RANGE(0x200000, 0x2037ff) AM_DEVREADWRITE8("c140_16a", c140_device, c140_r, c140_w, 0x00ff)    //C140///////////
/// AM_RANGE(0x201000, 0x20117f) AM_RAM //C140
/// AM_RANGE(0x202000, 0x20217f) AM_RAM //C140
/// AM_RANGE(0x203000, 0x20317f) AM_RAM //C140
	AM_RANGE(0x204000, 0x2047ff) AM_DEVREADWRITE8("c140_16g", c140_device, c140_r, c140_w, 0x00ff)    //C140
/// AM_RANGE(0x090000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( psn_b1_cpu_map, AS_PROGRAM, 16, gal3_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( gal3 )
	PORT_START("DSW_CPU_mst")   //
	PORT_DIPNAME( 0x00010000, 0x00010000, "CPU_mst_DIPSW 1-1")
	PORT_DIPSETTING(      0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW 1-2")
	PORT_DIPSETTING(      0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW 1-3")
	PORT_DIPSETTING(      0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW 1-4")
	PORT_DIPSETTING(      0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW 1-5")
	PORT_DIPSETTING(      0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW 1-6")
	PORT_DIPSETTING(      0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW 1-7")
	PORT_DIPSETTING(      0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "DIPSW 1-8")
	PORT_DIPSETTING(      0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x01000000, 0x01000000, "DIPSW 2-1")
	PORT_DIPSETTING(      0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, "DIPSW 2-2")
	PORT_DIPSETTING(      0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW 2-3")
	PORT_DIPSETTING(      0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW 2-4")
	PORT_DIPSETTING(      0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "DIPSW 2-5")  //on pour zolgear?
	PORT_DIPSETTING(      0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, "DIPSW 2-6")  //on pour zolgear?
	PORT_DIPSETTING(      0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )

	PORT_START("DSW_CPU_slv")   //
	PORT_DIPNAME( 0x00010000, 0x00010000, "CPU_slv_DIPSW 3-1")
	PORT_DIPSETTING(      0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW 3-2")
	PORT_DIPSETTING(      0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW 3-3")
	PORT_DIPSETTING(      0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW 3-4")
	PORT_DIPSETTING(      0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW 3-5")
	PORT_DIPSETTING(      0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW 3-6")
	PORT_DIPSETTING(      0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW 3-7")
	PORT_DIPSETTING(      0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "DIPSW 3-8")
	PORT_DIPSETTING(      0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x01000000, 0x00000000, "DIPSW 4-1")  //on
	PORT_DIPSETTING(      0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x00000000, "DIPSW 4-2")  //on
	PORT_DIPSETTING(      0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW 4-3")
	PORT_DIPSETTING(      0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW 4-4")
	PORT_DIPSETTING(      0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "DIPSW 4-5")
	PORT_DIPSETTING(      0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, "DIPSW 4-6")
	PORT_DIPSETTING(      0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW 4-7")
	PORT_DIPSETTING(      0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW 4-8")
	PORT_DIPSETTING(      0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,4),  /* number of tiles */
	8,      /* bits per pixel */
	{       /* plane offsets */
		0,1,2,3,4,5,6,7
	},
	{ /* x offsets */
		0*8,RGN_FRAC(1,4)+0*8,RGN_FRAC(2,4)+0*8,RGN_FRAC(3,4)+0*8,
		1*8,RGN_FRAC(1,4)+1*8,RGN_FRAC(2,4)+1*8,RGN_FRAC(3,4)+1*8,
		2*8,RGN_FRAC(1,4)+2*8,RGN_FRAC(2,4)+2*8,RGN_FRAC(3,4)+2*8,
		3*8,RGN_FRAC(1,4)+3*8,RGN_FRAC(2,4)+3*8,RGN_FRAC(3,4)+3*8
	},
	{ /* y offsets */
		0*32,1*32,2*32,3*32,
		4*32,5*32,6*32,7*32,
		8*32,9*32,10*32,11*32,
		12*32,13*32,14*32,15*32
	},
	8*64 /* sprite offset */
};

static GFXDECODE_START( namcos21 )
	GFXDECODE_ENTRY( "obj_board1", 0x000000, tile_layout,  0x000, 0x20 )
GFXDECODE_END

static MACHINE_CONFIG_START( gal3, gal3_state )
	MCFG_CPU_ADD("maincpu", M68020, 49152000/2)
	MCFG_CPU_PROGRAM_MAP(cpu_mst_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", gal3_state,  irq1_line_hold)

	MCFG_CPU_ADD("cpuslv", M68020, 49152000/2)
	MCFG_CPU_PROGRAM_MAP(cpu_slv_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", gal3_state,  irq1_line_hold)

	MCFG_CPU_ADD("rs_cpu", M68000, 49152000/4)
	MCFG_CPU_PROGRAM_MAP(rs_cpu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", gal3_state,  irq5_line_hold)  /// programmable via 148 IC

	MCFG_CPU_ADD("sound_cpu", M68000, 12000000) // ??
	MCFG_CPU_PROGRAM_MAP(sound_cpu_map)

	MCFG_CPU_ADD("psn_b1_cpu", M68000, 12000000) // ??
	MCFG_CPU_PROGRAM_MAP(psn_b1_cpu_map)
/*
    MCFG_CPU_ADD("psn_b2_cpu", M68000, 12000000) // ??
    MCFG_CPU_PROGRAM_MAP(psn_b1_cpu_map,0)

    MCFG_CPU_ADD("psn_b3_cpu", M68000, 12000000) // ??
    MCFG_CPU_PROGRAM_MAP(psn_b1_cpu_map,0)
*/
	MCFG_QUANTUM_TIME(attotime::from_hz(60*8000)) /* 8000 CPU slices per frame */

	MCFG_NVRAM_ADD_0FILL("nvmem")

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 512-1, 0*8, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(gal3_state, screen_update_gal3)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 512-1, 0*8, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(gal3_state, screen_update_gal3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", namcos21)
	MCFG_PALETTE_ADD("palette", NAMCOS21_NUM_COLORS)

	MCFG_VIDEO_START_OVERRIDE(gal3_state,gal3)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_C140_ADD("c140_16g", 8000000/374)
	MCFG_C140_BANK_TYPE(C140_TYPE_SYSTEM21)    //to be verified
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_C140_ADD("c140_16a", 8000000/374)
	MCFG_C140_BANK_TYPE(C140_TYPE_SYSTEM21)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END

/*

**************************************************************************************************
MASTER CPU PCB
**************************************************************************************************

PCB markings:
screened : 8623961202 CPU020
Etched   : (8623963202)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-MST-PRG0E  6B  27c4001     PRG0E
GLC1-MST-PRG1E  10B 27c4001     PRG1E
GLC1-MST-PRG2E  14B 27c4001     PRG2E
GLC1-MST-PRG3E  18B 27c4001     PRG3E

**************************************************************************************************
SLAVE CPU PCB
**************************************************************************************************

PCB markings:
screened : 8623961202 CPU020
Etched   : (8623963202)


label       loc.    Device      Filename
--------------------------------------------------------
GLC-SLV-PRG0    6B  27c010A     PRG0.bin
GLC-SLV-PRG1    10B 27c010A     PRG1.bin
GLC-SLV-PRG2    14B 27c010A     PRG2.bin
GLC-SLV-PRG3    18B 27c010A     PRG3.bin

**************************************************************************************************
DSP PCB
**************************************************************************************************


PCB markings:
screened : 8623961703 DSP
Etched   : (8623963703) TSK-A


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-DSP-PTOH   2F  27c040      PTOH.BIN
GLC1-DSP-PTOU   2K  27c040      PTOU.BIN
GLC1-DSP-PTOL   2N  27c040      PTOL.BIN

**************************************************************************************************
OBJ PCB
**************************************************************************************************

PCB markings:
screened : 8623962002
Etched   : (8623964002)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-OBJ-OBJ0   9T  27c040      OBJ0.BIN
GLC1-OBJ-OBJ2   9W  27c4000     OBJ2.BIN
GLC1-OBJ-OBJ1   9Y  27c040      OBJ1.BIN
GLC1-OBJ-OBJ3   9Z  27c4000     OBJ0.BIN

**************************************************************************************************
PSN PCB
**************************************************************************************************


PCB markings:
screened : V1079603
Etched   : (V1079703)


label       loc.    Device      Filename
--------------------------------------------------------
GLC-PSN-VOL IC100   27c010A     VOL.bin
GLC-PSN-PRG0B   IC22    27c010A     PRG0B.bin
GLC-PSN-PRG0B   IC23    27c010A     PRG1B.bin


PCB markings:
screened : V107960701
Etched   : (V107970701) TSK-A

**************************************************************************************************
RS PCB
**************************************************************************************************

label       loc.    Device      Filename
--------------------------------------------------------
GLC-RS-PRGLB    18B 27c010      PRGLB.BIN
GLC-RS-PRGUB    19B 27c010      PRGUB.BIN

**************************************************************************************************
SOUND PCB
**************************************************************************************************


PCB markings:
screened : V107965101
Etched   : (V107975101)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-SND-VOI0   13A 27c040      VOI0.BIN
GLC1-SND-VOI2   13C 27c040      VOI2.BIN
GLC1-SND-VOI8   10G 27c040      VOI8.BIN
GLC1-SND-VOI9   11/12G  27c040      VOI9.BIN
GLC1-SND-VOI10  13G 27c040      VOI10.BIN
GLC1-SND-VOI11  14G 27c040      VOI11.BIN

GLC1-SND-PRG0   1H  27c1000     PRG0.BIN
GLC1-SND-PRG1   2H  27c1000     PRG1.BIN
GLC1-SND-DATA1  4/5H    27c1000     DATA1.BIN


*/


ROM_START( gal3 )
	/********* CPU-MST board x1 *********/
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "glc1-mst-prg0e.6b", 0x00003, 0x80000, CRC(5deccd72) SHA1(8d50779221538cc171469a691fabb17b62a8e664) )
	ROM_LOAD32_BYTE( "glc1-mst-prg1e.10b", 0x00002, 0x80000, CRC(b6144e3b) SHA1(33f63d881e7012db7f971b074bc5f876a66198b7) )
	ROM_LOAD32_BYTE( "glc1-mst-prg2e.14b", 0x00001, 0x80000, CRC(13381084) SHA1(486c1e136e6594ba68858e40246c5fb9bef1c0d2) )
	ROM_LOAD32_BYTE( "glc1-mst-prg3e.18b", 0x00000, 0x80000, CRC(7917584a) SHA1(ec22de8a3751099d37e14cd05c736c33baa1ee1d) )

	/********* CPU-SLV board x1 *********/
	ROM_REGION( 0x080000, "cpuslv", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "glc-slv-prg0.6b",  0x00003, 0x20000, CRC(75b2341a) SHA1(73616f5633f583b9ebfba2380fde3e7b08743e9f) )
	ROM_LOAD32_BYTE( "glc-slv-prg1.10b", 0x00002, 0x20000, CRC(f37ba6c0) SHA1(f8ee29ee4d341bfd6595e92c090865b8e5d9578c) )
	ROM_LOAD32_BYTE( "glc-slv-prg2.14b", 0x00001, 0x20000, CRC(c38a933e) SHA1(96c85db607d8527e927ef23fc53324172ddb861a) )
	ROM_LOAD32_BYTE( "glc-slv-prg3.18b", 0x00000, 0x20000, CRC(deae86d2) SHA1(1898955423b8da585b6319406566aad02db20d64) )

	/********* DSP board x2 *********/
	ROM_REGION32_BE( 0x400000, "dsp_board1", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )  /* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )  /* least significant */
	/* and 5x C67 (TMS320C25) */

	ROM_REGION32_BE( 0x400000, "dsp_board2", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )  /* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )  /* least significant */
	/* and 5x C67 (TMS320C25) */

	/********* OBJ board x2 *********/
	ROM_REGION( 0x200000, "obj_board1", 0 )
	ROM_LOAD( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD( "glc1-obj-obj1.9w", 0x080000, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD( "glc1-obj-obj2.9y", 0x100000, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD( "glc1-obj-obj3.9z", 0x180000, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	ROM_REGION( 0x200000, "obj_board2", 0 )
	ROM_LOAD( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD( "glc1-obj-obj1.9w", 0x080000, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD( "glc1-obj-obj2.9y", 0x100000, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD( "glc1-obj-obj3.9z", 0x180000, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	/********* PSN board x3 *********/
	ROM_REGION( 0x040000, "psn_b1_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b1_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_b2_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b2_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_b3_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b3_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	/********* RS board x1 *********/
	ROM_REGION( 0x040000, "rs_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-rs-prglb.18b", 0x000001, 0x20000, CRC(9d0c8d03) SHA1(8fffef622cd4440ea9f17882cd54a8a49fbbc148) )
	ROM_LOAD16_BYTE( "glc-rs-prgub.19b", 0x000000, 0x20000, CRC(125ad94c) SHA1(4e2e316b639e9a3a78ecd5c827f3309efa3bc78c) )

	/********* SOUND board x1 *********/
	ROM_REGION( 0x080000, "sound_cpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "glc1-snd-prg0.1h", 0x000000, 0x20000, CRC(4845481c) SHA1(3cf90b8b2351b2bc015bf273552e19e09f84ee70) )
	ROM_LOAD16_BYTE( "glc1-snd-prg1.2h", 0x000001, 0x20000, CRC(3b98c175) SHA1(26e59700347bab7fa10f029e781f993f3a97d257) )
	ROM_LOAD16_BYTE( "glc1-snd-data1.4h",0x040001, 0x20000, CRC(8c7135f5) SHA1(b8c3866c70ac1c431140d6cfe50d9273db7d9b68) )

	ROM_REGION( 0x200000, "samples", ROMREGION_ERASE00 )
	ROM_LOAD( "glc1-snd-voi0.13a", 0x000000, 0x80000, CRC(ef3bda56) SHA1(2cdfec1860a6d2bd645d83b42cc232643818a699) )
	ROM_LOAD( "glc1-snd-voi2.13c", 0x000000, 0x80000, CRC(ef3bda56) SHA1(2cdfec1860a6d2bd645d83b42cc232643818a699) ) // == voi0
	ROM_LOAD( "glc1-snd-voi8.10g", 0x000000, 0x80000, CRC(bba0c15b) SHA1(b0abc22fd1ae8a9970ad45d9ebdb38e6b06033a7) )
	ROM_LOAD( "glc1-snd-voi9.11g", 0x080000, 0x80000, CRC(dd1b1ee4) SHA1(b69af15acaa9c3d79d7758adc8722ff5c1129b76) )
	ROM_LOAD( "glc1-snd-voi10.13g",0x100000, 0x80000, CRC(1c1dedf4) SHA1(b6b9dac68103ff2206d731d409a557a71afd98f7) )
	ROM_LOAD( "glc1-snd-voi11.14g",0x180000, 0x80000, CRC(559e2a8a) SHA1(9a2f28305c6073a0b9b80a5d9617cc25a921e9d0))

	/********* Laserdiscs *********/
	/* used 2 apparently, no idea what they connect to */

	DISK_REGION( "laserdisc1" )
	DISK_IMAGE_READONLY( "gal3_ld1", 0, NO_DUMP )

	DISK_REGION( "laserdisc2" )
	DISK_IMAGE_READONLY( "gal3_ld2", 0, NO_DUMP )
ROM_END

/*    YEAR, NAME,     PARENT, MACHINE,  INPUT,  INIT, MONITOR,  COMPANY,   FULLNAME,                       FLAGS */
GAMEL( 199?, gal3,    0,     gal3,    gal3, driver_device,    0,    ROT0,  "Namco", "Galaxian 3 - Theater 6 : Project Dragoon", MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_dualhsxs )
//GAMEL( 199?, gal3zlgr,    0,        gal3,    gal3, driver_device,    0, ROT0,  "Namco", "Galaxian 3 - Theater 6 J2 : Attack of The Zolgear", MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_dualhsxs )
