/* Sega MegaTech

About MegaTech:

Megatech games are identical to their Genesis/SMS equivlents, however the Megatech cartridges contain
a BIOS rom with the game instructions.  The last part number of the bios ROM is the cart/game ID code.

The instruction rom appears to map at 0x300000 in the cart space.

In Megatech games your coins buy you time to play the game, how you perform in the game does not
matter, you can die and start a new game providing you still have time, likewise you can be playing
well and run out of time if you fail to insert more coins.  This is the same method Nintendo used
with their Playchoice 10 system.

The BIOS screen is based around SMS hardware, with an additional Z80 and SMS VDP chip not present on
a standard Genesis.

SMS games run on Megatech in the Genesis's SMS compatability mode, where the Genesis Z80 becomes the
main CPU and the Genesis VDP acts in a mode mimicing the behavior of the SMS VDP. A pin on the carts
determines which mode the game runs in.

Additions will only be made to this driver if proof that the dumped set are original roms with original
Sega part numbers is given..


Sega Mega Tech Cartridges (Readme by Guru)
-------------------------

These are cart-based games for use with Sega Mega Tech hardware. There are 6 known types of carts. All carts
are very simple, almost exactly the same as Mega Play carts. They contain just 2 or 3 ROMs.
PCB 171-6215A has locations for 2 ROMs and is dated 1991. PCB 171-6215A is also used in Mega Play!
PCB 171-5782 has locations for 2 ROMs and is dated 1989.
PCB 171-5869A has locations for 3 ROMs and is dated 1989.
PCB 171-5834 has locations for 3 ROMs and is dated 1989.
PCB 171-5783 has locations for 2 ROMs and is dated 1989.
PCB 171-5784 has locations for 2 ROMs and is dated 1989. It also contains a custom Sega IC 315-5235

                                                                           |------------------------------- ROMs --------------------------------|
                                                                           |                                                                     |
Game                       PCB #       Sticker on PCB    Sticker on cart     IC1                          IC2                      IC3
-------------------------------------------------------------------------------------------------------------------------------------------------
Altered Beast              171-5782    837-6963-01       610-0239-01         MPR-12538F     (834200A)     EPR-12368-01   (27C256)  n/a
Space Harrier II           171-5782    837-6963-02       610-0239-02         MPR-11934      (834200)      EPR-12368-02   (27256)   n/a
Out Run                    171-5783    837-6963-06       610-0239-06         MPR-11078      (Mask)        EPR-12368-06   (27256)   n/a
Alien Syndrome             171-5783    837-6963-07       610-0239-07         MPR-11194      (232011)      EPR-12368-07   (27256)   n/a
Afterburner                171-5784    837-6963-10       610-0239-10         315-5235       (custom)      MPR-11271-T    (834000)  EPR-12368-10 (27256)
Great Football             171-5783    837-6963-19       610-0239-19         MPR-10576F     (831000)      EPR-12368-19   (27256)   n/a
World Championship Soccer  171-5782    837-6963-21       610-0239-21         MPR-12607B     (uPD23C4000)  EPR-12368-21   (27256)   n/a
Tetris                     171-5834    837-6963-22       610-0239-22         MPR-12356F     (831000)      MPR-12357F     (831000)  EPR-12368-22 (27256)
Ghouls & Ghosts            171-5869A   -                 610-0239-23         MPR-12605      (40 pins)     MPR-12606      (40 pins) EPR-12368-23 (27256)
Super Hang On              171-5782    837-6963-24       610-0239-24         MPR-12640      (234000)      EPR-12368-24   (27256)   n/a
Forgotten Worlds           171-5782    837-6963-26       610-0239-26         MPR-12672-H    (Mask)        EPR-12368-26   (27256)   n/a
The Revenge Of Shinobi     171-5782    837-6963-28       610-0239-28         MPR-12675 S44  (uPD23C4000)  EPR-12368-28   (27C256)  n/a
Arnold Palmer Tour Golf    171-5782    837-6963-31       610-0239-31         MPR-12645F     (23C4000)     EPR-12368-31   (27256)   n/a
Super Real Basket Ball     171-5782    837-6963-32       610-0239-32         MPR-12904F     (838200A)     EPR-12368-32   (27256)   n/a
Tommy Lasorda Baseball     171-5782    837-6963-35       610-0239-35         MPR-12706F     (834200A)     EPR-12368-35   (27256)   n/a
ESWAT                      171-5782    837-6963-38       610-0239-38         MPR-13192-H    (uPD23C4000)  EPR-12368-38   (27256)   n/a
Moonwalker                 171-5782    837-6963-40       610-0239-40         MPR-13285A S61 (uPD23C4000)  EPR-12368-40   (27256)   n/a
Shadow Dancer              171-5782    837-6963-43       610-0239-43         MPR-13571-S    (uPD23C4000)  EPR-12368-43   (27256)   n/a
Wrestle War                171-5782    837-6963-48       610-0239-48         MPR-14025-F    (23C4000)     EPR-12368-48   (27256)   n/a
Bonanza Bros.              171-5782    837-6963-49       610-0239-49         MPR-13905A-F   (23C4000)     EPR-12368-49   (27256)   n/a
Streets of Rage            171-5782    837-6963-51       610-0239-51         MPR-14125-SM   (uPD23C4000)  EPR-12368-51   (27C256)  n/a
Sonic The Hedgehog         171-5782    837-6963-52       610-0239-52         MPR-13913-F    (834200A)     EPR-12368-52   (27C256)  n/a
Spider-Man                 171-5782    837-6963-54       610-0239-54         MPR-14027-SM   (uPD23C4000)  EPR-12368-54   (27C256)  n/a
California Games           171-5834    837-6963-55-01    610-0239-55         EPR-14494      (27C020)      EPR-14495      (27C020)  EPR-12368-55 (27C256)
Mario Lemeux Hockey        171-5782    837-6963-59       610-0239-59         MPR-14376-H    (234000)      EPR-12368-59   (27256)   n/a
Turbo Outrun               171-5782    837-6963-61       610-0239-61         MPR-14674      (uPD23C4000)  EPR-12368-61   (27256)   n/a
Sonic Hedgehog 2           171-6215A   837-6963-62       610-0239-62         MPR-15000A-F   (838200)      EPR-12368-62   (27256)   n/a

*/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "rendlay.h"

#include "includes/segamsys.h"
#include "includes/megadriv.h"
#include "imagedev/cartslot.h"

#define MASTER_CLOCK		53693100

/* not currently used */
static INPUT_PORTS_START( megatech ) /* Genesis Input Ports */
	PORT_INCLUDE(megadriv)

	PORT_START("BIOS_IN0") // port 6
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Select") PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("BIOS_IN1") // port 6
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_COIN1 )  // a few coin inputs here
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service coin") PORT_CODE(KEYCODE_9)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Enter") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("BIOS_DSW0")
	PORT_DIPNAME( 0x02, 0x02, "Coin slot 3" )
	PORT_DIPSETTING(    0x00, "Inhibit" )
	PORT_DIPSETTING(    0x02, "Accept" )
	PORT_DIPNAME( 0x01, 0x01, "Coin slot 4" )
	PORT_DIPSETTING(    0x00, "Inhibit" )
	PORT_DIPSETTING(    0x01, "Accept" )
	PORT_DIPNAME( 0x1c, 0x1c, "Coin slot 3/4 value" )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 credits" )
	PORT_DIPNAME( 0xe0, 0x60, "Coin slot 2 value" )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Inhibit" )

	PORT_START("BIOS_DSW1")
	PORT_DIPNAME( 0x0f, 0x01, "Coin Slot 1 value" )
	PORT_DIPSETTING(    0x00, "Inhibit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x0a, "1 coin/10 credits" )
	PORT_DIPSETTING(    0x0b, "1 coin/11 credits" )
	PORT_DIPSETTING(    0x0c, "1 coin/12 credits" )
	PORT_DIPSETTING(    0x0d, "1 coin/13 credits" )
	PORT_DIPSETTING(    0x0e, "1 coin/14 credits" )
	PORT_DIPSETTING(    0x0f, "1 coin/15 credits" )
	PORT_DIPNAME( 0xf0, 0xa0, "Time per credit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, "7:30" )
	PORT_DIPSETTING(    0x20, "7:00" )
	PORT_DIPSETTING(    0x30, "6:30" )
	PORT_DIPSETTING(    0x40, "6:00" )
	PORT_DIPSETTING(    0x50, "5:30" )
	PORT_DIPSETTING(    0x60, "5:00" )
	PORT_DIPSETTING(    0x70, "4:30" )
	PORT_DIPSETTING(    0x80, "4:00" )
	PORT_DIPSETTING(    0x90, "3:30" )
	PORT_DIPSETTING(    0xa0, "3:00" )
	PORT_DIPSETTING(    0xb0, "2:30" )
	PORT_DIPSETTING(    0xc0, "2:00" )
	PORT_DIPSETTING(    0xd0, "1:30" )
	PORT_DIPSETTING(    0xe0, "1:00" )
	PORT_DIPSETTING(    0xf0, "0:30" )


	PORT_START("BIOS_J1")
	PORT_DIPNAME( 0x0001, 0x0001, "5" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END

/* MEGATECH specific */
static READ8_HANDLER( megatech_cart_select_r )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	return state->m_mt_cart_select_reg;
}



static TIMER_CALLBACK( megatech_z80_run_state )
{
	mtech_state *state = machine.driver_data<mtech_state>();
	char tempname[20];
	UINT8* game_region;

	sprintf(tempname, "game%d", param);
	game_region = machine.region(tempname)->base();

	memcpy(machine.region("maincpu")->base(), game_region, 0x400000);

	if (!state->m_cart_is_genesis[param])
	{
		printf("enabling SMS Z80\n");
		state->m_current_game_is_sms = 1;
		megatech_set_genz80_as_sms_standard_map(machine, "genesis_snd_z80", MAPPER_STANDARD);
		//cputag_set_input_line(machine, "genesis_snd_z80", INPUT_LINE_HALT, CLEAR_LINE);
		cputag_set_input_line(machine, "genesis_snd_z80", INPUT_LINE_RESET, CLEAR_LINE);
	}
	else
	{
		printf("disabling SMS Z80\n");
		state->m_current_game_is_sms = 0;
		megatech_set_megadrive_z80_as_megadrive_z80(machine, "genesis_snd_z80");
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_RESET, CLEAR_LINE);
		//cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, CLEAR_LINE);
	}
}

static TIMER_CALLBACK( megatech_z80_stop_state )
{
	UINT8* game_region;
	char tempname[20];

	printf("megatech_select_game %d\n", param+1);

	sprintf(tempname, "game%d", param);
	game_region = machine.region(tempname)->base();

	cputag_set_input_line(machine, "maincpu", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "genesis_snd_z80", INPUT_LINE_RESET, ASSERT_LINE);
	//cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
	//cputag_set_input_line(machine, "genesis_snd_z80", INPUT_LINE_HALT, ASSERT_LINE);
	devtag_reset(machine, "ymsnd");

	megadriv_stop_scanline_timer();// stop the scanline timer for the genesis vdp... it can be restarted in video eof when needed
	segae_md_sms_stop_scanline_timer();// stop the scanline timer for the sms vdp


	/* if the regions exist we're fine */
	if (game_region)
	{
		{
			machine.scheduler().timer_set(attotime::zero, FUNC(megatech_z80_run_state), param);
		}
	}
	else
	{
		/* no cart.. */
		memset(machine.region("mtbios")->base() + 0x8000, 0x00, 0x8000);
		memset(machine.region("maincpu")->base(), 0x00, 0x400000);
	}

	return;
}

static void megatech_select_game(running_machine &machine, int gameno)
{
	machine.scheduler().timer_set(attotime::zero, FUNC(megatech_z80_stop_state), gameno);
}

static WRITE8_HANDLER( megatech_cart_select_w )
{
	/* seems to write the slot number..
      but it stores something in (banked?) ram
      because it always seems to show the
      same instructions ... */
	mtech_state *state = space->machine().driver_data<mtech_state>();
	state->m_mt_cart_select_reg = data;

	megatech_select_game(space->machine(), state->m_mt_cart_select_reg);
}


static READ8_HANDLER( bios_ctrl_r )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();

	if (offset == 0)
		return 0;
	if (offset == 2)
		return state->m_bios_ctrl[offset] & 0xfe;

	return state->m_bios_ctrl[offset];
}

static WRITE8_HANDLER( bios_ctrl_w )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();

	if (offset == 1)
	{
		output_set_value("Alarm_sound", data>>7 & 0x01);
		state->m_bios_ctrl_inputs = data & 0x04;  // Genesis/SMS input ports disable bit
	}
	else if (offset == 2)
	{
		output_set_value("Flash_screen", data>>1 & 0x01);
	}

	state->m_bios_ctrl[offset] = data;
}

/* this sets 0x300000 which may indicate that the 68k can see the instruction rom
   there, this limiting the max game rom capacity to 3meg. */

static READ8_HANDLER( megatech_z80_read_68k_banked_data )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	address_space *space68k = space->machine().device<legacy_cpu_device>("maincpu")->space();
	UINT8 ret = space68k->read_byte(state->m_mt_bank_addr + offset);
	return ret;
}

static WRITE8_HANDLER( megatech_z80_write_68k_banked_data )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	address_space *space68k = space->machine().device<legacy_cpu_device>("maincpu")->space();
	space68k->write_byte(state->m_mt_bank_addr + offset,data);
}

static void megatech_z80_bank_w(running_machine &machine, UINT16 data)
{
	mtech_state *state = machine.driver_data<mtech_state>();
	state->m_mt_bank_addr = ((state->m_mt_bank_addr >> 1) | (data << 23)) & 0xff8000;
}

static WRITE8_HANDLER( mt_z80_bank_w )
{
	megatech_z80_bank_w(space->machine(), data & 1);
}

static READ8_HANDLER( megatech_banked_ram_r )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	return state->m_megatech_banked_ram[offset + 0x1000 * (state->m_mt_cart_select_reg & 0x07)];
}

static WRITE8_HANDLER( megatech_banked_ram_w )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	state->m_megatech_banked_ram[offset + 0x1000 * (state->m_mt_cart_select_reg & 0x07)] = data;
}



static ADDRESS_MAP_START( megatech_bios_map, AS_PROGRAM, 8, mtech_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM // from bios rom (0x0000-0x2fff populated in ROM)
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE_LEGACY(megatech_banked_ram_r, megatech_banked_ram_w) // copies instruction data here at startup, must be banked
	AM_RANGE(0x4000, 0x5fff) AM_RAM // plain ram?
	AM_RANGE(0x6000, 0x6000) AM_WRITE_LEGACY(mt_z80_bank_w )
	AM_RANGE(0x6400, 0x6400) AM_READ_PORT("BIOS_DSW0")
	AM_RANGE(0x6401, 0x6401) AM_READ_PORT("BIOS_DSW1")
	AM_RANGE(0x6404, 0x6404) AM_READWRITE_LEGACY(megatech_cart_select_r, megatech_cart_select_w) // cart select & ram bank
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("BIOS_IN0")
	AM_RANGE(0x6801, 0x6801) AM_READ_PORT("BIOS_IN1")
	AM_RANGE(0x6802, 0x6807) AM_READWRITE_LEGACY(bios_ctrl_r, bios_ctrl_w)
//  AM_RANGE(0x6805, 0x6805) AM_READ_PORT("???")
	AM_RANGE(0x7000, 0x77ff) AM_ROM // from bios rom (0x7000-0x77ff populated in ROM)
	//AM_RANGE(0x7800, 0x7fff) AM_RAM // ?
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE_LEGACY(megatech_z80_read_68k_banked_data, megatech_z80_write_68k_banked_data) // window into 68k address space, reads instr rom and writes to reset banks on z80 carts?
ADDRESS_MAP_END


static WRITE8_HANDLER( megatech_bios_port_ctrl_w )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	state->m_bios_port_ctrl = data;
}

static READ8_HANDLER( megatech_bios_joypad_r )
{
	mtech_state *state = space->machine().driver_data<mtech_state>();
	return megatech_bios_port_cc_dc_r(space->machine(), offset, state->m_bios_port_ctrl);
}

static WRITE8_HANDLER (megatech_bios_port_7f_w)
{
//  popmessage("CPU #3: I/O port 0x7F write, data %02x", data);
}



static ADDRESS_MAP_START( megatech_bios_portmap, AS_IO, 8, mtech_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x3f, 0x3f) AM_WRITE_LEGACY(megatech_bios_port_ctrl_w)

	AM_RANGE(0x7f, 0x7f) AM_READWRITE_LEGACY(sms_vcounter_r, megatech_bios_port_7f_w)
	AM_RANGE(0xbe, 0xbe) AM_READWRITE_LEGACY(sms_vdp_data_r, sms_vdp_data_w)
	AM_RANGE(0xbf, 0xbf) AM_READWRITE_LEGACY(sms_vdp_ctrl_r, sms_vdp_ctrl_w)

	AM_RANGE(0xdc, 0xdd) AM_READ_LEGACY(megatech_bios_joypad_r)  // player inputs
ADDRESS_MAP_END



static DRIVER_INIT(mt_slot)
{
	mtech_state *state = machine.driver_data<mtech_state>();
	state->m_megatech_banked_ram = auto_alloc_array(machine, UINT8, 0x1000*8);

	DRIVER_INIT_CALL(megadriv);
	DRIVER_INIT_CALL(megatech_bios);

	// this gets set in DEVICE_IMAGE_LOAD
	memset(state->m_cart_is_genesis, 0, ARRAY_LENGTH(state->m_cart_is_genesis));
}

static DRIVER_INIT(mt_crt)
{
	mtech_state *state = machine.driver_data<mtech_state>();
	UINT8* pin = machine.region("sms_pin")->base();
	DRIVER_INIT_CALL(mt_slot);

	state->m_cart_is_genesis[0] = !pin[0] ? 1 : 0;;
}

static VIDEO_START(mtnew)
{
	init_for_megadrive(machine); // create an sms vdp too, for compatibility mode
	VIDEO_START_CALL(megadriv);
}

//attotime::never
static SCREEN_UPDATE_RGB32(mtnew)
{
	mtech_state *state = screen.machine().driver_data<mtech_state>();

	/* if we're running an sms game then use the SMS update.. maybe this should be moved to the megadrive emulation core as compatibility mode is a feature of the chip */
	if (!state->m_current_game_is_sms)
		SCREEN_UPDATE32_CALL(megadriv);
	else
		SCREEN_UPDATE32_CALL(megatech_md_sms);
	return 0;
}

static SCREEN_VBLANK(mtnew)
{
	mtech_state *state = screen.machine().driver_data<mtech_state>();
	if (!state->m_current_game_is_sms)
		SCREEN_VBLANK_CALL(megadriv);
	else
		SCREEN_VBLANK_CALL(megatech_md_sms);
}

static MACHINE_RESET(mtnew)
{
	mtech_state *state = machine.driver_data<mtech_state>();
	state->m_mt_bank_addr = 0;

	MACHINE_RESET_CALL(megadriv);
	MACHINE_RESET_CALL(megatech_bios);
	MACHINE_RESET_CALL(megatech_md_sms);
	megatech_select_game(machine, 0);
}

static MACHINE_CONFIG_START( megatech, mtech_state )
	/* basic machine hardware */
	MCFG_FRAGMENT_ADD(md_ntsc)

	/* Megatech has an extra SMS based bios *and* an additional screen */
	MCFG_CPU_ADD("mtbios", Z80, MASTER_CLOCK / 15) /* ?? */
	MCFG_CPU_PROGRAM_MAP(megatech_bios_map)
	MCFG_CPU_IO_MAP(megatech_bios_portmap)

	MCFG_MACHINE_RESET(mtnew)

	MCFG_VIDEO_START(mtnew)

	MCFG_DEFAULT_LAYOUT(layout_dualhovu)

	MCFG_SCREEN_ADD("menu", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(342,262)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_STATIC(megatech_bios)
	MCFG_SCREEN_VBLANK_STATIC(megatech_bios)

	MCFG_SCREEN_MODIFY("megadriv")
	MCFG_SCREEN_UPDATE_STATIC(mtnew)
	MCFG_SCREEN_VBLANK_STATIC(mtnew)

	/* sound hardware */
	MCFG_SOUND_ADD("sn2", SN76496, MASTER_CLOCK/15)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END


struct megatech_cart_region
{
	const char *tag;
	int        slot;
	const char *region;
};

// we keep old region tags for compatibility with older macros... this might be changed at a later stage
static const struct megatech_cart_region megatech_cart_table[] =
{
	{ ":cart1", 0, "game0" },
	{ ":cart2", 1, "game1" },
	{ ":cart3", 2, "game2" },
	{ ":cart4", 3, "game3" },
	{ ":cart5", 4, "game4" },
	{ ":cart6", 5, "game5" },
	{ ":cart7", 6, "game6" },
	{ ":cart8", 7, "game7" },
	{ 0 }
};

static DEVICE_IMAGE_LOAD( megatech_cart )
{
	mtech_state *state = image.device().machine().driver_data<mtech_state>();
	const struct megatech_cart_region *mt_cart = &megatech_cart_table[0], *this_cart;
	const char	*pcb_name;

	/* First, determine where this cart has to be loaded */
	while (mt_cart->tag)
	{
		if (strcmp(mt_cart->tag, image.device().tag()) == 0)
			break;

		mt_cart++;
	}

	this_cart = mt_cart;

	if (image.software_entry() == NULL)
		return IMAGE_INIT_FAIL;

	//printf("load list\n");
	UINT8 *ROM = image.device().machine().region(this_cart->region)->base();
	//printf("load list2\n");
	UINT32 length = image.get_software_region_length("rom");
	memcpy(ROM, image.get_software_region("rom"), length);


	if ((pcb_name = image.get_feature("pcb_type")) == NULL)
		return IMAGE_INIT_FAIL;
	else
	{
		if (!mame_stricmp("genesis", pcb_name))
		{
			printf("%s is genesis\n", mt_cart->tag);
			state->m_cart_is_genesis[this_cart->slot] = 1;
		}
		else if (!mame_stricmp("sms", pcb_name))
		{
			printf("%s is sms\n", mt_cart->tag);
			state->m_cart_is_genesis[this_cart->slot] = 0;
		}
		else
		{
			printf("%s is invalid\n", mt_cart->tag);
		}

	}

	return IMAGE_INIT_PASS;
}

#define MCFG_MEGATECH_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_INTERFACE("megatech_cart") \
	MCFG_CARTSLOT_LOAD(megatech_cart)

MACHINE_CONFIG_FRAGMENT( megatech_cartslot )
	MCFG_MEGATECH_CARTSLOT_ADD("cart1")
	MCFG_MEGATECH_CARTSLOT_ADD("cart2")
	MCFG_MEGATECH_CARTSLOT_ADD("cart3")
	MCFG_MEGATECH_CARTSLOT_ADD("cart4")
	MCFG_MEGATECH_CARTSLOT_ADD("cart5")
	MCFG_MEGATECH_CARTSLOT_ADD("cart6")
	MCFG_MEGATECH_CARTSLOT_ADD("cart7")
	MCFG_MEGATECH_CARTSLOT_ADD("cart8")

	MCFG_SOFTWARE_LIST_ADD("cart_list","megatech")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( megatech_slot, megatech )
	MCFG_FRAGMENT_ADD( megatech_cartslot )
MACHINE_CONFIG_END


/* MegaTech Games - Genesis & sms! Games with a timer */

#define MEGATECH_BIOS \
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x10000, "mtbios", 0 ) \
	ROM_LOAD( "epr12664.20", 0x000000, 0x8000, CRC(f71e9526) SHA1(1c7887541d02c41426992d17f8e3db9e03975953) ) \

/* no games */
ROM_START( megatech )
	MEGATECH_BIOS

	// empty memory areas, to copy data into
	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game1", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game2", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game3", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game4", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game5", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game6", ROMREGION_ERASE00 )
	ROM_REGION16_BE( 0x400000, "game7", ROMREGION_ERASE00 )
ROM_END


/* Game 01 - Altered Beast (Genesis) */
ROM_START( mt_beast ) /* Altered Beast */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12538.ic1", 0x000000, 0x080000, CRC(3bea3dce) SHA1(ec72e4fde191dedeb3f148f132603ed3c23f0f86) )
	ROM_LOAD16_BYTE( "epr-12368-01.ic2", 0x300001, 0x08000, CRC(40cb0088) SHA1(e1711532c29f395a35a1cb34d789015881b5a1ed) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 02 - Space Harrier 2 */
ROM_START( mt_shar2 ) /* Space Harrier 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11934.ic1", 0x000000, 0x080000, CRC(932daa09) SHA1(a2d7a76f3604c6227d43229908bfbd02b0ef5fd9) )
	ROM_LOAD16_BYTE( "epr-12368-02.ic2", 0x300001, 0x08000, CRC(c129c66c) SHA1(e7c0c97db9df9eb04e2f9ff561b64305219b8f1f) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 03 - Super Thunder Blade */
ROM_START( mt_stbld ) /* Super Thunder Blade */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11996f.ic1", 0x000000, 0x080000,  CRC(9355c34e) SHA1(26ff91c2921408673c644b0b1c8931d98524bf63) )
	ROM_LOAD16_BYTE( "epr-12368-03.ic2", 0x300001, 0x08000,  CRC(1ba4ac5d) SHA1(9bde57d70189d159ebdc537a9026001abfd0deae) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 04 - Great Golf (SMS) */
/* Also known to have the ID# MPR-11128 instead of MPR-11129F, same contents */
ROM_START( mt_ggolf ) /* Great Golf */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11129f.ic1", 0x000000, 0x020000, CRC(c6611c84) SHA1(eab0eed872dd26b13bcf0b2dd74fcbbc078812c9) )
	ROM_LOAD16_BYTE( "epr-12368-04.ic2", 0x300001, 0x08000, CRC(62e5579b) SHA1(e1f531be5c40a1216d4192baeda9352384444410) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 05 - Great Soccer (SMS) - bad dump */
ROM_START( mt_gsocr ) /* Great Soccer */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp10747f.ic1", 0x000000, 0x020000, BAD_DUMP CRC(9cf53703) SHA1(c6b4d1de56bd5bf067ec7fc80449c07686d01337) )
	ROM_LOAD16_BYTE( "epr-12368-05.ic2", 0x300001, 0x08000, CRC(bab91fcc) SHA1(a160c9d34b253e93ac54fdcef33f95f44d8fa90c) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 06 - Out Run (SMS) */
ROM_START( mt_orun ) /* Out Run */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-11078.ic1", 0x000000, 0x040000, CRC(5589d8d2) SHA1(4f9b61b24f0d9fee0448cdbbe8fc05411dbb1102) )
	ROM_LOAD16_BYTE( "epr-12368-06.ic2", 0x300001, 0x08000, CRC(c7c74429) SHA1(22ee261a653e10d66e0d6703c988bb7f236a7571) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 07 - Alien Syndrome (SMS) */
ROM_START( mt_asyn ) /* Alien Syndrome */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-11194.ic1", 0x000000, 0x040000, CRC(4cc11df9) SHA1(5d786476b275de34efb95f576dd556cf4b335a83) )
	ROM_LOAD16_BYTE( "epr-12368-07.ic2", 0x300001, 0x08000, CRC(14f4a17b) SHA1(0fc010ac95762534892f1ae16986dbf1c25399d3) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 08 - Shinobi (SMS) */
ROM_START( mt_shnbi ) /* Shinobi */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11706.ic1", 0x000000, 0x040000, CRC(0C6FAC4E) SHA1(7C0778C055DC9C2B0AAE1D166DBDB4734E55B9D1) )
	ROM_LOAD16_BYTE( "epr-12368-08.ic2", 0x300001, 0x08000, CRC(103A0459) SHA1(D803DDF7926B83785E8503C985B8C78E7CCB5DAC) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 09 - Fantasy Zone (SMS) */
// note, dump was bad, but the good (uniquely identifiable) parts matched the 'fantasy zone (world) (v1.2).bin' SMS rom
// so I'm using that until it gets verified.
ROM_START( mt_fz ) /* Fantasy Zone */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-10118.ic1", 0x000000, 0x020000, CRC(65d7e4e0) SHA1(0278cd120dc3a7707eda9314c46c7f27f9e8fdda) )
	ROM_LOAD16_BYTE( "epr-12368-09.bin", 0x300001, 0x08000, CRC(373d2a70) SHA1(c39dd1003d71a417b12a359126bfef64c7a2fd00) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END



/* Game 10 - Afterburner (SMS) */
ROM_START( mt_aftrb ) /* Afterburner */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11271.ic1", 0x000000, 0x080000, CRC(1C951F8E) SHA1(51531DF038783C84640A0CAB93122E0B59E3B69A) )
	ROM_LOAD16_BYTE( "epr-12368-10.ic2", 0x300001, 0x08000, CRC(2A7CB590) SHA1(2236963BDDC89CA9045B530259CC7B5CCF889EAF) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 11 - Thunder Force 2 */
ROM_START( mt_tfor2 ) /* Thunder Force 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12559.ic1", 0x000000, 0x080000, CRC(b093bee3) SHA1(0bf6194c3d228425f8cf1903ed70d8da1b027b6a) )
	ROM_LOAD16_BYTE( "epr-12368-11.ic2", 0x300001, 0x08000, CRC(f4f27e8d) SHA1(ae1a2823deb416c53838115966f1833d5dac72d4) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 13 - Astro Warrior (SMS) */
ROM_START( mt_astro ) /* Astro Warrior */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ep13817.ic2", 0x000000, 0x20000, CRC(299cbb74) SHA1(901697a3535ad70190647f34ad5b30b695d54542) )
	ROM_LOAD16_BYTE( "epr-12368-13.ic1", 0x300001, 0x08000,  CRC(4038cbd1) SHA1(696bc1efce45d9f0052b2cf0332a232687c8d6ab) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 19 - Great Football (SMS) */
ROM_START( mt_gfoot ) /* Great Football */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-10576f.ic1", 0x000000, 0x020000, CRC(2055825f) SHA1(a768f44ce7e50083ffe8c4b5e3ac93ceb7bd3266) )
	ROM_LOAD16_BYTE( "epr-12368-19.ic2", 0x300001, 0x08000, CRC(e27cb37a) SHA1(2b6259957e86d033a5689fd716a9efcfeff7d5ba) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 20 - Last Battle */
ROM_START( mt_lastb ) /* Last Battle */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12578f.ic1", 0x000000, 0x080000, CRC(531191a0) SHA1(f6bc26e975c01a3e10ab4033e4c5f494627a1e2f) )
	ROM_LOAD16_BYTE( "epr-12368-20.ic2", 0x300001, 0x08000, CRC(e1a71c91) SHA1(c250da18660d8aea86eb2abace41ba46130dabc8) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 21 - World Championship Soccer (Genesis) */
ROM_START( mt_wcsoc ) /* World Championship Soccer */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12607b.ic1", 0x000000, 0x080000, CRC(bc591b30) SHA1(55e8577171c0933eee53af1dabd0f4c6462d5fc8) )
	ROM_LOAD16_BYTE( "epr-12368-21.ic2", 0x300001, 0x08000, CRC(028ee46b) SHA1(cd8f81d66e5ae62107eb20e0ca5db4b66d4b2987) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 22 - Tetris */
ROM_START( mt_tetri ) /* Tetris */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "mpr-12356f.ic1", 0x000001, 0x020000, CRC(1e71c1a0) SHA1(44b2312792e49d46d71e0417a7f022e5ffddbbfe) )
	ROM_LOAD16_BYTE( "mpr-12357f.ic2", 0x000000, 0x020000, CRC(d52ca49c) SHA1(a9159892eee2c0cf28ebfcfa99f81f80781851c6) )
	ROM_LOAD16_BYTE( "epr-12368-22.ic3", 0x300001, 0x08000, CRC(1c1b6468) SHA1(568a38f4186167486e39ab4aa2c1ceffd0b81156) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 23 - Ghouls and Ghosts (Genesis) */
ROM_START( mt_gng ) /* Ghouls and Ghosts */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12605.ic1", 0x000000, 0x020000, CRC(1066C6AB) SHA1(C30E4442732BDB38C96D780542F8550A94D127B0) )
	ROM_LOAD16_WORD_SWAP( "mpr12606.ic2", 0x080000, 0x020000, CRC(D0BE7777) SHA1(A44B2A3D427F6973B5C1A3DCD8D1776366ACB9F7) )
	ROM_CONTINUE(0x020000,0x60000)
	ROM_LOAD16_BYTE( "epr-12368-23.ic3", 0x300001, 0x08000, CRC(7ee58546) SHA1(ad5bb0934475eacdc5e354f67c96fe0d2512d33b) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 24 - Super Hang-On (Genesis) */
ROM_START( mt_shang ) /* Super Hang-On */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-12640.ic1", 0x000000, 0x080000, CRC(2fe2cf62) SHA1(4728bcc847deb38b16338cbd0154837cd4a07b7d) )
	ROM_LOAD16_BYTE( "epr-12368-24.ic2", 0x300001, 0x08000, CRC(6c2db7e3) SHA1(8de0a10ed9185c9e98f17784811a79d3ce8c4c03) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 25 - Golden Axe (Genesis) */
ROM_START( mt_gaxe ) /* Golden Axe */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "12806.ic1", 0x000000, 0x080000, CRC(43456820) SHA1(2f7f1fcd979969ac99426f11ab99999a5494a121) )
	ROM_LOAD16_BYTE( "epr-12368-25.ic2", 0x300001, 0x08000, CRC(1f07ed28) SHA1(9d54192f4c6c1f8a51c38a835c1dd1e4e3e8279e) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 26 - Forgotten Worlds */
/* why is this pre-swapped like a console dump?? */
ROM_START( mt_fwrld ) /* Forgotten Worlds */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD( "mpr-12672-h.ic1", 0x000000, 0x080000, CRC(d0ee6434) SHA1(8b9a37c206c332ef23dc71f09ec40e1a92b1f83a) )
	ROM_LOAD16_BYTE( "epr-12368-26.ic2", 0x300001, 0x08000, CRC(4623b573) SHA1(29df4a5c5de66cd9cb7519e4f30000f7dddc2138) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 27 - Mystic Defender */
ROM_START( mt_mystd ) /* Mystic Defender */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12707.1", 0x000000, 0x080000, CRC(4f2c513d) SHA1(f9bb548b3688170fe18bb3f1b5b54182354143cf) )
	ROM_LOAD16_BYTE( "epr-12368-27.ic2", 0x300001, 0x08000, CRC(caf46f78) SHA1(a9659e86a6a223646338cd8f29c346866e4406c7) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 28 - The Revenge of Shinobi */
ROM_START( mt_revsh ) /* The Revenge Of Shinobi */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12675.ic1", 0x000000, 0x080000, CRC(672A1D4D) SHA1(5FD0AF14C8F2CF8CEAB1AE61A5A19276D861289A) )
	ROM_LOAD16_BYTE( "epr-12368-28.ic2", 0x300001, 0x08000, CRC(0D30BEDE) SHA1(73A090D84B78A570E02FB54A33666DCADA52849B) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 29 - Parlour Games (SMS) */
ROM_START( mt_parlg ) /* Parlour Games */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp11404.ic1", 0x000000, 0x020000, CRC(E030E66C) SHA1(06664DAF208F07CB00B603B12ECCFC3F01213A17) )
	ROM_LOAD16_BYTE( "epr-12368-29.ic2", 0x300001, 0x08000, CRC(534151e8) SHA1(219238d90c1d3ac07ff64c9a2098b490fff68f04) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASEFF )
ROM_END


/* Game 31 - Arnold Palmer Tournament Gold */
ROM_START( mt_tgolf ) /* Arnold Palmer Tournament Golf */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12645f.ic1", 0x000000, 0x080000, CRC(c07ef8d2) SHA1(9d111fdc7bb92d52bfa048cd134aa488b4f475ef) )
	ROM_LOAD16_BYTE( "epr-12368-31.ic2", 0x300001, 0x08000, CRC(30af7e4a) SHA1(baf91d527393dc90aba9371abcb1e690bcc83c7e) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 32 - Super Real Basketball */
/* why is this pre-swapped like a console dump?? */
ROM_START( mt_srbb ) /* Super Real Basketball */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD( "mpr-12904f.ic1", 0x000000, 0x080000, CRC(4346e11a) SHA1(c86725780027ef9783cb7884c8770cc030b0cd0d) )
	ROM_LOAD16_BYTE( "epr-12368-32.ic2", 0x300001, 0x08000, CRC(f70adcbe) SHA1(d4412a7cd59fe282a1c6619aa1051a2a2e00e1aa) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 35 - Tommy Lasorda Baseball */
ROM_START( mt_tlbba ) /* Tommy Lasorda Baseball */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp12706.ic1", 0x000000, 0x080000, CRC(8901214f) SHA1(f5ec166be1cf9b86623b9d7a78ec903b899da32a) )
	ROM_LOAD16_BYTE( "epr-12368-35.ic2", 0x300001, 0x08000, CRC(67bbe482) SHA1(6fc283b22e68befabb44b2cc61a7f82a71d6f029) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 36 - Columns */
ROM_START( mt_cols ) /* Columns */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13193-t.ic1", 0x000000, 0x080000, CRC(8c770e2f) SHA1(02a3626025c511250a3f8fb3176eebccc646cda9) )
	ROM_LOAD16_BYTE( "epr-12368-36.ic3",   0x300001, 0x008000,  CRC(a4b29bac) SHA1(c9be866ac96243897d09612fe17562e0481f66e3) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 38 - ESWAT */
ROM_START( mt_eswat ) /* ESWAT */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13192-h.ic1", 0x000000, 0x080000, CRC(82f458ef) SHA1(58444b783312def71ecffc4ad021b72a609685cb) )
	ROM_LOAD16_BYTE( "epr-12368-38.ic2", 0x300001, 0x08000, CRC(43c5529b) SHA1(104f85adea6da1612c0aa96d553efcaa387d7aaf) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 39 - Super Monaco Grand Prix (Genesis) */
ROM_START( mt_smgp ) /* Super Monaco Grand Prix */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "13250.ic1", 0x000000, 0x080000, CRC(189b885f) SHA1(31c06ffcb48b1604989a94e584261457de4f1f46) )
	ROM_LOAD16_BYTE( "epr-12368-39.ic2", 0x300001, 0x08000, CRC(64b3ce25) SHA1(83a9f2432d146a712b037f96f261742f7dc810bb) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 40 - Moon Walker */
ROM_START( mt_mwalk ) /* Moon Walker */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13285a.ic1", 0x000000, 0x080000, CRC(189516e4) SHA1(2a79e07da2e831832b8d448cae87a833c85e67c9) )
	ROM_LOAD16_BYTE( "epr-12368-40.ic2", 0x300001, 0x08000, CRC(0482378c) SHA1(734772f3ddb5ff82b76c3514d18a464b2bce8381) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 41 - Crackdown */
ROM_START( mt_crack ) /* Crackdown */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13578a-s.ic1", 0x000000, 0x080000, CRC(23f19893) SHA1(09aca793871e2246af4dc24925bc1eda8ff34446) )
	ROM_LOAD16_BYTE( "epr-12368-41.ic2", 0x300001, 0x08000, CRC(3014acec) SHA1(07953e9ae5c23fc7e7d08993b215f4dfa88aa5d7) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 43 - Shadow Dancer */
ROM_START( mt_shado ) /* Shadow Dancer */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-13571-s.ic1", 0x000000, 0x080000, CRC(56a29310) SHA1(55836177e4a1e2deb68408976b29d0282cf661a9) )
	ROM_LOAD16_BYTE( "epr-12368-43.ic2", 0x300001, 0x08000, CRC(1116cbc7) SHA1(ba6dd21ceadeedf730b71b67acbd20d9067114f3) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 44 - Arrow Flash */
ROM_START( mt_arrow ) /* Arrow Flash */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr13396h.ic1", 0x000000, 0x080000, CRC(091226e3) SHA1(cb15c6277314f3c4a86b5ae5823f72811d5d269d) )
	ROM_LOAD16_BYTE( "epr-12368-44.ic2", 0x300001, 0x08000, CRC(e653065d) SHA1(96b014fc4df8eb2188ac94ed0a778d974fe6dcad) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 47 - Alien Storm */
ROM_START( mt_astrm ) /* Alien Storm */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13941.ic1", 0x000000, 0x080000, CRC(D71B3EE6) SHA1(05F272DAD243D132D517C303388248DC4C0482ED) )
	ROM_LOAD16_BYTE( "epr-12368-47.ic2", 0x300001, 0x08000, CRC(31FB683D) SHA1(E356DA020BBF817B97FB10C27F75CF5931EDF4FC) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 48 - Wrestle War */
ROM_START( mt_wwar ) /* Wrestle War */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14025-f.ic1", 0x000000, 0x080000, CRC(26e899fe) SHA1(6d28e154ae2e4196097a2aa96c5acd5dfe7e3d2b) )
	ROM_LOAD16_BYTE( "epr-12368-48.ic2", 0x300001, 0x08000, CRC(25817bc2) SHA1(ba1bbb952aff12fb4d3ecfb10d82c54128439395) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 49 - Bonanza Bros. */
ROM_START( mt_bbros ) /* Bonanza Bros. */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13905a.ic1", 0x000000, 0x100000, CRC(68a88d60) SHA1(2f56e8a2b0999de4fa0d14a1527f4e1df0f9c7a2) )
	ROM_LOAD16_BYTE( "epr-12368-49.ic2", 0x300001, 0x08000, CRC(c5101da2) SHA1(636f30043e2e9291e193ef9a2ead2e97a0bf7380) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 51 - Streets of Rage */
ROM_START( mt_srage ) /* Streets of Rage */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14125-s.ic1", 0x000000, 0x080000, CRC(db4ac746) SHA1(c7cc24e2329f279574513fa32bbf79f72f75aeea) )
	ROM_LOAD16_BYTE( "epr-12368-51.ic2", 0x300001, 0x08000, CRC(49b7d6f4) SHA1(96e69851c92715e7daf35b184cf374147a8d2880) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 52 - Sonic The Hedgehog (Genesis) */
ROM_START( mt_sonic ) /* Sonic The Hedgehog */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13913.ic1", 0x000000, 0x080000, CRC(480b4b5c) SHA1(ab1dc1f738e3b2d0898a314b123fa71182bf572e) )
	ROM_LOAD16_BYTE( "epr-12368-52.ic2", 0x300001, 0x8000,  CRC(6a69d20c) SHA1(e483b39ff6eca37dc192dc296d004049e220554a) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


ROM_START( mt_sonia ) /* Sonic (alt)*/
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp13933.ic1", 0x000000, 0x080000, CRC(13775004) SHA1(5decfd35944a2d0e7b996b9a4a12b616a309fd5e) )
	ROM_LOAD16_BYTE( "epr-12368-52.ic2", 0x300001, 0x8000,  CRC(6a69d20c) SHA1(e483b39ff6eca37dc192dc296d004049e220554a) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 53 - Fire Shark */
	/* alt version with these roms exists, but the content is the same */
	/* (6a221fd6) ep14706.ic1             mp14341.ic1  [even]     IDENTICAL */
	/* (09fa48af) ep14707.ic2             mp14341.ic1  [odd]      IDENTICAL */

ROM_START( mt_fshrk ) /* Fire Shark */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14341.ic1", 0x000000, 0x080000, CRC(04d65ebc) SHA1(24338aecdc52b6f416548be722ca475c83dbae96) )
	ROM_LOAD16_BYTE( "epr-12368-53.ic2", 0x300001, 0x08000,  CRC(4fa61044) SHA1(7810deea221c10b0b2f5233443d81f4f1998ee58) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 54 - Spiderman */
ROM_START( mt_spman ) /* Spiderman */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14027-sm.ic1", 0x000000, 0x080000, CRC(e2c08a06) SHA1(39e592eafd47e2aa6edbb4845d44750057bff890) )
	ROM_LOAD16_BYTE( "epr-12368-54.ic2", 0x300001, 0x08000,  CRC(30b68988) SHA1(04eeb0fad732a791b6bc0c0846306d567573649f) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 55 - California Games */
ROM_START( mt_calga ) /* California Games */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "epr-14494.ic1", 0x000001, 0x040000, CRC(cbe58b1b) SHA1(ea067fc08e644c993f8d13731425c9296c1a2a75) )
	ROM_LOAD16_BYTE( "epr-14495.ic2", 0x000000, 0x040000, CRC(cb956f4f) SHA1(3574c496b79aefdec7d02975490ebe3bb373bc60) )
	ROM_LOAD16_BYTE( "epr-12368-55.ic3", 0x300001, 0x08000, CRC(6f7dd8f5) SHA1(a6cb1aa8c3635738dd9e4d3e0d729d089fd9b599) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 57 - Golden Axe 2 (Genesis) */
ROM_START( mt_gaxe2 ) /* Golden Axe 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14272.ic1", 0x000000, 0x080000, CRC(d4784cae) SHA1(b6c286027d06fd850016a2a1ee1f1aeea080c3bb) )
	ROM_LOAD16_BYTE( "epr-12368-57.ic2", 0x300001, 0x08000, CRC(dc9b4433) SHA1(efd3a598569010cdc4bf38ecbf9ed1b4e14ffe36) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 58 - Sports Talk Football */
ROM_START( mt_stf ) /* Sports Talk Football */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14356a-f.ic1", 0x000000, 0x100000, CRC(20cf32f6) SHA1(752314346a7a98b3808b3814609e024dc0a4108c) )
	ROM_LOAD16_BYTE( "epr-12368-58.ic2", 0x300001, 0x08000, CRC(dce2708e) SHA1(fcebb1899ee11468f6bda705899f074e7de9d723) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 59 - Mario Lemieux Hockey */
ROM_START( mt_mlh ) /* Mario Lemieux Hockey */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mpr-14376-h.ic1", 0x000000, 0x80000, CRC(aa9be87e) SHA1(dceed94eaeb30e534f6953a4bc25ff37673b1e6b) )
	ROM_LOAD16_BYTE( "epr-12368-59.ic2", 0x300001, 0x08000, CRC(6d47b438) SHA1(0a145f6438e4e55c957ae559663c37662b685246) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 60 - Kid Chameleon */
ROM_START( mt_kcham ) /* Kid Chameleon */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp14557.ic1", 0x000000, 0x100000, CRC(e1a889a4) SHA1(a2768eacafc47d371e5276f0cce4b12b6041337a) )
	ROM_LOAD16_BYTE( "epr-12368-60.ic2", 0x300001, 0x08000, CRC(a8e4af18) SHA1(dfa49f6ec4047718f33dba1180f6204dbaff884c) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 61 - Turbo Outrun */
// original dump of epr-14674.ic1 had CRC(c2b9a802) SHA1(108cc844c944125f9d271a2f2db094301294e8c2)
// with the byte at offset 3 being F6 instead of Fe, this seems like a bad dump when compared to the Genesis rom which
// has been verified on multiple carts, chances are the ROM had developed a fault.
ROM_START( mt_tout ) /* Turbo Outrun */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "epr-14674.ic1", 0x000000, 0x080000, CRC(453712a2) SHA1(5d2c8430a9a14aac7f19c22617539b0503ab92cd) )
	ROM_LOAD16_BYTE( "epr-12368-61.ic2", 0x300001, 0x08000, CRC(4aa0b2a2) SHA1(bce03f88d6cfd02683d51c28058f6229fda13b49) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END


/* Game 62 - Sonic The Hedgehog 2 */
ROM_START( mt_soni2 ) /* Sonic The Hedgehog 2 */
	MEGATECH_BIOS

	ROM_REGION16_BE( 0x400000, "game0", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "mp15000a-f.ic1", 0x000000, 0x100000, CRC(679ebb49) SHA1(557482064677702454562f753460993067ef9e16) )
	ROM_LOAD16_BYTE( "epr-12368-62.ic2", 0x300001, 0x08000, CRC(14a8566f) SHA1(d1d14162144bf068ddd19e9736477ff98fb43f9e) )

	ROM_REGION( 0x01, "sms_pin", ROMREGION_ERASE00 )
ROM_END



/* nn */ /* nn is part of the instruction rom name, should there be a game for each number? */
/* -- */ CONS( 1989, megatech, 0, 0,     megatech_slot, megatech, mt_slot, "Sega",                  "Mega-Tech", GAME_IS_BIOS_ROOT )
/* 01 */ GAME( 1988, mt_beast, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Altered Beast (Mega-Tech)", GAME_NOT_WORKING )
/* 02 */ GAME( 1988, mt_shar2, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Space Harrier II (Mega-Tech)", GAME_NOT_WORKING )
/* 03 */ GAME( 1988, mt_stbld, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Super Thunder Blade (Mega-Tech)", GAME_NOT_WORKING )
/* 04 */ GAME( 1987, mt_ggolf, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Great Golf (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! */
/* 05 */ GAME( 198?, mt_gsocr, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Great Soccer (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! also bad */
/* 06 */ GAME( 1987, mt_orun,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Out Run (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! */
/* 07 */ GAME( 1987, mt_asyn,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Alien Syndrome (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! */
/* 08 */ GAME( 1987, mt_shnbi, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Shinobi (Mega-Tech, SMS based)", GAME_NOT_WORKING) /* sms */
/* 09 */ GAME( 1987, mt_fz,    megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Fantasy Zone (Mega-Tech, SMS based)", GAME_NOT_WORKING) /* sms */
/* 10 */ GAME( 1987, mt_aftrb, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "After Burner (Mega-Tech, SMS based)", GAME_NOT_WORKING) /* sms */
/* 11 */ GAME( 1989, mt_tfor2, megatech, megatech, megatech, mt_crt, ROT0, "Tecno Soft / Sega",     "Thunder Force II MD (Mega-Tech)", GAME_NOT_WORKING )
/* 12 */ // unknown
/* 13 */ GAME( 1986, mt_astro, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Astro Warrior (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! */
/* 14 */ // unknown
/* 15 */ // unknown
/* 16 */ // unknown
/* 17 */ // unknown
/* 18 */ // Kung Fu Kid (sms)
/* 19 */ GAME( 1987, mt_gfoot, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Great Football (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! */
/* 20 */ GAME( 1989, mt_lastb, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Last Battle (Mega-Tech)", GAME_NOT_WORKING )
/* 21 */ GAME( 1989, mt_wcsoc, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "World Championship Soccer (Mega-Tech)", GAME_NOT_WORKING )
/* 22 */ GAME( 1989, mt_tetri, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Tetris (Mega-Tech)", GAME_NOT_WORKING )
/* 23 */ GAME( 1989, mt_gng,   megatech, megatech, megatech, mt_crt, ROT0, "Capcom / Sega",         "Ghouls'n Ghosts (Mega-Tech)", GAME_NOT_WORKING )
/* 24 */ GAME( 1989, mt_shang, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Super Hang-On (Mega-Tech)", GAME_NOT_WORKING )
/* 25 */ GAME( 1989, mt_gaxe,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Golden Axe (Mega-Tech)", GAME_NOT_WORKING )
/* 26 */ GAME( 1989, mt_fwrld, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Forgotten Worlds (Mega-Tech)", GAME_NOT_WORKING )
/* 27 */ GAME( 1989, mt_mystd, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Mystic Defender (Mega-Tech)", GAME_NOT_WORKING )
/* 28 */ GAME( 1989, mt_revsh, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "The Revenge of Shinobi (Mega-Tech)", GAME_NOT_WORKING )
/* 29 */ GAME( 1987, mt_parlg, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Parlour Games (Mega-Tech, SMS based)", GAME_NOT_WORKING ) /* sms! */
/* 30 */ // unknown
/* 31 */ GAME( 1989, mt_tgolf, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Arnold Palmer Tournament Golf (Mega-Tech)", GAME_NOT_WORKING )
/* 32 */ GAME( 1989, mt_srbb,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Super Real Basketball (Mega-Tech)", GAME_NOT_WORKING )
/* 33 */ // unknown
/* 34 */ // unknown
/* 35 */ GAME( 1989, mt_tlbba, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Tommy Lasorda Baseball (Mega-Tech)", GAME_NOT_WORKING )
/* 36 */ GAME( 1990, mt_cols,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Columns (Mega-Tech)", GAME_NOT_WORKING )
/* 37 */ // unknown
/* 38 */ GAME( 1990, mt_eswat, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Cyber Police ESWAT: Enhanced Special Weapons and Tactics (Mega-Tech)", GAME_NOT_WORKING )
/* 39 */ GAME( 1990, mt_smgp,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Super Monaco GP (Mega-Tech)", GAME_NOT_WORKING )
/* 40 */ GAME( 1990, mt_mwalk, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Michael Jackson's Moonwalker (Mega-Tech)", GAME_NOT_WORKING )
/* 41 */ GAME( 1990, mt_crack, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Crack Down (Mega-Tech)", GAME_NOT_WORKING )
/* 42 */ // unknown
/* 43 */ GAME( 1990, mt_shado, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Shadow Dancer (Mega-Tech)", GAME_NOT_WORKING )
/* 44 */ GAME( 1990, mt_arrow, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Arrow Flash (Mega-Tech)", GAME_NOT_WORKING )
/* 45 */ // unknown
/* 46 */ // unknown
/* 47 */ GAME( 1990, mt_astrm, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Alien Storm (Mega-Tech)", GAME_NOT_WORKING )
/* 48 */ GAME( 1991, mt_wwar,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Wrestle War (Mega-Tech)", GAME_NOT_WORKING ) /* Copyright 1989, 1991 Sega */
/* 49 */ GAME( 1991, mt_bbros, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Bonanza Bros. (Mega-Tech)", GAME_NOT_WORKING )
/* 50 */ // unknown
/* 51 */ GAME( 1991, mt_srage, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Streets of Rage (Mega-Tech)", GAME_NOT_WORKING )
/* 52 */ GAME( 1991, mt_sonic, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Sonic The Hedgehog (Mega-Tech, set 1)", GAME_NOT_WORKING )
/*    */ GAME( 1991, mt_sonia, mt_sonic, megatech, megatech, mt_crt, ROT0, "Sega",                  "Sonic The Hedgehog (Mega-Tech, set 2)", GAME_NOT_WORKING )
/* 53 */ GAME( 1990, mt_fshrk, megatech, megatech, megatech, mt_crt, ROT0, "Toaplan / Sega",        "Fire Shark (Mega-Tech)", GAME_NOT_WORKING )
/* 54 */ GAME( 1991, mt_spman, megatech, megatech, megatech, mt_crt, ROT0, "Marvel / Sega",         "Spider-Man vs The Kingpin (Mega-Tech)", GAME_NOT_WORKING )
/* 55 */ GAME( 1991, mt_calga, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "California Games (Mega-Tech)", GAME_NOT_WORKING )
/* 56 */ // unknown
/* 57 */ GAME( 1991, mt_gaxe2, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Golden Axe II (Mega-Tech)", GAME_NOT_WORKING )
/* 58 */ GAME( 1991, mt_stf,   megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Joe Montana II: Sports Talk Football (Mega-Tech)", GAME_NOT_WORKING )
/* 59 */ GAME( 1991, mt_mlh,   megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Mario Lemieux Hockey (Mega-Tech)", GAME_NOT_WORKING )
/* 60 */ GAME( 1992, mt_kcham, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Kid Chameleon (Mega-Tech)", GAME_NOT_WORKING )
/* 61 */ GAME( 1992, mt_tout,  megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Turbo Outrun (Mega-Tech)", GAME_NOT_WORKING )
/* 62 */ GAME( 1992, mt_soni2, megatech, megatech, megatech, mt_crt, ROT0, "Sega",                  "Sonic The Hedgehog 2 (Mega-Tech)", GAME_NOT_WORKING )

/* more? */
