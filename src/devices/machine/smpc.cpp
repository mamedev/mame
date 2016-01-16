// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/************************************************************************************

Sega Saturn SMPC - System Manager and Peripheral Control MCU simulation

The SMPC is actually a 4-bit Hitachi HD404920FS MCU, labeled with a Sega custom
315-5744 (that needs decapping)

MCU simulation by Angelo Salese & R. Belmont

TODO:
- timings;
- fix intback issue with inputs (according to the docs, it should fall in between
  VBLANK-IN and OUT, for obvious reasons);
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
#include "machine/eepromser.h"

#define LOG_SMPC 0
#define LOG_PAD_CMD 0


/********************************************
 *
 * Bankswitch code for ST-V Multi Cart mode
 *
 *******************************************/

void saturn_state::stv_select_game(int gameno)
{
	if (m_prev_bankswitch != gameno)
	{
		if (m_cart_reg[gameno] && m_cart_reg[gameno]->base())
			memcpy(memregion("abus")->base(), m_cart_reg[gameno]->base(), 0x3000000);
		else
			memset(memregion("abus")->base(), 0x00, 0x3000000); // TODO: 1-filled?

		m_prev_bankswitch = gameno;
	}
}

/********************************************
 *
 * Command functions
 *
 *******************************************/

void saturn_state::smpc_master_on()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER( saturn_state::smpc_slave_enable )
{
	m_slave->set_input_line(INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);
	m_smpc.OREG[31] = param + 0x02; //read-back for last command issued
	m_smpc.SF = 0x00; //clear hand-shake flag
	m_smpc.slave_on = param;
//  printf("%d %d\n",machine().first_screen()->hpos(),machine().first_screen()->vpos());
}

TIMER_CALLBACK_MEMBER( saturn_state::smpc_sound_enable )
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);
	m_en_68k = param ^ 1;
	m_smpc.OREG[31] = param + 0x06; //read-back for last command issued
	m_smpc.SF = 0x00; //clear hand-shake flag
}

TIMER_CALLBACK_MEMBER( saturn_state::smpc_cd_enable )
{
	m_smpc.OREG[31] = param + 0x08; //read-back for last command issued
	m_smpc.SF = 0x08; //clear hand-shake flag (TODO: diagnostic wants this to have bit 3 high)
}

void saturn_state::smpc_system_reset()
{
	/*Only backup ram and SMPC ram are retained after that this command is issued.*/
	memset(m_scu_regs.get() ,0x00,0x000100);
	memset(m_scsp_regs.get(),0x00,0x001000);
	memset(m_sound_ram,0x00,0x080000);
	memset(m_workram_h,0x00,0x100000);
	memset(m_workram_l,0x00,0x100000);
	memset(m_vdp2_regs.get(),0x00,0x040000);
	memset(m_vdp2_vram.get(),0x00,0x100000);
	memset(m_vdp2_cram.get(),0x00,0x080000);
	memset(m_vdp1_vram.get(),0x00,0x100000);
	//A-Bus

	m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}

TIMER_CALLBACK_MEMBER( saturn_state::smpc_change_clock )
{
	UINT32 xtal;

	if(LOG_SMPC) printf ("Clock change execute at (%d %d)\n",machine().first_screen()->hpos(),machine().first_screen()->vpos());

	xtal = param ? MASTER_CLOCK_320 : MASTER_CLOCK_352;

	machine().device("maincpu")->set_unscaled_clock(xtal/2);
	machine().device("slave")->set_unscaled_clock(xtal/2);

	m_vdp2.dotsel = param ^ 1;
	stv_vdp2_dynamic_res_change();

	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	if(!m_NMI_reset)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	m_slave->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_slave->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	/* put issued command in OREG31 */
	m_smpc.OREG[31] = 0x0e + param;
	/* clear hand-shake flag */
	m_smpc.SF = 0x00;

	/* TODO: VDP1 / VDP2 / SCU / SCSP default power ON values? */
}

TIMER_CALLBACK_MEMBER( saturn_state::stv_intback_peripheral )
{
	if (m_smpc.intback_stage == 2)
	{
		m_smpc.SR = (0x80 | m_smpc.pmode);    // pad 2, no more data, echo back pad mode set by intback
		m_smpc.intback_stage = 0;
	}
	else
	{
		m_smpc.SR = (0xc0 | m_smpc.pmode);    // pad 1, more data, echo back pad mode set by intback
		m_smpc.intback_stage ++;
	}

	if(!(m_scu.ism & IRQ_SMPC))
		m_maincpu->set_input_line_and_vector(8, HOLD_LINE, 0x47);
	else
		m_scu.ist |= (IRQ_SMPC);

	m_smpc.OREG[31] = 0x10; /* callback for last command issued */
	m_smpc.SF = 0x00;    /* clear hand-shake flag */
}


TIMER_CALLBACK_MEMBER( saturn_state::stv_smpc_intback )
{
	int i;

//  printf("%02x %02x %02x\n",m_smpc.intback_buf[0],m_smpc.intback_buf[1],m_smpc.intback_buf[2]);

	if(m_smpc.intback_buf[0] != 0)
	{
		m_smpc.OREG[0] = (0x80) | ((m_NMI_reset & 1) << 6);

		for(i=0;i<7;i++)
			m_smpc.OREG[1+i] = m_smpc.rtc_data[i];

		m_smpc.OREG[8]=0x00;  // CTG0 / CTG1?

		m_smpc.OREG[9]=0x00;  // TODO: system region on Saturn

		m_smpc.OREG[10]= 0 << 7 |
								m_vdp2.dotsel << 6 |
								1 << 5 |
								1 << 4 |
								0 << 3 | //MSHNMI
								1 << 2 |
								0 << 1 | //SYSRES
								0 << 0;  //SOUNDRES
		m_smpc.OREG[11]= 0 << 6; //CDRES

		for(i=0;i<4;i++)
			m_smpc.OREG[12+i]=m_smpc.SMEM[i];

		for(i=0;i<15;i++)
			m_smpc.OREG[16+i]=0xff; // undefined

		m_smpc.intback_stage = (m_smpc.intback_buf[1] & 8) >> 3; // first peripheral
		m_smpc.SR = 0x40 | m_smpc.intback_stage << 5;
		m_smpc.pmode = m_smpc.intback_buf[0]>>4;

		//  /*This is for RTC,cartridge code and similar stuff...*/
		//if(LOG_SMPC) printf ("Interrupt: System Manager (SMPC) at scanline %04x, Vector 0x47 Level 0x08\n",scanline);
		if(!(m_scu.ism & IRQ_SMPC))
			m_maincpu->set_input_line_and_vector(8, HOLD_LINE, 0x47);
		else
			m_scu.ist |= (IRQ_SMPC);

		/* put issued command in OREG31 */
		m_smpc.OREG[31] = 0x10; // TODO: doc says 0?
		/* clear hand-shake flag */
		m_smpc.SF = 0x00;
	}
	else if(m_smpc.intback_buf[1] & 8)
	{
		m_smpc.intback_stage = (m_smpc.intback_buf[1] & 8) >> 3; // first peripheral
		m_smpc.SR = 0x40;
		m_smpc.OREG[31] = 0x10;
		machine().scheduler().timer_set(attotime::from_usec(0), timer_expired_delegate(FUNC(saturn_state::stv_intback_peripheral),this),0);
	}
	else
	{
		/* Shienryu calls this, it would be plainly illegal on Saturn, I'll just return the command and clear the hs flag for now. */
		m_smpc.OREG[31] = 0x10;
		m_smpc.SF = 0x00;
	}
}


/*
    [0] port status:
        0x04 Sega-tap
        0x16 Multi-tap
        0x2x clock serial peripheral
        0xf0 peripheral isn't connected
        0xf1 peripheral is connected
    [1] Peripheral ID (note: lowest four bits determines the size of the input packet)
        0x02 digital pad
        0x25 (tested by Game Basic?)
        0x34 keyboard
*/

void saturn_state::smpc_digital_pad(UINT8 pad_num, UINT8 offset)
{
	static const char *const padnames[] = { "JOY1", "JOY2" };
	UINT16 pad_data;

	pad_data = ioport(padnames[pad_num])->read();
	m_smpc.OREG[0+pad_num*offset] = 0xf1;
	m_smpc.OREG[1+pad_num*offset] = 0x02;
	m_smpc.OREG[2+pad_num*offset] = pad_data>>8;
	m_smpc.OREG[3+pad_num*offset] = pad_data & 0xff;
}

void saturn_state::smpc_analog_pad( UINT8 pad_num, UINT8 offset, UINT8 id)
{
	static const char *const padnames[] = { "AN_JOY1", "AN_JOY2" };
	static const char *const annames[2][3] = { { "AN_X1", "AN_Y1", "AN_Z1" },
												{ "AN_X2", "AN_Y2", "AN_Z2" }};
	UINT16 pad_data;

	pad_data = ioport(padnames[pad_num])->read();
	m_smpc.OREG[0+pad_num*offset] = 0xf1;
	m_smpc.OREG[1+pad_num*offset] = id;
	m_smpc.OREG[2+pad_num*offset] = pad_data>>8;
	m_smpc.OREG[3+pad_num*offset] = pad_data & 0xff;
	m_smpc.OREG[4+pad_num*offset] = ioport(annames[pad_num][0])->read();
	if(id == 0x15)
	{
		m_smpc.OREG[5+pad_num*offset] = ioport(annames[pad_num][1])->read();
		m_smpc.OREG[6+pad_num*offset] = ioport(annames[pad_num][2])->read();
	}
}

void saturn_state::smpc_keyboard(UINT8 pad_num, UINT8 offset)
{
	UINT16 game_key;

	game_key = 0xffff;

	game_key ^= ((ioport("KEYS_1")->read() & 0x80) << 8); // right
	game_key ^= ((ioport("KEYS_1")->read() & 0x40) << 8); // left
	game_key ^= ((ioport("KEYS_1")->read() & 0x20) << 8); // down
	game_key ^= ((ioport("KEYS_1")->read() & 0x10) << 8); // up
	game_key ^= ((ioport("KEYF")->read() & 0x80) << 4); // ESC -> START
	game_key ^= ((ioport("KEY3")->read() & 0x04) << 8); // Z / A trigger
	game_key ^= ((ioport("KEY4")->read() & 0x02) << 8); // C / C trigger
	game_key ^= ((ioport("KEY6")->read() & 0x04) << 6); // X / B trigger
	game_key ^= ((ioport("KEY2")->read() & 0x20) << 2); // Q / R trigger
	game_key ^= ((ioport("KEY3")->read() & 0x10) << 2); // A / X trigger
	game_key ^= ((ioport("KEY3")->read() & 0x08) << 2); // S / Y trigger
	game_key ^= ((ioport("KEY4")->read() & 0x08) << 1); // D / Z trigger
	game_key ^= ((ioport("KEY4")->read() & 0x10) >> 1); // E / L trigger

	m_smpc.OREG[0+pad_num*offset] = 0xf1;
	m_smpc.OREG[1+pad_num*offset] = 0x34;
	m_smpc.OREG[2+pad_num*offset] = game_key>>8; // game buttons, TODO
	m_smpc.OREG[3+pad_num*offset] = game_key & 0xff;
	/*
	    Keyboard Status hook-up
	    TODO: how shift key actually works? EGWord uses it in order to switch between hiragana and katakana modes.
	    x--- ---- 0
	    -x-- ---- caps lock
	    --x- ---- num lock
	    ---x ---- scroll lock
	    ---- x--- data ok
	    ---- -x-- 1
	    ---- --x- 1
	    ---- ---x Break key
	*/
	m_smpc.OREG[4+pad_num*offset] = m_keyb.status | 6;
	if(m_keyb.prev_data != m_keyb.data)
	{
		m_smpc.OREG[5+pad_num*offset] = m_keyb.data;
		m_keyb.repeat_count = 0;
		m_keyb.prev_data = m_keyb.data;
	}
	else
	{
		/* Very crude repeat support */
		m_keyb.repeat_count ++;
		m_keyb.repeat_count = m_keyb.repeat_count > 32 ? 32 : m_keyb.repeat_count;
		m_smpc.OREG[5+pad_num*offset] = (m_keyb.repeat_count == 32) ? m_keyb.data : 0;
	}
}

void saturn_state::smpc_mouse(UINT8 pad_num, UINT8 offset, UINT8 id)
{
	static const char *const mousenames[2][3] = { { "MOUSEB1", "MOUSEX1", "MOUSEY1" },
													{ "MOUSEB2", "MOUSEX2", "MOUSEY2" }};
	UINT8 mouse_ctrl;
	INT16 mouse_x, mouse_y;

	mouse_ctrl = ioport(mousenames[pad_num][0])->read();
	mouse_x = ioport(mousenames[pad_num][1])->read();
	mouse_y = ioport(mousenames[pad_num][2])->read();

	if(mouse_x < 0)
		mouse_ctrl |= 0x10;

	if(mouse_y < 0)
		mouse_ctrl |= 0x20;

	if((mouse_x & 0xff00) != 0xff00 && (mouse_x & 0xff00) != 0x0000)
		mouse_ctrl |= 0x40;

	if((mouse_y & 0xff00) != 0xff00 && (mouse_y & 0xff00) != 0x0000)
		mouse_ctrl |= 0x80;

	m_smpc.OREG[0+pad_num*offset] = 0xf1;
	m_smpc.OREG[1+pad_num*offset] = id; // 0x23 / 0xe3
	m_smpc.OREG[2+pad_num*offset] = mouse_ctrl;
	m_smpc.OREG[3+pad_num*offset] = mouse_x & 0xff;
	m_smpc.OREG[4+pad_num*offset] = mouse_y & 0xff;
}

/* TODO: is there ANY game on which the MD pad works? */
void saturn_state::smpc_md_pad(UINT8 pad_num, UINT8 offset, UINT8 id)
{
	static const char *const padnames[] = { "MD_JOY1", "MD_JOY2" };
	UINT16 pad_data;

	pad_data = ioport(padnames[pad_num])->read();
	m_smpc.OREG[0+pad_num*offset] = 0xf1;
	m_smpc.OREG[1+pad_num*offset] = id;
	m_smpc.OREG[2+pad_num*offset] = pad_data>>8;
	if(id == 0xe2) // MD 6 Button PAD
		m_smpc.OREG[3+pad_num*offset] = pad_data & 0xff;
}

void saturn_state::smpc_unconnected(UINT8 pad_num, UINT8 offset)
{
	m_smpc.OREG[0+pad_num*offset] = 0xf0;
}

TIMER_CALLBACK_MEMBER( saturn_state::intback_peripheral )
{
	int pad_num;
	static const UINT8 peri_id[10] = { 0x02, 0x13, 0x15, 0x23, 0x23, 0x34, 0xe1, 0xe2, 0xe3, 0xff };
	UINT8 read_id[2];
	UINT8 offset;

//  if (LOG_SMPC) logerror("SMPC: providing PAD data for intback, pad %d\n", intback_stage-2);

	read_id[0] = (ioport("INPUT_TYPE")->read()) & 0x0f;
	read_id[1] = (ioport("INPUT_TYPE")->read()) >> 4;

	/* doesn't work? */
	//pad_num = m_smpc.intback_stage - 1;

	if(LOG_PAD_CMD) printf("%d %d %d\n",m_smpc.intback_stage - 1,machine().first_screen()->vpos(),(int)machine().first_screen()->frame_number());

	offset = 0;

	for(pad_num=0;pad_num<2;pad_num++)
	{
		switch(read_id[pad_num])
		{
			case 0: smpc_digital_pad(pad_num,offset); break;
			case 1: smpc_analog_pad(pad_num,offset,peri_id[read_id[pad_num]]); break; /* Steering Wheel */
			case 2: smpc_analog_pad(pad_num,offset,peri_id[read_id[pad_num]]); break; /* Analog Pad */
			case 4: smpc_mouse(pad_num,offset,peri_id[read_id[pad_num]]); break; /* Pointing Device */
			case 5: smpc_keyboard(pad_num,offset); break;
			case 6: smpc_md_pad(pad_num,offset,peri_id[read_id[pad_num]]); break; /* MD 3B PAD */
			case 7: smpc_md_pad(pad_num,offset,peri_id[read_id[pad_num]]); break; /* MD 6B PAD */
			case 8: smpc_mouse(pad_num,offset,peri_id[read_id[pad_num]]); break; /* Saturn Mouse */
			case 9: smpc_unconnected(pad_num,offset); break;
		}

		offset += (peri_id[read_id[pad_num]] & 0xf) + 2; /* offset for port 2 */
	}

	if (m_smpc.intback_stage == 2)
	{
		m_smpc.SR = (0x80 | m_smpc.pmode);    // pad 2, no more data, echo back pad mode set by intback
		m_smpc.intback_stage = 0;
	}
	else
	{
		m_smpc.SR = (0xc0 | m_smpc.pmode);    // pad 1, more data, echo back pad mode set by intback
		m_smpc.intback_stage ++;
	}

	if(!(m_scu.ism & IRQ_SMPC))
		m_maincpu->set_input_line_and_vector(8, HOLD_LINE, 0x47);
	else
		m_scu.ist |= (IRQ_SMPC);

	m_smpc.OREG[31] = 0x10; /* callback for last command issued */
	m_smpc.SF = 0x00;    /* clear hand-shake flag */
}

TIMER_CALLBACK_MEMBER( saturn_state::saturn_smpc_intback )
{
	if(m_smpc.intback_buf[0] != 0)
	{
		{
			int i;

			m_smpc.OREG[0] = (0x80) | ((m_NMI_reset & 1) << 6); // bit 7: SETTIME (RTC isn't setted up properly)

			for(i=0;i<7;i++)
				m_smpc.OREG[1+i] = m_smpc.rtc_data[i];

			m_smpc.OREG[8]=0x00;  //Cartridge code?

			m_smpc.OREG[9] = m_saturn_region;

			m_smpc.OREG[10]= 0 << 7 |
										m_vdp2.dotsel << 6 |
										1 << 5 |
										1 << 4 |
										0 << 3 | //MSHNMI
										1 << 2 |
										0 << 1 | //SYSRES
										0 << 0;  //SOUNDRES
			m_smpc.OREG[11]= 0 << 6; //CDRES

			for(i=0;i<4;i++)
				m_smpc.OREG[12+i]=m_smpc.SMEM[i];

			for(i=0;i<15;i++)
				m_smpc.OREG[16+i]=0xff; // undefined
		}

		m_smpc.intback_stage = (m_smpc.intback_buf[1] & 8) >> 3; // first peripheral
		m_smpc.SR = 0x40 | m_smpc.intback_stage << 5;
		m_smpc.pmode = m_smpc.intback_buf[0]>>4;

		if(!(m_scu.ism & IRQ_SMPC))
			m_maincpu->set_input_line_and_vector(8, HOLD_LINE, 0x47);
		else
			m_scu.ist |= (IRQ_SMPC);

		/* put issued command in OREG31 */
		m_smpc.OREG[31] = 0x10;
		/* clear hand-shake flag */
		m_smpc.SF = 0x00;
	}
	else if(m_smpc.intback_buf[1] & 8)
	{
		m_smpc.intback_stage = (m_smpc.intback_buf[1] & 8) >> 3; // first peripheral
		m_smpc.SR = 0x40;
		m_smpc.OREG[31] = 0x10;
		machine().scheduler().timer_set(attotime::from_usec(0), timer_expired_delegate(FUNC(saturn_state::intback_peripheral),this),0);
	}
	else
	{
		printf("SMPC intback bogus behaviour called %02x %02x\n",m_smpc.IREG[0],m_smpc.IREG[1]);
	}

}

void saturn_state::smpc_rtc_write()
{
	int i;

	for(i=0;i<7;i++)
		m_smpc.rtc_data[i] = m_smpc.IREG[i];
}

void saturn_state::smpc_memory_setting()
{
	int i;

	for(i=0;i<4;i++)
		m_smpc.SMEM[i] = m_smpc.IREG[i];
}

void saturn_state::smpc_nmi_req()
{
	/*NMI is unconditionally requested */
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

TIMER_CALLBACK_MEMBER( saturn_state::smpc_nmi_set )
{
//  printf("%d %d\n",machine().first_screen()->hpos(),machine().first_screen()->vpos());

	m_NMI_reset = param;
	/* put issued command in OREG31 */
	m_smpc.OREG[31] = 0x19 + param;
	/* clear hand-shake flag */
	m_smpc.SF = 0x00;

	//m_smpc.OREG[0] = (0x80) | ((m_NMI_reset & 1) << 6);
}


TIMER_CALLBACK_MEMBER( saturn_state::smpc_audio_reset_line_pulse )
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}

/********************************************
 *
 * COMREG sub-routine
 *
 *******************************************/

void saturn_state::smpc_comreg_exec(address_space &space, UINT8 data, UINT8 is_stv)
{
	switch (data)
	{
		case 0x00:
			if(LOG_SMPC) printf ("SMPC: Master ON\n");
			smpc_master_on();
			break;
		//case 0x01: Master OFF?
		case 0x02:
		case 0x03:
			if(LOG_SMPC) printf ("SMPC: Slave %s %d %d\n",(data & 1) ? "off" : "on",machine().first_screen()->hpos(),machine().first_screen()->vpos());
			machine().scheduler().timer_set(attotime::from_usec(15), timer_expired_delegate(FUNC(saturn_state::smpc_slave_enable),this),data & 1);
			break;
		case 0x06:
		case 0x07:
			if(LOG_SMPC) printf ("SMPC: Sound %s\n",(data & 1) ? "off" : "on");

			if(!is_stv)
				machine().scheduler().timer_set(attotime::from_usec(15), timer_expired_delegate(FUNC(saturn_state::smpc_sound_enable),this),data & 1);
			break;
		/*CD (SH-1) ON/OFF */
		case 0x08:
		case 0x09:
			printf ("SMPC: CD %s\n",(data & 1) ? "off" : "on");
			machine().scheduler().timer_set(attotime::from_usec(20), timer_expired_delegate(FUNC(saturn_state::smpc_cd_enable),this),data & 1);
			break;
		case 0x0a:
		case 0x0b:
			popmessage ("SMPC: NETLINK %s, contact MAMEdev",(data & 1) ? "off" : "on");
			break;      case 0x0d:
			if(LOG_SMPC) printf ("SMPC: System Reset\n");
			smpc_system_reset();
			break;
		case 0x0e:
		case 0x0f:
			if(LOG_SMPC) printf ("SMPC: Change Clock to %s (%d %d)\n",data & 1 ? "320" : "352",machine().first_screen()->hpos(),machine().first_screen()->vpos());

			/* on ST-V timing of this is pretty fussy, you get 2 credits at start-up otherwise
			   My current theory is that SMPC first stops all CPUs until it executes the whole snippet for this,
			   and restarts them when the screen is again ready for use. I really don't think that the system
			   can do an usable mid-frame clock switching anyway.
			   */

			m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_slave->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

			machine().scheduler().timer_set(machine().first_screen()->time_until_pos(get_vblank_start_position()*get_ystep_count(), 0), timer_expired_delegate(FUNC(saturn_state::smpc_change_clock),this),data & 1);
			break;
		/*"Interrupt Back"*/
		case 0x10:
			if(0)
			{
				printf ("SMPC: Status Acquire %02x %02x %02x %d\n",m_smpc.IREG[0],m_smpc.IREG[1],m_smpc.IREG[2],machine().first_screen()->vpos());
			}

			int timing;

			timing = 8;

			if(m_smpc.IREG[0] != 0) // non-peripheral data
				timing += 8;

			/* TODO: At vblank-out actually ... */
			if(m_smpc.IREG[1] & 8) // peripheral data
				timing += 700;

			/* TODO: check if IREG[2] is setted to 0xf0 */
			{
				int i;

				for(i=0;i<3;i++)
					m_smpc.intback_buf[i] = m_smpc.IREG[i];
			}

			if(is_stv)
			{
				machine().scheduler().timer_set(attotime::from_usec(timing), timer_expired_delegate(FUNC(saturn_state::stv_smpc_intback),this),0); //TODO: variable time
			}
			else
			{
				if(LOG_PAD_CMD) printf("INTBACK %02x %02x %d %d\n",m_smpc.IREG[0],m_smpc.IREG[1],machine().first_screen()->vpos(),(int)machine().first_screen()->frame_number());
				machine().scheduler().timer_set(attotime::from_usec(timing), timer_expired_delegate(FUNC(saturn_state::saturn_smpc_intback),this),0); //TODO: is variable time correct?
			}
			break;
		/* RTC write*/
		case 0x16:
			if(LOG_SMPC) printf("SMPC: RTC write\n");
			smpc_rtc_write();
			break;
		/* SMPC memory setting*/
		case 0x17:
			if(LOG_SMPC) printf ("SMPC: memory setting\n");
			smpc_memory_setting();
			break;
		case 0x18:
			if(LOG_SMPC) printf ("SMPC: NMI request\n");
			smpc_nmi_req();
			break;
		case 0x19:
		case 0x1a:
			/* TODO: timing */
			if(LOG_SMPC) printf ("SMPC: NMI %sable %d %d\n",data & 1 ? "Dis" : "En",machine().first_screen()->hpos(),machine().first_screen()->vpos());
			machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(saturn_state::smpc_nmi_set),this),data & 1);
			break;
		default:
			printf ("cpu '%s' (PC=%08X) SMPC: undocumented Command %02x\n", space.device().tag().c_str(), space.device().safe_pc(), data);
	}
}

/********************************************
 *
 * ST-V handlers
 *
 *******************************************/

READ8_MEMBER( saturn_state::stv_SMPC_r )
{
	int return_data = 0;

	if(!(offset & 1))
		return 0;

	if(offset >= 0x21 && offset <= 0x5f)
		return_data = m_smpc.OREG[(offset-0x21) >> 1];

	if (offset == 0x61) // TODO: SR
		return_data = m_smpc.SR;

	if (offset == 0x63)
		return_data = m_smpc.SF;

	if (offset == 0x75)//PDR1 read
		return_data = ioport("DSW1")->read();

	if (offset == 0x77)//PDR2 read
		return_data = (0xfe | m_eeprom->do_read());

	return return_data;
}

WRITE8_MEMBER( saturn_state::stv_SMPC_w )
{
	if (!(offset & 1)) // avoid writing to even bytes
		return;

//  if(LOG_SMPC) printf ("8-bit SMPC Write to Offset %02x with Data %02x\n", offset, data);

	if(offset >= 1 && offset <= 0xd)
		m_smpc.IREG[offset >> 1] = data;

	if(offset == 1) //IREG0, check if a BREAK / CONTINUE request for INTBACK command
	{
		if(m_smpc.intback_stage)
		{
			if(data & 0x40)
			{
				if(LOG_PAD_CMD) printf("SMPC: BREAK request\n");
				m_smpc.SR &= 0x0f;
				m_smpc.intback_stage = 0;
			}
			else if(data & 0x80)
			{
				if(LOG_PAD_CMD) printf("SMPC: CONTINUE request\n");
				machine().scheduler().timer_set(attotime::from_usec(700), timer_expired_delegate(FUNC(saturn_state::stv_intback_peripheral),this),0); /* TODO: is timing correct? */
				m_smpc.OREG[31] = 0x10;
				m_smpc.SF = 0x01; //TODO: set hand-shake flag?
			}
		}
	}

	if (offset == 0x1f) // COMREG
	{
		smpc_comreg_exec(space,data,1);

		// we've processed the command, clear status flag
		if(data != 0x10 && data != 0x02 && data != 0x03 && data != 0x08 && data != 0x09 && data != 0xe && data != 0xf && data != 0x19 && data != 0x1a)
		{
			m_smpc.OREG[31] = data; //read-back command
			m_smpc.SF = 0x00;
		}
		/*TODO:emulate the timing of each command...*/
	}

	if(offset == 0x63)
		m_smpc.SF = data & 1;

	if(offset == 0x75)
	{
		/*
		-xx- ---- PDR1
		---x ---- EEPROM write bit
		---- x--- EEPROM CLOCK line
		---- -x-- EEPROM CS line
		---- --xx A-Bus bank bits
		*/
		m_eeprom->clk_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write((data >> 4) & 1);
		m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
		m_stv_multi_bank = data & 3;

		stv_select_game(m_stv_multi_bank);

		m_smpc.PDR1 = (data & 0x60);
	}

	if(offset == 0x77)
	{
		/*
		    -xx- ---- PDR2
		    ---x ---- Enable Sound System (ACTIVE LOW)
		*/
		//popmessage("PDR2 = %02x",m_smpc_ram[0x77]);

		if(LOG_SMPC) printf("SMPC: M68k %s\n",(data & 0x10) ? "off" : "on");
		//machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(saturn_state::smpc_sound_enable),this),(m_smpc_ram[0x77] & 0x10) >> 4);
		m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		m_en_68k = ((data & 0x10) >> 4) ^ 1;

		//if(LOG_SMPC) printf("SMPC: ram [0x77] = %02x\n",data);
		m_smpc.PDR2 = (data & 0x60);
	}

	if(offset == 0x7d)
	{
		/*
		---- --x- IOSEL2 direct (1) / control mode (0) port select
		---- ---x IOSEL1 direct (1) / control mode (0) port select
		*/
		m_smpc.IOSEL1 = (data & 1) >> 0;
		m_smpc.IOSEL2 = (data & 2) >> 1;
	}

	if(offset == 0x7f)
	{
		//enable PAD irq & VDP2 external latch for port 1/2
		m_smpc.EXLE1 = (data & 1) >> 0;
		m_smpc.EXLE2 = (data & 2) >> 1;
	}
}

/********************************************
 *
 * Saturn handlers
 *
 *******************************************/

UINT8 saturn_state::smpc_th_control_mode(UINT8 pad_n)
{
	int th;
	const char *const padnames[] = { "JOY1", "JOY2" };
	UINT8 res = 0;

	th = (pad_n == 0) ? ((m_smpc.PDR1>>5) & 3) : ((m_smpc.PDR2>>5) & 3);

	if (LOG_SMPC) printf("SMPC: SH-2 TH control mode, returning pad data %d for phase %d\n",pad_n+1, th);

	switch(th)
	{
		/* TODO: 3D Lemmings bogusly enables TH Control mode, wants this to return the ID, needs HW tests.  */
		case 3:
			res = th<<6;
			res |= 0x14;
			res |= machine().root_device().ioport(padnames[pad_n])->read() & 8; // L
			break;
		case 2:
			res = th<<6;
			//  1 C B Right Left Down Up
			//  WHP actually has a very specific code at 0x6015f30, doesn't like bits 0-1 active here ...
			res|= (((machine().root_device().ioport(padnames[pad_n])->read()>>4)) & 0x30); // C & B
			res|= (((machine().root_device().ioport(padnames[pad_n])->read()>>12)) & 0xc);
			break;
		case 1:
			res = th<<6;
			res |= 0x10;
			res |= (machine().root_device().ioport(padnames[pad_n])->read()>>4) & 0xf; // R, X, Y, Z
			break;
		case 0:
			res = th<<6;
			//  0 Start A 0 0    Down Up
			res|= (((machine().root_device().ioport(padnames[pad_n])->read()>>6)) & 0x30); // Start & A
			res|= (((machine().root_device().ioport(padnames[pad_n])->read()>>12)) & 0x3);
			//  ... and actually wants bits 2 - 3 active here.
			res|= 0xc;
			break;
	}

	return res;
}

UINT8 saturn_state::smpc_direct_mode(UINT8 pad_n)
{
	int hshake;
	const int shift_bit[4] = { 4, 12, 8, 0 };
	const char *const padnames[] = { "JOY1", "JOY2" };

	hshake = (pad_n == 0) ? ((m_smpc.PDR1>>5) & 3) : ((m_smpc.PDR2>>5) & 3);

	if (LOG_SMPC) logerror("SMPC: SH-2 direct mode, returning data for phase %d\n", hshake);

	return 0x80 | 0x10 | ((machine().root_device().ioport(padnames[pad_n])->read()>>shift_bit[hshake]) & 0xf);
}

READ8_MEMBER( saturn_state::saturn_SMPC_r )
{
	UINT8 return_data = 0;

	if (!(offset & 1)) // avoid reading to even bytes (TODO: is it 0s or 1s?)
		return 0x00;

	if(offset >= 0x21 && offset <= 0x5f)
		return_data = m_smpc.OREG[(offset-0x21) >> 1];

	if (offset == 0x61)
		return_data = m_smpc.SR;

	if (offset == 0x63)
	{
		//printf("SF %d %d\n",machine().first_screen()->hpos(),machine().first_screen()->vpos());
		return_data = m_smpc.SF;
	}

	if (offset == 0x75 || offset == 0x77)//PDR1/2 read
	{
		if ((m_smpc.IOSEL1 && offset == 0x75) || (m_smpc.IOSEL2 && offset == 0x77))
		{
			UINT8 cur_ddr;

			if(machine().root_device().ioport("INPUT_TYPE")->read() && !(space.debugger_access()))
			{
				popmessage("Warning: read with SH-2 direct mode with a non-pad device");
				return 0;
			}

			cur_ddr = (offset == 0x75) ? m_smpc.DDR1 : m_smpc.DDR2;

			switch(cur_ddr & 0x60)
			{
				case 0x00: break; // in diag test
				case 0x40: return_data = smpc_th_control_mode(offset == 0x77); break;
				case 0x60: return_data = smpc_direct_mode(offset == 0x77); break;
				default:
					popmessage("SMPC: unemulated control method %02x, contact MAMEdev",cur_ddr & 0x60);
					return_data = 0;
					break;
			}
		}
	}

	if (LOG_SMPC) logerror ("cpu %s (PC=%08X) SMPC: Read from Byte Offset %02x (%d) Returns %02x\n", space.device().tag().c_str(), space.device().safe_pc(), offset, offset>>1, return_data);

	return return_data;
}

WRITE8_MEMBER( saturn_state::saturn_SMPC_w )
{
	if (LOG_SMPC) logerror ("8-bit SMPC Write to Offset %02x (reg %d) with Data %02x\n", offset, offset>>1, data);

	if (!(offset & 1)) // avoid writing to even bytes
		return;

	if(offset >= 1 && offset <= 0xd)
		m_smpc.IREG[offset >> 1] = data;

	if(offset == 1) //IREG0, check if a BREAK / CONTINUE request for INTBACK command
	{
		if(m_smpc.intback_stage)
		{
			if(data & 0x40)
			{
				if(LOG_PAD_CMD) printf("SMPC: BREAK request %02x\n",data);
				m_smpc.SR &= 0x0f;
				m_smpc.intback_stage = 0;
			}
			else if(data & 0x80)
			{
				if(LOG_PAD_CMD) printf("SMPC: CONTINUE request %02x\n",data);
				machine().scheduler().timer_set(attotime::from_usec(700), timer_expired_delegate(FUNC(saturn_state::intback_peripheral),this),0); /* TODO: is timing correct? */
				m_smpc.OREG[31] = 0x10;
				m_smpc.SF = 0x01; //TODO: set hand-shake flag?
			}
		}
	}

	if (offset == 0x1f)
	{
		smpc_comreg_exec(space,data,0);

		// we've processed the command, clear status flag
		if(data != 0x10 && data != 2 && data != 3 && data != 6 && data != 7 && data != 0x08 && data != 0x09 && data != 0x0e && data != 0x0f && data != 0x19 && data != 0x1a)
		{
			m_smpc.OREG[31] = data; //read-back for last command issued
			m_smpc.SF = 0x00; //clear hand-shake flag
		}
		/*TODO:emulate the timing of each command...*/
	}

	if (offset == 0x63)
		m_smpc.SF = data & 1; // hand-shake flag

	if(offset == 0x75)  // PDR1
		m_smpc.PDR1 = data & 0x7f;

	if(offset == 0x77)  // PDR2
		m_smpc.PDR2 = data & 0x7f;

	if(offset == 0x79)
		m_smpc.DDR1 = data & 0x7f;

	if(offset == 0x7b)
		m_smpc.DDR2 = data & 0x7f;

	if(offset == 0x7d)
	{
		m_smpc.IOSEL1 = data & 1;
		m_smpc.IOSEL2 = (data & 2) >> 1;
	}

	if(offset == 0x7f)
	{
		//enable PAD irq & VDP2 external latch for port 1/2
		m_smpc.EXLE1 = (data & 1) >> 0;
		m_smpc.EXLE2 = (data & 2) >> 1;
	}
}
