// license:BSD-3-Clause
// copyright-holders:David Haywood, Xing Xing, Andreas Naive
/* PGM 2 hardware.

    Motherboard is bare bones stuff, and does not contain any ROMs.
    The IGS036 used by the games is an ARM based CPU, like IGS027A used on PGM1 it has internal ROM.
    Decryption should be correct in most cases.
    The ARM appears to be ARMv5T, probably an ARM9.

    PGM2 Motherboard Components:

     IS61LV25616AL(SRAM)
     IGS037(GFX PROCESSOR)
     YMZ774-S(SOUND)
     R5F21256SN(extra MCU for ICcard communication)
      - Appears to be referred to by the games as MPU

    Cartridges
     IGS036 (MAIN CPU) (differs per game, internal code)
     ROMs
     Custom program ROM module (KOV3 only)
      - on some games ROM socket contains Flash ROM + SRAM

     QFP100 chip (Xlinx CPLD)

     Single PCB versions of some of the titles were also available

    Only 5 Games were released for this platform, 3 of which are just updates / re-releases of older titles!
    The platform has since been superseded by PGM3, see pgm3.cpp

    Oriental Legend 2
    The King of Fighters '98 - Ultimate Match - Hero
    Knights of Valour 2 New Legend
    Dodonpachi Daioujou Tamashii
    Knights of Valour 3

    These were only released in Japan, seen for sale for around $250-$300

    Jigsaw World Arena
    Ochaken no Puzzle


    ToDo (emulation issues):

    Support remaining games (need IGS036 dumps)
    Identify which regions each game was released in and either dump alt. internal ROMs for each region, or
      create them until that can be done.
    properly implement RTC (integrated into the CPU)
    Verify Sprite Zoom (check exactly which pixels are doubled / missed on hardware for flipped , non-flipped cases etc.)
    Fix Save States (is this a driver problem or an ARM core problem, they don't work unless you get through the startup tests)
    Determine motherboard card reader MCU internal ROM size and add as NO_DUMP to the sets
    See if kov2nl needs another idle skip, after Game Over there is a period where the current one is ineffective

    Debug features (require DIP SW1:8 On and SW1:1 Off):
    - QC TEST mode: hold P1 A+B during boot
    - Debug/Cheat mode: hold P1 B+C during boot
      orleg2 and kov2nl: when ingame pressing P1 Start skips to next location, where might be more unknown debug features.


    Holographic Stickers

    The IGS036 CPUs have holographic stickers on them, there is a number printed on each sticker but it doesn't seem connected to the
    game code / revision contained within, it might just be to mark the date the board was produced as it seems to coincide with the
    design of the hologram.  For reference the ones being used for dumping are

    Dodonpachi Daioujou Tamashi (China) - W10
    King of Fighter 98 UMH (China) - C11
    Knights of Valour 2 (China) - V21
    Knights of Valour 3 (China) - V21
    Oriental Legend 2 (Oversea) - V21
    Oriental Legend 2 (China) - A8

    GPU registers, located at 301200xx, 16bit access.
    00 - bg scroll x
    02 - bg scroll y
    04 - zoom something, 0F-7F, default 1F
    06 - zoom something, 0F-7F, default 1F
    08 - fg scroll x
    0a - fg scroll y
    0e - resolution, 0 - low (kof98umh), 1 - high (rest of games), 2 - higher (kov3)
    10 - ? orleg2 - 0x13, kov2nl, kov3, kof98umh, ddpdojt - 0x14 at init
    14 - sprite enable ? set to 0 before spriteram update, to 1 after
    16 - set to 1 before access to vrams/palettes, reset after. bits: 0 - bg RAM and palette, 1 - fg RAM and palette, 2 - sprite palette.
    18 - vblank ack
    1a - ? 0 at init
    1c - ? orleg2 - 5, kov2nl, kov3, kof98umh, ddpdojt - 7 at init
    1e - ? 2 at init
    32 - shared RAM bank
    34, 36 - ? 0 at init
    38, 3a - sprite mask xor key

*/

#include "emu.h"
#include "pgm2.h"

// checked on startup, or doesn't boot
u32 pgm2_state::unk_startup_r()
{
	logerror("%s: unk_startup_r\n", machine().describe_context().c_str());
	return 0x00000180;
}

u32 pgm2_state::rtc_r()
{
	// write to FFFFFD20 if bit 18 set (0x40000) probably reset this RTC timer
	// TODO: somehow hook here current time/date, which is a bit complicated because value is relative, later to it added "base time" stored in SRAM
	return machine().time().seconds();
}

u8 pgm2_state::encryption_r(offs_t offset)
{
	return m_encryption_table[offset];
}

void pgm2_state::encryption_w(offs_t offset, u8 data)
{
	m_encryption_table[offset] = data;
}

void pgm2_state::sprite_encryption_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_spritekey);

	if (!m_sprite_predecrypted)
		m_realspritekey = bitswap<32>(m_spritekey ^ 0x90055555, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
}

void pgm2_state::device_post_load()
{
	// make sure the encrypted area is in the correct state after we load a savestate because we don't want to have to save the whole rom.

	memcpy(m_mainrom->base(), &m_encrypted_copy[0], m_mainrom->bytes());

	if (m_has_decrypted_kov3_module)
	{
		decrypt_kov3_module(module_key->addr_xor, module_key->data_xor);
	}

	if (m_has_decrypted)
	{
		igs036_decryptor decrypter(m_encryption_table);

		if (m_romboard_ram)
		{
			decrypter.decrypter_rom((u16*)m_mainrom->base(), m_mainrom->bytes(), m_romboard_ram.bytes());
		}
		else
		{
			decrypter.decrypter_rom((u16*)m_mainrom->base(), m_mainrom->bytes(), 0);
		}
	}
}

void pgm2_state::encryption_do_w(u32 data)
{
	igs036_decryptor decrypter(m_encryption_table);
	if (m_romboard_ram)
	{
		decrypter.decrypter_rom((u16*)&m_romboard_ram[0], m_romboard_ram.bytes(), 0);
		decrypter.decrypter_rom((u16*)m_mainrom->base(), m_mainrom->bytes(), m_romboard_ram.bytes());   // assume the rom at 0x0200000 also gets decrypted as if it was at 0x0200000 even if it isn't used (the game has already copied it to RAM where it properly decrypted)
	}
	else
	{
		decrypter.decrypter_rom((u16*)m_mainrom->base(), m_mainrom->bytes(), 0);
	}
	m_has_decrypted = true;
}


void pgm2_state::share_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_share_bank);
}

u8 pgm2_state::shareram_r(offs_t offset)
{
	return m_shareram[offset + (m_share_bank & 1) * 128];
}
void pgm2_state::shareram_w(offs_t offset, u8 data)
{
	m_shareram[offset + (m_share_bank & 1) * 128] = data;
}


TIMER_DEVICE_CALLBACK_MEMBER(pgm2_state::mcu_interrupt)
{
	m_arm_aic->set_irq(3, ASSERT_LINE);
}

// "MPU" MCU HLE starts here
// command delays are far from correct, might not work in other games
// command results probably incorrect (except for explicit checked bytes)
void pgm2_state::mcu_command(bool is_command)
{
	u8 const cmd = m_mcu_regs[0] & 0xff;
	//  if (is_command && cmd != 0xf6)
	//      logerror("MCU command %08x %08x\n", m_mcu_regs[0], m_mcu_regs[1]);

	if (is_command)
	{
		m_mcu_last_cmd = cmd;
		u8 status = 0xf7; // "command accepted" status
		int delay = 1;

		u8 arg1 = m_mcu_regs[0] >> 8;
		u8 arg2 = m_mcu_regs[0] >> 16;
		u8 arg3 = m_mcu_regs[0] >> 24;
		switch (cmd)
		{
		case 0xf6:  // get result
			m_mcu_regs[3] = m_mcu_result0;
			m_mcu_regs[4] = m_mcu_result1;
			m_mcu_last_cmd = 0;
			break;
		case 0xe0: // command port test
			m_mcu_result0 = m_mcu_regs[0];
			m_mcu_result1 = m_mcu_regs[1];
			break;
		case 0xe1: // shared RAM access (unimplemented)
		{
			// MCU access to RAM shared at 0x30100000, 2x banks, in the same time CPU and MCU access different banks
			u8 mode = m_mcu_regs[0] >> 16; // 0 - ???, 1 - read, 2 - write
			u8 data = m_mcu_regs[0] >> 24;
			if (mode == 2)
			{
				// where is offset ? so far assume this command fill whole page
				memset(&m_shareram[(~m_share_bank & 1) * 128], data, 128);
			}
			m_mcu_result0 = cmd;
			m_mcu_result1 = 0;
		}
		break;
		// C0-C9 commands is IC Card RW comms
		case 0xc0: // insert card or/and check card presence. result: F7 - ok, F4 - no card
			if (m_memcard[arg1 & 3]->present() == -1)
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc1: // check ready/busy ?
			if (m_memcard[arg1 & 3]->present() == -1)
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc2: // read data to shared RAM
			for (int i = 0; i < arg3; i++)
			{
				if (m_memcard[arg1 & 3]->present() != -1)
					m_shareram[i + (~m_share_bank & 1) * 128] = m_memcard[arg1 & 3]->read(arg2 + i);
				else
					status = 0xf4;
			}
			m_mcu_result0 = cmd;
			break;
		case 0xc3: // save data from shared RAM
			for (int i = 0; i < arg3; i++)
			{
				if (m_memcard[arg1 & 3]->present() != -1)
					m_memcard[arg1 & 3]->write(arg2 + i, m_shareram[i + (~m_share_bank & 1) * 128]);
				else
					status = 0xf4;
			}
			m_mcu_result0 = cmd;
			break;
		case 0xc4: // presumable read security mem (password only?)
			if (m_memcard[arg1 & 3]->present() != -1)
			{
				m_mcu_result1 = m_memcard[arg1 & 3]->read_sec(1) |
					(m_memcard[arg1 & 3]->read_sec(2) << 8) |
					(m_memcard[arg1 & 3]->read_sec(3) << 16);
			}
			else
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc5: // write security mem
			if (m_memcard[arg1 & 3]->present() != -1)
				m_memcard[arg1 & 3]->write_sec(arg2 & 3, arg3);
			else
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc6: // presumable write protection mem
			if (m_memcard[arg1 & 3]->present() != -1)
				m_memcard[arg1 & 3]->write_prot(arg2 & 3, arg3);
			else
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc7: // read protection mem
			if (m_memcard[arg1 & 3]->present() != -1)
			{
				m_mcu_result1 = m_memcard[arg1 & 3]->read_prot(0) |
					(m_memcard[arg1 & 3]->read_prot(1) << 8) |
					(m_memcard[arg1 & 3]->read_prot(2) << 16) |
					(m_memcard[arg1 & 3]->read_prot(3) << 24);
			}
			else
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc8: // write data mem
			if (m_memcard[arg1 & 3]->present() != -1)
				m_memcard[arg1 & 3]->write(arg2, arg3);
			else
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		case 0xc9: // card authentication
			if (m_memcard[arg1 & 3]->present() != -1)
				m_memcard[arg1 & 3]->auth(arg2, arg3, m_mcu_regs[1] & 0xff);
			else
				status = 0xf4;
			m_mcu_result0 = cmd;
			break;
		default:
			logerror("MCU unknown command %08x %08x\n", m_mcu_regs[0], m_mcu_regs[1]);
			status = 0xf4; // error
			break;
		}
		m_mcu_regs[3] = (m_mcu_regs[3] & 0xff00ffff) | (status << 16);
		m_mcu_timer->adjust(attotime::from_msec(delay));
	}
	else // next step
	{
		if (m_mcu_last_cmd)
		{
			m_mcu_regs[3] = (m_mcu_regs[3] & 0xff00ffff) | 0x00F20000;  // set "command done and return data" status
			m_mcu_timer->adjust(attotime::from_usec(100));
			m_mcu_last_cmd = 0;
		}
	}
}

u32 pgm2_state::mcu_r(offs_t offset)
{
	return m_mcu_regs[(offset >> 15) & 7];
}

void pgm2_state::mcu_w(offs_t offset, u32 data, u32 mem_mask)
{
	int reg = (offset >> 15) & 7;
	COMBINE_DATA(&m_mcu_regs[reg]);

	if (reg == 2 && m_mcu_regs[2]) // irq to mcu
		mcu_command(true);
	if (reg == 5 && m_mcu_regs[5]) // ack to mcu (written at the end of irq handler routine)
	{
		mcu_command(false);
		m_arm_aic->set_irq(3, CLEAR_LINE);
	}
}

void pgm2_state::vbl_ack_w(u16 data)
{
	m_arm_aic->set_irq(12, CLEAR_LINE);
}

void pgm2_state::unk30120014_w(offs_t offset, u16 data)
{
	if (offset == 0)
	{
		// 0/1 toggles (maybe sprite dma triggers?)
	}
	else
	{
		// interesting data
		//logerror("unk30120014_w %d %04x\n", offset, data);
	}
}

/*
 KOV3 ROM board uses special module intead of program ROM, tiny PCB with IC stamped "HW1" (might be FPGA, CPLD or ASIC) and BGA Flash ROM stamped "IG-L".
 This module uses few pins for serial comms (wired to IGS036 GPIO), it can not be dumped as regular ROM until special unlock procedure (return weird data pattern while locked).

 In case of KOV3 unlock sequence is:
  1) send via serial 0x0d and 64bit xor_value, result must be A3A3A3A36D6D6D6D
  2) send via serial 0x25 and 64bit xor_value, result is 64bit key^xor_value
  3) read first 10h bytes from ROM area (at this point ROM area read as scrambled or random data)
  4) write "key" to ROM area, using 2x 16bit writes, offsets and data is bitfields of 64bit key:
      u32 key0, key1;
      u16 *rom = (u16*)0x10000000;
      rom[((key0 & 0x3f) << 16) | (key1 >> 16)] = key1 & 0xffff;
      rom[key0 >> 22] = (key0 >> 6) & 0xffff;
     it is possible, 22bit address xor value derived from 1st write offset.
     meaning of other 10bit offset and 2x data words is not clear - each of them can be either "key bits" or "magic word" expected by security device.
  5) write static sequence of 4x words to ROM area, which switch module to special mode - next 4x reads will return checksum^key parts instead of rom data.
  6) read checksum from ROM area 10000002-10000009
  7) read first 10h bytes from ROM area and check they are not same as was at step 3)
  8) perform whole ROM summing, result must match key^checksum read at step 6)

 It is not clear if/how real address/data xor values derived from written "key",
 or security chip just waiting to be be written magic value at specific address in ROM area, and if this happen enable descrambling using hardcoded values.
 */

int pgm2_state::module_data_r()
{
	return module_out_latch ? ASSERT_LINE : CLEAR_LINE;
}
void pgm2_state::module_data_w(int state)
{
	module_in_latch = (state == ASSERT_LINE) ? 1 : 0;
}
void pgm2_state::module_clk_w(int state)
{
	if (module_prev_state != state && state == CLEAR_LINE)
	{
		if (module_clk_cnt < 80)
		{
			s8 const offs = module_clk_cnt / 8;
			u8 const bit = (module_clk_cnt & 7) ^ 7;
			module_rcv_buf[offs] &= ~(1 << bit);
			module_rcv_buf[offs] |= module_in_latch << bit;

			++module_clk_cnt;
			if (module_clk_cnt >= 80)
			{
				switch (module_rcv_buf[0])
				{
				case 0x0d: // init or status check
					module_send_buf[0] = module_send_buf[1] = module_send_buf[2] = module_send_buf[3] = 0xa3;
					module_send_buf[4] = module_send_buf[5] = module_send_buf[6] = module_send_buf[7] = 0x6d;
					break;
				case 0x25: // get key
					for (int i = 0; i < 8; i++)
						module_send_buf[i] = module_key->key[i ^ 3] ^ module_rcv_buf[i + 1];
					break;
				default:
					logerror("unknown FPGA command %02X!\n", module_rcv_buf[0]);
					break;
				}

				module_send_buf[8] = 0;
				for (int i = 0; i < 8; i++) // sum reply bytes
					module_send_buf[8] += module_send_buf[i];
			}
		}
		else
		{
			s8 const offs = (module_clk_cnt - 80) / 8;
			u8 const bit = (module_clk_cnt & 7) ^ 7;
			module_out_latch = BIT(module_send_buf[offs], bit);
			++module_clk_cnt;
			if (module_clk_cnt >= 152)
				module_clk_cnt = 0;
		}
	}
	module_prev_state = state;
}

u16 pgm2_state::module_rom_r(offs_t offset)
{
	if (module_sum_read && offset > 0 && offset < 5) // checksum read mode
	{
		if (offset == 4)
			module_sum_read = false;
		u32 const offs = ((offset - 1) * 2) ^ 2;
		return (module_key->sum[offs] ^ module_key->key[offs]) | ((module_key->sum[offs + 1] ^ module_key->key[offs + 1]) << 8);
	}

	return ((u16 *)m_mainrom->base())[offset];
}

void pgm2_state::module_rom_w(offs_t offset, u16 data)
{
	//logerror("module write %04X at %08X\n", data, offset);
	u32 const dec_val = ((module_key->key[0] | (module_key->key[1] << 8) | (module_key->key[2] << 16)) >> 6) & 0xffff;
	if (data == dec_val)
	{
		// might be wrong and normal data access enabled only after whole sequence complete
		decrypt_kov3_module(module_key->addr_xor, module_key->data_xor);
	}
	else
		switch (data)
		{
			// following might be wrong, and trigger is address or both
		case 0x00c2: // checksum read mode enable, step 1 and 4
			module_sum_read = true;
			if (offset != 0xe5a7 && offset != 0xa521)
				popmessage("module write %04X at %08X\n", data, offset);
			break;
		case 0x0084: // checksum read mode enable, step 2 and 3
			if (offset != 0x5e7a && offset != 0x5a12)
				popmessage("module write %04X at %08X\n", data, offset);
			break;
		default:
			logerror("module write %04X at %08X\n", data, offset);
			break;
		}
}

// very primitive Atmel ARM PIO simulation, should be improved and devicified
void pgm2_state::pio_sodr_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_pio_out_data |= data & mem_mask;
	module_data_w((m_pio_out_data & 0x100) ? ASSERT_LINE : CLEAR_LINE);
	module_clk_w((m_pio_out_data & 0x200) ? ASSERT_LINE : CLEAR_LINE);
}
void pgm2_state::pio_codr_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_pio_out_data &= ~(data & mem_mask);
	module_data_w((m_pio_out_data & 0x100) ? ASSERT_LINE : CLEAR_LINE);
	module_clk_w((m_pio_out_data & 0x200) ? ASSERT_LINE : CLEAR_LINE);
}
u32 pgm2_state::pio_pdsr_r()
{
	return (module_data_r() == ASSERT_LINE ? 1 : 0) << 8; // fpga data read and status (bit 7, must be 0)
}

void pgm2_state::pgm2_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); //.region("mainrom", 0x00000); // internal ROM

	map(0x02000000, 0x0200ffff).ram().share("sram"); // 'battery RAM' (in CPU?)

	map(0x03600000, 0x036bffff).rw(FUNC(pgm2_state::mcu_r), FUNC(pgm2_state::mcu_w));

	map(0x03900000, 0x03900003).portr("INPUTS0");
	map(0x03a00000, 0x03a00003).portr("INPUTS1");

	map(0x20000000, 0x2007ffff).ram().share("mainram");

	map(0x30000000, 0x30001fff).ram().share("sp_videoram"); // spriteram ('move' RAM in test mode)

	map(0x30020000, 0x30021fff).ram().w(FUNC(pgm2_state::bg_videoram_w)).share("bg_videoram");
	map(0x30040000, 0x30045fff).ram().w(FUNC(pgm2_state::fg_videoram_w)).share("fg_videoram");

	map(0x30060000, 0x30063fff).ram().w(m_sp_palette, FUNC(palette_device::write32)).share("sp_palette");

	map(0x30080000, 0x30081fff).ram().w(m_bg_palette, FUNC(palette_device::write32)).share("bg_palette");

	map(0x300a0000, 0x300a07ff).ram().w(m_tx_palette, FUNC(palette_device::write32)).share("tx_palette");

	map(0x300c0000, 0x300c01ff).ram().share("sp_zoom"); // sprite zoom table - it uploads the same data 4x, maybe xshrink,xgrow,yshrink,ygrow or just redundant mirrors

	/* linescroll RAM - it clears to 0x3bf on startup which is enough bytes for 240 lines if each rowscroll value was 8 bytes, but each row is 4,
	so only half of this is used? or tx can do it too (unlikely, as orl2 writes 256 lines of data) maybe just bad mem check bounds on orleg2.
	It reports pass even if it fails the first byte but if the first byte passes it attempts to test 0x10000 bytes, which is far too big so
	what is the real size? */
	map(0x300e0000, 0x300e03ff).ram().share("lineram").mirror(0x000fc00);

	map(0x30100000, 0x301000ff).rw(FUNC(pgm2_state::shareram_r), FUNC(pgm2_state::shareram_w)).umask32(0x00ff00ff);

	map(0x30120000, 0x30120003).ram().share("bgscroll"); // scroll
	map(0x30120008, 0x3012000b).ram().share("fgscroll");
	map(0x3012000c, 0x3012000f).ram().share("vidmode");
	map(0x30120014, 0x30120017).w(FUNC(pgm2_state::unk30120014_w));
	map(0x30120018, 0x30120019).w(FUNC(pgm2_state::vbl_ack_w));
	map(0x30120032, 0x30120033).w(FUNC(pgm2_state::share_bank_w));
	map(0x30120038, 0x3012003b).w(FUNC(pgm2_state::sprite_encryption_w));
	// there are other 0x301200xx regs

	map(0x40000000, 0x40000003).r("ymz774", FUNC(ymz774_device::read)).w("ymz774", FUNC(ymz774_device::write));

	// internal IGS036 - most of them is standard ATMEL peripherals followed by custom bits
	// map(0xfffa0000, 0xfffa00ff) TC (Timer Counter) not used, mentioned in disabled / unused code
	// map(0xffffec00, 0xffffec7f) SMC (Static Memory Controller)
	// map(0xffffee00, 0xffffee57) MATRIX (Bus Matrix)
	map(0xfffff000, 0xfffff14b).m(m_arm_aic, FUNC(arm_aic_device::regs_map));
	// map(0xfffff200, 0xfffff247) DBGU (Debug Unit)
	// map(0xfffff400, 0xfffff4af) PIO (Parallel Input Output Controller)
	map(0xfffff430, 0xfffff437).nopw(); // often
	// map(0xfffffd00, 0xfffffd0b) RSTC (Reset Controller)
	// map(0xfffffd20, 0xfffffd2f) RTTC (Real Time Timer)
	map(0xfffffd28, 0xfffffd2b).r(FUNC(pgm2_state::rtc_r));
	// map(0xfffffd40, 0xfffffd4b) WDTC (Watch Dog Timer)
	// custom IGS036 stuff starts here
	map(0xfffffa08, 0xfffffa0b).w(FUNC(pgm2_state::encryption_do_w)); // after uploading encryption? table might actually send it or enable external ROM? when read bits0-1 called FUSE 0 and 1, must be 0
	map(0xfffffa0c, 0xfffffa0f).r(FUNC(pgm2_state::unk_startup_r)); // written 0, then 0x1c, then expected to return (result&0x180)==0x180, then written 0x7c
	map(0xfffffc00, 0xfffffcff).rw(FUNC(pgm2_state::encryption_r), FUNC(pgm2_state::encryption_w));
}


void pgm2_state::pgm2_rom_map(address_map &map)
{
	pgm2_map(map);
	map(0x10000000, 0x10ffffff).rom().region("mainrom", 0); // external ROM
}

void pgm2_state::pgm2_ram_rom_map(address_map &map)
{
	pgm2_map(map);
	map(0x10000000, 0x101fffff).ram().share("romboard_ram"); // we should also probably decrypt writes once the encryption is enabled, but the game never writes with it turned on anyway
	map(0x10200000, 0x103fffff).rom().region("mainrom", 0); // external ROM
}

void pgm2_state::pgm2_module_rom_map(address_map &map)
{
	pgm2_rom_map(map);
	map(0x10000000, 0x107fffff).w(FUNC(pgm2_state::module_rom_w));
	map(0x10000000, 0x1000000f).r(FUNC(pgm2_state::module_rom_r));
	map(0xfffff430, 0xfffff433).w(FUNC(pgm2_state::pio_sodr_w));
	map(0xfffff434, 0xfffff437).w(FUNC(pgm2_state::pio_codr_w));
	map(0xfffff43c, 0xfffff43f).r(FUNC(pgm2_state::pio_pdsr_r));
}

static INPUT_PORTS_START( pgm2 )
	PORT_START("INPUTS0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Test Key P1 & P2") // test key p1+p2
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Test Key P3 & P4") // test key p3+p4
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Service P1 & P2") // service key p1+p2
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Service P3 & P4") // service key p3+p4
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_SERVICE( 0x01000000, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPNAME( 0x02000000, 0x02000000, "Music" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x02000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "Voice" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x04000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "Free" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x10000000, "Stop" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(          0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(          0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(          0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "Debug" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(          0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
INPUT_PORTS_END


void pgm2_state::irq(int state)
{
//  logerror("irq\n");
	if (state == ASSERT_LINE) m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
	else m_maincpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);
}

void pgm2_state::machine_start()
{
	save_item(NAME(m_encryption_table));
	save_item(NAME(m_has_decrypted));
	save_item(NAME(m_has_decrypted_kov3_module));
	save_item(NAME(m_spritekey));
	save_item(NAME(m_realspritekey));
	save_item(NAME(m_mcu_regs));
	save_item(NAME(m_mcu_result0));
	save_item(NAME(m_mcu_result1));
	save_item(NAME(m_mcu_last_cmd));
	save_item(NAME(m_shareram));
	save_item(NAME(m_share_bank));
	save_item(NAME(m_pio_out_data));
	save_item(NAME(module_in_latch));
	save_item(NAME(module_sum_read));
	save_item(NAME(module_out_latch));
	save_item(NAME(module_prev_state));
	save_item(NAME(module_clk_cnt));
	save_item(NAME(module_rcv_buf));
	save_item(NAME(module_send_buf));
}

void pgm2_state::machine_reset()
{
	m_spritekey = 0;
	m_realspritekey = 0;
	m_mcu_last_cmd = 0;
	m_share_bank = 0;

	// as the decryption is dynamic controlled by the program, restore the encrypted copy
	memcpy(m_mainrom->base(), &m_encrypted_copy[0], m_mainrom->bytes());

	m_has_decrypted = false;
	m_has_decrypted_kov3_module = false;

	m_pio_out_data = 0;
	module_prev_state = 0;
	module_sum_read = false;
	module_clk_cnt = 151; // this needed because of "false" clock pulse happen during gpio init
}

static const gfx_layout tiles32x32x8_layout =
{
	32,32,
	RGN_FRAC(1,1),
	7,
	{ 1, 2, 3, 4, 5, 6, 7 },
	{ STEP32(0,8) },
	{ STEP32(0,8*32) },
	256*32
};

static GFXDECODE_START( pgm2_tx )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_lsb, 0, 0x800/4/0x10 )
GFXDECODE_END

static GFXDECODE_START( pgm2_bg )
	GFXDECODE_ENTRY( "bgtile", 0, tiles32x32x8_layout, 0, 0x2000/4/0x80 )
GFXDECODE_END

void pgm2_state::pgm2(machine_config &config)
{
	// basic machine hardware
	IGS036(config, m_maincpu, 100000000); // Unknown clock / divider
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm2_state::pgm2_rom_map);

	TIMER(config, m_mcu_timer, 0).configure_generic(FUNC(pgm2_state::mcu_interrupt));

	ARM_AIC(config, m_arm_aic, 0).irq_callback().set(FUNC(pgm2_state::irq));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh(HZ_TO_ATTOSECONDS(59.08)); // 59.08Hz, 264 total lines @ 15.59KHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0, 448-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(pgm2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(pgm2_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode2, m_tx_palette, pgm2_tx);
	GFXDECODE(config, m_gfxdecode3, m_bg_palette, pgm2_bg);

	PALETTE(config, m_sp_palette).set_format(palette_device::xRGB_888, 0x4000 / 4); // sprites
	PALETTE(config, m_tx_palette).set_format(palette_device::xRGB_888, 0x800 / 4); // text
	PALETTE(config, m_bg_palette).set_format(palette_device::xRGB_888, 0x2000 / 4); // bg

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz774_device &ymz774(YMZ774(config, "ymz774", 16384000)); // is clock correct ?
	ymz774.add_route(0, "lspeaker", 1.0);
	ymz774.add_route(1, "rspeaker", 1.0);

	PGM2_MEMCARD(config, m_memcard[0], 0);
	PGM2_MEMCARD(config, m_memcard[1], 0);
	PGM2_MEMCARD(config, m_memcard[2], 0);
	PGM2_MEMCARD(config, m_memcard[3], 0);
}

// not strictly needed as the video code supports changing on the fly, but makes recording easier etc.
void pgm2_state::pgm2_lores(machine_config &config)
{
	pgm2(config);
	m_screen->set_refresh(HZ_TO_ATTOSECONDS(15625.0/264.0)); // not verified
	m_screen->set_visarea(0, 320-1, 0, 240-1);
}

void pgm2_state::pgm2_hires(machine_config &config)
{
	pgm2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm2_state::pgm2_module_rom_map);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
}

void pgm2_state::pgm2_ramrom(machine_config &config)
{
	pgm2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm2_state::pgm2_ram_rom_map);
}

/* using macros for the video / sound roms because the locations never change between sets, and
   we're going to have a LOT of clones to cover all the internal rom regions and external rom revision
   combinations, so it keeps things readable */

// Oriental Legend 2

#define ORLEG2_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASE00 ) \
	ROM_LOAD( "ig-a_text.u4",            0x00000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) ) \
	\
	ROM_REGION( 0x1000000, "bgtile", 0 ) \
	ROM_LOAD32_WORD( "ig-a_bgl.u35",     0x00000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) ) \
	ROM_LOAD32_WORD( "ig-a_bgh.u36",     0x00000002, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) ) \
	\
	ROM_REGION( 0x2000000, "sprites_mask", 0 ) /* 1bpp sprite mask data (packed) */ \
	ROM_LOAD32_WORD( "ig-a_bml.u12",     0x00000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) ) \
	ROM_LOAD32_WORD( "ig-a_bmh.u16",     0x00000002, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) ) \
	\
	ROM_REGION( 0x4000000, "sprites_colour", 0 ) /* sprite colour data (6bpp data, 2 bits unused except for 4 bytes that are randomly 0xff - check dump?) */ \
	ROM_LOAD32_WORD( "ig-a_cgl.u18",     0x00000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) ) \
	ROM_LOAD32_WORD( "ig-a_cgh.u26",     0x00000002, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) ) \
	\
	ROM_REGION( 0x1000000, "ymz774", ROMREGION_ERASEFF ) /* ymz774 */ \
	ROM_LOAD16_WORD_SWAP( "ig-a_sp.u2",  0x00000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) ) \
	\
	ROM_REGION( 0x10000, "sram", 0 ) \
	ROM_LOAD( "xyj2_nvram",            0x00000000, 0x10000, CRC(ccccc71c) SHA1(585b5ccbf89dd28d8532da785d7c8af12f31c6d6) )

/*
   External program revisions are CONFIRMED to be the same between regions, even if the label changes (localized game title + country specific extension code)

   Confirmed country codes used on labels
   FA = Oversea
   CN = China
   JP = Japan
   TW = Taiwan

*/

#define ORLEG2_PROGRAM_104(prefix, extension) \
	ROM_REGION32_LE( 0x1000000, "mainrom", 0 ) \
	ROM_LOAD( #prefix "_v104" #extension ".u7",  0x000000, 0x800000, CRC(7c24a4f5) SHA1(3cd9f9264ef2aad0869afdf096e88eb8d74b2570) ) // V104 08-03-03 13:25:37

#define ORLEG2_PROGRAM_103(prefix, extension) \
	ROM_REGION32_LE( 0x1000000, "mainrom", 0 ) \
	ROM_LOAD( #prefix "_v103" #extension ".u7",  0x000000, 0x800000, CRC(21c1fae8) SHA1(36eeb7a5e8dc8ee7c834f3ff1173c28cf6c2f1a3) ) // V103 08-01-30 14:45:17

#define ORLEG2_PROGRAM_101(prefix, extension) \
	ROM_REGION32_LE( 0x1000000, "mainrom", 0 ) \
	ROM_LOAD( #prefix "_v101" #extension ".u7",  0x000000, 0x800000, CRC(45805b53) SHA1(f2a8399c821b75fadc53e914f6f318707e70787c) ) // V101 07-12-24 09:32:32

/*
   Internal ROMs for CHINA, JAPAN and OVERSEA are confirmed to differ by just the region byte, other regions not yet verified.
   label is a localized version of the game title and the country code (see above)
   For OVERSEA this is "O/L2", but we omit the / due to naming rules
   For the CHINA version this uses the Chinese characters
*/

#define ORLEG2_INTERNAL_CHINA \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	ROM_LOAD( "xyj2_cn.igs036", 0x00000000, 0x0004000, CRC(bcce7641) SHA1(c3b5cf6e9f6eae09b6785314777a52b34c3c7657) ) /* Core V100 China */ \
	ROM_REGION( 0x108, "default_card", 0 ) \
	ROM_LOAD( "blank_orleg2_china_card.pg2", 0x000, 0x108, CRC(dc29556f) SHA1(2335cc7af25d4dd9763c6944d3f0eb50276de80a) )

#define ORLEG2_INTERNAL_OVERSEAS \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	ROM_LOAD( "ol2_fa.igs036", 0x00000000, 0x0004000, CRC(cc4d398a) SHA1(c50bcc81f02cd5aa8ad157d73209dc53bdedc023) ) // Core V100 Oversea

#define ORLEG2_INTERNAL_JAPAN \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	ROM_LOAD( "ol2_a10.igs036", 0x00000000, 0x0004000, CRC(69375284) SHA1(a120c6a3d8d7898cc3ca508abea78e5e54090c66) ) // Core V100 Japan

ROM_START( orleg2 )
	ORLEG2_INTERNAL_OVERSEAS
	ORLEG2_PROGRAM_104(ol2,fa)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_103 )
	ORLEG2_INTERNAL_OVERSEAS
	ORLEG2_PROGRAM_103(ol2,fa)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_101 )
	ORLEG2_INTERNAL_OVERSEAS
	ORLEG2_PROGRAM_101(ol2,fa)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_104cn )
	ORLEG2_INTERNAL_CHINA
	ORLEG2_PROGRAM_104(xyj2,cn)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_103cn )
	ORLEG2_INTERNAL_CHINA
	ORLEG2_PROGRAM_103(xyj2,cn)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_101cn )
	ORLEG2_INTERNAL_CHINA
	ORLEG2_PROGRAM_101(xyj2,cn)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_104jp )
	ORLEG2_INTERNAL_JAPAN
	ORLEG2_PROGRAM_104(ol2,a10)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_103jp )
	ORLEG2_INTERNAL_JAPAN
	ORLEG2_PROGRAM_103(ol2,a10)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_101jp )
	ORLEG2_INTERNAL_JAPAN
	ORLEG2_PROGRAM_101(ol2,a10)
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

// Knights of Valour 2 New Legend

#define KOV2NL_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASE00 ) \
	ROM_LOAD( "ig-a3_text.u4",           0x00000000, 0x0200000, CRC(214530ff) SHA1(4231a02054b0345392a077042b95779fd45d6c22) ) \
	\
	ROM_REGION( 0x1000000, "bgtile", 0 ) \
	ROM_LOAD32_WORD( "ig-a3_bgl.u35",    0x00000000, 0x0800000, CRC(2d46b1f6) SHA1(ea8c805eda6292e86a642e9633d8fee7054d10b1) ) \
	ROM_LOAD32_WORD( "ig-a3_bgh.u36",    0x00000002, 0x0800000, CRC(df710c36) SHA1(f826c3f496c4f17b46d18af1d8e02cac7b7027ac) ) \
	\
	ROM_REGION( 0x2000000, "sprites_mask", 0 ) /* 1bpp sprite mask data */ \
	ROM_LOAD32_WORD( "ig-a3_bml.u12",    0x00000000, 0x1000000, CRC(0bf63836) SHA1(b8e4f1951f8074b475b795bd7840c5a375b6f5ef) ) \
	ROM_LOAD32_WORD( "ig-a3_bmh.u16",    0x00000002, 0x1000000, CRC(4a378542) SHA1(5d06a8a8796285a786ebb690c34610f923ef5570) ) \
	\
	ROM_REGION( 0x4000000, "sprites_colour", 0 ) /* sprite colour data */ \
	ROM_LOAD32_WORD( "ig-a3_cgl.u18",    0x00000000, 0x2000000, CRC(8d923e1f) SHA1(14371cf385dd8857017d3111cd4710f4291b1ae2) ) \
	ROM_LOAD32_WORD( "ig-a3_cgh.u26",    0x00000002, 0x2000000, CRC(5b6fbf3f) SHA1(d1f52e230b91ee6cde939d7c2b74da7fd6527e73) ) \
	\
	ROM_REGION( 0x2000000, "ymz774", ROMREGION_ERASEFF ) /* ymz774 */ \
	ROM_LOAD16_WORD_SWAP( "ig-a3_sp.u37",            0x00000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) ) \
	\
	ROM_REGION(0x10000, "sram", 0) \
	ROM_LOAD("gsyx_nvram", 0x00000000, 0x10000, CRC(22400c16) SHA1(f775a16299c30f2ce23d683161b910e06eff37c1) )

#define KOV2NL_PROGRAM_302(prefix, extension) \
	ROM_REGION32_LE( 0x1000000, "mainrom", 0 ) \
	ROM_LOAD( #prefix "_v302" #extension ".u7", 0x00000000, 0x0800000, CRC(b19cf540) SHA1(25da5804bbfd7ef2cdf5cc5aabaa803d18b98929) ) // V302 08-12-03 15:27:34

#define KOV2NL_PROGRAM_301(prefix, extension) \
	ROM_REGION32_LE( 0x1000000, "mainrom", 0 ) \
	ROM_LOAD( #prefix "_v301" #extension ".u7", 0x000000, 0x800000, CRC(c4595c2c) SHA1(09e379556ef76f81a63664f46d3f1415b315f384) ) // V301 08-09-09 09:44:53

#define KOV2NL_PROGRAM_300(prefix, extension) \
	ROM_REGION32_LE( 0x1000000, "mainrom", 0 ) \
	ROM_LOAD( #prefix "_v300" #extension ".u7", 0x000000, 0x800000, CRC(08da7552) SHA1(303b97d7694405474c8133a259303ccb49db48b1) ) // V300 08-08-06 18:21:23


// Region 0x00 - China
#define KOV2NL_INTERNAL_CHINA \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	ROM_LOAD( "gsyx_igs036_china.rom", 0x00000000, 0x0004000, CRC(e09fe4ce) SHA1(c0cac64ef8727cbe79d503ec4df66ddb6f2c925e) ) /* Core V100 China */ \
	ROM_REGION( 0x108, "default_card", 0 ) \
	ROM_LOAD( "blank_gsyx_china.pg2", 0x000, 0x108, CRC(02842ae8) SHA1(a6cda633b09a706039a79b73db2c258094826f85) )

// Region 0x01 - Taiwan  CRC(b3ca3124) SHA1(793d3bdc4bfccb892eb51c351c4ccd103ee9b7ce)
// uses cards with CRC(1155f01f) SHA1(60f7bed1461b362a3da687503cd72ed2d5e96f30) (same as Oversea, Korea)

// Region 0x02 - Japan CRC(46344f1a) SHA1(fbe846be4a39e8a4c41417858311faaaebf67cb9)
// uses cards with CRC(0d63cb64) SHA1(957cce2d47f3369bc4f98b1652ba8639c08fb9bd) (unique)

// Region 0x03 - Korea CRC(15619af0) SHA1(619e58e13c4d4351e8a4359a1df1eb9952326e84)
// uses cards with CRC(1155f01f) SHA1(60f7bed1461b362a3da687503cd72ed2d5e96f30) (same as Oversea, Taiwan)
// (incomplete / partial translation, shows Oversea disclaimer and corrupt text on some screens, so likely unreleased or needs newer mainprg)

// Region 0x04 - Hong Kong  CRC(76b9b527) SHA1(e77a7b59aca221b5d04dcd1ffc632114be7e5647)
// uses cards with CRC(02842ae8) SHA1(a6cda633b09a706039a79b73db2c258094826f85) (same as China)

// Region 0x05 - Overseas
#define KOV2NL_INTERNAL_OVERSEA \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	ROM_LOAD( "kov2nl_igs036_oversea.rom", 0x00000000, 0x0004000, CRC(25ec60cd) SHA1(7dd12d2bc642bfa79520676fe5de458ce7d08ef6) ) /* Core V100 oversea */ \
	ROM_REGION( 0x108, "default_card", 0 ) \
	ROM_LOAD( "blank_kov2nl_overseas_card.pg2", 0x000, 0x108, CRC(1155f01f) SHA1(60f7bed1461b362a3da687503cd72ed2d5e96f30) )


ROM_START( kov2nl )
	KOV2NL_INTERNAL_OVERSEA
	KOV2NL_PROGRAM_302(kov2nl, fa)
	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov2nl_301 )
	KOV2NL_INTERNAL_OVERSEA
	KOV2NL_PROGRAM_301(kov2nl, fa)
	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov2nl_300 )
	KOV2NL_INTERNAL_OVERSEA
	KOV2NL_PROGRAM_300(kov2nl, fa)
	KOV2NL_VIDEO_SOUND_ROMS
ROM_END


ROM_START( kov2nl_302cn )
	KOV2NL_INTERNAL_CHINA
	KOV2NL_PROGRAM_302(gsyx, cn)
	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov2nl_301cn )
	KOV2NL_INTERNAL_CHINA
	KOV2NL_PROGRAM_301(gsyx, cn)
	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov2nl_300cn )
	KOV2NL_INTERNAL_CHINA
	KOV2NL_PROGRAM_300(gsyx, cn)
	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

// Dodonpachi Daioujou Tamashii

#define DDPDOJT_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASE00 ) \
	ROM_LOAD( "ddpdoj_text.u1",          0x00000000, 0x0200000, CRC(f18141d1) SHA1(a16e0a76bc926a158bb92dfd35aca749c569ef50) ) \
	\
	ROM_REGION( 0x2000000, "bgtile", 0 ) \
	ROM_LOAD32_WORD( "ddpdoj_bgl.u23",   0x00000000, 0x1000000, CRC(ff65fdab) SHA1(abdd5ca43599a2daa722547a999119123dd9bb28) ) \
	ROM_LOAD32_WORD( "ddpdoj_bgh.u24",   0x00000002, 0x1000000, CRC(bb84d2a6) SHA1(a576a729831b5946287fa8f0d923016f43a9bedb) ) \
	\
	ROM_REGION( 0x1000000, "sprites_mask", 0 ) /* 1bpp sprite mask data */ \
	ROM_LOAD32_WORD( "ddpdoj_mapl0.u13", 0x00000000, 0x800000, CRC(bcfbb0fc) SHA1(9ec478eba9905913cf997bd9b46c70c1ad383630) ) \
	ROM_LOAD32_WORD( "ddpdoj_maph0.u15", 0x00000002, 0x800000, CRC(0cc75d4e) SHA1(6d1b5ef0fdebf1e84fa199b939ffa07b810b12c9) ) \
	\
	ROM_REGION( 0x2000000, "sprites_colour", 0 ) /* sprite colour data */ \
	ROM_LOAD32_WORD( "ddpdoj_spa0.u9",   0x00000000, 0x1000000, CRC(1232c1b4) SHA1(ecc1c549ae19d2f052a85fe4a993608aedf49a25) ) \
	ROM_LOAD32_WORD( "ddpdoj_spb0.u18",  0x00000002, 0x1000000, CRC(6a9e2cbf) SHA1(8e0a4ea90f5ef534820303d62f0873f8ac9f080e) ) \
	\
	ROM_REGION( 0x1000000, "ymz774", ROMREGION_ERASEFF ) /* ymz774 */ \
	ROM_LOAD16_WORD_SWAP( "ddpdoj_wave0.u12",        0x00000000, 0x1000000, CRC(2b71a324) SHA1(f69076cc561f40ca564d804bc7bd455066f8d77c) ) \
	\
	ROM_REGION( 0x10000, "sram", 0 ) \
	ROM_LOAD( "ddpdojt_sram",            0x00000000, 0x10000, CRC(af99e304) SHA1(e44fed22b902431298748eca84533f8685926afd) )

ROM_START( ddpdojt )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "ddpdoj_igs036_china.rom",       0x00000000, 0x0004000, CRC(5db91464) SHA1(723d8086285805bd815e62120dfa9a4269bcd932) ) // Core V100 China

	ROM_REGION32_LE( 0x0200000, "mainrom", 0 )
	ROM_LOAD( "ddpdoj_v201cn.u4",        0x00000000, 0x0200000, CRC(89e4b760) SHA1(9fad1309da31d12a413731b416a8bbfdb304ed9e) ) // V201 10-03-27 17:45:12

	DDPDOJT_VIDEO_SOUND_ROMS
ROM_END

// Knights of Valour 3

/*
   The Kov3 Program rom is a module consisting of a NOR flash and a FPGA, this provides an extra layer of encryption on top of the usual
   that is only unlocked when the correct sequence is recieved from the ARM MCU (IGS036)

   Newer gambling games use the same modules.
*/

#define KOV3_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASE00 ) \
	ROM_LOAD( "kov3_text.u1",            0x00000000, 0x0200000, CRC(198b52d6) SHA1(e4502abe7ba01053d16c02114f0c88a3f52f6f40) ) \
	\
	ROM_REGION( 0x2000000, "bgtile", 0 ) \
	ROM_LOAD32_WORD( "kov3_bgl.u6",      0x00000000, 0x1000000, CRC(49a4c5bc) SHA1(26b7da91067bda196252520e9b4893361c2fc675) ) \
	ROM_LOAD32_WORD( "kov3_bgh.u7",      0x00000002, 0x1000000, CRC(adc1aff1) SHA1(b10490f0dbef9905cdb064168c529f0b5a2b28b8) ) \
	\
	ROM_REGION( 0x4000000, "sprites_mask", 0 ) /* 1bpp sprite mask data */ \
	ROM_LOAD32_WORD( "kov3_mapl0.u15",   0x00000000, 0x2000000, CRC(9e569bf7) SHA1(03d26e000e9d8e744546be9649628d2130f2ec4c) ) \
	ROM_LOAD32_WORD( "kov3_maph0.u16",   0x00000002, 0x2000000, CRC(6f200ad8) SHA1(cd12c136d4f5d424bd7daeeacd5c4127beb3d565) ) \
	\
	ROM_REGION( 0x8000000, "sprites_colour", 0 ) /* sprite colour data */ \
	ROM_LOAD32_WORD( "kov3_spa0.u17",    0x00000000, 0x4000000, CRC(3a1e58a9) SHA1(6ba251407c69ee62f7ea0baae91bc133acc70c6f) ) \
	ROM_LOAD32_WORD( "kov3_spb0.u10",    0x00000002, 0x4000000, CRC(90396065) SHA1(01bf9f69d77a792d5b39afbba70fbfa098e194f1) ) \
	\
	ROM_REGION( 0x4000000, "ymz774", ROMREGION_ERASEFF ) /* ymz774 */ \
	ROM_LOAD16_WORD_SWAP( "kov3_wave0.u13",              0x00000000, 0x4000000, CRC(aa639152) SHA1(2314c6bd05524525a31a2a4668a36a938b924ba4) ) \
	\
	ROM_REGION( 0x10000, "sram", 0 ) \
	ROM_LOAD( "kov3_sram",            0x00000000, 0x10000, CRC(d9608102) SHA1(dec5631642393f4ec76912c81fd60249bb45aa13) )

#define KOV3_INTERNAL_CHINA \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	ROM_LOAD( "kov3_igs036_china.rom", 0x00000000, 0x0004000, CRC(c7d33764) SHA1(5cd48f876e637d60391d39ac6e40bf243300cc75) ) /* Core V100 China */ \
	ROM_REGION( 0x108, "default_card", 0 ) \
	ROM_LOAD( "blank_kov3_china_card.pg2", 0x000, 0x108, CRC(bd5a968f) SHA1(b9045eb70e02afda7810431c592208053d863980) )


ROM_START( kov3 )
	KOV3_INTERNAL_CHINA

	ROM_REGION32_LE( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "kov3_v104cn_raw.bin",         0x00000000, 0x0800000, CRC(1b5cbd24) SHA1(6471d4842a08f404420dea2bd1c8b88798c80fd5) ) // V104 11-12-09 14:29:14

	KOV3_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov3_102 )
	KOV3_INTERNAL_CHINA

	ROM_REGION32_LE( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "kov3_v102cn_raw.bin",         0x00000000, 0x0800000, CRC(61d0dabd) SHA1(959b22ef4e342ca39c2386549ac7274f9d580ab8) ) // V102 11-11-01 18:56:07

	KOV3_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov3_101 )
	KOV3_INTERNAL_CHINA

	ROM_REGION32_LE( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "kov3_v101.bin",         0x00000000, 0x0800000, BAD_DUMP CRC(d6664449) SHA1(64d912425f018c3531951019b33e909657724547) ) // V101 11-10-03 14:37:29; dump was not raw, manually xored with fake value

	KOV3_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov3_100 )
	KOV3_INTERNAL_CHINA

	ROM_REGION32_LE( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "kov3_v100cn_raw.bin",         0x00000000, 0x0800000, CRC(93bca924) SHA1(ecaf2c4676eb3d9f5e4fdbd9388be41e51afa0e4) ) // V100 11-09-14 15:13:14

	KOV3_VIDEO_SOUND_ROMS
ROM_END

/* King of Fighters '98: Ultimate Match HERO

device types were as follows

kof98umh_v100cn.u4  SAMSUNG K8Q2815UQB
ig-d3_text.u1       cFeon EN29LV160AB
all others:         SPANSION S99-50070

*/

#define KOF98UMH_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASE00 ) \
	ROM_LOAD( "ig-d3_text.u1",          0x00000000, 0x0200000, CRC(9a0ea82e) SHA1(7844fd7e46c3fbb2164060f160da528254fd177e) ) \
	\
	ROM_REGION( 0x2000000, "bgtile", ROMREGION_ERASE00 ) \
	/* bgl/bgh unpopulated (no background tilemap) */ \
	\
	ROM_REGION( 0x08000000, "sprites_mask", 0 ) /* 1bpp sprite mask data */ \
	ROM_LOAD32_WORD( "ig-d3_mapl0.u13", 0x00000000, 0x4000000, CRC(5571d63e) SHA1(dad73797a35738013d82e3b8ca96fa001ec56f69) ) \
	ROM_LOAD32_WORD( "ig-d3_maph0.u15", 0x00000002, 0x4000000, CRC(0da7b1b8) SHA1(87741242bd827eca3788b490df6dcb65f7a89733) ) \
	\
	ROM_REGION( 0x20000000, "sprites_colour", 0 ) /* sprite colour data - some byte are 0x40 or even 0xff, but verified on 2 boards */ \
	ROM_LOAD32_WORD( "ig-d3_spa0.u9",   0x00000000, 0x4000000, CRC(cfef8f7d) SHA1(54f58d1b9eb7d2e4bbe13fbdfd98f5b14ce2086b) ) \
	ROM_LOAD32_WORD( "ig-d3_spb0.u18",  0x00000002, 0x4000000, CRC(f199d5c8) SHA1(91f5e8efd1f6a9e5aada51afdf5a8f52bac24185) ) \
	/* spa1/spb1 unpopulated */ \
	ROM_LOAD32_WORD( "ig-d3_spa2.u10",  0x10000000, 0x4000000, CRC(03bfd35c) SHA1(814998cd5ee01c9da775b73f7a0ba4216fe4970e) ) \
	ROM_LOAD32_WORD( "ig-d3_spb2.u20",  0x10000002, 0x4000000, CRC(9aaa840b) SHA1(3c6078d53bb5eca5c501540214287dd102102ea1) ) \
	/* spa3/spb3 unpopulated */ \
	\
	ROM_REGION( 0x08000000, "ymz774", ROMREGION_ERASEFF ) /* ymz774 */ \
	ROM_LOAD16_WORD_SWAP( "ig-d3_wave0.u12",        0x00000000, 0x4000000, CRC(edf2332d) SHA1(7e01c7e03e515814d7de117c265c3668d32842fa) ) \
	ROM_LOAD16_WORD_SWAP( "ig-d3_wave1.u11",        0x04000000, 0x4000000, CRC(62321b20) SHA1(a388c8a2489430fbe92fb26b3ef81c66ce97f318) ) \
	\
	ROM_REGION( 0x10000, "sram", 0 ) \
	ROM_LOAD( "kof98umh_sram",            0x00000000, 0x10000, CRC(60460ed9) SHA1(55cd8de37cee04ff7ad940fb52f8fb8db042c26e) )


ROM_START( kof98umh )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "kof98umh_internal_rom.bin",       0x00000000, 0x0004000, CRC(3ed2e50f) SHA1(35310045d375d9dda36c325e35257123a7b5b8c7) ) // Core V100 China

	ROM_REGION32_LE( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "kof98umh_v100cn.u4",        0x00000000, 0x1000000, CRC(2ea91e3b) SHA1(5a586bb99cc4f1b02e0db462d5aff721512e0640) ) // V100 09-08-23 17:52:03

	KOF98UMH_VIDEO_SOUND_ROMS
ROM_END

// Single board game (PCB-0779-00-JG)
ROM_START( bubucar )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "bu_bu_car-en_b5_internal_rom.u12", 0x0000000, 0x0004000, NO_DUMP )

	ROM_REGION32_LE( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "bubu-car-s1c0en_ev29lv160ab.u23",  0x0000000, 0x0200000, CRC(4c5009ef) SHA1(5b0c96d7bd1243523eb3670b7545cc7455b2d668) )

	ROM_REGION( 0x0200000, "tiles", ROMREGION_ERASE00 )
	ROM_LOAD( "jg_text.u31",                      0x0000000, 0x0200000, CRC(aa6e8317) SHA1(b982873d0e2faefb89d263f945a5a3ba5f4efcd4) ) // EN29LV160AB-70TCP

	ROM_REGION( 0x2000000, "bgtile", ROMREGION_ERASE00 )
	// BGL/BGH unpopulated (no background tilemap)

	ROM_REGION( 0x2000000, "sprites_mask", 0 )
	ROM_LOAD32_WORD( "jg_map_bml.u38",            0x0000000, 0x1000000, CRC(c8c9fa6f) SHA1(22bf354f2ace9d3f05835525f24cc578feff6453) ) // K8Q2815UQB
	ROM_LOAD32_WORD( "jg_map_bmh.u37",            0x0000002, 0x1000000, CRC(f9ba71fd) SHA1(79dc5ca48d1ba069b702a3854a28836040e981d0) ) // K8Q2815UQB

	ROM_REGION( 0x2000000, "sprites_colour", 0 )
	ROM_LOAD32_WORD( "jg_cg_cgl.u19",             0x0000000, 0x1000000, CRC(1d7ab9b2) SHA1(322d458f550ebe72b569efe1691aa8902de7a6d4) ) // K8Q2815UQB
	ROM_LOAD32_WORD( "jg_cg_cgh.u20",             0x0000002, 0x1000000, CRC(2edeb815) SHA1(c9595108691586dfbb0b30bff9535f13e4c3afb3) ) // K8Q2815UQB

	ROM_REGION( 0x1000000, "ymz774", ROMREGION_ERASEFF ) // YMZ774
	ROM_LOAD16_WORD_SWAP( "jg_wave.u18",          0x0000000, 0x1000000, CRC(8ba99c0c) SHA1(5a7cccfae47eee5c9ea4c172f5126d514156f771) ) // K8Q2815UQB

	ROM_REGION( 0x10000, "sram", 0 )
	ROM_LOAD( "bubucar_en_sram",                  0x0000000, 0x0010000, NO_DUMP )
ROM_END

static void iga_u16_decode(u16 *rom, int len, int ixor)
{
	int i;

	for (i = 1; i < len / 2; i+=2)
	{
		u16 x = ixor;

		if ( (i>>1) & 0x000001) x ^= 0x0010;
		if ( (i>>1) & 0x000002) x ^= 0x2004;
		if ( (i>>1) & 0x000004) x ^= 0x0801;
		if ( (i>>1) & 0x000008) x ^= 0x0300;
		if ( (i>>1) & 0x000010) x ^= 0x0080;
		if ( (i>>1) & 0x000020) x ^= 0x0020;
		if ( (i>>1) & 0x000040) x ^= 0x4008;
		if ( (i>>1) & 0x000080) x ^= 0x1002;
		if ( (i>>1) & 0x000100) x ^= 0x0400;
		if ( (i>>1) & 0x000200) x ^= 0x0040;
		if ( (i>>1) & 0x000400) x ^= 0x8000;

		rom[i] ^= x;
		rom[i] = bitswap<16>(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static void iga_u12_decode(u16* rom, int len, int ixor)
{
	int i;

	for (i = 0; i < len / 2; i+=2)
	{
		u16 x = ixor;

		if ( (i>>1) & 0x000001) x ^= 0x9004;
		if ( (i>>1) & 0x000002) x ^= 0x0028;
		if ( (i>>1) & 0x000004) x ^= 0x0182;
		if ( (i>>1) & 0x000008) x ^= 0x0010;
		if ( (i>>1) & 0x000010) x ^= 0x2040;
		if ( (i>>1) & 0x000020) x ^= 0x0801;
		if ( (i>>1) & 0x000040) x ^= 0x0000;
		if ( (i>>1) & 0x000080) x ^= 0x0000;
		if ( (i>>1) & 0x000100) x ^= 0x4000;
		if ( (i>>1) & 0x000200) x ^= 0x0600;
		if ( (i>>1) & 0x000400) x ^= 0x0000;

		rom[i] ^= x;
		rom[i] = bitswap<16>(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static void sprite_colour_decode(u16* rom, int len)
{
	int i;

	for (i = 0; i < len / 2; i++)
	{
		rom[i] = bitswap<16>(rom[i], 15, 14, // unused - 6bpp
								   13, 12, 11,
								   5, 4, 3,
								   7, 6, // unused - 6bpp
								   10, 9, 8,
								   2, 1, 0  );
	}
}

u32 pgm2_state::orleg2_speedup_r()
{
	u32 const pc = m_maincpu->pc();
	if ((pc == 0x1002faec) || (pc == 0x1002f9b8))
	{
		if ((m_mainram[0x20114 / 4] == 0x00) && (m_mainram[0x20118 / 4] == 0x00))
			m_maincpu->spin_until_interrupt();
	}
	/*else
	{
	    logerror("pc is %08x\n", pc);
	}*/

	return m_mainram[0x20114 / 4];
}

u32 pgm2_state::kov2nl_speedup_r()
{
	u32 const pc = m_maincpu->pc();

	if ((pc == 0x10053a94) || (pc == 0x1005332c) || (pc == 0x1005327c))
	{
		if ((m_mainram[0x20470 / 4] == 0x00) && (m_mainram[0x20474 / 4] == 0x00))
			m_maincpu->spin_until_interrupt();
	}
	/*
	else
	{
	    logerror("pc is %08x\n", pc);
	}
	*/

	return m_mainram[0x20470 / 4];
}

u32 pgm2_state::kof98umh_speedup_r()
{
	u32 const pc = m_maincpu->pc();

	if (pc == 0x100028f6)
	{
		if ((m_mainram[0x00060 / 4] == 0x00) && (m_mainram[0x00064 / 4] == 0x00))
			m_maincpu->spin_until_interrupt();
	}
	/*
	else
	{
	    logerror("pc is %08x\n", pc);
	}
	*/

	return m_mainram[0x00060 / 4];
}

u32 pgm2_state::kov3_speedup_r()
{
	u32 const pc = m_maincpu->pc();

	if ((pc == 0x1000729a) || (pc == 0x1000729e))
	{
		if ((m_mainram[0x000b4 / 4] == 0x00) && (m_mainram[0x000b8 / 4] == 0x00))
			m_maincpu->spin_until_interrupt();
	}
	/*
	else
	{
	    logerror("pc is %08x\n", pc);
	}
	*/

	return m_mainram[0x000b4 / 4];
}




u32 pgm2_state::ddpdojt_speedup_r()
{
	u32 const pc = m_maincpu->pc();

	if (pc == 0x10001a7e)
	{
		if ((m_mainram[0x00060 / 4] == 0x00) && (m_mainram[0x00064 / 4] == 0x00))
			m_maincpu->spin_until_interrupt();
	}
	/*
	else
	{
	    logerror("pc is %08x\n", pc);
	}
	*/

	return m_mainram[0x00060 / 4];
}

u32 pgm2_state::ddpdojt_speedup2_r()
{
	u32 const pc = m_maincpu->pc();

	if (pc == 0x1008fefe || pc == 0x1008fbe8)
	{
		if ((m_mainram[0x21e04 / 4] & 0x00ff0000) != 0) // not sure if this endian safe ?
			m_maincpu->spin_until_interrupt();
	}
	/*
	else
	{
	    logerror("pc is %08x\n", pc);
	}
	*/

	return m_mainram[0x21e04 / 4];
}


// for games with the internal ROMs fully dumped that provide the sprite key and program rom key at runtime
void pgm2_state::common_encryption_init()
{
	// store off a copy of the encrypted rom so we can restore it later when needed
	m_encrypted_copy.resize(m_mainrom->bytes());
	memcpy(&m_encrypted_copy[0], m_mainrom->base(), m_mainrom->bytes());

	u16 *src = (u16 *)memregion("sprites_mask")->base();

	iga_u12_decode(src, memregion("sprites_mask")->bytes(), 0x0000);
	iga_u16_decode(src, memregion("sprites_mask")->bytes(), 0x0000);
	m_sprite_predecrypted = false;

	src = (u16 *)memregion("sprites_colour")->base();
	sprite_colour_decode(src, memregion("sprites_colour")->bytes());

	m_has_decrypted = false;
}

void pgm2_state::init_orleg2()
{
	common_encryption_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20020114, 0x20020117, read32smo_delegate(*this, FUNC(pgm2_state::orleg2_speedup_r)));
}

void pgm2_state::init_kov2nl()
{
	common_encryption_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20020470, 0x20020473, read32smo_delegate(*this, FUNC(pgm2_state::kov2nl_speedup_r)));
}

void pgm2_state::init_ddpdojt()
{
	common_encryption_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20000060, 0x20000063, read32smo_delegate(*this, FUNC(pgm2_state::ddpdojt_speedup_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20021e04, 0x20021e07, read32smo_delegate(*this, FUNC(pgm2_state::ddpdojt_speedup2_r)));
}

// currently we don't know how to derive address/data xor values from real keys, so we need both
static const kov3_module_key kov3_104_key = { { 0x40,0xac,0x30,0x00,0x47,0x49,0x00,0x00 } ,{ 0xeb,0x7d,0x8d,0x90,0x2c,0xf4,0x09,0x82 }, 0x18ec71, 0xb89d }; // fake zero-key
static const kov3_module_key kov3_102_key = { { 0x49,0xac,0xb0,0xec,0x47,0x49,0x95,0x38 } ,{ 0x09,0xbd,0xf1,0x31,0xe6,0xf0,0x65,0x2b }, 0x021d37, 0x81d0 };
static const kov3_module_key kov3_101_key = { { 0xc1,0x2c,0xc1,0xe5,0x3c,0xc1,0x59,0x9e } ,{ 0xf2,0xb2,0xf0,0x89,0x37,0xf2,0xc7,0x0b }, 0, 0xffff }; // real xor values is unknown
static const kov3_module_key kov3_100_key = { { 0x40,0xac,0x30,0x00,0x47,0x49,0x00,0x00 } ,{ 0x96,0xf0,0x91,0xe1,0xb3,0xf1,0xef,0x90 }, 0x3e8aa8, 0xc530 }; // fake zero-key

void pgm2_state::init_kov3()
{
	common_encryption_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000b4, 0x200000b7, read32smo_delegate(*this, FUNC(pgm2_state::kov3_speedup_r)));
}

void pgm2_state::decrypt_kov3_module(u32 addrxor, u16 dataxor)
{
	u16 *src = (u16 *)m_mainrom->base();
	u32 size = m_mainrom->bytes();

	std::vector<u16> buffer(size/2);

	for (int i = 0; i < size/2; i++)
		buffer[i] = src[i^addrxor]^dataxor;

	memcpy(src, &buffer[0], size);

	m_has_decrypted_kov3_module = true;
}

void pgm2_state::init_kov3_104()
{
	module_key = &kov3_104_key;
	init_kov3();
}

void pgm2_state::init_kov3_102()
{
	module_key = &kov3_102_key;
	init_kov3();
}

void pgm2_state::init_kov3_101()
{
	module_key = &kov3_101_key;
	init_kov3();
}

void pgm2_state::init_kov3_100()
{
	module_key = &kov3_100_key;
	init_kov3();
}

void pgm2_state::init_kof98umh()
{
	common_encryption_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20000060, 0x20000063, read32smo_delegate(*this, FUNC(pgm2_state::kof98umh_speedup_r)));
}

void pgm2_state::init_bubucar()
{
	common_encryption_init();
	// m_maincpu->space(AS_PROGRAM).install_read_handler(0x20020114, 0x20020117, read32smo_delegate(*this, FUNC(pgm2_state::bubucar_speedup_r))); TODO: once this is fully dumped and decrypted
}





// PGM2

//华通电子/Huatong Electronics (distributor of orleg2, kov2nl in china) = 华通科技/Huatong Technology (distributor of kov3 in china),
//Same company but they changed name.

// Oriental Legend 2 - should be a V102 and V100 too
//西游释厄传2/Xīyóu shì è zhuàn 2 (China; Simplified Chinese)
//西遊釋厄傳2/Saiyū Shakuyakuden 2 (Japan; Traditional Chinese - Taiwan(undumped) too?)
GAME( 2007, orleg2,       0,      pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS", "Oriental Legend 2 (V104, Oversea)", MACHINE_SUPPORTS_SAVE ) // Overseas sets of OL2 do not use the card reader
GAME( 2007, orleg2_103,   orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS", "Oriental Legend 2 (V103, Oversea)", MACHINE_SUPPORTS_SAVE )
GAME( 2007, orleg2_101,   orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS", "Oriental Legend 2 (V101, Oversea)", MACHINE_SUPPORTS_SAVE )

GAME( 2007, orleg2_104cn, orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS (Huatong license)", "Xiyou Shi E Zhuan 2 (V104, China)", MACHINE_SUPPORTS_SAVE )
GAME( 2007, orleg2_103cn, orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS (Huatong license)", "Xiyou Shi E Zhuan 2 (V103, China)", MACHINE_SUPPORTS_SAVE )
GAME( 2007, orleg2_101cn, orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS (Huatong license)", "Xiyou Shi E Zhuan 2 (V101, China)", MACHINE_SUPPORTS_SAVE )

GAME( 2007, orleg2_104jp, orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS (Alta license)", "Saiyuu Shakuyakuden 2 (V104, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 2007, orleg2_103jp, orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS (Alta license)", "Saiyuu Shakuyakuden 2 (V103, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 2007, orleg2_101jp, orleg2, pgm2,        pgm2, pgm2_state, init_orleg2,   ROT0, "IGS (Alta license)", "Saiyuu Shakuyakuden 2 (V101, Japan)", MACHINE_SUPPORTS_SAVE )

// Knights of Valour 2 New Legend
//三國戰紀2撗掃于軍 New Legend/Sānguó zhàn jì 2 Guàng sǎo yú jūn New Legend (Oversea; Mixed Traditional and Simplified Chinese)
//三國戰紀2盖世英雄/Sānguó zhàn jì 2 Gàishì yīngxióng (China; Mixed Traditional and Simplified Chinese)
//三國戰紀2乱世英雄/Sangoku-Senki 2 Ranse Eiyū (Japan; Mixed Traditional and Simplified Chinese - Undumped)
GAME( 2008, kov2nl,       0,      pgm2,        pgm2, pgm2_state, init_kov2nl,   ROT0, "IGS", "Knights of Valour 2 New Legend / Sanguo Zhan Ji 2 Guang Sao Yu Jun (V302, Oversea)", MACHINE_SUPPORTS_SAVE )
GAME( 2008, kov2nl_301,   kov2nl, pgm2,        pgm2, pgm2_state, init_kov2nl,   ROT0, "IGS", "Knights of Valour 2 New Legend / Sanguo Zhan Ji 2 Guang Sao Yu Jun (V301, Oversea)", MACHINE_SUPPORTS_SAVE )
GAME( 2008, kov2nl_300,   kov2nl, pgm2,        pgm2, pgm2_state, init_kov2nl,   ROT0, "IGS", "Knights of Valour 2 New Legend / Sanguo Zhan Ji 2 Guang Sao Yu Jun (V300, Oversea)", MACHINE_SUPPORTS_SAVE )

GAME( 2008, kov2nl_302cn, kov2nl, pgm2,        pgm2, pgm2_state, init_kov2nl,   ROT0, "IGS (Huatong license)", "Sanguo Zhan Ji 2 Gaishi Yingxiong (V302, China)", MACHINE_SUPPORTS_SAVE )
GAME( 2008, kov2nl_301cn, kov2nl, pgm2,        pgm2, pgm2_state, init_kov2nl,   ROT0, "IGS (Huatong license)", "Sanguo Zhan Ji 2 Gaishi Yingxiong (V301, China)", MACHINE_SUPPORTS_SAVE )
GAME( 2008, kov2nl_300cn, kov2nl, pgm2,        pgm2, pgm2_state, init_kov2nl,   ROT0, "IGS (Huatong license)", "Sanguo Zhan Ji 2 Gaishi Yingxiong (V300, China)", MACHINE_SUPPORTS_SAVE )


// Dodonpachi Daioujou Tamashii - should be a V200 too
GAME( 2010, ddpdojt,      0,      pgm2_ramrom, pgm2, pgm2_state, init_ddpdojt,  ROT270, "IGS / Cave (Tong Li Animation license)", "DoDonPachi Dai-Ou-Jou Tamashii (V201, China)", MACHINE_SUPPORTS_SAVE )

// Knights of Valour 3 - should be a V103 and V101 too
//三国战纪3/Sānguó zhàn jì 3 (Simplified Chinese)
GAME( 2011, kov3,         0,      pgm2_hires,  pgm2, pgm2_state, init_kov3_104, ROT0, "IGS (Huatong license)", "Knights of Valour 3 / Sanguo Zhan Ji 3 (V104, China, Hong Kong, Taiwan)", MACHINE_SUPPORTS_SAVE )
GAME( 2011, kov3_102,     kov3,   pgm2_hires,  pgm2, pgm2_state, init_kov3_102, ROT0, "IGS (Huatong license)", "Knights of Valour 3 / Sanguo Zhan Ji 3 (V102, China, Hong Kong, Taiwan)", MACHINE_SUPPORTS_SAVE )
GAME( 2011, kov3_101,     kov3,   pgm2_hires,  pgm2, pgm2_state, init_kov3_101, ROT0, "IGS (Huatong license)", "Knights of Valour 3 / Sanguo Zhan Ji 3 (V101, China, Hong Kong, Taiwan)", MACHINE_SUPPORTS_SAVE )
GAME( 2011, kov3_100,     kov3,   pgm2_hires,  pgm2, pgm2_state, init_kov3_100, ROT0, "IGS (Huatong license)", "Knights of Valour 3 / Sanguo Zhan Ji 3 (V100, China, Hong Kong, Taiwan)", MACHINE_SUPPORTS_SAVE )

// King of Fighters '98: Ultimate Match Hero
GAME( 2009, kof98umh,     0,      pgm2_lores,  pgm2, pgm2_state, init_kof98umh, ROT0, "IGS / SNK Playmore / New Channel", "The King of Fighters '98: Ultimate Match HERO (China, V100, 09-08-23)", MACHINE_SUPPORTS_SAVE )

GAME( 2009, bubucar,      0,      pgm2,        pgm2, pgm2_state, init_bubucar,  ROT0, "IGS", "Bu Bu Car (English)", MACHINE_IS_SKELETON ) // Only the program ROM is dumped

// ジグソーワールドアリーナ/Jigsaw World Arena

// お茶犬のパズル/Ochaken no Puzzle (V101JP exists but undumped, Puzzle game of Japanese お茶犬/Ochaken franchises)
