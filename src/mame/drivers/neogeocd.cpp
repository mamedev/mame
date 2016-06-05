// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/***************************************************************************

    Neo-Geo CD hardware

    Thanks to:
        * The FBA team (Barry Harris) for much of the CDC / CDD code and system details.
          ( http://www.barryharris.me.uk/ )
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
#include "includes/neogeo.h"
#include "machine/nvram.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"
#include "machine/megacdcd.h"
#include "softlist.h"


/* Stubs for various functions called by the FBA code, replace with MAME specifics later */

UINT8 *NeoSpriteRAM, *NeoTextRAM;
//UINT8* NeoSpriteROM;
//UINT8* NeoTextROM;
UINT8* YM2610ADPCMAROM;
UINT8* NeoZ80ROMActive;

// was it actually released in eu / asia?
#define NEOCD_REGION_ASIA 3 // IronClad runs with a darkened screen (MVS has the same issue)
#define NEOCD_REGION_EUROPE 2 // ^
#define NEOCD_REGION_US 1
#define NEOCD_REGION_JAPAN 0


UINT8 NeoSystem = NEOCD_REGION_JAPAN;


class ngcd_state : public aes_state
{
public:
	ngcd_state(const machine_config &mconfig, device_type type, const char *tag)
		: aes_state(mconfig, type, tag)
		, m_tempcdc(*this,"tempcdc")
	{
		NeoCDDMAAddress1 = 0;
		NeoCDDMAAddress2 = 0;
		NeoCDDMAValue1   = 0;
		NeoCDDMAValue2   = 0;
		NeoCDDMACount    = 0;
		NeoCDDMAMode = 0;
		nIRQAcknowledge = ~0;
		nNeoCDIRQVectorAck = 0;
		nNeoCDIRQVector = 0;
		m_has_sprite_bus = true;
		m_has_text_bus = true;
		m_has_ymrom_bus = true;
		m_has_z80_bus = true;
	}

	optional_device<lc89510_temp_device> m_tempcdc;


	void NeoCDDoDMA(address_space& curr_space);
	void set_DMA_regs(int offset, UINT16 wordValue);

	DECLARE_READ16_MEMBER(neocd_memcard_r);
	DECLARE_WRITE16_MEMBER(neocd_memcard_w);
	DECLARE_READ16_MEMBER(neocd_control_r);
	DECLARE_WRITE16_MEMBER(neocd_control_w);
	DECLARE_READ8_MEMBER(neocd_transfer_r);
	DECLARE_WRITE8_MEMBER(neocd_transfer_w);

	DECLARE_INPUT_CHANGED_MEMBER(aes_jp1);

	DECLARE_MACHINE_START(neocd);
	DECLARE_MACHINE_RESET(neocd);

	// neoCD

	INT32 nActiveTransferArea;
	INT32 nSpriteTransferBank;
	INT32 nADPCMTransferBank;
	INT32 NeoCDDMAAddress1;
	INT32 NeoCDDMAAddress2;
	INT32 NeoCDDMAValue1;
	INT32 NeoCDDMAValue2;
	INT32 NeoCDDMACount;
	INT32 NeoCDDMAMode;
	INT32 nIRQAcknowledge;
	int nNeoCDIRQVectorAck;
	int nNeoCDIRQVector;

	bool m_has_sprite_bus;
	bool m_has_text_bus;
	bool m_has_ymrom_bus;
	bool m_has_z80_bus;

	int get_nNeoCDIRQVectorAck(void) { return nNeoCDIRQVectorAck; }
	void set_nNeoCDIRQVectorAck(int val) { nNeoCDIRQVectorAck = val; }
	int get_nNeoCDIRQVector(void) { return nNeoCDIRQVector; }
	void NeoCDIRQUpdate(UINT8 byteValue);

	// from the CDC
	void interrupt_callback_type1(void);
	void interrupt_callback_type2(void);
	void interrupt_callback_type3(void);

	UINT8 nTransferWriteEnable;

	bool prohibit_cdc_irq; // hack?

	UINT32 screen_update_neocd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DRIVER_INIT(neocdz);
	DECLARE_DRIVER_INIT(neocdzj);

	IRQ_CALLBACK_MEMBER(neocd_int_callback);

	std::unique_ptr<UINT8[]> m_meminternal_data;
protected:

	INT32 SekIdle(INT32 nCycles);
};



/*************************************
 *
 *  Memory card
 *
 *************************************/

#define MEMCARD_SIZE    0x0800


/* The NeoCD has an 8kB internal memory card, instead of memcard slots like the MVS and AES */
READ16_MEMBER(ngcd_state::neocd_memcard_r)
{
	return m_meminternal_data[offset] | 0xff00;
}


WRITE16_MEMBER(ngcd_state::neocd_memcard_w)
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

READ16_MEMBER(ngcd_state::neocd_control_r)
{
	UINT32 sekAddress = 0xff0000+ (offset*2);

	switch (sekAddress & 0xFFFF) {
		case 0x0016:
			return m_tempcdc->nff0016_r();

		// LC8951 registers
		case 0x0100:
			return m_tempcdc->segacd_cdc_mode_address_r(space, 0, 0xffff);
		case 0x0102:
			return m_tempcdc->CDC_Reg_r();

		// CD mechanism communication
		case 0x0160:
			return m_tempcdc->neocd_cdd_rx_r();

		case 0x011C: // region
			return ~((0x10 | (NeoSystem & 3)) << 8);
	}


//  bprintf(PRINT_NORMAL, _T("  - NGCD port 0x%06X read (word, PC: 0x%06X)\n"), sekAddress, SekGetPC(-1));

	return ~0;
}


WRITE16_MEMBER(ngcd_state::neocd_control_w)
{
	UINT32 sekAddress = 0xff0000+ (offset*2);
	UINT16 wordValue = data;

//  bprintf(PRINT_NORMAL, _T("  - NGCD port 0x%06X -> 0x%04X (PC: 0x%06X)\n"), sekAddress, wordValue, SekGetPC(-1));
	int byteValue = wordValue & 0xff;

	switch (sekAddress & 0xFFFE) {
		case 0x0002:

			m_tempcdc->nff0002_set(wordValue);

			break;

		case 0x000E:
			NeoCDIRQUpdate(wordValue); // irqack
			break;

		case 0x0016:
			m_tempcdc->nff0016_set(byteValue);
			break;

			// DMA controller
		case 0x0060:
			if (byteValue & 0x40) {
				NeoCDDoDMA(space);
			}
			break;

		case 0x0064:
		case 0x0066:
		case 0x0068:
		case 0x006A:
		case 0x006C:
		case 0x006E:
		case 0x0070:
		case 0x0072:
		case 0x007E:
			set_DMA_regs(sekAddress & 0xFFFE, wordValue);
			break;

		// upload DMA controller program

		case 0x0080:
		case 0x0082:
		case 0x0084:
		case 0x0086:
		case 0x0088:
		case 0x008A:
		case 0x008C:
		case 0x008E:
//          bprintf(PRINT_NORMAL, _T("  - DMA controller program[%02i] -> 0x%04X (PC: 0x%06X)\n"), sekAddress & 0x0F, wordValue, SekGetPC(-1));
			break;

		// LC8951 registers
		case 0x0100:
			m_tempcdc->segacd_cdc_mode_address_w(space, 0, byteValue, 0xffff);
			break;
		case 0x0102:
			m_tempcdc->CDC_Reg_w(byteValue);
			break;

		case 0x0104:
//          bprintf(PRINT_NORMAL, _T("  - NGCD 0xE00000 area -> 0x%02X (PC: 0x%06X)\n"), byteValue, SekGetPC(-1));
			if (ACCESSING_BITS_0_7)
			{
				nActiveTransferArea = byteValue;
			}
			break;

		case 0x0120:
//          bprintf(PRINT_NORMAL, _T("  - NGCD OBJ BUSREQ -> 1 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_sprite_bus = false;
			break;
		case 0x0122:
//          bprintf(PRINT_NORMAL, _T("  - NGCD PCM BUSREQ -> 1 (PC: 0x%06X) %x\n"), SekGetPC(-1), byteValue);
			m_has_ymrom_bus = false;
			break;
		case 0x0126:
//          bprintf(PRINT_NORMAL, _T("  - NGCD Z80 BUSREQ -> 1 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_z80_bus = false;
			space.machine().scheduler().synchronize();
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
			space.machine().scheduler().synchronize();
			m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			break;
		case 0x0148:
//          bprintf(PRINT_NORMAL, _T("  - NGCD FIX BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_text_bus = true;
			break;

		// CD mechanism communication
		case 0x0162:
			m_tempcdc->neocd_cdd_tx_w(byteValue);
			break;
		case 0x0164:
			m_tempcdc->NeoCDCommsControl(byteValue & 1, byteValue & 2);
			break;

		case 0x016c:
//          bprintf(PRINT_ERROR, _T("  - NGCD port 0x%06X -> 0x%02X (PC: 0x%06X)\n"), sekAddress, byteValue, SekGetPC(-1));
			//MapVectorTable(!(byteValue == 0xFF));
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

//extern INT32 bRunPause;
//bRunPause = 1;
			break;

		case 0x016e:
//          bprintf(PRINT_IMPORTANT, _T("  - NGCD 0xE00000 area write access %s (0x%02X, PC: 0x%06X)\n"), byteValue ? _T("enabled") : _T("disabled"), byteValue, SekGetPC(-1));

			nTransferWriteEnable = byteValue;
			break;

		case 0x0180: {
			// 1 during CD access, 0 otherwise, written frequently
			//printf("reset cdc %04x %04x\n",data, mem_mask);

			if (ACCESSING_BITS_0_7)
			{
				if (data==0x00)
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
		case 0x0182: {
		//  printf("blah %02x\n", byteValue);
			if (byteValue == 0x00)
			{
				machine().device("ymsnd")->reset();
				m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}
			else m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

			break;
		}
		case 0x01A0:
			nSpriteTransferBank = (byteValue & 3) << 20;
			break;
		case 0x01A2:
			nADPCMTransferBank  = (byteValue & 1) << 19;
			break;


		default: {
//          bprintf(PRINT_NORMAL, _T("  - NGCD port 0x%06X -> 0x%04X (PC: 0x%06X)\n"), sekAddress, wordValue, SekGetPC(-1));
		}
	}

}




/*
 *  Handling NeoCD banked RAM
 *  When the Z80 space is banked in to 0xe00000, only the low byte of each word is used
 */


READ8_MEMBER(ngcd_state::neocd_transfer_r)
{
	UINT32 sekAddress = 0xe00000+ (offset);
	int address;
	sekAddress ^= 1;

	switch (nActiveTransferArea) {
		case 0: // Sprites
			address = (nSpriteTransferBank + (sekAddress & 0x0FFFFF));

			// address is swizzled a bit due to out sprite decoding
			if ((address&3)==0) return NeoSpriteRAM[address];
			if ((address&3)==1) return NeoSpriteRAM[address^3];
			if ((address&3)==2) return NeoSpriteRAM[address^3];
			if ((address&3)==3) return NeoSpriteRAM[address];

			return NeoSpriteRAM[nSpriteTransferBank + (sekAddress & 0x0FFFFF)];
		case 1:                         // ADPCM
			return YM2610ADPCMAROM[nADPCMTransferBank + ((sekAddress & 0x0FFFFF) >> 1)];
		case 4:                         // Z80
			if ((sekAddress & 0xfffff) >= 0x20000) return ~0;
			return NeoZ80ROMActive[(sekAddress & 0x1FFFF) >> 1];
		case 5:                         // Text
			return NeoTextRAM[(sekAddress & 0x3FFFF) >> 1];
	}

	return ~0;

}

WRITE8_MEMBER(ngcd_state::neocd_transfer_w)
{
	UINT8 byteValue = data;
	UINT32 sekAddress = 0xe00000+ (offset);

	if (!nTransferWriteEnable) {
//      return;
	}
	int address;

	sekAddress ^= 1;

	switch (nActiveTransferArea) {
		case 0:                         // Sprites
			address = (nSpriteTransferBank + (sekAddress & 0x0FFFFF));

			// address is swizzled a bit due to out sprite decoding
			if ((address&3)==0) NeoSpriteRAM[address] = byteValue;
			if ((address&3)==1) NeoSpriteRAM[address^3] = byteValue;
			if ((address&3)==2) NeoSpriteRAM[address^3] = byteValue;
			if ((address&3)==3) NeoSpriteRAM[address] = byteValue;

			break;
		case 1:                         // ADPCM
			YM2610ADPCMAROM[nADPCMTransferBank + ((sekAddress & 0x0FFFFF) >> 1)] = byteValue;
			break;
		case 4:                         // Z80

			// kof98 and lresort attempt to write here when the system still has the z80 bank
			// it seems they attempt to write regular samples (not even deltat) maybe there is
			// some kind of fall-through behavior, or it shouldn't be allowed to select a
			// transfer area without the bus? - this should really be checked on hw
			if (m_has_z80_bus)
			{
				YM2610ADPCMAROM[nADPCMTransferBank + ((sekAddress & 0x0FFFFF) >> 1)] = byteValue;
			}
			else
			{
		//  printf("sekAddress %08x %02x\n", sekAddress, data);
				if ((sekAddress & 0xfffff) >= 0x20000) break;
				NeoZ80ROMActive[(sekAddress & 0x1FFFF) >> 1] = byteValue;
			}
			break;
		case 5:                         // Text
			NeoTextRAM[(sekAddress & 0x3FFFF) >> 1] = byteValue;
			break;
	}
}



void ngcd_state::set_DMA_regs(int offset, UINT16 wordValue)
{
	switch (offset)
	{
		case 0x0064:
			NeoCDDMAAddress1 &= 0x0000FFFF;
			NeoCDDMAAddress1 |= wordValue << 16;
			break;
		case 0x0066:
			NeoCDDMAAddress1 &= 0xFFFF0000;
			NeoCDDMAAddress1 |= wordValue;
			break;
		case 0x0068:
			NeoCDDMAAddress2 &= 0x0000FFFF;
			NeoCDDMAAddress2 |= wordValue << 16;
			break;
		case 0x006A:
			NeoCDDMAAddress2 &= 0xFFFF0000;
			NeoCDDMAAddress2 |= wordValue;
			break;
		case 0x006C:
			NeoCDDMAValue1 = wordValue;
			break;
		case 0x006E:
			NeoCDDMAValue2 = wordValue;
			break;
		case 0x0070:
			NeoCDDMACount &= 0x0000FFFF;
			NeoCDDMACount |= wordValue << 16;
			break;
		case 0x0072:
			NeoCDDMACount &= 0xFFFF0000;
			NeoCDDMACount |= wordValue;
			break;

		case 0x007E:
			NeoCDDMAMode = wordValue;
//          bprintf(PRINT_NORMAL, _T("  - DMA controller 0x%2X -> 0x%04X (PC: 0x%06X)\n"), sekAddress & 0xFF, wordValue, SekGetPC(-1));
			break;

	}
}



INT32 ngcd_state::SekIdle(INT32 nCycles)
{
	return nCycles;
}



/*
 *  CD-ROM / DMA control
 *
 *  DMA

    FF0061  Write 0x40 means start DMA transfer
    FF0064  Source address (in copy mode), Target address (in filll mode)
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


void ngcd_state::NeoCDDoDMA(address_space& curr_space)
{
	// The LC8953 chip has a programmable DMA controller, which is not properly emulated.
	// Since the software only uses it in a limited way, we can apply a simple heuristic
	// to determnine the requested operation.

	// Additionally, we don't know how many cycles DMA operations take.
	// Here, only bus access is used to get a rough approximation --
	// each read/write takes a single cycle, setup and everything else is ignored.

//  bprintf(PRINT_IMPORTANT, _T("  - DMA controller transfer started (PC: 0x%06X)\n"), SekGetPC(-1));

	switch (NeoCDDMAMode) {
		case 0xCFFD: {
//          bprintf(PRINT_NORMAL, _T("    adr : 0x%08X - 0x%08X <- address, skip odd bytes\n"), NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 8);

			//  - DMA controller 0x7E -> 0xCFFD (PC: 0xC07CE2)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC07CE8)
			//  - DMA controller program[02] -> 0xE8DA (PC: 0xC07CEE)
			//  - DMA controller program[04] -> 0x92DA (PC: 0xC07CF4)
			//  - DMA controller program[06] -> 0x92DB (PC: 0xC07CFA)
			//  - DMA controller program[08] -> 0x96DB (PC: 0xC07D00)
			//  - DMA controller program[10] -> 0x96F6 (PC: 0xC07D06)
			//  - DMA controller program[12] -> 0x2E02 (PC: 0xC07D0C)
			//  - DMA controller program[14] -> 0xFDFF (PC: 0xC07D12)

			SekIdle(NeoCDDMACount * 4);

			while (NeoCDDMACount--) {
				curr_space.write_word(NeoCDDMAAddress1 + 0, NeoCDDMAAddress1 >> 24);
				curr_space.write_word(NeoCDDMAAddress1 + 2, NeoCDDMAAddress1 >> 16);
				curr_space.write_word(NeoCDDMAAddress1 + 4, NeoCDDMAAddress1 >>  8);
				curr_space.write_word(NeoCDDMAAddress1 + 6, NeoCDDMAAddress1 >>  0);
				NeoCDDMAAddress1 += 8;
			}

			break;
		}

		case 0xE2DD: {
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- 0x%08X - 0x%08X, skip odd bytes\n"), NeoCDDMAAddress2, NeoCDDMAAddress2 + NeoCDDMACount * 2, NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 4);

			//  - DMA controller 0x7E -> 0xE2DD (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x82BE (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x93DA (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xBE93 (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0xDABE (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xF62D (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x02FD (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xFFFF (PC: 0xC0A1A0)

			SekIdle(NeoCDDMACount * 1);

			while (NeoCDDMACount--) {
				curr_space.write_word(NeoCDDMAAddress2 + 0, curr_space.read_byte(NeoCDDMAAddress1 + 0));
				curr_space.write_word(NeoCDDMAAddress2 + 2, curr_space.read_byte(NeoCDDMAAddress1 + 1));
				NeoCDDMAAddress1 += 2;
				NeoCDDMAAddress2 += 4;
			}

			break;
		}

		case 0xFC2D: {
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- LC8951 external buffer, skip odd bytes\n"), NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 4);

			//  - DMA controller 0x7E -> 0xFC2D (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x8492 (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0xDA92 (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xDAF6 (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0x2A02 (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xFDFF (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x48E7 (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xFFFE (PC: 0xC0A1A0)

			char* data = m_tempcdc->LC8915InitTransfer(NeoCDDMACount);
			if (data == nullptr) {
				break;
			}

			SekIdle(NeoCDDMACount * 4);

			while (NeoCDDMACount--) {
				curr_space.write_byte(NeoCDDMAAddress1 + 0, data[0]);
				curr_space.write_byte(NeoCDDMAAddress1 + 2, data[1]);
				NeoCDDMAAddress1 += 4;
				data += 2;
			}

			m_tempcdc->LC8915EndTransfer();

			break;
		}

		case 0xFE3D:

			//  - DMA controller 0x7E -> 0xFE3D (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x82BF (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x93BF (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xF629 (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0x02FD (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xFFFF (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0xF17D (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xFCF5 (PC: 0xC0A1A0)

		case 0xFE6D: {
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- 0x%08X - 0x%08X\n"), NeoCDDMAAddress2, NeoCDDMAAddress2 + NeoCDDMACount * 2, NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 2);

			//  - DMA controller 0x7E -> 0xFE6D (PC: 0xC0FD7A)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0FD7C)
			//  - DMA controller program[02] -> 0x82BF (PC: 0xC0FD7E)
			//  - DMA controller program[04] -> 0xF693 (PC: 0xC0FD80)
			//  - DMA controller program[06] -> 0xBF29 (PC: 0xC0FD82)
			//  - DMA controller program[08] -> 0x02FD (PC: 0xC0FD84)
			//  - DMA controller program[10] -> 0xFFFF (PC: 0xC0FD86)
			//  - DMA controller program[12] -> 0xC515 (PC: 0xC0FD88)
			//  - DMA controller program[14] -> 0xFCF5 (PC: 0xC0FD8A)

			SekIdle(NeoCDDMACount * 1);

			while (NeoCDDMACount--) {
				curr_space.write_word(NeoCDDMAAddress2, curr_space.read_word(NeoCDDMAAddress1));
				NeoCDDMAAddress1 += 2;
				NeoCDDMAAddress2 += 2;
			}

if (NeoCDDMAAddress2 == 0x0800)  {
// MapVectorTable(false);
//  bprintf(PRINT_ERROR, _T("    RAM vectors mapped (PC = 0x%08X\n"), SekGetPC(0));
//  extern INT32 bRunPause;
//  bRunPause = 1;
}
			break;
		}

		case 0xFEF5: {
//          bprintf(PRINT_NORMAL, _T("    adr : 0x%08X - 0x%08X <- address\n"), NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 4);

			//  - DMA controller 0x7E -> 0xFEF5 (PC: 0xC07CE2)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC07CE8)
			//  - DMA controller program[02] -> 0x92E8 (PC: 0xC07CEE)
			//  - DMA controller program[04] -> 0xBE96 (PC: 0xC07CF4)
			//  - DMA controller program[06] -> 0xF629 (PC: 0xC07CFA)
			//  - DMA controller program[08] -> 0x02FD (PC: 0xC07D00)
			//  - DMA controller program[10] -> 0xFFFF (PC: 0xC07D06)
			//  - DMA controller program[12] -> 0xFC3D (PC: 0xC07D0C)
			//  - DMA controller program[14] -> 0xFCF5 (PC: 0xC07D12)

			SekIdle(NeoCDDMACount * 2);

			while (NeoCDDMACount--) {
				curr_space.write_word(NeoCDDMAAddress1 + 0, NeoCDDMAAddress1 >> 16);
				curr_space.write_word(NeoCDDMAAddress1 + 2, NeoCDDMAAddress1 >>  0);
				NeoCDDMAAddress1 += 4;
			}

			break;
		}

		case 0xFFC5: {
//          bprintf(PRINT_NORMAL, _T("    copy: 0x%08X - 0x%08X <- LC8951 external buffer\n"), NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 2);

			//  - DMA controller 0x7E -> 0xFFC5 (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0xA6F6 (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x2602 (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xFDFF (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0xFC2D (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0xFCF5 (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x8492 (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0xDA92 (PC: 0xC0A1A0)

			char* data = m_tempcdc->LC8915InitTransfer(NeoCDDMACount);
			if (data == nullptr) {
				break;
			}

			SekIdle(NeoCDDMACount * 4);

			while (NeoCDDMACount--) {
				curr_space.write_byte(NeoCDDMAAddress1 + 0, data[0]);
				curr_space.write_byte(NeoCDDMAAddress1 + 1, data[1]);
				NeoCDDMAAddress1 += 2;
				data += 2;
			}

			m_tempcdc->LC8915EndTransfer();

			break;
		}

		case 0xFFCD:

			//  - DMA controller 0x7E -> 0xFFCD (PC: 0xC0A190)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC0A192)
			//  - DMA controller program[02] -> 0x92F6 (PC: 0xC0A194)
			//  - DMA controller program[04] -> 0x2602 (PC: 0xC0A196)
			//  - DMA controller program[06] -> 0xFDFF (PC: 0xC0A198)
			//  - DMA controller program[08] -> 0x7006 (PC: 0xC0A19A)
			//  - DMA controller program[10] -> 0x6100 (PC: 0xC0A19C)
			//  - DMA controller program[12] -> 0x2412 (PC: 0xC0A19E)
			//  - DMA controller program[14] -> 0x13FC (PC: 0xC0A1A0)

		case 0xFFDD: {
//          bprintf(PRINT_NORMAL, _T("    Fill: 0x%08X - 0x%08X <- 0x%04X\n"), NeoCDDMAAddress1, NeoCDDMAAddress1 + NeoCDDMACount * 2, NeoCDDMAValue1);

			//  - DMA controller 0x7E -> 0xFFDD (PC: 0xC07CE2)
			//  - DMA controller program[00] -> 0xFCF5 (PC: 0xC07CE8)
			//  - DMA controller program[02] -> 0x92F6 (PC: 0xC07CEE)
			//  - DMA controller program[04] -> 0x2602 (PC: 0xC07CF4)
			//  - DMA controller program[06] -> 0xFDFF (PC: 0xC07CFA)
			//  - DMA controller program[08] -> 0xFFFF (PC: 0xC07D00)
			//  - DMA controller program[10] -> 0xFCF5 (PC: 0xC07D06)
			//  - DMA controller program[12] -> 0x8AF0 (PC: 0xC07D0C)
			//  - DMA controller program[14] -> 0x1609 (PC: 0xC07D12)

			SekIdle(NeoCDDMACount * 1);

			while (NeoCDDMACount--) {
				curr_space.write_word(NeoCDDMAAddress1, NeoCDDMAValue1);
				NeoCDDMAAddress1 += 2;
			}

			break;
		}
		default: {
			//bprintf(PRINT_ERROR, _T("    Unknown transfer type 0x%04X (PC: 0x%06X)\n"), NeoCDDMAMode, SekGetPC(-1));
			//bprintf(PRINT_NORMAL, _T("    ??? : 0x%08X  0x%08X 0x%04X 0x%04X 0x%08X\n"), NeoCDDMAAddress1, NeoCDDMAAddress2, NeoCDDMAValue1, NeoCDDMAValue2, NeoCDDMACount);

//extern INT32 bRunPause;
//bRunPause = 1;

		}
	}
}


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

MACHINE_START_MEMBER(ngcd_state,neocd)
{
	m_type = NEOGEO_CD;
	common_machine_start();

	// set curr_slot to 0, so to allow checking m_slots[m_curr_slot] != nullptr
	m_curr_slot = 0;

	// initialize sprite to point to memory regions
	m_sprgen->m_fixed_layer_bank_type = 0;
	m_sprgen->set_screen(m_screen);
	m_sprgen->set_sprite_region(m_region_sprites->base(), m_region_sprites->bytes());
	m_sprgen->set_fixed_regions(m_region_fixed->base(), m_region_fixed->bytes(), m_region_fixedbios);
	m_sprgen->neogeo_set_fixed_layer_source(1);

	// irq levels for NEOCD (swapped compared to MVS / AES)
	m_vblank_level = 2;
	m_raster_level = 1;

	// initialize the memcard data structure
	// NeoCD doesn't have memcard slots, rather, it has a larger internal memory which works the same
	m_meminternal_data = make_unique_clear<UINT8[]>(0x2000);
	machine().device<nvram_device>("saveram")->set_base(m_meminternal_data.get(), 0x2000);
	save_pointer(NAME(m_meminternal_data.get()), 0x2000);

	m_use_cart_vectors = 0;

	m_tempcdc->reset_cd();
}


/*************************************
 *
 *  Machine reset
 *
 *************************************/

MACHINE_RESET_MEMBER(ngcd_state,neocd)
{
	neogeo_state::machine_reset();

	NeoSpriteRAM = memregion("sprites")->base();
	YM2610ADPCMAROM = memregion("ymsnd")->base();
	NeoZ80ROMActive = memregion("audiocpu")->base();
	NeoTextRAM = memregion("fixed")->base();

	m_tempcdc->NeoCDCommsReset();

	m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	nTransferWriteEnable = 0;
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( neocd_main_map, AS_PROGRAM, 16, ngcd_state )
//  AM_RANGE(0x000000, 0x00007f) AM_READ_BANK("vectors") // writes will fall through to area below
	AM_RANGE(0x000000, 0x00007f) AM_READ(banked_vectors_r)
	AM_RANGE(0x000000, 0x1fffff) AM_RAM AM_REGION("maincpu", 0x00000)

	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01fffe) AM_DEVREAD8("ctrl1", neogeo_control_port_device, ctrl_r, 0xff00)
	AM_RANGE(0x320000, 0x320001) AM_MIRROR(0x01fffe) AM_READ_PORT("AUDIO") AM_WRITE8(audio_command_w, 0xff00)
	AM_RANGE(0x340000, 0x340001) AM_MIRROR(0x01fffe) AM_DEVREAD8("ctrl2", neogeo_control_port_device, ctrl_r, 0xff00)
	AM_RANGE(0x360000, 0x37ffff) AM_READ(unmapped_r)
	AM_RANGE(0x380000, 0x380001) AM_MIRROR(0x01fffe) AM_READ(aes_in2_r)
	AM_RANGE(0x380000, 0x38007f) AM_MIRROR(0x01ff80) AM_WRITE8(io_control_w, 0x00ff)
	AM_RANGE(0x3a0000, 0x3a001f) AM_MIRROR(0x01ffe0) AM_READ(unmapped_r) AM_WRITE8(system_control_w, 0x00ff)
	AM_RANGE(0x3c0000, 0x3c0007) AM_MIRROR(0x01fff8) AM_READ(video_register_r)
	AM_RANGE(0x3c0000, 0x3c000f) AM_MIRROR(0x01fff0) AM_WRITE(video_register_w)
	AM_RANGE(0x3e0000, 0x3fffff) AM_READ(unmapped_r)
	AM_RANGE(0x400000, 0x401fff) AM_MIRROR(0x3fe000) AM_READWRITE(paletteram_r, paletteram_w)
	AM_RANGE(0x800000, 0x803fff) AM_READWRITE(neocd_memcard_r, neocd_memcard_w)
	AM_RANGE(0xc00000, 0xc7ffff) AM_MIRROR(0x080000) AM_ROM AM_REGION("mainbios", 0)
	AM_RANGE(0xd00000, 0xdfffff) AM_READ(unmapped_r)
	AM_RANGE(0xe00000, 0xefffff) AM_READWRITE8(neocd_transfer_r,neocd_transfer_w, 0xffff)
	AM_RANGE(0xf00000, 0xfeffff) AM_READ(unmapped_r)
	AM_RANGE(0xff0000, 0xff01ff) AM_READWRITE(neocd_control_r, neocd_control_w) // CDROM / DMA
	AM_RANGE(0xff0200, 0xffffff) AM_READ(unmapped_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/


static ADDRESS_MAP_START( neocd_audio_map, AS_PROGRAM, 8, ngcd_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_REGION("audiocpu", 0x00000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( neocd_audio_io_map, AS_IO, 8, ngcd_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READWRITE(audio_command_r, soundlatch_clear_byte_w)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xff00) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff10) AM_MASK(0x0010) AM_WRITE(audio_cpu_enable_nmi_w)
	// banking reads are actually NOP on NeoCD? but some games still access them
//  AM_RANGE(0x08, 0x0b) AM_MIRROR(0xfff0) AM_MASK(0xff03) AM_READ(audio_cpu_bank_select_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff00) AM_WRITE(soundlatch2_byte_w)

	// ??
	AM_RANGE(0x80, 0x80) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0xff00) AM_WRITENOP
ADDRESS_MAP_END


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

IRQ_CALLBACK_MEMBER(ngcd_state::neocd_int_callback)
{
	if (irqline==4)
	{
		if (get_nNeoCDIRQVectorAck()) {
			set_nNeoCDIRQVectorAck(0);
			return get_nNeoCDIRQVector();
		}
	}

	return (0x60+irqline*4)/4;
}

void ngcd_state::interrupt_callback_type1(void)
{
	nIRQAcknowledge &= ~0x20;
	NeoCDIRQUpdate(0);
}

void ngcd_state::interrupt_callback_type2(void)
{
	nIRQAcknowledge &= ~0x10;
	NeoCDIRQUpdate(0);
}

void ngcd_state::interrupt_callback_type3(void)
{
	nIRQAcknowledge &= ~0x08;
	NeoCDIRQUpdate(0);
}


void ngcd_state::NeoCDIRQUpdate(UINT8 byteValue)
{
	// do we also need to check the regular interrupts like FBA?

	nIRQAcknowledge |= (byteValue & 0x38);

	if (!prohibit_cdc_irq)
	{
		if ((nIRQAcknowledge & 0x08) == 0) {
			nNeoCDIRQVector = 0x17;
			nNeoCDIRQVectorAck = 1;
			m_maincpu->set_input_line(4, HOLD_LINE);
			return;
		}
		if ((nIRQAcknowledge & 0x10) == 0) {
			nNeoCDIRQVector = 0x16;
			nNeoCDIRQVectorAck = 1;
			m_maincpu->set_input_line(4, HOLD_LINE);
			return;
		}
		if ((nIRQAcknowledge & 0x20) == 0) {
			nNeoCDIRQVector = 0x15;
			nNeoCDIRQVectorAck = 1;
			m_maincpu->set_input_line(4, HOLD_LINE);
			return;
		}
	}
}


UINT32 ngcd_state::screen_update_neocd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// fill with background color first
	bitmap.fill(*m_bg_pen, cliprect);

	if (m_has_sprite_bus) m_sprgen->draw_sprites(bitmap, cliprect.min_y);

	if (m_has_text_bus) m_sprgen->draw_fixed_layer(bitmap, cliprect.min_y);

	return 0;
}


static MACHINE_CONFIG_DERIVED_CLASS( neocd, neogeo_base, ngcd_state )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(neocd_main_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(ngcd_state,neocd_int_callback)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(neocd_audio_map)
	MCFG_CPU_IO_MAP(neocd_audio_io_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(ngcd_state, screen_update_neocd)

	// temporary until things are cleaned up
	MCFG_DEVICE_ADD("tempcdc", LC89510_TEMP, 0) // cd controller
	MCFG_SEGACD_HACK_SET_NEOCD
	MCFG_SET_TYPE1_INTERRUPT_CALLBACK( ngcd_state, interrupt_callback_type1 )
	MCFG_SET_TYPE2_INTERRUPT_CALLBACK( ngcd_state, interrupt_callback_type2 )
	MCFG_SET_TYPE3_INTERRUPT_CALLBACK( ngcd_state, interrupt_callback_type3 )

	MCFG_NVRAM_ADD_0FILL("saveram")

	MCFG_MACHINE_START_OVERRIDE(ngcd_state,neocd)
	MCFG_MACHINE_RESET_OVERRIDE(ngcd_state,neocd)

	MCFG_NEOGEO_CONTROL_PORT_ADD("ctrl1", neogeo_controls, "joy", false)
	MCFG_NEOGEO_CONTROL_PORT_ADD("ctrl2", neogeo_controls, "joy", false)

	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("neocd_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","neocd")
MACHINE_CONFIG_END



/*************************************
 *
 *  Driver initalization
 *
 *************************************/

ROM_START( neocd )
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_SYSTEM_BIOS( 0, "top",   "Top loading NeoGeo CD" )
	ROMX_LOAD( "top-sp1.bin",    0x00000, 0x80000, CRC(c36a47c0) SHA1(235f4d1d74364415910f73c10ae5482d90b4274f), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "front",   "Front loading NeoGeo CD" )
	ROMX_LOAD( "front-sp1.bin",    0x00000, 0x80000, CRC(cac62307) SHA1(53bc1f283cdf00fa2efbb79f2e36d4c8038d743a), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2))

	ROM_REGION( 0x100000, "ymsnd", ROMREGION_ERASEFF )
	/* 1MB of Sound RAM */

	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )
	/* 64KB of Z80 RAM */

	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	/* 2MB of 68K RAM */

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASEFF )
	/* 4MB of Sprite Tile RAM */

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASEFF )
	/* 128KB of Text Tile RAM */

	ROM_REGION( 0x20000, "zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
ROM_END

ROM_START( neocdz )
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_LOAD16_WORD_SWAP( "neocd.bin",    0x00000, 0x80000, CRC(df9de490) SHA1(7bb26d1e5d1e930515219cb18bcde5b7b23e2eda) )

	ROM_REGION( 0x100000, "ymsnd", ROMREGION_ERASEFF )
	/* 1MB of Sound RAM */

	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )
	/* 64KB of Z80 RAM */

	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	/* 2MB of 68K RAM */

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASEFF )
	/* 4MB of Sprite Tile RAM */

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASEFF )
	/* 128KB of Text Tile RAM */

	ROM_REGION( 0x20000, "zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
ROM_END

#define rom_neocdzj    rom_neocdz

DRIVER_INIT_MEMBER(ngcd_state,neocdz)
{
	NeoSystem = NEOCD_REGION_US;
}

DRIVER_INIT_MEMBER(ngcd_state,neocdzj)
{
	NeoSystem = NEOCD_REGION_JAPAN;
}


/*    YEAR  NAME  PARENT COMPAT MACHINE INPUT  INIT     COMPANY      FULLNAME            FLAGS */
CONS( 1996, neocdz,  0,      0,   neocd, neocd, ngcd_state,  neocdz,  "SNK", "Neo-Geo CDZ (US)", 0 ) // the CDZ is the newer model
CONS( 1996, neocdzj, neocdz, 0,   neocd, neocd, ngcd_state,  neocdzj,  "SNK", "Neo-Geo CDZ (Japan)", 0 )


CONS( 1994, neocd,   neocdz, 0,   neocd, neocd, driver_device,  0,  "SNK", "Neo-Geo CD", MACHINE_NOT_WORKING ) // older  model, ignores disc protections?
