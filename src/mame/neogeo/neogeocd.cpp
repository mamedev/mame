// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/***************************************************************************

    Neo-Geo CD hardware

    Thanks to:
        * Jan Klaassen (of the former FBA team) for much of the CDC / CDD code and system details.
        * Mirko Buffoni for a commented disassembly of the NeoCD bios rom.

    Current status:
        - NeoCDZ runs, the original NeoCD does not
           - Might think the tray is open? (check)
        - Some unknown / unhandled CD commands, code is still a bit messy
           - CDDA continues to play during loading, should stop it
        - Games using Raster Effects are broken without a kludge
           - CPU gets overloaded with IRQs from the timer callback...
        - Double Dragon doesn't load, it erases the IRQ table
           - might need better handling of the Vector Table Mapping, or better interrupts (see point above)
        - Softlist are based on an old Tosec set and should be updated to the TruRip set once we can convert CCD
          without throwing away gap data etc.

****************************************************************************/

#include "emu.h"
#include "neogeo.h"

#include "megacdcd.h"

#include "imagedev/cdromimg.h"
#include "machine/74259.h"
#include "machine/nvram.h"

#include "softlist.h"


namespace {

// was it actually released in eu / asia?
//static constexpr unsigned NEOCD_REGION_ASIA = 3; // IronClad runs with a darkened screen (MVS has the same issue)
//static constexpr unsigned NEOCD_REGION_EUROPE = 2; // ^
static constexpr unsigned NEOCD_REGION_US = 1;
static constexpr unsigned NEOCD_REGION_JAPAN = 0;


class ngcd_state : public aes_base_state
{
public:
	ngcd_state(const machine_config &mconfig, device_type type, const char *tag)
		: aes_base_state(mconfig, type, tag)
		, m_tempcdc(*this, "tempcdc")
		, m_z80_ram(*this, "z80_ram")
		, m_adpcm_ram(*this, "adpcm_ram")
	{
		m_dma_address1 = 0;
		m_dma_address2 = 0;
		m_dma_value1   = 0;
		m_dma_value2   = 0;
		m_dma_count    = 0;
		m_dma_mode = 0;
		m_irq_ack = ~0;
		m_irq_vector_ack = 0;
		m_irq_vector = 0;
		m_has_sprite_bus = true;
		m_has_text_bus = true;
		m_has_ymrom_bus = true;
		m_has_z80_bus = true;
	}

	void neocd_ntsc(machine_config &config);

	void init_neocdz();
	void init_neocdzj();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	optional_device<lc89510_temp_device> m_tempcdc;
	required_shared_ptr<uint8_t> m_z80_ram;
	required_shared_ptr<uint8_t> m_adpcm_ram;

	std::unique_ptr<uint8_t[]> m_meminternal_data;
	std::unique_ptr<uint8_t[]> m_sprite_ram;
	std::unique_ptr<uint8_t[]> m_fix_ram;

	// neoCD
	uint8_t m_system_region = 0;
	int32_t m_active_transfer_area = 0;
	int32_t m_sprite_transfer_bank = 0;
	int32_t m_adpcm_transfer_bank = 0;
	int32_t m_dma_address1;
	int32_t m_dma_address2;
	int32_t m_dma_value1;
	int32_t m_dma_value2;
	int32_t m_dma_count;
	int32_t m_dma_mode;
	int32_t m_irq_ack;
	int32_t m_irq_vector_ack;
	int32_t m_irq_vector;

	bool m_has_sprite_bus;
	bool m_has_text_bus;
	bool m_has_ymrom_bus;
	bool m_has_z80_bus;

	uint8_t m_transfer_write_enable = 0;

	bool prohibit_cdc_irq = false; // hack?

	int32_t seek_idle(int32_t cycles);

	void do_dma(address_space& curr_space);
	void set_dma_regs(offs_t offset, uint16_t data);

	uint16_t memcard_r(offs_t offset);
	void memcard_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t control_r(offs_t offset);
	void control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t transfer_r(offs_t offset);
	void transfer_w(offs_t offset, uint8_t data);

	DECLARE_INPUT_CHANGED_MEMBER(aes_jp1);

	int get_irq_vector_ack(void) { return m_irq_vector_ack; }
	void set_irq_vector_ack(int val) { m_irq_vector_ack = val; }
	int get_irq_vector(void) { return m_irq_vector; }
	void irq_update(uint8_t data);

	// from the CDC
	void interrupt_callback_type1(void);
	void interrupt_callback_type2(void);
	void interrupt_callback_type3(void);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t cdc_irq_ack();

	void neocd_audio_io_map(address_map &map) ATTR_COLD;
	void neocd_audio_map(address_map &map) ATTR_COLD;
	void neocd_main_map(address_map &map) ATTR_COLD;
	void neocd_vector_map(address_map &map) ATTR_COLD;
	void neocd_ym_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Memory card
 *
 *************************************/


/* The NeoCD has an 8kB internal memory card, instead of memcard slots like the MVS and AES */
uint16_t ngcd_state::memcard_r(offs_t offset)
{
	return m_meminternal_data[offset] | 0xff00;
}


void ngcd_state::memcard_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_meminternal_data[offset] = data;
	}
}


/*************************************
 *
 *  System control register
 *
 *************************************/

uint16_t ngcd_state::control_r(offs_t offset)
{
	uint32_t const seek_address = 0xff0000 + (offset * 2);

	switch (seek_address & 0xffff)
	{
		case 0x0016:
			return m_tempcdc->nff0016_r();

		// LC8951 registers
		case 0x0100:
			return m_tempcdc->segacd_cdc_mode_address_r();
		case 0x0102:
			return m_tempcdc->CDC_Reg_r();

		// CD mechanism communication
		case 0x0160:
			return m_tempcdc->neocd_cdd_rx_r();

		case 0x011c: // region
			return ~((0x10 | (m_system_region & 3)) << 8);
	}


//  bprintf(PRINT_NORMAL, _T("  - NGCD port 0x%06X read (word, PC: 0x%06X)\n"), seek_address, SekGetPC(-1));

	return ~0;
}


void ngcd_state::control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t const seek_address = 0xff0000 + (offset * 2);

//  bprintf(PRINT_NORMAL, _T("  - NGCD port 0x%06X -> 0x%04X (PC: 0x%06X)\n"), seek_address, data, SekGetPC(-1));
	uint8_t const byte_value = data & 0xff;

	switch (seek_address & 0xfffe)
	{
		case 0x0002:
			m_tempcdc->nff0002_set(data);
			break;

		case 0x000e:
			irq_update(data); // irqack
			break;

		case 0x0016:
			m_tempcdc->nff0016_set(byte_value);
			break;

			// DMA controller
		case 0x0060:
			if (BIT(byte_value, 6))
			{
				do_dma(space);
			}
			break;

		case 0x0064:
		case 0x0066:
		case 0x0068:
		case 0x006a:
		case 0x006c:
		case 0x006e:
		case 0x0070:
		case 0x0072:
		case 0x007e:
			set_dma_regs(seek_address & 0xfffe, data);
			break;

		// upload DMA controller program

		case 0x0080:
		case 0x0082:
		case 0x0084:
		case 0x0086:
		case 0x0088:
		case 0x008a:
		case 0x008c:
		case 0x008e:
//          bprintf(PRINT_NORMAL, _T("  - DMA controller program[%02i] -> 0x%04X (PC: 0x%06X)\n"), seek_address & 0x0F, data, SekGetPC(-1));
			break;

		// LC8951 registers
		case 0x0100:
			m_tempcdc->segacd_cdc_mode_address_w(0, byte_value, 0xffff);
			break;
		case 0x0102:
			m_tempcdc->CDC_Reg_w(byte_value);
			break;

		case 0x0104:
//          bprintf(PRINT_NORMAL, _T("  - NGCD 0xE00000 area -> 0x%02X (PC: 0x%06X)\n"), byte_value, SekGetPC(-1));
			if (ACCESSING_BITS_0_7)
			{
				m_active_transfer_area = byte_value;
			}
			break;

		case 0x0120:
//          bprintf(PRINT_NORMAL, _T("  - NGCD OBJ BUSREQ -> 1 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_sprite_bus = false;
			break;
		case 0x0122:
//          bprintf(PRINT_NORMAL, _T("  - NGCD PCM BUSREQ -> 1 (PC: 0x%06X) %x\n"), SekGetPC(-1), byte_value);
			m_has_ymrom_bus = false;
			break;
		case 0x0126:
//          bprintf(PRINT_NORMAL, _T("  - NGCD Z80 BUSREQ -> 1 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_z80_bus = false;
			machine().scheduler().synchronize();
			m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			break;
		case 0x0128:
//          bprintf(PRINT_NORMAL, _T("  - NGCD FIX BUSREQ -> 1 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_text_bus = false;
			break;
		case 0x0140:
//          bprintf(PRINT_NORMAL, _T("  - NGCD OBJ BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_sprite_bus = true;
			m_sprgen->optimize_sprite_data();
			break;
		case 0x0142:
//          bprintf(PRINT_NORMAL, _T("  - NGCD PCM BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_ymrom_bus = true;
			break;
		case 0x0146:
//          bprintf(PRINT_NORMAL, _T("  - NGCD Z80 BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_z80_bus = true;
			machine().scheduler().synchronize();
			m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			break;
		case 0x0148:
//          bprintf(PRINT_NORMAL, _T("  - NGCD FIX BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_text_bus = true;
			break;

		// CD mechanism communication
		case 0x0162:
			m_tempcdc->neocd_cdd_tx_w(byte_value);
			break;
		case 0x0164:
			m_tempcdc->NeoCDCommsControl(byte_value & 1, byte_value & 2);
			break;

		case 0x016c:
//          bprintf(PRINT_ERROR, _T("  - NGCD port 0x%06X -> 0x%02X (PC: 0x%06X)\n"), seek_address, byte_value, SekGetPC(-1));
			//MapVectorTable(!(byte_value == 0xFF));
			if (ACCESSING_BITS_0_7)
			{
				// even like this doubledr ends up mapping vectors in, then erasing them causing the loading to crash??
				// is there some way to enable write protection on the RAM vector area or is it some IRQ masking issue?
				// the games still write to the normal address for this too?
				// writes 00 / 01 / ff
				printf("MapVectorTable? %04x %04x\n",data,mem_mask);

				//m_bank_vectors->set_entry(data == 0 ? 0 : 1);
				m_use_cart_vectors = (data == 0 ? 0 : 1);
			}

//extern int32_t bRunPause;
//bRunPause = 1;
			break;

		case 0x016e:
//          bprintf(PRINT_IMPORTANT, _T("  - NGCD 0xE00000 area write access %s (0x%02X, PC: 0x%06X)\n"), byte_value ? _T("enabled") : _T("disabled"), byte_value, SekGetPC(-1));

			m_transfer_write_enable = byte_value;
			break;

		case 0x0180:
		{
			// 1 during CD access, 0 otherwise, written frequently
			//printf("reset cdc %04x %04x\n",data, mem_mask);

			if (ACCESSING_BITS_0_7)
			{
				if (data == 0x00)
				{
				// not a good idea, causes hangs
				//  m_tempcdc->NeoCDCommsReset();

					// I doubt this is correct either, but we need something to stop
					// the interrupts during gameplay and I'm not sure what...
					prohibit_cdc_irq = true;
				}
				else
				{
					prohibit_cdc_irq = false;
				}
			}
			break;
		}
		case 0x0182:
		{
		//  printf("blah %02x\n", byte_value);
			if (byte_value == 0x00)
			{
				m_ym->reset();
				m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}
			else m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

			break;
		}
		case 0x01a0:
			m_sprite_transfer_bank = (byte_value & 3) << 20;
			break;
		case 0x01a2:
			m_adpcm_transfer_bank  = (byte_value & 1) << 19;
			break;


		default:
		{
//          bprintf(PRINT_NORMAL, _T("  - NGCD port 0x%06X -> 0x%04X (PC: 0x%06X)\n"), seek_address, data, SekGetPC(-1));
		}
	}

}




/*
 *  Handling NeoCD banked RAM
 *  When the Z80 space is banked in to 0xe00000, only the low byte of each word is used
 */


uint8_t ngcd_state::transfer_r(offs_t offset)
{
	uint32_t const seek_address = (0xe00000 + offset) ^ 1;

	switch (m_active_transfer_area)
	{
		case 0: // Sprites
		{
			uint32_t const address = (m_sprite_transfer_bank + (seek_address & 0x0fffff));

			// address is swizzled a bit due to out sprite decoding
			return m_sprite_ram[address ^ ((BIT(address, 0) == BIT(address, 1)) ? 0 : 3)];
		}
		case 1:                         // ADPCM
			return m_adpcm_ram[m_adpcm_transfer_bank + ((seek_address & 0x0fffff) >> 1)];
		case 4:                         // Z80
			if ((seek_address & 0xfffff) >= 0x20000) return ~0;
			return m_z80_ram[(seek_address & 0x1ffff) >> 1];
		case 5:                         // Text
			return m_fix_ram[(seek_address & 0x3ffff) >> 1];
	}

	return ~0;

}

void ngcd_state::transfer_w(offs_t offset, uint8_t data)
{
	uint32_t const seek_address = (0xe00000 + offset) ^ 1;

	if (!m_transfer_write_enable)
	{
//      return;
	}

	switch (m_active_transfer_area)
	{
		case 0:                         // Sprites
		{
			uint32_t const address = (m_sprite_transfer_bank + (seek_address & 0x0fffff));

			// address is swizzled a bit due to out sprite decoding
			m_sprite_ram[address ^ ((BIT(address, 0) == BIT(address, 1)) ? 0 : 3)] = data;
			break;
		}
		case 1:                         // ADPCM
			m_adpcm_ram[m_adpcm_transfer_bank + ((seek_address & 0x0fffff) >> 1)] = data;
			break;
		case 4:                         // Z80

			// kof98 and lresort attempt to write here when the system still has the z80 bank
			// it seems they attempt to write regular samples (not even deltat) maybe there is
			// some kind of fall-through behavior, or it shouldn't be allowed to select a
			// transfer area without the bus? - this should really be checked on hw
			if (m_has_z80_bus)
			{
				m_adpcm_ram[m_adpcm_transfer_bank + ((seek_address & 0x0fffff) >> 1)] = data;
			}
			else
			{
		//  printf("seek_address %08x %02x\n", seek_address, data);
				if ((seek_address & 0xfffff) >= 0x20000) break;
				m_z80_ram[(seek_address & 0x1ffff) >> 1] = data;
			}
			break;
		case 5:                         // Text
			m_fix_ram[(seek_address & 0x3ffff) >> 1] = data;
			break;
	}
}



void ngcd_state::set_dma_regs(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x0064:
			m_dma_address1 &= 0x0000ffff;
			m_dma_address1 |= data << 16;
			break;
		case 0x0066:
			m_dma_address1 &= 0xffff0000;
			m_dma_address1 |= data;
			break;
		case 0x0068:
			m_dma_address2 &= 0x0000ffff;
			m_dma_address2 |= data << 16;
			break;
		case 0x006a:
			m_dma_address2 &= 0xffff0000;
			m_dma_address2 |= data;
			break;
		case 0x006c:
			m_dma_value1 = data;
			break;
		case 0x006e:
			m_dma_value2 = data;
			break;
		case 0x0070:
			m_dma_count &= 0x0000ffff;
			m_dma_count |= data << 16;
			break;
		case 0x0072:
			m_dma_count &= 0xffff0000;
			m_dma_count |= data;
			break;

		case 0x007e:
			m_dma_mode = data;
//          bprintf(PRINT_NORMAL, _T("  - DMA controller 0x%2X -> 0x%04X (PC: 0x%06X)\n"), seek_address & 0xFF, data, SekGetPC(-1));
			break;
	}
}



int32_t ngcd_state::seek_idle(int32_t cycles)
{
	return cycles;
}



/*
 *  CD-ROM / DMA control
 *
 *  DMA

    FF0061  Write 0x40 means start DMA transfer
    FF0064  Source address (in copy mode), Target address (in fill mode)
    FF0068  Target address (in copy mode)
    FF006C  Fill word
    FF0070  Words count
    FF007E  \
    ......   | DMA programming words?   NeoGeoCD uses Sanyo Puppet LC8359 chip to
    FF008E  /                           interface with CD, and do DMA transfers

    Memory access control

    FF011C  DIP SWITCH (Region code)
    FF0105  Area Selector (5 = FIX, 0 = SPR, 4 = Z80, 1 = PCM)
    FF01A1  Sprite bank selector
    FF01A3  PCM bank selector
    FF0120  Prepare sprite area for transfer
    FF0122  Prepare PCM area for transfer
    FF0126  Prepare Z80 area for transfer
    FF0128  Prepare Fix area for transfer
    FF0140  Terminate work on Spr Area  (Sprites must be decoded here)
    FF0142  Terminate work on Pcm Area
    FF0146  Terminate work on Z80 Area  (Z80 needs to be reset)
    FF0148  Terminate work on Fix Area

    CD-ROM:
    0xff0102 == 0xF0 start cd transfer
    int m=bcd(fast_r8(0x10f6c8));
    int s=bcd(fast_r8(0x10f6c9));
    int f=bcd(fast_r8(0x10f6ca));
    int seccount=fast_r16(0x10f688);

    inisec=((m*60)+s)*75+f;
    inisec-=150;
    dstaddr=0x111204; // this must come from somewhere

    the value @ 0x10f688 is decremented each time a sector is read until it's 0.

 *
 */


void ngcd_state::do_dma(address_space& curr_space)
{
	// The LC8953 chip has a programmable DMA controller, which is not properly emulated.
	// Since the software only uses it in a limited way, we can apply a simple heuristic
	// to determine the requested operation.

	// Additionally, we don't know how many cycles DMA operations take.
	// Here, only bus access is used to get a rough approximation --
	// each read/write takes a single cycle, setup and everything else is ignored.

//  bprintf(PRINT_IMPORTANT, _T("  - DMA controller transfer started (PC: 0x%06X)\n"), SekGetPC(-1));

	switch (m_dma_mode)
	{
		case 0xcffd:
		{
//          bprintf(PRINT_NORMAL, _T("    adr : 0x%08X - 0x%08X <- address, skip odd bytes\n"), m_dma_address1, m_dma_address1 + m_dma_count * 8);

			//  - DMA controller 0x7E -> 0xCFFD (PC: 0xC07CE2)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC07CE8)
			//  - DMA controller program[02] -> 0xE8DA (PC: 0xC07CEE)
			//  - DMA controller program[04] -> 0x92DA (PC: 0xC07CF4)
			//  - DMA controller program[06] -> 0x92DB (PC: 0xC07CFA)
			//  - DMA controller program[08] -> 0x96DB (PC: 0xC07D00)
			//  - DMA controller program[10] -> 0x96F6 (PC: 0xC07D06)
			//  - DMA controller program[12] -> 0x2E02 (PC: 0xC07D0C)
			//  - DMA controller program[14] -> 0xFDFF (PC: 0xC07D12)

			seek_idle(m_dma_count * 4);

			while (m_dma_count--)
			{
				curr_space.write_word(m_dma_address1 + 0, m_dma_address1 >> 24);
				curr_space.write_word(m_dma_address1 + 2, m_dma_address1 >> 16);
				curr_space.write_word(m_dma_address1 + 4, m_dma_address1 >>  8);
				curr_space.write_word(m_dma_address1 + 6, m_dma_address1 >>  0);
				m_dma_address1 += 8;
			}

			break;
		}

		case 0xe2dd:
		{
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- 0x%08X - 0x%08X, skip odd bytes\n"), m_dma_address2, m_dma_address2 + m_dma_count * 2, m_dma_address1, m_dma_address1 + m_dma_count * 4);

			//  - DMA controller 0x7E -> 0xE2DD (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x82BE (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x93DA (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xBE93 (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0xDABE (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xF62D (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x02FD (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xFFFF (PC: 0xC0A1A0)

			seek_idle(m_dma_count * 1);

			while (m_dma_count--)
			{
				curr_space.write_word(m_dma_address2 + 0, curr_space.read_byte(m_dma_address1 + 0));
				curr_space.write_word(m_dma_address2 + 2, curr_space.read_byte(m_dma_address1 + 1));
				m_dma_address1 += 2;
				m_dma_address2 += 4;
			}

			break;
		}

		case 0xfc2d:
		{
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- LC8951 external buffer, skip odd bytes\n"), m_dma_address1, m_dma_address1 + m_dma_count * 4);

			//  - DMA controller 0x7E -> 0xFC2D (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x8492 (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0xDA92 (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xDAF6 (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0x2A02 (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xFDFF (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x48E7 (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xFFFE (PC: 0xC0A1A0)

			char* data = m_tempcdc->LC8915InitTransfer(m_dma_count);
			if (data == nullptr)
			{
				break;
			}

			seek_idle(m_dma_count * 4);

			while (m_dma_count--)
			{
				curr_space.write_byte(m_dma_address1 + 0, data[0]);
				curr_space.write_byte(m_dma_address1 + 2, data[1]);
				m_dma_address1 += 4;
				data += 2;
			}

			m_tempcdc->LC8915EndTransfer();

			break;
		}

		case 0xfe3d:

			//  - DMA controller 0x7E -> 0xFE3D (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x82BF (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x93BF (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xF629 (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0x02FD (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xFFFF (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0xF17D (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xFCF5 (PC: 0xC0A1A0)

		case 0xfe6d:
		{
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- 0x%08X - 0x%08X\n"), m_dma_address2, m_dma_address2 + m_dma_count * 2, m_dma_address1, m_dma_address1 + m_dma_count * 2);

			//  - DMA controller 0x7E -> 0xFE6D (PC: 0xC0FD7A)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0FD7C)
			//  - DMA controller program[02] -> 0x82BF (PC: 0xC0FD7E)
			//  - DMA controller program[04] -> 0xF693 (PC: 0xC0FD80)
			//  - DMA controller program[06] -> 0xBF29 (PC: 0xC0FD82)
			//  - DMA controller program[08] -> 0x02FD (PC: 0xC0FD84)
			//  - DMA controller program[10] -> 0xFFFF (PC: 0xC0FD86)
			//  - DMA controller program[12] -> 0xC515 (PC: 0xC0FD88)
			//  - DMA controller program[14] -> 0xFCF5 (PC: 0xC0FD8A)

			seek_idle(m_dma_count * 1);

			while (m_dma_count--)
			{
				curr_space.write_word(m_dma_address2, curr_space.read_word(m_dma_address1));
				m_dma_address1 += 2;
				m_dma_address2 += 2;
			}

			if (m_dma_address2 == 0x0800)
			{
			// MapVectorTable(false);
			//  bprintf(PRINT_ERROR, _T("    RAM vectors mapped (PC = 0x%08X\n"), SekGetPC(0));
			//  extern int32_t bRunPause;
			//  bRunPause = 1;
			}
			break;
		}

		case 0xfef5:
		{
//          bprintf(PRINT_NORMAL, _T("    adr : 0x%08X - 0x%08X <- address\n"), m_dma_address1, m_dma_address1 + m_dma_count * 4);

			//  - DMA controller 0x7E -> 0xFEF5 (PC: 0xC07CE2)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC07CE8)
			//  - DMA controller program[02] -> 0x92E8 (PC: 0xC07CEE)
			//  - DMA controller program[04] -> 0xBE96 (PC: 0xC07CF4)
			//  - DMA controller program[06] -> 0xF629 (PC: 0xC07CFA)
			//  - DMA controller program[08] -> 0x02FD (PC: 0xC07D00)
			//  - DMA controller program[10] -> 0xFFFF (PC: 0xC07D06)
			//  - DMA controller program[12] -> 0xFC3D (PC: 0xC07D0C)
			//  - DMA controller program[14] -> 0xFCF5 (PC: 0xC07D12)

			seek_idle(m_dma_count * 2);

			while (m_dma_count--)
			{
				curr_space.write_word(m_dma_address1 + 0, m_dma_address1 >> 16);
				curr_space.write_word(m_dma_address1 + 2, m_dma_address1 >>  0);
				m_dma_address1 += 4;
			}

			break;
		}

		case 0xffc5:
		{
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- LC8951 external buffer\n"), m_dma_address1, m_dma_address1 + m_dma_count * 2);

			//  - DMA controller 0x7E -> 0xFFC5 (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0xA6F6 (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x2602 (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xFDFF (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0xFC2D (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xFCF5 (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x8492 (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xDA92 (PC: 0xC0A1A0)

			char* data = m_tempcdc->LC8915InitTransfer(m_dma_count);
			if (data == nullptr)
			{
				break;
			}

			seek_idle(m_dma_count * 4);

			while (m_dma_count--)
			{
				curr_space.write_byte(m_dma_address1 + 0, data[0]);
				curr_space.write_byte(m_dma_address1 + 1, data[1]);
				m_dma_address1 += 2;
				data += 2;
			}

			m_tempcdc->LC8915EndTransfer();

			break;
		}

		case 0xffcd:

			//  - DMA controller 0x7E -> 0xFFCD (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x92F6 (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x2602 (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xFDFF (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0x7006 (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0x6100 (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x2412 (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0x13FC (PC: 0xC0A1A0)

		case 0xffdd:
		{
//          bprintf(PRINT_NORMAL, _T("    Fill: 0x%08X - 0x%08X <- 0x%04X\n"), m_dma_address1, m_dma_address1 + m_dma_count * 2, m_dma_value1);

			//  - DMA controller 0x7E -> 0xFFDD (PC: 0xC07CE2)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC07CE8)
			//  - DMA controller program[02] -> 0x92F6 (PC: 0xC07CEE)
			//  - DMA controller program[04] -> 0x2602 (PC: 0xC07CF4)
			//  - DMA controller program[06] -> 0xFDFF (PC: 0xC07CFA)
			//  - DMA controller program[08] -> 0xFFFF (PC: 0xC07D00)
			//  - DMA controller program[10] -> 0xFCF5 (PC: 0xC07D06)
			//  - DMA controller program[12] -> 0x8AF0 (PC: 0xC07D0C)
			//  - DMA controller program[14] -> 0x1609 (PC: 0xC07D12)

			seek_idle(m_dma_count * 1);

			while (m_dma_count--)
			{
				curr_space.write_word(m_dma_address1, m_dma_value1);
				m_dma_address1 += 2;
			}

			break;
		}
		default:
		{
			//bprintf(PRINT_ERROR, _T("    Unknown transfer type 0x%04X (PC: 0x%06X)\n"), m_dma_mode, SekGetPC(-1));
			//bprintf(PRINT_NORMAL, _T("    ??? : 0x%08X  0x%08X 0x%04X 0x%04X 0x%08X\n"), m_dma_address1, m_dma_address2, m_dma_value1, m_dma_value2, m_dma_count);

			//extern int32_t bRunPause;
			//bRunPause = 1;

		}
	}
}


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void ngcd_state::machine_start()
{
	aes_base_state::machine_start();

	// set curr_slot to 0, so to allow checking m_slots[m_curr_slot] != nullptr
	m_curr_slot = 0;

	// initialize sprite to point to memory regions
	m_sprite_ram = make_unique_clear<uint8_t[]>(0x400000);
	m_fix_ram = make_unique_clear<uint8_t[]>(0x20000);
	save_pointer(NAME(m_sprite_ram), 0x400000);
	save_pointer(NAME(m_fix_ram), 0x20000);

	m_sprgen->set_sprite_region(m_sprite_ram.get(), 0x400000);
	m_sprgen->set_fixed_regions(m_fix_ram.get(), 0x20000, nullptr);
	m_sprgen->set_fixed_layer_source(1);

	m_vblank_level = 1;
	m_raster_level = 3;

	// initialize the memcard data structure
	// NeoCD doesn't have memcard slots, rather, it has a larger internal memory which works the same
	m_meminternal_data = make_unique_clear<uint8_t[]>(0x2000);
	subdevice<nvram_device>("saveram")->set_base(m_meminternal_data.get(), 0x2000);
	save_pointer(NAME(m_meminternal_data), 0x2000);

	m_tempcdc->reset_cd();

	save_item(NAME(m_active_transfer_area));
	save_item(NAME(m_sprite_transfer_bank));
	save_item(NAME(m_adpcm_transfer_bank));
	save_item(NAME(m_dma_address1));
	save_item(NAME(m_dma_address2));
	save_item(NAME(m_dma_value1));
	save_item(NAME(m_dma_value2));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_irq_ack));
	save_item(NAME(m_irq_vector_ack));
	save_item(NAME(m_irq_vector));

	save_item(NAME(m_has_sprite_bus));
	save_item(NAME(m_has_text_bus));
	save_item(NAME(m_has_ymrom_bus));
	save_item(NAME(m_has_z80_bus));

	save_item(NAME(m_transfer_write_enable));

	save_item(NAME(prohibit_cdc_irq));
}


/*************************************
 *
 *  Machine reset
 *
 *************************************/

void ngcd_state::machine_reset()
{
	aes_base_state::machine_reset();

	m_tempcdc->NeoCDCommsReset();

	m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_transfer_write_enable = 0;
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void ngcd_state::neocd_main_map(address_map &map)
{
	aes_base_main_map(map);

	map(0x000000, 0x1fffff).ram().share("maincpu");
	map(0x000000, 0x00007f).r(FUNC(ngcd_state::banked_vectors_r)); // writes will fall through to area above

	map(0x800000, 0x803fff).rw(FUNC(ngcd_state::memcard_r), FUNC(ngcd_state::memcard_w));
	map(0xc00000, 0xc7ffff).mirror(0x080000).rom().region("mainbios", 0);
	map(0xd00000, 0xdfffff).r(FUNC(ngcd_state::unmapped_r));
	map(0xe00000, 0xefffff).rw(FUNC(ngcd_state::transfer_r), FUNC(ngcd_state::transfer_w));
	map(0xf00000, 0xfeffff).r(FUNC(ngcd_state::unmapped_r));
	map(0xff0000, 0xff01ff).rw(FUNC(ngcd_state::control_r), FUNC(ngcd_state::control_w)); // CDROM / DMA
	map(0xff0200, 0xffffff).r(FUNC(ngcd_state::unmapped_r));
}

void ngcd_state::neocd_vector_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));

	map(0xfffff3, 0xfffff3).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffff5, 0xfffff5).r(FUNC(ngcd_state::cdc_irq_ack));
	map(0xfffff7, 0xfffff7).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
}


/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/


void ngcd_state::neocd_audio_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_z80_ram);
}


void ngcd_state::neocd_audio_io_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::clear_w));
	map(0x04, 0x07).mirror(0xff00).rw(m_ym, FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x08, 0x08).mirror(0xff00).select(0x0010).w(FUNC(ngcd_state::audio_cpu_enable_nmi_w));
	// banking reads are actually NOP on NeoCD? but some games still access them
//  map(0x08, 0x0b).mirror(0x00f0).select(0xff00).r(FUNC(ngcd_state::audio_cpu_bank_select_r));
	map(0x0c, 0x0c).mirror(0xff00).w(m_soundlatch2, FUNC(generic_latch_8_device::write));

	// ??
	map(0x80, 0x80).mirror(0xff00).nopw();
	map(0xc0, 0xc0).mirror(0xff00).nopw();
	map(0xc1, 0xc1).mirror(0xff00).nopw();
}

void ngcd_state::neocd_ym_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share(m_adpcm_ram);
}


/*************************************
 *
 *  Input port definitions
 *
 *************************************/

static INPUT_PORTS_START( neocd )
	PORT_INCLUDE( aes )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_UNUSED ) // the NeoCD memcard is internal
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/


/* NeoCD uses custom vectors on IRQ4 to handle various events from the CDC */

uint8_t ngcd_state::cdc_irq_ack()
{
	if (get_irq_vector_ack())
	{
		if (!machine().side_effects_disabled())
			set_irq_vector_ack(0);
		return get_irq_vector();
	}

	return (0x60+4*4)/4;
}

void ngcd_state::interrupt_callback_type1(void)
{
	m_irq_ack &= ~0x20;
	irq_update(0);
}

void ngcd_state::interrupt_callback_type2(void)
{
	m_irq_ack &= ~0x10;
	irq_update(0);
}

void ngcd_state::interrupt_callback_type3(void)
{
	m_irq_ack &= ~0x08;
	irq_update(0);
}


void ngcd_state::irq_update(uint8_t data)
{
	// do we also need to check the regular interrupts like FBA?

	m_irq_ack |= (data & 0x38);

	if (!prohibit_cdc_irq)
	{
		if ((m_irq_ack & 0x08) == 0)
		{
			m_irq_vector = 0x17;
			m_irq_vector_ack = 1;
			m_maincpu->set_input_line(2, HOLD_LINE);
			return;
		}
		if ((m_irq_ack & 0x10) == 0)
		{
			m_irq_vector = 0x16;
			m_irq_vector_ack = 1;
			m_maincpu->set_input_line(2, HOLD_LINE);
			return;
		}
		if ((m_irq_ack & 0x20) == 0)
		{
			m_irq_vector = 0x15;
			m_irq_vector_ack = 1;
			m_maincpu->set_input_line(2, HOLD_LINE);
			return;
		}
	}
}


uint32_t ngcd_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// fill with background color first
	bitmap.fill(*m_bg_pen, cliprect);

	if (m_has_sprite_bus) m_sprgen->draw_sprites(bitmap, cliprect.min_y);

	if (m_has_text_bus) m_sprgen->draw_fixed_layer(bitmap, cliprect.min_y);

	return 0;
}

// NTSC region
void ngcd_state::neocd_ntsc(machine_config &config)
{
	neogeo_base(config);
	neogeo_stereo(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &ngcd_state::neocd_main_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &ngcd_state::neocd_vector_map);

	m_audiocpu->set_addrmap(AS_PROGRAM, &ngcd_state::neocd_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &ngcd_state::neocd_audio_io_map);

	// what IS going on with "neocdz doubledr" and why do games write here if it's hooked up to nothing?
	subdevice<hc259_device>("systemlatch")->q_out_cb<1>().set([this](int state) { logerror("%s NeoCD: write %d to regular vector change address?\n", machine().describe_context(), state); });

	m_screen->set_screen_update(FUNC(ngcd_state::screen_update));

	// temporary until things are cleaned up
	LC89510_TEMP(config, m_tempcdc, 0); // cd controller
	m_tempcdc->set_cdrom_tag("cdrom");
	m_tempcdc->set_is_neoCD(true);
	m_tempcdc->set_type1_interrupt_callback(FUNC(ngcd_state::interrupt_callback_type1));
	m_tempcdc->set_type2_interrupt_callback(FUNC(ngcd_state::interrupt_callback_type2));
	m_tempcdc->set_type3_interrupt_callback(FUNC(ngcd_state::interrupt_callback_type3));

	NVRAM(config, "saveram", nvram_device::DEFAULT_ALL_0);

	NEOGEO_CONTROL_PORT(config, m_ctrl1, neogeo_controls, "joy", false);
	NEOGEO_CONTROL_PORT(config, m_ctrl2, neogeo_controls, "joy", false);

	CDROM(config, "cdrom").set_interface("cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("neocd");

	m_ym->set_addrmap(0, &ngcd_state::neocd_ym_map);
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

ROM_START( neocd )
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_SYSTEM_BIOS( 0, "top",   "Top loading Neo-Geo CD" )
	ROMX_LOAD("top-sp1.bin",    0x00000, 0x80000, CRC(c36a47c0) SHA1(235f4d1d74364415910f73c10ae5482d90b4274f), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "front",   "Front loading Neo-Geo CD" )
	ROMX_LOAD("front-sp1.bin",    0x00000, 0x80000, CRC(cac62307) SHA1(53bc1f283cdf00fa2efbb79f2e36d4c8038d743a), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "unibios32", "Universe BIOS (Hack, Ver. 3.2)" )
	ROMX_LOAD("uni-bioscd32.rom",    0x00000, 0x80000, CRC(0ffb3127) SHA1(5158b728e62b391fb69493743dcf7abbc62abc82), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "unibios33", "Universe BIOS (Hack, Ver. 3.3)" )
	ROMX_LOAD("uni-bioscd33.rom",    0x00000, 0x80000, CRC(ff3abc59) SHA1(5142f205912869b673a71480c5828b1eaed782a8), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(3))

	ROM_REGION( 0x20000, "spritegen:zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
ROM_END

ROM_START( neocdz )
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_SYSTEM_BIOS( 0, "official",   "Official BIOS" )
	ROMX_LOAD("neocd.bin",    0x00000, 0x80000, CRC(df9de490) SHA1(7bb26d1e5d1e930515219cb18bcde5b7b23e2eda), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "unibios32", "Universe BIOS (Hack, Ver. 3.2)" )
	ROMX_LOAD("uni-bioscd32.rom",    0x00000, 0x80000, CRC(0ffb3127) SHA1(5158b728e62b391fb69493743dcf7abbc62abc82), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "unibios33", "Universe BIOS (Hack, Ver. 3.3)" )
	ROMX_LOAD("uni-bioscd33.rom",    0x00000, 0x80000, CRC(ff3abc59) SHA1(5142f205912869b673a71480c5828b1eaed782a8), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2))

	ROM_REGION( 0x20000, "spritegen:zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
ROM_END

#define rom_neocdzj    rom_neocdz

void ngcd_state::init_neocdz()
{
	m_system_region = NEOCD_REGION_US;
}

void ngcd_state::init_neocdzj()
{
	m_system_region = NEOCD_REGION_JAPAN;
}

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   CLASS        INIT          COMPANY FULLNAME               FLAGS */
CONS( 1996, neocdz,  0,      0,      neocd_ntsc, neocd,  ngcd_state,  init_neocdz,  "SNK",  "Neo-Geo CDZ (US)",    0 ) // the CDZ is the newer model
CONS( 1996, neocdzj, neocdz, 0,      neocd_ntsc, neocd,  ngcd_state,  init_neocdzj, "SNK",  "Neo-Geo CDZ (Japan)", 0 )

// NTSC region?
CONS( 1994, neocd,   neocdz, 0,      neocd_ntsc, neocd,  ngcd_state,  empty_init,   "SNK",  "Neo-Geo CD (NTSC?)",  MACHINE_NOT_WORKING ) // older  model, ignores disc protections?
