// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/**************************************************************************************

    Sega Saturn (c) 1994 Sega

    @todo List of things that needs to be implemented:
    - There's definitely an ack mechanism in SCU irqs. This is almost surely done via
      the ISM register (i.e. going 0->1 to the given bit acks it).
    - There might be a delay to exactly when SCU irqs happens. This is due to the basic
      fact that SCU runs at 14-ish MHz, so it needs some time before actually firing the
      irq.
    - Vblank-Out actually happens at the last screen line, not at 0.
    - VDP2 V counter has a similar roll-back as MD correspondent register:
      vpos line 0 == 0x1ff (Vblank-Out happens here)
      vpos line 1 == 0
      ...
      vpos line 241 == 0xf0 (Vblank-In happens here)
      vpos line 246 == 0xf5
      vpos line 247 == 0x1ef (rolls back here)
      vpos line 263 == 0x1ff again
    - HBlank bit seems to follow a normal logic instead.
    - Timer 0 doesn't work if the TENB bit isn't enabled (documentation is a bit fussy
      over this).
    - Timer 0 fires at the HBlank-In signal, not before.
    - VDP2 H Counter actually counts x2 in non Hi-Res mode.
    - Timer 1 is definitely annoying. Starts from H-Blank signal and starts counting from
      that position.
      H counter value 0x282 (642) -> timer 1 fires at setting 1
      H counter value 0x284 (644) -> 2
      H counter value 0x2a0 (672) -> 0x10
      H counter value 0x2c0 (704) -> 0x20
      H counter value 0x300 (768) -> 0x40
      H counter value 0x340 (832) -> 0x60
      H counter value 0x352 (850) -> 0x69
      H counter value 0x000 (0)   -> 0x6a, V counter goes +1 here (max range?)
      H counter value 0x02c (44)  -> 0x80
      H counter value 0x0ec (236) -> 0xe0
      H counter value 0x12c (300) -> 0x100
    - Timer 1 seems to count backwards compared to Timer 0 from setting 0x6b onward.
    - Yabause claims that if VDP2 DISP bit isn't enabled then vblank irqs (hblank too?)
      doesn't happen.

**************************************************************************************/

#include "emu.h"
#include "includes/saturn.h"
#include "cpu/sh/sh2.h"
#include "cpu/scudsp/scudsp.h"

/* TODO: do this in a verboselog style */
#define LOG_CDB  0
#define LOG_SCU  1
#define LOG_IRQ  0
#define LOG_IOGA 0



/**************************************************************************************/

WRITE16_MEMBER(saturn_state::saturn_soundram_w)
{
	//machine().scheduler().synchronize(); // force resync

	COMBINE_DATA(&m_sound_ram[offset]);
}

READ16_MEMBER(saturn_state::saturn_soundram_r)
{
	//machine().scheduler().synchronize(); // force resync

	return m_sound_ram[offset];
}

/* communication,SLAVE CPU acquires data from the MASTER CPU and triggers an irq.  */
WRITE32_MEMBER(saturn_state::minit_w)
{
	//logerror("%s MINIT write = %08x\n", machine().describe_context(),data);
	machine().scheduler().boost_interleave(m_minit_boost_timeslice, attotime::from_usec(m_minit_boost));
	machine().scheduler().trigger(1000);
	machine().scheduler().synchronize(); // force resync
	m_slave->pulse_frt_input();
}

WRITE32_MEMBER(saturn_state::sinit_w)
{
	//logerror("%s SINIT write = %08x\n", machine().describe_context(),data);
	machine().scheduler().boost_interleave(m_sinit_boost_timeslice, attotime::from_usec(m_sinit_boost));
	machine().scheduler().synchronize(); // force resync
	m_maincpu->pulse_frt_input();
}

/*
TODO:
Some games seems to not like either MAME's interleave system and/or SH-2 DRC, causing an hard crash.
Reported games are:
Blast Wind (before FMV)
Choro Q Park (car selection)
060311E4: MOV.L R14,@-SP ;R14 = 0x60ffba0 / R15 = 0x60ffba0
060311E6: MOV SP,R14 ;R14 = 0x60ffba0 / R15 = 0x60ffb9c / [0x60ffb9c] <- 0x60ffba0
060311E8: MOV.L @SP+,R14 ;R14 = 0x60ffb9c / R15 = 0x60ffb9c / [0x60ffb9c] -> R14
060311EA: RTS ;R14 = 0x60ffba0 / R15 = 0x60ffba0
060311EC: NOP
06031734: MULS.W R9, R8 ;R14 = 0x60ffba0 / R15 = 0x60ffba0 / EA = 0x60311E4
on DRC this becomes:
R14 0x6031b78 (cause of the crash later on), R15 = 0x60ffba4 and EA = 0

Shinrei Jusatsushi Taromaru (options menu)

*/

WRITE32_MEMBER(saturn_state::saturn_minit_w)
{
	//logerror("%s MINIT write = %08x\n", machine().describe_context(),data);
	if(m_fake_comms->read() & 1)
		machine().scheduler().synchronize(); // force resync
	else
	{
		machine().scheduler().boost_interleave(m_minit_boost_timeslice, attotime::from_usec(m_minit_boost));
		machine().scheduler().trigger(1000);
	}

	m_slave->pulse_frt_input();
}

WRITE32_MEMBER(saturn_state::saturn_sinit_w)
{
	//logerror("%s SINIT write = %08x\n", machine().describe_context(),data);
	if(m_fake_comms->read() & 1)
		machine().scheduler().synchronize(); // force resync
	else
		machine().scheduler().boost_interleave(m_sinit_boost_timeslice, attotime::from_usec(m_sinit_boost));

	m_maincpu->pulse_frt_input();
}


READ8_MEMBER(saturn_state::saturn_backupram_r)
{
	if(!(offset & 1))
		return 0; // yes, it makes sure the "holes" are there.

	return m_backupram[offset >> 1] & 0xff;
}

WRITE8_MEMBER(saturn_state::saturn_backupram_w)
{
	if(!(offset & 1))
		return;

	m_backupram[offset >> 1] = data;
}


WRITE_LINE_MEMBER(saturn_state::m68k_reset_callback)
{
	m_smpc_hle->m68k_reset_trigger();

	printf("m68k RESET opcode triggered\n");
}

WRITE8_MEMBER(saturn_state::scsp_irq)
{
	// don't bother the 68k if it's off
	if (!m_en_68k)
	{
		return;
	}

	if (offset != 0)
	{
		if (data == ASSERT_LINE) m_scsp_last_line = offset;
		m_audiocpu->set_input_line(offset, data);
	}
	else
	{
		m_audiocpu->set_input_line(m_scsp_last_line, data);
	}
}


/*
(Preliminary) explanation about this:
VBLANK-OUT is used at the start of the vblank period.It also sets the timer zero
variable to 0.
If the Timer Compare register is zero too,the Timer 0 irq is triggered.

HBLANK-IN is used at the end of each scanline except when in VBLANK-IN/OUT periods.

The timer 0 is also incremented by one at each HBLANK and checked with the value
of the Timer Compare register;if equal,the timer 0 irq is triggered here too.
Notice that the timer 0 compare register can be more than the VBLANK maximum range,in
this case the timer 0 irq is simply never triggered.This is a known Sega Saturn/ST-V "bug".

VBLANK-IN is used at the end of the vblank period.

SCU register[36] is the timer zero compare register.
SCU register[40] is for IRQ masking.

TODO:
- VDP1 timing and CEF emulation isn't accurate at all.
*/


TIMER_DEVICE_CALLBACK_MEMBER(saturn_state::saturn_scanline)
{
	int scanline = param;
	int y_step,vblank_line;

	vblank_line = get_vblank_start_position();
	y_step = get_ystep_count();

	//popmessage("%08x %d T0 %d T1 %d %08x",m_scu.ism ^ 0xffffffff,max_y,m_scu_regs[36],m_scu_regs[37],m_scu_regs[38]);

	if(scanline == 0*y_step)
	{
		m_scu->vblank_out_w(1);
	}
	else if(scanline == vblank_line*y_step)
	{
		m_scu->vblank_in_w(1);

		// flip odd bit here
		m_vdp2.odd ^= 1;
		/* TODO: when Automatic Draw actually happens? Night Striker S is very fussy on this, and it looks like that VDP1 starts at more or less vblank-in time ... */
		video_update_vdp1();
	}
	else if((scanline % y_step) == 0 && scanline < vblank_line*y_step)
	{
		m_scu->hblank_in_w(1);
	}

	if(scanline == (vblank_line+1)*y_step)
	{
		/* docs mentions that VBE happens one line after vblank-in. */
		if(STV_VDP1_VBE)
			m_vdp1.framebuffer_clear_on_next_frame = 1;
	}

	// TODO: temporary for Batman Forever, presumably anonymous timer not behaving well.
	//       VDP1 timing needs some HW work anyway so I'm currently firing VDP1 after 8 scanlines for now, will de-anon the timers in a later stage.
	if(scanline == (vblank_line+8)*y_step)
	{
		m_scu->vdp1_end_w(1);
	}

	m_scu->check_scanline_timers(scanline, y_step);
}

TIMER_DEVICE_CALLBACK_MEMBER(saturn_state::saturn_slave_scanline )
{
	int scanline = param;
	int y_step,vblank_line;

	vblank_line = get_vblank_start_position();
	y_step = get_ystep_count();

	if(scanline == vblank_line*y_step)
		m_slave->set_input_line_and_vector(0x6, HOLD_LINE, 0x43); // SH2
	else if((scanline % y_step) == 0 && scanline < vblank_line*y_step)
		m_slave->set_input_line_and_vector(0x2, HOLD_LINE, 0x41); // SH2
}

static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	0x100000/(32*8/8),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	0x100000/(32*32/8),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		32*8+0, 32*8+4, 32*8+8, 32*8+12, 32*8+16, 32*8+20, 32*8+24, 32*8+28,

		},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		32*16, 32*17,32*18, 32*19,32*20,32*21,32*22,32*23

		},
	32*32
};

static const gfx_layout tiles8x8x8_layout =
{
	8,8,
	0x100000/(32*8/8),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	32*8    /* really 64*8, but granularity is 32 bytes */
};

static const gfx_layout tiles16x16x8_layout =
{
	16,16,
	0x100000/(64*16/8),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56,
	64*8+0, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8

	},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	64*16, 64*17, 64*18, 64*19, 64*20, 64*21, 64*22, 64*23
	},
	64*16   /* really 128*16, but granularity is 32 bytes */
};




GFXDECODE_START( gfx_stv )
	GFXDECODE_ENTRY( nullptr, 0, tiles8x8x4_layout,   0x00, (0x80*(2+1))  )
	GFXDECODE_ENTRY( nullptr, 0, tiles16x16x4_layout, 0x00, (0x80*(2+1))  )
	GFXDECODE_ENTRY( nullptr, 0, tiles8x8x8_layout,   0x00, (0x08*(2+1))  )
	GFXDECODE_ENTRY( nullptr, 0, tiles16x16x8_layout, 0x00, (0x08*(2+1))  )
GFXDECODE_END


WRITE_LINE_MEMBER( saturn_state::master_sh2_reset_w )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(saturn_state::master_sh2_nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( saturn_state::slave_sh2_reset_w )
{
	m_slave->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
//  m_smpc.slave_on = state;
}

WRITE_LINE_MEMBER( saturn_state::sound_68k_reset_w )
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
	m_en_68k = state ^ 1;
}

// TODO: edge triggered?
WRITE_LINE_MEMBER( saturn_state::system_reset_w )
{
	if(!state)
		return;

	// TODO: actually send a device reset signal to the connected devices
	/*Only backup ram and SMPC ram are retained after that this command is issued.*/
	m_scu->reset();
	memset(m_sound_ram,0x00,0x080000);
	memset(m_workram_h,0x00,0x100000);
	memset(m_workram_l,0x00,0x100000);
	memset(m_vdp2_regs.get(),0x00,0x040000);
	memset(m_vdp2_vram.get(),0x00,0x100000);
	memset(m_vdp2_cram.get(),0x00,0x080000);
	memset(m_vdp1_vram.get(),0x00,0x100000);
	//A-Bus
}

WRITE_LINE_MEMBER(saturn_state::system_halt_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_slave->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(saturn_state::dot_select_w)
{
	const XTAL &xtal = state ? MASTER_CLOCK_320 : MASTER_CLOCK_352;

	m_maincpu->set_unscaled_clock(xtal/2);
	m_slave->set_unscaled_clock(xtal/2);

	m_vdp2.dotsel = state ^ 1;
	stv_vdp2_dynamic_res_change();
}


