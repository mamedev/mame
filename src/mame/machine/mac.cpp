// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/****************************************************************************

    machine/mac.c

    Mac hardware - Mac 128k, 512k, 512ke, Plus, SE, Classic, II, PowerBook (SCSI, SCC, ADB, etc)

    Nate Woods
    Ernesto Corvi
    Raphael Nabet
    R. Belmont

    Mac Model Feature Summary:
                                CPU             FDC     Kbd/Mouse  PRAM     Video
         - Mac 128k             68k             IWM     orig       orig     Original
         - Mac 512k             68k             IWM     orig       orig     Original
         - Mac 512ke            68k             IWM     orig       orig     Original
         - Mac Plus             68k             IWM     orig       ext      Original
         - Mac SE               68k             IWM     MacII ADB  ext      Original
         - Mac Classic          68k             SWIM    MacII ADB  ext      Original
         - Mac Portable         68k (16 MHz)    SWIM    ADB-PMU    PMU      640x400 B&W
         - PowerBook 100        68k (16 MHz)    SWIM    ADB-PMU    PMU      640x400 B&W
         - Mac II               020             IWM     MacII ADB  ext      NuBus card
         - Mac IIx              030             SWIM    MacII ADB  ext      NuBus card
         - Mac IIfx             030             SWIM    IOP ADB    ext      NuBus card
         - Mac SE/30            030             SWIM    MacII ADB  ext      Internal fake NuBus card
         - Mac IIcx             030             SWIM    MacII ADB  ext      NuBus card
         - Mac IIci             030             SWIM    MacII ADB  ext      Internal "RBV" type
         - Mac IIsi             030             SWIM    Egret ADB  n/a      Internal "RBV" type
         - PowerBook 140/145(B) 030 (16/25 MHz) SWIM    ADB-PMU    PMU      640x400 B&W (passive matrix)
         - PowerBook 170        030 (25 MHz)    SWIM    ADB-PMU    PMU      640x400 B&W (active matrix)
         - Mac IIvx/IIvi        030             SWIM    Egret ADB  n/a      Internal "VASP" type
         - Mac LC               020             SWIM    Egret ADB  n/a      Internal "V8" type
         - Mac LC II            030             SWIM    Egret ADB  n/a      Internal "V8" type
         - Mac LC III           030             SWIM    Egret ADB  n/a      Internal "Sonora" type
         - Mac Classic II       030             SWIM    Egret ADB  n/a      Internal "Eagle" type (V8 clone)
         - Mac Color Classic    030             SWIM    Cuda ADB   n/a      Internal "Spice" type (V8 clone)
         - Mac Quadra 700       040 (25 MHz)    SWIM II MacII ADB  ext      Internal "DAFB" type
         - Mac Quadra 900       040 (33 MHz)    SWIM II IOP ADB    ext      Internal "DAFB" type

    Notes:
        - The Mac Plus boot code seems to check to see the extent of ROM
          mirroring to determine if SCSI is available.  If the ROM is mirrored,
          then SCSI is not available.  Thanks to R. Belmont for making this
          discovery.
        - On the SE and most later Macs, the first access to ROM turns off the overlay.
          However, the Mac II/IIx/IIcx (and others?) have the old-style VIA overlay control bit!
        - The Mac II can have either a 68551 PMMU fitted or an Apple custom that handles 24 vs. 32
          bit addressing mode.  The ROM is *not* 32-bit clean so Mac OS normally runs in 24 bit mode,
          but A/UX can run 32.
        - There are 5 known kinds of host-side ADB hardware:
          * "Mac II ADB" used in the SE, II, IIx, IIcx, SE/30, IIci, Quadra 610, Quadra 650, Quadra 700,
             Quadra 800, Centris 610 and Centris 650.  This is a bit-banger using the VIA and a simple PIC.
          * "PMU ADB" used in the Mac Portable and all 680x0-based PowerBooks.
          * "Egret ADB" used in the IIsi, IIvi, IIvx, Classic II, LC, LC II, LC III, Performa 460,
             and Performa 600.  This is a 68HC05 with a different internal ROM than CUDA.
          * "IOP ADB" (ADB driven by a 6502 coprocessor, similar to Lisa) used in the IIfx,
            Quadra 900, and Quadra 950.
          * "Cuda ADB" (Apple's CUDA chip, which is a 68HC05 MCU) used in the Color Classic, LC 520,
            LC 55x, LC 57x, LC 58x, Quadra 630, Quadra 660AV, Quadra 840AV, PowerMac 6100/7100/8100,
            IIci, and PowerMac 5200.

     TODO:
        - Call the RTC timer

     Egret version spotting:
     341S0850 - 0x???? (1.01, earlier) - LC, LC II
     341S0851 - 0x0101 (1.01, later) - Classic II, IIsi, IIvx/IIvi, LC III
     344S0100 - 0x0100 (1.00) - Some (early production?) IIsi

     Cuda version spotting:
     341S0262 - 0x0003f200 (3.02) - some PMac 6500, Bondi blue iMac
     341S0285 - No version (x.xx) - PMac 4400 + Mac clones ("Cuda Lite" with 768 bytes more ROM + PS/2 keyboard/mouse support)
     341S0060 - 0x00020028 (2.40) - Performa/Quadra 6xx, PMac 6200, x400, some x500, Pippin, "Gossamer" G3, others?
                                    (verified found in PMac 5500-225, G3-333)
     341S0788 - 0x00020025 (2.37) - LC 475/575/Quadra 605, Quadra 660AV/840AV, PMac 7200
     341S0417 - 0x00020023 (2.35) - Color Classic

     Caboose version spotting:
     341S0853 - 0x0100 (1.00) - Quadra 950

     PG&E (68HC05 PMU) version spotting:
     (find the text "BORG" in the system ROM, the next 32768 bytes are the PG&E image.
      offset +4 in the image is the version byte).
     01 - PowerBook Duo 210/230/250
     02 - PowerBook 540c, PBDuo 270C, PBDuo 280/280C
     03 - PowerBook 150
     08 - PB190cs, PowerBook 540c PPC update, all PowerPC PowerBooks through WallStreet G3s

****************************************************************************/

#include "emu.h"
#include "includes/mac.h"
#include "machine/applefdc.h"
#include "machine/sonydriv.h"
#include "debug/debugcpu.h"
#include "debugger.h"

#define AUDIO_IS_CLASSIC (m_model <= MODEL_MAC_CLASSIC)
#define MAC_HAS_VIA2    ((m_model >= MODEL_MAC_II) && (m_model != MODEL_MAC_IIFX))

#define INTS_RBV    ((m_model >= MODEL_MAC_IICI) && (m_model <= MODEL_MAC_IIVI)) || ((m_model >= MODEL_MAC_LC) && (m_model <= MODEL_MAC_LC_580))

#ifdef MAME_DEBUG
#define LOG_ADB         0
#define LOG_VIA         0
#define LOG_MAC_IWM     0
#define LOG_GENERAL     0
#define LOG_KEYBOARD    0
#define LOG_MEMORY      0
#else
#define LOG_ADB         0
#define LOG_VIA         0
#define LOG_MAC_IWM     0
#define LOG_GENERAL     0
#define LOG_KEYBOARD    0
#define LOG_MEMORY      0
#endif

extern TIMER_CALLBACK(mac_adb_tick);    // macadb.c
extern TIMER_CALLBACK(mac_pmu_tick);    // macadb.c

static offs_t mac_dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);

// returns non-zero if this Mac has ADB
int mac_state::has_adb()
{
	return m_model >= MODEL_MAC_SE;
}

// handle disk enable lines
void mac_fdc_set_enable_lines(device_t *device, int enable_mask)
{
	mac_state *mac = device->machine().driver_data<mac_state>();

	if (mac->m_model != MODEL_MAC_SE)
	{
		sony_set_enable_lines(device, enable_mask);
	}
	else
	{
		if (enable_mask)
		{
			sony_set_enable_lines(device, mac->m_drive_select ? 1 : 2);
		}
		else
		{
			sony_set_enable_lines(device, enable_mask);
		}
	}
}

void mac_state::mac_install_memory(offs_t memory_begin, offs_t memory_end,
	offs_t memory_size, void *memory_data, int is_rom, const char *bank)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	offs_t memory_mask;

	memory_size = MIN(memory_size, (memory_end + 1 - memory_begin));
	memory_mask = memory_size - 1;

	if (!is_rom)
	{
		space.install_readwrite_bank(memory_begin, memory_end, memory_mask, 0, bank);
	}
	else
	{
		space.unmap_write(memory_begin, memory_end, memory_mask, 0);
		space.install_read_bank(memory_begin, memory_end, memory_mask, 0, bank);
	}

	membank(bank)->set_base(memory_data);

	if (LOG_MEMORY)
	{
		printf("mac_install_memory(): bank=%s range=[0x%06x...0x%06x] mask=0x%06x ptr=0x%p\n",
			bank, memory_begin, memory_end, memory_mask, memory_data);
	}
}



/*
    Interrupt handling
*/

void mac_state::field_interrupts()
{
	int take_interrupt = -1;

	if (m_model < MODEL_MAC_PORTABLE)
	{
		if ((m_scc_interrupt) || (m_scsi_interrupt))
		{
			take_interrupt = 2;
		}
		else if (m_via_interrupt)
		{
			take_interrupt = 1;
		}
	}
	else if ((m_model == MODEL_MAC_PORTABLE) || (m_model == MODEL_MAC_PB100))
	{
		if ((m_scc_interrupt) || (m_asc_interrupt))
		{
			take_interrupt = 2;
		}
		else if (m_via_interrupt)
		{
			take_interrupt = 1;
		}
	}
	else if ((m_model < MODEL_MAC_POWERMAC_6100) && (m_model != MODEL_MAC_IIFX))
	{
		if (m_scc_interrupt)
		{
			take_interrupt = 4;
		}
		else if (m_via2_interrupt)
		{
			take_interrupt = 2;
		}
		else if (m_via_interrupt)
		{
			take_interrupt = 1;
		}
	}
	else
	{
		return; // no interrupts for PowerPC yet
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

WRITE_LINE_MEMBER(mac_state::set_scc_interrupt)
{
	m_scc_interrupt = state;
	this->field_interrupts();
}

void mac_state::set_via_interrupt(int value)
{
	m_via_interrupt = value;
	this->field_interrupts();
}


void mac_state::set_via2_interrupt(int value)
{
	m_via2_interrupt = value;
	this->field_interrupts();
}

WRITE_LINE_MEMBER(mac_state::mac_asc_irq)
{
	if (INTS_RBV)
	{
		if (state == ASSERT_LINE)
		{
			m_rbv_regs[3] |= 0x10; // any VIA 2 interrupt | sound interrupt
			rbv_recalc_irqs();
		}
		else
		{
			m_rbv_regs[3] &= ~0x10;
			rbv_recalc_irqs();
		}
	}
	else if ((m_model == MODEL_MAC_PORTABLE) || (m_model == MODEL_MAC_PB100))
	{
//      m_asc_interrupt = state;
//      field_interrupts();
	}
	else if ((m_model >= MODEL_MAC_II) && (m_model != MODEL_MAC_IIFX))
	{
		m_via2->write_cb1(state^1);
	}
}

WRITE16_MEMBER ( mac_state::mac_autovector_w )
{
	if (LOG_GENERAL)
		logerror("mac_autovector_w: offset=0x%08x data=0x%04x\n", offset, data);

	/* This should throw an exception */

	/* Not yet implemented */
}

READ16_MEMBER ( mac_state::mac_autovector_r )
{
	if (LOG_GENERAL)
		logerror("mac_autovector_r: offset=0x%08x\n", offset);

	/* This should throw an exception */

	/* Not yet implemented */
	return 0;
}

void mac_state::set_scc_waitrequest(int waitrequest)
{
	if (LOG_GENERAL)
		logerror("set_scc_waitrequest: waitrequest=%i\n", waitrequest);

	/* Not Yet Implemented */
}

void mac_state::v8_resize()
{
	offs_t memory_size;
	UINT8 *memory_data;
	int is_rom;

	is_rom = (m_overlay) ? 1 : 0;

	// get what memory we're going to map
	if (is_rom)
	{
		/* ROM mirror */
		memory_size = memregion("bootrom")->bytes();
		memory_data = memregion("bootrom")->base();
		is_rom = TRUE;
	}
	else
	{
		/* RAM */
		memory_size = m_ram->size();
		memory_data = m_ram->pointer();
		is_rom = FALSE;
	}

//    printf("mac_v8_resize: memory_size = %x, ctrl bits %02x (overlay %d = %s)\n", memory_size, m_rbv_regs[1] & 0xe0, m_overlay, is_rom ? "ROM" : "RAM");

	if (is_rom)
	{
		mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom, "bank1");
		
		// install catcher in place of ROM that will detect the first access to ROM in its real location
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xa00000, 0xafffff, read32_delegate(FUNC(mac_state::rom_switch_r), this), 0xffffffff);
	}
	else
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		UINT32 onboard_amt, simm_amt, simm_size;
		static const UINT32 simm_sizes[4] = { 0, 2*1024*1024, 4*1024*1024, 8*1024*1024 };

		// re-install ROM in its normal place
		size_t rom_mask = memregion("bootrom")->bytes() - 1;
		m_maincpu->space(AS_PROGRAM).install_read_bank(0xa00000, 0xafffff, rom_mask, 0, "bankR");
		membank("bankR")->set_base((void *)memregion("bootrom")->base());

		// force unmap of entire RAM region
		space.unmap_write(0, 0x9fffff, 0x9fffff, 0);

		// LC and Classic II have 2 MB built-in, all other V8-style machines have 4 MB
		// we reserve the first 2 or 4 MB of mess_ram for the onboard,
		// RAM above that mark is the SIMM
		onboard_amt = ((m_model == MODEL_MAC_LC) || (m_model == MODEL_MAC_CLASSIC_II)) ? 2*1024*1024 : 4*1024*1024;
		simm_amt = (m_rbv_regs[1]>>6) & 3;  // size of SIMM RAM window
		simm_size = memory_size - onboard_amt;  // actual amount of RAM available for SIMMs

		// installing SIMM RAM?
		if (simm_amt != 0)
		{
//            printf("mac_v8_resize: SIMM region size is %x, SIMM size is %x, onboard size is %x\n", simm_sizes[simm_amt], simm_size, onboard_amt);

			if ((simm_amt > 0) && (simm_size > 0))
			{
//              mac_install_memory(0x000000, simm_sizes[simm_amt]-1, simm_sizes[simm_amt], memory_data + onboard_amt, is_rom, "bank1");
				mac_install_memory(0x000000, simm_size-1, simm_size, memory_data + onboard_amt, is_rom, "bank1");
			}

			// onboard RAM sits immediately above the SIMM, if any
			if (simm_sizes[simm_amt] + onboard_amt <= 0x800000)
			{
				mac_install_memory(simm_sizes[simm_amt], simm_sizes[simm_amt] + onboard_amt - 1, onboard_amt, memory_data, is_rom, "bank2");
			}

			// a mirror of the first 2 MB of on board RAM always lives at 0x800000
			mac_install_memory(0x800000, 0x9fffff, 0x200000, memory_data, is_rom, "bank3");
		}
		else
		{
//          printf("mac_v8_resize: SIMM off, mobo RAM at 0 and top\n");

			mac_install_memory(0x000000, onboard_amt-1, onboard_amt, memory_data, is_rom, "bank1");
			mac_install_memory(0x900000, 0x9fffff, 0x200000, memory_data+0x100000, is_rom, "bank3");
		}
	}
}

void mac_state::set_memory_overlay(int overlay)
{
	offs_t memory_size;
	UINT8 *memory_data;
	int is_rom;

	/* normalize overlay */
	overlay = overlay ? TRUE : FALSE;

	if (overlay != m_overlay)
	{
		/* set up either main RAM area or ROM mirror at 0x000000-0x3fffff */
		if (overlay)
		{
			/* ROM mirror */
			memory_size = memregion("bootrom")->bytes();
			memory_data = memregion("bootrom")->base();
			is_rom = TRUE;
		}
		else
		{
			/* RAM */
			memory_size = m_ram->size();
			memory_data = m_ram->pointer();
			is_rom = FALSE;
		}

		/* install the memory */
		if (((m_model >= MODEL_MAC_LC) && (m_model <= MODEL_MAC_COLOR_CLASSIC) && ((m_model != MODEL_MAC_LC_III) && (m_model != MODEL_MAC_LC_III_PLUS))) || (m_model == MODEL_MAC_CLASSIC_II))
		{
			m_overlay = overlay;
			v8_resize();
		}
		else if ((m_model == MODEL_MAC_IICI) || (m_model == MODEL_MAC_IISI))
		{
			// ROM is OK to flood to 3fffffff
			if (is_rom)
			{
				mac_install_memory(0x00000000, 0x3fffffff, memory_size, memory_data, is_rom, "bank1");
			}
			else    // RAM: be careful not to populate ram B with a mirror or the ROM will get confused
			{
				mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom, "bank1");
			}
		}
		else if ((m_model == MODEL_MAC_PORTABLE) || (m_model == MODEL_MAC_PB100) || (m_model == MODEL_MAC_IIVX) || (m_model == MODEL_MAC_IIFX))
		{
			address_space& space = m_maincpu->space(AS_PROGRAM);
			space.unmap_write(0x000000, 0x9fffff, 0x9fffff, 0);
			mac_install_memory(0x000000, memory_size-1, memory_size, memory_data, is_rom, "bank1");
		}
		else if ((m_model == MODEL_MAC_PB140) || (m_model == MODEL_MAC_PB160) || ((m_model >= MODEL_MAC_PBDUO_210) && (m_model <= MODEL_MAC_PBDUO_270c)))
		{
			address_space& space = m_maincpu->space(AS_PROGRAM);
			space.unmap_write(0x000000, 0xffffff, 0xffffff, 0);
			mac_install_memory(0x000000, memory_size-1, memory_size, memory_data, is_rom, "bank1");
		}
		else if ((m_model >= MODEL_MAC_II) && (m_model <= MODEL_MAC_SE30))
		{
			mac_install_memory(0x00000000, 0x3fffffff, memory_size, memory_data, is_rom, "bank1");
		}
		else if ((m_model == MODEL_MAC_LC_III) || (m_model == MODEL_MAC_LC_III_PLUS) || (m_model >= MODEL_MAC_LC_475 && m_model <= MODEL_MAC_LC_580))   // up to 36 MB
		{
			mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom, "bank1");

			if (is_rom)
			{
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x40000000, 0x4fffffff, read32_delegate(FUNC(mac_state::rom_switch_r), this), 0xffffffff);
			}
			else
			{
				size_t rom_mask = memregion("bootrom")->bytes() - 1;
				m_maincpu->space(AS_PROGRAM).install_read_bank(0x40000000, 0x4fffffff, rom_mask, 0, "bankR");
				membank("bankR")->set_base((void *)memregion("bootrom")->base());
			}
		}
		else if (m_model == MODEL_MAC_QUADRA_700)
		{
			mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom, "bank1");
		}
		else
		{
			mac_install_memory(0x00000000, 0x003fffff, memory_size, memory_data, is_rom, "bank1");
		}

		m_overlay = overlay;

		if (LOG_GENERAL)
			logerror("mac_set_memory_overlay: overlay=%i\n", overlay);
	}
}

READ32_MEMBER(mac_state::rom_switch_r)
{
	offs_t ROM_size = memregion("bootrom")->bytes(); 
	UINT32 *ROM_data = (UINT32 *)memregion("bootrom")->base();

	// disable the overlay
	if (m_overlay)
	{
		set_memory_overlay(0);
	}

//	printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, ROM_size-1, offset & ((ROM_size - 1)>>2));

	return ROM_data[offset & ((ROM_size - 1)>>2)];
}


/*
    R Nabet 000531 : added keyboard code
*/

/* *************************************************************************
 * non-ADB keyboard support
 *
 * The keyboard uses a i8021 (?) microcontroller.
 * It uses a bidirectional synchonous serial line, connected to the VIA (SR feature)
 *
 * Our emulation is more a hack than anything else - the keyboard controller is
 * not emulated, instead we interpret keyboard commands directly.  I made
 * many guesses, which may be wrong
 *
 * todo :
 * * find the correct model number for the Mac Plus keyboard ?
 * * emulate original Macintosh keyboards (2 layouts : US and international)
 *
 * references :
 * * IM III-29 through III-32 and III-39 through III-42
 * * IM IV-250
 * *************************************************************************/

/*
    scan_keyboard()

    scan the keyboard, and returns key transition code (or NULL ($7B) if none)
*/
#ifndef MAC_USE_EMULATED_KBD
int mac_state::scan_keyboard()
{
	int i, j;
	int keybuf = 0;
	int keycode;
	ioport_port *ports[7] = { m_key0, m_key1, m_key2, m_key3, m_key4, m_key5, m_key6 };

	if (m_keycode_buf_index)
	{
		return m_keycode_buf[--m_keycode_buf_index];
	}

	for (i=0; i<7; i++)
	{
		keybuf = ports[i]->read();

		if (keybuf != m_key_matrix[i])
		{
			/* if state has changed, find first bit which has changed */
			if (LOG_KEYBOARD)
				logerror("keyboard state changed, %d %X\n", i, keybuf);

			for (j=0; j<16; j++)
			{
				if (((keybuf ^ m_key_matrix[i]) >> j) & 1)
				{
					/* update m_key_matrix */
					m_key_matrix[i] = (m_key_matrix[i] & ~ (1 << j)) | (keybuf & (1 << j));

					if (i < 4)
					{
						/* create key code */
						keycode = (i << 5) | (j << 1) | 0x01;
						if (! (keybuf & (1 << j)))
						{
							/* key up */
							keycode |= 0x80;
						}
						return keycode;
					}
					else if (i < 6)
					{
						/* create key code */
						keycode = ((i & 3) << 5) | (j << 1) | 0x01;

						if ((keycode == 0x05) || (keycode == 0x0d) || (keycode == 0x11) || (keycode == 0x1b))
						{
							/* these keys cause shift to be pressed (for compatibility with mac 128/512) */
							if (keybuf & (1 << j))
							{
								/* key down */
								if (! (m_key_matrix[3] & 0x0100))
								{
									/* shift key is really up */
									m_keycode_buf[0] = keycode;
									m_keycode_buf[1] = 0x79;
									m_keycode_buf_index = 2;
									return 0x71;    /* "presses" shift down */
								}
							}
							else
							{   /* key up */
								if (! (m_key_matrix[3] & 0x0100))
								{
									/* shift key is really up */
									m_keycode_buf[0] = keycode | 0x80;
									m_keycode_buf[1] = 0x79;
									m_keycode_buf_index = 2;
									return 0xF1;    /* "releases" shift */
								}
							}
						}

						if (! (keybuf & (1 << j)))
						{
							/* key up */
							keycode |= 0x80;
						}
						m_keycode_buf[0] = keycode;
						m_keycode_buf_index = 1;
						return 0x79;
					}
					else /* i == 6 */
					{
						/* create key code */
						keycode = (j << 1) | 0x01;
						if (! (keybuf & (1 << j)))
						{
							/* key up */
							keycode |= 0x80;
						}
						m_keycode_buf[0] = keycode;
						m_keycode_buf_index = 1;
						return 0x79;
					}
				}
			}
		}
	}

	return 0x7B;    /* return NULL */
}

/*
    power-up init
*/
void mac_state::keyboard_init()
{
	int i;

	/* init flag */
	m_kbd_comm = FALSE;
	m_kbd_receive = FALSE;
	m_kbd_shift_reg=0;
	m_kbd_shift_count=0;

	/* clear key matrix */
	for (i=0; i<7; i++)
	{
		m_key_matrix[i] = 0;
	}

	/* purge transmission buffer */
	m_keycode_buf_index = 0;
}
#endif

/******************* Keyboard <-> VIA communication ***********************/

#ifdef MAC_USE_EMULATED_KBD

WRITE_LINE_MEMBER(mac_state::mac_kbd_clk_in)
{
	printf("CLK: %d\n", state^1);
	m_via1->write_cb1(state ? 0 : 1);
}

WRITE_LINE_MEMBER(mac_state::mac_via_out_cb2)
{
	printf("Sending %d to kbd (PC=%x)\n", data, m_maincpu->pc());
	m_mackbd->data_w((data & 1) ? ASSERT_LINE : CLEAR_LINE);
}

#else   // keyboard HLE

TIMER_CALLBACK_MEMBER(mac_state::kbd_clock)
{
	int i;

	if (m_kbd_comm == TRUE)
	{
		for (i=0; i<8; i++)
		{
			/* Put data on CB2 if we are sending*/
			if (m_kbd_receive == FALSE)
				m_via1->write_cb2(m_kbd_shift_reg&0x80?1:0);
			m_kbd_shift_reg <<= 1;
			m_via1->write_cb1(0);
			m_via1->write_cb1(1);
		}
		if (m_kbd_receive == TRUE)
		{
			m_kbd_receive = FALSE;
			/* Process the command received from mac */
			keyboard_receive(m_kbd_shift_reg & 0xff);
		}
		else
		{
			/* Communication is over */
			m_kbd_comm = FALSE;
		}
	}
}

void mac_state::kbd_shift_out(int data)
{
	if (m_kbd_comm == TRUE)
	{
		m_kbd_shift_reg = data;
		machine().scheduler().timer_set(attotime::from_msec(1), timer_expired_delegate(FUNC(mac_state::kbd_clock),this));
	}
}

WRITE_LINE_MEMBER(mac_state::mac_via_out_cb2)
{
	if (m_kbd_comm == FALSE && state == 0)
	{
		/* Mac pulls CB2 down to initiate communication */
		m_kbd_comm = TRUE;
		m_kbd_receive = TRUE;
		machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(mac_state::kbd_clock),this));
	}
	if (m_kbd_comm == TRUE && m_kbd_receive == TRUE)
	{
		/* Shift in what mac is sending */
		m_kbd_shift_reg = (m_kbd_shift_reg & ~1) | state;
	}
}

/*
    called when inquiry times out (1/4s)
*/
TIMER_CALLBACK_MEMBER(mac_state::inquiry_timeout_func)
{
	if (LOG_KEYBOARD)
		logerror("keyboard enquiry timeout\n");
	kbd_shift_out(0x7B); /* always send NULL */
}

/*
    called when a command is received from the mac
*/
void mac_state::keyboard_receive(int val)
{
	switch (val)
	{
	case 0x10:
		/* inquiry - returns key transition code, or NULL ($7B) if time out (1/4s) */
		if (LOG_KEYBOARD)
			logerror("keyboard command : inquiry\n");

		m_inquiry_timeout->adjust(
			attotime(0, DOUBLE_TO_ATTOSECONDS(0.25)), 0);
		break;

	case 0x14:
		/* instant - returns key transition code, or NULL ($7B) */
		if (LOG_KEYBOARD)
			logerror("keyboard command : instant\n");

		kbd_shift_out(scan_keyboard());
		break;

	case 0x16:
		/* model number - resets keyboard, return model number */
		if (LOG_KEYBOARD)
			logerror("keyboard command : model number\n");

		{   /* reset */
			int i;

			/* clear key matrix */
			for (i=0; i<7; i++)
			{
				m_key_matrix[i] = 0;
			}

			/* purge transmission buffer */
			m_keycode_buf_index = 0;
		}

		/* format : 1 if another device (-> keypad ?) connected | next device (-> keypad ?) number 1-8
		                    | keyboard model number 1-8 | 1  */
		/* keyboards :
		    3 : mac 512k, US and international layout ? Mac plus ???
		    other values : Apple II keyboards ?
		*/
		/* keypads :
		    ??? : standard keypad (always available on Mac Plus) ???
		*/
		kbd_shift_out(0x17);   /* probably wrong */
		break;

	case 0x36:
		/* test - resets keyboard, return ACK ($7D) or NAK ($77) */
		if (LOG_KEYBOARD)
			logerror("keyboard command : test\n");

		kbd_shift_out(0x7D);   /* ACK */
		break;

	default:
		if (LOG_KEYBOARD)
			logerror("unknown keyboard command 0x%X\n", val);

		kbd_shift_out(0);
		break;
	}
}
#endif

/* *************************************************************************
 * Mouse
 * *************************************************************************/

void mac_state::mouse_callback()
{
	int     new_mx, new_my;
	int     x_needs_update = 0, y_needs_update = 0;

	new_mx = m_mouse1->read();
	new_my = m_mouse2->read();

	/* see if it moved in the x coord */
	if (new_mx != last_mx)
	{
		int     diff = new_mx - last_mx;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		count_x += diff;

		last_mx = new_mx;
	}
	/* see if it moved in the y coord */
	if (new_my != last_my)
	{
		int     diff = new_my - last_my;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		count_y += diff;

		last_my = new_my;
	}

	/* update any remaining count and then return */
	if (count_x)
	{
		if (count_x < 0)
		{
			count_x++;
			m_mouse_bit_x = 0;
			x_needs_update = 2;
		}
		else
		{
			count_x--;
			m_mouse_bit_x = 1;
			x_needs_update = 1;
		}
	}
	else if (count_y)
	{
		if (count_y < 0)
		{
			count_y++;
			m_mouse_bit_y = 1;
			y_needs_update = 1;
		}
		else
		{
			count_y--;
			m_mouse_bit_y = 0;
			y_needs_update = 2;
		}
	}

	if (x_needs_update || y_needs_update)
		/* assert Port B External Interrupt on the SCC */
		scc_mouse_irq(x_needs_update, y_needs_update );
}

/* *************************************************************************
 * SCSI
 * *************************************************************************/

/*

From http://www.mac.m68k-linux.org/devel/plushw.php

The address of each register is computed as follows:

  $580drn

  where r represents the register number (from 0 through 7),
  n determines whether it a read or write operation
  (0 for reads, or 1 for writes), and
  d determines whether the DACK signal to the NCR 5380 is asserted.
  (0 for not asserted, 1 is for asserted)

Here's an example of the address expressed in binary:

  0101 1000 0000 00d0 0rrr 000n

Note:  Asserting the DACK signal applies only to write operations to
       the output data register and read operations from the input
       data register.

  Symbolic            Memory
  Location            Location   NCR 5380 Internal Register

  scsiWr+sODR+dackWr  $580201    Output Data Register with DACK
  scsiWr+sODR         $580001    Output Data Register
  scsiWr+sICR         $580011    Initiator Command Register
  scsiWr+sMR          $580021    Mode Register
  scsiWr+sTCR         $580031    Target Command Register
  scsiWr+sSER         $580041    Select Enable Register
  scsiWr+sDMAtx       $580051    Start DMA Send
  scsiWr+sTDMArx      $580061    Start DMA Target Receive
  scsiWr+sIDMArx      $580071    Start DMA Initiator Receive

  scsiRd+sIDR+dackRd  $580260    Current SCSI Data with DACK
  scsiRd+sCDR         $580000    Current SCSI Data
  scsiRd+sICR         $580010    Initiator Command Register
  scsiRd+sMR          $580020    Mode Registor
  scsiRd+sTCR         $580030    Target Command Register
  scsiRd+sCSR         $580040    Current SCSI Bus Status
  scsiRd+sBSR         $580050    Bus and Status Register
  scsiRd+sIDR         $580060    Input Data Register
  scsiRd+sRESET       $580070    Reset Parity/Interrupt
             */

READ16_MEMBER ( mac_state::macplus_scsi_r )
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	if ((reg == 6) && (offset == 0x130))
	{
		reg = R5380_CURDATA_DTACK;
	}

	return m_ncr5380->ncr5380_read_reg(reg)<<8;
}

READ32_MEMBER (mac_state::macii_scsi_drq_r)
{
	switch (mem_mask)
	{
		case 0xff000000:
			return m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<24;

		case 0xffff0000:
			return (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<24) | (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<16);

		case 0xffffffff:
			return (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<24) | (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<16) | (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<8) | m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK);

		default:
			logerror("macii_scsi_drq_r: unknown mem_mask %08x\n", mem_mask);
	}

	return 0;
}

WRITE32_MEMBER (mac_state::macii_scsi_drq_w)
{
	switch (mem_mask)
	{
		case 0xff000000:
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>24);
			break;

		case 0xffff0000:
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>24);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>16);
			break;

		case 0xffffffff:
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>24);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>16);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>8);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data&0xff);
			break;

		default:
			logerror("macii_scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
			break;
	}
}

WRITE16_MEMBER ( mac_state::macplus_scsi_w )
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_w: data %x offset %x mask %x\n", data, offset, mem_mask);

	if ((reg == 0) && (offset == 0x100))
	{
		reg = R5380_OUTDATA_DTACK;
	}

	m_ncr5380->ncr5380_write_reg(reg, data);
}

WRITE16_MEMBER ( mac_state::macii_scsi_w )
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_w: data %x offset %x mask %x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());

	if ((reg == 0) && (offset == 0x100))
	{
		reg = R5380_OUTDATA_DTACK;
	}

	m_ncr5380->ncr5380_write_reg(reg, data>>8);
}

WRITE_LINE_MEMBER(mac_state::mac_scsi_irq)
{
/*  mac_state *mac = machine.driver_data<mac_state>();

    if ((mac->m_scsiirq_enable) && ((mac->m_model == MODEL_MAC_SE) || (mac->m_model == MODEL_MAC_CLASSIC)))
    {
        mac->m_scsi_interrupt = state;
        mac->field_interrupts();
    }*/
}

WRITE_LINE_MEMBER(mac_state::irq_539x_1_w)
{
	if (state)  // make sure a CB1 transition occurs
	{
		m_via2->write_cb2(0);
		m_via2->write_cb2(1);
	}
}

WRITE_LINE_MEMBER(mac_state::drq_539x_1_w)
{
	m_dafb_scsi1_drq = state;
}

/* *************************************************************************
 * SCC
 *
 * Serial Communications Controller
 * *************************************************************************/

void mac_state::scc_mouse_irq(int x, int y)
{
	scc8530_t *scc = machine().device<scc8530_t>("scc");
	static int lasty = 0;
	static int lastx = 0;

	if (x && y)
	{
		if (m_last_was_x) {
			scc->set_status(0x0a);
			if(x == 2) {
				if(lastx) {
					scc->set_reg_a(0, 0x04);
					m_mouse_bit_x = 0;
				} else {
					scc->set_reg_a(0, 0x0C);
					m_mouse_bit_x = 1;
				}
			} else {
				if(lastx) {
					scc->set_reg_a(0, 0x04);
					m_mouse_bit_x = 1;
				} else {
					scc->set_reg_a(0, 0x0C);
					m_mouse_bit_x = 0;
				}
			}
			lastx = !lastx;
		} else {
			scc->set_status(0x02);
			if(y == 2) {
				if(lasty) {
					scc->set_reg_b(0, 0x04);
					m_mouse_bit_y = 0;
				} else {
					scc->set_reg_b(0, 0x0C);
					m_mouse_bit_y = 1;
				}
			} else {
				if(lasty) {
					scc->set_reg_b(0, 0x04);
					m_mouse_bit_y = 1;
				} else {
					scc->set_reg_b(0, 0x0C);
					m_mouse_bit_y = 0;
				}
			}
			lasty = !lasty;
		}

		m_last_was_x ^= 1;
	}
	else
	{
		if (x) {
			scc->set_status(0x0a);
			if(x == 2) {
				if(lastx) {
					scc->set_reg_a(0, 0x04);
					m_mouse_bit_x = 0;
				} else {
					scc->set_reg_a(0, 0x0C);
					m_mouse_bit_x = 1;
				}
			} else {
				if(lastx) {
					scc->set_reg_a(0, 0x04);
					m_mouse_bit_x = 1;
				} else {
					scc->set_reg_a(0, 0x0C);
					m_mouse_bit_x = 0;
				}
			}
			lastx = !lastx;
		} else {
			scc->set_status(0x02);
			if(y == 2) {
				if(lasty) {
					scc->set_reg_b(0, 0x04);
					m_mouse_bit_y = 0;
				} else {
					scc->set_reg_b(0, 0x0C);
					m_mouse_bit_y = 1;
				}
			} else {
				if(lasty) {
					scc->set_reg_b(0, 0x04);
					m_mouse_bit_y = 1;
				} else {
					scc->set_reg_b(0, 0x0C);
					m_mouse_bit_y = 0;
				}
			}
			lasty = !lasty;
		}
	}

	this->set_scc_interrupt(1);
}



READ16_MEMBER ( mac_state::mac_scc_r )
{
	scc8530_t *scc = space.machine().device<scc8530_t>("scc");
	UINT16 result;

	result = scc->reg_r(space, offset);
	return (result << 8) | result;
}



WRITE16_MEMBER ( mac_state::mac_scc_w )
{
	scc8530_t *scc = space.machine().device<scc8530_t>("scc");
	scc->reg_w(space, offset, data);
}

WRITE16_MEMBER ( mac_state::mac_scc_2_w )
{
	scc8530_t *scc = space.machine().device<scc8530_t>("scc");
	scc->reg_w(space, offset, data >> 8);
}

/* ********************************** *
 * IWM Code specific to the Mac Plus  *
 * ********************************** */

READ16_MEMBER ( mac_state::mac_iwm_r )
{
	/* The first time this is called is in a floppy test, which goes from
	 * $400104 to $400126.  After that, all access to the floppy goes through
	 * the disk driver in the MacOS
	 *
	 * I just thought this would be on interest to someone trying to further
	 * this driver along
	 */

	UINT16 result = 0;
	applefdc_base_device *fdc = space.machine().device<applefdc_base_device>("fdc");

	result = fdc->read(offset >> 8);

	if (LOG_MAC_IWM)
		printf("mac_iwm_r: offset=0x%08x mem_mask %04x = %02x (PC %x)\n", offset, mem_mask, result, space.device().safe_pc());

	return (result << 8) | result;
}

WRITE16_MEMBER ( mac_state::mac_iwm_w )
{
	applefdc_base_device *fdc = space.machine().device<applefdc_base_device>("fdc");

	if (LOG_MAC_IWM)
		printf("mac_iwm_w: offset=0x%08x data=0x%04x mask %04x (PC=%x)\n", offset, data, mem_mask, space.device().safe_pc());

	if (ACCESSING_BITS_0_7)
		fdc->write((offset >> 8), data & 0xff);
	else
		fdc->write((offset >> 8), data>>8);
}

WRITE_LINE_MEMBER(mac_state::mac_adb_via_out_cb2)
{
//        printf("VIA OUT CB2 = %x\n", state);
	if (ADB_IS_EGRET)
	{
		m_egret->set_via_data(state & 1);
	}
	else if (ADB_IS_CUDA)
	{
		m_cuda->set_via_data(state & 1);
	}
	else
	{
		if (state)
			m_adb_command |= 1;
		else
			m_adb_command &= ~1;
	}
}

/* *************************************************************************
 * VIA
 * *************************************************************************
 *
 *
 * PORT A
 *
 *  bit 7               R   SCC Wait/Request
 *  bit 6               W   Main/Alternate screen buffer select
 *  bit 5               W   Floppy Disk Line Selection
 *  bit 4               W   Overlay/Normal memory mapping
 *  bit 3               W   Main/Alternate sound buffer
 *  bit 2-0             W   Sound Volume
 *
 *
 * PORT B
 *
 *  bit 7               W   Sound enable
 *  bit 6               R   Video beam in display
 *  bit 5   (pre-ADB)   R   Mouse Y2
 *          (ADB)       W   ADB ST1
 *  bit 4   (pre-ADB)   R   Mouse X2
 *          (ADB)       W   ADB ST0
 *  bit 3   (pre-ADB)   R   Mouse button (active low)
 *          (ADB)       R   ADB INT
 *  bit 2               W   Real time clock enable
 *  bit 1               W   Real time clock data clock
 *  bit 0               RW  Real time clock data
 *
 */

#define PA6 0x40
#define PA4 0x10
#define PA2 0x04
#define PA1 0x02

READ8_MEMBER(mac_state::mac_via_in_a)
{
//  printf("VIA1 IN_A (PC %x)\n", m_maincpu->safe_pc());

	switch (m_model)
	{
		case MODEL_MAC_CLASSIC:
		case MODEL_MAC_II:
		case MODEL_MAC_II_FDHD:
		case MODEL_MAC_IIX:
		case MODEL_MAC_POWERMAC_6100:
		case MODEL_MAC_POWERMAC_7100:
		case MODEL_MAC_POWERMAC_8100:
			return 0x81;        // bit 0 must be set on most Macs to avoid attempting to boot from AppleTalk

		case MODEL_MAC_SE30:
			return 0x81 | PA6;

		case MODEL_MAC_LC:
		case MODEL_MAC_LC_II:
		case MODEL_MAC_IIVX:
			return 0x81 | PA6 | PA4 | PA2;

		case MODEL_MAC_IICI:
			return 0x81 | PA6 | PA2 | PA1;

		case MODEL_MAC_IISI:
			return 0x81 | PA4 | PA2 | PA1;

		case MODEL_MAC_IIFX:
			return 0x81 | PA6 | PA4 | PA1;

		case MODEL_MAC_IICX:
			return 0x81 | PA6;

		case MODEL_MAC_PB140:   // since the ASICs are different, these are allowed to "collide"
		case MODEL_MAC_PB160:
		case MODEL_MAC_CLASSIC_II:
		case MODEL_MAC_QUADRA_800:
			return 0x81 | PA4 | PA1;

		case MODEL_MAC_QUADRA_700:
			return 0x81 | PA6;

		case MODEL_MAC_QUADRA_900:
			return 0x81 | PA6 | PA4;

		case MODEL_MAC_COLOR_CLASSIC:
			return 0x81 | PA1;

		default:
			return 0x80;
	}
}

READ8_MEMBER(mac_state::mac_via_in_a_pmu)
{
//  printf("VIA1 IN_A (PC %x)\n", m_maincpu->safe_pc());

	#if LOG_ADB
//  printf("Read PM data %x\n", m_pm_data_recv);
	#endif
	return m_pm_data_recv;
}

READ8_MEMBER(mac_state::mac_via_in_b)
{
	int val = 0;
	/* video beam in display (! VBLANK && ! HBLANK basically) */
	if (machine().first_screen())
	{
		if (machine().first_screen()->vpos() >= MAC_V_VIS)
			val |= 0x40;
	}

	if (ADB_IS_BITBANG_CLASS)
	{
		val |= m_adb_state<<4;

		if (!m_adb_irq_pending)
		{
			val |= 0x08;
		}

		val |= m_rtc->data_r();
	}
	else if (ADB_IS_EGRET)
	{
		val |= m_egret->get_xcvr_session()<<3;
	}
	else if (ADB_IS_CUDA)
	{
		val |= m_cuda->get_treq()<<3;
	}
	else
	{
		if (m_mouse_bit_y)  /* Mouse Y2 */
			val |= 0x20;
		if (m_mouse_bit_x)  /* Mouse X2 */
			val |= 0x10;
		if ((m_mouse0->read() & 0x01) == 0)
			val |= 0x08;

		val |= m_rtc->data_r();
	}

//  printf("VIA1 IN_B = %02x (PC %x)\n", val, m_maincpu->safe_pc());

	return val;
}

READ8_MEMBER(mac_state::mac_via_in_b_via2pmu)
{
	int val = 0;
	// TODO: is this valid for VIA2 PMU machines?
	/* video beam in display (! VBLANK && ! HBLANK basically) */
	if (machine().first_screen())
	{
		if (machine().first_screen()->vpos() >= MAC_V_VIS)
			val |= 0x40;
	}

//  printf("VIA1 IN_B = %02x (PC %x)\n", val, m_maincpu->safe_pc());

	return val;
}

READ8_MEMBER(mac_state::mac_via_in_b_pmu)
{
	int val = 0;
//  printf("Read VIA B: PM_ACK %x\n", m_pm_ack);
	val = 0x80 | 0x04 | m_pm_ack;   // SCC wait/request (bit 2 must be set at 900c1a or startup tests always fail)

//  printf("VIA1 IN_B = %02x (PC %x)\n", val, m_maincpu->safe_pc());

	return val;
}

WRITE8_MEMBER(mac_state::mac_via_out_a)
{
	mac_sound_device *sound = machine().device<mac_sound_device>("custom");
	device_t *fdc = machine().device("fdc");
//  printf("VIA1 OUT A: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	set_scc_waitrequest((data & 0x80) >> 7);
	m_screen_buffer = (data & 0x40) >> 6;
	sony_set_sel_line(fdc,(data & 0x20) >> 5);
	if (m_model == MODEL_MAC_SE)    // on SE this selects which floppy drive (0 = upper, 1 = lower)
	{
		m_drive_select = ((data & 0x10) >> 4);
	}

	if (m_model < MODEL_MAC_SE) // SE no longer has dual buffers
	{
		sound->set_sound_buffer((data & 0x08) >> 3);
	}

	if (m_model < MODEL_MAC_II)
	{
		sound->set_volume(data & 0x07);
	}

	/* Early Mac models had VIA A4 control overlaying.  In the Mac SE (and
	 * possibly later models), overlay was set on reset, but cleared on the
	 * first access to the ROM. */
	if (m_model < MODEL_MAC_SE)
	{
		set_memory_overlay((data & 0x10) >> 4);
	}
}

WRITE8_MEMBER(mac_state::mac_via_out_a_pmu)
{
//  printf("VIA1 OUT A: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	#if LOG_ADB
//  printf("%02x to PM\n", data);
	#endif
	m_pm_data_send = data;
	return;
}

WRITE8_MEMBER(mac_state::mac_via_out_b)
{
	mac_sound_device *sound = machine().device<mac_sound_device>("custom");
//  printf("VIA1 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	if (AUDIO_IS_CLASSIC)
	{
		sound->enable_sound((data & 0x80) == 0);
	}

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

WRITE8_MEMBER(mac_state::mac_via_out_b_bbadb)
{
//  printf("VIA1 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	// SE and Classic have SCSI enable/disable here
	if ((m_model == MODEL_MAC_SE) || (m_model == MODEL_MAC_CLASSIC))
	{
		m_scsiirq_enable = (data & 0x40) ? 0 : 1;
//      printf("VIAB & 0x40 = %02x, IRQ enable %d\n", data & 0x40, m_scsiirq_enable);
	}

	if (m_model == MODEL_MAC_SE30)
	{
		// 0x40 = 0 means enable vblank on SE/30
		m_se30_vbl_enable = (data & 0x40) ? 0 : 1;

		// clear the interrupt if we disabled it
		if (!m_se30_vbl_enable)
		{
			nubus_slot_interrupt(0xe, 0);
		}
	}

	mac_adb_newaction((data & 0x30) >> 4);

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

WRITE8_MEMBER(mac_state::mac_via_out_b_egadb)
{
//  printf("VIA1 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	#if LOG_ADB
	printf("68K: New Egret state: SS %d VF %d (PC %x)\n", (data>>5)&1, (data>>4)&1, space.device().safe_pc());
	#endif
	m_egret->set_via_full((data&0x10) ? 1 : 0);
	m_egret->set_sys_session((data&0x20) ? 1 : 0);
}

WRITE8_MEMBER(mac_state::mac_via_out_b_cdadb)
{
//  printf("VIA1 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	#if LOG_ADB
	printf("68K: New Cuda state: TIP %d BYTEACK %d (PC %x)\n", (data>>5)&1, (data>>4)&1, space.device().safe_pc());
	#endif
	m_cuda->set_byteack((data&0x10) ? 1 : 0);
	m_cuda->set_tip((data&0x20) ? 1 : 0);
}

WRITE8_MEMBER(mac_state::mac_via_out_b_via2pmu)
{
//  printf("VIA1 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());
}

WRITE8_MEMBER(mac_state::mac_via_out_b_pmu)
{
//  printf("VIA1 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	device_t *fdc = machine().device("fdc");

	sony_set_sel_line(fdc,(data & 0x20) >> 5);
	m_drive_select = ((data & 0x10) >> 4);

	if ((data & 1) && !(m_pm_req & 1))
	{
		#if LOG_ADB
		printf("PM: 68k dropping /REQ\n");
		#endif

		if (m_pm_state == 0)     // do this in receive state only
		{
			m_pm_data_recv = 0xff;
			m_pm_ack |= 2;

			// check if length byte matches
			if ((m_pm_dptr >= 2) && (m_pm_cmd[1] == (m_pm_dptr-2)))
			{
				pmu_exec();
				#if LOG_ADB
				printf("PMU exec: command %02x length %d\n", m_pm_cmd[0], m_pm_cmd[1]);
				#endif
			}
		}
	}
	else if (!(data & 1) && (m_pm_req & 1))
	{
		if (m_pm_state == 0)
		{
			#if LOG_ADB
			printf("PM: 68k asserting /REQ, clocking in byte [%d] = %02x\n", m_pm_dptr, m_pm_data_send);
			#endif
			m_pm_ack &= ~2; // clear, we're waiting for more bytes
			m_pm_cmd[m_pm_dptr++] = m_pm_data_send;
		}
		else    // receiving, so this is different
		{
			m_pm_data_recv = m_pm_out[m_pm_sptr++];
			m_pm_slen--;
			m_pm_ack |= 2;  // raise ACK to indicate available byte
			#if LOG_ADB
			printf("PM: 68k asserted /REQ, sending byte %02x\n", m_pm_data_recv);
			#endif

			// another byte to send?
			if (m_pm_slen)
			{
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
			}
			else
			{
				m_pm_state = 0; // back to receive state
				m_pmu_send_timer->adjust(attotime::never);
			}
		}
	}

	m_pm_req = data & 1;
}

WRITE_LINE_MEMBER(mac_state::mac_via_irq)
{
	/* interrupt the 68k (level 1) */
	set_via_interrupt(state);
}

READ16_MEMBER ( mac_state::mac_via_r )
{
	UINT16 data;

	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_r: offset=0x%02x\n", offset);
	data = m_via1->read(space, offset);

	m_maincpu->adjust_icount(m_via_cycles);

	return (data & 0xff) | (data << 8);
}

WRITE16_MEMBER ( mac_state::mac_via_w )
{
	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_w: offset=0x%02x data=0x%08x\n", offset, data);

	if (ACCESSING_BITS_0_7)
		m_via1->write(space, offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(space, offset, (data >> 8) & 0xff);

	m_maincpu->adjust_icount(m_via_cycles);
}

/* *************************************************************************
 * VIA 2 (on Mac IIs, PowerBooks > 100, and PowerMacs)
 * *************************************************************************/

WRITE_LINE_MEMBER(mac_state::mac_via2_irq)
{
	set_via2_interrupt(state);
}

READ16_MEMBER ( mac_state::mac_via2_r )
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via2->read(space, offset);

	if (LOG_VIA)
		logerror("mac_via2_r: offset=0x%02x = %02x (PC=%x)\n", offset*2, data, space.device().safe_pc());

	return (data & 0xff) | (data << 8);
}

WRITE16_MEMBER ( mac_state::mac_via2_w )
{
	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via2_w: offset=%x data=0x%08x mask=%x (PC=%x)\n", offset, data, mem_mask, space.device().safe_pc());

	if (ACCESSING_BITS_0_7)
		m_via2->write(space, offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(space, offset, (data >> 8) & 0xff);
}


READ8_MEMBER(mac_state::mac_via2_in_a)
{
	UINT8 result;

	if ((m_model == MODEL_MAC_QUADRA_700) || (m_model == MODEL_MAC_QUADRA_900) || (m_model == MODEL_MAC_QUADRA_950))
	{
		result = 0x80 | m_nubus_irq_state;
	}
	else
	{
		result = 0xc0 | m_nubus_irq_state;
	}

	return result;
}

READ8_MEMBER(mac_state::mac_via2_in_a_pmu)
{
	return m_pm_data_recv;
}

READ8_MEMBER(mac_state::mac_via2_in_b)
{
//  logerror("VIA2 IN B (PC %x)\n", m_maincpu->safe_pc());

	if ((m_model == MODEL_MAC_LC) || (m_model == MODEL_MAC_LC_II) || (m_model == MODEL_MAC_CLASSIC_II))
	{
		return 0x4f;
	}

	if ((m_model == MODEL_MAC_SE30) || (m_model == MODEL_MAC_IIX))
	{
		return 0x87;    // bits 3 and 6 are tied low on SE/30
	}

	return 0xcf;        // indicate no NuBus transaction error
}

READ8_MEMBER(mac_state::mac_via2_in_b_pmu)
{
//  logerror("VIA2 IN B (PC %x)\n", m_maincpu->safe_pc());

	if (m_pm_ack == 2)
	{
		return 0xcf;
	}
	else
	{
		return 0xcd;
	}
}

WRITE8_MEMBER(mac_state::mac_via2_out_a)
{
//  logerror("VIA2 OUT A: %02x (PC %x)\n", data, m_maincpu->safe_pc());
}

WRITE8_MEMBER(mac_state::mac_via2_out_a_pmu)
{
//  logerror("VIA2 OUT A: %02x (PC %x)\n", data, m_maincpu->safe_pc());
	m_pm_data_send = data;
}

WRITE8_MEMBER(mac_state::mac_via2_out_b)
{
//  logerror("VIA2 OUT B: %02x (PC %x)\n", data, m_maincpu->safe_pc());

	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data>>7);

	if (m_model == MODEL_MAC_II)
	{
		m68000_base_device *m68k = downcast<m68000_base_device *>(m_maincpu.target());
		m68k->set_hmmu_enable((data & 0x8) ? M68K_HMMU_DISABLE : M68K_HMMU_ENABLE_II);
	}
}

WRITE8_MEMBER(mac_state::mac_via2_out_b_pmu)
{
//  logerror("VIA2 OUT B PMU: %02x (PC %x)\n", data, m_maincpu->pc());

	if ((data & 4) && !(m_pm_req & 4))
	{
		#if LOG_ADB
		printf("PM: 68k dropping /REQ\n");
		#endif

		if (m_pm_state == 0)     // do this in receive state only
		{
			m_pm_data_recv = 0xff;
			m_pm_ack |= 2;

			// check if length byte matches
			if ((m_pm_dptr >= 2) && (m_pm_cmd[1] == (m_pm_dptr-2)))
			{
				pmu_exec();
				#if LOG_ADB
				printf("PMU exec: command %02x length %d\n", m_pm_cmd[0], m_pm_cmd[1]);
				#endif
			}
		}
	}
	else if (!(data & 4) && (m_pm_req & 4))
	{
		if (m_pm_state == 0)
		{
			#if LOG_ADB
			printf("PM: 68k asserting /REQ, clocking in byte [%d] = %02x\n", m_pm_dptr, m_pm_data_send);
			#endif
			m_pm_ack &= ~2; // clear, we're waiting for more bytes
			m_pm_cmd[m_pm_dptr++] = m_pm_data_send;
		}
		else    // receiving, so this is different
		{
			m_pm_data_recv = m_pm_out[m_pm_sptr++];
			m_pm_slen--;
			m_pm_ack |= 2;  // raise ACK to indicate available byte
			#if LOG_ADB
			printf("PM: 68k asserted /REQ, sending byte %02x\n", m_pm_data_recv);
			#endif

			// another byte to send?
			if (m_pm_slen)
			{
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
			}
			else
			{
				m_pm_state = 0; // back to receive state
				m_pmu_send_timer->adjust(attotime::never);
			}
		}
	}

	m_pm_req = data & 4;
}

// This signal is generated internally on RBV, V8, Sonora, VASP, Eagle, etc.
TIMER_CALLBACK_MEMBER(mac_state::mac_6015_tick)
{
	m_via1->write_ca1(0);
	m_via1->write_ca1(1);
}

/* *************************************************************************
 * Main
 * *************************************************************************/

void mac_state::machine_start()
{
	if (has_adb())
	{
		this->m_adb_timer = machine().scheduler().timer_alloc(FUNC(mac_adb_tick));
		this->m_adb_timer->adjust(attotime::never);

		// also allocate PMU timer
		if (ADB_IS_PM_CLASS)
		{
			m_pmu_send_timer = machine().scheduler().timer_alloc(FUNC(mac_pmu_tick));
			this->m_adb_timer->adjust(attotime::never);
			m_pmu_int_status = 0;
		}
	}

	if (machine().first_screen())
	{
		this->m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac_state::mac_scanline_tick),this));
		this->m_scanline_timer->adjust(machine().first_screen()->time_until_pos(0, 0));
	}

	m_6015_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac_state::mac_6015_tick),this));
	m_6015_timer->adjust(attotime::never);
}

void mac_state::machine_reset()
{
	// stop 60.15 Hz timer
	m_6015_timer->adjust(attotime::never);

	m_rbv_vbltime = 0;

	if (m_model >= MODEL_MAC_POWERMAC_6100 && m_model <= MODEL_MAC_POWERMAC_8100)
	{
		m_awacs->set_dma_base(m_maincpu->space(AS_PROGRAM), 0x10000, 0x12000);
	}

	// start 60.15 Hz timer for most systems
	if (((m_model >= MODEL_MAC_IICI) && (m_model <= MODEL_MAC_IIVI)) || (m_model >= MODEL_MAC_LC))
	{
		m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
	}

	// we use the CPU clock divided by the VIA clock (783360 Hz) rounded up as
	// an approximation for the right number of wait states.  this yields good
	// results - it's towards the end of the worst-case delay on h/w.
	switch (m_maincpu->clock())
	{
		case 7833600:   // C7M on classic Macs
			m_via_cycles = -10;
			break;

		case 7833600*2: // "16 MHz" Macs
			m_via_cycles = -30;
			break;

		case 20000000:  // 20 MHz Macs
			m_via_cycles = -40;
			break;

		case 25000000:  // 25 MHz Macs
			m_via_cycles = -50;
			break;

		case 7833600*4: // 32 MHz Macs (these are C7M * 4 like IIvx)
			m_via_cycles = -60;
			break;

		case 33000000:  // 33 MHz Macs ('040s)
			m_via_cycles = -64;
			break;

		case 40000000:  // 40 MHz Macs
			m_via_cycles = -80;
			break;

		case 60000000:  // 60 MHz PowerMac
			m_via_cycles = -120;
			break;

		case 66000000:  // 66 MHz PowerMac
			m_via_cycles = -128;
			break;

		default:
			fatalerror("mac: unknown clock\n");
	}

	// clear PMU response timer
	if (ADB_IS_PM_CLASS)
	{
		this->m_adb_timer->adjust(attotime::never);
	}

	// default to 32-bit mode on LC
	if (m_model == MODEL_MAC_LC)
	{
		m68000_base_device *m68k = downcast<m68000_base_device *>(m_maincpu.target());
		m68k->set_hmmu_enable(M68K_HMMU_DISABLE);
	}

	m_last_taken_interrupt = -1;

	/* setup the memory overlay */
	if (m_model < MODEL_MAC_POWERMAC_6100)  // no overlay for PowerPC
	{
		m_overlay = -1; // insure no match
		this->set_memory_overlay(1);
	}

	if (m_overlay_timeout != (emu_timer *)nullptr)
	{
		if ((m_model == MODEL_MAC_LC_III) || (m_model == MODEL_MAC_LC_III_PLUS) || (m_model >= MODEL_MAC_LC_475 && m_model <= MODEL_MAC_LC_580))   // up to 36 MB 
		{
			m_overlay_timeout->adjust(attotime::never);
		}
		else if (((m_model >= MODEL_MAC_LC) && (m_model <= MODEL_MAC_COLOR_CLASSIC) && ((m_model != MODEL_MAC_LC_III) && (m_model != MODEL_MAC_LC_III_PLUS))) || (m_model == MODEL_MAC_CLASSIC_II))
		{
			m_overlay_timeout->adjust(attotime::never);
		}
		else
		{
			m_overlay_timeout->adjust(m_maincpu->cycles_to_attotime(8));
		}
	}

	/* setup videoram */
	this->m_screen_buffer = 1;

	/* setup 'classic' sound */
	if (machine().device("custom") != nullptr)
	{
		machine().device<mac_sound_device>("custom")->set_sound_buffer(0);
	}
	else if (MAC_HAS_VIA2)  // prime CB1 for ASC and slot interrupts
	{
		m_via2_ca1_hack = 1;
		m_via2->write_ca1(1);
		m_via2->write_cb1(1);
	}

	if (has_adb())
	{
		this->adb_reset();
	}

	if ((m_model == MODEL_MAC_SE) || (m_model == MODEL_MAC_CLASSIC))
	{
		machine().device<mac_sound_device>("custom")->set_sound_buffer(1);

		// classic will fail RAM test and try to boot appletalk if RAM is not all zero
		memset(m_ram->pointer(), 0, m_ram->size());
	}

	m_scsi_interrupt = 0;
	if ((m_maincpu->debug()) && (m_model < MODEL_MAC_POWERMAC_6100))
	{
		m_maincpu->debug()->set_dasm_override(mac_dasm_override);
	}

	m_drive_select = 0;
	m_scsiirq_enable = 0;
	m_via2_vbl = 0;
	m_se30_vbl_enable = 0;
	m_nubus_irq_state = 0xff;
#ifndef MAC_USE_EMULATED_KBD
	m_keyboard_reply = 0;
	m_kbd_comm = 0;
	m_kbd_receive = 0;
	m_kbd_shift_reg = 0;
	m_kbd_shift_count = 0;
#endif
	m_mouse_bit_x = m_mouse_bit_y = 0;
	m_pm_data_send = m_pm_data_recv = m_pm_ack = m_pm_req = m_pm_dptr = 0;
	m_pm_state = 0;
	m_last_taken_interrupt = 0;
}

WRITE_LINE_MEMBER(mac_state::cuda_reset_w)
{
	if ((state == ASSERT_LINE) && (m_model < MODEL_MAC_POWERMAC_6100))
	{
		set_memory_overlay(0);
		set_memory_overlay(1);
	}

	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

void mac_state::mac_state_load()
{
	int overlay;
	overlay = m_overlay;
	if (m_model < MODEL_MAC_POWERMAC_6100) // no overlay for PowerPC
	{
		m_overlay = -1;
		set_memory_overlay(overlay);
	}
}


TIMER_CALLBACK_MEMBER(mac_state::overlay_timeout_func)
{
	if (m_overlay != -1)
	{
		set_memory_overlay(0);      // kill the overlay
	}

	// we're a one-time-only thing
	m_overlay_timeout->adjust(attotime::never);
}

READ32_MEMBER(mac_state::mac_read_id)
{
//    printf("Mac read ID reg @ PC=%x\n", m_maincpu->pc());

	switch (m_model)
	{
		case MODEL_MAC_LC_III:
			return 0xa55a0001;  // 25 MHz LC III

		case MODEL_MAC_LC_III_PLUS:
			return 0xa55a0003;  // 33 MHz LC III+

		case MODEL_MAC_LC_475:
			return 0xa55a2221;

		case MODEL_MAC_LC_520:
			return 0xa55a0100;

		case MODEL_MAC_LC_550:
			return 0xa55a0101;

		case MODEL_MAC_LC_575:
			return 0xa55a222e;

		case MODEL_MAC_POWERMAC_6100:
			return 0xa55a3011;

		case MODEL_MAC_POWERMAC_7100:
			return 0xa55a3012;

		case MODEL_MAC_POWERMAC_8100:
			return 0xa55a3013;

		case MODEL_MAC_PBDUO_210:
			return 0xa55a1004;

		case MODEL_MAC_PBDUO_230:
			return 0xa55a1005;

		case MODEL_MAC_PBDUO_250:
			return 0xa55a1006;

		case MODEL_MAC_QUADRA_605:
			return 0xa55a2225;

		case MODEL_MAC_QUADRA_610:
		case MODEL_MAC_QUADRA_650:
		case MODEL_MAC_QUADRA_800:
			return 0xa55a2bad;

		case MODEL_MAC_QUADRA_660AV:
		case MODEL_MAC_QUADRA_840AV:
			return 0xa55a2830;

		default:
			return 0;
	}
}

void mac_state::mac_driver_init(model_t model)
{
	m_overlay = 1;
	m_scsi_interrupt = 0;
	m_model = model;

	if (model < MODEL_MAC_PORTABLE)
	{
		/* set up RAM mirror at 0x600000-0x6fffff (0x7fffff ???) */
		mac_install_memory(0x600000, 0x6fffff, m_ram->size(), m_ram->pointer(), FALSE, "bank2");

		/* set up ROM at 0x400000-0x4fffff (-0x5fffff for mac 128k/512k/512ke) */
		mac_install_memory(0x400000, (model >= MODEL_MAC_PLUS) ? 0x4fffff : 0x5fffff,
			memregion("bootrom")->bytes(), memregion("bootrom")->base(), TRUE, "bank3");
	}

	m_overlay = -1;
	if (m_model < MODEL_MAC_POWERMAC_6100) // no overlay for PowerPC
	{
		set_memory_overlay(1);
	}

	memset(m_ram->pointer(), 0, m_ram->size());

	if ((model == MODEL_MAC_SE) || (model == MODEL_MAC_CLASSIC) || (model == MODEL_MAC_CLASSIC_II) || (model == MODEL_MAC_LC) || (model == MODEL_MAC_COLOR_CLASSIC) || (model >= MODEL_MAC_LC_475 && model <= MODEL_MAC_LC_580) ||
		(model == MODEL_MAC_LC_II) || (model == MODEL_MAC_LC_III) || (model == MODEL_MAC_LC_III_PLUS) || ((m_model >= MODEL_MAC_II) && (m_model <= MODEL_MAC_SE30)) ||
		(model == MODEL_MAC_PORTABLE) || (model == MODEL_MAC_PB100) || (model == MODEL_MAC_PB140) || (model == MODEL_MAC_PB160) || (model == MODEL_MAC_PBDUO_210) || (model >= MODEL_MAC_QUADRA_700 && model <= MODEL_MAC_QUADRA_800))
	{
		m_overlay_timeout = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac_state::overlay_timeout_func),this));
	}
	else
	{
		m_overlay_timeout = (emu_timer *)nullptr;
	}

	/* setup keyboard */
#ifndef MAC_USE_EMULATED_KBD
	keyboard_init();
	m_inquiry_timeout = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac_state::inquiry_timeout_func),this));
#else
	/* clear key matrix for macadb */
	for (int i=0; i<7; i++)
	{
		m_key_matrix[i] = 0;
	}
#endif

	/* save state stuff */
	machine().save().register_postload(save_prepost_delegate(FUNC(mac_state::mac_state_load), this));
}

#define MAC_DRIVER_INIT(label, model)   \
DRIVER_INIT_MEMBER(mac_state,label)     \
{   \
	mac_driver_init(model); \
}

MAC_DRIVER_INIT(mac128k512k, MODEL_MAC_128K512K)
MAC_DRIVER_INIT(mac512ke, MODEL_MAC_512KE)
MAC_DRIVER_INIT(macplus, MODEL_MAC_PLUS)
MAC_DRIVER_INIT(macse, MODEL_MAC_SE)
MAC_DRIVER_INIT(macclassic, MODEL_MAC_CLASSIC)
MAC_DRIVER_INIT(maclc, MODEL_MAC_LC)
MAC_DRIVER_INIT(maclc2, MODEL_MAC_LC_II)
MAC_DRIVER_INIT(maclc3, MODEL_MAC_LC_III)
MAC_DRIVER_INIT(maclc3plus, MODEL_MAC_LC_III_PLUS)
MAC_DRIVER_INIT(maciici, MODEL_MAC_IICI)
MAC_DRIVER_INIT(maciisi, MODEL_MAC_IISI)
MAC_DRIVER_INIT(macii, MODEL_MAC_II)
MAC_DRIVER_INIT(macse30, MODEL_MAC_SE30)
MAC_DRIVER_INIT(macclassic2, MODEL_MAC_CLASSIC_II)
MAC_DRIVER_INIT(maclrcclassic, MODEL_MAC_COLOR_CLASSIC)
MAC_DRIVER_INIT(macpm6100, MODEL_MAC_POWERMAC_6100)
MAC_DRIVER_INIT(macpm7100, MODEL_MAC_POWERMAC_7100)
MAC_DRIVER_INIT(macpm8100, MODEL_MAC_POWERMAC_8100)
MAC_DRIVER_INIT(macprtb, MODEL_MAC_PORTABLE)
MAC_DRIVER_INIT(macpb100, MODEL_MAC_PB100)
MAC_DRIVER_INIT(macpb140, MODEL_MAC_PB140)
MAC_DRIVER_INIT(macpb160, MODEL_MAC_PB160)
MAC_DRIVER_INIT(maciivx, MODEL_MAC_IIVX)
MAC_DRIVER_INIT(maciifx, MODEL_MAC_IIFX)
MAC_DRIVER_INIT(macpd210, MODEL_MAC_PBDUO_210)
MAC_DRIVER_INIT(macquadra700, MODEL_MAC_QUADRA_700)
MAC_DRIVER_INIT(maciicx, MODEL_MAC_IICX)
MAC_DRIVER_INIT(maciifdhd, MODEL_MAC_II_FDHD)
MAC_DRIVER_INIT(maciix, MODEL_MAC_IIX)
MAC_DRIVER_INIT(maclc520, MODEL_MAC_LC_520)

void mac_state::nubus_slot_interrupt(UINT8 slot, UINT32 state)
{
	static const UINT8 masks[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
	UINT8 mask = 0x3f;

	// quadra 700/900/950 use the top 2 bits of the interrupt register for ethernet and video
	if ((m_model == MODEL_MAC_QUADRA_700) || (m_model == MODEL_MAC_QUADRA_900) || (m_model == MODEL_MAC_QUADRA_950))
	{
		mask = 0xff;
	}

	slot -= 9;

	if (state)
	{
		m_nubus_irq_state &= ~masks[slot];
	}
	else
	{
		m_nubus_irq_state |= masks[slot];
	}

	if ((m_model != MODEL_MAC_IIFX) && (!INTS_RBV))
	{
		if ((m_nubus_irq_state & mask) != mask)
		{
			// HACK: sometimes we miss an ack (possible misbehavior in the VIA?)
			if (m_via2_ca1_hack == 0)
			{
				m_via2->write_ca1(1);
			}
			m_via2_ca1_hack = 0;
			m_via2->write_ca1(0);
		}
		else
		{
			m_via2_ca1_hack = 1;
			m_via2->write_ca1(1);
		}
	}

	if (INTS_RBV)
	{
		m_rbv_regs[2] &= ~0x38;
		m_rbv_regs[2] |= (m_nubus_irq_state & 0x38);
		rbv_recalc_irqs();
	}
}

void mac_state::vblank_irq()
{
	/* handle ADB keyboard/mouse */
	if (has_adb())
	{
		this->adb_vblank();
	}

#ifndef MAC_USE_EMULATED_KBD
	/* handle keyboard */
	if (m_kbd_comm == TRUE && m_kbd_receive == FALSE)
	{
		int keycode = scan_keyboard();

		if (keycode != 0x7B)
		{
			/* if key pressed, send the code */

			logerror("keyboard enquiry successful, keycode %X\n", keycode);

			m_inquiry_timeout->reset();
			kbd_shift_out(keycode);
		}
	}
#endif

	/* signal VBlank on CA1 input on the VIA */
	if ((m_model < MODEL_MAC_II) || (m_model == MODEL_MAC_PB140) || (m_model == MODEL_MAC_PB160) || (m_model == MODEL_MAC_QUADRA_700))
	{
		ca1_data ^= 1;
		m_via1->write_ca1(ca1_data);
	}

	if (++irq_count == 60)
	{
		irq_count = 0;

		ca2_data ^= 1;
		/* signal 1 Hz irq on CA2 input on the VIA */
		m_via1->write_ca2(ca2_data);
	}

	// handle SE/30 vblank IRQ
	if (m_model == MODEL_MAC_SE30)
	{
		if (m_se30_vbl_enable)
		{
			m_via2_vbl ^= 1;
			if (!m_via2_vbl)
			{
				this->nubus_slot_interrupt(0xe, 1);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(mac_state::mac_scanline_tick)
{
	int scanline;

	if (machine().device("custom") != nullptr)
	{
		machine().device<mac_sound_device>("custom")->sh_updatebuffer();
	}

	if (m_rbv_vbltime > 0)
	{
		m_rbv_vbltime--;

		if (m_rbv_vbltime == 0)
		{
			m_rbv_regs[2] |= 0x40;
			rbv_recalc_irqs();
		}
	}

	scanline = machine().first_screen()->vpos();
	if (scanline == MAC_V_VIS)
		vblank_irq();

	/* check for mouse changes at 10 irqs per frame */
	if (m_model <= MODEL_MAC_PLUS)
	{
		if (!(scanline % 10))
			mouse_callback();
	}

	m_scanline_timer->adjust(machine().first_screen()->time_until_pos((scanline+1) % MAC_V_TOTAL, 0));
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_9_w)
{
	nubus_slot_interrupt(9, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_a_w)
{
	nubus_slot_interrupt(0xa, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_b_w)
{
	nubus_slot_interrupt(0xb, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_c_w)
{
	nubus_slot_interrupt(0xc, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_d_w)
{
	nubus_slot_interrupt(0xd, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_e_w)
{
	nubus_slot_interrupt(0xe, state);
}

/* *************************************************************************
 * Trap Tracing
 *
 * This is debug code that will output diagnostics regarding OS traps called
 * *************************************************************************/

const char *lookup_trap(UINT16 opcode)
{
	static const struct
	{
		UINT16 trap;
		const char *name;
	} traps[] =
	{
		{ 0xA000, "_Open" },
		{ 0xA001, "_Close" },
		{ 0xA002, "_Read" },
		{ 0xA003, "_Write" },
		{ 0xA004, "_Control" },
		{ 0xA005, "_Status" },
		{ 0xA006, "_KillIO" },
		{ 0xA007, "_GetVolInfo" },
		{ 0xA008, "_Create" },
		{ 0xA009, "_Delete" },
		{ 0xA00A, "_OpenRF" },
		{ 0xA00B, "_Rename" },
		{ 0xA00C, "_GetFileInfo" },
		{ 0xA00D, "_SetFileInfo" },
		{ 0xA00E, "_UnmountVol" },
		{ 0xA00F, "_MountVol" },
		{ 0xA010, "_Allocate" },
		{ 0xA011, "_GetEOF" },
		{ 0xA012, "_SetEOF" },
		{ 0xA013, "_FlushVol" },
		{ 0xA014, "_GetVol" },
		{ 0xA015, "_SetVol" },
		{ 0xA016, "_FInitQueue" },
		{ 0xA017, "_Eject" },
		{ 0xA018, "_GetFPos" },
		{ 0xA019, "_InitZone" },
		{ 0xA01B, "_SetZone" },
		{ 0xA01C, "_FreeMem" },
		{ 0xA01F, "_DisposePtr" },
		{ 0xA020, "_SetPtrSize" },
		{ 0xA021, "_GetPtrSize" },
		{ 0xA023, "_DisposeHandle" },
		{ 0xA024, "_SetHandleSize" },
		{ 0xA025, "_GetHandleSize" },
		{ 0xA027, "_ReallocHandle" },
		{ 0xA029, "_HLock" },
		{ 0xA02A, "_HUnlock" },
		{ 0xA02B, "_EmptyHandle" },
		{ 0xA02C, "_InitApplZone" },
		{ 0xA02D, "_SetApplLimit" },
		{ 0xA02E, "_BlockMove" },
		{ 0xA02F, "_PostEvent" },
		{ 0xA030, "_OSEventAvail" },
		{ 0xA031, "_GetOSEvent" },
		{ 0xA032, "_FlushEvents" },
		{ 0xA033, "_VInstall" },
		{ 0xA034, "_VRemove" },
		{ 0xA035, "_OffLine" },
		{ 0xA036, "_MoreMasters" },
		{ 0xA038, "_WriteParam" },
		{ 0xA039, "_ReadDateTime" },
		{ 0xA03A, "_SetDateTime" },
		{ 0xA03B, "_Delay" },
		{ 0xA03C, "_CmpString" },
		{ 0xA03D, "_DrvrInstall" },
		{ 0xA03E, "_DrvrRemove" },
		{ 0xA03F, "_InitUtil" },
		{ 0xA040, "_ResrvMem" },
		{ 0xA041, "_SetFilLock" },
		{ 0xA042, "_RstFilLock" },
		{ 0xA043, "_SetFilType" },
		{ 0xA044, "_SetFPos" },
		{ 0xA045, "_FlushFile" },
		{ 0xA047, "_SetTrapAddress" },
		{ 0xA049, "_HPurge" },
		{ 0xA04A, "_HNoPurge" },
		{ 0xA04B, "_SetGrowZone" },
		{ 0xA04C, "_CompactMem" },
		{ 0xA04D, "_PurgeMem" },
		{ 0xA04E, "_AddDrive" },
		{ 0xA04F, "_RDrvrInstall" },
		{ 0xA050, "_CompareString" },
		{ 0xA051, "_ReadXPRam" },
		{ 0xA052, "_WriteXPRam" },
		{ 0xA054, "_UprString" },
		{ 0xA055, "_StripAddress" },
		{ 0xA056, "_LowerText" },
		{ 0xA057, "_SetAppBase" },
		{ 0xA058, "_InsTime" },
		{ 0xA059, "_RmvTime" },
		{ 0xA05A, "_PrimeTime" },
		{ 0xA05B, "_PowerOff" },
		{ 0xA05C, "_MemoryDispatch" },
		{ 0xA05D, "_SwapMMUMode" },
		{ 0xA05E, "_NMInstall" },
		{ 0xA05F, "_NMRemove" },
		{ 0xA060, "_FSDispatch" },
		{ 0xA061, "_MaxBlock" },
		{ 0xA063, "_MaxApplZone" },
		{ 0xA064, "_MoveHHi" },
		{ 0xA065, "_StackSpace" },
		{ 0xA067, "_HSetRBit" },
		{ 0xA068, "_HClrRBit" },
		{ 0xA069, "_HGetState" },
		{ 0xA06A, "_HSetState" },
		{ 0xA06C, "_InitFS" },
		{ 0xA06D, "_InitEvents" },
		{ 0xA06E, "_SlotManager" },
		{ 0xA06F, "_SlotVInstall" },
		{ 0xA070, "_SlotVRemove" },
		{ 0xA071, "_AttachVBL" },
		{ 0xA072, "_DoVBLTask" },
		{ 0xA075, "_SIntInstall" },
		{ 0xA076, "_SIntRemove" },
		{ 0xA077, "_CountADBs" },
		{ 0xA078, "_GetIndADB" },
		{ 0xA079, "_GetADBInfo" },
		{ 0xA07A, "_SetADBInfo" },
		{ 0xA07B, "_ADBReInit" },
		{ 0xA07C, "_ADBOp" },
		{ 0xA07D, "_GetDefaultStartup" },
		{ 0xA07E, "_SetDefaultStartup" },
		{ 0xA07F, "_InternalWait" },
		{ 0xA080, "_GetVideoDefault" },
		{ 0xA081, "_SetVideoDefault" },
		{ 0xA082, "_DTInstall" },
		{ 0xA083, "_SetOSDefault" },
		{ 0xA084, "_GetOSDefault" },
		{ 0xA085, "_PMgrOp" },
		{ 0xA086, "_IOPInfoAccess" },
		{ 0xA087, "_IOPMsgRequest" },
		{ 0xA088, "_IOPMoveData" },
		{ 0xA089, "_SCSIAtomic" },
		{ 0xA08A, "_Sleep" },
		{ 0xA08B, "_CommToolboxDispatch" },
		{ 0xA08D, "_DebugUtil" },
		{ 0xA08F, "_DeferUserFn" },
		{ 0xA090, "_SysEnvirons" },
		{ 0xA092, "_EgretDispatch" },
		{ 0xA094, "_ServerDispatch" },
		{ 0xA09E, "_PowerMgrDispatch" },
		{ 0xA09F, "_PowerDispatch" },
		{ 0xA0A4, "_HeapDispatch" },
		{ 0xA0AC, "_FSMDispatch" },
		{ 0xA0AE, "_VADBProc" },
		{ 0xA0DD, "_PPC" },
		{ 0xA0FE, "_TEFindWord" },
		{ 0xA0FF, "_TEFindLine" },
		{ 0xA11A, "_GetZone" },
		{ 0xA11D, "_MaxMem" },
		{ 0xA11E, "_NewPtr" },
		{ 0xA122, "_NewHandle" },
		{ 0xA126, "_HandleZone" },
		{ 0xA128, "_RecoverHandle" },
		{ 0xA12F, "_PPostEvent" },
		{ 0xA146, "_GetTrapAddress" },
		{ 0xA148, "_PtrZone" },
		{ 0xA162, "_PurgeSpace" },
		{ 0xA166, "_NewEmptyHandle" },
		{ 0xA193, "_Microseconds" },
		{ 0xA198, "_HWPriv" },
		{ 0xA1AD, "_Gestalt" },
		{ 0xA200, "_HOpen" },
		{ 0xA207, "_HGetVInfo" },
		{ 0xA208, "_HCreate" },
		{ 0xA209, "_HDelete" },
		{ 0xA20A, "_HOpenRF" },
		{ 0xA20B, "_HRename" },
		{ 0xA20C, "_HGetFileInfo" },
		{ 0xA20D, "_HSetFileInfo" },
		{ 0xA20E, "_HUnmountVol" },
		{ 0xA210, "_AllocContig" },
		{ 0xA214, "_HGetVol" },
		{ 0xA215, "_HSetVol" },
		{ 0xA22E, "_BlockMoveData" },
		{ 0xA241, "_HSetFLock" },
		{ 0xA242, "_HRstFLock" },
		{ 0xA247, "_SetOSTrapAddress" },
		{ 0xA256, "_StripText" },
		{ 0xA260, "_HFSDispatch" },
		{ 0xA285, "_IdleUpdate" },
		{ 0xA28A, "_SlpQInstall" },
		{ 0xA31E, "_NewPtrClear" },
		{ 0xA322, "_NewHandleClear" },
		{ 0xA346, "_GetOSTrapAddress" },
		{ 0xA3AD, "_NewGestalt" },
		{ 0xA456, "_UpperText" },
		{ 0xA458, "_InsXTime" },
		{ 0xA485, "_IdleState" },
		{ 0xA48A, "_SlpQRemove" },
		{ 0xA51E, "_NewPtrSys" },
		{ 0xA522, "_NewHandleSys" },
		{ 0xA562, "_PurgeSpaceSys" },
		{ 0xA5AD, "_ReplaceGestalt" },
		{ 0xA647, "_SetToolBoxTrapAddress" },
		{ 0xA656, "_StripUpperText" },
		{ 0xA685, "_SerialPower" },
		{ 0xA71E, "_NewPtrSysClear" },
		{ 0xA722, "_NewHandleSysClear" },
		{ 0xA746, "_GetToolBoxTrapAddress" },
		{ 0xA7AD, "_GetGestaltProcPtr" },
		{ 0xA800, "_SoundDispatch" },
		{ 0xA801, "_SndDisposeChannel" },
		{ 0xA802, "_SndAddModifier" },
		{ 0xA803, "_SndDoCommand" },
		{ 0xA804, "_SndDoImmediate" },
		{ 0xA805, "_SndPlay" },
		{ 0xA806, "_SndControl" },
		{ 0xA807, "_SndNewChannel" },
		{ 0xA808, "_InitProcMenu" },
		{ 0xA809, "_GetControlVariant" },
		{ 0xA80A, "_GetWVariant" },
		{ 0xA80B, "_PopUpMenuSelect" },
		{ 0xA80C, "_RGetResource" },
		{ 0xA811, "_TESelView" },
		{ 0xA812, "_TEPinScroll" },
		{ 0xA813, "_TEAutoView" },
		{ 0xA814, "_SetFractEnable" },
		{ 0xA815, "_SCSIDispatch" },
		{ 0xA817, "_CopyMask" },
		{ 0xA819, "_XMunger" },
		{ 0xA81A, "_HOpenResFile" },
		{ 0xA81B, "_HCreateResFile" },
		{ 0xA81D, "_InvalMenuBar" },
		{ 0xA821, "_MaxSizeRsrc" },
		{ 0xA822, "_ResourceDispatch" },
		{ 0xA823, "_AliasDispatch" },
		{ 0xA824, "_HFSUtilDispatch" },
		{ 0xA825, "_MenuDispatch" },
		{ 0xA826, "_InsertMenuItem" },
		{ 0xA827, "_HideDialogItem" },
		{ 0xA828, "_ShowDialogItem" },
		{ 0xA82A, "_ComponentDispatch" },
		{ 0xA833, "_ScrnBitMap" },
		{ 0xA834, "_SetFScaleDisable" },
		{ 0xA835, "_FontMetrics" },
		{ 0xA836, "_GetMaskTable" },
		{ 0xA837, "_MeasureText" },
		{ 0xA838, "_CalcMask" },
		{ 0xA839, "_SeedFill" },
		{ 0xA83A, "_ZoomWindow" },
		{ 0xA83B, "_TrackBox" },
		{ 0xA83C, "_TEGetOffset" },
		{ 0xA83D, "_TEDispatch" },
		{ 0xA83E, "_TEStyleNew" },
		{ 0xA847, "_FracCos" },
		{ 0xA848, "_FracSin" },
		{ 0xA849, "_FracSqrt" },
		{ 0xA84A, "_FracMul" },
		{ 0xA84B, "_FracDiv" },
		{ 0xA84D, "_FixDiv" },
		{ 0xA84E, "_GetItemCmd" },
		{ 0xA84F, "_SetItemCmd" },
		{ 0xA850, "_InitCursor" },
		{ 0xA851, "_SetCursor" },
		{ 0xA852, "_HideCursor" },
		{ 0xA853, "_ShowCursor" },
		{ 0xA854, "_FontDispatch" },
		{ 0xA855, "_ShieldCursor" },
		{ 0xA856, "_ObscureCursor" },
		{ 0xA858, "_BitAnd" },
		{ 0xA859, "_BitXOr" },
		{ 0xA85A, "_BitNot" },
		{ 0xA85B, "_BitOr" },
		{ 0xA85C, "_BitShift" },
		{ 0xA85D, "_BitTst" },
		{ 0xA85E, "_BitSet" },
		{ 0xA85F, "_BitClr" },
		{ 0xA860, "_WaitNextEvent" },
		{ 0xA861, "_Random" },
		{ 0xA862, "_ForeColor" },
		{ 0xA863, "_BackColor" },
		{ 0xA864, "_ColorBit" },
		{ 0xA865, "_GetPixel" },
		{ 0xA866, "_StuffHex" },
		{ 0xA867, "_LongMul" },
		{ 0xA868, "_FixMul" },
		{ 0xA869, "_FixRatio" },
		{ 0xA86A, "_HiWord" },
		{ 0xA86B, "_LoWord" },
		{ 0xA86C, "_FixRound" },
		{ 0xA86D, "_InitPort" },
		{ 0xA86E, "_InitGraf" },
		{ 0xA86F, "_OpenPort" },
		{ 0xA870, "_LocalToGlobal" },
		{ 0xA871, "_GlobalToLocal" },
		{ 0xA872, "_GrafDevice" },
		{ 0xA873, "_SetPort" },
		{ 0xA874, "_GetPort" },
		{ 0xA875, "_SetPBits" },
		{ 0xA876, "_PortSize" },
		{ 0xA877, "_MovePortTo" },
		{ 0xA878, "_SetOrigin" },
		{ 0xA879, "_SetClip" },
		{ 0xA87A, "_GetClip" },
		{ 0xA87B, "_ClipRect" },
		{ 0xA87C, "_BackPat" },
		{ 0xA87D, "_ClosePort" },
		{ 0xA87E, "_AddPt" },
		{ 0xA87F, "_SubPt" },
		{ 0xA880, "_SetPt" },
		{ 0xA881, "_EqualPt" },
		{ 0xA882, "_StdText" },
		{ 0xA883, "_DrawChar" },
		{ 0xA884, "_DrawString" },
		{ 0xA885, "_DrawText" },
		{ 0xA886, "_TextWidth" },
		{ 0xA887, "_TextFont" },
		{ 0xA888, "_TextFace" },
		{ 0xA889, "_TextMode" },
		{ 0xA88A, "_TextSize" },
		{ 0xA88B, "_GetFontInfo" },
		{ 0xA88C, "_StringWidth" },
		{ 0xA88D, "_CharWidth" },
		{ 0xA88E, "_SpaceExtra" },
		{ 0xA88F, "_OSDispatch" },
		{ 0xA890, "_StdLine" },
		{ 0xA891, "_LineTo" },
		{ 0xA892, "_Line" },
		{ 0xA893, "_MoveTo" },
		{ 0xA894, "_Move" },
		{ 0xA895, "_ShutDown" },
		{ 0xA896, "_HidePen" },
		{ 0xA897, "_ShowPen" },
		{ 0xA898, "_GetPenState" },
		{ 0xA899, "_SetPenState" },
		{ 0xA89A, "_GetPen" },
		{ 0xA89B, "_PenSize" },
		{ 0xA89C, "_PenMode" },
		{ 0xA89D, "_PenPat" },
		{ 0xA89E, "_PenNormal" },
		{ 0xA89F, "_Moof" },
		{ 0xA8A0, "_StdRect" },
		{ 0xA8A1, "_FrameRect" },
		{ 0xA8A2, "_PaintRect" },
		{ 0xA8A3, "_EraseRect" },
		{ 0xA8A4, "_InverRect" },
		{ 0xA8A5, "_FillRect" },
		{ 0xA8A6, "_EqualRect" },
		{ 0xA8A7, "_SetRect" },
		{ 0xA8A8, "_OffsetRect" },
		{ 0xA8A9, "_InsetRect" },
		{ 0xA8AA, "_SectRect" },
		{ 0xA8AB, "_UnionRect" },
		{ 0xA8AD, "_PtInRect" },
		{ 0xA8AE, "_EmptyRect" },
		{ 0xA8AF, "_StdRRect" },
		{ 0xA8B0, "_FrameRoundRect" },
		{ 0xA8B1, "_PaintRoundRect" },
		{ 0xA8B2, "_EraseRoundRect" },
		{ 0xA8B3, "_InverRoundRect" },
		{ 0xA8B4, "_FillRoundRect" },
		{ 0xA8B5, "_ScriptUtil" },
		{ 0xA8B6, "_StdOval" },
		{ 0xA8B7, "_FrameOval" },
		{ 0xA8B8, "_PaintOval" },
		{ 0xA8B9, "_EraseOval" },
		{ 0xA8BA, "_InvertOval" },
		{ 0xA8BB, "_FillOval" },
		{ 0xA8BC, "_SlopeFromAngle" },
		{ 0xA8BD, "_StdArc" },
		{ 0xA8BE, "_FrameArc" },
		{ 0xA8BF, "_PaintArc" },
		{ 0xA8C0, "_EraseArc" },
		{ 0xA8C1, "_InvertArc" },
		{ 0xA8C2, "_FillArc" },
		{ 0xA8C3, "_PtToAngle" },
		{ 0xA8C4, "_AngleFromSlope" },
		{ 0xA8C5, "_StdPoly" },
		{ 0xA8C6, "_FramePoly" },
		{ 0xA8C7, "_PaintPoly" },
		{ 0xA8C8, "_ErasePoly" },
		{ 0xA8C9, "_InvertPoly" },
		{ 0xA8CA, "_FillPoly" },
		{ 0xA8CB, "_OpenPoly" },
		{ 0xA8CC, "_ClosePoly" },
		{ 0xA8CD, "_KillPoly" },
		{ 0xA8CE, "_OffsetPoly" },
		{ 0xA8CF, "_PackBits" },
		{ 0xA8D0, "_UnpackBits" },
		{ 0xA8D1, "_StdRgn" },
		{ 0xA8D2, "_FrameRgn" },
		{ 0xA8D3, "_PaintRgn" },
		{ 0xA8D4, "_EraseRgn" },
		{ 0xA8D5, "_InverRgn" },
		{ 0xA8D6, "_FillRgn" },
		{ 0xA8D7, "_BitMapToRegion" },
		{ 0xA8D8, "_NewRgn" },
		{ 0xA8D9, "_DisposeRgn" },
		{ 0xA8DA, "_OpenRgn" },
		{ 0xA8DB, "_CloseRgn" },
		{ 0xA8DC, "_CopyRgn" },
		{ 0xA8DD, "_SetEmptyRgn" },
		{ 0xA8DE, "_SetRecRgn" },
		{ 0xA8DF, "_RectRgn" },
		{ 0xA8E0, "_OffsetRgn" },
		{ 0xA8E1, "_InsetRgn" },
		{ 0xA8E2, "_EmptyRgn" },
		{ 0xA8E3, "_EqualRgn" },
		{ 0xA8E4, "_SectRgn" },
		{ 0xA8E5, "_UnionRgn" },
		{ 0xA8E6, "_DiffRgn" },
		{ 0xA8E7, "_XOrRgn" },
		{ 0xA8E8, "_PtInRgn" },
		{ 0xA8E9, "_RectInRgn" },
		{ 0xA8EA, "_SetStdProcs" },
		{ 0xA8EB, "_StdBits" },
		{ 0xA8EC, "_CopyBits" },
		{ 0xA8ED, "_StdTxMeas" },
		{ 0xA8EE, "_StdGetPic" },
		{ 0xA8EF, "_ScrollRect" },
		{ 0xA8F0, "_StdPutPic" },
		{ 0xA8F1, "_StdComment" },
		{ 0xA8F2, "_PicComment" },
		{ 0xA8F3, "_OpenPicture" },
		{ 0xA8F4, "_ClosePicture" },
		{ 0xA8F5, "_KillPicture" },
		{ 0xA8F6, "_DrawPicture" },
		{ 0xA8F7, "_Layout" },
		{ 0xA8F8, "_ScalePt" },
		{ 0xA8F9, "_MapPt" },
		{ 0xA8FA, "_MapRect" },
		{ 0xA8FB, "_MapRgn" },
		{ 0xA8FC, "_MapPoly" },
		{ 0xA8FD, "_PrGlue" },
		{ 0xA8FE, "_InitFonts" },
		{ 0xA8FF, "_GetFName" },
		{ 0xA900, "_GetFNum" },
		{ 0xA901, "_FMSwapFont" },
		{ 0xA902, "_RealFont" },
		{ 0xA903, "_SetFontLock" },
		{ 0xA904, "_DrawGrowIcon" },
		{ 0xA905, "_DragGrayRgn" },
		{ 0xA906, "_NewString" },
		{ 0xA907, "_SetString" },
		{ 0xA908, "_ShowHide" },
		{ 0xA909, "_CalcVis" },
		{ 0xA90A, "_CalcVBehind" },
		{ 0xA90B, "_ClipAbove" },
		{ 0xA90C, "_PaintOne" },
		{ 0xA90D, "_PaintBehind" },
		{ 0xA90E, "_SaveOld" },
		{ 0xA90F, "_DrawNew" },
		{ 0xA910, "_GetWMgrPort" },
		{ 0xA911, "_CheckUpDate" },
		{ 0xA912, "_InitWindows" },
		{ 0xA913, "_NewWindow" },
		{ 0xA914, "_DisposeWindow" },
		{ 0xA915, "_ShowWindow" },
		{ 0xA916, "_HideWindow" },
		{ 0xA917, "_GetWRefCon" },
		{ 0xA918, "_SetWRefCon" },
		{ 0xA919, "_GetWTitle" },
		{ 0xA91A, "_SetWTitle" },
		{ 0xA91B, "_MoveWindow" },
		{ 0xA91C, "_HiliteWindow" },
		{ 0xA91D, "_SizeWindow" },
		{ 0xA91E, "_TrackGoAway" },
		{ 0xA91F, "_SelectWindow" },
		{ 0xA920, "_BringToFront" },
		{ 0xA921, "_SendBehind" },
		{ 0xA922, "_BeginUpDate" },
		{ 0xA923, "_EndUpDate" },
		{ 0xA924, "_FrontWindow" },
		{ 0xA925, "_DragWindow" },
		{ 0xA926, "_DragTheRgn" },
		{ 0xA927, "_InvalRgn" },
		{ 0xA928, "_InvalRect" },
		{ 0xA929, "_ValidRgn" },
		{ 0xA92A, "_ValidRect" },
		{ 0xA92B, "_GrowWindow" },
		{ 0xA92C, "_FindWindow" },
		{ 0xA92D, "_CloseWindow" },
		{ 0xA92E, "_SetWindowPic" },
		{ 0xA92F, "_GetWindowPic" },
		{ 0xA930, "_InitMenus" },
		{ 0xA931, "_NewMenu" },
		{ 0xA932, "_DisposeMenu" },
		{ 0xA933, "_AppendMenu" },
		{ 0xA934, "_ClearMenuBar" },
		{ 0xA935, "_InsertMenu" },
		{ 0xA936, "_DeleteMenu" },
		{ 0xA937, "_DrawMenuBar" },
		{ 0xA938, "_HiliteMenu" },
		{ 0xA939, "_EnableItem" },
		{ 0xA93A, "_DisableItem" },
		{ 0xA93B, "_GetMenuBar" },
		{ 0xA93C, "_SetMenuBar" },
		{ 0xA93D, "_MenuSelect" },
		{ 0xA93E, "_MenuKey" },
		{ 0xA93F, "_GetItmIcon" },
		{ 0xA940, "_SetItmIcon" },
		{ 0xA941, "_GetItmStyle" },
		{ 0xA942, "_SetItmStyle" },
		{ 0xA943, "_GetItmMark" },
		{ 0xA944, "_SetItmMark" },
		{ 0xA945, "_CheckItem" },
		{ 0xA946, "_GetMenuItemText" },
		{ 0xA947, "_SetMenuItemText" },
		{ 0xA948, "_CalcMenuSize" },
		{ 0xA949, "_GetMenuHandle" },
		{ 0xA94A, "_SetMFlash" },
		{ 0xA94B, "_PlotIcon" },
		{ 0xA94C, "_FlashMenuBar" },
		{ 0xA94D, "_AppendResMenu" },
		{ 0xA94E, "_PinRect" },
		{ 0xA94F, "_DeltaPoint" },
		{ 0xA950, "_CountMItems" },
		{ 0xA951, "_InsertResMenu" },
		{ 0xA952, "_DeleteMenuItem" },
		{ 0xA953, "_UpdtControl" },
		{ 0xA954, "_NewControl" },
		{ 0xA955, "_DisposeControl" },
		{ 0xA956, "_KillControls" },
		{ 0xA957, "_ShowControl" },
		{ 0xA958, "_HideControl" },
		{ 0xA959, "_MoveControl" },
		{ 0xA95A, "_GetControlReference" },
		{ 0xA95B, "_SetControlReference" },
		{ 0xA95C, "_SizeControl" },
		{ 0xA95D, "_HiliteControl" },
		{ 0xA95E, "_GetControlTitle" },
		{ 0xA95F, "_SetControlTitle" },
		{ 0xA960, "_GetControlValue" },
		{ 0xA961, "_GetControlMinimum" },
		{ 0xA962, "_GetControlMaximum" },
		{ 0xA963, "_SetControlValue" },
		{ 0xA964, "_SetControlMinimum" },
		{ 0xA965, "_SetControlMaximum" },
		{ 0xA966, "_TestControl" },
		{ 0xA967, "_DragControl" },
		{ 0xA968, "_TrackControl" },
		{ 0xA969, "_DrawControls" },
		{ 0xA96A, "_GetControlAction" },
		{ 0xA96B, "_SetControlAction" },
		{ 0xA96C, "_FindControl" },
		{ 0xA96E, "_Dequeue" },
		{ 0xA96F, "_Enqueue" },
		{ 0xA970, "_GetNextEvent" },
		{ 0xA971, "_EventAvail" },
		{ 0xA972, "_GetMouse" },
		{ 0xA973, "_StillDown" },
		{ 0xA974, "_Button" },
		{ 0xA975, "_TickCount" },
		{ 0xA976, "_GetKeys" },
		{ 0xA977, "_WaitMouseUp" },
		{ 0xA978, "_UpdtDialog" },
		{ 0xA97B, "_InitDialogs" },
		{ 0xA97C, "_GetNewDialog" },
		{ 0xA97D, "_NewDialog" },
		{ 0xA97E, "_SelectDialogItemText" },
		{ 0xA97F, "_IsDialogEvent" },
		{ 0xA980, "_DialogSelect" },
		{ 0xA981, "_DrawDialog" },
		{ 0xA982, "_CloseDialog" },
		{ 0xA983, "_DisposeDialog" },
		{ 0xA984, "_FindDialogItem" },
		{ 0xA985, "_Alert" },
		{ 0xA986, "_StopAlert" },
		{ 0xA987, "_NoteAlert" },
		{ 0xA988, "_CautionAlert" },
		{ 0xA98B, "_ParamText" },
		{ 0xA98C, "_ErrorSound" },
		{ 0xA98D, "_GetDialogItem" },
		{ 0xA98E, "_SetDialogItem" },
		{ 0xA98F, "_SetDialogItemText" },
		{ 0xA990, "_GetDialogItemText" },
		{ 0xA991, "_ModalDialog" },
		{ 0xA992, "_DetachResource" },
		{ 0xA993, "_SetResPurge" },
		{ 0xA994, "_CurResFile" },
		{ 0xA995, "_InitResources" },
		{ 0xA996, "_RsrcZoneInit" },
		{ 0xA997, "_OpenResFile" },
		{ 0xA998, "_UseResFile" },
		{ 0xA999, "_UpdateResFile" },
		{ 0xA99A, "_CloseResFile" },
		{ 0xA99B, "_SetResLoad" },
		{ 0xA99C, "_CountResources" },
		{ 0xA99D, "_GetIndResource" },
		{ 0xA99E, "_CountTypes" },
		{ 0xA99F, "_GetIndType" },
		{ 0xA9A0, "_GetResource" },
		{ 0xA9A1, "_GetNamedResource" },
		{ 0xA9A2, "_LoadResource" },
		{ 0xA9A3, "_ReleaseResource" },
		{ 0xA9A4, "_HomeResFile" },
		{ 0xA9A5, "_SizeRsrc" },
		{ 0xA9A6, "_GetResAttrs" },
		{ 0xA9A7, "_SetResAttrs" },
		{ 0xA9A8, "_GetResInfo" },
		{ 0xA9A9, "_SetResInfo" },
		{ 0xA9AA, "_ChangedResource" },
		{ 0xA9AB, "_AddResource" },
		{ 0xA9AC, "_AddReference" },
		{ 0xA9AD, "_RmveResource" },
		{ 0xA9AE, "_RmveReference" },
		{ 0xA9AF, "_ResError" },
		{ 0xA9B0, "_WriteResource" },
		{ 0xA9B1, "_CreateResFile" },
		{ 0xA9B2, "_SystemEvent" },
		{ 0xA9B3, "_SystemClick" },
		{ 0xA9B4, "_SystemTask" },
		{ 0xA9B5, "_SystemMenu" },
		{ 0xA9B6, "_OpenDeskAcc" },
		{ 0xA9B7, "_CloseDeskAcc" },
		{ 0xA9B8, "_GetPattern" },
		{ 0xA9B9, "_GetCursor" },
		{ 0xA9BA, "_GetString" },
		{ 0xA9BB, "_GetIcon" },
		{ 0xA9BC, "_GetPicture" },
		{ 0xA9BD, "_GetNewWindow" },
		{ 0xA9BE, "_GetNewControl" },
		{ 0xA9BF, "_GetRMenu" },
		{ 0xA9C0, "_GetNewMBar" },
		{ 0xA9C1, "_UniqueID" },
		{ 0xA9C2, "_SysEdit" },
		{ 0xA9C3, "_KeyTranslate" },
		{ 0xA9C4, "_OpenRFPerm" },
		{ 0xA9C5, "_RsrcMapEntry" },
		{ 0xA9C6, "_SecondsToDate" },
		{ 0xA9C7, "_DateToSeconds" },
		{ 0xA9C8, "_SysBeep" },
		{ 0xA9C9, "_SysError" },
		{ 0xA9CA, "_PutIcon" },
		{ 0xA9CB, "_TEGetText" },
		{ 0xA9CC, "_TEInit" },
		{ 0xA9CD, "_TEDispose" },
		{ 0xA9CE, "_TETextBox" },
		{ 0xA9CF, "_TESetText" },
		{ 0xA9D0, "_TECalText" },
		{ 0xA9D1, "_TESetSelect" },
		{ 0xA9D2, "_TENew" },
		{ 0xA9D3, "_TEUpdate" },
		{ 0xA9D4, "_TEClick" },
		{ 0xA9D5, "_TECopy" },
		{ 0xA9D6, "_TECut" },
		{ 0xA9D7, "_TEDelete" },
		{ 0xA9D8, "_TEActivate" },
		{ 0xA9D9, "_TEDeactivate" },
		{ 0xA9DA, "_TEIdle" },
		{ 0xA9DB, "_TEPaste" },
		{ 0xA9DC, "_TEKey" },
		{ 0xA9DD, "_TEScroll" },
		{ 0xA9DE, "_TEInsert" },
		{ 0xA9DF, "_TESetAlignment" },
		{ 0xA9E0, "_Munger" },
		{ 0xA9E1, "_HandToHand" },
		{ 0xA9E2, "_PtrToXHand" },
		{ 0xA9E3, "_PtrToHand" },
		{ 0xA9E4, "_HandAndHand" },
		{ 0xA9E5, "_InitPack" },
		{ 0xA9E6, "_InitAllPacks" },
		{ 0xA9EF, "_PtrAndHand" },
		{ 0xA9F0, "_LoadSeg" },
		{ 0xA9F1, "_UnLoadSeg" },
		{ 0xA9F2, "_Launch" },
		{ 0xA9F3, "_Chain" },
		{ 0xA9F4, "_ExitToShell" },
		{ 0xA9F5, "_GetAppParms" },
		{ 0xA9F6, "_GetResFileAttrs" },
		{ 0xA9F7, "_SetResFileAttrs" },
		{ 0xA9F8, "_MethodDispatch" },
		{ 0xA9F9, "_InfoScrap" },
		{ 0xA9FA, "_UnloadScrap" },
		{ 0xA9FB, "_LoadScrap" },
		{ 0xA9FC, "_ZeroScrap" },
		{ 0xA9FD, "_GetScrap" },
		{ 0xA9FE, "_PutScrap" },
		{ 0xA9FF, "_Debugger" },
		{ 0xAA00, "_OpenCPort" },
		{ 0xAA01, "_InitCPort" },
		{ 0xAA02, "_CloseCPort" },
		{ 0xAA03, "_NewPixMap" },
		{ 0xAA04, "_DisposePixMap" },
		{ 0xAA05, "_CopyPixMap" },
		{ 0xAA06, "_SetPortPix" },
		{ 0xAA07, "_NewPixPat" },
		{ 0xAA08, "_DisposePixPat" },
		{ 0xAA09, "_CopyPixPat" },
		{ 0xAA0A, "_PenPixPat" },
		{ 0xAA0B, "_BackPixPat" },
		{ 0xAA0C, "_GetPixPat" },
		{ 0xAA0D, "_MakeRGBPat" },
		{ 0xAA0E, "_FillCRect" },
		{ 0xAA0F, "_FillCOval" },
		{ 0xAA10, "_FillCRoundRect" },
		{ 0xAA11, "_FillCArc" },
		{ 0xAA12, "_FillCRgn" },
		{ 0xAA13, "_FillCPoly" },
		{ 0xAA14, "_RGBForeColor" },
		{ 0xAA15, "_RGBBackColor" },
		{ 0xAA16, "_SetCPixel" },
		{ 0xAA17, "_GetCPixel" },
		{ 0xAA18, "_GetCTable" },
		{ 0xAA19, "_GetForeColor" },
		{ 0xAA1A, "_GetBackColor" },
		{ 0xAA1B, "_GetCCursor" },
		{ 0xAA1C, "_SetCCursor" },
		{ 0xAA1D, "_AllocCursor" },
		{ 0xAA1E, "_GetCIcon" },
		{ 0xAA1F, "_PlotCIcon" },
		{ 0xAA20, "_OpenCPicture" },
		{ 0xAA21, "_OpColor" },
		{ 0xAA22, "_HiliteColor" },
		{ 0xAA23, "_CharExtra" },
		{ 0xAA24, "_DisposeCTable" },
		{ 0xAA25, "_DisposeCIcon" },
		{ 0xAA26, "_DisposeCCursor" },
		{ 0xAA27, "_GetMaxDevice" },
		{ 0xAA28, "_GetCTSeed" },
		{ 0xAA29, "_GetDeviceList" },
		{ 0xAA2A, "_GetMainDevice" },
		{ 0xAA2B, "_GetNextDevice" },
		{ 0xAA2C, "_TestDeviceAttribute" },
		{ 0xAA2D, "_SetDeviceAttribute" },
		{ 0xAA2E, "_InitGDevice" },
		{ 0xAA2F, "_NewGDevice" },
		{ 0xAA30, "_DisposeGDevice" },
		{ 0xAA31, "_SetGDevice" },
		{ 0xAA32, "_GetGDevice" },
		{ 0xAA35, "_InvertColor" },
		{ 0xAA36, "_RealColor" },
		{ 0xAA37, "_GetSubTable" },
		{ 0xAA38, "_UpdatePixMap" },
		{ 0xAA39, "_MakeITable" },
		{ 0xAA3A, "_AddSearch" },
		{ 0xAA3B, "_AddComp" },
		{ 0xAA3C, "_SetClientID" },
		{ 0xAA3D, "_ProtectEntry" },
		{ 0xAA3E, "_ReserveEntry" },
		{ 0xAA3F, "_SetEntries" },
		{ 0xAA40, "_QDError" },
		{ 0xAA41, "_SetWinColor" },
		{ 0xAA42, "_GetAuxWin" },
		{ 0xAA43, "_SetControlColor" },
		{ 0xAA44, "_GetAuxiliaryControlRecord" },
		{ 0xAA45, "_NewCWindow" },
		{ 0xAA46, "_GetNewCWindow" },
		{ 0xAA47, "_SetDeskCPat" },
		{ 0xAA48, "_GetCWMgrPort" },
		{ 0xAA49, "_SaveEntries" },
		{ 0xAA4A, "_RestoreEntries" },
		{ 0xAA4B, "_NewColorDialog" },
		{ 0xAA4C, "_DelSearch" },
		{ 0xAA4D, "_DelComp" },
		{ 0xAA4E, "_SetStdCProcs" },
		{ 0xAA4F, "_CalcCMask" },
		{ 0xAA50, "_SeedCFill" },
		{ 0xAA51, "_CopyDeepMask" },
		{ 0xAA52, "_HFSPinaforeDispatch" },
		{ 0xAA53, "_DictionaryDispatch" },
		{ 0xAA54, "_TextServicesDispatch" },
		{ 0xAA56, "_SpeechRecognitionDispatch" },
		{ 0xAA57, "_DockingDispatch" },
		{ 0xAA59, "_MixedModeDispatch" },
		{ 0xAA5A, "_CodeFragmentDispatch" },
		{ 0xAA5C, "_OCEUtils" },
		{ 0xAA5D, "_DigitalSignature" },
		{ 0xAA5E, "_TBDispatch" },
		{ 0xAA60, "_DeleteMCEntries" },
		{ 0xAA61, "_GetMCInfo" },
		{ 0xAA62, "_SetMCInfo" },
		{ 0xAA63, "_DisposeMCInfo" },
		{ 0xAA64, "_GetMCEntry" },
		{ 0xAA65, "_SetMCEntries" },
		{ 0xAA66, "_MenuChoice" },
		{ 0xAA67, "_ModalDialogMenuSetup" },
		{ 0xAA68, "_DialogDispatch" },
		{ 0xAA73, "_ControlDispatch" },
		{ 0xAA74, "_AppearanceDispatch" },
		{ 0xAA7E, "_SysDebugDispatch" },
		{ 0xAA80, "_AVLTreeDispatch" },
		{ 0xAA81, "_FileMappingDispatch" },
		{ 0xAA90, "_InitPalettes" },
		{ 0xAA91, "_NewPalette" },
		{ 0xAA92, "_GetNewPalette" },
		{ 0xAA93, "_DisposePalette" },
		{ 0xAA94, "_ActivatePalette" },
		{ 0xAA95, "_NSetPalette" },
		{ 0xAA96, "_GetPalette" },
		{ 0xAA97, "_PmForeColor" },
		{ 0xAA98, "_PmBackColor" },
		{ 0xAA99, "_AnimateEntry" },
		{ 0xAA9A, "_AnimatePalette" },
		{ 0xAA9B, "_GetEntryColor" },
		{ 0xAA9C, "_SetEntryColor" },
		{ 0xAA9D, "_GetEntryUsage" },
		{ 0xAA9E, "_SetEntryUsage" },
		{ 0xAAA1, "_CopyPalette" },
		{ 0xAAA2, "_PaletteDispatch" },
		{ 0xAAA3, "_CodecDispatch" },
		{ 0xAAA4, "_ALMDispatch" },
		{ 0xAADB, "_CursorDeviceDispatch" },
		{ 0xAAF2, "_ControlStripDispatch" },
		{ 0xAAF3, "_ExpansionManager" },
		{ 0xAB1D, "_QDExtensions" },
		{ 0xABC3, "_NQDMisc" },
		{ 0xABC9, "_IconDispatch" },
		{ 0xABCA, "_DeviceLoop" },
		{ 0xABEB, "_DisplayDispatch" },
		{ 0xABED, "_DragDispatch" },
		{ 0xABF1, "_GestaltValueDispatch" },
		{ 0xABF2, "_ThreadDispatch" },
		{ 0xABF6, "_CollectionMgr" },
		{ 0xABF8, "_StdOpcodeProc" },
		{ 0xABFC, "_TranslationDispatch" },
		{ 0xABFF, "_DebugStr" }
	};

	int i;

	for (i = 0; i < ARRAY_LENGTH(traps); i++)
	{
		if (traps[i].trap == opcode)
			return traps[i].name;
	}
	return nullptr;
}



static offs_t mac_dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
{
	UINT16 opcode;
	unsigned result = 0;
	const char *trap;

	opcode = oprom[0]<<8 | oprom[1];
	if ((opcode & 0xF000) == 0xA000)
	{
		trap = lookup_trap(opcode);
		if (trap)
		{
			strcpy(buffer, trap);
			result = 2;
		}
	}
	return result;
}



#ifdef MAC_TRACETRAP
void mac_state::mac_tracetrap(const char *cpu_name_local, int addr, int trap)
{
	struct sonycscodeentry
	{
		int csCode;
		const char *name;
	};

	static const sonycscodeentry cscodes[] =
	{
		{ 1, "KillIO" },
		{ 5, "VerifyDisk" },
		{ 6, "FormatDisk" },
		{ 7, "EjectDisk" },
		{ 8, "SetTagBuffer" },
		{ 9, "TrackCacheControl" },
		{ 23, "ReturnDriveInfo" }
	};

	static const char *const scsisels[] =
	{
		"SCSIReset",    /* $00 */
		"SCSIGet",      /* $01 */
		"SCSISelect",   /* $02 */
		"SCSICmd",      /* $03 */
		"SCSIComplete", /* $04 */
		"SCSIRead",     /* $05 */
		"SCSIWrite",    /* $06 */
		NULL,           /* $07 */
		"SCSIRBlind",   /* $08 */
		"SCSIWBlind",   /* $09 */
		"SCSIStat",     /* $0A */
		"SCSISelAtn",   /* $0B */
		"SCSIMsgIn",    /* $0C */
		"SCSIMsgOut",   /* $0D */
	};

	int i, a0, a7, d0, d1;
	int csCode, ioVRefNum, ioRefNum, ioCRefNum, ioCompletion, ioBuffer, ioReqCount, ioPosOffset;
	char *s;
	unsigned char *mem;
	char buf[256];
	const char *trapstr;

	trapstr = lookup_trap(trap);
	if (trapstr)
		strcpy(buf, trapstr);
	else
		sprintf(buf, "Trap $%04x", trap);

	s = &buf[strlen(buf)];
	mem = mac_ram_ptr;
	a0 = M68K_A0);
	a7 = cpu_get_reg(M68K_A7);
	d0 = cpu_get_reg(M68K_D0);
	d1 = cpu_get_reg(M68K_D1);

	switch(trap)
	{
	case 0xa004:    /* _Control */
		ioVRefNum = *((INT16*) (mem + a0 + 22));
		ioCRefNum = *((INT16*) (mem + a0 + 24));
		csCode = *((UINT16*) (mem + a0 + 26));
		sprintf(s->state().state_int(" ioVRefNum=%i ioCRefNum=%i csCode=%i", ioVRefNum, ioCRefNum, csCode);

		for (i = 0; i < ARRAY_LENGTH(cscodes); i++)
		{
			if (cscodes[i].csCode == csCode)
			{
				strcat(s, "=");
				strcat(s, cscodes[i].name);
				break;
			}
		}
		break;

	case 0xa002:    /* _Read */
		ioCompletion = (*((INT16*) (mem + a0 + 12)) << 16) + *((INT16*) (mem + a0 + 14));
		ioVRefNum = *((INT16*) (mem + a0 + 22));
		ioRefNum = *((INT16*) (mem + a0 + 24));
		ioBuffer = (*((INT16*) (mem + a0 + 32)) << 16) + *((INT16*) (mem + a0 + 34));
		ioReqCount = (*((INT16*) (mem + a0 + 36)) << 16) + *((INT16*) (mem + a0 + 38));
		ioPosOffset = (*((INT16*) (mem + a0 + 46)) << 16) + *((INT16*) (mem + a0 + 48));
		sprintf(s, " ioCompletion=0x%08x ioVRefNum=%i ioRefNum=%i ioBuffer=0x%08x ioReqCount=%i ioPosOffset=%i",
			ioCompletion, ioVRefNum, ioRefNum, ioBuffer, ioReqCount, ioPosOffset);
		break;

	case 0xa04e:    /* _AddDrive */
		sprintf(s, " drvrRefNum=%i drvNum=%i qEl=0x%08x", (int) (INT16) d0, (int) (INT16) d1, a0);
		break;

	case 0xa9a0:    /* _GetResource */
		/* HACKHACK - the 'type' output assumes that the host is little endian
		 * since this is just trace code it isn't much of an issue
		 */
		sprintf(s, " type='%c%c%c%c' id=%i", (char) mem[a7+3], (char) mem[a7+2],
			(char) mem[a7+5], (char) mem[a7+4], *((INT16*) (mem + a7)));
		break;

	case 0xa815:    /* _SCSIDispatch */
		i = *((UINT16*) (mem + a7));
		if (i < ARRAY_LENGTH(scsisels))
			if (scsisels[i])
				sprintf(s, " (%s)", scsisels[i]);
		break;
	}

	logerror("mac_trace_trap: %s at 0x%08x: %s\n",cpu_name_local, addr, buf);
}
#endif
