
#include "emu.h"
#include "includes/megadriv.h"

class pico_state : public md_cons_state
{
public:
	pico_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_cons_state(mconfig, type, tag) { }

	UINT8 m_page_register;
};

/*************************************
 *
 *  Input handlers
 *
 *************************************/

/* We need to always initialize 6 buttons pad */
static TIMER_CALLBACK( mess_io_timeout_timer_callback )
{
	md_cons_state *state = machine.driver_data<md_cons_state>();
	state->m_mess_io_stage[(int)(FPTR)ptr] = -1;
}

/* J-Cart controller port */
WRITE16_HANDLER( jcart_ctrl_w )
{
	md_cons_state *state = space.machine().driver_data<md_cons_state>();
	state->m_jcart_io_data[0] = (data & 1) << 6;
	state->m_jcart_io_data[1] = (data & 1) << 6;
}

READ16_HANDLER( jcart_ctrl_r )
{
	md_cons_state *state = space.machine().driver_data<md_cons_state>();
	UINT16 retdata = 0;
	UINT8 joy[2];

	if (state->m_jcart_io_data[0] & 0x40)
	{
		joy[0] = state->ioport("JCART3_3B")->read_safe(0);
		joy[1] = state->ioport("JCART4_3B")->read_safe(0);
		retdata = (state->m_jcart_io_data[0] & 0x40) | joy[0] | (joy[1] << 8);
	}
	else
	{
		joy[0] = ((state->ioport("JCART3_3B")->read_safe(0) & 0xc0) >> 2) |
		  (state->ioport("JCART3_3B")->read_safe(0) & 0x03);
		joy[1] = ((state->ioport("JCART4_3B")->read_safe(0) & 0xc0) >> 2) |
		  (state->ioport("JCART4_3B")->read_safe(0) & 0x03);
		retdata = (state->m_jcart_io_data[0] & 0x40) | joy[0] | (joy[1] << 8);
	}
	return retdata;
}

static void mess_init_6buttons_pad(running_machine &machine)
{
	md_cons_state *state = machine.driver_data<md_cons_state>();
	int i;

	for (i = 0; i < 3; i++)
	{
		state->m_mess_io_timeout[i] = machine.scheduler().timer_alloc(FUNC(mess_io_timeout_timer_callback), (void*)(FPTR)i);
		state->m_mess_io_stage[i] = -1;
	}
}

/* These overwrite the MAME ones in DRIVER_INIT */
/* They're needed to give the users the choice between different controllers */
static UINT8 mess_md_io_read_data_port(running_machine &machine, int portnum)
{
	md_cons_state *state = machine.driver_data<md_cons_state>();
	static const char *const pad6names[2][4] = {
		{ "PAD1_6B", "PAD2_6B", "UNUSED", "UNUSED" },
		{ "EXTRA1", "EXTRA2", "UNUSED", "UNUSED" }
	};
	static const char *const pad3names[4] = { "PAD1_3B", "PAD2_3B", "UNUSED", "UNUSED" };

	UINT8 retdata;
	int controller;
	UINT8 helper_6b = (megadrive_io_ctrl_regs[portnum] & 0x3f) | 0xc0; // bits 6 & 7 always come from megadrive_io_data_regs
	UINT8 helper_3b = (megadrive_io_ctrl_regs[portnum] & 0x7f) | 0x80; // bit 7 always comes from megadrive_io_data_regs

	switch (portnum)
	{
		case 0:
			controller = (machine.root_device().ioport("CTRLSEL")->read() & 0x0f);
			break;

		case 1:
			controller = (machine.root_device().ioport("CTRLSEL")->read() & 0xf0);
			break;

		default:
			controller = 0;
			break;
	}

	/* Are we using a 6 buttons Joypad? */
	if (controller)
	{
		if (megadrive_io_data_regs[portnum] & 0x40)
		{
			if (state->m_mess_io_stage[portnum] == 2)
			{
				/* here we read B, C & the additional buttons */
				retdata = (megadrive_io_data_regs[portnum] & helper_6b) |
							(((state->ioport(pad6names[0][portnum])->read_safe(0) & 0x30) |
								(state->ioport(pad6names[1][portnum])->read_safe(0) & 0x0f)) & ~helper_6b);
			}
			else
			{
				/* here we read B, C & the directional buttons */
				retdata = (megadrive_io_data_regs[portnum] & helper_6b) |
							((state->ioport(pad6names[0][portnum])->read_safe(0) & 0x3f) & ~helper_6b);
			}
		}
		else
		{
			if (state->m_mess_io_stage[portnum] == 1)
			{
				/* here we read ((Start & A) >> 2) | 0x00 */
				retdata = (megadrive_io_data_regs[portnum] & helper_6b) |
							(((state->ioport(pad6names[0][portnum])->read_safe(0) & 0xc0) >> 2) & ~helper_6b);
			}
			else if (state->m_mess_io_stage[portnum]==2)
			{
				/* here we read ((Start & A) >> 2) | 0x0f */
				retdata = (megadrive_io_data_regs[portnum] & helper_6b) |
							((((state->ioport(pad6names[0][portnum])->read_safe(0) & 0xc0) >> 2) | 0x0f) & ~helper_6b);
			}
			else
			{
				/* here we read ((Start & A) >> 2) | Up and Down */
				retdata = (megadrive_io_data_regs[portnum] & helper_6b) |
							((((state->ioport(pad6names[0][portnum])->read_safe(0) & 0xc0) >> 2) |
								(state->ioport(pad6names[0][portnum])->read_safe(0) & 0x03)) & ~helper_6b);
			}
		}

	//  mame_printf_debug("read io data port stage %d port %d %02x\n",mess_io_stage[portnum],portnum,retdata);

		retdata |= (retdata << 8);
	}
	/* Otherwise it's a 3 buttons Joypad */
	else
	{
		if (megadrive_io_data_regs[portnum] & 0x40)
		{
			/* here we read B, C & the directional buttons */
			retdata = (megadrive_io_data_regs[portnum] & helper_3b) |
						(((state->ioport(pad3names[portnum])->read_safe(0) & 0x3f) | 0x40) & ~helper_3b);
		}
		else
		{
			/* here we read ((Start & A) >> 2) | Up and Down */
			retdata = (megadrive_io_data_regs[portnum] & helper_3b) |
						((((state->ioport(pad3names[portnum])->read_safe(0) & 0xc0) >> 2) |
							(state->ioport(pad3names[portnum])->read_safe(0) & 0x03) | 0x40) & ~helper_3b);
		}
	}

	return retdata;
}


static void mess_md_io_write_data_port(running_machine &machine, int portnum, UINT16 data)
{
	md_cons_state *state = machine.driver_data<md_cons_state>();
	int controller;

	switch (portnum)
	{
		case 0:
			controller = (machine.root_device().ioport("CTRLSEL")->read() & 0x0f);
			break;

		case 1:
			controller = (machine.root_device().ioport("CTRLSEL")->read() & 0xf0);
			break;

		default:
			controller = 0;
			break;
	}

	if (controller)
	{
		if (megadrive_io_ctrl_regs[portnum] & 0x40)
		{
			if (((megadrive_io_data_regs[portnum] & 0x40) == 0x00) && ((data & 0x40) == 0x40))
			{
				state->m_mess_io_stage[portnum]++;
				state->m_mess_io_timeout[portnum]->adjust(machine.device<cpu_device>("maincpu")->cycles_to_attotime(8192));
			}

		}
	}
	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);
}


/*************************************
 *
 *  Input ports
 *
 *************************************/


static INPUT_PORTS_START( md )
	PORT_START("CTRLSEL")	/* Controller selection */
	PORT_CONFNAME( 0x0f, 0x00, "Player 1 Controller" )
	PORT_CONFSETTING( 0x00, "Joystick 3 Buttons" )
	PORT_CONFSETTING( 0x01, "Joystick 6 Buttons" )
//  PORT_CONFSETTING( 0x02, "Sega Mouse" )
/* there exists both a 2 buttons version of the Mouse (Jpn ver, to be used with RPGs, it
    can aslo be used as trackball) and a 3 buttons version (US ver, no trackball feats.) */
//  PORT_CONFSETTING( 0x03, "Sega Menacer" )
//  PORT_CONFSETTING( 0x04, "Konami Justifier" )
//  PORT_CONFSETTING( 0x05, "Team Player (Sega Multitap)" )
//  PORT_CONFSETTING( 0x06, "4-Play (EA Multitap)" )
//  PORT_CONFSETTING( 0x07, "J-Cart" )
	PORT_CONFNAME( 0xf0, 0x00, "Player 2 Controller" )
	PORT_CONFSETTING( 0x00, "Joystick 3 Buttons" )
	PORT_CONFSETTING( 0x10, "Joystick 6 Buttons" )
	PORT_CONFNAME( 0xf00, 0x00, "Player 3 Controller (J-Cart)" )
	PORT_CONFSETTING( 0x00, "Joystick 3 Buttons" )
	PORT_CONFNAME( 0xf000, 0x00, "Player 4 Controller (J-Cart)" )
	PORT_CONFSETTING( 0x00, "Joystick 3 Buttons" )

	PORT_START("PAD1_3B")		/* Joypad 1 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)

	PORT_START("PAD2_3B")		/* Joypad 2 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)

	PORT_START("JCART3_3B")		/* Joypad 3 on J-Cart (3 button + start) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 B") PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 C") PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 A") PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf00, EQUALS, 0x00)

	PORT_START("JCART4_3B")		/* Joypad 4 on J-Cart (3 button + start) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 B") PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 C") PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 A") PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf000, EQUALS, 0x00)

	PORT_START("PAD1_6B")		/* Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)

	PORT_START("EXTRA1")	/* Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Z") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Y") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 X") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Mode") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)

	PORT_START("PAD2_6B")		/* Joypad 2 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)

	PORT_START("EXTRA2")	/* Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Z") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Y") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 X") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Mode") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)

	PORT_START("RESET")		/* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( ms_megadriv )
{
	mess_init_6buttons_pad(machine);
	MACHINE_START_CALL( md_sram );
}

static MACHINE_RESET( ms_megadriv )
{
	MACHINE_RESET_CALL( megadriv );
}

static MACHINE_CONFIG_START( ms_megadriv, md_cons_state )
	MCFG_FRAGMENT_ADD( md_ntsc )
	MCFG_NVRAM_HANDLER_CLEAR()

	MCFG_MACHINE_START( ms_megadriv )
	MCFG_MACHINE_RESET( ms_megadriv )

	MCFG_FRAGMENT_ADD( genesis_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ms_megadpal, md_cons_state )
	MCFG_FRAGMENT_ADD( md_pal )
	MCFG_NVRAM_HANDLER_CLEAR()

	MCFG_MACHINE_START( ms_megadriv )
	MCFG_MACHINE_RESET( ms_megadriv )

	MCFG_FRAGMENT_ADD( genesis_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ms_megdsvp, mdsvp_state )
	MCFG_FRAGMENT_ADD( md_ntsc )
	MCFG_FRAGMENT_ADD( md_svp )
	MCFG_NVRAM_HANDLER_CLEAR()

	MCFG_MACHINE_START( ms_megadriv )
	MCFG_MACHINE_RESET( ms_megadriv )

	MCFG_FRAGMENT_ADD( genesis_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ms_megdsvppal, mdsvp_state )
	MCFG_FRAGMENT_ADD( md_pal )
	MCFG_FRAGMENT_ADD( md_svp )
	MCFG_NVRAM_HANDLER_CLEAR()

	MCFG_MACHINE_START( ms_megadriv )
	MCFG_MACHINE_RESET( ms_megadriv )

	MCFG_FRAGMENT_ADD( genesis_cartslot )
MACHINE_CONFIG_END




/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


/* we don't use the bios rom (it's not needed and only provides security on early models) */

ROM_START(genesis)
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(megadriv)
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(megadrij)
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END


ROM_START(gensvp)
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(mdsvp)
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(mdsvpj)
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(md_cons_state,mess_md_common)
{
	megadrive_io_read_data_port_ptr = mess_md_io_read_data_port;
	megadrive_io_write_data_port_ptr = mess_md_io_write_data_port;
}

DRIVER_INIT_MEMBER(md_cons_state,genesis)
{
	DRIVER_INIT_CALL(megadriv);
	DRIVER_INIT_CALL(mess_md_common);
	megadrive_region_export = 1;
	megadrive_region_pal = 0;
}

DRIVER_INIT_MEMBER(md_cons_state,md_eur)
{
	DRIVER_INIT_CALL(megadrie);
	DRIVER_INIT_CALL(mess_md_common);
	megadrive_region_export = 1;
	megadrive_region_pal = 1;
}

DRIVER_INIT_MEMBER(md_cons_state,md_jpn)
{
	DRIVER_INIT_CALL(megadrij);
	DRIVER_INIT_CALL(mess_md_common);
	megadrive_region_export = 0;
	megadrive_region_pal = 0;
}

/****************************************** SegaCD & 32X emulation ****************************************/


static MACHINE_CONFIG_DERIVED( ms_32x, genesis_32x )
	MCFG_FRAGMENT_ADD( _32x_cartslot )
	MCFG_NVRAM_HANDLER_CLEAR()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ms_32x_pal, genesis_32x_pal )
	MCFG_FRAGMENT_ADD( _32x_cartslot )
	MCFG_NVRAM_HANDLER_CLEAR()
MACHINE_CONFIG_END

#define _32X_ROMS \
	ROM_REGION16_BE( 0x400000, "gamecart", ROMREGION_ERASE00 ) /* 68000 Code */ \
	ROM_REGION32_BE( 0x400000, "gamecart_sh2", ROMREGION_ERASE00 ) /* Copy for the SH2 */ \
	ROM_REGION16_BE( 0x400000, "32x_68k_bios", 0 ) /* 68000 Code */ \
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) ) \
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 ) \
	/* temp, rom should only be visible here when one of the regs is set, tempo needs it */ \
	/* ROM_CART_LOAD("cart", 0x000000, 0x400000, ROM_NOMIRROR) */ \
	ROM_COPY( "32x_68k_bios", 0x0, 0x0, 0x100) \
	ROM_REGION32_BE( 0x400000, "master", 0 ) /* SH2 Code */ \
	ROM_SYSTEM_BIOS( 0, "retail", "Mars Version 1.0 (retail)" ) \
	ROMX_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 1, "sdk", "Mars Version 1.0 (early sdk)" ) \
	ROMX_LOAD( "32x_m_bios_sdk.bin", 0x000000,  0x000800, BAD_DUMP CRC(c7102c53) SHA1(ed73a47f186b373b8eff765f84ef26c3d9ef6cb0), ROM_BIOS(2) ) \
	ROM_REGION32_BE( 0x400000, "slave", 0 ) /* SH2 Code */ \
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) ) \


ROM_START( 32x )
	_32X_ROMS
ROM_END

ROM_START( 32xe )
	_32X_ROMS
ROM_END

ROM_START( 32xj )
	_32X_ROMS
ROM_END

/* We need proper names for most of these BIOS ROMs! */
ROM_START( segacd )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v1.10 confirmed dump by dead_screem */
	ROM_LOAD( "mpr-15045b.bin", 0x000000,  0x020000, CRC(c6d10268) SHA1(f4f315adcef9b8feb0364c21ab7f0eaf5457f3ed) )
ROM_END

ROM_START( megacd )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v1.00, confirmed good dump */
	ROM_LOAD( "megacd_model1_bios_1_00_e.bin", 0x000000,  0x020000, CRC(529ac15a) SHA1(f891e0ea651e2232af0c5c4cb46a0cae2ee8f356) )
ROM_END

ROM_START( megacdj )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* Confirmed by ElBarto */
	ROM_SYSTEM_BIOS(0, "v100s", "v1.00S")
	ROMX_LOAD( "mpr-14088h.bin", 0x000000,  0x020000, CRC(3773d5aa) SHA1(bbf729a1aaa1667b783749299e1ad932aaf5f253), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE)
	/* Confirmed by ElBarto */
	ROM_SYSTEM_BIOS(1, "v100g", "v1.00g")
	ROMX_LOAD( "epr-14088b.bin", 0x000000,  0x020000, CRC(69ed6ccd) SHA1(27d11c3836506f01ee81cd142c0cd8b51abebbd2), ROM_BIOS(2) | ROM_GROUPWORD | ROM_REVERSE)
	/* Confirmed by ElBarto */
	ROM_SYSTEM_BIOS(2, "v100l", "v1.00L")
	ROMX_LOAD( "mpr-14088c.bin", 0x000000,  0x020000, CRC(03134289) SHA1(d60cb5a53f26d6b13e354bc149217587f2301718), ROM_BIOS(3) | ROM_GROUPWORD | ROM_REVERSE)
	/* Confirmed by ElBarto */
	ROM_SYSTEM_BIOS(3, "v100o", "v1.00o")
	ROMX_LOAD( "epr-14088d.bin", 0x000000,  0x020000, CRC(dfa95ee9) SHA1(e13666c76fa0a2e94e2f651b26b0fd625bf55f07), ROM_BIOS(4) | ROM_GROUPWORD | ROM_REVERSE)
	ROM_SYSTEM_BIOS(4, "v100p", "v1.00P")
	ROMX_LOAD( "megacd_model1_bios_1_00p_j.bin", 0x000000,  0x020000, CRC(9d2da8f2) SHA1(4846f448160059a7da0215a5df12ca160f26dd69), ROM_BIOS(5) )
ROM_END

/* Asia bios, when run in USA region will show :
ERROR!
THIS IS A PAL-COMPATIBLE MEGA CD
FOR EXCLUSIVE USE IN SOUTHEAST ASIA.

Confirmed by Jakovasaur
*/
ROM_START( megacda )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "epr-14536h.bin", 0x000000,  0x020000, CRC(550f30bb) SHA1(e4193c6ae44c3cea002707d2a88f1fbcced664de))
ROM_END

ROM_START( segacd2 )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_SYSTEM_BIOS(0, "v211x", "Model 2 v2.11X")
	ROMX_LOAD( "mpr-15764-t.bin", 0x000000,  0x020000, CRC(2e49d72c) SHA1(328a3228c29fba244b9db2055adc1ec4f7a87e6b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "v200", "Model 2 v2.00") /* verified dump */
	ROMX_LOAD( "us_scd2_930314.bin", 0x000000,  0x020000, CRC(8af65f58) SHA1(5a8c4b91d3034c1448aac4b5dc9a6484fce51636), ROM_BIOS(2) )
	/* this is reportedly a bad dump, it has many differences from the verified dump and does not boot in Kega */
	/* ROMX_LOAD( "segacd_model2_bios_2_00_u.bin", 0x000000,  0x020000, CRC(340b4be4) SHA1(bd3ee0c8ab732468748bf98953603ce772612704), ROM_BIOS(2) ) */
	ROM_SYSTEM_BIOS(2, "v200w", "Model 2 v2.00W")
	ROMX_LOAD( "segacd_model2_bios_2_00w_u.bin", 0x000000,  0x020000, CRC(9f6f6276) SHA1(5adb6c3af218c60868e6b723ec47e36bbdf5e6f0), ROM_BIOS(3) )
ROM_END

/* All confirmed good dump by ElBarto */
ROM_START( megacd2 )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_SYSTEM_BIOS(0, "v200w", "v2.00W")
	ROMX_LOAD( "mpr-15512a.bin", 0x000000,  0x020000, CRC(53f1757c) SHA1(67bf3970ca5a05fd5ce3d6c446789c5d971b98a4), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )
	ROM_SYSTEM_BIOS(1, "v200", "v2.00")
	ROMX_LOAD( "mpr-15512.bin", 0x000000,  0x020000, CRC(cb76f114) SHA1(939f173cadc41e996a3c34498da1bf55e7e18ff8), ROM_BIOS(2) | ROM_GROUPWORD | ROM_REVERSE )
ROM_END

/* Confirmed good dump by ElBarto */
ROM_START( megacd2j )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_SYSTEM_BIOS(0, "v200c", "v2.00C")
	ROMX_LOAD( "mpr-15398.bin", 0x000000,  0x020000, CRC(1e4344e6) SHA1(4d1251a6973d932e734ae5e8c6b9b55eb40e4143), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )
ROM_END

ROM_START( laseract )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_SYSTEM_BIOS(0, "v104", "v1.04")
	ROMX_LOAD( "laseractive_bios_1_04_u.bin", 0x000000,  0x020000, CRC(50cd3d23) SHA1(aa811861f8874775075bd3f53008c8aaf59b07db), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "v102", "v1.02")
	ROMX_LOAD( "laseractive_bios_1_02_u.bin", 0x000000,  0x020000, CRC(3b10cf41) SHA1(8af162223bb12fc19b414f126022910372790103), ROM_BIOS(2) )
ROM_END

ROM_START( laseractj )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v1.02 */
	ROM_LOAD( "laseractive_bios_1_02_j.bin", 0x000000,  0x020000, CRC(00eedb3a) SHA1(26237b333db4a4c6770297fa5e655ea95840d5d9) )
ROM_END

ROM_START( xeye )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v2.00 (US), confirmed good with a chip dump */
	ROM_LOAD( "g304.bin", 0x000000,  0x020000, CRC(290f8e33) SHA1(651f14d5a5e0ecb974a60c0f43b1d2006323fb09) )
ROM_END

ROM_START( wmega )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v1.00 (Japan NTSC) Sega BIOS, chip-dumped */
	ROM_LOAD( "g301.bin", 0x000000,  0x020000, CRC(d21fe71d) SHA1(3fc9358072f74bd24e3e297ea11b2bf15a7af891) )
ROM_END

ROM_START( wmegam2 )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v2.00 */
	ROM_LOAD( "wondermega_m2_bios_2_00_j.bin", 0x000000,  0x020000, CRC(2b19972f) SHA1(b3f32e409bd5508c89ed8be33d41a58d791d0e5d) )
ROM_END

ROM_START( cdx )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v2.21X */
	ROM_LOAD( "segacdx_bios_2_21_u.bin", 0x000000,  0x020000, CRC(d48c44b5) SHA1(2b125c0545afa089b617f2558e686ea723bdc06e) )
ROM_END

ROM_START( multmega )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	/* v2.21X */
	ROM_LOAD( "opr-16140.bin", 0x000000,  0x020000, CRC(aacb851e) SHA1(75548ac9aaa6e81224499f9a1403b2b42433f5b7) )
	/* the below was marked "confirmed good dump", but 0x72 and 0x73 are 0xFF, indicating a bad dump made from memory */
	/* ROM_LOAD( "multimega_bios_2_21_e.bin", 0x000000,  0x020000, CRC(34d3cce1) SHA1(73fc9c014ad803e9e7d8076b3642a8a5224b3e51) ) */
ROM_END

/* some games use the 32x and SegaCD together to give better quality FMV */
ROM_START( 32x_scd )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x400000, "gamecart", 0 ) /* 68000 Code */
	ROM_LOAD( "mpr-15764-t.bin", 0x000000,  0x020000, CRC(2e49d72c) SHA1(328a3228c29fba244b9db2055adc1ec4f7a87e6b) )

	ROM_REGION32_BE( 0x400000, "gamecart_sh2", 0 ) /* Copy for the SH2 */
	ROM_COPY( "gamecart", 0x0, 0x0, 0x400000)

	ROM_REGION16_BE( 0x400000, "32x_68k_bios", 0 ) /* 68000 Code */
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION32_BE( 0x400000, "master", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION32_BE( 0x400000, "slave", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END


/****************************************** PICO emulation ****************************************/

/*
   Pico Implementation By ElBarto (Emmanuel Vadot, elbarto@megadrive.org)
   Still missing the PCM custom chip
   Some game will not boot due to this

 Pico Info from Notaz (http://notaz.gp2x.de/docs/picodoc.txt)

 addr   acc   description
-------+-----+------------
800001  byte  Version register.
              ?vv? ????, where v can be:
                00 - hardware is for Japan
                01 - European version
                10 - USA version
                11 - ?
800003  byte  Buttons, 0 for pressed, 1 for released:
                bit 0: UP (white)
                bit 1: DOWN (orange)
                bit 2: LEFT (blue)
                bit 3: RIGHT (green)
                bit 4: red button
                bit 5: unused?
                bit 6: unused?
                bit 7: pen button
800005  byte  Most significant byte of pen x coordinate.
800007  byte  Least significant byte of pen x coordinate.
800009  byte  Most significant byte of pen y coordinate.
80000b  byte  Least significant byte of pen y coordinate.
80000d  byte  Page register. One bit means one uncovered page sensor.
                00 - storyware closed
                01, 03, 07, 0f, 1f, 3f - pages 1-6
                either page 5 or page 6 is often unused.
800010  word  PCM data register.
        r/w   read returns free bytes left in PCM FIFO buffer
              writes write data to buffer.
800012  word  PCM control register.
        r/w   For writes, it has following possible meanings:
              ?p?? ???? ???? ?rrr
                p - set to enable playback?
                r - sample rate / PCM data type?
                  0: 8kHz 4bit ADPCM?
                  1-7: 16kHz variants?
              For reads, if bit 15 is cleared, it means PCM is 'busy' or
              something like that, as games sometimes wait for it to become 1.
800019  byte  Games write 'S'
80001b  byte  Games write 'E'
80001d  byte  Games write 'G'
80001f  byte  Games write 'A'

*/

#define PICO_PENX	1
#define PICO_PENY	2

static UINT16 pico_read_penpos(running_machine &machine, int pen)
{
	UINT16 penpos = 0;

	switch (pen)
    {
		case PICO_PENX:
			penpos = machine.root_device().ioport("PENX")->read_safe(0);
			penpos |= 0x6;
			penpos = penpos * 320 / 255;
			penpos += 0x3d;
			break;
		case PICO_PENY:
			penpos = machine.root_device().ioport("PENY")->read_safe(0);
			penpos |= 0x6;
			penpos = penpos * 251 / 255;
			penpos += 0x1fc;
			break;
    }

	return penpos;
}

static READ16_HANDLER( pico_68k_io_read )
{
	pico_state *state = space.machine().driver_data<pico_state>();
	UINT8 retdata = 0;

	switch (offset)
	{
		case 0:	/* Version register ?XX?????? where XX is 00 for japan, 01 for europe and 10 for USA*/
			retdata = (megadrive_region_export << 6) | (megadrive_region_pal << 5);
			break;
		case 1:
			retdata = state->ioport("PAD")->read_safe(0);
			break;

			/*
           Still notes from notaz for the pen :

           The pen can be used to 'draw' either on the drawing pad or on the storyware
           itself. Both storyware and drawing pad are mapped on single virtual plane, where
           coordinates range:

           x: 0x03c - 0x17c
           y: 0x1fc - 0x2f7 (drawing pad)
              0x2f8 - 0x3f3 (storyware)
           */
		case 2:
			retdata = pico_read_penpos(space.machine(), PICO_PENX) >> 8;
			break;
		case 3:
			retdata = pico_read_penpos(space.machine(), PICO_PENX) & 0x00ff;
			break;
		case 4:
			retdata = pico_read_penpos(space.machine(), PICO_PENY) >> 8;
			break;
		case 5:
			retdata = pico_read_penpos(space.machine(), PICO_PENY) & 0x00ff;
			break;
		case 6:
	    /* Page register :
           00 - storyware closed
           01, 03, 07, 0f, 1f, 3f - pages 1-6
           either page 5 or page 6 is often unused.
        */
			{
				UINT8 tmp = state->ioport("PAGE")->read_safe(0);
				if (tmp == 2 && state->m_page_register != 0x3f)
				{
					state->m_page_register <<= 1;
					state->m_page_register |= 1;
				}
				if (tmp == 1 && state->m_page_register != 0x00)
					state->m_page_register >>= 1;
				retdata = state->m_page_register;
				break;
			}
		case 7:
			/* Returns free bytes left in the PCM FIFO buffer */
			retdata = 0x00;
			break;
		case 8:
	    /*
           For reads, if bit 15 is cleared, it means PCM is 'busy' or
           something like that, as games sometimes wait for it to become 1.
        */
			retdata = 0x00;
	}

	return retdata | retdata << 8;
}

static WRITE16_HANDLER( pico_68k_io_write )
{
	switch (offset)
	{
	}
}

static ADDRESS_MAP_START( pico_mem, AS_PROGRAM, 16, pico_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM

	AM_RANGE(0x800000, 0x80001f) AM_READWRITE_LEGACY(pico_68k_io_read, pico_68k_io_write)

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega_genesis_vdp_device, megadriv_vdp_r,megadriv_vdp_w)
	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000)
ADDRESS_MAP_END


static INPUT_PORTS_START( pico )
	PORT_START("PAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Red Button")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Pen Button")

	PORT_START("PAGE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Increment Page")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Decrement Page")

	PORT_START("PENX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("PEN X")

	PORT_START("PENY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0,255 ) PORT_PLAYER(1) PORT_NAME("PEN Y")
INPUT_PORTS_END


static MACHINE_CONFIG_START( pico, pico_state )
	MCFG_FRAGMENT_ADD( md_ntsc )
	MCFG_NVRAM_HANDLER_CLEAR()

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pico_mem)

	MCFG_DEVICE_REMOVE("genesis_snd_z80")

	MCFG_MACHINE_RESET( ms_megadriv )

	MCFG_FRAGMENT_ADD( pico_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( picopal, pico_state )
	MCFG_FRAGMENT_ADD( md_pal )
	MCFG_NVRAM_HANDLER_CLEAR()

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pico_mem)

	MCFG_DEVICE_REMOVE("genesis_snd_z80")

	MCFG_MACHINE_RESET( ms_megadriv )

	MCFG_FRAGMENT_ADD( pico_cartslot )
MACHINE_CONFIG_END



ROM_START( pico )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START( picou )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START( picoj )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT     COMPAT  MACHINE          INPUT   INIT       COMPANY   FULLNAME */
CONS( 1989, genesis,    0,         0,      ms_megadriv,     md, md_cons_state,     genesis,   "Sega",   "Genesis (USA, NTSC)", 0)
CONS( 1990, megadriv,   genesis,   0,      ms_megadpal,     md, md_cons_state,     md_eur,    "Sega",   "Mega Drive (Europe, PAL)", 0)
CONS( 1988, megadrij,   genesis,   0,      ms_megadriv,     md, md_cons_state,     md_jpn,    "Sega",   "Mega Drive (Japan, NTSC)", 0)

// these should not exist, the SVP hardware is in the cart and should be installed dynamically when selected from the Software List
// this however involves installing entire CPUs at run/load time and I don't think we can do that.
CONS( 1993, gensvp,     genesis,   0,      ms_megdsvp,      md, md_cons_state,     genesis,   "Sega",   "Genesis (USA, NTSC, for SVP cart)", 0)
CONS( 1990, mdsvp,      genesis,   0,      ms_megdsvppal,   md, md_cons_state,     md_eur,    "Sega",   "Mega Drive (Europe, PAL, for SVP cart)", 0)
CONS( 1988, mdsvpj,     genesis,   0,      ms_megdsvp,      md, md_cons_state,     md_jpn,    "Sega",   "Mega Drive (Japan, NTSC, for SVP cart)", 0)

// the 32X plugged in the cart slot, games plugged into the 32x.  Maybe it should be handled as an expansion device?
CONS( 1994, 32x,        0,         0,      ms_32x,          md, md_cons_state, genesis,      "Sega",   "Genesis with 32X (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1994, 32xe,       32x,       0,      ms_32x_pal,      md, md_cons_state, md_eur,		 "Sega",   "Mega Drive with 32X (Europe, PAL)", GAME_NOT_WORKING )
CONS( 1994, 32xj,       32x,       0,      ms_32x,          md, md_cons_state, md_jpn,		 "Sega",   "Mega Drive with 32X (Japan, NTSC)", GAME_NOT_WORKING )

// the SegaCD plugged into the expansion port..
CONS( 1992, segacd,     0,         0,      genesis_scd_scd, md, md_cons_state,     genesis,   "Sega",   "Sega CD (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1993, megacd,     segacd,    0,      genesis_scd_mcd, md, md_cons_state,     md_eur,    "Sega",   "Mega-CD (Europe, PAL)", GAME_NOT_WORKING )
CONS( 1991, megacdj,    segacd,    0,      genesis_scd_mcdj,md, md_cons_state,     md_jpn,    "Sega",   "Mega-CD (Japan, NTSC)", GAME_NOT_WORKING ) // this bios doesn't work with our ram interleave needed by a few games?!
CONS( 1991, megacda,    segacd,    0,      genesis_scd_mcdj,md, md_cons_state,     md_eur,    "Sega",   "Mega-CD (Asia, PAL)", GAME_NOT_WORKING )
CONS( 1993, segacd2,    0,         0,      genesis_scd_scd, md, md_cons_state,     genesis,   "Sega",   "Sega CD 2 (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1993, megacd2,    segacd2,   0,      genesis_scd_mcd, md, md_cons_state,     md_eur,    "Sega",   "Mega-CD 2 (Europe, PAL)", GAME_NOT_WORKING )
CONS( 1993, megacd2j,   segacd2,   0,      genesis_scd_mcdj,md, md_cons_state,     md_jpn,    "Sega",   "Mega-CD 2 (Japan, NTSC)", GAME_NOT_WORKING )
CONS( 1993, laseract,   0,         0,      genesis_scd,     md, md_cons_state,     genesis,   "Pioneer","LaserActive (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1993, laseractj,  laseract,  0,      genesis_scd,     md, md_cons_state,     md_jpn,    "Pioneer","LaserActive (Japan, NTSC)", GAME_NOT_WORKING )
CONS( 1993, xeye,       0,         0,      genesis_scd,     md, md_cons_state,     genesis,   "JVC",    "X'eye (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1992, wmega,      xeye,      0,      genesis_scd,     md, md_cons_state,     md_jpn,    "Sega",   "Wondermega (Japan, NTSC)", GAME_NOT_WORKING )
CONS( 1993, wmegam2,    xeye,      0,      genesis_scd,     md, md_cons_state,     md_jpn,    "Victor", "Wondermega M2 (Japan, NTSC)", GAME_NOT_WORKING )
CONS( 1994, cdx,        0,         0,      genesis_scd,     md, md_cons_state,     genesis,   "Sega",   "CDX (USA, NTSC)", GAME_NOT_WORKING )
CONS( 1994, multmega,   cdx,       0,      genesis_scd,     md, md_cons_state,     md_eur,    "Sega",   "Multi-Mega (Europe, PAL)", GAME_NOT_WORKING )
CONS( 1994, 32x_scd,    0,         0,      genesis_32x_scd, md, md_cons_state,	   genesis,  "Sega",   "Sega CD (USA, NTSC, w/32X)", GAME_NOT_WORKING )

// this is a standalone system based on the md-like hardware (same vdp etc.)

CONS( 1994, pico,       0,         0,      picopal,         pico, md_cons_state,   md_eur,    "Sega",   "Pico (Europe, PAL)", 0)
CONS( 1994, picou,      pico,      0,      pico,            pico, md_cons_state,   genesis,   "Sega",   "Pico (USA, NTSC)", 0)
CONS( 1993, picoj,      pico,      0,      pico,            pico, md_cons_state,   md_jpn,    "Sega",   "Pico (Japan, NTSC)", 0)
