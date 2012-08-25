/*

Megadrive / Genesis support

this could probably do with a complete rewrite at this point to take into
account all new information discovered since this was created, it's looking
rather old now.



Cleanup / Rewrite notes:

On SegaC2 the VDP never turns on the IRQ6 enable register
  This is because on the real PCB that line of the VDP isn't
  connected.  Instead the IRQ6 interrupt is triggered by the
  line that is used to generate the Z80 interrupt on a standard
  genesis.  (Once, every frame, on screenline 224)

  I should provide interrupt callback functions for each
  vdp line state change, which can be configured in the init
  rather than hardcoding them.


Known Non-Issues (confirmed on Real Genesis)
    Castlevania - Bloodlines (U) [!] - Pause text is missing on upside down level
    Blood Shot (E) (M4) [!] - corrupt texture in level 1 is correct...



*/


#include "emu.h"
#include "coreutil.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sh2/sh2.h"
#include "cpu/sh2/sh2comn.h"
#include "cpu/z80/z80.h"
#include "sound/2612intf.h"
#include "sound/cdda.h"
#include "sound/dac.h"
#include "sound/rf5c68.h"
#include "sound/sn76496.h"
#include "imagedev/chd_cd.h"
#include "includes/megadriv.h"
#include "machine/nvram.h"
#include "cpu/ssp1601/ssp1601.h"
#include "megacd.lh"

#include "machine/megavdp.h"




static cpu_device *_genesis_snd_z80_cpu;
int genesis_other_hacks = 0; // misc hacks

timer_device* megadriv_scanline_timer;
UINT16* megadrive_ram = NULL;

struct genesis_z80_vars
{
	int z80_is_reset;
	int z80_has_bus;
	UINT32 z80_bank_addr;
	UINT8* z80_prgram;
};

genesis_z80_vars genz80;

void megadriv_z80_hold(running_machine &machine)
{
	if ((genz80.z80_has_bus == 1) && (genz80.z80_is_reset == 0))
		cputag_set_input_line(machine, ":genesis_snd_z80", 0, HOLD_LINE);
}

void megadriv_z80_clear(running_machine &machine)
{
	cputag_set_input_line(machine, ":genesis_snd_z80", 0, CLEAR_LINE);
}

static void megadriv_z80_bank_w(UINT16 data)
{
	genz80.z80_bank_addr = ( ( genz80.z80_bank_addr >> 1 ) | ( data << 23 ) ) & 0xff8000;
}

static WRITE16_HANDLER( megadriv_68k_z80_bank_write )
{
	//logerror("%06x: 68k writing bit to bank register %01x\n", cpu_get_pc(&space->device()),data&0x01);
	megadriv_z80_bank_w(data&0x01);
}

static WRITE8_HANDLER(megadriv_z80_z80_bank_w)
{
	//logerror("%04x: z80 writing bit to bank register %01x\n", cpu_get_pc(&space->device()),data&0x01);
	megadriv_z80_bank_w(data&0x01);
}


static READ16_HANDLER( megadriv_68k_check_z80_bus );
static WRITE16_HANDLER(megadriv_68k_req_z80_bus);

static READ16_HANDLER( megadriv_68k_read_z80_ram );
static WRITE16_HANDLER( megadriv_68k_write_z80_ram );

static WRITE16_HANDLER( megadriv_68k_req_z80_reset );



READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read)
{
	//mame_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		return ym2612_r(device, offset);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (read) without bus\n", device->machine().describe_context());
		return 0;
	}

	return -1;
}


WRITE8_DEVICE_HANDLER( megadriv_68k_YM2612_write)
{
	//mame_printf_debug("megadriv_68k_YM2612_write %02x %04x %04x\n",offset,data,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		ym2612_w(device, offset, data);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (write) without bus\n", device->machine().describe_context());
	}
}

/* Megadrive / Genesis has 3 I/O ports */
static emu_timer *io_timeout[3];
static int io_stage[3];

static TIMER_CALLBACK( io_timeout_timer_callback )
{
	io_stage[(int)(FPTR)ptr] = -1;
}

static void init_megadri6_io(running_machine &machine)
{
	int i;

	for (i=0; i<3; i++)
	{
		io_timeout[i] = machine.scheduler().timer_alloc(FUNC(io_timeout_timer_callback), (void*)(FPTR)i);
	}
}

/* pointers to our io data read/write functions */
UINT8 (*megadrive_io_read_data_port_ptr)(running_machine &machine, int offset);
void (*megadrive_io_write_data_port_ptr)(running_machine &machine, int offset, UINT16 data);

/*

    A10001h = A0         Version register

    A10003h = 7F         Data register for port A
    A10005h = 7F         Data register for port B
    A10007h = 7F         Data register for port C

    A10009h = 00         Ctrl register for port A
    A1000Bh = 00         Ctrl register for port B
    A1000Dh = 00         Ctrl register for port C

    A1000Fh = FF         TxData register for port A
    A10011h = 00         RxData register for port A
    A10013h = 00         S-Ctrl register for port A

    A10015h = FF         TxData register for port B
    A10017h = 00         RxData register for port B
    A10019h = 00         S-Ctrl register for port B

    A1001Bh = FF         TxData register for port C
    A1001Dh = 00         RxData register for port C
    A1001Fh = 00         S-Ctrl register for port C




 Bit 7 - (Not connected)
 Bit 6 - TH
 Bit 5 - TL
 Bit 4 - TR
 Bit 3 - RIGHT
 Bit 2 - LEFT
 Bit 1 - DOWN
 Bit 0 - UP


*/

INPUT_PORTS_START( md_common )
	PORT_START("PAD1")		/* Joypad 1 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") // b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") // c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") // a
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 START") // start

	PORT_START("PAD2")		/* Joypad 2 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") // b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") // c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") // a
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 START") // start
INPUT_PORTS_END


INPUT_PORTS_START( megadriv )
	PORT_INCLUDE( md_common )

	PORT_START("RESET")		/* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)
INPUT_PORTS_END

INPUT_PORTS_START( megadri6 )
	PORT_INCLUDE( megadriv )

	PORT_START("EXTRA1")	/* Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Z") // z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Y") // y
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 X") // x
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("P1 MODE") // mode

	PORT_START("EXTRA2")	/* Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Z") // z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Y") // y
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 X") // x
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("P2 MODE") // mode
INPUT_PORTS_END

UINT8 megadrive_io_data_regs[3];
UINT8 megadrive_io_ctrl_regs[3];
static UINT8 megadrive_io_tx_regs[3];
int megadrive_6buttons_pad = 0;

static void megadrive_reset_io(running_machine &machine)
{
	int i;

	megadrive_io_data_regs[0] = 0x7f;
	megadrive_io_data_regs[1] = 0x7f;
	megadrive_io_data_regs[2] = 0x7f;
	megadrive_io_ctrl_regs[0] = 0x00;
	megadrive_io_ctrl_regs[1] = 0x00;
	megadrive_io_ctrl_regs[2] = 0x00;
	megadrive_io_tx_regs[0] = 0xff;
	megadrive_io_tx_regs[1] = 0xff;
	megadrive_io_tx_regs[2] = 0xff;

	for (i=0; i<3; i++)
	{
		io_stage[i] = -1;
	}
}

/************* 6 buttons version **************************/
static UINT8 megadrive_io_read_data_port_6button(running_machine &machine, int portnum)
{
	UINT8 retdata, helper = (megadrive_io_ctrl_regs[portnum] & 0x3f) | 0xc0; // bits 6 & 7 always come from megadrive_io_data_regs
	static const char *const pad3names[] = { "PAD1", "PAD2", "IN0", "UNK" };
	static const char *const pad6names[] = { "EXTRA1", "EXTRA2", "IN0", "UNK" };

	if (megadrive_io_data_regs[portnum] & 0x40)
	{
		if (io_stage[portnum] == 2)
		{
			/* here we read B, C & the additional buttons */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						(((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0x30) |
							(machine.root_device().ioport(pad6names[portnum])->read_safe(0) & 0x0f)) & ~helper);
		}
		else
		{
			/* here we read B, C & the directional buttons */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0x3f) & ~helper);
		}
	}
	else
	{
		if (io_stage[portnum] == 1)
		{
			/* here we read ((Start & A) >> 2) | 0x00 */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						(((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0xc0) >> 2) & ~helper);
		}
		else if (io_stage[portnum]==2)
		{
			/* here we read ((Start & A) >> 2) | 0x0f */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						((((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0xc0) >> 2) | 0x0f) & ~helper);
		}
		else
		{
			/* here we read ((Start & A) >> 2) | Up and Down */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						((((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0xc0) >> 2) |
							(machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0x03)) & ~helper);
		}
	}

//  mame_printf_debug("read io data port stage %d port %d %02x\n",io_stage[portnum],portnum,retdata);

	return retdata | (retdata << 8);
}


/************* 3 buttons version **************************/
UINT8 megadrive_io_read_data_port_3button(running_machine &machine, int portnum)
{
	UINT8 retdata, helper = (megadrive_io_ctrl_regs[portnum] & 0x7f) | 0x80; // bit 7 always comes from megadrive_io_data_regs
	static const char *const pad3names[] = { "PAD1", "PAD2", "IN0", "UNK" };

	if (megadrive_io_data_regs[portnum] & 0x40)
	{
		/* here we read B, C & the directional buttons */
		retdata = (megadrive_io_data_regs[portnum] & helper) |
					(((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0x3f) | 0x40) & ~helper);
	}
	else
	{
		/* here we read ((Start & A) >> 2) | Up and Down */
		retdata = (megadrive_io_data_regs[portnum] & helper) |
					((((machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0xc0) >> 2) |
						(machine.root_device().ioport(pad3names[portnum])->read_safe(0) & 0x03) | 0x40) & ~helper);
	}

	return retdata;
}

/* used by megatech bios, the test mode accesses the joypad/stick inputs like this */
UINT8 megatech_bios_port_cc_dc_r(running_machine &machine, int offset, int ctrl)
{
	UINT8 retdata;

	if (ctrl == 0x55)
	{
			/* A keys */
			retdata = ((machine.root_device().ioport("PAD1")->read() & 0x40) >> 2) |
				((machine.root_device().ioport("PAD2")->read() & 0x40) >> 4) | 0xeb;
	}
	else
	{
		if (offset == 0)
		{
			retdata = (machine.root_device().ioport("PAD1")->read() & 0x3f) | ((machine.root_device().ioport("PAD2")->read() & 0x03) << 6);
		}
		else
		{
			retdata = ((machine.root_device().ioport("PAD2")->read() & 0x3c) >> 2) | 0xf0;
		}

	}

	return retdata;
}

static UINT8 megadrive_io_read_ctrl_port(int portnum)
{
	UINT8 retdata;
	retdata = megadrive_io_ctrl_regs[portnum];
	//mame_printf_debug("read io ctrl port %d %02x\n",portnum,retdata);

	return retdata | (retdata << 8);
}

static UINT8 megadrive_io_read_tx_port(int portnum)
{
	UINT8 retdata;
	retdata = megadrive_io_tx_regs[portnum];
	return retdata | (retdata << 8);
}

static UINT8 megadrive_io_read_rx_port(int portnum)
{
	return 0x00;
}

static UINT8 megadrive_io_read_sctrl_port(int portnum)
{
	return 0x00;
}


READ16_HANDLER( megadriv_68k_io_read )
{
	UINT8 retdata;

	retdata = 0;
      /* Charles MacDonald ( http://cgfm2.emuviews.com/ )
          D7 : Console is 1= Export (USA, Europe, etc.) 0= Domestic (Japan)
          D6 : Video type is 1= PAL, 0= NTSC
          D5 : Sega CD unit is 1= not present, 0= connected.
          D4 : Unused (always returns zero)
          D3 : Bit 3 of version number
          D2 : Bit 2 of version number
          D1 : Bit 1 of version number
          D0 : Bit 0 of version number
      */

	//return (space->machine().rand()&0x0f0f)|0xf0f0;//0x0000;
	switch (offset)
	{
		case 0:
			logerror("%06x read version register\n", cpu_get_pc(&space->device()));
			retdata = megadrive_region_export<<7 | // Export
			          megadrive_region_pal<<6 | // NTSC
			          (sega_cd_connected?0x00:0x20) | // 0x20 = no sega cd
			          0x00 | // Unused (Always 0)
			          0x00 | // Bit 3 of Version Number
			          0x00 | // Bit 2 of Version Number
			          0x00 | // Bit 1 of Version Number
			          0x01 ; // Bit 0 of Version Number
			break;

		/* Joystick Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          retdata = megadrive_io_read_data_port(offset-1);
			retdata = megadrive_io_read_data_port_ptr(space->machine(), offset-1);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			retdata = megadrive_io_read_ctrl_port(offset-4);
			break;

		/* Serial I/O Registers */

		case 0x7: retdata = megadrive_io_read_tx_port(0); break;
		case 0x8: retdata = megadrive_io_read_rx_port(0); break;
		case 0x9: retdata = megadrive_io_read_sctrl_port(0); break;

		case 0xa: retdata = megadrive_io_read_tx_port(1); break;
		case 0xb: retdata = megadrive_io_read_rx_port(1); break;
		case 0xc: retdata = megadrive_io_read_sctrl_port(1); break;

		case 0xd: retdata = megadrive_io_read_tx_port(2); break;
		case 0xe: retdata = megadrive_io_read_rx_port(2); break;
		case 0xf: retdata = megadrive_io_read_sctrl_port(2); break;

	}

	return retdata | (retdata << 8);
}


static void megadrive_io_write_data_port_3button(running_machine &machine, int portnum, UINT16 data)
{
	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/****************************** 6 buttons version*****************************/

static void megadrive_io_write_data_port_6button(running_machine &machine, int portnum, UINT16 data)
{
	if (megadrive_io_ctrl_regs[portnum]&0x40)
	{
		if (((megadrive_io_data_regs[portnum]&0x40)==0x00) && ((data&0x40) == 0x40))
		{
			io_stage[portnum]++;
			io_timeout[portnum]->adjust(machine.device<cpu_device>("maincpu")->cycles_to_attotime(8192));
		}

	}

	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/*************************** 3 buttons version ****************************/

static void megadrive_io_write_ctrl_port(running_machine &machine, int portnum, UINT16 data)
{
	megadrive_io_ctrl_regs[portnum] = data;
//  mame_printf_debug("Setting IO Control Register #%d data %04x\n",portnum,data);
}

static void megadrive_io_write_tx_port(running_machine &machine, int portnum, UINT16 data)
{
	megadrive_io_tx_regs[portnum] = data;
}

static void megadrive_io_write_rx_port(running_machine &machine, int portnum, UINT16 data)
{

}

static void megadrive_io_write_sctrl_port(running_machine &machine, int portnum, UINT16 data)
{

}


WRITE16_HANDLER( megadriv_68k_io_write )
{
//  mame_printf_debug("IO Write #%02x data %04x mem_mask %04x\n",offset,data,mem_mask);


	switch (offset)
	{
		case 0x0:
			mame_printf_debug("Write to Version Register?!\n");
			break;

		/* Joypad Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          megadrive_io_write_data_port(offset-1,data);
			megadrive_io_write_data_port_ptr(space->machine(), offset-1,data);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			megadrive_io_write_ctrl_port(space->machine(),offset-4,data);
			break;

		/* Serial I/O Registers */

		case 0x7: megadrive_io_write_tx_port(space->machine(),0,data); break;
		case 0x8: megadrive_io_write_rx_port(space->machine(),0,data); break;
		case 0x9: megadrive_io_write_sctrl_port(space->machine(),0,data); break;

		case 0xa: megadrive_io_write_tx_port(space->machine(),1,data); break;
		case 0xb: megadrive_io_write_rx_port(space->machine(),1,data); break;
		case 0xc: megadrive_io_write_sctrl_port(space->machine(),1,data); break;

		case 0xd: megadrive_io_write_tx_port(space->machine(),2,data); break;
		case 0xe: megadrive_io_write_rx_port(space->machine(),2,data); break;
		case 0xf: megadrive_io_write_sctrl_port(space->machine(),2,data); break;
	}
}



static ADDRESS_MAP_START( megadriv_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE_LEGACY(megadriv_68k_read_z80_ram,megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE_LEGACY(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8_LEGACY("ymsnd", megadriv_68k_YM2612_read,megadriv_68k_YM2612_write, 0xffff)

	AM_RANGE(0xa06000, 0xa06001) AM_WRITE_LEGACY(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE_LEGACY(megadriv_68k_io_read,megadriv_68k_io_write)

	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE_LEGACY(megadriv_68k_check_z80_bus,megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE_LEGACY(megadriv_68k_req_z80_reset)

	/* these are fake - remove allocs in VIDEO_START to use these to view ram instead */
//  AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE_LEGACY(&megadrive_vdp_vram)
//  AM_RANGE(0xb10000, 0xb1007f) AM_RAM AM_BASE_LEGACY(&megadrive_vdp_vsram)
//  AM_RANGE(0xb10100, 0xb1017f) AM_RAM AM_BASE_LEGACY(&megadrive_vdp_cram)

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega_genesis_vdp_device, megadriv_vdp_r,megadriv_vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_DEVREADWRITE("gen_vdp", sega_genesis_vdp_device, megadriv_vdp_r,megadriv_vdp_w) // the earth defend

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE_LEGACY(&megadrive_ram)
//  AM_RANGE(0xff0000, 0xffffff) AM_READONLY
	/*       0xe00000 - 0xffffff) == MAIN RAM (64kb, Mirrored, most games use ff0000 - ffffff) */
ADDRESS_MAP_END


/* z80 sounds/sub CPU */


static READ16_HANDLER( megadriv_68k_read_z80_ram )
{
	//mame_printf_debug("read z80 ram %04x\n",mem_mask);

	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		return genz80.z80_prgram[(offset<<1)^1] | (genz80.z80_prgram[(offset<<1)]<<8);
	}
	else
	{
		logerror("%06x: 68000 attempting to access Z80 (read) address space without bus\n", cpu_get_pc(&space->device()));
		return space->machine().rand();
	}
}

static WRITE16_HANDLER( megadriv_68k_write_z80_ram )
{
	//logerror("write z80 ram\n");

	if ((genz80.z80_has_bus==0) && (genz80.z80_is_reset==0))
	{

		if (!ACCESSING_BITS_0_7) // byte (MSB) access
		{
			genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
		else if (!ACCESSING_BITS_8_15)
		{
			genz80.z80_prgram[(offset<<1)^1] = (data & 0x00ff);
		}
		else // for WORD access only the MSB is used, LSB is ignored
		{
			genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
	}
	else
	{
		logerror("%06x: 68000 attempting to access Z80 (write) address space without bus\n", cpu_get_pc(&space->device()));
	}
}


static READ16_HANDLER( megadriv_68k_check_z80_bus )
{
	UINT16 retvalue;

	/* Double Dragon, Shadow of the Beast, Super Off Road, and Time Killers have buggy
       sound programs.  They request the bus, then have a loop which waits for the bus
       to be unavailable, checking for a 0 value due to bad coding.  The real hardware
       appears to return bits of the next instruction in the unused bits, thus meaning
       the value is never zero.  Time Killers is the most fussy, and doesn't like the
       read_next_instruction function from system16, so I just return a random value
       in the unused bits */
	UINT16 nextvalue = space->machine().rand();//read_next_instruction(space)&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		//logerror("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", cpu_get_pc(&space->device()),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		//logerror("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", cpu_get_pc(&space->device()),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		//logerror("%06x: 68000 check z80 Bus (word access) %04x\n", cpu_get_pc(&space->device()),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  mame_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", cpu_get_pc(&space->device()),mem_mask, retvalue);
		return retvalue;
	}
}


static TIMER_CALLBACK( megadriv_z80_run_state )
{
	/* Is the z80 RESET line pulled? */
	if ( genz80.z80_is_reset )
	{
		devtag_reset( machine, "genesis_snd_z80" );
		machine.device<cpu_device>( "genesis_snd_z80" )->suspend(SUSPEND_REASON_HALT, 1 );
		devtag_reset( machine, "ymsnd" );
	}
	else
	{
		/* Check if z80 has the bus */
		if ( genz80.z80_has_bus )
		{
			machine.device<cpu_device>( "genesis_snd_z80" )->resume(SUSPEND_REASON_HALT );
		}
		else
		{
			machine.device<cpu_device>( "genesis_snd_z80" )->suspend(SUSPEND_REASON_HALT, 1 );
		}
	}
}


static WRITE16_HANDLER( megadriv_68k_req_z80_bus )
{
	/* Request the Z80 bus, allows 68k to read/write Z80 address space */
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (word access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}

	/* If the z80 is running, sync the z80 execution state */
	if ( ! genz80.z80_is_reset )
		space->machine().scheduler().timer_set( attotime::zero, FUNC(megadriv_z80_run_state ));
}

static WRITE16_HANDLER ( megadriv_68k_req_z80_reset )
{
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (word access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	space->machine().scheduler().timer_set( attotime::zero, FUNC(megadriv_z80_run_state ));
}


// just directly access the 68k space, this makes it easier to deal with
// add-on hardware which changes the cpu mapping like the 32x and SegaCD.
// - we might need to add exceptions for example, z80 reading / writing the
//   z80 area of the 68k if games misbehave
static READ8_HANDLER( z80_read_68k_banked_data )
{
	address_space *space68k = space->machine().device<legacy_cpu_device>("maincpu")->space();
	UINT8 ret = space68k->read_byte(genz80.z80_bank_addr+offset);
	return ret;
}

static WRITE8_HANDLER( z80_write_68k_banked_data )
{
	address_space *space68k = space->machine().device<legacy_cpu_device>("maincpu")->space();
	space68k->write_byte(genz80.z80_bank_addr+offset,data);
}


static WRITE8_HANDLER( megadriv_z80_vdp_write )
{
	switch (offset)
	{
		case 0x11:
		case 0x13:
		case 0x15:
		case 0x17:
			sn76496_w(space->machine().device("snsnd"), 0, data);
			break;

		default:
			mame_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}

}



static READ8_HANDLER( megadriv_z80_vdp_read )
{
	mame_printf_debug("megadriv_z80_vdp_read %02x\n",offset);
	return space->machine().rand();
}

static READ8_HANDLER( megadriv_z80_unmapped_read )
{
	return 0xff;
}

static ADDRESS_MAP_START( megadriv_z80_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1") AM_MIRROR(0x2000) // RAM can be accessed by the 68k
	AM_RANGE(0x4000, 0x4003) AM_DEVREADWRITE_LEGACY("ymsnd", ym2612_r,ym2612_w)

	AM_RANGE(0x6000, 0x6000) AM_WRITE_LEGACY(megadriv_z80_z80_bank_w)
	AM_RANGE(0x6001, 0x6001) AM_WRITE_LEGACY(megadriv_z80_z80_bank_w) // wacky races uses this address

	AM_RANGE(0x6100, 0x7eff) AM_READ_LEGACY(megadriv_z80_unmapped_read)

	AM_RANGE(0x7f00, 0x7fff) AM_READWRITE_LEGACY(megadriv_z80_vdp_read,megadriv_z80_vdp_write)

	AM_RANGE(0x8000, 0xffff) AM_READWRITE_LEGACY(z80_read_68k_banked_data,z80_write_68k_banked_data) // The Z80 can read the 68k address space this way
ADDRESS_MAP_END

static ADDRESS_MAP_START( megadriv_z80_io_map, AS_IO, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0xff) AM_NOP
ADDRESS_MAP_END


/************************************ Megadrive Bootlegs *************************************/

// smaller ROM region because some bootlegs check for RAM there
static ADDRESS_MAP_START( md_bootleg_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM	/* Cartridge Program Rom */
	AM_RANGE(0x200000, 0x2023ff) AM_RAM // tested

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE_LEGACY(megadriv_68k_read_z80_ram, megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE_LEGACY(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8_LEGACY("ymsnd", megadriv_68k_YM2612_read, megadriv_68k_YM2612_write, 0xffff)
	AM_RANGE(0xa06000, 0xa06001) AM_WRITE_LEGACY(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE_LEGACY(megadriv_68k_io_read, megadriv_68k_io_write)
	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE_LEGACY(megadriv_68k_check_z80_bus, megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE_LEGACY(megadriv_68k_req_z80_reset)

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega_genesis_vdp_device, megadriv_vdp_r,megadriv_vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_DEVREADWRITE("gen_vdp", sega_genesis_vdp_device, megadriv_vdp_r,megadriv_vdp_w)

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE_LEGACY(&megadrive_ram)
ADDRESS_MAP_END

MACHINE_CONFIG_DERIVED( md_bootleg, megadriv )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(md_bootleg_map)
MACHINE_CONFIG_END





INPUT_PORTS_START( megdsvp )
	PORT_INCLUDE( megadriv )

	PORT_START("MEMORY_TEST") /* special memtest mode */
	/* Region setting for Console */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

MACHINE_CONFIG_FRAGMENT( md_svp )
	MCFG_CPU_ADD("svp", SSP1601, MASTER_CLOCK_NTSC / 7 * 3) /* ~23 MHz (guessed) */
	MCFG_CPU_PROGRAM_MAP(svp_ssp_map)
	MCFG_CPU_IO_MAP(svp_ext_map)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( megdsvp, megadriv )

	MCFG_CPU_ADD("svp", SSP1601, MASTER_CLOCK_NTSC / 7 * 3) /* ~23 MHz (guessed) */
	MCFG_CPU_PROGRAM_MAP(svp_ssp_map)
	MCFG_CPU_IO_MAP(svp_ext_map)
	/* IRQs are not used by this CPU */
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( megdsvppal, megadpal )

	MCFG_CPU_ADD("svp", SSP1601, MASTER_CLOCK_PAL / 7 * 3) /* ~23 MHz (guessed) */
	MCFG_CPU_PROGRAM_MAP(svp_ssp_map)
	MCFG_CPU_IO_MAP(svp_ext_map)
	/* IRQs are not used by this CPU */
MACHINE_CONFIG_END





SCREEN_UPDATE_RGB32(megadriv)
{
	sega_genesis_vdp_device *vdp = screen.machine().device<sega_genesis_vdp_device>("gen_vdp"); // yuck

	/* Copy our screen buffer here */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32* desty = &bitmap.pix32(y, 0);
		UINT16* srcy;

		if (!vdp->m_use_alt_timing)
		{
			srcy = &vdp->m_render_bitmap->pix(y, 0);
		}
		else
		{
			srcy = vdp->m_render_line;
		}

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 src = srcy[x];
			desty[x] = MAKE_RGB(pal5bit(src >> 10), pal5bit(src >> 5), pal5bit(src >> 0));
		}
	}

	return 0;
}











/*****************************************************************************************/


MACHINE_START( megadriv )
{
	if (megadrive_6buttons_pad)
		init_megadri6_io(machine);
}

MACHINE_RESET( megadriv )
{
	md_base_state *state = machine.driver_data<md_base_state>();

	/* default state of z80 = reset, with bus */
	mame_printf_debug("Resetting Megadrive / Genesis\n");

	if (machine.device("genesis_snd_z80") != NULL)
	{
		genz80.z80_is_reset = 1;
		genz80.z80_has_bus = 1;
		genz80.z80_bank_addr = 0;
		genesis_scanline_counter = -1;
		machine.scheduler().timer_set( attotime::zero, FUNC(megadriv_z80_run_state ));
	}

	megadrive_reset_io(machine);

	if (!state->m_vdp->m_use_alt_timing)
	{
		megadriv_scanline_timer = machine.device<timer_device>("md_scan_timer");
		megadriv_scanline_timer->adjust(attotime::zero);
	}

	if (genesis_other_hacks)
	{
	//  set_refresh_rate(megadriv_framerate);
	//  machine.device("maincpu")->set_clock_scale(0.9950f); /* Fatal Rewind is very fussy... (and doesn't work now anyway, so don't bother with this) */
		if (megadrive_ram)
			memset(megadrive_ram,0x00,0x10000);
	}

	megadriv_reset_vdp(machine);



	/* if any of these extra CPUs exist, pause them until we actually turn them on */
	if (_32x_master_cpu != NULL)
	{
		device_set_input_line(_32x_master_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (_32x_slave_cpu != NULL)
	{
		device_set_input_line(_32x_slave_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (_segacd_68k_cpu != NULL )
	{
		MACHINE_RESET_CALL( segacd );
	}

}

void megadriv_stop_scanline_timer(running_machine &machine)
{
	md_base_state *state = machine.driver_data<md_base_state>();

	if (!state->m_vdp->m_use_alt_timing)
		megadriv_scanline_timer->reset();
}



UINT16* megadriv_backupram;
int megadriv_backupram_length;

static NVRAM_HANDLER( megadriv )
{
	if (megadriv_backupram!=NULL)
	{
		if (read_or_write)
			file->write(megadriv_backupram, megadriv_backupram_length);
		else
		{
			if (file)
			{
				file->read(megadriv_backupram, megadriv_backupram_length);
			}
			else
			{
				int x;
				for (x=0;x<megadriv_backupram_length/2;x++)
					megadriv_backupram[x]=0xffff;//machine.rand(); // dino dini's needs 0xff or game rules are broken
			}
		}
	}
}


// this comes from the VDP on lines 240 (on) 241 (off) and is connected to the z80 irq 0
void genesis_vdp_sndirqline_callback_genesis_z80(running_machine &machine, bool state)
{
	if (machine.device(":genesis_snd_z80") != NULL)
	{
		if (state == true)
		{
			megadriv_z80_hold(machine);
		}
		else if (state == false)
		{
			megadriv_z80_clear(machine);
		}
	}
}

// this comes from the vdp, and is connected to 68k irq level 6 (main vbl interrupt)
void genesis_vdp_lv6irqline_callback_genesis_68k(running_machine &machine, bool state)
{
	if (state==true)
		cputag_set_input_line(machine, "maincpu", 6, HOLD_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 6, CLEAR_LINE);
}

// this comes from the vdp, and is connected to 68k irq level 4 (raster interrupt)
void genesis_vdp_lv4irqline_callback_genesis_68k(running_machine &machine, bool state)
{
	if (state==true)
		cputag_set_input_line(machine, "maincpu", 4, HOLD_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 4, CLEAR_LINE);
}

/* Callback when the 68k takes an IRQ */
static IRQ_CALLBACK(genesis_int_callback)
{
	md_base_state *state = device->machine().driver_data<md_base_state>();

	if (irqline==4)
	{
		state->m_vdp->vdp_clear_irq4_pending();
	}

	if (irqline==6)
	{
		state->m_vdp->vdp_clear_irq6_pending();
	}

	return (0x60+irqline*4)/4; // vector address
}

MACHINE_CONFIG_FRAGMENT( megadriv_timers )
	MCFG_TIMER_ADD("md_scan_timer", megadriv_scanline_timer_callback)
MACHINE_CONFIG_END



MACHINE_CONFIG_FRAGMENT( md_ntsc )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_NTSC / 7) /* 7.67 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_map)
	/* IRQs are handled via the timers */

	MCFG_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_NTSC / 15) /* 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_z80_map)
	MCFG_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MCFG_MACHINE_START(megadriv)
	MCFG_MACHINE_RESET(megadriv)

	MCFG_FRAGMENT_ADD(megadriv_timers)

	MCFG_DEVICE_ADD("gen_vdp", SEGA_GEN_VDP, 0)
	sega_genesis_vdp_device::set_genesis_vdp_sndirqline_callback(*device, genesis_vdp_sndirqline_callback_genesis_z80);
	sega_genesis_vdp_device::set_genesis_vdp_lv6irqline_callback(*device, genesis_vdp_lv6irqline_callback_genesis_68k);
	sega_genesis_vdp_device::set_genesis_vdp_lv4irqline_callback(*device, genesis_vdp_lv4irqline_callback_genesis_68k);



	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 620)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(megadriv) /* Copies a bitmap */
	MCFG_SCREEN_VBLANK_STATIC(megadriv) /* Used to Sync the timing */

	MCFG_TIMER_ADD_SCANLINE("scantimer", megadriv_scanline_timer_callback_alt_timing, "megadriv", 0, 1)

	MCFG_NVRAM_HANDLER(megadriv)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(megadriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7) /* 7.67 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_NTSC/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END

MACHINE_CONFIG_START( megadriv, md_cons_state )
	MCFG_FRAGMENT_ADD(md_ntsc)
MACHINE_CONFIG_END

/************ PAL hardware has a different master clock *************/

MACHINE_CONFIG_FRAGMENT( md_pal )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_PAL / 7) /* 7.67 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_map)
	/* IRQs are handled via the timers */

	MCFG_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_PAL / 15) /* 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_z80_map)
	MCFG_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MCFG_MACHINE_START(megadriv)
	MCFG_MACHINE_RESET(megadriv)

	MCFG_FRAGMENT_ADD(megadriv_timers)

	MCFG_DEVICE_ADD("gen_vdp", SEGA_GEN_VDP, 0)
	sega_genesis_vdp_device::set_genesis_vdp_sndirqline_callback(*device, genesis_vdp_sndirqline_callback_genesis_z80);
	sega_genesis_vdp_device::set_genesis_vdp_lv6irqline_callback(*device, genesis_vdp_lv6irqline_callback_genesis_68k);
	sega_genesis_vdp_device::set_genesis_vdp_lv4irqline_callback(*device, genesis_vdp_lv4irqline_callback_genesis_68k);

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 620)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(megadriv) /* Copies a bitmap */
	MCFG_SCREEN_VBLANK_STATIC(megadriv) /* Used to Sync the timing */

	MCFG_NVRAM_HANDLER(megadriv)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(megadriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_PAL/7) /* 7.67 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_PAL/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END

MACHINE_CONFIG_START( megadpal, md_cons_state )
	MCFG_FRAGMENT_ADD(md_pal)
MACHINE_CONFIG_END




MACHINE_CONFIG_DERIVED( genesis_32x, megadriv )

	MCFG_DEVICE_ADD("sega32x", SEGA_32X_NTSC, 0)

	// we need to remove and re-add the sound system because the balance is different
	// due to MAME / MESS having severe issues if the dac output is > 0.40? (sound is corrupted even if DAC is slient?!)
	MCFG_DEVICE_REMOVE("ymsnd")
	MCFG_DEVICE_REMOVE("snsnd")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7)
	MCFG_SOUND_ROUTE(0, "lspeaker", (0.50)/2)
	MCFG_SOUND_ROUTE(1, "rspeaker", (0.50)/2)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_NTSC/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", (0.25)/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", (0.25)/2)

MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( genesis_32x_pal, megadpal )

	MCFG_DEVICE_ADD("sega32x", SEGA_32X_PAL, 0)

	// we need to remove and re-add the sound system because the balance is different
	// due to MAME / MESS having severe issues if the dac output is > 0.40? (sound is corrupted even if DAC is slient?!)
	MCFG_DEVICE_REMOVE("ymsnd")
	MCFG_DEVICE_REMOVE("snsnd")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7)
	MCFG_SOUND_ROUTE(0, "lspeaker", (0.50)/2)
	MCFG_SOUND_ROUTE(1, "rspeaker", (0.50)/2)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_NTSC/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", (0.25)/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", (0.25)/2)

MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( genesis_scd, megadriv )
	MCFG_NVRAM_HANDLER_CLEAR()
	MCFG_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MCFG_CPU_PROGRAM_MAP(segacd_map)

	MCFG_TIMER_ADD("sw_timer", NULL) //stopwatch timer

	MCFG_DEFAULT_LAYOUT( layout_megacd )

	MCFG_NVRAM_ADD_0FILL("backupram")

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 ) // TODO: accurate volume balance
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	MCFG_SOUND_ADD("rfsnd", RF5C68, SEGACD_CLOCK) // RF5C164!
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	MCFG_TIMER_ADD("scd_dma_timer", scd_dma_timer_callback)

	MCFG_QUANTUM_PERFECT_CPU("segacd_68k") // perfect sync to the fastest cpu
MACHINE_CONFIG_END

struct cdrom_interface scd_cdrom =
{
	"scd_cdrom",
	NULL
};

/* Different Softlists for different regions (for now at least) */
MACHINE_CONFIG_DERIVED( genesis_scd_scd, genesis_scd )
	MCFG_CDROM_ADD( "cdrom",scd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","segacd")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_scd_mcd, genesis_scd )
	MCFG_CDROM_ADD( "cdrom",scd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","megacd")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_scd_mcdj, genesis_scd )
	MCFG_CDROM_ADD( "cdrom",scd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","megacdj")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_32x_scd, genesis_32x )

	MCFG_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MCFG_CPU_PROGRAM_MAP(segacd_map)

	MCFG_TIMER_ADD("sw_timer", NULL) //stopwatch timer
	MCFG_NVRAM_ADD_0FILL("backupram")
	MCFG_TIMER_ADD("scd_dma_timer", scd_dma_timer_callback)

	MCFG_DEFAULT_LAYOUT( layout_megacd )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	MCFG_SOUND_ADD("rfsnd", RF5C68, SEGACD_CLOCK) // RF5C164
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.25 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.25 )

	MCFG_CDROM_ADD( "cdrom", scd_cdrom)
	MCFG_SOFTWARE_LIST_ADD("cd_list","segacd")

	//MCFG_QUANTUM_PERFECT_CPU("32x_master_sh2")
MACHINE_CONFIG_END




static int megadriv_tas_callback(device_t *device)
{
	return 0; // writeback not allowed
}

static void megadriv_init_common(running_machine &machine)
{
	/* Look to see if this system has the standard Sound Z80 */
	_genesis_snd_z80_cpu = machine.device<cpu_device>("genesis_snd_z80");
	if (_genesis_snd_z80_cpu != NULL)
	{
		//printf("GENESIS Sound Z80 cpu found '%s'\n", _genesis_snd_z80_cpu->tag() );

		genz80.z80_prgram = auto_alloc_array(machine, UINT8, 0x2000);
		machine.root_device().membank("bank1")->set_base(genz80.z80_prgram );
	}

	/* Look to see if this system has the 32x Master SH2 */
	_32x_master_cpu = machine.device<cpu_device>(_32X_MASTER_TAG);
	if (_32x_master_cpu != NULL)
	{
		printf("32x MASTER SH2 cpu found '%s'\n", _32x_master_cpu->tag() );
	}

	/* Look to see if this system has the 32x Slave SH2 */
	_32x_slave_cpu = machine.device<cpu_device>(_32X_SLAVE_TAG);
	if (_32x_slave_cpu != NULL)
	{
		printf("32x SLAVE SH2 cpu found '%s'\n", _32x_slave_cpu->tag() );
	}



	sega_cd_connected = 0;
	segacd_wordram_mapped = 0;
	_segacd_68k_cpu = machine.device<cpu_device>("segacd_68k");
	if (_segacd_68k_cpu != NULL)
	{
		printf("Sega CD secondary 68k cpu found '%s'\n", _segacd_68k_cpu->tag() );
		sega_cd_connected = 1;
		segacd_init_main_cpu(machine);
		scd_dma_timer = machine.device<timer_device>("scd_dma_timer");

	}

	_svp_cpu = machine.device<cpu_device>("svp");
	if (_svp_cpu != NULL)
	{
		printf("SVP (cpu) found '%s'\n", _svp_cpu->tag() );
	}

	device_set_irq_callback(machine.device("maincpu"), genesis_int_callback);
	megadriv_backupram = NULL;
	megadriv_backupram_length = 0;

	vdp_get_word_from_68k_mem = vdp_get_word_from_68k_mem_default;

	m68k_set_tas_callback(machine.device("maincpu"), megadriv_tas_callback);

	// the drivers which need 6 buttons pad set this to 1 in their init befare calling the megadrive init
	if (megadrive_6buttons_pad)
	{
		megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_6button;
		megadrive_io_write_data_port_ptr = megadrive_io_write_data_port_6button;
		mame_printf_debug("6 button game\n");
	}
	else
	{
		megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_3button;
		megadrive_io_write_data_port_ptr = megadrive_io_write_data_port_3button;
		mame_printf_debug("3 button game\n");
	}

	{
		/* only really useful on official games, ea games etc. don't bother
          some games specify a single address, (start 200001, end 200001)
          this usually means there is serial eeprom instead */
		int i;
		UINT16 *rom = (UINT16*)machine.root_device().memregion("maincpu")->base();

		mame_printf_debug("DEBUG:: Header: Backup RAM string (ignore for games without)\n");
		for (i=0;i<12;i++)
		{
			if (i==2) mame_printf_debug("\nstart: ");
			if (i==4) mame_printf_debug("\nend  : ");
			if (i==6) mame_printf_debug("\n");

			mame_printf_debug("%04x ",rom[(0x1b0/2)+i]);
		}
		mame_printf_debug("\n");
	}

	/* if we have an SVP cpu then do some extra initilization for it */
	if (_svp_cpu != NULL)
	{
		svp_init(machine);
	}


}

DRIVER_INIT_MEMBER(md_base_state,megadriv_c2)
{
	genvdp_use_cram = 0;
	genesis_other_hacks = 0;

	megadriv_init_common(machine());
	megadriv_framerate = 60;
}



DRIVER_INIT_MEMBER(md_base_state,megadriv)
{
	genvdp_use_cram = 1;
	genesis_other_hacks = 1;

	megadriv_init_common(machine());
	megadriv_framerate = 60;
}

DRIVER_INIT_MEMBER(md_base_state,megadrij)
{
	genvdp_use_cram = 1;
	genesis_other_hacks = 1;

	megadriv_init_common(machine());
	megadriv_framerate = 60;
}

DRIVER_INIT_MEMBER(md_base_state,megadrie)
{
	genvdp_use_cram = 1;
	genesis_other_hacks = 1;

	megadriv_init_common(machine());
	megadriv_framerate = 50;
}

DRIVER_INIT_MEMBER(md_base_state,mpnew)
{
	DRIVER_INIT_CALL(megadrij);
	megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_3button;
	megadrive_io_write_data_port_ptr = megadrive_io_write_data_port_3button;
}

/* used by megatech */
static READ8_HANDLER( z80_unmapped_port_r )
{
//  printf("unmapped z80 port read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_port_w )
{
//  printf("unmapped z80 port write %04x\n",offset);
}

static READ8_HANDLER( z80_unmapped_r )
{
	printf("unmapped z80 read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_w )
{
	printf("unmapped z80 write %04x\n",offset);
}


/* sets the megadrive z80 to it's normal ports / map */
void megatech_set_megadrive_z80_as_megadrive_z80(running_machine &machine, const char* tag)
{
	device_t *ym = machine.device("ymsnd");

	/* INIT THE PORTS *********************************************************************************************/
	machine.device(tag)->memory().space(AS_IO)->install_legacy_readwrite_handler(0x0000, 0xffff, FUNC(z80_unmapped_port_r), FUNC(z80_unmapped_port_w));

	/* catch any addresses that don't get mapped */
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x0000, 0xffff, FUNC(z80_unmapped_r), FUNC(z80_unmapped_w));


	machine.device(tag)->memory().space(AS_PROGRAM)->install_readwrite_bank(0x0000, 0x1fff, "bank1");
	machine.root_device().membank("bank1")->set_base(genz80.z80_prgram );

	machine.device(tag)->memory().space(AS_PROGRAM)->install_ram(0x0000, 0x1fff, genz80.z80_prgram);


	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(*ym, 0x4000, 0x4003, FUNC(ym2612_r), FUNC(ym2612_w));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_write_handler    (0x6000, 0x6000, FUNC(megadriv_z80_z80_bank_w));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_write_handler    (0x6001, 0x6001, FUNC(megadriv_z80_z80_bank_w));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_read_handler     (0x6100, 0x7eff, FUNC(megadriv_z80_unmapped_read));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x7f00, 0x7fff, FUNC(megadriv_z80_vdp_read), FUNC(megadriv_z80_vdp_write));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x8000, 0xffff, FUNC(z80_read_68k_banked_data), FUNC(z80_write_68k_banked_data));
}






SCREEN_VBLANK(megadriv)
{
	md_base_state *state = screen.machine().driver_data<md_base_state>();

	if (screen.machine().root_device().ioport(":RESET")->read_safe(0x00) & 0x01)
		cputag_set_input_line(screen.machine(), ":maincpu", INPUT_LINE_RESET, PULSE_LINE);

	// rising edge
	if (vblank_on)
	{
		if (!state->m_vdp->m_use_alt_timing)
		{
			state->m_vdp->vdp_handle_eof(screen.machine());
			megadriv_scanline_timer->adjust(attotime::zero);
		}
	}
}
