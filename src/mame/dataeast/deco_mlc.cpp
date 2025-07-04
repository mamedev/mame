// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Data East MLC Hardware:

    The MLC system is basically an 8" x 6" x 2" plastic box with a JAMMA connector on it.
    The PCB's are very compact and have almost entirely surface mounted components on both
    sides of both boards. One board contains the RAM, sound hardware and I/O, the other
    board contains the CPU and ROMs. All main boards are identical between the MLC games and
    can be changed just by plugging in another game and pressing test to reset the EEPROM
    defaults.

    PCB Layout
    ----------

    Jamma Side:

    DE-0444-1
    |-----------------------------|
    |                             |
    | W24257            DE150     |
    |   W24257                    |
    |                             |
    |J                      93C45 |
    |A                            |
    |M                            |
    |M               DE223        |
    |A                            |
    |                             |
    |                42MHz  W24257|
    |                       W24257|
    |                       W24257|
    |            XILINK     W24257|
    |5            XC3130          |
    |0                            |
    |P                            |
    |I YMZ280B   YAC513           |
    |N                            |
    |-----------------------------|
    Notes:
        - Yamaha YMZ280B clock: 14.000MHz (42 / 3)
        - DE150 custom (GFX)
        - DE223 custom (GFX)
        - Xilinx XC3130 (TQFP100, Bus Controller?)
        - YAC513 D/A converter
        - 93C45 EEPROM, 128 bytes x 8 bit (equivalent to 93C46)
        - All SRAM is Winbond W24257S-70LL (32kx8)
        - 50 PIN connector looks like flat cable SCSI connector used on any regular PC SCSI controller card. It appears
        to be used for extra controls and hookup of a 2nd speaker for stereo output. It could also be used for externally
        programming some IC's or factory diagnostic/repairs?
        - Bottom side of the JAMMA side contains nothing significant except a sound AMP, test SW, LED, some
        logic chips and connectors for joining the PCBs together.
        - Vsync: 58Hz

    As the CPU is stored on the game board it is possible that each game could
    have a different CPU - however there are only two known configurations.  Avengers
    in Galactic Storm uses a SH2 processor, whereas all the others use a custom Deco
    processor (156 - encrypted ARM).

    Skull Fang:

    DE-0445-1 (top)                     DE-0445-1 (bottom)
    |-----------------------------|     |-----------------------------|
    |                             |     |                             |
    |                             |     |                             |
    |                             |     |           DE156     MCH-07  |
    |  MCH-06             SH00-0  |     |                             |
    |                             |     |                             |
    |                     SH01-0  |     |                             |
    |                             |     |                             |
    |                             |     |                             |
    |                             |     |                             |
    |                             |     |                             |
    |                             |     |                             |
    | MCH-04    MCH-02    MCH-00  |     |                             |
    |                             |     |                             |
    |                             |     | MCH-01    MCH-03    MCH-05  |
    |                             |     |                             |
    | SH02-0                      |     |                             |
    |                             |     |                             |
    |                             |     |                             |
    |                             |     |                             |
    |-----------------------------|     |-----------------------------|

    Notes:
        - DE156 clock: 7.000MHz (42MHz / 6, QFP100, clock measured on pin 90)

        Stadium Hero contains a '146' protection chip on the ROM/CPU pcb, but
        it is barely used by the game (only checked at startup). See deco146.cpp.

    Driver TODO:
        stadhr96 - protection? issues (or 156 co-processor? or timing?)
        avengrgs - doesn't generate enough line interrupts?
        ddream95 - seems to have a dual screen mode(??)
        skullfng - slowdowns not verified from real PCB, random hangs sometimes

    Graphic TODO:
        blending, raster effect features aren't fully emulated, verified currently
        Not verified : Can sprites affect 8bpp and shadowing simultaneously?
        Not verified what palette highest bits actually do
        Zoom algorithm is incorrect/unverified, reference: https://www.youtube.com/watch?v=eCwAVt0GbhQ

    Driver by Bryan McPhail, bmcphail@tendril.co.uk, thank you to Avedis and The Guru.

    Note: MLC stands for MotherLess Cassette

***************************************************************************/

#include "emu.h"
#include "deco_mlc.h"

#include "deco156_m.h"
#include "machine/eepromser.h"
#include "cpu/arm/arm.h"
#include "cpu/sh/sh7604.h"
#include "speaker.h"

#include <algorithm>

/***************************************************************************/

u32 deco_mlc_state::mlc_440008_r()
{
	return 0xffffffff;
}

u32 deco_mlc_state::mlc_44001c_r(offs_t offset)
{
/*
    test3 7 - vbl loop on 0x10 0000 at end of IRQ
    avengrgs tests other bits too
*/
//if (offset==0)
//  return machine().rand()|(machine().rand()<<16);
//  logerror("%08x:  Test3_r %d\n",m_maincpu->pc(),offset);
//  return 0x00100000;
	return 0xffffffff;
}

void deco_mlc_state::mlc_44001c_w(u32 data)
{
}

u32 deco_mlc_state::mlc_200070_r()
{
	if (!machine().side_effects_disabled())
		m_vbl_i ^= 0xffffffff;
//logerror("vbl r %08x\n", m_maincpu->pc());
	// Todo: Vblank probably in $10
	return m_vbl_i;
}

u32 deco_mlc_state::mlc_200000_r()
{
	return 0xffffffff;
}

u32 deco_mlc_state::mlc_200004_r()
{
	return 0xffffffff;
}

u32 deco_mlc_state::mlc_20007c_r()
{
	return 0xffffffff;
}

u32 deco_mlc_state::mlc_scanline_r()
{
//  logerror("read scanline counter (%d)\n", m_screen->vpos());
	return m_screen->vpos();
}


void deco_mlc_state::eeprom_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		const u8 ebyte = (data >> 8) & 0xff;
//      if (ebyte & 0x80)
//      {
			m_eeprom->clk_write((ebyte & 0x2) ? ASSERT_LINE : CLEAR_LINE);
			m_eeprom->di_write(ebyte & 0x1);
			m_eeprom->cs_write((ebyte & 0x4) ? ASSERT_LINE : CLEAR_LINE);
//      }
	}
	else if (ACCESSING_BITS_0_7)
	{
		// Master volume control (TODO: probably logarithmic)
		m_ymz->set_output_gain(0, (255.0 - data) / 255.0);
		m_ymz->set_output_gain(1, (255.0 - data) / 255.0);
	}
	else
		logerror("%s:  eprom_w %08x mask %08x\n",machine().describe_context(),data,mem_mask);
}

TIMER_DEVICE_CALLBACK_MEMBER(deco_mlc_state::interrupt_gen)
{
//  logerror("hit scanline IRQ %d (%08x)\n", m_screen->vpos(), info.i);
	m_maincpu->set_input_line(m_irqLevel, HOLD_LINE);
}

void deco_mlc_state::irq_ram_w(offs_t offset, u32 data, u32 mem_mask)
{
//  int scanline=m_screen->vpos();
	COMBINE_DATA(&m_irq_ram[offset]);

	/*
	TODO : Verify this on real PCB
	Word 0 : Used but unknown
	    skullfng : 0x00000cf3
	    hoops**  : 0xffffdfff
	    avengrgs : 0x00000cd3
	    stadhr96 : 0x000028f3

	Word 1 : 0xc0 at shadow, 0x00 at alpha, other bits unknown
	    skullfng : 0x000000c0 or 0x00000000
	    hoops**  : 0xfffffffc
	    avengrgs : 0xffffffff
	    stadhr96 : 0x0000fcc0

	Word 2 : Used but unknown
	    skullfng : 0x00200008
	    hoops**  : 0x00000000
	    avengrgs : 0x00000000
	    stadhr96 : 0x00200008

	Word 3 : Unknown(Always 0)
	*/

	switch (offset * 4)
	{
	case 0x10: // IRQ ack.  Value written doesn't matter
		m_maincpu->set_input_line(m_irqLevel, CLEAR_LINE);
		return;
	case 0x14: // Prepare scanline interrupt
		if(m_irq_ram[0x14 / 4] == -1) // TODO: likely to be anything that doesn't fit into the screen v-pos range.
			m_raster_irq_timer->adjust(attotime::never);
		else
			m_raster_irq_timer->adjust(m_screen->time_until_pos(m_irq_ram[0x14 / 4]));
		//logerror("prepare scanline to fire at %d (currently on %d)\n", m_irq_ram[0x14 / 4], m_screen->vpos());
		return;

	default:
		break;
	};

//  logerror("irqw %04x %04x (%d)\n", offset * 4, data & 0xffff, scanline);
}


u32 deco_mlc_state::spriteram_r(offs_t offset, u32 mem_mask)
{
	u32 retdata = 0;

	if (ACCESSING_BITS_16_31)
	{
		retdata |= 0xffff0000;
	}

	if (ACCESSING_BITS_0_15)
	{
		retdata |= m_spriteram[offset];
	}

	return retdata;
}


void  deco_mlc_state::spriteram_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
	}

	if (ACCESSING_BITS_0_15)
	{
		data &=0x0000ffff;
		COMBINE_DATA(&m_spriteram[offset]);
	}
}

u16 deco_mlc_state::sh96_protection_region_0_146_r(offs_t offset)
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	u8 cs = 0;
	u16 data = m_deco146->read_data( deco146_addr, cs );
	return data;
}

void deco_mlc_state::sh96_protection_region_0_146_w(offs_t offset, u16 data, u16 mem_mask)
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	u8 cs = 0;
	m_deco146->write_data( deco146_addr, data, mem_mask, cs );
}


/******************************************************************************/

void deco_mlc_state::avengrgs_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rom().mirror(0xff000000);
	map(0x0100000, 0x011ffff).ram().share("mainram").mirror(0xff000000);
	map(0x0200000, 0x0200003).r(FUNC(deco_mlc_state::mlc_200000_r)).mirror(0xff000000);
	map(0x0200004, 0x0200007).r(FUNC(deco_mlc_state::mlc_200004_r)).mirror(0xff000000);
	map(0x0200070, 0x0200073).r(FUNC(deco_mlc_state::mlc_200070_r)).mirror(0xff000000);
	map(0x0200074, 0x0200077).r(FUNC(deco_mlc_state::mlc_scanline_r)).mirror(0xff000000);
	map(0x020007c, 0x020007f).r(FUNC(deco_mlc_state::mlc_20007c_r)).mirror(0xff000000);
	map(0x0200000, 0x020007f).w(FUNC(deco_mlc_state::irq_ram_w)).share("irq_ram").mirror(0xff000000);
	map(0x0200080, 0x02000ff).ram().share("clip_ram").mirror(0xff000000);
	map(0x0204000, 0x0206fff).rw(FUNC(deco_mlc_state::spriteram_r), FUNC(deco_mlc_state::spriteram_w)).mirror(0xff000000);
	map(0x0280000, 0x029ffff).ram().share("vram").mirror(0xff000000);
	map(0x0300000, 0x0307fff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).umask32(0x0000ffff).cswidth(32).share("palette").mirror(0xff000000);
	map(0x0400000, 0x0400003).portr("INPUTS").mirror(0xff000000);
	map(0x0440000, 0x0440003).portr("INPUTS2").mirror(0xff000000);
	map(0x0440004, 0x0440007).portr("INPUTS3").mirror(0xff000000);
	map(0x0440008, 0x044000b).r(FUNC(deco_mlc_state::mlc_440008_r)).mirror(0xff000000);
	map(0x044001c, 0x044001f).rw(FUNC(deco_mlc_state::mlc_44001c_r), FUNC(deco_mlc_state::mlc_44001c_w)).mirror(0xff000000);
	map(0x0500000, 0x0500003).w(FUNC(deco_mlc_state::eeprom_w)).mirror(0xff000000);
	map(0x0600000, 0x0600007).rw(m_ymz, FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask32(0xff000000).mirror(0xff000000);
}

void deco_mlc_state::decomlc_no146_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rom();
	map(0x0100000, 0x011ffff).ram().share("mainram");
	map(0x0200000, 0x0200003).r(FUNC(deco_mlc_state::mlc_200000_r));
	map(0x0200004, 0x0200007).r(FUNC(deco_mlc_state::mlc_200004_r));
	map(0x0200070, 0x0200073).r(FUNC(deco_mlc_state::mlc_200070_r));
	map(0x0200074, 0x0200077).r(FUNC(deco_mlc_state::mlc_scanline_r));
	map(0x020007c, 0x020007f).r(FUNC(deco_mlc_state::mlc_20007c_r));
	map(0x0200000, 0x020007f).w(FUNC(deco_mlc_state::irq_ram_w)).share("irq_ram");
	map(0x0200080, 0x02000ff).ram().share("clip_ram");
	map(0x0204000, 0x0206fff).rw(FUNC(deco_mlc_state::spriteram_r), FUNC(deco_mlc_state::spriteram_w));
	map(0x0280000, 0x029ffff).ram().share("vram");
	map(0x0300000, 0x0307fff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).umask32(0x0000ffff).cswidth(32).share("palette");
	map(0x0400000, 0x0400003).portr("INPUTS");
	map(0x0440000, 0x0440003).portr("INPUTS2");
	map(0x0440004, 0x0440007).portr("INPUTS3");
	map(0x0440008, 0x044000b).r(FUNC(deco_mlc_state::mlc_440008_r));
	map(0x044001c, 0x044001f).rw(FUNC(deco_mlc_state::mlc_44001c_r), FUNC(deco_mlc_state::mlc_44001c_w));
	map(0x0500000, 0x0500003).w(FUNC(deco_mlc_state::eeprom_w));
	map(0x0600000, 0x0600007).rw(m_ymz, FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask32(0xff000000);
}

void deco_mlc_state::decomlc_146_map(address_map &map)
{
	decomlc_no146_map(map);

	map(0x070f000, 0x070ffff).rw(FUNC(deco_mlc_state::sh96_protection_region_0_146_r), FUNC(deco_mlc_state::sh96_protection_region_0_146_w)).umask32(0xffff0000);
}

/******************************************************************************/

static INPUT_PORTS_START( mlc )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x080000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout spritelayout_4bpp =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+16, RGN_FRAC(1,2)+0, 16, 0 },
	{ 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	16*32
};

static const gfx_layout spritelayout_5bpp =
{
	16,16,
	RGN_FRAC(1,3),
	5,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(1,3)+16, RGN_FRAC(1,3)+0, 16, 0 },
	{ 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	16*32
};

static const gfx_layout spritelayout_6bpp =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+16, RGN_FRAC(2,3)+0, RGN_FRAC(1,3)+16, RGN_FRAC(1,3)+0, 16, 0 },
	{ 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	16*32
};

static const gfx_layout spritelayout_acchi =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static GFXDECODE_START( gfx_deco_mlc )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout_4bpp,   0, 256 )
GFXDECODE_END

static GFXDECODE_START( gfx_5bpp )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout_5bpp,   0, 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_6bpp )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout_6bpp,   0,  64 )
GFXDECODE_END

static GFXDECODE_START( gfx_acchi )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout_acchi,   0,  64 )
GFXDECODE_END

/******************************************************************************/

void deco_mlc_state::machine_reset()
{
	m_vbl_i = 0xffffffff;
}

void deco_mlc_state::avengrgs(machine_config &config)
{
	// basic machine hardware
	SH7604(config, m_maincpu, 42000000/2); // 21 MHz clock confirmed on real board
	m_maincpu->set_addrmap(AS_PROGRAM, &deco_mlc_state::avengrgs_map);

	EEPROM_93C46_16BIT(config, m_eeprom); // Actually 93c45

	TIMER(config, m_raster_irq_timer).configure_generic(FUNC(deco_mlc_state::interrupt_gen));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58);
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(deco_mlc_state::screen_update));
	m_screen->screen_vblank().set(FUNC(deco_mlc_state::screen_vblank_mlc));
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_deco_mlc);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(16);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YMZ280B(config, m_ymz, 42000000 / 3);
	m_ymz->add_route(0, "speaker", 1.0, 0);
	m_ymz->add_route(1, "speaker", 1.0, 1);
}

void deco_mlc_state::mlc(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 42000000/6); // 42 MHz -> 7MHz clock confirmed on real board
	m_maincpu->set_addrmap(AS_PROGRAM, &deco_mlc_state::decomlc_no146_map);

	EEPROM_93C46_16BIT(config, m_eeprom); // Actually 93c45

	TIMER(config, m_raster_irq_timer).configure_generic(FUNC(deco_mlc_state::interrupt_gen));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58);
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(deco_mlc_state::screen_update));
	m_screen->screen_vblank().set(FUNC(deco_mlc_state::screen_vblank_mlc));
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_deco_mlc);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(16);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YMZ280B(config, m_ymz, 42000000 / 3);
	m_ymz->add_route(0, "speaker", 1.0, 0);
	m_ymz->add_route(1, "speaker", 1.0, 1);
}

void deco_mlc_state::mlc_6bpp(machine_config &config)
{
	mlc(config);

	m_gfxdecode->set_info(gfx_6bpp);
}

void deco_mlc_state::mlc_5bpp(machine_config &config)
{
	mlc(config);

	m_gfxdecode->set_info(gfx_5bpp);

	// TODO: mono? ch.0 doesn't output any sound in-game
	m_ymz->add_route(1, "speaker", 1.0, 0);
	m_ymz->add_route(0, "speaker", 1.0, 1);
}

void deco_mlc_state::stadhr96(machine_config &config)
{
	mlc_6bpp(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &deco_mlc_state::decomlc_146_map);

	DECO146PROT(config, m_deco146, 0);
	m_deco146->set_use_magic_read_address_xor(true);
}

void deco_mlc_state::acchi(machine_config &config)
{
	mlc(config);

	m_gfxdecode->set_info(gfx_acchi);
}

/***************************************************************************/

/*
Marvel Comics Avengers In Galactic Storm (Japan)
Data East, 1996

This game is special because it uses a standard Hitachi SH2 CPU instead of the
custom DE156 encrypted CPU.

Notes:
      - SH2 (QFP144) clock: 21.000MHz (42 / 2)
      - All ROMs SD* are 4M x 16bit EPROMS (27C4096)
      - All MCG* ROMs are surface mounted 16M mask ROMs
      - (mcg-01.1d read in 8 bit mode because this ROM had fixed bits when read in 16 bit
        mode, reading as 8 bit gave a good read. Others read in 16 bit mode)
*/

ROM_START( avengrgs ) // America/Europe Version 1.0, 1996.01.18
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "sf_00-0.7k", 0x000002, 0x80000, CRC(7d20e2df) SHA1(e8be1751029aea74680ac00cd7f3cf84e1adfc56) )
	ROM_LOAD32_WORD_SWAP( "sf_01-0.7l", 0x000000, 0x80000, CRC(f37c0a01) SHA1(8c4e28cde9e93457197b1849e6c9ef9516b5732f) )

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcg-00.1j", 0x000001, 0x200000, CRC(99129d9a) SHA1(1d1574e2326dca1043e05c229b54497df6ed5a35) )
	ROM_LOAD16_BYTE( "mcg-02.1f", 0x000000, 0x200000, CRC(29af9866) SHA1(56531911f8724975a7f81e61b7dec7fa72d50747) )
	ROM_LOAD16_BYTE( "mcg-01.1d", 0x400001, 0x200000, CRC(3638861b) SHA1(0896110acdb4442e4819f73285b9e725fc787b7a) )
	ROM_LOAD16_BYTE( "mcg-03.7m", 0x400000, 0x200000, CRC(4a0c965f) SHA1(b658ae5e6e2ff6f42b605bb6c49ad8a67507f2ab) )
	ROM_LOAD16_BYTE( "mcg-08.7p", 0x800001, 0x200000, CRC(c253943e) SHA1(b97a1d565ffbf2190ba0b25de5ef0bb3b9c9248b) )
	ROM_LOAD16_BYTE( "mcg-09.7n", 0x800000, 0x200000, CRC(8fb9870b) SHA1(046b6d07610cf09f008d1595605139071671d95c) )
	ROM_LOAD16_BYTE( "mcg-04.3j", 0xc00001, 0x200000, CRC(a4954c0e) SHA1(897a7313505f562879578941931a39afd34c9eef) )
	ROM_LOAD16_BYTE( "mcg-06.3f", 0xc00000, 0x200000, CRC(01571cf6) SHA1(74f85d523f2783374f041aa95abe6d1b8c872127) )
	ROM_LOAD16_BYTE( "mcg-05.3d", 0x1000001, 0x200000, CRC(182c2b49) SHA1(e53c06e95508e6c7e746f81668a4f7c08bfc6d36) )
	ROM_LOAD16_BYTE( "mcg-07.8m", 0x1000000, 0x200000, CRC(d09a3635) SHA1(8e184f3a3046bd8401762bbb480f5832fde91dde) )
	ROM_LOAD16_BYTE( "mcg-10.8p", 0x1400001, 0x200000, CRC(1383f524) SHA1(eadd8b579cc21ae119b7439c7882e39f22ac3b8c) )
	ROM_LOAD16_BYTE( "mcg-11.8n", 0x1400000, 0x200000, CRC(8f7fc281) SHA1(8cac51036088dbf4ff3c2b91ef88ef30a30b0be1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sf_02-0.6j", 0x000000, 0x80000, CRC(c98585dd) SHA1(752e246e2c72eb2b786c49d69f7ee4401a15c8aa) )

	ROM_REGION( 0x800000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "mcg-12.5a",  0x000000, 0x200000, CRC(bef9b28f) SHA1(b7a2a0539ea4d22b48ce3f3eb367017f219da2c1) ) // basic coin sounds etc.
	ROM_LOAD( "mcg-13.9k",  0x200000, 0x200000, CRC(92301551) SHA1(a7891e7a3c8d7f165ca73f5d5a034501df46e9a2) ) // music
	ROM_LOAD( "mcg-14.6a",  0x400000, 0x200000, CRC(c0d8b5f0) SHA1(08eecf6e7d0273e41cda3472709a67e2b16068c9) ) // music

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "avengrgs.nv",  0x00, 0x80, CRC(c0e84b4e) SHA1(e7afca68cc5fa69ded32bc0a1dcc6a59fe7f081b) )
ROM_END

ROM_START( avengrgsj ) // Japan Version 1.2, 1996.01.17
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "sd_00-2.7k", 0x000002, 0x80000, CRC(136be46a) SHA1(7679f5f78f7983d43ecdb9bdd04e45792a13d9f2) )
	ROM_LOAD32_WORD_SWAP( "sd_01-2.7l", 0x000000, 0x80000, CRC(9d87f576) SHA1(dd20cd060d020d81f4e012be10d0211be7526641) )

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcg-00.1j", 0x000001, 0x200000, CRC(99129d9a) SHA1(1d1574e2326dca1043e05c229b54497df6ed5a35) )
	ROM_LOAD16_BYTE( "mcg-02.1f", 0x000000, 0x200000, CRC(29af9866) SHA1(56531911f8724975a7f81e61b7dec7fa72d50747) )
	ROM_LOAD16_BYTE( "mcg-01.1d", 0x400001, 0x200000, CRC(3638861b) SHA1(0896110acdb4442e4819f73285b9e725fc787b7a) )
	ROM_LOAD16_BYTE( "mcg-03.7m", 0x400000, 0x200000, CRC(4a0c965f) SHA1(b658ae5e6e2ff6f42b605bb6c49ad8a67507f2ab) )
	ROM_LOAD16_BYTE( "mcg-08.7p", 0x800001, 0x200000, CRC(c253943e) SHA1(b97a1d565ffbf2190ba0b25de5ef0bb3b9c9248b) )
	ROM_LOAD16_BYTE( "mcg-09.7n", 0x800000, 0x200000, CRC(8fb9870b) SHA1(046b6d07610cf09f008d1595605139071671d95c) )
	ROM_LOAD16_BYTE( "mcg-04.3j", 0xc00001, 0x200000, CRC(a4954c0e) SHA1(897a7313505f562879578941931a39afd34c9eef) )
	ROM_LOAD16_BYTE( "mcg-06.3f", 0xc00000, 0x200000, CRC(01571cf6) SHA1(74f85d523f2783374f041aa95abe6d1b8c872127) )
	ROM_LOAD16_BYTE( "mcg-05.3d", 0x1000001, 0x200000, CRC(182c2b49) SHA1(e53c06e95508e6c7e746f81668a4f7c08bfc6d36) )
	ROM_LOAD16_BYTE( "mcg-07.8m", 0x1000000, 0x200000, CRC(d09a3635) SHA1(8e184f3a3046bd8401762bbb480f5832fde91dde) )
	ROM_LOAD16_BYTE( "mcg-10.8p", 0x1400001, 0x200000, CRC(1383f524) SHA1(eadd8b579cc21ae119b7439c7882e39f22ac3b8c) )
	ROM_LOAD16_BYTE( "mcg-11.8n", 0x1400000, 0x200000, CRC(8f7fc281) SHA1(8cac51036088dbf4ff3c2b91ef88ef30a30b0be1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sd_02-0.6j", 0x000000, 0x80000, CRC(24fc2b3c) SHA1(805eaa8e8ba49320ba83bda6307cc1d15d619358) )

	ROM_REGION( 0x800000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "mcg-12.5a",  0x000000, 0x200000, CRC(bef9b28f) SHA1(b7a2a0539ea4d22b48ce3f3eb367017f219da2c1) ) // basic coin sounds etc.
	ROM_LOAD( "mcg-13.9k",  0x200000, 0x200000, CRC(92301551) SHA1(a7891e7a3c8d7f165ca73f5d5a034501df46e9a2) ) // music
	ROM_LOAD( "mcg-14.6a",  0x400000, 0x200000, CRC(c0d8b5f0) SHA1(08eecf6e7d0273e41cda3472709a67e2b16068c9) ) // music

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "avengrgsj.nv",  0x00, 0x80, CRC(7ea70843) SHA1(f010b77b824e37c5e8c5722d5fff79751118f0b7) )
ROM_END


ROM_START( stadhr96 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sh-eaj.2a", 0x000000, 0x80000, CRC(10d1496a) SHA1(1dc151547463a38d717159b3dfce7ffd78a943ad) ) // FRI SEP 20 14:32:35 JST 1996
	ROM_LOAD32_WORD( "sh-eaj.2b", 0x000002, 0x80000, CRC(608a9144) SHA1(15e2fa99dc96e8ebd9868713ae7708cb824fc6c5) ) //    EUROPE (DISTRIBUTED)

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcm-00.2e", 0x0000001, 0x400000, CRC(c1919c3c) SHA1(168000ff1512a147d7029ee8878dd70de680fb08) )
	ROM_LOAD16_BYTE( "mcm-01.8m", 0x0000000, 0x400000, CRC(2255d47d) SHA1(ba3298e781fce1c84f68290bc464f2bc991382c0) )
	ROM_LOAD16_BYTE( "mcm-02.4e", 0x0800001, 0x400000, CRC(38c39822) SHA1(393d2c1c3c0bcb99df706d32ee3f8b681891dcac) )
	ROM_LOAD16_BYTE( "mcm-03.10m",0x0800000, 0x400000, CRC(4bd84ca7) SHA1(43dad8ced344f8d629d36f30ab2332879ba067d2) )
	ROM_LOAD16_BYTE( "mcm-04.6e", 0x1000001, 0x400000, CRC(7c0bd84c) SHA1(730b085a893d3c70592a8b4aecaeeaf4aceede56) )
	ROM_LOAD16_BYTE( "mcm-05.11m",0x1000000, 0x400000, CRC(476f03d7) SHA1(5c58ab4fc0e29f76619827bc27fa64cce2627e48) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sh-eaf.6h", 0x000000, 0x80000, CRC(f074a5c8) SHA1(72709cc0ac2d0df19393b405d8f927834f563e69) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "mcm-06.6a",  0x000000, 0x400000,  CRC(fbc178f3) SHA1(f44cb913177b6552b30c139505c3284bc445ba13) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "eeprom-stadhr96.bin",  0x00, 0x80, CRC(77861793) SHA1(df43b3ee55b7eb840cd6d3e5c5e04c68ce64bb57) )
ROM_END

ROM_START( stadhr96u )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "eah00-0.2a", 0x000000, 0x80000, CRC(f45b2ca0) SHA1(442dbfea97abb98451b323986878504ac0370e85) ) // FRI SEP 20 14:01:45 JST 1996
	ROM_LOAD32_WORD( "eah01-0.2b", 0x000002, 0x80000, CRC(328a2bca) SHA1(7e398b48719e5d71b2212d5b65be667e20663589) ) //            U.S.A.

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcm-00.2e", 0x0000001, 0x400000, CRC(c1919c3c) SHA1(168000ff1512a147d7029ee8878dd70de680fb08) )
	ROM_LOAD16_BYTE( "mcm-01.8m", 0x0000000, 0x400000, CRC(2255d47d) SHA1(ba3298e781fce1c84f68290bc464f2bc991382c0) )
	ROM_LOAD16_BYTE( "mcm-02.4e", 0x0800001, 0x400000, CRC(38c39822) SHA1(393d2c1c3c0bcb99df706d32ee3f8b681891dcac) )
	ROM_LOAD16_BYTE( "mcm-03.10m",0x0800000, 0x400000, CRC(4bd84ca7) SHA1(43dad8ced344f8d629d36f30ab2332879ba067d2) )
	ROM_LOAD16_BYTE( "mcm-04.6e", 0x1000001, 0x400000, CRC(7c0bd84c) SHA1(730b085a893d3c70592a8b4aecaeeaf4aceede56) )
	ROM_LOAD16_BYTE( "mcm-05.11m",0x1000000, 0x400000, CRC(476f03d7) SHA1(5c58ab4fc0e29f76619827bc27fa64cce2627e48) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "eaf02-0.6h", 0x000000, 0x80000, CRC(f95ad7ce) SHA1(878dcc1d5f76c8523c788e66bb4a8c5740d515e5) )

	ROM_REGION( 0x800000, "ymz", 0 )
	ROM_LOAD( "mcm-06.6a",  0x000000, 0x400000,  CRC(fbc178f3) SHA1(f44cb913177b6552b30c139505c3284bc445ba13) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "eeprom-stadhr96u.bin",  0x00, 0x80, CRC(71d796ba) SHA1(dc23117e24a8e79ca04f60a7cd23e22922ec9846) )
ROM_END

ROM_START( stadhr96j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "ead00-4.2a", 0x000000, 0x80000, CRC(b0adfc39) SHA1(3094dfb7c7f8fa9d7e10d98dff8fb8aba285d710) ) // WED SEP  5 00:00:00 JST 1996 (FINAL)
	ROM_LOAD32_WORD( "ead01-4.2b", 0x000002, 0x80000, CRC(0b332820) SHA1(28b757fe529250711fcb82424ba63c222a9329b9) ) //                JAPAN

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcm-00.2e", 0x0000001, 0x400000, CRC(c1919c3c) SHA1(168000ff1512a147d7029ee8878dd70de680fb08) )
	ROM_LOAD16_BYTE( "mcm-01.8m", 0x0000000, 0x400000, CRC(2255d47d) SHA1(ba3298e781fce1c84f68290bc464f2bc991382c0) )
	ROM_LOAD16_BYTE( "mcm-02.4e", 0x0800001, 0x400000, CRC(38c39822) SHA1(393d2c1c3c0bcb99df706d32ee3f8b681891dcac) )
	ROM_LOAD16_BYTE( "mcm-03.10m",0x0800000, 0x400000, CRC(4bd84ca7) SHA1(43dad8ced344f8d629d36f30ab2332879ba067d2) )
	ROM_LOAD16_BYTE( "mcm-04.6e", 0x1000001, 0x400000, CRC(7c0bd84c) SHA1(730b085a893d3c70592a8b4aecaeeaf4aceede56) )
	ROM_LOAD16_BYTE( "mcm-05.11m",0x1000000, 0x400000, CRC(476f03d7) SHA1(5c58ab4fc0e29f76619827bc27fa64cce2627e48) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "ead02-0.6h", 0x000000, 0x80000, CRC(f95ad7ce) SHA1(878dcc1d5f76c8523c788e66bb4a8c5740d515e5) )

	ROM_REGION( 0x800000, "ymz", 0 )
	ROM_LOAD( "mcm-06.6a",  0x000000, 0x400000,  CRC(fbc178f3) SHA1(f44cb913177b6552b30c139505c3284bc445ba13) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "eeprom-stadhr96j.bin",  0x00, 0x80, CRC(cf98098f) SHA1(54fc9bdd1ce9b836dad7b4a9909608e8f9842f71) )
ROM_END

// currently not working: 'This game board is not compatible with country code of mother board. Do not use, or damage will occur and void warranty of both boards.'
// according to a dumper: 'I ran into this issue a few years ago when I was "updating" some Stadium heros 96 boards to English. Some converted with an EPROM swap and other gave me the error.
// It wound up being a small serial EEPROM that was added to the PCB(it was socketed IIRC, on the bottom side of the PCB) and when I removed, it booted fine.
// The other SH 96's didn't have this part and was the only difference.

ROM_START( stadhr96j2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "eae00-5.2a", 0x000000, 0x80000, CRC(902b84e9) SHA1(62dfbb5b5e2bfea503fe0e3a96194dd34dab9829) ) /* WED SEP  4 00:00:00 JST 1996 (FINAL) */
	ROM_LOAD32_WORD( "eae01-5.2b", 0x000002, 0x80000, CRC(16245497) SHA1(776a9c20a9021987c618608d3b6ab569833bd439) ) /*                JAPAN                 */

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcm-00.2e", 0x0000001, 0x400000, CRC(c1919c3c) SHA1(168000ff1512a147d7029ee8878dd70de680fb08) )
	ROM_LOAD16_BYTE( "mcm-01.8m", 0x0000000, 0x400000, CRC(2255d47d) SHA1(ba3298e781fce1c84f68290bc464f2bc991382c0) )
	ROM_LOAD16_BYTE( "mcm-02.4e", 0x0800001, 0x400000, CRC(38c39822) SHA1(393d2c1c3c0bcb99df706d32ee3f8b681891dcac) )
	ROM_LOAD16_BYTE( "mcm-03.10m",0x0800000, 0x400000, CRC(4bd84ca7) SHA1(43dad8ced344f8d629d36f30ab2332879ba067d2) )
	ROM_LOAD16_BYTE( "mcm-04.6e", 0x1000001, 0x400000, CRC(7c0bd84c) SHA1(730b085a893d3c70592a8b4aecaeeaf4aceede56) )
	ROM_LOAD16_BYTE( "mcm-05.11m",0x1000000, 0x400000, CRC(476f03d7) SHA1(5c58ab4fc0e29f76619827bc27fa64cce2627e48) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "eae02-0.6h", 0x000000, 0x80000, CRC(57c30ca8) SHA1(e3bd2faf4637078c5594f62c7bc6db3067809cf9) )

	ROM_REGION( 0x800000, "ymz", 0 )
	ROM_LOAD( "mcm-06.6a",  0x000000, 0x400000,  CRC(fbc178f3) SHA1(f44cb913177b6552b30c139505c3284bc445ba13) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
ROM_END

ROM_START( stadhr96k )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sh_kr.2a",  0x0000000, 0x080000, CRC(5731a89c) SHA1(fc5c0740b47e4eee49f128dede5ab66ecf36f8ca) ) // No label on ROM
	ROM_LOAD32_WORD( "sh_kr.2b",  0x0000002, 0x080000, CRC(9f43d85a) SHA1(af1e28213e5c0ecde3f5e914be08666f01b5b149) ) // No label on ROM

	ROM_REGION( 0x1800000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mcm-00.2e", 0x0000001, 0x400000, CRC(c1919c3c) SHA1(168000ff1512a147d7029ee8878dd70de680fb08) )
	ROM_LOAD16_BYTE( "mcm-01.8m", 0x0000000, 0x400000, CRC(2255d47d) SHA1(ba3298e781fce1c84f68290bc464f2bc991382c0) )
	ROM_LOAD16_BYTE( "mcm-02.4e", 0x0800001, 0x400000, CRC(38c39822) SHA1(393d2c1c3c0bcb99df706d32ee3f8b681891dcac) )
	ROM_LOAD16_BYTE( "mcm-03.10m",0x0800000, 0x400000, CRC(4bd84ca7) SHA1(43dad8ced344f8d629d36f30ab2332879ba067d2) )
	ROM_LOAD16_BYTE( "mcm-04.6e", 0x1000001, 0x400000, CRC(7c0bd84c) SHA1(730b085a893d3c70592a8b4aecaeeaf4aceede56) )
	ROM_LOAD16_BYTE( "mcm-05.11m",0x1000000, 0x400000, CRC(476f03d7) SHA1(5c58ab4fc0e29f76619827bc27fa64cce2627e48) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "eae02-0.6h", 0x000000, 0x080000, CRC(57c30ca8) SHA1(e3bd2faf4637078c5594f62c7bc6db3067809cf9) )

	ROM_REGION( 0x800000, "ymz", 0 )
	ROM_LOAD( "mcm-06.6a",  0x000000, 0x400000, CRC(fbc178f3) SHA1(f44cb913177b6552b30c139505c3284bc445ba13) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
ROM_END


ROM_START( hoops96 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sz00-0.2a", 0x000000, 0x80000, CRC(971b4376) SHA1(e60d8d628bd1dc95d7f2b8840b0b188e68905c12) )
	ROM_LOAD32_WORD( "sz01-0.2b", 0x000002, 0x80000, CRC(b9679d7b) SHA1(3510b97390f2214cedb3387d32c7a7fd639a0a6e) )

	ROM_REGION( 0x0c00000, "gfx1",ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "mce-00.2e", 0x0000001, 0x200000, CRC(11b9bd96) SHA1(ed17fa9008b8e42951fd1f4c50939f1dd99cfeaf) )
	ROM_LOAD16_BYTE( "mce-01.8m", 0x0000000, 0x200000, CRC(6817d0c6) SHA1(ac1ee407b3981e0a9d45c429d301a93997f52c35) )
	ROM_LOAD16_BYTE( "mce-02.4e", 0x0400001, 0x200000, CRC(be7ff8ba) SHA1(40991d000dfbe7fc7f4f053e14c1b7b0b3cf2865) )
	ROM_LOAD16_BYTE( "mce-03.10m",0x0400000, 0x200000, CRC(756c282e) SHA1(5095bf8d8aae8133543bdc3f5b787efd403a5cf6) )
	ROM_LOAD32_WORD_SWAP( "mce-04.8n", 0x0800000, 0x200000, CRC(91da9b4f) SHA1(25c3a7abbaca006ad345150b5d689faf8b13affb) ) // extra plane of gfx, needs rearranging to decode

	ROM_REGION( 0x80000, "gfx2", ROMREGION_ERASEFF ) /* Code Lookup */
	ROM_LOAD( "rr02-0.6h", 0x020000, 0x20000, CRC(9490041c) SHA1(febedd0683dbcb080d304d03e4a3b501caeb6bb8) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "mce-05.6a",  0x000000, 0x400000,  CRC(e7a9355a) SHA1(039b23666e224c33ebb02baa80e496f8bce0514f) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "hoops.nv",  0x00, 0x80, CRC(67b18457) SHA1(5d6a0034bfc3d395ecd941ed024c8884b43f2a31) )
ROM_END

ROM_START( hoops95 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "hoops.a2", 0x000000, 0x80000, CRC(02b8c61a) SHA1(ae49f6bd8a3bffa181fa6a2740d92287e9d5dc02) )
	ROM_LOAD32_WORD( "hoops.b2", 0x000002, 0x80000, CRC(a1dc3519) SHA1(73a292c34f4172cf12827a21b100cc6653650a5b) )

	ROM_REGION( 0x0c00000, "gfx1",ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "mce-00.2e", 0x0000001, 0x200000, CRC(11b9bd96) SHA1(ed17fa9008b8e42951fd1f4c50939f1dd99cfeaf) )
	ROM_LOAD16_BYTE( "mce-01.8m", 0x0000000, 0x200000, CRC(6817d0c6) SHA1(ac1ee407b3981e0a9d45c429d301a93997f52c35) )
	ROM_LOAD16_BYTE( "mce-02.4e", 0x0400001, 0x200000, CRC(be7ff8ba) SHA1(40991d000dfbe7fc7f4f053e14c1b7b0b3cf2865) )
	ROM_LOAD16_BYTE( "mce-03.10m",0x0400000, 0x200000, CRC(756c282e) SHA1(5095bf8d8aae8133543bdc3f5b787efd403a5cf6) )
	ROM_LOAD32_WORD_SWAP( "mce-04.8n", 0x0800000, 0x200000, CRC(91da9b4f) SHA1(25c3a7abbaca006ad345150b5d689faf8b13affb) ) // extra plane of gfx, needs rearranging to decode

	ROM_REGION( 0x80000, "gfx2", ROMREGION_ERASEFF ) /* Code Lookup */
	ROM_LOAD( "rl02-0.6h", 0x020000, 0x20000, CRC(9490041c) SHA1(febedd0683dbcb080d304d03e4a3b501caeb6bb8) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "mce-05.6a",  0x000000, 0x400000,  CRC(e7a9355a) SHA1(039b23666e224c33ebb02baa80e496f8bce0514f) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "hoops.nv",  0x00, 0x80, CRC(67b18457) SHA1(5d6a0034bfc3d395ecd941ed024c8884b43f2a31) )
ROM_END

ROM_START( ddream95 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "rl00-2.2a", 0x000000, 0x80000, CRC(07645092) SHA1(5f24bd6102b7e6212888b703f86bed5a19e08e85) )
	ROM_LOAD32_WORD( "rl01-2.2b", 0x000002, 0x80000, CRC(cfc629fc) SHA1(c0bcfa75c6446def4af99b14a1a869b5576c244f) )

	ROM_REGION( 0x0c00000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mce-00.2e", 0x0000001, 0x200000, CRC(11b9bd96) SHA1(ed17fa9008b8e42951fd1f4c50939f1dd99cfeaf) )
	ROM_LOAD16_BYTE( "mce-01.8m", 0x0000000, 0x200000, CRC(6817d0c6) SHA1(ac1ee407b3981e0a9d45c429d301a93997f52c35) )
	ROM_LOAD16_BYTE( "mce-02.4e", 0x0400001, 0x200000, CRC(be7ff8ba) SHA1(40991d000dfbe7fc7f4f053e14c1b7b0b3cf2865) )
	ROM_LOAD16_BYTE( "mce-03.10m",0x0400000, 0x200000, CRC(756c282e) SHA1(5095bf8d8aae8133543bdc3f5b787efd403a5cf6) )
	ROM_LOAD32_WORD_SWAP( "mce-04.8n", 0x0800000, 0x200000, CRC(91da9b4f) SHA1(25c3a7abbaca006ad345150b5d689faf8b13affb) ) // extra plane of gfx, needs rearranging to decode

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rl02-0.6h", 0x020000, 0x20000, CRC(9490041c) SHA1(febedd0683dbcb080d304d03e4a3b501caeb6bb8) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "mce-05.6a",  0x000000, 0x400000,  CRC(e7a9355a) SHA1(039b23666e224c33ebb02baa80e496f8bce0514f) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "hoops.nv",  0x00, 0x80, CRC(67b18457) SHA1(5d6a0034bfc3d395ecd941ed024c8884b43f2a31) )
ROM_END

ROM_START( skullfng )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sw00-0.2a", 0x000000, 0x80000, CRC(9658d9ce) SHA1(bd5b58a35e4fe301dc13bfe962e674fc8b26cf60) )
	ROM_LOAD32_WORD( "sw01-0.2b", 0x000002, 0x80000, CRC(c0d83d14) SHA1(42a5e2fa0e26919b94566da3dec622cd25dd9558) )

	ROM_REGION( 0xc00000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mch-00.2e", 0x000001, 0x200000, CRC(d5cc4238) SHA1(f1bd86386e44a3f600475aeab310f7ea632998df) )
	ROM_LOAD16_BYTE( "mch-01.8m", 0x000000, 0x200000, CRC(d37cf0cd) SHA1(c2fe7062a123ca2df65217c6dced857b803d8a8d) )
	ROM_LOAD16_BYTE( "mch-02.4e", 0x400001, 0x200000, CRC(4046314d) SHA1(32e3b7ddbe20ffa6ba6ebe9bd55a32e3b3a120f6) )
	ROM_LOAD16_BYTE( "mch-03.10m",0x400000, 0x200000, CRC(1dea8f6c) SHA1(c2ad59592385a00e323aac9057906c9384b67078) )
	ROM_LOAD16_BYTE( "mch-04.6e", 0x800001, 0x200000, CRC(4869dfe8) SHA1(296df6274ecb3eed485de24258cf462e3942f1fa) )
	ROM_LOAD16_BYTE( "mch-05.11m",0x800000, 0x200000, CRC(ef0b54ba) SHA1(3be56c064ac81686096be5f31ad2aad948ba6701) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sw02-0.6h", 0x000000, 0x80000, CRC(0d3ae757) SHA1(480fc3855d330380b75a47a271f3571a59aee10c) )

	ROM_REGION( 0x800000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "mch-06.6a",  0x200000, 0x200000, CRC(b2efe4ae) SHA1(5a9dab74c2ba73a65e8f1419b897467804734fa2) )
	ROM_LOAD( "mch-07.11j", 0x400000, 0x200000, CRC(bc1a50a1) SHA1(3de191fbc92d2ae84e54263f1c70afec6ff7cc3c) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "skullfng.eeprom",  0x00, 0x80, CRC(240d882e) SHA1(3c1a15ccac91d95b02a8c54b051aa64ff28ce2ab) )
ROM_END

ROM_START( skullfngj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sh00-0.2a", 0x000000, 0x80000, CRC(e50358e8) SHA1(e66ac5e1b16273cb905254c99b2bce435145a414) )
	ROM_LOAD32_WORD( "sh01-0.2b", 0x000002, 0x80000, CRC(2c288bcc) SHA1(4ed1d5818362383240378056bf575f6acf8a593a) )

	ROM_REGION( 0xc00000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mch-00.2e", 0x000001, 0x200000, CRC(d5cc4238) SHA1(f1bd86386e44a3f600475aeab310f7ea632998df) )
	ROM_LOAD16_BYTE( "mch-01.8m", 0x000000, 0x200000, CRC(d37cf0cd) SHA1(c2fe7062a123ca2df65217c6dced857b803d8a8d) )
	ROM_LOAD16_BYTE( "mch-02.4e", 0x400001, 0x200000, CRC(4046314d) SHA1(32e3b7ddbe20ffa6ba6ebe9bd55a32e3b3a120f6) )
	ROM_LOAD16_BYTE( "mch-03.10m",0x400000, 0x200000, CRC(1dea8f6c) SHA1(c2ad59592385a00e323aac9057906c9384b67078) )
	ROM_LOAD16_BYTE( "mch-04.6e", 0x800001, 0x200000, CRC(4869dfe8) SHA1(296df6274ecb3eed485de24258cf462e3942f1fa) )
	ROM_LOAD16_BYTE( "mch-05.11m",0x800000, 0x200000, CRC(ef0b54ba) SHA1(3be56c064ac81686096be5f31ad2aad948ba6701) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sh02-0.6h", 0x000000, 0x80000, CRC(0d3ae757) SHA1(480fc3855d330380b75a47a271f3571a59aee10c) )

	ROM_REGION( 0x800000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "mch-06.6a",  0x200000, 0x200000, CRC(b2efe4ae) SHA1(5a9dab74c2ba73a65e8f1419b897467804734fa2) )
	ROM_LOAD( "mch-07.11j", 0x400000, 0x200000, CRC(bc1a50a1) SHA1(3de191fbc92d2ae84e54263f1c70afec6ff7cc3c) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "skullfng.eeprom",  0x00, 0x80, CRC(240d882e) SHA1(3c1a15ccac91d95b02a8c54b051aa64ff28ce2ab) )
ROM_END

ROM_START( skullfnga )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sx00-0.2a", 0x000000, 0x80000, CRC(749c0972) SHA1(8a5f170f2f8d800ad2d47ecf2e5b8978a1f14f50) )
	ROM_LOAD32_WORD( "sx01-0.2b", 0x000002, 0x80000, CRC(61ae7dc3) SHA1(185448fb3d9fe887db0202b195c97674e3dd5f65) )

	ROM_REGION( 0xc00000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mch-00.2e", 0x000001, 0x200000, CRC(d5cc4238) SHA1(f1bd86386e44a3f600475aeab310f7ea632998df) )
	ROM_LOAD16_BYTE( "mch-01.8m", 0x000000, 0x200000, CRC(d37cf0cd) SHA1(c2fe7062a123ca2df65217c6dced857b803d8a8d) )
	ROM_LOAD16_BYTE( "mch-02.4e", 0x400001, 0x200000, CRC(4046314d) SHA1(32e3b7ddbe20ffa6ba6ebe9bd55a32e3b3a120f6) )
	ROM_LOAD16_BYTE( "mch-03.10m",0x400000, 0x200000, CRC(1dea8f6c) SHA1(c2ad59592385a00e323aac9057906c9384b67078) )
	ROM_LOAD16_BYTE( "mch-04.6e", 0x800001, 0x200000, CRC(4869dfe8) SHA1(296df6274ecb3eed485de24258cf462e3942f1fa) )
	ROM_LOAD16_BYTE( "mch-05.11m",0x800000, 0x200000, CRC(ef0b54ba) SHA1(3be56c064ac81686096be5f31ad2aad948ba6701) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "sx02-0.6h", 0x000000, 0x80000, CRC(0d3ae757) SHA1(480fc3855d330380b75a47a271f3571a59aee10c) )

	ROM_REGION( 0x800000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "mch-06.6a",  0x200000, 0x200000, CRC(b2efe4ae) SHA1(5a9dab74c2ba73a65e8f1419b897467804734fa2) )
	ROM_LOAD( "mch-07.11j", 0x400000, 0x200000, CRC(bc1a50a1) SHA1(3de191fbc92d2ae84e54263f1c70afec6ff7cc3c) )

	ROM_REGION16_BE( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "skullfng.eeprom",  0x00, 0x80, CRC(240d882e) SHA1(3c1a15ccac91d95b02a8c54b051aa64ff28ce2ab) )
ROM_END

ROM_START( acchi ) // DE-0444-1 + DE-0457-0
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD( "eaa-00-3.l10", 0x000000, 0x80000, CRC(faff1710) SHA1(b28d210b16b7a0a818ecfeb59696b093be27dc2b) )
	ROM_LOAD32_WORD( "eaa-01-3.l11", 0x000002, 0x80000, CRC(00510371) SHA1(1beef396065c10e5f16b80a92d960abcd4838363) )

	ROM_REGION( 0x1400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "faa-09-0.c2", 0x0000000, 0x200000, CRC(b616058f) SHA1(9dd8f259f08b76e177fc2a266159e4a716b5f4c6) )
	ROM_LOAD16_BYTE( "faa-08-0.c1", 0x0000001, 0x200000, CRC(70ba2149) SHA1(fbcf7c65021e55ed74b0333852860bfb56f05cab) )
	ROM_LOAD16_BYTE( "faa-07-0.h2", 0x0400000, 0x200000, CRC(60028a62) SHA1(883935cb5421b344c9a83665a6bbdcff4986f1e7) )
	ROM_LOAD16_BYTE( "faa-05-0.h1", 0x0400001, 0x200000, CRC(1e7a7a0a) SHA1(09333289177aa102ca082f55d27c453037769b4c) )
	ROM_LOAD16_BYTE( "faa-06-0.f2", 0x0800000, 0x200000, CRC(5538ff81) SHA1(f68369a0ee25fe5ac3ce5f36c56ca72e46a08279) )
	ROM_LOAD16_BYTE( "faa-04-0.f1", 0x0800001, 0x200000, CRC(35280765) SHA1(9dcee2724f16fa5c371bf89a4257bbb7eb25f733) )
	ROM_LOAD16_BYTE( "faa-03-0.e2", 0x0c00000, 0x200000, CRC(af457714) SHA1(dcb94ac0cc632a9d84bb896588f33b47bf87a695) )
	ROM_LOAD16_BYTE( "faa-01-0.e1", 0x0c00001, 0x200000, CRC(a245270b) SHA1(a2279af96878fb099f8c1e67c2ecf3d0254b36a3) )
	ROM_LOAD16_BYTE( "faa-02-0.b2", 0x1000000, 0x200000, CRC(bdf75bd6) SHA1(f271192a851e3750beeea6033dc43df614967de1) )
	ROM_LOAD16_BYTE( "faa-00-0.b1", 0x1000001, 0x200000, CRC(2cf34cd6) SHA1(4d96ca597ad84bedabf53dc17a976f0d95ab99b4) )
	// there are two unpopulated IC spaces at d1 and d2

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "eaa-02-0.l12", 0x000000, 0x80000, CRC(09baf624) SHA1(548269cead3204c6d269955f5b91937a231bd6af) )

	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 )
	ROM_LOAD( "faa-10-0.a5",  0x000000, 0x200000, CRC(9f280e5f) SHA1(47c3532b4142ff677f4f0b763ba142883ba3d545) )
	ROM_LOAD( "faa-11-0.a7",  0x200000, 0x200000, CRC(3775ae66) SHA1(28208d6fee4bf638545dc6b002c900dbb7ab85b0) )
ROM_END

/***************************************************************************/

void deco_mlc_state::descramble_sound(  )
{
	// the same as simpl156 / heavy smash?
	u8 *rom = memregion("ymz")->base();
	int length = memregion("ymz")->bytes();
	std::vector<u8> buf(length);

	for (u32 x = 0; x < length; x++)
	{
		const u32 addr = bitswap<24>(x,
				23,22,21, 0,20,
				19,18,17,16,
				15,14,13,12,
				11,10, 9, 8,
				 7, 6, 5, 4,
				 3, 2, 1);

		buf[addr] = rom[x];
	}

	std::copy(buf.begin(), buf.end(), &rom[0]);
}

u32 deco_mlc_state::avengrgs_speedup_r()
{
	const u32 a = m_mainram[0x89a0 / 4];
	const u32 p = m_maincpu->pc();

	if ((p == 0x3234 || p == 0x32dc) && (a & 1)) m_maincpu->spin_until_interrupt();

	return a;
}

void deco_mlc_state::init_avengrgs()
{
	// init options
	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);

	// set up speed cheat
	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_add_pcflush(0x3234);
	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_add_pcflush(0x32dc);

	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_add_fastram(0x0100000, 0x01088ff, 0, &m_mainram[0]);
	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_add_fastram(0x0108a00, 0x011ffff, 0, &m_mainram[0x8a00 / 4]);
	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_add_fastram(0x0200080, 0x02000ff, 0, &m_clip_ram[0]);
	dynamic_cast<sh7604_device *>(m_maincpu.target())->sh2drc_add_fastram(0x0280000, 0x029ffff, 0, &m_vram[0]);

	m_irqLevel = 1;
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01089a0, 0x01089a3, read32smo_delegate(*this, FUNC(deco_mlc_state::avengrgs_speedup_r)));
	descramble_sound();
}

void deco_mlc_state::init_mlc()
{
	/* The timing in the ARM core isn't as accurate as it should be, so bump up the
	    effective clock rate here to compensate otherwise we have slowdowns in
	    Skull Fang where there probably shouldn't be. */
	m_maincpu->set_clock_scale(2.0);
	m_irqLevel = ARM_IRQ_LINE;
	deco156_decrypt(machine());
	descramble_sound();
}

void deco_mlc_state::init_acchi() // sound ROMs don't appear to be scrambled
{
	m_maincpu->set_clock_scale(2.0); // avoids hangs in attract mode / end of round, see init_mlc()
	m_irqLevel = ARM_IRQ_LINE;
	deco156_decrypt(machine());
}


/***************************************************************************/

GAME( 1995, avengrgs,   0,        avengrgs, mlc, deco_mlc_state, init_avengrgs, ROT0,   "Data East Corporation", "Avengers In Galactic Storm (US/Europe 1.0)",     MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, avengrgsj,  avengrgs, avengrgs, mlc, deco_mlc_state, init_avengrgs, ROT0,   "Data East Corporation", "Avengers In Galactic Storm (Japan 1.2)",         MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, stadhr96,   0,        stadhr96, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Stadium Hero '96 (Europe, EAJ, Tuning license)", MACHINE_IMPERFECT_GRAPHICS ) // Rom labels are EAJ  ^^
GAME( 1996, stadhr96u,  stadhr96, stadhr96, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Stadium Hero '96 (USA, EAH)",                    MACHINE_IMPERFECT_GRAPHICS ) // Rom labels are EAH  ^^
GAME( 1996, stadhr96j,  stadhr96, stadhr96, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Stadium Hero '96 (Japan, EAD)",                  MACHINE_IMPERFECT_GRAPHICS ) // Rom labels are EAD (this isn't a Konami region code!)
GAME( 1996, stadhr96j2, stadhr96, stadhr96, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Stadium Hero '96 (Japan?, EAE)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rom labels are EAE ^^
GAME( 1996, stadhr96k,  stadhr96, stadhr96, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Stadium Hero '96 (Korea, Dream Island license)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, skullfng,   0,        mlc_6bpp, mlc, deco_mlc_state, init_mlc,      ROT270, "Data East Corporation", "Skull Fang (Europe 1.13)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Version 1.13, Europe, Master 96.02.19 13:45
GAME( 1996, skullfngj,  skullfng, mlc_6bpp, mlc, deco_mlc_state, init_mlc,      ROT270, "Data East Corporation", "Skull Fang - Kuhga Gaiden (Japan 1.09)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Version 1.09, Japan, Master 96.02.08 14:39
GAME( 1996, skullfnga,  skullfng, mlc_6bpp, mlc, deco_mlc_state, init_mlc,      ROT270, "Data East Corporation", "Skull Fang - Kuhga Gaiden (Asia 1.13)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Version 1.13, Asia, Master 96.02.19 13:49
GAME( 1996, hoops96,    0,        mlc_5bpp, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Hoops '96 (Europe/Asia 2.0)",                    MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, ddream95,   hoops96,  mlc_5bpp, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Dunk Dream '95 (Japan 1.4, EAM)",                MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, hoops95,    hoops96,  mlc_5bpp, mlc, deco_mlc_state, init_mlc,      ROT0,   "Data East Corporation", "Hoops (Europe/Asia 1.7)",                        MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, acchi,      0,        acchi,    mlc, deco_mlc_state, init_acchi,    ROT0,   "Data East Corporation", "Janken Game Acchi Muite Hoi! (Japan 1.3)",       MACHINE_IMPERFECT_GRAPHICS )
