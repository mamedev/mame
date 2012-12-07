/***************************************************************************

    Neo-Geo AES hardware

    Credits (from MAME neogeo.c, since this is just a minor edit of that driver):
        * This driver was made possible by the research done by
          Charles MacDonald.  For a detailed description of the Neo-Geo
          hardware, please visit his page at:
          http://cgfm2.emuviews.com/temp/mvstech.txt
        * Presented to you by the Shin Emu Keikaku team.
        * The following people have all spent probably far
          too much time on this:
          AVDB
          Bryan McPhail
          Fuzz
          Ernesto Corvi
          Andrew Prime
          Zsolt Vasvari

    MESS cartridge support by R. Belmont based on work by Michael Zapf

    Current status:
        - Cartridges run.
		- Riding Hero runs in slow-mo (probably related to comms HW / IO port handling)

    ToDo :
        - Change input code to allow selection of the mahjong panel in PORT_CATEGORY.
        - Clean up code, to reduce duplication of MAME source


    Neo-Geo CD hardware

	Thanks to:
	    * The FBA team (Barry Harris) for much of the CDC / CDD code and system details.
		  ( http://www.barryharris.me.uk/ )
		* Mirko Buffoni for a commented disassembly of the NeoCD bios rom.

    Current status:
		- NeoCDZ runs, the original NeoCD does not
		   - Might think the tray is open? (check)
		- Some unknown / unhandled CD commands, code is still a bit messy
		- Games using Raster Effects are broken, even non-IRQ based ones like mosyougi
		   - Are we overloading the CPU with interrupts from the CDC, incorrect masking? or something else?
		- Double Dragon doesn't load, it erases the IRQ table
		   - might need better handling of the Vector Table Mapping, or better interrupts (see point above)
		- Softlist are based on an old Tosec set and should be updated to the TruRip set once we can convert CCD
		  without throwing away gap data etc.
		- Backup RAM isn't saved?

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/neogeo.h"
#include "machine/pd4990a.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "imagedev/cartslot.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"
#include "machine/megacdcd.h"


extern const char layout_neogeo[];




static IRQ_CALLBACK(neocd_int_callback);

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
INT32 nNeoCDZ80ProgWriteWordCancelHack = 0;









class ng_aes_state : public neogeo_state
{
public:
	ng_aes_state(const machine_config &mconfig, device_type type, const char *tag)
		: neogeo_state(mconfig, type, tag),
		m_tempcdc(*this,"tempcdc")
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

	}

	optional_device<lc89510_temp_device> m_tempcdc;
	


	void NeoCDDoDMA(address_space& curr_space);
	void set_DMA_regs(int offset, UINT16 wordValue);

	UINT8 *m_memcard_data;
	DECLARE_WRITE16_MEMBER(save_ram_w);
	DECLARE_READ16_MEMBER(memcard_r);
	DECLARE_WRITE16_MEMBER(memcard_w);
	DECLARE_READ16_MEMBER(neocd_memcard_r);
	DECLARE_WRITE16_MEMBER(neocd_memcard_w);
	DECLARE_READ16_MEMBER(neocd_control_r);
	DECLARE_WRITE16_MEMBER(neocd_control_w);
	DECLARE_READ8_MEMBER(neocd_transfer_r);
	DECLARE_WRITE8_MEMBER(neocd_transfer_w);
	DECLARE_READ16_MEMBER(aes_in0_r);
	DECLARE_READ16_MEMBER(aes_in1_r);
	DECLARE_READ16_MEMBER(aes_in2_r);
	DECLARE_DRIVER_INIT(neogeo);
	DECLARE_MACHINE_START(neocd);
	DECLARE_MACHINE_START(neogeo);
	DECLARE_MACHINE_RESET(neogeo);
	DECLARE_MACHINE_RESET(neocd);

	DECLARE_CUSTOM_INPUT_MEMBER(get_memcard_status);

	// neoCD

	UINT8 neogeoReadTransfer(UINT32 sekAddress, int is_byte_transfer);
	void neogeoWriteTransfer(UINT32 sekAddress, UINT8 byteValue, int is_byte_transfer);

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

	int get_nNeoCDIRQVectorAck(void) { return nNeoCDIRQVectorAck; }
	void set_nNeoCDIRQVectorAck(int val) { nNeoCDIRQVectorAck = val; }
	int get_nNeoCDIRQVector(void) { return nNeoCDIRQVector; }
	void NeoCDIRQUpdate(UINT8 byteValue);
	
	// from the CDC
	void interrupt_callback_type1(void);
	void interrupt_callback_type2(void);
	void interrupt_callback_type3(void);

	UINT8 nTransferWriteEnable;

	address_space* curr_space;
};


/*************************************
 *
 *  Global variables
 *
 *************************************/

//static UINT16 *save_ram;

//UINT16* neocd_work_ram;

/*************************************
 *
 *  Forward declarations
 *
 *************************************/

//static void set_output_latch(running_machine &machine, UINT8 data);
//static void set_output_data(running_machine &machine, UINT8 data);






/*************************************
 *
 *  Memory card
 *
 *************************************/

#define MEMCARD_SIZE	0x0800

CUSTOM_INPUT_MEMBER(ng_aes_state::get_memcard_status)
{
	/* D0 and D1 are memcard presence indicators, D2 indicates memcard
       write protect status (we are always write enabled) */
	if(strcmp((char*)machine().system().name,"aes") != 0)
		return 0x00;  // On the Neo Geo CD, the memory card is internal and therefore always present.
	else
		return (memcard_present(machine()) == -1) ? 0x07 : 0x00;
}

READ16_MEMBER(ng_aes_state::memcard_r)
{
	UINT16 ret;

	if (memcard_present(machine()) != -1)
		ret = m_memcard_data[offset] | 0xff00;
	else
		ret = 0xffff;

	return ret;
}


WRITE16_MEMBER(ng_aes_state::memcard_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (memcard_present(machine()) != -1)
			m_memcard_data[offset] = data;
	}
}

/* The NeoCD has an 8kB internal memory card, instead of memcard slots like the MVS and AES */
READ16_MEMBER(ng_aes_state::neocd_memcard_r)
{
	return m_memcard_data[offset] | 0xff00;
}


WRITE16_MEMBER(ng_aes_state::neocd_memcard_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_memcard_data[offset] = data;
	}
}

static MEMCARD_HANDLER( neogeo_aes )
{
	ng_aes_state *state = machine.driver_data<ng_aes_state>();
	switch (action)
	{
	case MEMCARD_CREATE:
		memset(state->m_memcard_data, 0, MEMCARD_SIZE);
		file.write(state->m_memcard_data, MEMCARD_SIZE);
		break;

	case MEMCARD_INSERT:
		file.read(state->m_memcard_data, MEMCARD_SIZE);
		break;

	case MEMCARD_EJECT:
		file.write(state->m_memcard_data, MEMCARD_SIZE);
		break;
	}
}






/*************************************
 *
 *  System control register
 *
 *************************************/








READ16_MEMBER(ng_aes_state::neocd_control_r)
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


WRITE16_MEMBER(ng_aes_state::neocd_control_w)
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
			nActiveTransferArea = byteValue;
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
			video_reset();
			break;
		case 0x0142:
//          bprintf(PRINT_NORMAL, _T("  - NGCD PCM BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_ymrom_bus = true;
			break;
		case 0x0146:
//          bprintf(PRINT_NORMAL, _T("  - NGCD Z80 BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			space.machine().scheduler().synchronize();
			m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			break;
		case 0x0148:
//          bprintf(PRINT_NORMAL, _T("  - NGCD FIX BUSREQ -> 0 (PC: 0x%06X)\n"), SekGetPC(-1));
			m_has_text_bus = true;
			video_reset();
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

				if (!data) neogeo_set_main_cpu_vector_table_source(machine(), 0); // bios vectors
				else neogeo_set_main_cpu_vector_table_source(machine(), 1); // ram (aka cart) vectors

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
				//	m_tempcdc->NeoCDCommsReset();
				}
			}
			break;
		}
		case 0x0182: {
		//	printf("blah %02x\n", byteValue);
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


READ8_MEMBER(ng_aes_state::neocd_transfer_r)
{
	UINT32 sekAddress = 0xe00000+ (offset);
	int address;
	sekAddress ^= 1;

	switch (nActiveTransferArea) {
		case 0:	// Sprites
			address = (nSpriteTransferBank + (sekAddress & 0x0FFFFF));

			// address is swizzled a bit due to out sprite decoding
			if ((address&3)==0) return NeoSpriteRAM[address];
			if ((address&3)==1) return NeoSpriteRAM[address^3];
			if ((address&3)==2) return NeoSpriteRAM[address^3];
			if ((address&3)==3) return NeoSpriteRAM[address];

			return NeoSpriteRAM[nSpriteTransferBank + (sekAddress & 0x0FFFFF)];
		case 1:							// ADPCM
			return YM2610ADPCMAROM[nADPCMTransferBank + ((sekAddress & 0x0FFFFF) >> 1)];
		case 4:							// Z80
			if ((sekAddress & 0xfffff) >= 0x20000) return ~0;
			return NeoZ80ROMActive[(sekAddress & 0x1FFFF) >> 1];
		case 5:							// Text
			return NeoTextRAM[(sekAddress & 0x3FFFF) >> 1];
	}

	return ~0;

}

WRITE8_MEMBER(ng_aes_state::neocd_transfer_w)
{
	UINT8 byteValue = data;
	UINT32 sekAddress = 0xe00000+ (offset);

	if (!nTransferWriteEnable) {
//      return;
	}
	int address;

	sekAddress ^= 1;

	switch (nActiveTransferArea) {
		case 0:							// Sprites
			address = (nSpriteTransferBank + (sekAddress & 0x0FFFFF));

			// address is swizzled a bit due to out sprite decoding
			if ((address&3)==0) NeoSpriteRAM[address] = byteValue;
			if ((address&3)==1) NeoSpriteRAM[address^3] = byteValue;
			if ((address&3)==2) NeoSpriteRAM[address^3] = byteValue;
			if ((address&3)==3) NeoSpriteRAM[address] = byteValue;

			break;
		case 1:							// ADPCM
			YM2610ADPCMAROM[nADPCMTransferBank + ((sekAddress & 0x0FFFFF) >> 1)] = byteValue;
			break;
		case 4:							// Z80
			if ((sekAddress & 0xfffff) >= 0x20000) break;

			NeoZ80ROMActive[(sekAddress & 0x1FFFF) >> 1] = byteValue;
			break;
		case 5:							// Text
			NeoTextRAM[(sekAddress & 0x3FFFF) >> 1] = byteValue;
			break;
	}
}



void ng_aes_state::set_DMA_regs(int offset, UINT16 wordValue)
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





static INT32 SekIdle(INT32 nCycles)
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


void ng_aes_state::NeoCDDoDMA(address_space& curr_space)
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
			if (data == NULL) {
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
			if (data == NULL) {
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


/*
 * Handling selectable controller types
 */

READ16_MEMBER(ng_aes_state::aes_in0_r)
{
	UINT32 ret = 0xffff;
	UINT32 ctrl = ioport("CTRLSEL")->read();

	switch(ctrl & 0x0f)
	{
	case 0x00:
		ret = 0xffff;
		break;
	case 0x01:
		ret = ioport("IN0")->read();
		break;
	case 0x02:
		switch (m_controller_select)
		{
			case 0x09: ret = ioport("MJ01_P1")->read(); break;
			case 0x12: ret = ioport("MJ02_P1")->read(); break;
			case 0x1b: ret = ioport("MJ03_P1")->read(); break; /* player 1 normal inputs? */
			case 0x24: ret = ioport("MJ04_P1")->read(); break;
			default:
				ret = ioport("IN0")->read();
				break;
		}
		break;
	}

	return ret;
}

READ16_MEMBER(ng_aes_state::aes_in1_r)
{
	UINT32 ret = 0xffff;
	UINT32 ctrl = ioport("CTRLSEL")->read();

	switch(ctrl & 0xf0)
	{
	case 0x00:
		ret = 0xffff;
		break;
	case 0x10:
		ret = ioport("IN1")->read();
		break;
	case 0x20:
		switch (m_controller_select)
		{
			case 0x09: ret = ioport("MJ01_P2")->read(); break;
			case 0x12: ret = ioport("MJ02_P2")->read(); break;
			case 0x1b: ret = ioport("MJ03_P2")->read(); break; /* player 2 normal inputs? */
			case 0x24: ret = ioport("MJ04_P2")->read(); break;
			default:
				ret = ioport("IN1")->read();
				break;
		}
		break;
	}

	return ret;
}


READ16_MEMBER(ng_aes_state::aes_in2_r)
{
	UINT32 in2 = ioport("IN2")->read();
	UINT32 ret = in2;
	UINT32 sel = ioport("CTRLSEL")->read();

	if((sel & 0x02) && (m_controller_select == 0x24))
		ret ^= 0x0200;

	if((sel & 0x20) && (m_controller_select == 0x24))
		ret ^= 0x0800;

	return ret;
}


/*************************************
 *
 *  Machine initialization
 *
 *************************************/


static void common_machine_start(running_machine &machine)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	/* set the BIOS bank */
	state->membank(NEOGEO_BANK_BIOS)->set_base(state->memregion("mainbios")->base());

	/* set the initial main CPU bank */
	neogeo_main_cpu_banking_init(machine);

	/* set the initial audio CPU ROM banks */
	neogeo_audio_cpu_banking_init(machine);

	state->create_interrupt_timers(machine);

	/* irq levels for MVS / AES */
	state->m_vblank_level = 1;
	state->m_raster_level = 2;

	/* start with an IRQ3 - but NOT on a reset */
	state->m_irq3_pending = 1;

	/* get devices */
	state->m_maincpu = machine.device<cpu_device>("maincpu");
	state->m_audiocpu = machine.device<cpu_device>("audiocpu");
	state->m_upd4990a = machine.device("upd4990a");

	/* register state save */
	state->save_item(NAME(state->m_display_position_interrupt_control));
	state->save_item(NAME(state->m_display_counter));
	state->save_item(NAME(state->m_vblank_interrupt_pending));
	state->save_item(NAME(state->m_display_position_interrupt_pending));
	state->save_item(NAME(state->m_irq3_pending));
	state->save_item(NAME(state->m_audio_result));
	state->save_item(NAME(state->m_controller_select));
	state->save_item(NAME(state->m_main_cpu_bank_address));
	state->save_item(NAME(state->m_main_cpu_vector_table_source));
	state->save_item(NAME(state->m_audio_cpu_banks));
	state->save_item(NAME(state->m_audio_cpu_rom_source));
	state->save_item(NAME(state->m_audio_cpu_rom_source_last));
	state->save_item(NAME(state->m_save_ram_unlocked));
	state->save_item(NAME(state->m_output_data));
	state->save_item(NAME(state->m_output_latch));
	state->save_item(NAME(state->m_el_value));
	state->save_item(NAME(state->m_led1_value));
	state->save_item(NAME(state->m_led2_value));
	state->save_item(NAME(state->m_recurse));

	machine.save().register_postload(save_prepost_delegate(FUNC(neogeo_postload), &machine));
}

MACHINE_START_MEMBER(ng_aes_state,neogeo)
{
	common_machine_start(machine());
	m_is_mvs = false;

	/* initialize the memcard data structure */
	m_memcard_data = auto_alloc_array_clear(machine(), UINT8, MEMCARD_SIZE);
	save_pointer(NAME(m_memcard_data), 0x0800);
}

MACHINE_START_MEMBER(ng_aes_state,neocd)
{
//	UINT8* ROM = machine().root_device().memregion("mainbios")->base();
//	UINT8* RAM = machine().root_device().memregion("maincpu")->base();
//	UINT8* Z80bios = machine().root_device().memregion("audiobios")->base();
//	int x;
	m_has_audio_banking = false;
	m_is_cartsys = false;

	common_machine_start(machine());
	m_is_mvs = false;

	/* irq levels for NEOCD (swapped compared to MVS / AES) */
	m_vblank_level = 2;
	m_raster_level = 1;


	/* initialize the memcard data structure */
	/* NeoCD doesn't have memcard slots, rather, it has a larger internal memory which works the same */
	m_memcard_data = auto_alloc_array_clear(machine(), UINT8, 0x2000);
	save_pointer(NAME(m_memcard_data), 0x2000);

	// copy initial 68k vectors into RAM
	// memcpy(RAM,ROM,0x80);




	// for custom vectors
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(neocd_int_callback);

	neogeo_set_main_cpu_vector_table_source(machine(), 0); // default to the BIOS vectors

	m_tempcdc->reset_cd();
	
}


//static DEVICE_IMAGE_LOAD(aes_cart)
//{
//  else
//      return IMAGE_INIT_FAIL;

//  return IMAGE_INIT_PASS;
//}

/*************************************
 *
 *  Machine reset
 *
 *************************************/

MACHINE_RESET_MEMBER(ng_aes_state,neogeo)
{
	offs_t offs;
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	/* reset system control registers */
	for (offs = 0; offs < 8; offs++)
		system_control_w(space, offs, 0, 0x00ff);

	machine().device("maincpu")->reset();

	neogeo_reset_rng(machine());

	start_interrupt_timers(machine());

	/* trigger the IRQ3 that was set by MACHINE_START */
	update_interrupts(machine());

	m_recurse = 0;

	/* AES apparently always uses the cartridge's fixed bank mode */
	neogeo_set_fixed_layer_source(machine(),1);

	NeoSpriteRAM = memregion("sprites")->base();
	YM2610ADPCMAROM = memregion("ymsnd")->base();
	NeoZ80ROMActive = memregion("audiocpu")->base();
	NeoTextRAM = memregion("fixed")->base();
}

MACHINE_RESET_MEMBER(ng_aes_state,neocd)
{
	MACHINE_RESET_CALL_MEMBER( neogeo );

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

static ADDRESS_MAP_START( aes_main_map, AS_PROGRAM, 16, ng_aes_state )
	AM_RANGE(0x000000, 0x00007f) AM_ROMBANK(NEOGEO_BANK_VECTORS)
	AM_RANGE(0x000080, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x0f0000) AM_RAM
	/* some games have protection devices in the 0x200000 region, it appears to map to cart space, not surprising, the ROM is read here too */
	AM_RANGE(0x200000, 0x2fffff) AM_ROMBANK(NEOGEO_BANK_CARTRIDGE)
	AM_RANGE(0x2ffff0, 0x2fffff) AM_WRITE(main_cpu_bank_select_w)
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ff7e) AM_READ(aes_in0_r)
	AM_RANGE(0x300080, 0x300081) AM_MIRROR(0x01ff7e) AM_READ_PORT("IN4")
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITENOP	// AES has no watchdog
	AM_RANGE(0x320000, 0x320001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN3") AM_WRITE(audio_command_w)
	AM_RANGE(0x340000, 0x340001) AM_MIRROR(0x01fffe) AM_READ(aes_in1_r)
	AM_RANGE(0x360000, 0x37ffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x380000, 0x380001) AM_MIRROR(0x01fffe) AM_READ(aes_in2_r)
	AM_RANGE(0x380000, 0x38007f) AM_MIRROR(0x01ff80) AM_WRITE(io_control_w)
	AM_RANGE(0x3a0000, 0x3a001f) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITE(system_control_w)
	AM_RANGE(0x3c0000, 0x3c0007) AM_MIRROR(0x01fff8) AM_READ(neogeo_video_register_r)
	AM_RANGE(0x3c0000, 0x3c000f) AM_MIRROR(0x01fff0) AM_WRITE(neogeo_video_register_w)
	AM_RANGE(0x3e0000, 0x3fffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x400000, 0x401fff) AM_MIRROR(0x3fe000) AM_READWRITE(neogeo_paletteram_r, neogeo_paletteram_w)
	AM_RANGE(0x800000, 0x800fff) AM_READWRITE(memcard_r, memcard_w)
	AM_RANGE(0xc00000, 0xc1ffff) AM_MIRROR(0x0e0000) AM_ROMBANK(NEOGEO_BANK_BIOS)
	AM_RANGE(0xd00000, 0xd0ffff) AM_MIRROR(0x0f0000) AM_READ(neogeo_unmapped_r) AM_SHARE("save_ram") //AM_RAM_WRITE(save_ram_w)
	AM_RANGE(0xe00000, 0xffffff) AM_READ(neogeo_unmapped_r)
ADDRESS_MAP_END




static ADDRESS_MAP_START( neocd_main_map, AS_PROGRAM, 16, ng_aes_state )
	AM_RANGE(0x000000, 0x00007f) AM_READ_BANK(NEOGEO_BANK_VECTORS) // writes will fall through to area below
	AM_RANGE(0x000000, 0x1fffff) AM_RAM AM_REGION("maincpu", 0x00000)

	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ff7e) AM_READ(aes_in0_r)
	AM_RANGE(0x300080, 0x300081) AM_MIRROR(0x01ff7e) AM_READ_PORT("IN4")
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITENOP	// AES has no watchdog
	AM_RANGE(0x320000, 0x320001) AM_MIRROR(0x01fffe) AM_READ_PORT("IN3") AM_WRITE(audio_command_w)
	AM_RANGE(0x340000, 0x340001) AM_MIRROR(0x01fffe) AM_READ(aes_in1_r)
	AM_RANGE(0x360000, 0x37ffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x380000, 0x380001) AM_MIRROR(0x01fffe) AM_READ(aes_in2_r)
	AM_RANGE(0x380000, 0x38007f) AM_MIRROR(0x01ff80) AM_WRITE(io_control_w)
	AM_RANGE(0x3a0000, 0x3a001f) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITE(system_control_w)
	AM_RANGE(0x3c0000, 0x3c0007) AM_MIRROR(0x01fff8) AM_READ(neogeo_video_register_r)
	AM_RANGE(0x3c0000, 0x3c000f) AM_MIRROR(0x01fff0) AM_WRITE(neogeo_video_register_w)
	AM_RANGE(0x3e0000, 0x3fffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x400000, 0x401fff) AM_MIRROR(0x3fe000) AM_READWRITE(neogeo_paletteram_r, neogeo_paletteram_w)
	AM_RANGE(0x800000, 0x803fff) AM_READWRITE(neocd_memcard_r, neocd_memcard_w)
	AM_RANGE(0xc00000, 0xcfffff) AM_ROMBANK(NEOGEO_BANK_BIOS)
	AM_RANGE(0xd00000, 0xd0ffff) AM_MIRROR(0x0f0000) AM_READ(neogeo_unmapped_r) AM_SHARE("save_ram") //AM_RAM_WRITE(save_ram_w)
	AM_RANGE(0xe00000, 0xefffff) AM_READWRITE8(neocd_transfer_r,neocd_transfer_w, 0xffff)
	AM_RANGE(0xf00000, 0xfeffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0xff0000, 0xff01ff) AM_READWRITE(neocd_control_r, neocd_control_w) // CDROM / DMA
	AM_RANGE(0xff0200, 0xffffff) AM_READ(neogeo_unmapped_r)
ADDRESS_MAP_END





/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/


static ADDRESS_MAP_START( neocd_audio_map, AS_PROGRAM, 8, ng_aes_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_REGION("audiocpu", 0x00000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( neocd_audio_io_map, AS_IO, 8, ng_aes_state )
  /*AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READWRITE(audio_command_r, audio_cpu_clear_nmi_w);*/  /* may not and NMI clear */
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READ(audio_command_r)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xff00) AM_DEVREADWRITE_LEGACY("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff00) /* write - NMI enable / acknowledge? (the data written doesn't matter) */
	// banking reads are actually NOP on NeoCD? but some games still access them
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_f000_f7ff_r)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_e000_efff_r)
	AM_RANGE(0x0a, 0x0a) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_c000_dfff_r)
	AM_RANGE(0x0b, 0x0b) AM_MIRROR(0xfff0) AM_MASK(0xfff0) AM_READ(audio_cpu_bank_select_8000_bfff_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff00) AM_WRITE(audio_result_w)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0xff00) /* write - NMI disable? (the data written doesn't matter) */

	// ??
	AM_RANGE(0x80, 0x80) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0xff00) AM_WRITENOP
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0xff00) AM_WRITENOP
ADDRESS_MAP_END


/*************************************
 *
 *  Standard Neo-Geo DIPs and
 *  input port definition
 *
 *************************************/

#define STANDARD_DIPS																		\
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" ) PORT_DIPLOCATION("SW:1")					\
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0002, 0x0002, "Coin Chutes?" ) PORT_DIPLOCATION("SW:2")					\
	PORT_DIPSETTING(	  0x0000, "1?" )													\
	PORT_DIPSETTING(	  0x0002, "2?" )													\
	PORT_DIPNAME( 0x0004, 0x0004, "Autofire (in some games)" ) PORT_DIPLOCATION("SW:3")		\
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0018, 0x0018, "COMM Setting (Cabinet No.)" ) PORT_DIPLOCATION("SW:4,5")	\
	PORT_DIPSETTING(	  0x0018, "1" )														\
	PORT_DIPSETTING(	  0x0008, "2" )														\
	PORT_DIPSETTING(	  0x0010, "3" )														\
	PORT_DIPSETTING(	  0x0000, "4" )														\
	PORT_DIPNAME( 0x0020, 0x0020, "COMM Setting (Link Enable)" ) PORT_DIPLOCATION("SW:6")	\
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")			\
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )											\
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")						\
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )											\
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )

#define STANDARD_IN2																				\
	PORT_START("IN2")																				\
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )													\
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )									\
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("1P Select") PORT_CODE(KEYCODE_5) PORT_PLAYER(1)	\
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )									\
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("2P Select") PORT_CODE(KEYCODE_6) PORT_PLAYER(2)	\
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ng_aes_state, get_memcard_status, NULL)			\
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )  /* Matrimelee expects this bit to be active high when on an AES */

#define STANDARD_IN3																				\
	PORT_START("IN3")																				\
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )  /* Coin 1 - AES has no coin slots, it's a console */	\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )  /* Coin 2 - AES has no coin slots, it's a console */	\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )  /* Service Coin - not used, AES is a console */	\
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* having this ACTIVE_HIGH causes you to start with 2 credits using USA bios roms */	\
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* having this ACTIVE_HIGH causes you to start with 2 credits using USA bios roms */	\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) /* what is this? */								\
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state,get_calendar_status, NULL)			\
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state,get_audio_result, NULL) \

#define STANDARD_IN4																			\
	PORT_START("IN4")																			\
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )												\
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* what is this? - is 0 for 1 or 2 slot MVS (and AES?)*/							\
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter BIOS") PORT_CODE(KEYCODE_F2)	\
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

static INPUT_PORTS_START( controller )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)


	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
INPUT_PORTS_END

static INPUT_PORTS_START( mjpanel )
	PORT_START("MJ01_P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MJ02_P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MJ03_P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MJ04_P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MJ01_P2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MJ02_P2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MJ03_P2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)

	PORT_START("MJ04_P2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( aes )
// was there anyting in place of dipswitch in the home console?
/*  PORT_START("IN0")
    PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
//  PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" ) PORT_DIPLOCATION("SW:1")
//  PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
//  PORT_DIPNAME( 0x0002, 0x0002, "Coin Chutes?" ) PORT_DIPLOCATION("SW:2")
//  PORT_DIPSETTING(      0x0000, "1?" )
//  PORT_DIPSETTING(      0x0002, "2?" )
//  PORT_DIPNAME( 0x0004, 0x0000, "Mahjong Control Panel (or Auto Fire)" ) PORT_DIPLOCATION("SW:3")
//  PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
//  PORT_DIPNAME( 0x0018, 0x0018, "COMM Setting (Cabinet No.)" ) PORT_DIPLOCATION("SW:4,5")
//  PORT_DIPSETTING(      0x0018, "1" )
//  PORT_DIPSETTING(      0x0008, "2" )
//  PORT_DIPSETTING(      0x0010, "3" )
//  PORT_DIPSETTING(      0x0000, "4" )
//  PORT_DIPNAME( 0x0020, 0x0020, "COMM Setting (Link Enable)" ) PORT_DIPLOCATION("SW:6")
//  PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
//  PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")
//  PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
//  PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")
//  PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(aes_controller_r, NULL) */

	PORT_START("CTRLSEL")  /* Select Controller Type */
	PORT_CONFNAME( 0x0f, 0x01, "P1 Controller")
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x01, "NeoGeo Controller" )
	PORT_CONFSETTING(  0x02, "NeoGeo Mahjong Panel" )
	PORT_CONFNAME( 0xf0, 0x10, "P2 Controller")
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x10, "NeoGeo Controller" )
	PORT_CONFSETTING(  0x20, "NeoGeo Mahjong Panel" )

	PORT_INCLUDE( controller )

	PORT_INCLUDE( mjpanel )

// were some of these present in the AES?!?
	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/


static MACHINE_CONFIG_DERIVED_CLASS( aes, neogeo_base, ng_aes_state )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(aes_main_map) // some different input handling, probably also responsible for Riding Hero not running properly
	
	MCFG_MEMCARD_HANDLER(neogeo_aes)

	MCFG_MACHINE_START_OVERRIDE(ng_aes_state, neogeo)
	MCFG_MACHINE_RESET_OVERRIDE(ng_aes_state, neogeo)

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_LOAD(neo_cartridge)
	MCFG_CARTSLOT_INTERFACE("neo_cart")
	MCFG_CARTSLOT_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("cart_list","neogeo")
	MCFG_SOFTWARE_LIST_FILTER("cart_list","AES")
MACHINE_CONFIG_END


/* NeoCD uses custom vectors on IRQ4 to handle various events from the CDC */

static IRQ_CALLBACK(neocd_int_callback)
{
	ng_aes_state *state = device->machine().driver_data<ng_aes_state>();

	if (irqline==4)
	{
		if (state->get_nNeoCDIRQVectorAck()) {
			state->set_nNeoCDIRQVectorAck(0);
			return state->get_nNeoCDIRQVector();
		}
	}

	return (0x60+irqline*4)/4;
}

void ng_aes_state::interrupt_callback_type1(void)
{
	nIRQAcknowledge &= ~0x20;
	NeoCDIRQUpdate(0);
}

void ng_aes_state::interrupt_callback_type2(void)
{
	nIRQAcknowledge &= ~0x10;
	NeoCDIRQUpdate(0);
}

void ng_aes_state::interrupt_callback_type3(void)
{
	nIRQAcknowledge &= ~0x08;
	NeoCDIRQUpdate(0);
}


void ng_aes_state::NeoCDIRQUpdate(UINT8 byteValue)
{
	// do we also need to check the regular interrupts like FBA?

	nIRQAcknowledge |= (byteValue & 0x38);

	if ((nIRQAcknowledge & 0x08) == 0) {
		nNeoCDIRQVector = 0x17;
		nNeoCDIRQVectorAck = 1;
		machine().device("maincpu")->execute().set_input_line(4, HOLD_LINE);
		return;
	}
	if ((nIRQAcknowledge & 0x10) == 0) {
		nNeoCDIRQVector = 0x16;
		nNeoCDIRQVectorAck = 1;
		machine().device("maincpu")->execute().set_input_line(4, HOLD_LINE);
		return;
	}
	if ((nIRQAcknowledge & 0x20) == 0) {
		nNeoCDIRQVector = 0x15;
		nNeoCDIRQVectorAck = 1;
		machine().device("maincpu")->execute().set_input_line(4, HOLD_LINE);
		return;
	}
}

struct cdrom_interface neocd_cdrom =
{
	"neocd_cdrom",
	NULL
};

static MACHINE_CONFIG_DERIVED_CLASS( neocd, neogeo_base, ng_aes_state )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(neocd_main_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(neocd_audio_map)
	MCFG_CPU_IO_MAP(neocd_audio_io_map)

	// temporary until things are cleaned up
	MCFG_DEVICE_ADD("tempcdc", LC89510_TEMP, 0) // cd controller
	MCFG_SEGACD_HACK_SET_NEOCD
	MCFG_SET_TYPE1_INTERRUPT_CALLBACK( ng_aes_state, interrupt_callback_type1 )
	MCFG_SET_TYPE2_INTERRUPT_CALLBACK( ng_aes_state, interrupt_callback_type2 )
	MCFG_SET_TYPE3_INTERRUPT_CALLBACK( ng_aes_state, interrupt_callback_type3 )


	MCFG_MEMCARD_HANDLER(neogeo_aes)

	MCFG_MACHINE_START_OVERRIDE(ng_aes_state,neocd)
	MCFG_MACHINE_RESET_OVERRIDE(ng_aes_state,neocd)

	MCFG_CDROM_ADD( "cdrom",neocd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","neocd")
MACHINE_CONFIG_END

/*************************************
 *
 *  Driver initalization
 *
 *************************************/

DRIVER_INIT_MEMBER(ng_aes_state,neogeo)
{
}


ROM_START( aes )
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_SYSTEM_BIOS( 0, "jap-aes",   "Japan AES" )
	ROMX_LOAD("neo-po.bin",  0x00000, 0x020000, CRC(16d0c132) SHA1(4e4a440cae46f3889d20234aebd7f8d5f522e22c), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1))	/* AES Console (Japan) Bios */
	ROM_SYSTEM_BIOS( 1, "asia-aes",   "Asia AES" )
	ROMX_LOAD("neo-epo.bin", 0x00000, 0x020000, CRC(d27a71f1) SHA1(1b3b22092f30c4d1b2c15f04d1670eb1e9fbea07), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2))	/* AES Console (Asia?) Bios */
//  ROM_SYSTEM_BIOS( 2, "uni-bios_2_3","Universe Bios (Hack, Ver. 2.3)" )
//  ROMX_LOAD( "uni-bios_2_3.rom",  0x00000, 0x020000, CRC(27664eb5) SHA1(5b02900a3ccf3df168bdcfc98458136fd2b92ac0), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(3) ) /* Universe Bios v2.3 (hack) */

	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )

	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "ymsnd", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "ymsnd.deltat", ROMREGION_ERASEFF )

	ROM_REGION( 0x900000, "sprites", ROMREGION_ERASEFF )
ROM_END

ROM_START( neocd )
	ROM_REGION16_BE( 0x100000, "mainbios", 0 )
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



	ROM_REGION( 0x20000, "audiobios", ROMREGION_ERASEFF )
	ROM_REGION( 0x20000, "fixedbios", ROMREGION_ERASEFF )
	ROM_REGION( 0x20000, "zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
ROM_END

ROM_START( neocdz )
	ROM_REGION16_BE( 0x100000, "mainbios", 0 )
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




	ROM_REGION( 0x20000, "audiobios", ROMREGION_ERASEFF )
	ROM_REGION( 0x20000, "fixedbios", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "zoomy", 0 )
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )
ROM_END

/*    YEAR  NAME  PARENT COMPAT MACHINE INPUT  INIT     COMPANY      FULLNAME            FLAGS */
CONS( 1990, aes,    0,		0,   aes,      aes, ng_aes_state,   neogeo,  "SNK", "Neo-Geo AES", 0)

CONS( 1996, neocdz, 0,	    0,   neocd,    aes, ng_aes_state,   neogeo,  "SNK", "Neo-Geo CDZ", GAME_NOT_WORKING ) // the CDZ is the newer slot-loading model, faster drive etc.
CONS( 1994, neocd,  neocdz,	0,   neocd,    aes, ng_aes_state,   neogeo,  "SNK", "Neo-Geo CD", GAME_NOT_WORKING ) // older Top Loading model, ignores disc protections?
