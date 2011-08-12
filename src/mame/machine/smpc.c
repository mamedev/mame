/************************************************************************************

Sega Saturn SMPC - System Manager and Peripheral Control MCU simulation

The SMPC is actually a 4-bit Hitachi HD404920FS MCU, labeled with a Sega custom
315-5744 (that needs decapping)

MCU simulation by Angelo Salese & R. Belmont

TODO:
- timings;
- fix intback issue with inputs;
- better arrangement of variables;
- clean-ups;

*************************************************************************************/
/* SMPC Addresses

00
01 -w  Input Register 0 (IREG)
02
03 -w  Input Register 1
04
05 -w  Input Register 2
06
07 -w  Input Register 3
08
09 -w  Input Register 4
0a
0b -w  Input Register 5
0c
0d -w  Input Register 6
0e
0f
10
11
12
13
14
15
16
17
18
19
1a
1b
1c
1d
1e
1f -w  Command Register (COMREG)
20
21 r-  Output Register 0 (OREG)
22
23 r-  Output Register 1
24
25 r-  Output Register 2
26
27 r-  Output Register 3
28
29 r-  Output Register 4
2a
2b r-  Output Register 5
2c
2d r-  Output Register 6
2e
2f r-  Output Register 7
30
31 r-  Output Register 8
32
33 r-  Output Register 9
34
35 r-  Output Register 10
36
37 r-  Output Register 11
38
39 r-  Output Register 12
3a
3b r-  Output Register 13
3c
3d r-  Output Register 14
3e
3f r-  Output Register 15
40
41 r-  Output Register 16
42
43 r-  Output Register 17
44
45 r-  Output Register 18
46
47 r-  Output Register 19
48
49 r-  Output Register 20
4a
4b r-  Output Register 21
4c
4d r-  Output Register 22
4e
4f r-  Output Register 23
50
51 r-  Output Register 24
52
53 r-  Output Register 25
54
55 r-  Output Register 26
56
57 r-  Output Register 27
58
59 r-  Output Register 28
5a
5b r-  Output Register 29
5c
5d r-  Output Register 30
5e
5f r-  Output Register 31
60
61 r-  SR
62
63 rw  SF
64
65
66
67
68
69
6a
6b
6c
6d
6e
6f
70
71
72
73
74
75 rw PDR1
76
77 rw PDR2
78
79 -w DDR1
7a
7b -w DDR2
7c
7d -w IOSEL2/1
7e
7f -w EXLE2/1
*/

#include "emu.h"
#include "coreutil.h"
#include "includes/stv.h"
#include "machine/smpc.h"
#include "machine/eeprom.h"

#define LOG_SMPC 0
#define LOG_PAD_CMD 0

READ8_HANDLER( stv_SMPC_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	int return_data;

	return_data = state->m_smpc_ram[offset];

	if (offset == 0x61) // ?? many games need this or the controls don't work
		return_data = 0x20 ^ 0xff;

	if (offset == 0x75)//PDR1 read
		return_data = input_port_read(space->machine(), "DSW1");

	if (offset == 0x77)//PDR2 read
		return_data=  (0xfe | space->machine().device<eeprom_device>("eeprom")->read_bit());

//  if (offset == 0x33) //country code
//      return_data = input_port_read(machine, "FAKE");

	//if(LOG_SMPC) printf ("cpu %s (PC=%08X) SMPC: Read from Byte Offset %02x Returns %02x\n", space->device().tag(), cpu_get_pc(&space->device()), offset, return_data);


	return return_data;
}

static TIMER_CALLBACK( stv_bankswitch_state )
{
	saturn_state *state = machine.driver_data<saturn_state>();
	static const char *const banknames[] = { "game0", "game1", "game2", "game3" };
	UINT8* game_region;

	if(state->m_prev_bankswitch != param)
	{
		game_region = machine.region(banknames[param])->base();

		if (game_region)
			memcpy(machine.region("abus")->base(), game_region, 0x3000000);
		else
			memset(machine.region("abus")->base(), 0x00, 0x3000000);

		state->m_prev_bankswitch = param;
	}
}

static void stv_select_game(running_machine &machine, int gameno)
{
	machine.scheduler().timer_set(attotime::zero, FUNC(stv_bankswitch_state), gameno);
}

static void smpc_master_on(running_machine &machine)
{
	saturn_state *state = machine.driver_data<saturn_state>();

	device_set_input_line(state->m_maincpu, INPUT_LINE_RESET, CLEAR_LINE);
}

static TIMER_CALLBACK( smpc_slave_enable )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	device_set_input_line(state->m_slave, INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);
	state->m_smpc_ram[0x5f] = param + 0x02; //read-back for last command issued
	state->m_smpc_ram[0x63] = 0x00; //clear hand-shake flag
}

static TIMER_CALLBACK( smpc_sound_enable )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	device_set_input_line(state->m_audiocpu, INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);
	state->m_en_68k = param ^ 1;
	state->m_smpc_ram[0x5f] = param + 0x06; //read-back for last command issued
	state->m_smpc_ram[0x63] = 0x00; //clear hand-shake flag
}

static void smpc_system_reset(running_machine &machine)
{
	saturn_state *state = machine.driver_data<saturn_state>();

	/*Only backup ram and SMPC ram are retained after that this command is issued.*/
	memset(state->m_scu_regs ,0x00,0x000100);
	memset(state->m_scsp_regs,0x00,0x001000);
	memset(state->m_sound_ram,0x00,0x080000);
	memset(state->m_workram_h,0x00,0x100000);
	memset(state->m_workram_l,0x00,0x100000);
	memset(state->m_vdp2_regs,0x00,0x040000);
	memset(state->m_vdp2_vram,0x00,0x100000);
	memset(state->m_vdp2_cram,0x00,0x080000);
	memset(state->m_vdp1_vram,0x00,0x100000);
	//A-Bus

	device_set_input_line(state->m_maincpu, INPUT_LINE_RESET, PULSE_LINE);
}

static void smpc_change_clock(running_machine &machine, UINT8 cmd)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	UINT32 xtal;

	xtal = cmd ? MASTER_CLOCK_320 : MASTER_CLOCK_352;

	machine.device("maincpu")->set_unscaled_clock(xtal/2);
	machine.device("slave")->set_unscaled_clock(xtal/2);

	state->m_vdp2.dotsel = cmd ^ 1;
	stv_vdp2_dynamic_res_change(machine);

	device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, PULSE_LINE); // ff said this causes nmi, should we set a timer then nmi?
	device_set_input_line(state->m_slave, INPUT_LINE_RESET, ASSERT_LINE); // command also asserts slave cpu
	/* TODO: VDP1 / VDP2 / SCU / SCSP default power ON values */
}

static TIMER_CALLBACK( stv_smpc_intback )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	state->m_smpc_ram[0x21] = (0x80) | ((state->m_NMI_reset & 1) << 6);

	{
		int i;

		for(i=0;i<7;i++)
			state->m_smpc_ram[0x23+i*2] = state->m_smpc.rtc_data[i];
	}

	state->m_smpc_ram[0x31]=0x00;  //?

	//state->m_smpc_ram[0x33]=input_port_read(space->machine(), "FAKE");

	state->m_smpc_ram[0x35]= 0 << 7 |
	                         state->m_vdp2.dotsel << 6 |
	                         1 << 5 |
	                         1 << 4 |
	                         0 << 3 | //MSHNMI
	                         1 << 2 |
	                         0 << 1 | //SYSRES
	                         0 << 0;  //SOUNDRES
	state->m_smpc_ram[0x37]= 0 << 6; //CDRES

	state->m_smpc_ram[0x39]=0xff;
	state->m_smpc_ram[0x3b]=0xff;
	state->m_smpc_ram[0x3d]=0xff;
	state->m_smpc_ram[0x3f]=0xff;

	state->m_smpc_ram[0x41]=0xff;
	state->m_smpc_ram[0x43]=0xff;
	state->m_smpc_ram[0x45]=0xff;
	state->m_smpc_ram[0x47]=0xff;
	state->m_smpc_ram[0x49]=0xff;
	state->m_smpc_ram[0x4b]=0xff;
	state->m_smpc_ram[0x4d]=0xff;
	state->m_smpc_ram[0x4f]=0xff;
	state->m_smpc_ram[0x51]=0xff;
	state->m_smpc_ram[0x53]=0xff;
	state->m_smpc_ram[0x55]=0xff;
	state->m_smpc_ram[0x57]=0xff;
	state->m_smpc_ram[0x59]=0xff;
	state->m_smpc_ram[0x5b]=0xff;
	state->m_smpc_ram[0x5d]=0xff;

	//  /*This is for RTC,cartridge code and similar stuff...*/
	//if(LOG_SMPC) printf ("Interrupt: System Manager (SMPC) at scanline %04x, Vector 0x47 Level 0x08\n",scanline);
	if(!(state->m_scu.ism & IRQ_SMPC))
		device_set_input_line_and_vector(state->m_maincpu, 8, HOLD_LINE, 0x47);
	else
		state->m_scu.ist |= (IRQ_SMPC);

	/* clear hand-shake flag */
	state->m_smpc_ram[0x5f] = 0x10;
	state->m_smpc_ram[0x63] = 0x00;
}

static TIMER_CALLBACK( intback_peripheral )
{
	saturn_state *state = machine.driver_data<saturn_state>();
	int pad,pad_num;
	static const char *const padnames[] = { "JOY1", "JOY2" };

	/* doesn't work? */
	//pad_num = state->m_smpc.intback_stage - 1;

	if(LOG_PAD_CMD) printf("%d\n",state->m_smpc.intback_stage - 1);

//  if (LOG_SMPC) logerror("SMPC: providing PAD data for intback, pad %d\n", intback_stage-2);
	for(pad_num=0;pad_num<2;pad_num++)
	{
		pad = input_port_read(machine, padnames[pad_num]);
		state->m_smpc_ram[0x21+pad_num*8] = 0xf1;	// no tap, direct connect
		state->m_smpc_ram[0x23+pad_num*8] = 0x02;	// saturn pad
		state->m_smpc_ram[0x25+pad_num*8] = pad>>8;
		state->m_smpc_ram[0x27+pad_num*8] = pad & 0xff;
	}

	if (state->m_smpc.intback_stage == 2)
	{
		state->m_smpc.smpcSR = (0x80 | state->m_smpc.pmode);	// pad 2, no more data, echo back pad mode set by intback
		state->m_smpc.intback_stage = 0;
	}
	else
	{
		state->m_smpc.smpcSR = (0xc0 | state->m_smpc.pmode);	// pad 1, more data, echo back pad mode set by intback
		state->m_smpc.intback_stage ++;
	}

	if(!(state->m_scu.ism & IRQ_SMPC))
		device_set_input_line_and_vector(state->m_maincpu, 8, HOLD_LINE, 0x47);
	else
		state->m_scu.ist |= (IRQ_SMPC);

	state->m_smpc_ram[0x5f] = 0x10; /* callback for last command issued */
	state->m_smpc_ram[0x63] = 0x00;	/* clear hand-shake flag */
}

static TIMER_CALLBACK( saturn_smpc_intback )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	if(state->m_smpc_ram[1] != 0)
	{
		{
			state->m_smpc_ram[0x21] = (0x80) | ((state->m_NMI_reset & 1) << 6);

			{
				int i;

				for(i=0;i<7;i++)
					state->m_smpc_ram[0x23+i*2] = state->m_smpc.rtc_data[i];
			}

			state->m_smpc_ram[0x31]=0x00;  //?

			//state->m_smpc_ram[0x33]=input_port_read(space->machine(), "FAKE");

			state->m_smpc_ram[0x35]= 0 << 7 |
			                         state->m_vdp2.dotsel << 6 |
			                         1 << 5 |
			                         1 << 4 |
			                         0 << 3 | //MSHNMI
			                         1 << 2 |
			                         0 << 1 | //SYSRES
			                         0 << 0;  //SOUNDRES
			state->m_smpc_ram[0x37]= 0 << 6; //CDRES

			state->m_smpc_ram[0x39]=state->m_smpc.SMEM[0];
			state->m_smpc_ram[0x3b]=state->m_smpc.SMEM[1];
			state->m_smpc_ram[0x3d]=state->m_smpc.SMEM[2];
			state->m_smpc_ram[0x3f]=state->m_smpc.SMEM[3];

			state->m_smpc_ram[0x41]=0xff;
			state->m_smpc_ram[0x43]=0xff;
			state->m_smpc_ram[0x45]=0xff;
			state->m_smpc_ram[0x47]=0xff;
			state->m_smpc_ram[0x49]=0xff;
			state->m_smpc_ram[0x4b]=0xff;
			state->m_smpc_ram[0x4d]=0xff;
			state->m_smpc_ram[0x4f]=0xff;
			state->m_smpc_ram[0x51]=0xff;
			state->m_smpc_ram[0x53]=0xff;
			state->m_smpc_ram[0x55]=0xff;
			state->m_smpc_ram[0x57]=0xff;
			state->m_smpc_ram[0x59]=0xff;
			state->m_smpc_ram[0x5b]=0xff;
			state->m_smpc_ram[0x5d]=0xff;
		}

		state->m_smpc.intback_stage = (state->m_smpc_ram[3] & 8) >> 3; // first peripheral
		state->m_smpc.smpcSR = 0x40 | state->m_smpc.intback_stage << 5;
		state->m_smpc.pmode = state->m_smpc_ram[1]>>4;
		state->m_smpc_ram[0x5f] = 0x10;

		if(!(state->m_scu.ism & IRQ_SMPC))
			device_set_input_line_and_vector(state->m_maincpu, 8, HOLD_LINE, 0x47);
		else
			state->m_scu.ist |= (IRQ_SMPC);

		/* clear hand-shake flag */
		state->m_smpc_ram[0x63] = 0x00;
	}
	else if(state->m_smpc_ram[3] & 8)
	{
		state->m_smpc.intback_stage = (state->m_smpc_ram[3] & 8) >> 3; // first peripheral
		state->m_smpc.smpcSR = 0x40;
		state->m_smpc_ram[0x5f] = 0x10;
		machine.scheduler().timer_set(attotime::from_usec(0), FUNC(intback_peripheral),0);
	}
	else
	{
		printf("SMPC intback bogus behaviour called %02x %02x\n",state->m_smpc_ram[1],state->m_smpc_ram[3]);
	}

}

static void smpc_rtc_write(running_machine &machine)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	int i;

	for(i=0;i<7;i++)
		state->m_smpc.rtc_data[i] = state->m_smpc_ram[0x01+i*2];
}

static void smpc_memory_setting(running_machine &machine)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	int i;

	for(i=0;i<4;i++)
		state->m_smpc.SMEM[i] = state->m_smpc_ram[0x01+i*2];
}

static void smpc_nmi_req(running_machine &machine)
{
	saturn_state *state = machine.driver_data<saturn_state>();

	/*NMI is unconditionally requested?*/
	device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, PULSE_LINE);
}

static void smpc_nmi_set(running_machine &machine,UINT8 cmd)
{
	saturn_state *state = machine.driver_data<saturn_state>();

	state->m_NMI_reset = cmd ^ 1;
	state->m_smpc_ram[0x21] = (0x80) | ((state->m_NMI_reset & 1) << 6);
}

WRITE8_HANDLER( stv_SMPC_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	system_time systime;
	space->machine().base_datetime(systime);

//  if(LOG_SMPC) printf ("8-bit SMPC Write to Offset %02x with Data %02x\n", offset, data);
	state->m_smpc_ram[offset] = data;

	if(offset == 0x75)
	{
		/*
        -xx- ---- PDR1
        ---x ---- EEPROM write bit
        ---- x--- EEPROM CLOCK line
        ---- -x-- EEPROM CS line
        ---- --xx A-Bus bank bits
        */
		eeprom_device *eeprom = space->machine().device<eeprom_device>("eeprom");
		eeprom->set_clock_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
		eeprom->write_bit(data & 0x10);
		eeprom->set_cs_line((data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
		state->m_stv_multi_bank = data & 3;

		stv_select_game(space->machine(), state->m_stv_multi_bank);

		state->m_smpc.PDR1 = (data & 0x60);
	}

	if(offset == 0x77)
	{
		/*
            -xx- ---- PDR2
            ---x ---- Enable Sound System (ACTIVE LOW)
        */
		//popmessage("PDR2 = %02x",state->m_smpc_ram[0x77]);

		if(LOG_SMPC) printf("SMPC: M68k %s\n",(state->m_smpc_ram[0x77] & 0x10) ? "off" : "on");
		//space->machine().scheduler().timer_set(attotime::from_usec(100), FUNC(smpc_sound_enable),(state->m_smpc_ram[0x77] & 0x10) >> 4);
		device_set_input_line(state->m_audiocpu, INPUT_LINE_RESET, (state->m_smpc_ram[0x77] & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		state->m_en_68k = ((state->m_smpc_ram[0x77] & 0x10) >> 4) ^ 1;

		//if(LOG_SMPC) printf("SMPC: ram [0x77] = %02x\n",state->m_smpc_ram[0x77]);
		state->m_smpc.PDR2 = (data & 0x60);
	}

	if(offset == 0x7d)
	{
		/*
        ---- --x- IOSEL2 direct (1) / control mode (0) port select
        ---- ---x IOSEL1 direct (1) / control mode (0) port select
        */
		state->m_smpc.IOSEL1 = (state->m_smpc_ram[0x7d] & 1) >> 0;
		state->m_smpc.IOSEL2 = (state->m_smpc_ram[0x7d] & 2) >> 1;
	}

	if(offset == 0x7f)
	{
		//enable PAD irq & VDP2 external latch for port 1/2
		state->m_smpc.EXLE1 = (state->m_smpc_ram[0x7f] & 1) >> 0;
		state->m_smpc.EXLE2 = (state->m_smpc_ram[0x7f] & 2) >> 1;
	}

	if (offset == 0x1f) // COMREG
	{
		switch (data)
		{
			case 0x00:
				if(LOG_SMPC) printf ("SMPC: Master ON\n");
				smpc_master_on(space->machine());
				break;
			//in theory 0x01 is for Master OFF,but obviously is not used.
			case 0x02:
			case 0x03:
				if(LOG_SMPC) printf ("SMPC: Slave %s\n",(data & 1) ? "off" : "on");
				space->machine().scheduler().timer_set(attotime::from_usec(100), FUNC(smpc_slave_enable),data & 1);
				break;
			case 0x06:
			case 0x07:
				if(LOG_SMPC) printf ("SMPC: Sound %s, ignored\n",(data & 1) ? "off" : "on");
				break;
			/*CD (SH-1) ON/OFF,guess that this is needed for Sports Fishing games...*/
			//case 0x08:
			//case 0x09:
			case 0x0d:
				if(LOG_SMPC) printf ("SMPC: System Reset\n");
				smpc_system_reset(space->machine());
				break;
			case 0x0e:
			case 0x0f:
				if(LOG_SMPC) printf ("SMPC: Change Clock to %s\n",data & 1 ? "320" : "352");
				smpc_change_clock(space->machine(),data & 1);
				break;
			/*"Interrupt Back"*/
			case 0x10:
				if(LOG_SMPC) printf ("SMPC: Status Acquire\n");
				space->machine().scheduler().timer_set(attotime::from_msec(16), FUNC(stv_smpc_intback),0); //TODO: variable time
				break;
			/* RTC write*/
			case 0x16:
				if(LOG_SMPC) printf("SMPC: RTC write\n");
				smpc_rtc_write(space->machine());
				break;
			/* SMPC memory setting*/
			case 0x17:
				if(LOG_SMPC) printf ("SMPC: memory setting\n");
				//smpc_memory_setting(space->machine());
				break;
			case 0x18:
				if(LOG_SMPC) printf ("SMPC: NMI request\n");
				smpc_nmi_req(space->machine());
				break;
			case 0x19:
			case 0x1a:
				if(LOG_SMPC) printf ("SMPC: NMI %sable\n",data & 1 ? "Dis" : "En");
				smpc_nmi_set(space->machine(),data & 1);
				break;
			default:
				printf ("cpu '%s' (PC=%08X) SMPC: undocumented Command %02x\n", space->device().tag(), cpu_get_pc(&space->device()), data);
		}

		// we've processed the command, clear status flag
		if(data != 0x10 && data != 0x02 && data != 0x03)
		{
			state->m_smpc_ram[0x5f] = data; //read-back command
			state->m_smpc_ram[0x63] = 0x00;
		}
		/*TODO:emulate the timing of each command...*/
	}
}

READ8_HANDLER( saturn_SMPC_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	int return_data;

	return_data = state->m_smpc_ram[offset];

	if ((offset == 0x61))
		return_data = state->m_smpc.smpcSR;

	if (offset == 0x75 || offset == 0x77)//PDR1/2 read
	{
/*
    PORT_START("JOY1")
    PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
    PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
    PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
    PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
    PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) // START
    PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1) // A
    PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1) // B
    PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1) // C
    PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P1 R") PORT_PLAYER(1) // R
    PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 X") PORT_PLAYER(1) // X
    PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Y") PORT_PLAYER(1) // Y
    PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Z") PORT_PLAYER(1) // Z
    PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("P1 L") PORT_PLAYER(1) // L
*/
		if ((state->m_smpc.IOSEL1 && offset == 0x75) || (state->m_smpc.IOSEL2 && offset == 0x77))
		{
			int hshake;
			const int shift_bit[4] = { 4, 12, 8, 0 };
			const char *const padnames[] = { "JOY1", "JOY2" };

			if(offset == 0x75)
				hshake = (state->m_smpc.PDR1>>5) & 3;
			else
				hshake = (state->m_smpc.PDR2>>5) & 3;

			if (LOG_SMPC) logerror("SMPC: SH-2 direct mode, returning data for phase %d\n", hshake);

			return_data = 0x80 | 0x10 | ((input_port_read(space->machine(), padnames[offset == 0x77])>>shift_bit[hshake]) & 0xf);
		}
	}

	if (offset == 0x33) return_data = state->m_saturn_region;

	if (LOG_SMPC) logerror ("cpu %s (PC=%08X) SMPC: Read from Byte Offset %02x (%d) Returns %02x\n", space->device().tag(), cpu_get_pc(&space->device()), offset, offset>>1, return_data);


	return return_data;
}

WRITE8_HANDLER( saturn_SMPC_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	system_time systime;
//	UINT8 last;
	running_machine &machine = space->machine();

	/* get the current date/time from the core */
	machine.current_datetime(systime);

  if (LOG_SMPC) logerror ("8-bit SMPC Write to Offset %02x (reg %d) with Data %02x (prev %02x)\n", offset, offset>>1, data, state->m_smpc_ram[offset]);

//  if (offset == 0x7d) printf("IOSEL2 %d IOSEL1 %d\n", (data>>1)&1, data&1);

//	last = state->m_smpc_ram[offset];

	if (offset == 1)
	{
		if(state->m_smpc.intback_stage)
		{
			if(data & 0x40)
			{
				if(LOG_PAD_CMD) printf("SMPC: BREAK request\n");
				state->m_smpc.smpcSR &= 0x0f;
				state->m_smpc.intback_stage = 0;
			}
			else if(data & 0x80)
			{
				if(LOG_PAD_CMD) printf("SMPC: CONTINUE request\n");
				space->machine().scheduler().timer_set(attotime::from_usec(200), FUNC(intback_peripheral),0); /* TODO: is timing correct? */
				state->m_smpc_ram[0x1f] = 0x10;
				//state->m_smpc_ram[0x63] = 0x01; //TODO: set hand-shake flag?
			}
		}
	}

	state->m_smpc_ram[offset] = data;

	if (offset == 0x75)	// PDR1
	{
		state->m_smpc.PDR1 = (data & state->m_smpc_ram[0x79]);
	}

	if (offset == 0x77)	// PDR2
	{
		state->m_smpc.PDR2 = (data & state->m_smpc_ram[0x7b]);
	}

	if(offset == 0x7d)
	{
		state->m_smpc.IOSEL1 = state->m_smpc_ram[0x7d] & 1;
		state->m_smpc.IOSEL2 = (state->m_smpc_ram[0x7d] & 2) >> 1;
	}

	if(offset == 0x7f)
	{
		//enable PAD irq & VDP2 external latch for port 1/2
		state->m_smpc.EXLE1 = (state->m_smpc_ram[0x7f] & 1) >> 0;
		state->m_smpc.EXLE2 = (state->m_smpc_ram[0x7f] & 2) >> 1;
	}

	if (offset == 0x1f)
	{
		switch (data)
		{
			case 0x00:
				if(LOG_SMPC) printf ("SMPC: Master ON\n");
				smpc_master_on(space->machine());
				break;
			//in theory 0x01 is for Master OFF
			case 0x02:
			case 0x03:
				if(LOG_SMPC) printf ("SMPC: Slave %s\n",(data & 1) ? "off" : "on");
				space->machine().scheduler().timer_set(attotime::from_usec(100), FUNC(smpc_slave_enable),data & 1);
				break;
			case 0x06:
			case 0x07:
				if(LOG_SMPC) printf ("SMPC: Sound %s\n",(data & 1) ? "off" : "on");
				space->machine().scheduler().timer_set(attotime::from_usec(100), FUNC(smpc_sound_enable),data & 1);
				break;
			/*CD (SH-1) ON/OFF,guess that this is needed for Sports Fishing games...*/
			//case 0x08:
			//case 0x09:
			case 0x0d:
				if(LOG_SMPC) printf ("SMPC: System Reset\n");
				smpc_system_reset(space->machine());
				break;
			case 0x0e:
			case 0x0f:
				if(LOG_SMPC) printf ("SMPC: Change Clock to %s\n",data & 1 ? "320" : "352");
				smpc_change_clock(space->machine(),data & 1);
				break;
			/*"Interrupt Back"*/
			case 0x10:
                if(LOG_SMPC) printf ("SMPC: Status Acquire (IntBack)\n");
				int timing;

				timing = 100;

				if(state->m_smpc_ram[1] != 0) // non-peripheral data
					timing = 200;

				if(state->m_smpc_ram[3] & 8) // peripheral data
					timing = 15000;

				if(LOG_PAD_CMD) printf("INTBACK %02x %02x\n",state->m_smpc_ram[1],state->m_smpc_ram[3]);

				space->machine().scheduler().timer_set(attotime::from_usec(timing), FUNC(saturn_smpc_intback),0); //TODO: is variable time correct?
				break;
			/* RTC write*/
			case 0x16:
				if(LOG_SMPC) printf("SMPC: RTC write\n");
				smpc_rtc_write(space->machine());
				break;
			/* SMPC memory setting*/
			case 0x17:
				if(LOG_SMPC) printf ("SMPC: memory setting\n");
				smpc_memory_setting(space->machine());
				break;
			case 0x18:
				if(LOG_SMPC) printf ("SMPC: NMI request\n");
				smpc_nmi_req(space->machine());
				break;
			case 0x19:
			case 0x1a:
				if(LOG_SMPC) printf ("SMPC: NMI %sable\n",data & 1 ? "Dis" : "En");
				smpc_nmi_set(space->machine(),data & 1);
				break;
			default:
				printf ("cpu %s (PC=%08X) SMPC: undocumented Command %02x\n", space->device().tag(), cpu_get_pc(&space->device()), data);
		}

		// we've processed the command, clear status flag
		if(data != 0x10 && data != 2 && data != 3 && data != 6 && data != 7)
		{
			state->m_smpc_ram[0x5f] = data; //read-back for last command issued
			state->m_smpc_ram[0x63] = 0x00; //clear hand-shake flag
		}
		/*TODO:emulate the timing of each command...*/
	}
}
