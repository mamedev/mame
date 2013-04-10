/*****************************************************************************

    nes.c

    Nintendo Entertainment System (Famicom)

 ****************************************************************************/

#include "emu.h"
#include "crsshair.h"
#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"
#include "includes/nes.h"
//#include "includes/nes_mmc.h"
#include "imagedev/cartslot.h"
#include "imagedev/flopdrv.h"
#include "hashfile.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Set to dump info about the inputs to the errorlog */
#define LOG_JOY     0

/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG   0
#define SPLIT_CHR   0

/***************************************************************************
    FUNCTIONS
***************************************************************************/

void nes_state::init_nes_core()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	static const char *const bank_names[] = { "bank1", "bank2", "bank3", "bank4" };
	int i;
	m_prg_chunks = 0;
	m_chr_chunks = 0;
	m_vram_chunks = 0;
	m_pcb_id = NO_BOARD;

	m_rom = machine().root_device().memregion("maincpu")->base();
	m_ciram = machine().root_device().memregion("ciram")->base();
	// other pointers got set in the loading routine

	/* Brutal hack put in as a consequence of the new memory system; we really need to fix the NES code */
	space.install_readwrite_bank(0x0000, 0x07ff, 0, 0x1800, "bank10");

	machine().device("ppu")->memory().space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(FUNC(nes_state::nes_chr_r),this), write8_delegate(FUNC(nes_state::nes_chr_w),this));
	machine().device("ppu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(nes_state::nes_nt_r),this), write8_delegate(FUNC(nes_state::nes_nt_w),this));

	membank("bank10")->set_base(m_rom);

	// get cart info from cartslot
	if (m_cartslot && m_cartslot->m_cart)
	{
		m_prg_chunks = m_cartslot->m_cart->get_prg_size() / 0x4000;
		m_chr_chunks = m_cartslot->m_cart->get_vrom_size() / 0x2000;
		m_vram_chunks = m_cartslot->m_cart->get_vram_size() / 0x2000;
//      printf("%d - %d - %d\n", m_prg_chunks, m_chr_chunks, m_vram_chunks);
		m_prg = m_cartslot->m_cart->get_prg_base();
		if (m_cartslot->m_cart->get_vrom_size())
			m_vrom = m_cartslot->m_cart->get_vrom_base();
		if (m_cartslot->m_cart->get_vram_size())
			m_vram = auto_alloc_array(machine(), UINT8, m_cartslot->m_cart->get_vram_size());

		m_pcb_id = m_cartslot->get_pcb_id();
		m_hard_mirroring = m_cartslot->m_cart->get_mirroring();
		m_four_screen_vram = m_cartslot->m_cart->get_four_screen_vram();

		// setup memory pointers and related variables
		m_prg_ram = m_cartslot->m_cart->get_battery_size() ? 1 : 0;
		m_wram_size = m_cartslot->m_cart->get_prgram_size();
		m_mapper_ram_size = m_cartslot->m_cart->get_mapper_ram_size();
		m_mapper_bram_size = m_cartslot->m_cart->get_mapper_bram_size();
		m_battery = m_cartslot->m_cart->get_battery_size() ? 1 : 0;
		m_battery_size = m_cartslot->m_cart->get_battery_size();

		if (m_prg_ram)
			m_wram = m_cartslot->m_cart->get_prgram_base();
		if (m_mapper_ram_size)
			m_mapper_ram = m_cartslot->m_cart->get_mapper_ram_base();
		if (m_battery)
			m_battery_ram = m_cartslot->m_cart->get_battery_base();
		if (m_mapper_bram_size)
			m_mapper_bram = m_cartslot->m_cart->get_mapper_bram_base();

		// setup the rest of the variables involved
		m_chr_open_bus = m_cartslot->get_chr_open_bus();
		m_ce_mask = m_cartslot->get_ce_mask();
		m_ce_state = m_cartslot->get_ce_state();
		m_vrc_ls_prg_a = m_cartslot->get_vrc_ls_prg_a();
		m_vrc_ls_prg_b = m_cartslot->get_vrc_ls_prg_b();
		m_vrc_ls_chr = m_cartslot->get_vrc_ls_chr();
		m_crc_hack = m_cartslot->get_crc_hack();
	}

	int prg_banks = (m_prg_chunks == 1) ? (2 * 2) : (m_prg_chunks * 2);

	/* If there is Disk Expansion and no cart has been loaded, setup memory accordingly */
	if (m_disk_expansion && m_pcb_id == NO_BOARD)
	{
		/* If we are loading a disk we have already filled m_fds_data and we don't want to overwrite it,
		 if we are loading a cart image identified as mapper 20 (probably wrong mapper...) we need to alloc
		 memory for use in nes_fds_r/nes_fds_w. Same goes for allocation of fds_ram (used for bank2)  */
		if (m_fds_data == NULL)
		{
			UINT32 size = (m_prg_chunks == 1) ? 2 * 0x4000 : m_prg_chunks * 0x4000;
			m_fds_data = auto_alloc_array_clear(machine(), UINT8, size);
			memcpy(m_fds_data, m_prg, size);    // copy in fds_data the cart PRG
		}
		if (m_fds_ram == NULL)
			m_fds_ram = auto_alloc_array(machine(), UINT8, 0x8000);

		space.install_read_handler(0x4030, 0x403f, read8_delegate(FUNC(nes_state::nes_fds_r),this));
		space.install_read_bank(0x6000, 0xdfff, "bank2");
		space.install_read_bank(0xe000, 0xffff, "bank1");

		space.install_write_handler(0x4020, 0x402f, write8_delegate(FUNC(nes_state::nes_fds_w),this));
		space.install_write_bank(0x6000, 0xdfff, "bank2");

		membank("bank1")->set_base(&m_rom[0xe000]);
		membank("bank2")->set_base(m_fds_ram);
		return;
	}
	else
	{
		/* Set up the mapper callbacks */
		pcb_handlers_setup();

		/* Set up the memory handlers for the mapper */
		space.install_read_bank(0x8000, 0x9fff, "bank1");
		space.install_read_bank(0xa000, 0xbfff, "bank2");
		space.install_read_bank(0xc000, 0xdfff, "bank3");
		space.install_read_bank(0xe000, 0xffff, "bank4");
		space.install_readwrite_bank(0x6000, 0x7fff, "bank5");

		/* configure banks 1-4 */
		for (i = 0; i < 4; i++)
		{
			membank(bank_names[i])->configure_entries(0, prg_banks, m_prg, 0x2000);
			// some mappers (e.g. MMC5) can map PRG RAM in  0x8000-0xffff as well
			if (m_prg_ram)
				membank(bank_names[i])->configure_entries(prg_banks, m_wram_size / 0x2000, m_wram, 0x2000);
			// however, at start we point to PRG ROM
			membank(bank_names[i])->set_entry(i);
			m_prg_bank[i] = i;
		}

		/* bank 5 configuration is more delicate, since it can have PRG RAM, PRG ROM or SRAM mapped to it */
		/* we first map PRG ROM banks, then the battery bank (if a battery is present), and finally PRG RAM (m_wram) */
		membank("bank5")->configure_entries(0, prg_banks, m_prg, 0x2000);
		m_battery_bank5_start = prg_banks;
		m_prgram_bank5_start = prg_banks;
		m_empty_bank5_start = prg_banks;

		/* add battery ram, but only if there's no trainer since they share overlapping memory. */
		if (m_battery && !m_trainer)
		{
			UINT32 bank_size = (m_battery_size > 0x2000) ? 0x2000 : m_battery_size;
			int bank_num = (m_battery_size > 0x2000) ? m_battery_size / 0x2000 : 1;
			membank("bank5")->configure_entries(prg_banks, bank_num, m_battery_ram, bank_size);
			m_prgram_bank5_start += bank_num;
			m_empty_bank5_start += bank_num;
		}
		/* add prg ram. */
		if (m_prg_ram)
		{
			membank("bank5")->configure_entries(m_prgram_bank5_start, m_wram_size / 0x2000, m_wram, 0x2000);
			m_empty_bank5_start += m_wram_size / 0x2000;
		}

		membank("bank5")->configure_entry(m_empty_bank5_start, m_rom + 0x6000);

		/* if we have any additional PRG RAM, point bank5 to its first bank */
		if (m_battery || m_prg_ram)
			m_prg_bank[4] = m_battery_bank5_start;
		else
			m_prg_bank[4] = m_empty_bank5_start; // or shall we point to "maincpu" region at 0x6000? point is that we should never access this region if no sram or wram is present!

		membank("bank5")->set_entry(m_prg_bank[4]);

		if (m_four_screen_vram)
		{
			m_extended_ntram = auto_alloc_array_clear(machine(), UINT8, 0x2000);
			save_pointer(NAME(m_extended_ntram), 0x2000);
		}

		if (m_four_screen_vram)
			set_nt_mirroring(PPU_MIRROR_4SCREEN);
		else
		{
			switch (m_hard_mirroring)
			{
				case PPU_MIRROR_HORZ:
				case PPU_MIRROR_VERT:
				case PPU_MIRROR_HIGH:
				case PPU_MIRROR_LOW:
					set_nt_mirroring(m_hard_mirroring);
					break;
				default:
					set_nt_mirroring(PPU_MIRROR_NONE);
					break;
			}
		}
	}

	// there are still some quirk about writes to bank5... I hope to fix them soon. (mappers 34,45,52,246 have both mid_w and WRAM-->check)
	if (!m_mmc_write_low.isnull())
		space.install_write_handler(0x4100, 0x5fff, m_mmc_write_low);
	if (!m_mmc_write_mid.isnull())
		space.install_write_handler(0x6000, 0x7fff, m_mmc_write_mid);
	if (!m_mmc_write.isnull())
		space.install_write_handler(0x8000, 0xffff, m_mmc_write);

	// In fact, we also allow single pcbs to overwrite the bank read handlers defined above,
	// because some pcbs (mainly pirate ones) require protection values to be read instead of
	// the expected ROM banks: these handlers, though, must take care of the ROM access as well
	if (!m_mmc_read_low.isnull())
		space.install_read_handler(0x4100, 0x5fff, m_mmc_read_low);
	if (!m_mmc_read_mid.isnull())
		space.install_read_handler(0x6000, 0x7fff, m_mmc_read_mid);
	if (!m_mmc_read.isnull())
		space.install_read_handler(0x8000, 0xffff, m_mmc_read);

	// install additional handlers
	if (m_pcb_id == BTL_SMB2B || m_mapper == 50)
	{
		space.install_write_handler(0x4020, 0x403f, write8_delegate(FUNC(nes_state::smb2jb_extra_w),this));
		space.install_write_handler(0x40a0, 0x40bf, write8_delegate(FUNC(nes_state::smb2jb_extra_w),this));
	}

	if (m_pcb_id == KAISER_KS7017)
	{
		space.install_read_handler(0x4030, 0x4030, read8_delegate(FUNC(nes_state::ks7017_extra_r),this));
		space.install_write_handler(0x4020, 0x40ff, write8_delegate(FUNC(nes_state::ks7017_extra_w),this));
	}

	if (m_pcb_id == UNL_603_5052)
	{
		space.install_read_handler(0x4020, 0x40ff, read8_delegate(FUNC(nes_state::unl_6035052_extra_r),this));
		space.install_write_handler(0x4020, 0x40ff, write8_delegate(FUNC(nes_state::unl_6035052_extra_w),this));
	}

	if (m_pcb_id == WAIXING_SH2)
		machine().device("ppu")->memory().space(AS_PROGRAM).install_read_handler(0, 0x1fff, read8_delegate(FUNC(nes_state::waixing_sh2_chr_r),this));
}

// to be probably removed (it does nothing since a long time)
int nes_state::nes_ppu_vidaccess( int address, int data )
{
	return data;
}

void nes_state::machine_reset()
{
	/* Reset the mapper variables. Will also mark the char-gen ram as dirty */
	if (m_disk_expansion && m_pcb_id == NO_BOARD)
		m_ppu->set_hblank_callback(ppu2c0x_hblank_delegate(FUNC(nes_state::fds_irq),this));
	else
		nes_pcb_reset();

	/* Reset the serial input ports */
	m_in_0.shift = 0;
	m_in_1.shift = 0;

	m_maincpu->reset();
}

TIMER_CALLBACK_MEMBER(nes_state::nes_irq_callback)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	m_irq_timer->adjust(attotime::never);
}

void nes_state::nes_banks_restore()
{
	membank("bank1")->set_entry(m_prg_bank[0]);
	membank("bank2")->set_entry(m_prg_bank[1]);
	membank("bank3")->set_entry(m_prg_bank[2]);
	membank("bank4")->set_entry(m_prg_bank[3]);
	membank("bank5")->set_entry(m_prg_bank[4]);
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

	machine.save().register_postload(save_prepost_delegate(FUNC(nes_state::nes_banks_restore), state));
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void nes_state::machine_start()
{
	m_ppu = machine().device<ppu2c0x_device>("ppu");

	init_nes_core();

	m_maincpu           = machine().device<cpu_device>("maincpu");
	m_sound             = machine().device("nessound");
	m_io_ctrlsel        = ioport("CTRLSEL");
	m_io_fckey[0]       = ioport("FCKEY0");
	m_io_fckey[1]       = ioport("FCKEY1");
	m_io_fckey[2]       = ioport("FCKEY2");
	m_io_fckey[3]       = ioport("FCKEY3");
	m_io_fckey[4]       = ioport("FCKEY4");
	m_io_fckey[5]       = ioport("FCKEY5");
	m_io_fckey[6]       = ioport("FCKEY6");
	m_io_fckey[7]       = ioport("FCKEY7");
	m_io_fckey[8]       = ioport("FCKEY8");
	m_io_subkey[0 ]     = ioport("SUBKEY0");
	m_io_subkey[1 ]     = ioport("SUBKEY1");
	m_io_subkey[2 ]     = ioport("SUBKEY2");
	m_io_subkey[3 ]     = ioport("SUBKEY3");
	m_io_subkey[4 ]     = ioport("SUBKEY4");
	m_io_subkey[5 ]     = ioport("SUBKEY5");
	m_io_subkey[6 ]     = ioport("SUBKEY6");
	m_io_subkey[7 ]     = ioport("SUBKEY7");
	m_io_subkey[8 ]     = ioport("SUBKEY8");
	m_io_subkey[9 ]     = ioport("SUBKEY9");
	m_io_subkey[10]     = ioport("SUBKEY10");
	m_io_subkey[11]     = ioport("SUBKEY11");
	m_io_subkey[12]     = ioport("SUBKEY12");
	m_io_pad[0]         = ioport("PAD1");
	m_io_pad[1]         = ioport("PAD2");
	m_io_pad[2]         = ioport("PAD3");
	m_io_pad[3]         = ioport("PAD4");
	m_io_cc_left        = ioport("CC_LEFT");
	m_io_cc_right       = ioport("CC_RIGHT");
	m_io_zapper1_t      = ioport("ZAPPER1_T");
	m_io_zapper1_x      = ioport("ZAPPER1_X");
	m_io_zapper1_y      = ioport("ZAPPER1_Y");
	m_io_zapper2_t      = ioport("ZAPPER2_T");
	m_io_zapper2_x      = ioport("ZAPPER2_X");
	m_io_zapper2_y      = ioport("ZAPPER2_Y");
	m_io_paddle         = ioport("PADDLE");
	m_io_mahjong[0]     = ioport("MAH0");
	m_io_mahjong[1]     = ioport("MAH1");
	m_io_mahjong[2]     = ioport("MAH2");
	m_io_mahjong[3]     = ioport("MAH3");
	m_prg_bank_mem[0]   = membank("bank1");
	m_prg_bank_mem[1]   = membank("bank2");
	m_prg_bank_mem[2]   = membank("bank3");
	m_prg_bank_mem[3]   = membank("bank4");
	m_prg_bank_mem[4]   = membank("bank5");

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

	m_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(nes_state::nes_irq_callback),this));
	nes_state_register(machine());
}


//-------------------------------------------------
//  update_prg_banks
//-------------------------------------------------

void nes_state::update_prg_banks(int prg_bank_start, int prg_bank_end)
{
	for (int prg_bank = prg_bank_start; prg_bank <= prg_bank_end; prg_bank++)
	{
		assert(prg_bank >= 0);
		assert(prg_bank < ARRAY_LENGTH(m_prg_bank));
		assert(prg_bank < ARRAY_LENGTH(m_prg_bank_mem));

		m_prg_bank_mem[prg_bank]->set_entry(m_prg_bank[prg_bank]);
	}
}



READ8_MEMBER(nes_state::nes_IN0_r)
{
	int cfg = m_io_ctrlsel->read();
	int ret;

	if ((cfg & 0x000f) >= 0x08) // for now we treat the FC keyboard separately from other inputs!
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
			int x = m_in_0.i1;  /* read Zapper x-position */
			int y = m_in_0.i2;  /* read Zapper y-position */
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

READ8_MEMBER(nes_state::nes_IN1_r)
{
	int cfg = m_io_ctrlsel->read();
	int ret;

	// row of the keyboard matrix are read 4-bits at time, and gets returned as bit1->bit4
	if ((cfg & 0x000f) == 0x08) // for now we treat the FC keyboard separately from other inputs!
	{
		if (m_fck_scan < 9)
			ret = ~(((m_io_fckey[m_fck_scan]->read() >> (m_fck_mode * 4)) & 0x0f) << 1) & 0x1e;
		else
			ret = 0x1e;
	}
	else if ((cfg & 0x000f) == 0x09)    // for now we treat the Subor keyboard separately from other inputs!
	{
		if (m_fck_scan < 12)
			ret = ~(((m_io_subkey[m_fck_scan]->read() >> (m_fck_mode * 4)) & 0x0f) << 1) & 0x1e;
		else
			ret = 0x1e;
	}
	else
	{
		/* Some games expect bit 6 to be set because the last entry on the data bus shows up */
		/* in the unused upper 3 bits, so typically a read from $4017 leaves 0x40 there. */
		ret = 0x40;

		/* Handle data line 0's serial output */
		if((cfg & 0x00f0) == 0x0070)
			ret |= (((m_in_1.i0 >> m_in_1.shift) & 0x01) << 1);
		else
			ret |= ((m_in_1.i0 >> m_in_1.shift) & 0x01);

		/* zapper */
		if ((cfg & 0x00f0) == 0x0030)
		{
			int x = m_in_1.i1;  /* read Zapper x-position */
			int y = m_in_1.i2;  /* read Zapper y-position */
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
static void nes_read_input_device( running_machine &machine, int cfg, nes_input *vals, int pad_port, int supports_zapper, int mux_data )
{
	nes_state *state = machine.driver_data<nes_state>();

	vals->i0 = 0;
	vals->i1 = 0;
	vals->i2 = 0;

	switch (cfg & 0x0f)
	{
		case 0x01:  /* gamepad */
			if (pad_port >= 0)
				vals->i0 = state->m_io_pad[pad_port]->read();
			break;

		case 0x02:  /* zapper 1 */
			if (supports_zapper)
			{
				vals->i0 = state->m_io_zapper1_t->read();
				vals->i1 = state->m_io_zapper1_x->read();
				vals->i2 = state->m_io_zapper1_y->read();
			}
			break;

		case 0x03:  /* zapper 2 */
			if (supports_zapper)
			{
				vals->i0 = state->m_io_zapper2_t->read();
				vals->i1 = state->m_io_zapper2_x->read();
				vals->i2 = state->m_io_zapper2_y->read();
			}
			break;

		case 0x04:  /* arkanoid paddle */
			if (pad_port == 1)
				vals->i0 = (UINT8) ((UINT8) state->m_io_paddle->read() + (UINT8)0x52) ^ 0xff;
			break;

		case 0x06:  /* crazy climber controller */
			if (pad_port == 0)
			{
				state->m_in_0.i0 = state->m_io_cc_left->read();
				state->m_in_1.i0 = state->m_io_cc_right->read();
			}
			break;

		case 0x07: /* Mahjong Panel */
			if(mux_data & 0xf8)
				logerror("Error: Mahjong panel read with mux data %02x\n",mux_data);
			else
				vals->i0 = state->m_io_mahjong[mux_data >> 1]->read();
			break;
	}
}


TIMER_CALLBACK_MEMBER(nes_state::lightgun_tick)
{
	if ((m_io_ctrlsel->read() & 0x000f) == 0x0002)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
	}

	if ((m_io_ctrlsel->read() & 0x00f0) == 0x0030)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_NONE);
	}
}

WRITE8_MEMBER(nes_state::nes_IN0_w)
{
	int cfg = m_io_ctrlsel->read();

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(nes_state::lightgun_tick),this));

	if ((cfg & 0x000f) >= 0x08) // for now we treat the FC keyboard separately from other inputs!
	{
		// here we should also have the tape output

		if (BIT(data, 2))   // keyboard active
		{
			int lines = ((cfg & 0x000f) == 0x04) ? 9 : 12;
			UINT8 out = BIT(data, 1);   // scan

			if (m_fck_mode && !out && ++m_fck_scan > lines)
				m_fck_scan = 0;

			m_fck_mode = out;   // access lower or upper 4 bits

			if (BIT(data, 0))   // reset
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
			nes_read_input_device(machine(), cfg >>  0, &m_in_0, 0,  TRUE,data & 0xfe);
			nes_read_input_device(machine(), cfg >>  4, &m_in_1, 1,  TRUE,data & 0xfe);
			nes_read_input_device(machine(), cfg >>  8, &m_in_2, 2, FALSE,data & 0xfe);
			nes_read_input_device(machine(), cfg >> 12, &m_in_3, 3, FALSE,data & 0xfe);
		}
		else // crazy climber pad
		{
			nes_read_input_device(machine(), 0, &m_in_1, 1,  TRUE,data & 0xfe);
			nes_read_input_device(machine(), 0, &m_in_2, 2, FALSE,data & 0xfe);
			nes_read_input_device(machine(), 0, &m_in_3, 3, FALSE,data & 0xfe);
			nes_read_input_device(machine(), cfg >>  0, &m_in_0, 0,  TRUE,data & 0xfe);
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

void nes_state::fds_irq( int scanline, int vblank, int blanked )
{
	if (m_IRQ_enable_latch)
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);

	if (m_IRQ_enable)
	{
		if (m_IRQ_count <= 114)
		{
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			m_IRQ_enable = 0;
			m_fds_status0 |= 0x01;
		}
		else
			m_IRQ_count -= 114;
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
			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

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
		state->m_fds_data = auto_alloc_array(image.device().machine(),UINT8,state->m_fds_sides * 65500);    // I don't think we can arrive here ever, probably it can be removed...

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
