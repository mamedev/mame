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
		state->m_bios_ctrl_inputs = data & 0x04;  // Genesis/SMS input ports disable bit
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



static ADDRESS_MAP_START( megatech_bios_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM // from bios rom (0x0000-0x2fff populated in ROM)
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE(megatech_banked_ram_r, megatech_banked_ram_w) // copies instruction data here at startup, must be banked
	AM_RANGE(0x4000, 0x5fff) AM_RAM // plain ram?
	AM_RANGE(0x6000, 0x6000) AM_WRITE( mt_z80_bank_w )
	AM_RANGE(0x6400, 0x6400) AM_READ_PORT("BIOS_DSW0")
	AM_RANGE(0x6401, 0x6401) AM_READ_PORT("BIOS_DSW1")
	AM_RANGE(0x6404, 0x6404) AM_READWRITE(megatech_cart_select_r, megatech_cart_select_w) // cart select & ram bank
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("BIOS_IN0")
	AM_RANGE(0x6801, 0x6801) AM_READ_PORT("BIOS_IN1")
	AM_RANGE(0x6802, 0x6807) AM_READWRITE(bios_ctrl_r, bios_ctrl_w)
//  AM_RANGE(0x6805, 0x6805) AM_READ_PORT("???")
	AM_RANGE(0x7000, 0x77ff) AM_ROM // from bios rom (0x7000-0x77ff populated in ROM)
	//AM_RANGE(0x7800, 0x7fff) AM_RAM // ?
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(megatech_z80_read_68k_banked_data, megatech_z80_write_68k_banked_data) // window into 68k address space, reads instr rom and writes to reset banks on z80 carts?
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



static ADDRESS_MAP_START( megatech_bios_portmap, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x3f, 0x3f) AM_WRITE(megatech_bios_port_ctrl_w)

	AM_RANGE(0x7f, 0x7f) AM_READWRITE(sms_vcounter_r, megatech_bios_port_7f_w)
	AM_RANGE(0xbe, 0xbe) AM_READWRITE(sms_vdp_data_r, sms_vdp_data_w)
	AM_RANGE(0xbf, 0xbf) AM_READWRITE(sms_vdp_ctrl_r, sms_vdp_ctrl_w)

	AM_RANGE(0xdc, 0xdd) AM_READ(megatech_bios_joypad_r)  // player inputs
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


static VIDEO_START(mtnew)
{
	init_for_megadrive(machine); // create an sms vdp too, for compatibility mode
	VIDEO_START_CALL(megadriv);
}

//attotime::never
static SCREEN_UPDATE(mtnew)
{
	mtech_state *state = screen->machine().driver_data<mtech_state>();
	device_t *megadriv_screen = screen->machine().device("megadriv");
	device_t *menu_screen = screen->machine().device("menu");

	if (screen == megadriv_screen)
	{
		/* if we're running an sms game then use the SMS update.. maybe this should be moved to the megadrive emulation core as compatibility mode is a feature of the chip */
		if (!state->m_current_game_is_sms)
			SCREEN_UPDATE_CALL(megadriv);
		else
			SCREEN_UPDATE_CALL(megatech_md_sms);
	}
	else if (screen == menu_screen)
		SCREEN_UPDATE_CALL(megatech_bios);
	return 0;
}

static SCREEN_EOF(mtnew)
{
	mtech_state *state = machine.driver_data<mtech_state>();
	if (!state->m_current_game_is_sms)
		SCREEN_EOF_CALL(megadriv);
	else
		SCREEN_EOF_CALL(megatech_md_sms);

	SCREEN_EOF_CALL(megatech_bios);
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
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(342,262)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)
	MCFG_SCREEN_UPDATE(mtnew)

	MCFG_SCREEN_MODIFY("megadriv")
	MCFG_SCREEN_UPDATE(mtnew)
	MCFG_SCREEN_EOF(mtnew)

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
	{ "cart1", 0, "game0" },
	{ "cart2", 1, "game1" },
	{ "cart3", 2, "game2" },
	{ "cart4", 3, "game3" },
	{ "cart5", 4, "game4" },
	{ "cart6", 5, "game5" },
	{ "cart7", 6, "game6" },
	{ "cart8", 7, "game7" },
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


CONS( 1989, megatech, 0, 0,     megatech_slot, megatech, mt_slot, "Sega",                  "Mega-Tech", GAME_IS_BIOS_ROOT )
/* megatech cartridges are handled using the softlists, see hash/megatech.xml */
