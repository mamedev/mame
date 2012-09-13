#include "emu.h"
#include "crsshair.h"
#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"
#include "includes/nes.h"
#include "machine/nes_mmc.h"
#include "imagedev/cartslot.h"
#include "imagedev/flopdrv.h"
#include "hashfile.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Set to dump info about the inputs to the errorlog */
#define LOG_JOY		0

/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG	0
#define SPLIT_CHR	0

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void init_nes_core(running_machine &machine);
static void nes_machine_stop(running_machine &machine);


static void fds_irq(device_t *device, int scanline, int vblank, int blanked);

/***************************************************************************
    FUNCTIONS
***************************************************************************/

static void init_nes_core( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	static const char *const bank_names[] = { "bank1", "bank2", "bank3", "bank4" };
	int prg_banks = (state->m_prg_chunks == 1) ? (2 * 2) : (state->m_prg_chunks * 2);
	int i;

	state->m_rom = machine.root_device().memregion("maincpu")->base();
	state->m_ciram = machine.root_device().memregion("ciram")->base();
	// other pointers got set in the loading routine

	/* Brutal hack put in as a consequence of the new memory system; we really need to fix the NES code */
	space->install_readwrite_bank(0x0000, 0x07ff, 0, 0x1800, "bank10");

	machine.device("ppu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0, 0x1fff, FUNC(nes_chr_r), FUNC(nes_chr_w));
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2000, 0x3eff, FUNC(nes_nt_r), FUNC(nes_nt_w));

	state->membank("bank10")->set_base(state->m_rom);

	/* If there is Disk Expansion and no cart has been loaded, setup memory accordingly */
	if (state->m_disk_expansion && state->m_pcb_id == NO_BOARD)
	{
		/* If we are loading a disk we have already filled state->m_fds_data and we don't want to overwrite it,
         if we are loading a cart image identified as mapper 20 (probably wrong mapper...) we need to alloc
         memory for use in nes_fds_r/nes_fds_w. Same goes for allocation of fds_ram (used for bank2)  */
		if (state->m_fds_data == NULL)
		{
			UINT32 size = (state->m_prg_chunks == 1) ? 2 * 0x4000 : state->m_prg_chunks * 0x4000;
			state->m_fds_data = auto_alloc_array_clear(machine, UINT8, size);
			memcpy(state->m_fds_data, state->m_prg, size);	// copy in fds_data the cart PRG
		}
		if (state->m_fds_ram == NULL)
			state->m_fds_ram = auto_alloc_array(machine, UINT8, 0x8000);

		space->install_read_handler(0x4030, 0x403f, read8_delegate(FUNC(nes_state::nes_fds_r),state));
		space->install_read_bank(0x6000, 0xdfff, "bank2");
		space->install_read_bank(0xe000, 0xffff, "bank1");

		space->install_write_handler(0x4020, 0x402f, write8_delegate(FUNC(nes_state::nes_fds_w),state));
		space->install_write_bank(0x6000, 0xdfff, "bank2");

		state->membank("bank1")->set_base(&state->m_rom[0xe000]);
		state->membank("bank2")->set_base(state->m_fds_ram);
		return;
	}

	/* Set up the mapper callbacks */
	pcb_handlers_setup(machine);

	/* Set up the memory handlers for the mapper */
	space->install_read_bank(0x8000, 0x9fff, "bank1");
	space->install_read_bank(0xa000, 0xbfff, "bank2");
	space->install_read_bank(0xc000, 0xdfff, "bank3");
	space->install_read_bank(0xe000, 0xffff, "bank4");
	space->install_readwrite_bank(0x6000, 0x7fff, "bank5");

	/* configure banks 1-4 */
	for (i = 0; i < 4; i++)
	{
		state->membank(bank_names[i])->configure_entries(0, prg_banks, state->m_prg, 0x2000);
		// some mappers (e.g. MMC5) can map PRG RAM in  0x8000-0xffff as well
		if (state->m_prg_ram)
			state->membank(bank_names[i])->configure_entries(prg_banks, state->m_wram_size / 0x2000, state->m_wram, 0x2000);
		// however, at start we point to PRG ROM
		state->membank(bank_names[i])->set_entry(i);
		state->m_prg_bank[i] = i;
	}

	/* bank 5 configuration is more delicate, since it can have PRG RAM, PRG ROM or SRAM mapped to it */
	/* we first map PRG ROM banks, then the battery bank (if a battery is present), and finally PRG RAM (state->m_wram) */
	state->membank("bank5")->configure_entries(0, prg_banks, state->m_prg, 0x2000);
	state->m_battery_bank5_start = prg_banks;
	state->m_prgram_bank5_start = prg_banks;
	state->m_empty_bank5_start = prg_banks;

	/* add battery ram, but only if there's no trainer since they share overlapping memory. */
	if (state->m_battery && !state->m_trainer)
	{
		UINT32 bank_size = (state->m_battery_size > 0x2000) ? 0x2000 : state->m_battery_size;
		int bank_num = (state->m_battery_size > 0x2000) ? state->m_battery_size / 0x2000 : 1;
		state->membank("bank5")->configure_entries(prg_banks, bank_num, state->m_battery_ram, bank_size);
		state->m_prgram_bank5_start += bank_num;
		state->m_empty_bank5_start += bank_num;
	}
	/* add prg ram. */
	if (state->m_prg_ram)
	{
		state->membank("bank5")->configure_entries(state->m_prgram_bank5_start, state->m_wram_size / 0x2000, state->m_wram, 0x2000);
		state->m_empty_bank5_start += state->m_wram_size / 0x2000;
	}

	state->membank("bank5")->configure_entry(state->m_empty_bank5_start, state->m_rom + 0x6000);

	/* if we have any additional PRG RAM, point bank5 to its first bank */
	if (state->m_battery || state->m_prg_ram)
		state->m_prg_bank[4] = state->m_battery_bank5_start;
	else
		state->m_prg_bank[4] = state->m_empty_bank5_start; // or shall we point to "maincpu" region at 0x6000? point is that we should never access this region if no sram or wram is present!

	state->membank("bank5")->set_entry(state->m_prg_bank[4]);

	if (state->m_four_screen_vram)
	{
		state->m_extended_ntram = auto_alloc_array_clear(machine, UINT8, 0x2000);
		state->save_pointer(NAME(state->m_extended_ntram), 0x2000);
	}

	if (state->m_four_screen_vram)
		set_nt_mirroring(machine, PPU_MIRROR_4SCREEN);
	else
	{
		switch (state->m_hard_mirroring)
		{
			case PPU_MIRROR_HORZ:
			case PPU_MIRROR_VERT:
			case PPU_MIRROR_HIGH:
			case PPU_MIRROR_LOW:
				set_nt_mirroring(machine, state->m_hard_mirroring);
				break;
			default:
				set_nt_mirroring(machine, PPU_MIRROR_NONE);
				break;
		}
	}

	// there are still some quirk about writes to bank5... I hope to fix them soon. (mappers 34,45,52,246 have both mid_w and WRAM-->check)
	if (state->m_mmc_write_mid)
		space->install_legacy_write_handler(0x6000, 0x7fff, state->m_mmc_write_mid,state->m_mmc_write_mid_name);
	if (state->m_mmc_write)
		space->install_legacy_write_handler(0x8000, 0xffff, state->m_mmc_write, state->m_mmc_write_name);

	// In fact, we also allow single pcbs to overwrite the bank read handlers defined above,
	// because some pcbs (mainly pirate ones) require protection values to be read instead of
	// the expected ROM banks: these handlers, though, must take care of the ROM access as well
	if (state->m_mmc_read_mid)
		space->install_legacy_read_handler(0x6000, 0x7fff, state->m_mmc_read_mid,state->m_mmc_read_mid_name);
	if (state->m_mmc_read)
		space->install_legacy_read_handler(0x8000, 0xffff, state->m_mmc_read,state->m_mmc_read_name);

	// install additional handlers
	if (state->m_pcb_id == BTL_SMB2B || state->m_mapper == 50)
	{
		space->install_legacy_write_handler(0x4020, 0x403f, FUNC(smb2jb_extra_w));
		space->install_legacy_write_handler(0x40a0, 0x40bf, FUNC(smb2jb_extra_w));
	}

	if (state->m_pcb_id == KAISER_KS7017)
	{
		space->install_legacy_read_handler(0x4030, 0x4030, FUNC(ks7017_extra_r));
		space->install_legacy_write_handler(0x4020, 0x40ff, FUNC(ks7017_extra_w));
	}

	if (state->m_pcb_id == UNL_603_5052)
	{
		space->install_legacy_read_handler(0x4020, 0x40ff, FUNC(unl_6035052_extra_r));
		space->install_legacy_write_handler(0x4020, 0x40ff, FUNC(unl_6035052_extra_w));
	}

	if (state->m_pcb_id == WAIXING_SH2)
		machine.device("ppu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0, 0x1fff, FUNC(waixing_sh2_chr_r));
}

// to be probably removed (it does nothing since a long time)
int nes_ppu_vidaccess( device_t *device, int address, int data )
{
	return data;
}

void nes_state::machine_reset()
{

	/* Reset the mapper variables. Will also mark the char-gen ram as dirty */
	if (m_disk_expansion && m_pcb_id == NO_BOARD)
		m_ppu->set_hblank_callback(fds_irq);
	else
		nes_pcb_reset(machine());

	/* Reset the serial input ports */
	m_in_0.shift = 0;
	m_in_1.shift = 0;

	machine().device("maincpu")->reset();
}

static TIMER_CALLBACK( nes_irq_callback )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	state->m_irq_timer->adjust(attotime::never);
}

static void nes_banks_restore(nes_state *state)
{
	state->membank("bank1")->set_entry(state->m_prg_bank[0]);
	state->membank("bank2")->set_entry(state->m_prg_bank[1]);
	state->membank("bank3")->set_entry(state->m_prg_bank[2]);
	state->membank("bank4")->set_entry(state->m_prg_bank[3]);
	state->membank("bank5")->set_entry(state->m_prg_bank[4]);
}

static void nes_state_register( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->save_item(NAME(state->m_prg_bank));

	state->save_item(NAME(state->m_MMC5_floodtile));
	state->save_item(NAME(state->m_MMC5_floodattr));
	state->save_item(NAME(state->m_mmc5_vram_control));

	state->save_item(NAME(state->m_nes_vram_sprite));
	state->save_item(NAME(state->m_last_frame_flip));

	// shared mapper variables
	state->save_item(NAME(state->m_IRQ_enable));
	state->save_item(NAME(state->m_IRQ_enable_latch));
	state->save_item(NAME(state->m_IRQ_count));
	state->save_item(NAME(state->m_IRQ_count_latch));
	state->save_item(NAME(state->m_IRQ_toggle));
	state->save_item(NAME(state->m_IRQ_reset));
	state->save_item(NAME(state->m_IRQ_status));
	state->save_item(NAME(state->m_IRQ_mode));
	state->save_item(NAME(state->m_mult1));
	state->save_item(NAME(state->m_mult2));
	state->save_item(NAME(state->m_mmc_chr_source));
	state->save_item(NAME(state->m_mmc_cmd1));
	state->save_item(NAME(state->m_mmc_cmd2));
	state->save_item(NAME(state->m_mmc_count));
	state->save_item(NAME(state->m_mmc_prg_base));
	state->save_item(NAME(state->m_mmc_prg_mask));
	state->save_item(NAME(state->m_mmc_chr_base));
	state->save_item(NAME(state->m_mmc_chr_mask));
	state->save_item(NAME(state->m_mmc_prg_bank));
	state->save_item(NAME(state->m_mmc_vrom_bank));
	state->save_item(NAME(state->m_mmc_extra_bank));

	state->save_item(NAME(state->m_fds_motor_on));
	state->save_item(NAME(state->m_fds_door_closed));
	state->save_item(NAME(state->m_fds_current_side));
	state->save_item(NAME(state->m_fds_head_position));
	state->save_item(NAME(state->m_fds_status0));
	state->save_item(NAME(state->m_fds_read_mode));
	state->save_item(NAME(state->m_fds_write_reg));
	state->save_item(NAME(state->m_fds_last_side));
	state->save_item(NAME(state->m_fds_count));

	state->save_pointer(NAME(state->m_wram), state->m_wram_size);
	if (state->m_battery)
		state->save_pointer(NAME(state->m_battery_ram), state->m_battery_size);

	machine.save().register_postload(save_prepost_delegate(FUNC(nes_banks_restore), state));
}

void nes_state::machine_start()
{

	m_ppu = machine().device<ppu2c0x_device>("ppu");
	init_nes_core(machine());
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(nes_machine_stop),&machine()));

	m_maincpu = machine().device<cpu_device>("maincpu");
	m_sound = machine().device("nessound");
	m_cart = machine().device("cart");

	// If we're starting famicom with no disk inserted, we still haven't initialized the VRAM needed for
	// video emulation, so we need to take care of it now
	if (!m_vram)
	{
		m_vram = auto_alloc_array(machine(), UINT8, 0x4000);
		for (int i = 0; i < 8; i++)
		{
			m_chr_map[i].source = CHRRAM;
			m_chr_map[i].origin = i * 0x400; // for save state uses!
			m_chr_map[i].access = &m_vram[m_chr_map[i].origin];
		}
	}

	m_irq_timer = machine().scheduler().timer_alloc(FUNC(nes_irq_callback));
	nes_state_register(machine());
}

static void nes_machine_stop( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	device_image_interface *image = dynamic_cast<device_image_interface *>(state->m_cart);
	/* Write out the battery file if necessary */
	if (state->m_battery)
		image->battery_save(state->m_battery_ram, state->m_battery_size);

	if (state->m_mapper_bram_size)
		image->battery_save(state->m_mapper_bram, state->m_mapper_bram_size);
}



READ8_MEMBER(nes_state::nes_IN0_r)
{
	int cfg = ioport("CTRLSEL")->read();
	int ret;

	if ((cfg & 0x000f) >= 0x07)	// for now we treat the FC keyboard separately from other inputs!
	{
		// here we should have the tape input
		ret = 0;
	}
	else
	{
		/* Some games expect bit 6 to be set because the last entry on the data bus shows up */
		/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
		ret = 0x40;

		ret |= ((m_in_0.i0 >> m_in_0.shift) & 0x01);

		/* zapper */
		if ((cfg & 0x000f) == 0x0002)
		{
			int x = m_in_0.i1;	/* read Zapper x-position */
			int y = m_in_0.i2;	/* read Zapper y-position */
			UINT32 pix, color_base;

			/* get the pixel at the gun position */
			pix = m_ppu->get_pixel(x, y);

			/* get the color base from the ppu */
			color_base = m_ppu->get_colorbase();

			/* look at the screen and see if the cursor is over a bright pixel */
			if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
				(pix == color_base + 0x33) || (pix == color_base + 0x34))
			{
				ret &= ~0x08; /* sprite hit */
			}
			else
				ret |= 0x08;  /* no sprite hit */

			/* If button 1 is pressed, indicate the light gun trigger is pressed */
			ret |= ((m_in_0.i0 & 0x01) << 4);
		}

		if (LOG_JOY)
			logerror("joy 0 read, val: %02x, pc: %04x, bits read: %d, chan0: %08x\n", ret, space.device().safe_pc(), m_in_0.shift, m_in_0.i0);

		m_in_0.shift++;
	}

	return ret;
}

// row of the keyboard matrix are read 4-bits at time, and gets returned as bit1->bit4
static UINT8 nes_read_fc_keyboard_line( running_machine &machine, UINT8 scan, UINT8 mode )
{
	static const char *const fc_keyport_names[] = { "FCKEY0", "FCKEY1", "FCKEY2", "FCKEY3", "FCKEY4", "FCKEY5", "FCKEY6", "FCKEY7", "FCKEY8" };
	return ((machine.root_device().ioport(fc_keyport_names[scan])->read() >> (mode * 4)) & 0x0f) << 1;
}

static UINT8 nes_read_subor_keyboard_line( running_machine &machine, UINT8 scan, UINT8 mode )
{
	static const char *const sub_keyport_names[] = { "SUBKEY0", "SUBKEY1", "SUBKEY2", "SUBKEY3", "SUBKEY4",
		"SUBKEY5", "SUBKEY6", "SUBKEY7", "SUBKEY8", "SUBKEY9", "SUBKEY10", "SUBKEY11", "SUBKEY12" };
	return ((machine.root_device().ioport(sub_keyport_names[scan])->read() >> (mode * 4)) & 0x0f) << 1;
}

READ8_MEMBER(nes_state::nes_IN1_r)
{
	int cfg = ioport("CTRLSEL")->read();
	int ret;

	if ((cfg & 0x000f) == 0x07)	// for now we treat the FC keyboard separately from other inputs!
	{
		if (m_fck_scan < 9)
			ret = ~nes_read_fc_keyboard_line(machine(), m_fck_scan, m_fck_mode) & 0x1e;
		else
			ret = 0x1e;
	}
	else if ((cfg & 0x000f) == 0x08)	// for now we treat the Subor keyboard separately from other inputs!
	{
		if (m_fck_scan < 12)
			ret = ~nes_read_subor_keyboard_line(machine(), m_fck_scan, m_fck_mode) & 0x1e;
		else
			ret = 0x1e;
	}
	else
	{
		/* Some games expect bit 6 to be set because the last entry on the data bus shows up */
		/* in the unused upper 3 bits, so typically a read from $4017 leaves 0x40 there. */
		ret = 0x40;

		/* Handle data line 0's serial output */
		ret |= ((m_in_1.i0 >> m_in_1.shift) & 0x01);

		/* zapper */
		if ((cfg & 0x00f0) == 0x0030)
		{
			int x = m_in_1.i1;	/* read Zapper x-position */
			int y = m_in_1.i2;	/* read Zapper y-position */
			UINT32 pix, color_base;

			/* get the pixel at the gun position */
			pix = m_ppu->get_pixel(x, y);

			/* get the color base from the ppu */
			color_base = m_ppu->get_colorbase();

			/* look at the screen and see if the cursor is over a bright pixel */
			if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
				(pix == color_base + 0x33) || (pix == color_base + 0x34))
			{
				ret &= ~0x08; /* sprite hit */
			}
			else
				ret |= 0x08;  /* no sprite hit */

			/* If button 1 is pressed, indicate the light gun trigger is pressed */
			ret |= ((m_in_1.i0 & 0x01) << 4);
		}

		/* arkanoid dial */
		else if ((cfg & 0x00f0) == 0x0040)
		{
			/* Handle data line 2's serial output */
			ret |= ((m_in_1.i2 >> m_in_1.shift) & 0x01) << 3;

			/* Handle data line 3's serial output - bits are reversed */
			/* NPW 27-Nov-2007 - there is no third subscript! commenting out */
			/* ret |= ((m_in_1[3] >> m_in_1.shift) & 0x01) << 4; */
			/* ret |= ((m_in_1[3] << m_in_1.shift) & 0x80) >> 3; */
		}

		if (LOG_JOY)
			logerror("joy 1 read, val: %02x, pc: %04x, bits read: %d, chan0: %08x\n", ret, space.device().safe_pc(), m_in_1.shift, m_in_1.i0);

		m_in_1.shift++;
	}

	return ret;
}


// FIXME: this is getting messier and messier (no pun intended). inputs reading should be simplified and port_categories cleaned up
// to also emulate the fact that nothing should be in Port 2 if there is a Crazy Climber pad, etc.
static void nes_read_input_device( running_machine &machine, int cfg, nes_input *vals, int pad_port, int supports_zapper )
{
	nes_state *state = machine.driver_data<nes_state>();
	static const char *const padnames[] = { "PAD1", "PAD2", "PAD3", "PAD4", "CC_LEFT", "CC_RIGHT" };

	vals->i0 = 0;
	vals->i1 = 0;
	vals->i2 = 0;

	switch (cfg & 0x0f)
	{
		case 0x01:	/* gamepad */
			if (pad_port >= 0)
				vals->i0 = machine.root_device().ioport(padnames[pad_port])->read();
			break;

		case 0x02:	/* zapper 1 */
			if (supports_zapper)
			{
				vals->i0 = machine.root_device().ioport("ZAPPER1_T")->read();
				vals->i1 = machine.root_device().ioport("ZAPPER1_X")->read();
				vals->i2 = machine.root_device().ioport("ZAPPER1_Y")->read();
			}
			break;

		case 0x03:	/* zapper 2 */
			if (supports_zapper)
			{
				vals->i0 = machine.root_device().ioport("ZAPPER2_T")->read();
				vals->i1 = machine.root_device().ioport("ZAPPER2_X")->read();
				vals->i2 = machine.root_device().ioport("ZAPPER2_Y")->read();
			}
			break;

		case 0x04:	/* arkanoid paddle */
			if (pad_port == 1)
				vals->i0 = (UINT8) ((UINT8) machine.root_device().ioport("PADDLE")->read() + (UINT8)0x52) ^ 0xff;
			break;

		case 0x06:	/* crazy climber controller */
			if (pad_port == 0)
			{
				state->m_in_0.i0 = machine.root_device().ioport(padnames[4])->read();
				state->m_in_1.i0 = machine.root_device().ioport(padnames[5])->read();
			}
			break;
	}
}


static TIMER_CALLBACK( lightgun_tick )
{
	if ((machine.root_device().ioport("CTRLSEL")->read() & 0x000f) == 0x0002)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_NONE);
	}

	if ((machine.root_device().ioport("CTRLSEL")->read() & 0x00f0) == 0x0030)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 1, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 1, CROSSHAIR_SCREEN_NONE);
	}
}

WRITE8_MEMBER(nes_state::nes_IN0_w)
{
	int cfg = ioport("CTRLSEL")->read();

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine().scheduler().timer_set(attotime::zero, FUNC(lightgun_tick));

	if ((cfg & 0x000f) >= 0x07)	// for now we treat the FC keyboard separately from other inputs!
	{
		// here we should also have the tape output

		if (BIT(data, 2))	// keyboard active
		{
			int lines = ((cfg & 0x000f) == 0x04) ? 9 : 12;
			UINT8 out = BIT(data, 1);	// scan

			if (m_fck_mode && !out && ++m_fck_scan > lines)
				m_fck_scan = 0;

			m_fck_mode = out;	// access lower or upper 4 bits

			if (BIT(data, 0))	// reset
				m_fck_scan = 0;
		}
	}
	else
	{
		if (data & 0x01)
			return;

		if (LOG_JOY)
			logerror("joy 0 bits read: %d\n", m_in_0.shift);

		/* Toggling bit 0 high then low resets both controllers */
		m_in_0.shift = 0;
		m_in_1.shift = 0;

		/* Read the input devices */
		if ((cfg & 0x000f) != 0x06)
		{
			nes_read_input_device(machine(), cfg >>  0, &m_in_0, 0,  TRUE);
			nes_read_input_device(machine(), cfg >>  4, &m_in_1, 1,  TRUE);
			nes_read_input_device(machine(), cfg >>  8, &m_in_2, 2, FALSE);
			nes_read_input_device(machine(), cfg >> 12, &m_in_3, 3, FALSE);
		}
		else // crazy climber pad
		{
			nes_read_input_device(machine(), 0, &m_in_1, 1,  TRUE);
			nes_read_input_device(machine(), 0, &m_in_2, 2, FALSE);
			nes_read_input_device(machine(), 0, &m_in_3, 3, FALSE);
			nes_read_input_device(machine(), cfg >>  0, &m_in_0, 0,  TRUE);
		}

		if (cfg & 0x0f00)
			m_in_0.i0 |= (m_in_2.i0 << 8) | (0x08 << 16);

		if (cfg & 0xf000)
			m_in_1.i0 |= (m_in_3.i0 << 8) | (0x04 << 16);
	}
}


WRITE8_MEMBER(nes_state::nes_IN1_w)
{
}

struct nes_cart_lines
{
	const char *tag;
	int line;
};

static const struct nes_cart_lines nes_cart_lines_table[] =
{
	{ "PRG A0",    0 },
	{ "PRG A1",    1 },
	{ "PRG A2",    2 },
	{ "PRG A3",    3 },
	{ "PRG A4",    4 },
	{ "PRG A5",    5 },
	{ "PRG A6",    6 },
	{ "PRG A7",    7 },
	{ "CHR A10",  10 },
	{ "CHR A11",  11 },
	{ "CHR A12",  12 },
	{ "CHR A13",  13 },
	{ "CHR A14",  14 },
	{ "CHR A15",  15 },
	{ "CHR A16",  16 },
	{ "CHR A17",  17 },
	{ "NC",      127 },
	{ 0 }
};

static int nes_cart_get_line( const char *feature )
{
	const struct nes_cart_lines *nes_line = &nes_cart_lines_table[0];

	if (feature == NULL)
		return 128;

	while (nes_line->tag)
	{
		if (strcmp(nes_line->tag, feature) == 0)
			break;

		nes_line++;
	}

	return nes_line->line;
}

DEVICE_IMAGE_LOAD( nes_cart )
{
	nes_state *state = image.device().machine().driver_data<nes_state>();
	state->m_pcb_id = NO_BOARD;	// initialization

	if (image.software_entry() == NULL)
	{
		const char *mapinfo = NULL;
		int mapint1 = 0, mapint2 = 0, mapint3 = 0, mapint4 = 0; //, goodcrcinfo = 0;
		char magic[4], extend[5];
		int local_options = 0;
		char m;

		/* Check first 4 bytes of the image to decide if it is UNIF or iNES */
		/* Unfortunately, many .unf files have been released as .nes, so we cannot rely on extensions only */
		memset(magic, '\0', sizeof(magic));
		image.fread(magic, 4);

		if ((magic[0] == 'N') && (magic[1] == 'E') && (magic[2] == 'S'))	/* If header starts with 'NES' it is iNES */
		{
			UINT32 prg_size;
			state->m_ines20 = 0;
			state->m_battery_size = NES_BATTERY_SIZE; // with iNES we can only support 8K WRAM battery (iNES 2.0 might fix this)
			state->m_prg_ram = 1;	// always map state->m_wram in bank5 (eventually, this should be enabled only for some mappers)

			// check if the image is recognized by nes.hsi
			mapinfo = hashfile_extrainfo(image);

			// image_extrainfo() resets the file position back to start.
			// Let's skip past the magic header once again.
			image.fseek(4, SEEK_SET);

			image.fread(&state->m_prg_chunks, 1);
			image.fread(&state->m_chr_chunks, 1);
			/* Read the first ROM option byte (offset 6) */
			image.fread(&m, 1);

			/* Interpret the iNES header flags */
			state->m_mapper = (m & 0xf0) >> 4;
			local_options = m & 0x0f;

			/* Read the second ROM option byte (offset 7) */
			image.fread(&m, 1);

			switch (m & 0xc)
			{
				case 0x4:
				case 0xc:
					// probably the header got corrupted: don't trust upper bits for mapper
					break;

				case 0x8:	// it's iNES 2.0 format
					state->m_ines20 = 1;
				case 0x0:
				default:
					state->m_mapper = state->m_mapper | (m & 0xf0);
					break;
			}

			if (mapinfo /* && !state->m_ines20 */)
			{
				if (4 == sscanf(mapinfo,"%d %d %d %d", &mapint1, &mapint2, &mapint3, &mapint4))
				{
					/* image is present in nes.hsi: overwrite the header settings with these */
					state->m_mapper = mapint1;
					local_options = mapint2 & 0x0f;
					state->m_crc_hack = (mapint2 & 0xf0) >> 4;	// this is used to differentiate among variants of the same Mapper (see below)
					state->m_prg_chunks = mapint3;
					state->m_chr_chunks = mapint4;
					logerror("NES.HSI info: %d %d %d %d\n", mapint1, mapint2, mapint3, mapint4);
//                  mame_printf_error("NES.HSI info: %d %d %d %d\n", mapint1, mapint2, mapint3, mapint4);
//                  goodcrcinfo = 1;
					state->m_ines20 = 0;
				}
				else
				{
					logerror("NES: [%s], Invalid mapinfo found\n", mapinfo);
				}
			}
			else
			{
				logerror("NES: No extrainfo found\n");
			}

			state->m_hard_mirroring = (local_options & 0x01) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ;
//          mame_printf_error("%s\n", state->m_hard_mirroring & 0x01 ? "Vertical" : "Horizontal");
			state->m_battery = local_options & 0x02;
			state->m_trainer = local_options & 0x04;
			state->m_four_screen_vram = local_options & 0x08;

			if (state->m_battery)
				logerror("-- Battery found\n");

			if (state->m_trainer)
				logerror("-- Trainer found\n");

			if (state->m_four_screen_vram)
				logerror("-- 4-screen VRAM\n");

			if (state->m_ines20)
			{
				logerror("Extended iNES format:\n");
				image.fread(&extend, 5);
				state->m_mapper |= (extend[0] & 0x0f) << 8;
				logerror("-- mapper: %d\n", state->m_mapper);
				logerror("-- submapper: %d\n", (extend[0] & 0xf0) >> 4);
				state->m_prg_chunks |= ((extend[1] & 0x0f) << 8);
				state->m_chr_chunks |= ((extend[1] & 0xf0) << 4);
				logerror("-- PRG chunks: %d\n", state->m_prg_chunks);
				logerror("-- CHR chunks: %d\n", state->m_chr_chunks);
				logerror("-- PRG NVWRAM: %d\n", extend[2] & 0x0f);
				logerror("-- PRG WRAM: %d\n", (extend[2] & 0xf0) >> 4);
				logerror("-- CHR NVWRAM: %d\n", extend[3] & 0x0f);
				logerror("-- CHR WRAM: %d\n", (extend[3] & 0xf0) >> 4);
				logerror("-- TV System: %d\n", extend[4] & 3);
			}

			// Allocate class pointers for PRG/VROM/VRAM/WRAM
			prg_size = (state->m_prg_chunks == 1) ? 2 * 0x4000 : state->m_prg_chunks * 0x4000;
			state->m_prg = auto_alloc_array(image.device().machine(), UINT8, prg_size);
			if (state->m_chr_chunks)
				state->m_vrom = auto_alloc_array(image.device().machine(), UINT8, state->m_chr_chunks * 0x2000);

			state->m_vram_chunks = 2;
			state->m_vram = auto_alloc_array(image.device().machine(), UINT8, 0x4000);

			// FIXME: this should only be allocated if there is actual wram in the cart (i.e. if state->m_prg_ram = 1)!
			// or if there is a trainer, I think
			state->m_wram_size = 0x10000;
			state->m_wram = auto_alloc_array(image.device().machine(), UINT8, state->m_wram_size);

			/* Setup PCB type (needed to add proper handlers later) */
			state->m_pcb_id = nes_get_mmc_id(image.device().machine(), state->m_mapper);

			// a few mappers correspond to multiple PCBs, so we need a few additional checks
			switch (state->m_pcb_id)
			{
				case STD_CNROM:
					if (state->m_mapper == 185)
					{
						switch (state->m_crc_hack)
						{
							case 0x0: // pin26: CE, pin27: CE (B-Wings, Bird Week)
								state->m_ce_mask = 0x03;
								state->m_ce_state = 0x03;
								break;
							case 0x4: // pin26: CE, pin27: /CE (Mighty Bomb Jack, Spy Vs. Spy)
								state->m_ce_mask = 0x03;
								state->m_ce_state = 0x01;
								break;
							case 0x8: // pin26: /CE, pin27: CE (Sansu 1, 2, 3 Nen)
								state->m_ce_mask = 0x03;
								state->m_ce_state = 0x02;
								break;
							case 0xc: // pin26: /CE, pin27: /CE (Seicross v2.0)
								state->m_ce_mask = 0x03;
								state->m_ce_state = 0x00;
								break;
						}
					}
					break;
				case KONAMI_VRC2:
					if (state->m_mapper == 22)
					{
						state->m_vrc_ls_prg_a = 0;
						state->m_vrc_ls_prg_b = 1;
						state->m_vrc_ls_chr = 1;
					}
					if (state->m_mapper == 23 && !state->m_crc_hack)
					{
						state->m_vrc_ls_prg_a = 1;
						state->m_vrc_ls_prg_b = 0;
						state->m_vrc_ls_chr = 0;
					}
					if (state->m_mapper == 23 && state->m_crc_hack)
					{
						// here there are also Akumajou Special, Crisis Force, Parodius da!, Tiny Toons which are VRC-4
						state->m_vrc_ls_prg_a = 3;
						state->m_vrc_ls_prg_b = 2;
						state->m_pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
					}
					break;
				case KONAMI_VRC4:
					if (state->m_mapper == 21)
					{
						// Wai Wai World 2 & Ganbare Goemon Gaiden 2 (the latter with crc_hack)
						state->m_vrc_ls_prg_a = state->m_crc_hack ? 7 : 2;
						state->m_vrc_ls_prg_b = state->m_crc_hack ? 6 : 1;
					}
					if (state->m_mapper == 25)	// here there is also Ganbare Goemon Gaiden which is VRC-2
					{
						state->m_vrc_ls_prg_a = state->m_crc_hack ? 2 : 0;
						state->m_vrc_ls_prg_b = state->m_crc_hack ? 3 : 1;
					}
					break;
				case KONAMI_VRC6:
					if (state->m_mapper == 24)
					{
						state->m_vrc_ls_prg_a = 1;
						state->m_vrc_ls_prg_b = 0;
					}
					if (state->m_mapper == 26)
					{
						state->m_vrc_ls_prg_a = 0;
						state->m_vrc_ls_prg_b = 1;
					}
					break;
				case IREM_G101:
					if (state->m_crc_hack)
						state->m_hard_mirroring = PPU_MIRROR_HIGH;	// Major League has hardwired mirroring
					break;
				case DIS_74X161X161X32:
					if (state->m_mapper == 70)
						state->m_hard_mirroring = PPU_MIRROR_VERT;	// only hardwired mirroring makes different mappers 70 & 152
					break;
				case SUNSOFT_2:
					if (state->m_mapper == 93)
						state->m_hard_mirroring = PPU_MIRROR_VERT;	// only hardwired mirroring makes different mappers 89 & 93
					break;
				case STD_BXROM:
					if (state->m_crc_hack)
						state->m_pcb_id = AVE_NINA01;	// Mapper 34 is used for 2 diff boards
					break;
				case BANDAI_LZ93:
					if (state->m_crc_hack)
						state->m_pcb_id = BANDAI_JUMP2;	// Mapper 153 is used for 2 diff boards
					break;
				case IREM_HOLYDIV:
					if (state->m_crc_hack)
						state->m_pcb_id = JALECO_JF16;	// Mapper 78 is used for 2 diff boards
					break;
				case CAMERICA_BF9093:
					if (state->m_crc_hack)
						state->m_pcb_id = CAMERICA_BF9097;	// Mapper 71 is used for 2 diff boards
					break;
				case HES_BOARD:
					if (state->m_crc_hack)
						state->m_pcb_id = HES6IN1_BOARD;	// Mapper 113 is used for 2 diff boards
					break;
				case WAIXING_ZS:
					if (state->m_crc_hack)
						state->m_pcb_id = WAIXING_DQ8;	// Mapper 242 is used for 2 diff boards
					break;
				case BMC_GOLD_7IN1:
					if (state->m_crc_hack)
						state->m_pcb_id = BMC_MARIOPARTY_7IN1;	// Mapper 52 is used for 2 diff boards
					break;
					//FIXME: we also have to fix Action 52 PRG loading somewhere...
			}

			/* Allocate internal Mapper RAM for boards which require it */
			if (state->m_pcb_id == STD_EXROM)
				state->m_mapper_ram = auto_alloc_array(image.device().machine(), UINT8, 0x400);

			if (state->m_pcb_id == TAITO_X1_005 || state->m_pcb_id == TAITO_X1_005_A)
				state->m_mapper_bram = auto_alloc_array(image.device().machine(), UINT8, 0x80);

			if (state->m_pcb_id == TAITO_X1_017)
				state->m_mapper_bram = auto_alloc_array(image.device().machine(), UINT8, 0x1400);

			if (state->m_pcb_id == NAMCOT_163)
				state->m_mapper_ram = auto_alloc_array(image.device().machine(), UINT8, 0x2000);

			if (state->m_pcb_id == FUKUTAKE_BOARD)
				state->m_mapper_ram = auto_alloc_array(image.device().machine(), UINT8, 2816);

			/* Position past the header */
			image.fseek(16, SEEK_SET);

			/* Load the 0x200 byte trainer at 0x7000 if it exists */
			if (state->m_trainer)
				image.fread(&state->m_wram[0x1000], 0x200);

			/* Read in the program chunks */
			image.fread(&state->m_prg[0], 0x4000 * state->m_prg_chunks);
			if (state->m_prg_chunks == 1)
				memcpy(&state->m_prg[0x4000], &state->m_prg[0], 0x4000);

#if SPLIT_PRG
			{
				FILE *prgout;
				char outname[255];

				sprintf(outname, "%s.prg", image.filename());
				prgout = fopen(outname, "wb");
				if (prgout)
				{
					fwrite(&state->m_prg[0], 1, 0x4000 * state->m_prg_chunks, prgout);
					mame_printf_error("Created PRG chunk\n");
				}

				fclose(prgout);
			}
#endif

			logerror("**\n");
			logerror("Mapper: %d\n", state->m_mapper);
			logerror("PRG chunks: %02x, size: %06x\n", state->m_prg_chunks, 0x4000 * state->m_prg_chunks);
			// mame_printf_error("Mapper: %d\n", state->m_mapper);
			// mame_printf_error("PRG chunks: %02x, size: %06x\n", state->m_prg_chunks, 0x4000 * state->m_prg_chunks);

			/* Read in any chr chunks */
			if (state->m_chr_chunks > 0)
			{
				image.fread(state->m_vrom, state->m_chr_chunks * 0x2000);
				if (state->m_mapper == 2)
					logerror("Warning: VROM has been found in VRAM-based mapper. Either the mapper is set wrong or the ROM image is incorrect.\n");
			}

#if SPLIT_CHR
			if (state->m_chr_chunks > 0)
			{
				FILE *chrout;
				char outname[255];

				sprintf(outname, "%s.chr", image.filename());
				chrout= fopen(outname, "wb");
				if (chrout)
				{
					fwrite(state->m_vrom, 1, 0x2000 * state->m_chr_chunks, chrout);
					mame_printf_error("Created CHR chunk\n");
				}
				fclose(chrout);
			}
#endif

			logerror("CHR chunks: %02x, size: %06x\n", state->m_chr_chunks, 0x2000 * state->m_chr_chunks);
			logerror("**\n");
			// mame_printf_error("CHR chunks: %02x, size: %06x\n", state->m_chr_chunks, 0x2000 * state->m_chr_chunks);
			// mame_printf_error("**\n");
		}
		else if ((magic[0] == 'U') && (magic[1] == 'N') && (magic[2] == 'I') && (magic[3] == 'F')) /* If header starts with 'UNIF' it is UNIF */
		{
			UINT32 unif_ver = 0;
			char magic2[4];
			UINT8 buffer[4];
			UINT32 chunk_length = 0, read_length = 0x20;
			UINT32 prg_start = 0, chr_start = 0, prg_size;
			char unif_mapr[32];	// here we should store MAPR chunks
			UINT32 size = image.length();
			int mapr_chunk_found = 0;
			// allocate space to temporarily store PRG & CHR banks
			UINT8 *temp_prg = auto_alloc_array(image.device().machine(), UINT8, 256 * 0x4000);
			UINT8 *temp_chr = auto_alloc_array(image.device().machine(), UINT8, 256 * 0x2000);
			UINT8 temp_byte = 0;

			/* init prg/chr chunks to 0: the exact number of chunks will be determined while reading the file */
			state->m_prg_chunks = 0;
			state->m_chr_chunks = 0;

			image.fread(&buffer, 4);
			unif_ver = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
			logerror("UNIF file found, version %d\n", unif_ver);

			if (size <= 0x20)
			{
				logerror("%s only contains the UNIF header and no data.\n", image.filename());
				return IMAGE_INIT_FAIL;
			}

			do
			{
				image.fseek(read_length, SEEK_SET);

				memset(magic2, '\0', sizeof(magic2));
				image.fread(&magic2, 4);

				/* We first run through the whole image to find a [MAPR] chunk. This is needed
                 because, unfortunately, the MAPR chunk is not always the first chunk (see
                 Super 24-in-1). When such a chunk is found, we set mapr_chunk_found=1 and
                 we go back to load other chunks! */
				if (!mapr_chunk_found)
				{
					if ((magic2[0] == 'M') && (magic2[1] == 'A') && (magic2[2] == 'P') && (magic2[3] == 'R'))
					{
						mapr_chunk_found = 1;
						logerror("[MAPR] chunk found: ");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						if (chunk_length <= 0x20)
							image.fread(&unif_mapr, chunk_length);

						// find out prg/chr size, battery, wram, etc.
						unif_mapr_setup(image.device().machine(), unif_mapr);

						/* now that we found the MAPR chunk, we can go back to load other chunks */
						image.fseek(0x20, SEEK_SET);
						read_length = 0x20;
					}
					else
					{
						logerror("Skip this chunk. We need a [MAPR] chunk before anything else.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
				}
				else
				{
					/* What kind of chunk do we have here? */
					if ((magic2[0] == 'M') && (magic2[1] == 'A') && (magic2[2] == 'P') && (magic2[3] == 'R'))
					{
						/* The [MAPR] chunk has already been read, so we skip it */
						/* TO DO: it would be nice to check if more than one MAPR chunk is present */
						logerror("[MAPR] chunk found (in the 2nd run). Already loaded.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'R') && (magic2[1] == 'E') && (magic2[2] == 'A') && (magic2[3] == 'D'))
					{
						logerror("[READ] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'N') && (magic2[1] == 'A') && (magic2[2] == 'M') && (magic2[3] == 'E'))
					{
						logerror("[NAME] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'W') && (magic2[1] == 'R') && (magic2[2] == 'T') && (magic2[3] == 'R'))
					{
						logerror("[WRTR] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'T') && (magic2[1] == 'V') && (magic2[2] == 'C') && (magic2[3] == 'I'))
					{
						logerror("[TVCI] chunk found.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						image.fread(&temp_byte, 1);
						logerror("Television Standard : %s\n", (temp_byte == 0) ? "NTSC" : (temp_byte == 1) ? "PAL" : "Does not matter");

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'T') && (magic2[1] == 'V') && (magic2[2] == 'S') && (magic2[3] == 'C')) // is this the same as TVCI??
					{
						logerror("[TVSC] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'D') && (magic2[1] == 'I') && (magic2[2] == 'N') && (magic2[3] == 'F'))
					{
						logerror("[DINF] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'C') && (magic2[1] == 'T') && (magic2[2] == 'R') && (magic2[3] == 'L'))
					{
						logerror("[CTRL] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'B') && (magic2[1] == 'A') && (magic2[2] == 'T') && (magic2[3] == 'R'))
					{
						logerror("[BATR] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'V') && (magic2[1] == 'R') && (magic2[2] == 'O') && (magic2[3] == 'R'))
					{
						logerror("[VROR] chunk found. No support yet.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'M') && (magic2[1] == 'I') && (magic2[2] == 'R') && (magic2[3] == 'R'))
					{
						logerror("[MIRR] chunk found.\n");
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						image.fread(&temp_byte, 1);
						switch (temp_byte)
						{
							case 0:	// Horizontal Mirroring (Hard Wired)
								state->m_hard_mirroring = PPU_MIRROR_HORZ;
								break;
							case 1:	// Vertical Mirroring (Hard Wired)
								state->m_hard_mirroring = PPU_MIRROR_VERT;
								break;
							case 2:	// Mirror All Pages From $2000 (Hard Wired)
								state->m_hard_mirroring = PPU_MIRROR_LOW;
								break;
							case 3:	// Mirror All Pages From $2400 (Hard Wired)
								state->m_hard_mirroring = PPU_MIRROR_HIGH;
								break;
							case 4:	// Four Screens of VRAM (Hard Wired)
								state->m_four_screen_vram = 1;
								break;
							case 5:	// Mirroring Controlled By Mapper Hardware
								logerror("Mirroring handled by the board hardware.\n");
								// default to horizontal at start
								state->m_hard_mirroring = PPU_MIRROR_HORZ;
								break;
							default:
								logerror("Undocumented mirroring value.\n");
								// default to horizontal
								state->m_hard_mirroring = PPU_MIRROR_HORZ;
								break;
						}

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'P') && (magic2[1] == 'C') && (magic2[2] == 'K'))
					{
						logerror("[PCK%c] chunk found. No support yet.\n", magic2[3]);
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'C') && (magic2[1] == 'C') && (magic2[2] == 'K'))
					{
						logerror("[CCK%c] chunk found. No support yet.\n", magic2[3]);
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'P') && (magic2[1] == 'R') && (magic2[2] == 'G'))
					{
						logerror("[PRG%c] chunk found. ", magic2[3]);
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						// FIXME: we currently don't support PRG chunks smaller than 16K!
						state->m_prg_chunks += (chunk_length / 0x4000);

						if (chunk_length / 0x4000)
							logerror("It consists of %d 16K-blocks.\n", chunk_length / 0x4000);
						else
							logerror("This chunk is smaller than 16K: the emulation might have issues. Please report this file to the MESS forums.\n");

						/* Read in the program chunks */
						image.fread(&temp_prg[prg_start], chunk_length);

						prg_start += chunk_length;
						read_length += (chunk_length + 8);
					}
					else if ((magic2[0] == 'C') && (magic2[1] == 'H') && (magic2[2] == 'R'))
					{
						logerror("[CHR%c] chunk found. ", magic2[3]);
						image.fread(&buffer, 4);
						chunk_length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

						state->m_chr_chunks += (chunk_length / 0x2000);

						logerror("It consists of %d 8K-blocks.\n", chunk_length / 0x2000);

						/* Read in the vrom chunks */
						image.fread(&temp_chr[chr_start], chunk_length);

						chr_start += chunk_length;
						read_length += (chunk_length + 8);
					}
					else
					{
						logerror("Unsupported UNIF chunk or corrupted header. Please report the problem at MESS Board.\n");
						read_length = size;
					}
				}
			} while (size > read_length);

			if (!mapr_chunk_found)
			{
				auto_free(image.device().machine(), temp_prg);
				auto_free(image.device().machine(), temp_chr);
				fatalerror("UNIF should have a [MAPR] chunk to work. Check if your image has been corrupted\n");
			}

			if (!prg_start)
			{
				auto_free(image.device().machine(), temp_prg);
				auto_free(image.device().machine(), temp_chr);
				fatalerror("Unsupported UNIF chunk or corrupted header. Please report the problem at MESS Board.\n");
			}

			// Allocate class pointers for PRG/VROM/VRAM/WRAM and copy data there from the temp copies

			/* Take care of PRG */
			prg_size = (state->m_prg_chunks == 1) ? 2 * 0x4000 : state->m_prg_chunks * 0x4000;
			state->m_prg = auto_alloc_array(image.device().machine(), UINT8, prg_size);

			memcpy(&state->m_prg[0], &temp_prg[0], state->m_prg_chunks * 0x4000);
			/* If only a single 16K PRG chunk is present, mirror it! */
			if (state->m_prg_chunks == 1)
				memcpy(&state->m_prg[0x4000], &state->m_prg[0], 0x4000);

			/* Take care of CHR ROM */
			if (state->m_chr_chunks)
			{
				state->m_vrom = auto_alloc_array(image.device().machine(), UINT8, state->m_chr_chunks * 0x2000);
				memcpy(&state->m_vrom[0x00000], &temp_chr[0x00000], state->m_chr_chunks * 0x2000);
			}

			/* Take care of CHR RAM */
			if (state->m_vram_chunks)
				state->m_vram = auto_alloc_array(image.device().machine(), UINT8, state->m_vram_chunks * 0x2000);

			// FIXME: this should only be allocated if there is actual wram in the cart (i.e. if state->m_prg_ram = 1)!
			state->m_wram_size = 0x10000;
			state->m_wram = auto_alloc_array(image.device().machine(), UINT8, state->m_wram_size);

#if SPLIT_PRG
			{
				FILE *prgout;
				char outname[255];

				sprintf(outname, "%s.prg", image.filename());
				prgout = fopen(outname, "wb");
				if (prgout)
				{
					fwrite(&state->m_prg[0], 1, 0x4000 * state->m_prg_chunks, prgout);
					mame_printf_error("Created PRG chunk\n");
				}

				fclose(prgout);
			}
#endif

#if SPLIT_CHR
			if (state->m_chr_chunks > 0)
			{
				FILE *chrout;
				char outname[255];

				sprintf(outname, "%s.chr", image.filename());
				chrout= fopen(outname, "wb");
				if (chrout)
				{
					fwrite(state->m_vrom, 1, 0x2000 * state->m_chr_chunks, chrout);
					mame_printf_error("Created CHR chunk\n");
				}
				fclose(chrout);
			}
#endif
			// free the temporary copy of PRG/CHR
			auto_free(image.device().machine(), temp_prg);
			auto_free(image.device().machine(), temp_chr);
			logerror("UNIF support is only very preliminary.\n");
		}
		else
		{
			logerror("%s is NOT a file in either iNES or UNIF format.\n", image.filename());
			return IMAGE_INIT_FAIL;
		}
	}
	else
	{
		UINT32 prg_size = image.get_software_region_length("prg");
		UINT32 chr_size = image.get_software_region_length("chr");
		UINT32 vram_size = image.get_software_region_length("vram");
		vram_size += image.get_software_region_length("vram2");

		// validate the xml fields
		if (!prg_size)
			fatalerror("No PRG entry for this software! Please check if the xml list got corrupted\n");
		if (prg_size < 0x8000)
			fatalerror("PRG entry is too small! Please check if the xml list got corrupted\n");

		// Allocate class pointers for PRG/VROM/VRAM/WRAM and copy data there from the temp copies
		state->m_prg = auto_alloc_array(image.device().machine(), UINT8, prg_size);

		if (chr_size)
			state->m_vrom = auto_alloc_array(image.device().machine(), UINT8, chr_size);

		if (vram_size)
			state->m_vram = auto_alloc_array(image.device().machine(), UINT8, vram_size);

		memcpy(state->m_prg, image.get_software_region("prg"), prg_size);

		if (chr_size)
			memcpy(state->m_vrom, image.get_software_region("chr"), chr_size);

		state->m_prg_chunks = prg_size / 0x4000;
		state->m_chr_chunks = chr_size / 0x2000;
		state->m_vram_chunks = vram_size / 0x2000;

		state->m_pcb_id = nes_get_pcb_id(image.device().machine(), image.get_feature("pcb"));

		if (state->m_pcb_id == STD_TVROM || state->m_pcb_id == STD_DRROM || state->m_pcb_id == IREM_LROG017)
			state->m_four_screen_vram = 1;
		else
			state->m_four_screen_vram = 0;

		state->m_battery = (image.get_software_region("bwram") != NULL) ? 1 : 0;
		state->m_battery_size = image.get_software_region_length("bwram");

		if (state->m_pcb_id == BANDAI_LZ93EX)
		{
			// allocate the 24C01 or 24C02 EEPROM
			state->m_battery = 1;
			state->m_battery_size += 0x2000;
		}

		if (state->m_pcb_id == BANDAI_DATACH)
		{
			// allocate the 24C01 and 24C02 EEPROM
			state->m_battery = 1;
			state->m_battery_size += 0x4000;
		}

		state->m_prg_ram = (image.get_software_region("wram") != NULL) ? 1 : 0;
		state->m_wram_size = image.get_software_region_length("wram");
		state->m_mapper_ram_size = image.get_software_region_length("mapper_ram");
		state->m_mapper_bram_size = image.get_software_region_length("mapper_bram");

		if (state->m_prg_ram)
			state->m_wram = auto_alloc_array(image.device().machine(), UINT8, state->m_wram_size);
		if (state->m_mapper_ram_size)
			state->m_mapper_ram = auto_alloc_array(image.device().machine(), UINT8, state->m_mapper_ram_size);
		if (state->m_mapper_bram_size)
			state->m_mapper_bram = auto_alloc_array(image.device().machine(), UINT8, state->m_mapper_bram_size);

		/* Check for mirroring */
		if (image.get_feature("mirroring") != NULL)
		{
			const char *mirroring = image.get_feature("mirroring");
			if (!strcmp(mirroring, "horizontal"))
				state->m_hard_mirroring = PPU_MIRROR_HORZ;
			if (!strcmp(mirroring, "vertical"))
				state->m_hard_mirroring = PPU_MIRROR_VERT;
			if (!strcmp(mirroring, "high"))
				state->m_hard_mirroring = PPU_MIRROR_HIGH;
			if (!strcmp(mirroring, "low"))
				state->m_hard_mirroring = PPU_MIRROR_LOW;
		}

		state->m_chr_open_bus = 0;
		state->m_ce_mask = 0;
		state->m_ce_state = 0;
		state->m_vrc_ls_prg_a = 0;
		state->m_vrc_ls_prg_b = 0;
		state->m_vrc_ls_chr = 0;

		/* Check for pins in specific boards which require them */
		if (state->m_pcb_id == STD_CNROM)
		{
			if (image.get_feature("chr-pin26") != NULL)
			{
				state->m_ce_mask |= 0x01;
				state->m_ce_state |= !strcmp(image.get_feature("chr-pin26"), "CE") ? 0x01 : 0;
			}
			if (image.get_feature("chr-pin27") != NULL)
			{
				state->m_ce_mask |= 0x02;
				state->m_ce_state |= !strcmp(image.get_feature("chr-pin27"), "CE") ? 0x02 : 0;
			}
		}

		if (state->m_pcb_id == TAITO_X1_005 && image.get_feature("x1-pin17") != NULL && image.get_feature("x1-pin31") != NULL)
		{
			if (!strcmp(image.get_feature("x1-pin17"), "CIRAM A10") && !strcmp(image.get_feature("x1-pin31"), "NC"))
				state->m_pcb_id = TAITO_X1_005_A;
		}

		if (state->m_pcb_id == KONAMI_VRC2)
		{
			state->m_vrc_ls_prg_a = nes_cart_get_line(image.get_feature("vrc2-pin3"));
			state->m_vrc_ls_prg_b = nes_cart_get_line(image.get_feature("vrc2-pin4"));
			state->m_vrc_ls_chr = (nes_cart_get_line(image.get_feature("vrc2-pin21")) != 10) ? 1 : 0;
//          mame_printf_error("VRC-2, pin3: A%d, pin4: A%d, pin21: %s\n", state->m_vrc_ls_prg_a, state->m_vrc_ls_prg_b, state->m_vrc_ls_chr ? "NC" : "A10");
		}

		if (state->m_pcb_id == KONAMI_VRC4)
		{
			state->m_vrc_ls_prg_a = nes_cart_get_line(image.get_feature("vrc4-pin3"));
			state->m_vrc_ls_prg_b = nes_cart_get_line(image.get_feature("vrc4-pin4"));
//          mame_printf_error("VRC-4, pin3: A%d, pin4: A%d\n", state->m_vrc_ls_prg_a, state->m_vrc_ls_prg_b);
		}

		if (state->m_pcb_id == KONAMI_VRC6)
		{
			state->m_vrc_ls_prg_a = nes_cart_get_line(image.get_feature("vrc6-pin9"));
			state->m_vrc_ls_prg_b = nes_cart_get_line(image.get_feature("vrc6-pin10"));
//          mame_printf_error("VRC-6, pin9: A%d, pin10: A%d\n", state->m_vrc_ls_prg_a, state->m_vrc_ls_prg_b);
		}

		/* Check for other misc board variants */
		if (state->m_pcb_id == STD_SOROM)
		{
			if (image.get_feature("mmc1_type") != NULL && !strcmp(image.get_feature("mmc1_type"), "MMC1A"))
				state->m_pcb_id = STD_SOROM_A;	// in MMC1-A PRG RAM is always enabled
		}

		if (state->m_pcb_id == STD_SXROM)
		{
			if (image.get_feature("mmc1_type") != NULL && !strcmp(image.get_feature("mmc1_type"), "MMC1A"))
				state->m_pcb_id = STD_SXROM_A;	// in MMC1-A PRG RAM is always enabled
		}

		if (state->m_pcb_id == STD_NXROM || state->m_pcb_id == SUNSOFT_DCS)
		{
			if (image.get_software_region("minicart") != NULL)	// check for dual minicart
			{
				state->m_pcb_id = SUNSOFT_DCS;
				// we shall load somewhere the minicart, but we still do not support this
			}
		}

#if 0
		if (state->m_pcb_id == UNSUPPORTED_BOARD)
			mame_printf_error("This board (%s) is currently not supported by MESS\n", image.get_feature("pcb"));
		mame_printf_error("PCB Feature: %s\n", image.get_feature("pcb"));
		mame_printf_error("PRG chunks: %d\n", state->m_prg_chunks);
		mame_printf_error("CHR chunks: %d\n", state->m_chr_chunks);
		mame_printf_error("VRAM: Present %s, size: %d\n", state->m_vram_chunks ? "Yes" : "No", vram_size);
		mame_printf_error("NVWRAM: Present %s, size: %d\n", state->m_battery ? "Yes" : "No", state->m_battery_size);
		mame_printf_error("WRAM:   Present %s, size: %d\n", state->m_prg_ram ? "Yes" : "No", state->m_wram_size);
#endif
	}

	// Attempt to load a battery file for this ROM
	// A few boards have internal RAM with a battery (MMC6, Taito X1-005 & X1-017, etc.)
	if (state->m_battery || state->m_mapper_bram_size)
	{
		UINT8 *temp_nvram = auto_alloc_array(image.device().machine(), UINT8, state->m_battery_size + state->m_mapper_bram_size);
		image.battery_load(temp_nvram, state->m_battery_size + state->m_mapper_bram_size, 0x00);
		if (state->m_battery)
		{
			state->m_battery_ram = auto_alloc_array(image.device().machine(), UINT8, state->m_battery_size);
			memcpy(state->m_battery_ram, temp_nvram, state->m_battery_size);
		}
		if (state->m_mapper_bram_size)
			memcpy(state->m_mapper_bram, temp_nvram + state->m_battery_size, state->m_mapper_bram_size);

		auto_free(image.device().machine(), temp_nvram);
	}

	return IMAGE_INIT_PASS;
}



void nes_partialhash(hash_collection &dest, const unsigned char *data,
	unsigned long length, const char *functions)
{
	if (length <= 16)
		return;
	dest.compute(&data[16], length - 16, functions);
}

/**************************

 Disk System emulation

**************************/

static void fds_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (state->m_IRQ_enable_latch)
		state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);

	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count <= 114)
		{
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_enable = 0;
			state->m_fds_status0 |= 0x01;
		}
		else
			state->m_IRQ_count -= 114;
	}
}

READ8_MEMBER(nes_state::nes_fds_r)
{
	UINT8 ret = 0x00;

	switch (offset)
	{
		case 0x00: /* $4030 - disk status 0 */
			ret = m_fds_status0;
			/* clear the disk IRQ detect flag */
			m_fds_status0 &= ~0x01;
			break;
		case 0x01: /* $4031 - data latch */
			/* don't read data if disk is unloaded */
			if (m_fds_data == NULL)
				ret = 0;
			else if (m_fds_current_side)
			{
				// a bunch of games (e.g. bshashsc and fairypin) keep reading beyond the last track
				// what is the expected behavior?
				if (m_fds_head_position > 65500)
					m_fds_head_position = 0;
				ret = m_fds_data[(m_fds_current_side - 1) * 65500 + m_fds_head_position++];
			}
			else
				ret = 0;
			break;
		case 0x02: /* $4032 - disk status 1 */
			/* return "no disk" status if disk is unloaded */
			if (m_fds_data == NULL)
				ret = 1;
			else if (m_fds_last_side != m_fds_current_side)
			{
				/* If we've switched disks, report "no disk" for a few reads */
				ret = 1;
				m_fds_count++;
				if (m_fds_count == 50)
				{
					m_fds_last_side = m_fds_current_side;
					m_fds_count = 0;
				}
			}
			else
				ret = (m_fds_current_side == 0); /* 0 if a disk is inserted */
			break;
		case 0x03: /* $4033 */
			ret = 0x80;
			break;
		default:
			ret = 0x00;
			break;
	}

	return ret;
}

WRITE8_MEMBER(nes_state::nes_fds_w)
{

	switch (offset)
	{
		case 0x00:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0xff00) | data;
			break;
		case 0x01:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0x00ff) | (data << 8);
			break;
		case 0x02:
			m_IRQ_count = m_IRQ_count_latch;
			m_IRQ_enable = data;
			break;
		case 0x03:
			// d0 = sound io (1 = enable)
			// d1 = disk io (1 = enable)
			break;
		case 0x04:
			/* write data out to disk */
			break;
		case 0x05:
			m_fds_motor_on = BIT(data, 0);

			if (BIT(data, 1))
				m_fds_head_position = 0;

			m_fds_read_mode = BIT(data, 2);
			set_nt_mirroring(machine(), BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

			if ((!(data & 0x40)) && (m_fds_write_reg & 0x40))
				m_fds_head_position -= 2; // ???

			m_IRQ_enable_latch = BIT(data, 7);
			m_fds_write_reg = data;
			break;
	}
}

static void nes_load_proc( device_image_interface &image )
{
	nes_state *state = image.device().machine().driver_data<nes_state>();
	int header = 0;
	state->m_fds_sides = 0;

	if (image.length() % 65500)
		header = 0x10;

	state->m_fds_sides = (image.length() - header) / 65500;

	if (state->m_fds_data == NULL)
		state->m_fds_data = auto_alloc_array(image.device().machine(),UINT8,state->m_fds_sides * 65500);	// I don't think we can arrive here ever, probably it can be removed...

	/* if there is an header, skip it */
	image.fseek(header, SEEK_SET);

	image.fread(state->m_fds_data, 65500 * state->m_fds_sides);
	return;
}

static void nes_unload_proc( device_image_interface &image )
{
	nes_state *state = image.device().machine().driver_data<nes_state>();

	/* TODO: should write out changes here as well */
	state->m_fds_sides =  0;
}

DRIVER_INIT_MEMBER(nes_state,famicom)
{

	/* clear some of the variables we don't use */
	m_trainer = 0;
	m_battery = 0;
	m_prg_ram = 0;
	m_four_screen_vram = 0;
	m_hard_mirroring = 0;
	m_prg_chunks = m_chr_chunks = 0;

	/* initialize the disk system */
	m_disk_expansion = 1;
	m_pcb_id = NO_BOARD;

	m_fds_sides = 0;
	m_fds_last_side = 0;
	m_fds_count = 0;
	m_fds_motor_on = 0;
	m_fds_door_closed = 0;
	m_fds_current_side = 1;
	m_fds_head_position = 0;
	m_fds_status0 = 0;
	m_fds_read_mode = m_fds_write_reg = 0;

	m_fds_data = auto_alloc_array_clear(machine(), UINT8, 65500 * 2);
	m_fds_ram = auto_alloc_array_clear(machine(), UINT8, 0x8000);
	save_pointer(NAME(m_fds_ram), 0x8000);

	floppy_install_load_proc(floppy_get_device(machine(), 0), nes_load_proc);
	floppy_install_unload_proc(floppy_get_device(machine(), 0), nes_unload_proc);
}
