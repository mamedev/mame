// license:BSD-3-Clause
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
       | | |                |-------- ........ more video boards ......... (max 2 per slave?)      |
       | | |                                                                                   LD Player
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
#include "cpu/m68000/m68020.h"
#include "cpu/tms32025/tms32025.h"
#include "machine/nvram.h"
#include "sound/c140.h"
#include "layout/generic.h"
#include "speaker.h"
#include "namco_c355spr.h"
#include "namcos21_dsp_c67.h"
#include "namcos21_3d.h"
#include "emupal.h"


namespace {

class gal3_state : public driver_device
{
public:
	gal3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_c355spr(*this, "c355spr_%u", 1U),
		m_palette(*this, "palette_%u", 1U),
		m_rso_shared_ram(*this, "rso_shared_ram"),
		m_c140_16a(*this, "c140_16a"),
		m_c140_16g(*this, "c140_16g"),
		m_namcos21_3d(*this, "namcos21_3d_%u", 1U),
		m_namcos21_dsp_c67(*this, "namcos21dsp_c67_%u", 1U)
	{ }

	void gal3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device_array<namco_c355spr_device, 2> m_c355spr;
	required_device_array<palette_device, 2> m_palette;
	uint16_t m_video_enable[2];
	required_shared_ptr<uint16_t> m_rso_shared_ram;
	required_device<c140_device> m_c140_16a;
	required_device<c140_device> m_c140_16g;

	required_device_array<namcos21_3d_device, 2> m_namcos21_3d;
	required_device_array<namcos21_dsp_c67_device, 2> m_namcos21_dsp_c67;

	uint32_t m_led_mst = 0;
	uint32_t m_led_slv = 0;

	uint32_t led_mst_r();
	void led_mst_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t led_slv_r();
	void led_slv_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Screen> uint16_t video_enable_r();
	template<int Screen> void video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rso_r(offs_t offset);
	void rso_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// using ind16 for now because namco_c355spr_device::zdrawgfxzoom does not support rgb32, will probably need to be improved for LD use
	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpu_mst_map(address_map &map) ATTR_COLD;
	void cpu_slv_map(address_map &map) ATTR_COLD;
	void psn_b1_cpu_map(address_map &map) ATTR_COLD;
	void rs_cpu_map(address_map &map) ATTR_COLD;
	void sound_cpu_map(address_map &map) ATTR_COLD;
};


void gal3_state::machine_start()
{
	save_item(NAME(m_led_mst));
	save_item(NAME(m_led_slv));
}

void gal3_state::video_start()
{
	save_item(NAME(m_video_enable));
}

uint32_t gal3_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xff, cliprect); // TODO : actually laserdisc layer
	screen.priority().fill(0, cliprect);

	m_c355spr[0]->build_sprite_list_and_render_sprites(cliprect); // TODO : buffered?

	int i;
	char mst[18], slv[18];
	static int pivot = 15;
	int pri;

	if( machine().input().code_pressed_once(KEYCODE_H)&&(pivot<15) )    pivot+=1;
	if( machine().input().code_pressed_once(KEYCODE_J)&&(pivot>0) ) pivot-=1;

	for( pri=0; pri<pivot; pri++ )
	{
		m_c355spr[0]->draw(screen, bitmap, cliprect, pri);
	}

/*  CopyVisiblePolyFrameBuffer( bitmap, cliprect,0,0x7fbf );

    for( pri=pivot; pri<15; pri++ )
    {
       m_c355spr[0]->draw(screen, bitmap, cliprect, pri);
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

uint32_t gal3_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xff, cliprect); // TODO : actually laserdisc layer
	screen.priority().fill(0, cliprect);
	m_c355spr[1]->build_sprite_list_and_render_sprites(cliprect); // TODO : buffered?

	static int pivot = 15;
	int pri;

	if( machine().input().code_pressed_once(KEYCODE_H)&&(pivot<15) )    pivot+=1;
	if( machine().input().code_pressed_once(KEYCODE_J)&&(pivot>0) ) pivot-=1;

	for( pri=0; pri<pivot; pri++ )
	{
		m_c355spr[1]->draw(screen, bitmap, cliprect, pri);
	}

/*  CopyVisiblePolyFrameBuffer( bitmap, cliprect,0,0x7fbf );

    for( pri=pivot; pri<15; pri++ )
    {
       m_c355spr[1]->draw(screen, bitmap, cliprect, pri);
    }*/

	return 0;
}


/***************************************************************************************/

uint32_t gal3_state::led_mst_r()
{
	return m_led_mst;
}

void gal3_state::led_mst_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_led_mst);
}

uint32_t gal3_state::led_slv_r()
{
	return m_led_slv;
}

void gal3_state::led_slv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_led_slv);
}

template<int Screen>
uint16_t gal3_state::video_enable_r()
{
	return m_video_enable[Screen];
}

template<int Screen>
void gal3_state::video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_enable[Screen]); // 0xff53, instead of 0x40 in namcos21
}

uint16_t gal3_state::rso_r(offs_t offset)
{
	/*store $5555 @$0046, and readback @$0000
	read @$0144 and store at A6_21e & A4_5c
	Check @$009a==1 to start DEMO
	HACK*/
	return m_rso_shared_ram[offset];
}

void gal3_state::rso_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rso_shared_ram[offset]);
}


void gal3_state::cpu_mst_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x20000000, 0x20001fff).ram().share("nvmem");   //NVRAM
/// map(0x40000000, 0x4000ffff).w(FUNC(gal3_state::)); //
	map(0x44000000, 0x44000003).portr("DSW_CPU_mst");
	map(0x44800000, 0x44800003).r(FUNC(gal3_state::led_mst_r)).w(FUNC(gal3_state::led_mst_w)); //LEDs
	map(0x48000000, 0x48000003).nopr(); //irq1 v-blank ack
	map(0x4c000000, 0x4c000003).nopr(); //irq3 ack
	map(0x60000000, 0x60007fff).ram().share("share1");  //CRAM
	map(0x60010000, 0x60017fff).ram().share("share1");  //Mirror
	map(0x80000000, 0x8007ffff).ram(); //512K Local RAM
/// map(0xc0000000, 0xc000000b).nopw();    //upload?
	map(0xc000000c, 0xc000000f).nopr(); //irq2 ack
/// map(0xd8000000, 0xd800000f).ram(); // protection or 68681?
	map(0xf2800000, 0xf2800fff).rw(FUNC(gal3_state::rso_r), FUNC(gal3_state::rso_w)); //RSO PCB
}

void gal3_state::cpu_slv_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
/// map(0x40000000, 0x4000ffff).w(FUNC(gal3_state::)); //
	map(0x44000000, 0x44000003).portr("DSW_CPU_slv");
	map(0x44800000, 0x44800003).r(FUNC(gal3_state::led_slv_r)).w(FUNC(gal3_state::led_slv_w)); //LEDs
	map(0x48000000, 0x48000003).nopr(); //irq1 ack
/// map(0x50000000, 0x50000003).rw(FUNC(gal3_state::), FUNC(gal3_state::));
/// map(0x54000000, 0x54000003).rw(FUNC(gal3_state::), FUNC(gal3_state::));
	map(0x60000000, 0x60007fff).ram().share("share1");
	map(0x60010000, 0x60017fff).ram().share("share1");
	map(0x80000000, 0x8007ffff).ram(); //512K Local RAM

	// Video chain 1
	map(0xf1200000, 0xf120ffff).rw(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_hack_w));
	map(0xf1400000, 0xf1400003).w(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::pointram_control_w));
	map(0xf1440000, 0xf1440003).rw(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::pointram_data_r), FUNC(namcos21_dsp_c67_device::pointram_data_w));
	map(0xf1440004, 0xf147ffff).nopw();
	map(0xf1480000, 0xf14807ff).rw(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::namcos21_depthcue_r), FUNC(namcos21_dsp_c67_device::namcos21_depthcue_w));

	map(0xf1700000, 0xf170ffff).rw(m_c355spr[0], FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w)).share("objram_1");
	map(0xf1720000, 0xf1720007).rw(m_c355spr[0], FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0xf1740000, 0xf174ffff).rw(m_palette[0], FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette_1");
	map(0xf1750000, 0xf175ffff).rw(m_palette[0], FUNC(palette_device::read16_ext), FUNC(palette_device::write16_ext)).share("palette_1_ext");
	map(0xf1760000, 0xf1760001).rw(FUNC(gal3_state::video_enable_r<0>), FUNC(gal3_state::video_enable_w<0>));

	// Video chain 2
	map(0xf2200000, 0xf220ffff).rw(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_hack_w));
	map(0xf2400000, 0xf2400003).w(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::pointram_control_w));
	map(0xf2440000, 0xf2440003).rw(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::pointram_data_r), FUNC(namcos21_dsp_c67_device::pointram_data_w));
	map(0xf2440004, 0xf247ffff).nopw();
	map(0xf2480000, 0xf24807ff).rw(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::namcos21_depthcue_r), FUNC(namcos21_dsp_c67_device::namcos21_depthcue_w));

	map(0xf2700000, 0xf270ffff).rw(m_c355spr[1], FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w)).share("objram_2");
	map(0xf2720000, 0xf2720007).rw(m_c355spr[1], FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0xf2740000, 0xf274ffff).rw(m_palette[1], FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette_2");
	map(0xf2750000, 0xf275ffff).rw(m_palette[1], FUNC(palette_device::read16_ext), FUNC(palette_device::write16_ext)).share("palette_2_ext");
	map(0xf2760000, 0xf2760001).rw(FUNC(gal3_state::video_enable_r<1>), FUNC(gal3_state::video_enable_w<1>));
}

void gal3_state::rs_cpu_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); //64K working RAM

/// map(0x180000, 0x183fff).ram(); //Nvram

	map(0x1c0000, 0x1c0001).ram(); //148?
	map(0x1c2000, 0x1c2001).ram(); //?
	map(0x1c4000, 0x1c4001).ram(); //?
	map(0x1c6000, 0x1c6001).ram(); //?
	map(0x1c8000, 0x1c8001).ram(); //?
	map(0x1ca000, 0x1ca001).ram(); //?
	map(0x1cc000, 0x1cc001).ram(); //?
	map(0x1ce000, 0x1ce001).ram(); //?
	map(0x1d2000, 0x1d2001).ram(); //?
	map(0x1d4000, 0x1d4001).ram(); //?
	map(0x1d6000, 0x1d6001).ram(); //?
	map(0x1de000, 0x1de001).ram(); //?
	map(0x1e4000, 0x1e4001).ram(); //?
	map(0x1e6000, 0x1e6001).ram(); //?

	map(0x200000, 0x200001).ram(); //?

	map(0x2c0000, 0x2c0001).ram(); //?
	map(0x2c0800, 0x2c0801).ram(); //?
	map(0x2c1000, 0x2c1001).ram(); //?
	map(0x2c1800, 0x2c1801).ram(); //?
	map(0x2c2000, 0x2c2001).ram(); //?
	map(0x2c2800, 0x2c2801).ram(); //?
	map(0x2c3000, 0x2c3001).ram(); //?
	map(0x2c3800, 0x2c3801).ram(); //?
	map(0x2c4000, 0x2c4001).ram(); //?

	map(0x300000, 0x300fff).ram().share("rso_shared_ram");  //shared RAM

	map(0x400000, 0x400017).ram(); //MC68681?
	map(0x480000, 0x480017).ram(); //?
	map(0x500000, 0x500017).ram(); //?
	map(0x580000, 0x580017).ram(); //?
	map(0x600000, 0x600017).ram(); //?
	map(0x680000, 0x680017).ram(); //?

	map(0x800000, 0x80000f).ram(); //?
	map(0x840000, 0x843fff).ram(); //8 bit, 139 SCI RAM?
	map(0x880000, 0x88000f).ram(); //?
	map(0x8c0000, 0x8c3fff).ram(); //8 bit
	map(0x900000, 0x90000f).ram(); //?
	map(0x940000, 0x943fff).ram(); //8 bit
	map(0x980000, 0x98000f).ram(); //?
	map(0x9c0000, 0x9c3fff).ram(); //8 bit
	map(0xa00000, 0xa0000f).ram(); //?
	map(0xa40000, 0xa43fff).ram(); //8 bit
	map(0xa80000, 0xa8000f).ram(); //?
	map(0xac0000, 0xac3fff).ram(); //8 bit
	map(0xb00000, 0xb0000f).ram(); //?
	map(0xb40000, 0xb43fff).ram(); //8 bit
	map(0xb80000, 0xb8000f).ram(); //?
	map(0xbc0000, 0xbc3fff).ram(); //8 bit
	map(0xc00000, 0xc0000f).ram(); //?
	map(0xc40000, 0xc43fff).ram(); //8 bit

/// map(0xc44000, 0xffffff).ram(); /////////////
}

void gal3_state::sound_cpu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).ram();
/// map(0x0c0000, 0x0cffff).ram(); //00, 20, 30, 40, 50
/// map(0x100000, 0x10000f).ram();
	map(0x110000, 0x113fff).ram();
/// map(0x120000, 0x120003).ram(); //2ieme byte
/// map(0x200000, 0x20017f).ram(); //C140
	map(0x200000, 0x2037ff).rw(m_c140_16a, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w)).umask16(0x00ff);    //C140///////////
/// map(0x201000, 0x20117f).ram(); //C140
/// map(0x202000, 0x20217f).ram(); //C140
/// map(0x203000, 0x20317f).ram(); //C140
	map(0x204000, 0x2047ff).rw(m_c140_16g, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w)).umask16(0x00ff);    //C140
/// map(0x090000, 0xffffff).ram();
}

void gal3_state::psn_b1_cpu_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0xffffff).ram();
}

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

void gal3_state::gal3(machine_config &config)
{
	m68020_device &maincpu(M68020(config, "maincpu", 49152000/2));
	maincpu.set_addrmap(AS_PROGRAM, &gal3_state::cpu_mst_map);
	maincpu.set_vblank_int("lscreen", FUNC(gal3_state::irq1_line_hold));

	m68020_device &cpusly(M68020(config, "cpuslv", 49152000/2));
	cpusly.set_addrmap(AS_PROGRAM, &gal3_state::cpu_slv_map);
	cpusly.set_vblank_int("lscreen", FUNC(gal3_state::irq1_line_hold));

	m68000_device &rs_cpu(M68000(config, "rs_cpu", 49152000/4));
	rs_cpu.set_addrmap(AS_PROGRAM, &gal3_state::rs_cpu_map);
	rs_cpu.set_vblank_int("lscreen", FUNC(gal3_state::irq5_line_hold));  /// programmable via 148 IC

	m68000_device &sound_cpu(M68000(config, "sound_cpu", 49152000/4)); // ??
	sound_cpu.set_addrmap(AS_PROGRAM, &gal3_state::sound_cpu_map);

	m68000_device &psn_b1_cpu(M68000(config, "psn_b1_cpu", 12000000)); // ??
	psn_b1_cpu.set_addrmap(AS_PROGRAM, &gal3_state::psn_b1_cpu_map);
/*
    m68000_device &psn_b2_cpu(M68000(config, "psn_b2_cpu", 12000000)); // ??
    psn_b2_cpu.set_addrmap(AS_PROGRAM, &gal3_state::psn_b1_cpu_map);

    m68000_device &psn_b3_cpu(M68000(config, "psn_b3_cpu", 12000000)); // ??
    psn_b3_cpu.set_addrmap(AS_PROGRAM, &gal3_state::psn_b1_cpu_map);
*/
	config.set_maximum_quantum(attotime::from_hz(60*8000)); /* 8000 CPU slices per frame */

	NVRAM(config, "nvmem", nvram_device::DEFAULT_ALL_0);

	// video chain 1

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(64*8, 64*8);
	lscreen.set_visarea(0*8, 512-1, 0*8, 512-1);
	lscreen.set_screen_update(FUNC(gal3_state::screen_update_left));
	lscreen.set_palette(m_palette[0]);

	PALETTE(config, m_palette[0]).set_format(palette_device::xBRG_888, 0x10000/2);
	m_palette[0]->set_membits(16);

	NAMCO_C355SPR(config, m_c355spr[0], 0);
	m_c355spr[0]->set_screen("lscreen");
	m_c355spr[0]->set_palette(m_palette[0]);
	m_c355spr[0]->set_scroll_offsets(0x26, 0x19);
	m_c355spr[0]->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate());
	m_c355spr[0]->set_palxor(0xf); // reverse mapping
	m_c355spr[0]->set_color_base(0x1000); // TODO : verify palette offset
	m_c355spr[0]->set_external_prifill(true);

	NAMCOS21_3D(config, m_namcos21_3d[0], 0);
	m_namcos21_3d[0]->set_zz_shift_mult(11, 0x200);
	m_namcos21_3d[0]->set_depth_reverse(false);
	m_namcos21_3d[0]->set_framebuffer_size(496,480);

	NAMCOS21_DSP_C67(config, m_namcos21_dsp_c67[0], 0);
	m_namcos21_dsp_c67[0]->set_renderer_tag("namcos21_3d_1");

	// video chain 2

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(64*8, 64*8);
	rscreen.set_visarea(0*8, 512-1, 0*8, 512-1);
	rscreen.set_screen_update(FUNC(gal3_state::screen_update_right));
	rscreen.set_palette(m_palette[1]);

	PALETTE(config, m_palette[1]).set_format(palette_device::xBRG_888, 0x10000/2);
	m_palette[1]->set_membits(16);

	NAMCO_C355SPR(config, m_c355spr[1], 0);
	m_c355spr[1]->set_screen("rscreen");
	m_c355spr[1]->set_palette(m_palette[1]);
	m_c355spr[1]->set_scroll_offsets(0x26, 0x19);
	m_c355spr[1]->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate());
	m_c355spr[1]->set_palxor(0xf); // reverse mapping
	m_c355spr[1]->set_color_base(0x1000); // TODO : verify palette offset
	m_c355spr[1]->set_external_prifill(true);

	NAMCOS21_3D(config, m_namcos21_3d[1], 0);
	m_namcos21_3d[1]->set_zz_shift_mult(11, 0x200);
	m_namcos21_3d[1]->set_depth_reverse(false);
	m_namcos21_3d[1]->set_framebuffer_size(496,480);

	NAMCOS21_DSP_C67(config, m_namcos21_dsp_c67[1], 0);
	m_namcos21_dsp_c67[1]->set_renderer_tag("namcos21_3d_2");


	SPEAKER(config, "speaker", 2).front();

	// TODO: Total 5 of C140s in sound board, verified from gal3zlgr PCB - gal3 uses same board?
	C140(config, m_c140_16g, 49152000/2304);
	//m_c140_16g->set_addrmap(0, &gal3_state::c140_16g_map);    //to be verified
	m_c140_16g->add_route(0, "speaker", 0.50, 0);
	m_c140_16g->add_route(1, "speaker", 0.50, 1);

	C140(config, m_c140_16a, 49152000/2304);
	//m_c140_16a->set_addrmap(0, &gal3_state::c140_16a_map);    //to be verified
	m_c140_16a->add_route(0, "speaker", 0.50, 0);
	m_c140_16a->add_route(1, "speaker", 0.50, 1);
}

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
	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67_1:point24", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )  /* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )  /* least significant */

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67_2:point24", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )  /* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )  /* least significant */

	/********* OBJ board x2 *********/
	ROM_REGION( 0x200000, "c355spr_1", 0 )
	ROM_LOAD32_BYTE( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD32_BYTE( "glc1-obj-obj1.9w", 0x000001, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD32_BYTE( "glc1-obj-obj2.9y", 0x000002, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD32_BYTE( "glc1-obj-obj3.9z", 0x000003, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	ROM_REGION( 0x200000, "c355spr_2", 0 )
	ROM_LOAD32_BYTE( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD32_BYTE( "glc1-obj-obj1.9w", 0x000001, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD32_BYTE( "glc1-obj-obj2.9y", 0x000002, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD32_BYTE( "glc1-obj-obj3.9z", 0x000003, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

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
	ROM_LOAD( "glc1-snd-voi11.14g",0x180000, 0x80000, CRC(559e2a8a) SHA1(9a2f28305c6073a0b9b80a5d9617cc25a921e9d0) )

	/********* Laserdiscs *********/
	/* used 2 apparently, no idea what they connect to */

	DISK_REGION( "laserdisc1" )
	DISK_IMAGE_READONLY( "gal3_ld1", 0, NO_DUMP )

	DISK_REGION( "laserdisc2" )
	DISK_IMAGE_READONLY( "gal3_ld2", 0, NO_DUMP )
ROM_END

} // anonymous namespace


/*     YEAR  NAME     PARENT  MACHINE  INPUT  CLASS       INIT        MONITOR  COMPANY  FULLNAME                                    FLAGS */
GAMEL( 1992, gal3,    0,      gal3,    gal3,  gal3_state, empty_init, ROT0,    "Namco", "Galaxian 3 - Theater 6 : Project Dragoon", MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_dualhsxs )
//GAMEL( 1994, gal3zlgr,    0,        gal3,    gal3, driver_device,    0, ROT0,  "Namco", "Galaxian 3 - Theater 6 J2 : Attack of The Zolgear", MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_dualhsxs )
