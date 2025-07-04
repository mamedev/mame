// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Gals Panic II =-

                    driver by   Luca Elia (l.elia@tin.it)

CPU     :   2 x 68000  +  MCU
SOUND   :   2 x OKIM6295
OTHER   :   EEPROM
CUSTOM  :   ?

To Do:

- Simulation of the MCU: it sits between the 2 68000's and passes
    messages along. It is currently incomplete, thus no backgrounds
    and the game is unplayable

- The layers are offset


Gals Panic 2 (Korea)
Kaneko 1993

PCB Layout (for single large PCB version)
----------

Z04G2-004
|--------------------------------------------------------------------|
|LPF6K  GP2-104.U36* GP2-103.U62*  GP2-203.U102   GP2-201.U171       |
|PX4460  M6295         424260  34MHz      GP2-202.U121  GP2-200.U189 |
|   VOL  M6295                 20MHz      G204BK5.U170               |
| LA4460   GP2-100-K.U61      |------|    G204AK5.U169 GP2-204A.U188*|
|   DSW2   GP2-101.U60 424260 |KANEKO| 6264                          |
|   DSW1   GP2-102.U59        |KC-002| 6264            GP2-204B.U187*|
|                      D42101 |L0002 |                               |
|                             |------| TMP68HC000N-16    GROM26.U186*|
|J                                         G000K5.U165               |
|A          D431000                                      GROM20.U185*|
|M  MC-1091            62256               G001K5.U164               |
|M        |------|                                       GROM25.U184*|
|A        |KANEKO|     62256                                         |
|   6116  |KC-SHU|                                       GROM24.U183*|
|   6116  |L0003 |                                                   |
|         |------|                                      GP2-303B.U182|
| V-080D                                  62256                      |
| V-080D                       93C46      62256         GP2-303A.U181|
| V-080D   |------|            |------| |------| GROM22U.U161        |
|          |KANEKO|            |KANEKO| |KANEKO|        GP2-302B.U180|
| |------| |KC-TAS|  27MHz     |KC-SHU| |KC-001| GROM22L.U160        |
| |KANEKO| |L0005 |      16MHz |L0003 | |L0001 |        GP2-302A.U179|
| |KC-BYO| |------| |------|   |------| |------| GROM21U.U159        |
| |L0004 |          |KANEKO|  D431000 |------|        GP2-301B-K.U178|
| |------|          |KC-YUU|          |KANEKO|   GROM21L.U158        |
|          |------| |L0006 |          |KC-001|        GP2-301A-K.U177|
| D431000  |KANEKO| |------|          |L0001 |                       |
| D431000  |KC-BYO|                   |------|        GP2-300B-K.U176|
| 6116     |L0004 |                        |------|                  |
| 6116     |------|                D431000 |KANEKO|   GP2-300A-K.U175|
|          D431000  TMP68HC000N-16 D431000 |PISCES|                  |
|          D431000      G002K5.U64         |451   |     GROM10.U174* |
|          6116 6116    G003K5.U63         |------|                  |
|--------------------------------------------------------------------|
Notes:
           * - These ROMs not populated. Korean-specific ROMs have a K as part of the label text
       68000 - Clock 13.500MHz [27/2]
       M6295 - Clock 2.000MHz [16/8]. Pin 7 HIGH
      V-080D - Custom Kaneko RGB DAC
     MC-1091 - Custom Kaneko I/O module
      LFP-6K - Custom Kaneko sound filter/DAC
      PX4460 - Custom Kaneko sound filter/DAC
      PISCES - NEC uPD78324 series MCU with 32k internal rom. Clock 13.500MHz [27/2] on pins 51 & 52
       VSync - 59.1856Hz
       HSync - 15.625kHz

       (TODO: VTOTAL = 264, HTOTAL = 432, pixel clock 27 MHz / 4)

***************************************************************************/

#include "emu.h"
#include "galpani2.h"

#include "cpu/m68000/m68000.h"
#include "screen.h"
#include "speaker.h"

/***************************************************************************


                                    EEPROM


***************************************************************************/

uint16_t galpani2_state::galpani2_eeprom_r()
{
	return (m_eeprom_word & ~1) | (m_eeprom->do_read() & 1);
}

void galpani2_state::galpani2_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_eeprom_word );
	if ( ACCESSING_BITS_0_7 )
	{
		// latch the bit
		m_eeprom->di_write((data & 0x02) >> 1);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
	}
}


/***************************************************************************


                                MCU Simulation

100010.w    software watchdog?
100020.b    number of tasks for the mcu

***************************************************************************/

void galpani2_state::machine_start()
{
	uint8_t *ROM = memregion("subdata")->base();
	membank("subdatabank")->configure_entries(0, 0x2000000/0x0800000, ROM, 0x0800000);
	membank("subdatabank")->set_entry(0);

	save_item(NAME(m_eeprom_word));
	save_item(NAME(m_old_mcu_nmi1));
	save_item(NAME(m_old_mcu_nmi2));
}

void galpani2_state::machine_reset()
{
	machine().scheduler().perfect_quantum(attotime::from_usec(50)); //initial mcu xchk
}

static void galpani2_write_kaneko(cpu_device *cpu)
{
	address_space &dstspace = cpu->space(AS_PROGRAM);
	int i,x,tpattidx;
	unsigned char testpattern[] = {0xFF,0x55,0xAA,0xDD,0xBB,0x99};

	/* Write "KANEKO" to 100000-100005, but do not clash with ram test */

	x  = 0;

	for (i = 0x100000; i < 0x100007; i++)
	{
		for (tpattidx = 0; tpattidx < 6; tpattidx++)
		{
			if (dstspace.read_byte(i) == testpattern[tpattidx]) x = 1; //ram test fragment present
		}
	}

	if  ( x == 0 )
	{
		dstspace.write_byte(0x100000,0x4b); //K
		dstspace.write_byte(0x100001,0x41); //A
		dstspace.write_byte(0x100002,0x4e); //N
		dstspace.write_byte(0x100003,0x45); //E
		dstspace.write_byte(0x100004,0x4b); //K
		dstspace.write_byte(0x100005,0x4f); //O
	}
}

void galpani2_state::galpani2_mcu_init_w(uint8_t data)
{
	address_space &srcspace = m_maincpu->space(AS_PROGRAM);
	address_space &dstspace = m_subcpu->space(AS_PROGRAM);
	uint32_t mcu_address, mcu_data;

	for ( mcu_address = 0x100010; mcu_address < (0x100010 + 6); mcu_address += 1 )
	{
		mcu_data    =   srcspace.read_byte(mcu_address );
		dstspace.write_byte(mcu_address-0x10, mcu_data);
	}
	m_subcpu->set_input_line(INPUT_LINE_IRQ7, HOLD_LINE); //MCU Initialised
}

void galpani2_state::galpani2_mcu_nmi1()
{
	address_space &mspace = m_maincpu->space(AS_PROGRAM);
	address_space &sspace = m_subcpu->space(AS_PROGRAM);

	uint8_t len = mspace.read_byte(0x100020);
	for(uint8_t slot = 0; slot < len; slot += 4) {
		uint8_t command = mspace.read_byte(0x100021 + slot);
		uint32_t address = 0x100000 | mspace.read_word(0x100022 + slot);

		switch (command)
		{
		case 0x02: { //Copy N bytes from RAM2 to RAM1?, gp2se is the only one to use it, often!
			uint16_t src  = mspace.read_word(address + 2);
			uint16_t dst  = mspace.read_word(address + 6);
			uint16_t size = mspace.read_word(address + 8);
			logerror("MCU master %02x:%06x copy s:%04x, m:%04x, size:%04x\n", slot, address, src, dst, size);

			for(uint16_t i = 0; i != size; i++)
			{
				mspace.write_byte(0x100000 + dst, sspace.read_byte(0x100000 + src));
				src ++;
				dst ++;
			}
			break;
		}

		case 0x0a: { // Copy N bytes from RAM1 to RAM2
			uint16_t src  = mspace.read_word(address + 2);
			uint16_t dst  = mspace.read_word(address + 6);
			uint16_t size = mspace.read_word(address + 8);
			logerror("MCU master %02x:%06x copy m:%04x, s:%04x, size:%04x\n", slot, address, src, dst, size);

			for(uint16_t i = 0; i != size; i++)
			{
				sspace.write_byte(0x100000 + dst, mspace.read_byte(0x100000 + src));
				src ++;
				dst ++;
			}
			break;
		}

		//case 0x10: //? Clear gal?
		//case 0x14: //? Display gal?
		//until
		//case 0x50: //? Display gal?
		//case 0x68: //? Display "Changed" monster?
		//until
		//case 0x6E: //? Display "Changed" monster?
		//case 0x85: //? Do what?
		default:
			machine().debug_break();
			logerror("MCU master %02x:%04x: unknown command %02x\n", slot, address, command);
			break;
		}

		/* Raise a "job done" flag */
		if (command)
			mspace.write_word(address, 0xffff);
	}
	if (len)
		mspace.write_byte(0x100020, 0x00);
}

void galpani2_state::galpani2_mcu_nmi2()
{
	static const uint32_t imlist[794] = {
		0x000ccbf4, 0x000eac7e, 0x0010db66, 0x00125050, 0x0012b3b6, 0x00132140, 0x00137cae, 0x0013f256, 0x00144ac4, 0x0014dc5c, 0x00153bf8, 0x00158cae, 0x0015e5b2, 0x001672a6, 0x00170c5e, 0x0017306c,
		0x00173a88, 0x00174d44, 0x001759da, 0x0017693e, 0x001773f4, 0x0017802c, 0x0017abd6, 0x0017baca, 0x0017d37a, 0x0017e97e, 0x0017ff34, 0x00180cfe, 0x00181e5c, 0x00183be0, 0x001858b2, 0x00186dae,
		0x00187c82, 0x0018946a, 0x00189fdc, 0x0018abd2, 0x0018b7c8, 0x0018c6ec, 0x0018e7c4, 0x00190cba, 0x00191cb4, 0x00192d08, 0x00193fbc, 0x00196cc6, 0x00197f72, 0x00199400, 0x0019ab40, 0x0019b790,
		0x0019c974, 0x001a1c3a, 0x001a53d4, 0x001a615c, 0x001a6f70, 0x001a7e6c, 0x001a8a60, 0x001aad0c, 0x001acec6, 0x001ada66, 0x001ae690, 0x001afdb4, 0x001b1378, 0x001b1dbe, 0x001b2804, 0x001b3bcc,
		0x001b4648, 0x001b577e, 0x001b66fa, 0x001b76fa, 0x001b96fa, 0x001bb5e8, 0x001bc174, 0x001bce04, 0x001be434, 0x001bf3c2, 0x001bfeca, 0x001c0708, 0x001c1abc, 0x001c29a8, 0x001c38b0, 0x001c4a28,
		0x001c5ce6, 0x001c725a, 0x001c8bc0, 0x001c98ec, 0x001ca6d4, 0x001cb4de, 0x001cbe86, 0x001cc82a, 0x001cd1ce, 0x001cdb72, 0x001ce516, 0x001ceeba, 0x001cf85e, 0x001d03c2, 0x001d0f16, 0x001d1a0e,
		0x001d250c, 0x001d2fd2, 0x001d3ad6, 0x001d4574, 0x001d4f84, 0x001d5bae, 0x001d678e, 0x001d7340, 0x001d7dc2, 0x001d85f8, 0x001d8c30, 0x001d9220, 0x001d9be6, 0x001da5ac, 0x001daf72, 0x001db938,
		0x001dc198, 0x001dca00, 0x001dd600, 0x001dddec, 0x001de6c2, 0x001decdc, 0x001df6b0, 0x001e0140, 0x001e085e, 0x001e0d54, 0x001e16e2, 0x001e1ffa, 0x001e2d1c, 0x001e3700, 0x001e42de, 0x001e4b66,
		0x001e530a, 0x001e598e, 0x001e6052, 0x001e6da4, 0x001e753a, 0x001e7cfe, 0x001e84c2, 0x001e8b84, 0x001e96e6, 0x001ea9a8, 0x001eadfc, 0x001eaf72, 0x001eb2b8, 0x001ebe46, 0x001ec652, 0x001ecefe,
		0x001edbb8, 0x001ee46e, 0x001ef220, 0x001f02cc, 0x001f10fc, 0x001f1f9e, 0x001f2e42, 0x001f3d16, 0x001f531c, 0x001f62c2, 0x001f7900, 0x001f8840, 0x001f9236, 0x001f9dc8, 0x001faba0, 0x001fc168,
		0x001fc988, 0x001fd3de, 0x001fe5e0, 0x00200000, 0x0020ce1a, 0x00219cde, 0x0022394e, 0x0022d5e8, 0x0023d180, 0x0024cbe6, 0x00255490, 0x0025d494, 0x00265748, 0x0026cbf2, 0x0027d586, 0x0028d466,
		0x0029694a, 0x002a04f6, 0x002aee02, 0x002bdcc8, 0x002c6816, 0x002cfb1e, 0x002dec9a, 0x002ebc86, 0x002f6714, 0x00300f54, 0x0030c55e, 0x00317276, 0x00327f04, 0x00338c7c, 0x00346bd6, 0x00353482,
		0x0035a410, 0x00361a64, 0x0036d80a, 0x00378dfa, 0x0037d4cc, 0x0038394e, 0x0038bcb4, 0x00393c92, 0x003ab826, 0x003c33b2, 0x003d58ee, 0x003e6582, 0x003f2cac, 0x003fefbc, 0x0040a15e, 0x00414ae2,
		0x004198e0, 0x0041ea8c, 0x004307ce, 0x0044250c, 0x00450582, 0x0045d55a, 0x0046daa8, 0x0047ddca, 0x0048cea4, 0x0049bcfa, 0x004ac926, 0x004bcd98, 0x004c994c, 0x004db07c, 0x004eb484, 0x004fb774,
		0x00507246, 0x00516308, 0x005244bc, 0x00532684, 0x0053fce4, 0x0054d68c, 0x0055a036, 0x00566a2e, 0x00572e12, 0x0057f656, 0x0058a95e, 0x0059393c, 0x0059ee82, 0x005a8444, 0x005b1268, 0x005bbb30,
		0x005c5802, 0x005cec60, 0x005e1b74, 0x005ebfc0, 0x005f63ee, 0x00605150, 0x006114e4, 0x0061ccc0, 0x0062aa52, 0x006375d8, 0x006417d0, 0x006497f0, 0x0064f578, 0x0065618e, 0x0065d8fe, 0x00660b22,
		0x006648b2, 0x00668644, 0x0066c562, 0x00674776, 0x006819aa, 0x00690a12, 0x0069f4c4, 0x006adbea, 0x006bc27a, 0x006c86ce, 0x006d6b68, 0x006e500a, 0x006f3634, 0x00702412, 0x0070c8ca, 0x00716d44,
		0x007210d8, 0x0072b232, 0x00735254, 0x0073f1e0, 0x0074938a, 0x00753506, 0x0075d6de, 0x007678ba, 0x00771a4e, 0x0077bc10, 0x00787526, 0x0079325a, 0x0079f450, 0x007abc56, 0x007b84b4, 0x007c2e12,
		0x007cc098, 0x007d46cc, 0x007dd882, 0x007e447c, 0x007ef81e, 0x007fb22a, 0x0080252a, 0x0080a3b0, 0x00812c7e, 0x0081be46, 0x0082592a, 0x0082fb34, 0x0083b0da, 0x008480f6, 0x00850484, 0x00858c4e,
		0x00861422, 0x0086ca68, 0x008780ba, 0x008835ba, 0x0088eb36, 0x0089a21c, 0x008a58dc, 0x008b0d34, 0x008c01b8, 0x008cf94a, 0x008df31a, 0x008eefba, 0x008ff166, 0x0090f69c, 0x0091f344, 0x00929424,
		0x00937394, 0x009454de, 0x00954228, 0x0095fa9e, 0x00969674, 0x00972660, 0x00979952, 0x00987280, 0x00994baa, 0x009a24d0, 0x009afde8, 0x009bdbc6, 0x009cda1a, 0x009db8c8, 0x009eb4ee, 0x009f9726,
		0x00a08c62, 0x00a15874, 0x00a396a0, 0x00a5b598, 0x00a7a3d6, 0x00a9d562, 0x00ac03b0, 0x00ae1ae0, 0x00b0514a, 0x00b23ff4, 0x00b472fa, 0x00b693bc, 0x00b8d0be, 0x00bb1062, 0x00bcdb34, 0x00bed9ea,
		0x00bfbb04, 0x00c09b8a, 0x00c2104e, 0x00c36f8c, 0x00c4bf52, 0x00c5cfee, 0x00c64c4e, 0x00c6b144, 0x00c71e82, 0x00c7807a, 0x00c7ccaa, 0x00c83a78, 0x00c87fe4, 0x00c8df9e, 0x00c942c2, 0x00c98c5c,
		0x00cb5cee, 0x00cbecdc, 0x00cc7f04, 0x00cd18da, 0x00cdb400, 0x00ce5056, 0x00ceea2c, 0x00cf84b8, 0x00d01fde, 0x00d0baa8, 0x00d156fe, 0x00d1f0e8, 0x00d28b74, 0x00d3263e, 0x00d3c028, 0x00d464cc,
		0x00d50bb6, 0x00d5d5fe, 0x00d6a21e, 0x00d759b0, 0x00d823f8, 0x00d8f018, 0x00d9a7aa, 0x00da21be, 0x00daa136, 0x00db30aa, 0x00dbc0b6, 0x00dc5380, 0x00dce2f4, 0x00dd707c, 0x00de0088, 0x00de9352,
		0x00df20da, 0x00df8c16, 0x00dfff2e, 0x00e08d9e, 0x00e12642, 0x00e1caec, 0x00e2595c, 0x00e2f832, 0x00e390d6, 0x00e41cb0, 0x00e4c15a, 0x00e56030, 0x00e5ec0a, 0x00e634f0, 0x00e67f52, 0x00e6d362,
		0x00e72772, 0x00e785c6, 0x00e7e902, 0x00e850f8, 0x00e8e4a4, 0x00e98b5c, 0x00e9f352, 0x00ea97d8, 0x00eb2b84, 0x00eb9e9e, 0x00ec4556, 0x00ecbb98, 0x00ed601e, 0x00edd338, 0x00ee497a, 0x00eee790,
		0x00ef8642, 0x00f02f1e, 0x00f0d6b4, 0x00f18050, 0x00f2292c, 0x00f2cecc, 0x00f37662, 0x00f41a3a, 0x00f4c3d6, 0x00f564a2, 0x00f60a42, 0x00f6ae1a, 0x00f74ee6, 0x00f7c780, 0x00f8412e, 0x00f8bdba,
		0x00f93a00, 0x00f9b618, 0x00fa32a4, 0x00faad4c, 0x00fb2992, 0x00fba29e, 0x00fc1eb6, 0x00fc99b4, 0x00fd145c, 0x00fd8d68, 0x00fe0866, 0x00fe7b24, 0x00feebbc, 0x00ff7060, 0x00fff494, 0x0100821a,
		0x010106be, 0x01018af2, 0x01021878, 0x0102aaf0, 0x01034cce, 0x0104087c, 0x0104c42a, 0x010552fe, 0x0105d4e8, 0x01067b28, 0x01070f84, 0x0107aa12, 0x01085052, 0x0108ede2, 0x0109823e, 0x010a1d3e,
		0x010ab7cc, 0x010b5764, 0x010bf4f4, 0x010c8ff4, 0x010d2f8c, 0x010da00e, 0x010e0d58, 0x010ea416, 0x010f3ad4, 0x010fad1c, 0x011020ca, 0x0110aea6, 0x01113620, 0x0111d4d8, 0x011262b4, 0x0112fe58,
		0x011385d2, 0x0114182c, 0x0114b6e4, 0x01155118, 0x0115ecbc, 0x01167f16, 0x0117194a, 0x0117a9d4, 0x01184160, 0x0118d94a, 0x011973c8, 0x011a1ed2, 0x011ab6bc, 0x011b610c, 0x011bfb8a, 0x011ca956,
		0x011d5460, 0x011df7d0, 0x011ea220, 0x011f4fec, 0x011ff35c, 0x0120ace0, 0x0121810a, 0x01226602, 0x01238036, 0x01249b06, 0x012551d4, 0x0125f67c, 0x0126b690, 0x0126dccc, 0x0126e77e, 0x0126f57a,
		0x01279d4c, 0x012880a0, 0x01292630, 0x0129e3bc, 0x012a89ae, 0x012b6660, 0x012c5a90, 0x012d3c5e, 0x012df5ca, 0x012eaf86, 0x012f6a48, 0x01305a94, 0x0131af60, 0x0132e3d8, 0x01337bb8, 0x01346216,
		0x0134c896, 0x01355652, 0x0135cf0c, 0x013647ca, 0x01372338, 0x01375ff2, 0x0137afc0, 0x01383e88, 0x0139825a, 0x013b9cb6, 0x013dd902, 0x013ff79a, 0x0141d440, 0x0141e0b0, 0x0141f032, 0x0141fefc,
		0x01420cdc, 0x01421920, 0x01422ac4, 0x014239d8, 0x01424ad6, 0x01426476, 0x01428038, 0x014294f4, 0x0142a6b4, 0x0142af38, 0x0142bd48, 0x0142cd52, 0x0142df28, 0x0142f07a, 0x0143072c, 0x01432662,
		0x014344e2, 0x01437346, 0x0143a38c, 0x0143b5da, 0x0143cb60, 0x0143e350, 0x0143f852, 0x01440b3e, 0x014420a8, 0x0144338a, 0x01444358, 0x01444f24, 0x01446fa8, 0x01448d84, 0x0144ac96, 0x0144ce78,
		0x0144e3d0, 0x0144eb2e, 0x0144f2f0, 0x0144faac, 0x014502c4, 0x01450b96, 0x014519f8, 0x0145315a, 0x01454882, 0x01455e32, 0x014573be, 0x014588cc, 0x01459dda, 0x0145b2da, 0x0145d020, 0x0145e690,
		0x0145faec, 0x01460a0e, 0x01462416, 0x01463636, 0x01464894, 0x01465d24, 0x0146749e, 0x01469062, 0x0146ac32, 0x0146ca82, 0x0146e9ea, 0x01470ba8, 0x01473c36, 0x01474f10, 0x014762a4, 0x014779f8,
		0x01478bc4, 0x01479d8a, 0x0147af50, 0x0147c116, 0x0147d2dc, 0x0147e4a2, 0x01481bd6, 0x01482cf2, 0x014841ac, 0x014858c6, 0x014879de, 0x0148a1b4, 0x0148ae88, 0x0148d57a, 0x01490290, 0x014930c4,
		0x01495580, 0x01496a54, 0x0149765c, 0x01498c4a, 0x0149a166, 0x0149b68e, 0x0149cbf6, 0x0149d82c, 0x0149f176, 0x014a017e, 0x014a17ea, 0x014a1d20, 0x014a1eae, 0x014a220c, 0x014a3240, 0x014a4008,
		0x014a511c, 0x014a6620, 0x014a771e, 0x014a8ada, 0x014a9d06, 0x014aaf34, 0x014ac64a, 0x014acdee, 0x014ad592, 0x014addb8, 0x014ae69e, 0x014af186, 0x014afc22, 0x014b032c, 0x014b0a44, 0x014b0d9a,
		0x014b10da, 0x014b1fc4, 0x014b2e7e, 0x014b3992, 0x014b48d8, 0x014b5056, 0x014b5834, 0x014b5fac, 0x014b67d6, 0x014b7bc6, 0x014b8340, 0x014b8e2e, 0x014b991c, 0x014ba0da, 0x014ba786, 0x014bb710,
		0x014bc7c8, 0x014bd046, 0x014bd7ac, 0x014be204, 0x014bef08, 0x014bf9de, 0x014c04f0, 0x014c0d6a, 0x014c15ca, 0x014c2266, 0x014c2e62, 0x014c3522, 0x014c3be2, 0x014c462a, 0x014c5096, 0x014c5f02,
		0x014c6a66, 0x014c71d0, 0x014c7848, 0x014c82ac, 0x014c8d10, 0x014c9422, 0x014c9b34, 0x014ca48e, 0x014cae3a, 0x014cb748, 0x014cc056, 0x014cc702, 0x014ccd6a, 0x014cd85e, 0x014ce322, 0x014ced58,
		0x014cfe4a, 0x014d0b68, 0x014d16ba, 0x014d1ffe, 0x014d2952, 0x014d350c, 0x014d40c6, 0x014d480e, 0x014d505c, 0x014d5db2, 0x014d6b0c, 0x014d7542, 0x014d7f9c, 0x014d8998, 0x014d904c, 0x014d9758,
		0x014d9e88, 0x014da878, 0x014db76a, 0x014dbd3c, 0x014dc724, 0x014dd248, 0x014dd812, 0x014de390, 0x014ded12, 0x014df474, 0x014dfd30, 0x014e05ec, 0x014e0d4a, 0x014e18a2, 0x014e1e0e, 0x014e2dfc,
		0x014e402c, 0x014e54fe, 0x014e5b52, 0x014e62e8, 0x014e6a80, 0x014e71de, 0x014e799a, 0x014e827c, 0x014e8e6a, 0x014e99a4, 0x014ea49c, 0x014eaf4c, 0x014eb9d8, 0x014ec6b4, 0x014ed3c8, 0x014edfc8,
		0x014eed5e, 0x014ef1c2, 0x014ef61e, 0x014efa7a, 0x014efed6, 0x014f02f4, 0x014f0750, 0x014f1212, 0x014f1cd4, 0x014f2796, 0x014f3258, 0x014f3d1c, 0x014f47e0, 0x014f56dc, 0x014f65b0, 0x014f7476,
		0x014f8360, 0x014f925e, 0x014f9cb0, 0x014fa518, 0x014fad36, 0x014fbb0e, 0x014fca76, 0x014fd416, 0x014fe15e, 0x014fe914
	};

	address_space &sspace = m_subcpu->space(AS_PROGRAM);

	uint8_t len = sspace.read_byte(0x101012);
	for(uint8_t slot = 0; slot < len; slot += 4) {
		uint8_t command = sspace.read_byte(0x101013 + slot);
		uint32_t address = 0x100000 | sspace.read_word(0x101014 + slot);

		switch (command)
		{
		case 0x0c: {
			uint16_t img = sspace.read_word(address);
			uint32_t iadr = img < 794 ? imlist[img] : 0;
			logerror("MCU slave %02x:%06x: image address lookup %04x -> %08x\n", slot, address, img, iadr);
			sspace.write_dword(address+2, iadr);
			break;
		}

		default:
			machine().debug_break();
			logerror("MCU slave %02x:%06x: unknown command %02x\n", slot, address, command);
			break;
		}

		/* Raise a "job done" flag */
		if (command)
			sspace.write_word(address, 0xffff);
	}
	if (len)
		sspace.write_byte(0x101012, 0x00);

	galpani2_write_kaneko(m_maincpu);
	//logerror("%s : MCU executes CHECKs synchro\n", machine().describe_context());
}

void galpani2_state::galpani2_mcu_nmi1_w(uint8_t data) //driven by CPU1's int5 ISR
{
//for galpan2t:
//Triggered from 'maincpu' (00007D60),once, with no command, using alternate line, during init
//Triggered from 'maincpu' (000080BE),once, for unknown command, during init
//Triggered from 'maincpu' (0000741E),from here on...driven by int5, even if there's no command
	if ( (data & 1) && !(m_old_mcu_nmi1 & 1) )  galpani2_mcu_nmi1();
	//if ( (data & 0x10) && !(m_old_mcu_nmi1 & 0x10) )    galpani2_mcu_nmi1(machine());
	//alternate line, same function?
	m_old_mcu_nmi1 = data;
}

void galpani2_state::galpani2_mcu_nmi2_w(uint8_t data) //driven by CPU2's int5 ISR
{
	if ( (data & 1) && !(m_old_mcu_nmi2 & 1) )  galpani2_mcu_nmi2();
	m_old_mcu_nmi2 = data;
}


/***************************************************************************


                            CPU#1 - Main + Sound


***************************************************************************/

void galpani2_state::galpani2_coin_lockout_w(uint8_t data)
{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);
		machine().bookkeeping().coin_lockout_w(0,~data & 0x04);
		machine().bookkeeping().coin_lockout_w(1,~data & 0x08);
		// & 0x10     CARD in lockout?
		// & 0x20     CARD in lockout?
		// & 0x40     CARD out
}

void galpani2_state::galpani2_oki1_bank_w(uint8_t data)
{
	uint8_t *ROM = memregion("oki1")->base();
	logerror("%s : %s bank %08X\n",machine().describe_context(),tag(),data);
	memcpy(ROM + 0x30000, ROM + 0x40000 + 0x10000 * (~data & 0xf), 0x10000);
}

void galpani2_state::galpani2_oki2_bank_w(uint8_t data)
{
	m_oki2->set_rom_bank(data & 0xf);
	logerror("%s : %s bank %08X\n",machine().describe_context(),tag(),data);
}


void galpani2_state::galpani2_mem1(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                  // ROM
	map(0x100000, 0x10ffff).ram().share("ram");                     // Work RAM
	map(0x110000, 0x11000f).ram();                                  // ? corrupted? stack dumper on POST failure, pc+sr on gp2se
	map(0x300000, 0x301fff).ram();                                  // ?
	map(0x302000, 0x303fff).ram().share("spriteram");               // Sprites
	map(0x304000, 0x30401f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
//  map(0x308000, 0x308001).nopw();                                 // ? 0 at startup
	map(0x30c000, 0x30c001).nopw();                                 // ? hblank effect ?
	map(0x310000, 0x3101ff).ram().w(m_bg8palette, FUNC(palette_device::write16)).share("bg8palette");    // ?
	map(0x314000, 0x314001).nopw();                                 // ? flip backgrounds ?
	map(0x318000, 0x318001).rw(FUNC(galpani2_state::galpani2_eeprom_r), FUNC(galpani2_state::galpani2_eeprom_w)); // EEPROM
	// TODO: writes fadeout palettes for gal select after selection!?
	map(0x380000, 0x387fff).ram();                                  // Palette?
	map(0x388000, 0x38ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");   // Palette
//  map(0x390000, 0x3901ff).nopw();                                 // ? at startup of service mode

	map(0x400000, 0x43ffff).ram().share("bg8.0");                   // Background 0
	map(0x440000, 0x440001).ram().share("bg8_scrolly.0");           // Background 0 Scroll Y
	map(0x480000, 0x480001).ram().share("bg8_scrollx.0");           // Background 0 Scroll X
//  map(0x4c0000, 0x4c0001).nopw();                                 // ? 0 at startup only
	map(0x500000, 0x53ffff).ram().share("bg8.1");                   // Background 1
	map(0x540000, 0x540001).ram().share("bg8_scrolly.1");           // Background 1 Scroll Y
	map(0x580000, 0x580001).ram().share("bg8_scrollx.1");           // Background 1 Scroll X
//  map(0x5c0000, 0x5c0001).nopw();                                 // ? 0 at startup only

	map(0x540572, 0x540573).nopr();                                         // ? galpani2 at F0A4
	map(0x54057a, 0x54057b).nopr();                                         // ? galpani2 at F148
	map(0x54059a, 0x54059b).nopr();                                         // ? galpani2 at F0A4
	map(0x5405a2, 0x5405a3).nopr();                                         // ? galpani2 at F0A4 and F148
	map(0x5405aa, 0x5405ab).nopr();                                         // ? galpani2 at F0A4 and F148
	map(0x5405b2, 0x5405b3).nopr();                                         // ? galpani2 at F0A4 and F148
	map(0x5405ba, 0x5405bb).nopr();                                         // ? galpani2 at F0A4 and F148
	map(0x5405c2, 0x5405c3).nopr();                                         // ? galpani2 at F0A4 and F148
	map(0x5405ca, 0x5405cb).nopr();                                         // ? galpani2 at F148

	map(0x600000, 0x600001).noprw();                                        // Watchdog
	map(0x640001, 0x640001).w(FUNC(galpani2_state::galpani2_mcu_init_w));   // ? 0 before resetting and at startup, Reset mcu ?
	map(0x680001, 0x680001).w(FUNC(galpani2_state::galpani2_mcu_nmi1_w));             // ? 0 -> 1 -> 0 (lev 5) / 0 -> $10 -> 0
	map(0x6c0000, 0x6c0000).w(FUNC(galpani2_state::galpani2_coin_lockout_w));   // Coin + Card Lockout
	map(0x780000, 0x780001).portr("DSW1_P1");
	map(0x780002, 0x780003).portr("DSW2_P2");
	map(0x780004, 0x780005).portr("SPECIAL");
	map(0x780006, 0x780007).portr("SERVICE");
	map(0xc00001, 0xc00001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // 2 x OKIM6295
	map(0xc40001, 0xc40001).rw(m_oki2, FUNC(okim6295_device::read), FUNC(okim6295_device::write));   //
	map(0xc80001, 0xc80001).w(FUNC(galpani2_state::galpani2_oki1_bank_w));   //
	map(0xcc0001, 0xcc0001).w(FUNC(galpani2_state::galpani2_oki2_bank_w));   //
}


/***************************************************************************


                            CPU#2 - Backgrounds


***************************************************************************/


void galpani2_state::subdatabank_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data &= mem_mask;

	if (data & 0xfffc) printf("subdatabank_select_w %04x\n", data);
	membank("subdatabank")->set_entry(data&3);
}


void galpani2_state::galpani2_mem2(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                                             // ROM
	map(0x100000, 0x13ffff).ram().share("ram2");                                        // Work RAM
	map(0x400000, 0x5fffff).ram().share("bg15");  // bg15
//  map(0x600000, 0x600001).noprw();                               // ? 0 at startup only
//  map(0x640000, 0x640001).nopw();                                // ? 0 at startup only
//  map(0x680000, 0x680001).nopw();                                // ? 0 at startup only
//  map(0x6c0000, 0x6c0001).nopw();                                // ? 0 at startup only
	map(0x700000, 0x700001).noprw();                               // Watchdog
//  map(0x740000, 0x740001).nopw();                                // ? Reset mcu
	map(0x780001, 0x780001).w(FUNC(galpani2_state::galpani2_mcu_nmi2_w));    // ? 0 -> 1 -> 0 (lev 5)
	map(0x7c0000, 0x7c0001).w(FUNC(galpani2_state::subdatabank_select_w));   // Rom Bank
	map(0x800000, 0xffffff).bankr("subdatabank");
}

/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( galpani2 )
	PORT_START("DSW1_P1")   /* 780000.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW B:8,7,6")
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0004, "Normal +" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0001, "Ultra Hard" )
	PORT_DIPSETTING(      0x0000, "God Hands" )
	PORT_DIPNAME( 0x0008, 0x0008, "Picture Mode" )          PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Adult" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW B:4,3")
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Max Unit of Players" )       PORT_DIPLOCATION("SW B:2,1")
	PORT_DIPSETTING(      0x00c0, "9" )
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0040, "4" )
	PORT_DIPSETTING(      0x0000, "6" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("DSW2_P2")   /* 780002.w */
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW A:8,7,6,5")
	PORT_DIPSETTING(      0x000f, "1 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x000e, "2 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x000d, "3 Coin/1 Credit  3/1" )
	PORT_DIPSETTING(      0x000c, "4 Coin/1 Credit  4/1" )
	PORT_DIPSETTING(      0x000b, "5 Coin/1 Credit  5/1" )
	PORT_DIPSETTING(      0x000a, "2 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0009, "3 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0008, "4 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0007, "5 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0006, "2 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0005, "3 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0004, "4 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0003, "5 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0002, "1 Coin/2 Credit  1/2" )
	PORT_DIPSETTING(      0x0001, "1 Coin/3 Credit  1/3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Card Dispenser" )        PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(      0x0010, "Used" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW A:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW A:1" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("SPECIAL")   /* 780004.w */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM )  // CARD full
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_CUSTOM )  // CARD full
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM )  // CARD empty

	PORT_START("SERVICE")   /* 780006.w */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2  ) // this button is used in gp2se as an alt way to bring up the service menu, booting with it held down breaks the game tho!
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	//missing "test" input
INPUT_PORTS_END

static INPUT_PORTS_START( gp2se )
	PORT_INCLUDE( galpani2 )

	PORT_MODIFY("DSW1_P1")  /* 780000.w */
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW B:5" ) // picture mode is "normal fix" and cannot be changed

	PORT_MODIFY("DSW2_P2")  /* 780002.w */
	PORT_DIPNAME( 0x0010, 0x0010, "Card Dispenser" )        PORT_DIPLOCATION("SW A:4") // Reversed compared to other sets.
	PORT_DIPSETTING(      0x0000, "Used" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Unused ) )

	//missing "test" input
INPUT_PORTS_END

/***************************************************************************


                                Machine Drivers


***************************************************************************/


/* CPU#1 Interrups , lev 3,4 & 5 are tested on power up. The rest is rte, but lev 6 */
TIMER_DEVICE_CALLBACK_MEMBER(galpani2_state::galpani2_interrupt1)
{
	int scanline = param;

	if(scanline == 240)
			m_maincpu->set_input_line(5, HOLD_LINE);

	/* MCU related? */
	if(scanline == 128)
	{
		m_maincpu->set_input_line(3, HOLD_LINE);
		m_maincpu->set_input_line(4, HOLD_LINE);
	}

	if(scanline == 0)
			m_maincpu->set_input_line(6, HOLD_LINE); // hblank?
}

/* CPU#2 interrupts, lev 3,4 & 5 are tested on power up. The rest is rte, but lev 7 */
TIMER_DEVICE_CALLBACK_MEMBER(galpani2_state::galpani2_interrupt2)
{
	int scanline = param;

	if(scanline == 240)
		m_subcpu->set_input_line(5, HOLD_LINE);

	if(scanline == 128)
		m_subcpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0)
		m_subcpu->set_input_line(3, HOLD_LINE);
}

void galpani2_state::galpani2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(27'000'000)/2);       /* Confirmed on galpani2i PCB */
	m_maincpu->set_addrmap(AS_PROGRAM, &galpani2_state::galpani2_mem1);
	TIMER(config, "m_scantimer").configure_scanline(FUNC(galpani2_state::galpani2_interrupt1), "screen", 0, 1);
	//config.m_perfect_cpu_quantum = subtag("maincpu");

	M68000(config, m_subcpu, XTAL(27'000'000)/2);           /* Confirmed on galpani2i PCB */
	m_subcpu->set_addrmap(AS_PROGRAM, &galpani2_state::galpani2_mem2);
	TIMER(config, "s_scantimer").configure_scanline(FUNC(galpani2_state::galpani2_interrupt2), "screen", 0, 1);

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 256);
	screen.set_visarea(0, 320-1, 0, 256-1-16);
	screen.set_screen_update(FUNC(galpani2_state::screen_update_galpani2));

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 0x4000); // sprites
	PALETTE(config, m_bg8palette).set_format(palette_device::xGRB_555, 0x200 / 2); // bg8
	PALETTE(config, m_bg15palette, palette_device::GRB_555); // 32768 static colors for the bg

	KANEKO_KC002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_offsets(0x10000 - 0x16c0 + 0xc00, 0);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	OKIM6295(config, "oki1", XTAL(20'000'000)/10, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);    /* Confirmed on galpani2i PCB */

	OKIM6295(config, m_oki2, XTAL(20'000'000)/10, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);    /* Confirmed on galpani2i PCB */
}


/***************************************************************************


                                Roms Loading


***************************************************************************/

//POST's displayed checksums (ROM $0-$FFFFF) or (ROM $0-$FFFFF even)/(ROM $1-$FFFFF odd)
//
//galpani2 = 6A6C                bkg layer offset
//galpani2g= 15A3                bkg layer offset
//galpani2i= 54FC                bkg layer offset
//galpani2t= 18A3                bkg layer offset
//galpani2k= DE53                bkg layer offset
//galpani2j= 08E1 / A582         bkg layer OK          has demo
//gp2quiz  = 78D6 / 84A0         bkg layer OK          has demo
//gp2se    = 6D8C / FCE4 (Japan) bkg layer unknown

/***************************************************************************

                                Gals Panic II


Location      Device         File ID       Checksum
---------------------------------------------------
CPU U134      27C4001      G001T1-U134-0     21A3   [ CPU 2 PROG ]
CPU U125       27C010      G002T1-U125-0     8072   [ CPU 1 PROG ]
CPU U126       27C010      G003T1-U126-0     C9C4   [ CPU 1 PROG ]
CPU U133      27C4001      G204T1-U133-0     9EF7   [ CPU 2 PROG ]
ROM U27        27C020      G204T1-U27-00     CA1D
ROM U33        27C020      G204T1-U33-00     AF5D
ROM U3       LH538500      GP2-100-0043      C90A   [ SOUND DATA ]
ROM U7       LH538500      GP2-101-0044      CFEF   [ SOUND DATA ]
ROM U10     KM2316000      GP2-102-0045      1558   [ SOUND DATA ]
ROM U21      LM538500      GP2-200-0046      2E2E
ROM U20      LH538500      GP2-201-0047      DB6E
ROM U19      LH538500      GP2-202-0048      E181
ROM U18      LH538500      GP2-203-0049      E520
ROM U51      LH538500      GP2-300A-0052     50E9
ROM U52      LH538500      GP2-300B-0053     DC51
ROM U60     KM2316000      GP2-301-0035      35F6
ROM U59     KM2316000      GP2-302-0036      BFF5
ROM U58     KM2316000      GP2-303-0037      B860
ROM U57     KM2316000      GP2-304-0038      BD55
ROM U56     KM2316000      GP2-305-0039      D0F4
ROM U55     KM2316000      GP2-306-0040      1311
ROM U54     KM2316000      GP2-307-0041      1874
ROM U53     KM2316000      GP2-308-0042      375F
ROM U46      LH538500      GP2-309A-0050     97ED
ROM U47      LH538500      GP2-309B-0051     2C13
ROM U48      LH538500      GP2-309A-0055     2059
ROM U75      GAL16V8A      S075.JED          08F9
ROM U76      GAL16V8A      S076.JED          0878
ROM U1      PEEL18CV8      S001.JED          03CA
ROM U14     PEEL18CV8      S014.JED          039A


Notes:  CPU - Main PCB   Z04G2-003
        ROM - ROM PCB    Z04G2-SUB3

        Checksums for the PLDs are the JEDEC checksums, not the file checksums

Brief Hardware Overview
-----------------------

CPU1         - 68HC000-16
CPU2         - 68HC000-16
Sound     2x - M6295

Custom ICs   - 10x PQFPs

***************************************************************************/

ROM_START( galpani2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000a2.u165-1", 0x000000, 0x080000, CRC(0c6dfe3f) SHA1(22b16eaa3fee7f8f8434c6775255b25c8d960620) )
	ROM_LOAD16_BYTE( "g001a2.u164-1", 0x000001, 0x080000, CRC(b3a5951f) SHA1(78cf2d85a8b3cd46c5e30fd13b474af2ed2ee09b) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002a2.u64-1", 0x000000, 0x020000, CRC(c0b94eaf) SHA1(4f3a65b238b31ee8d256b7025253f01eaf6e55d5) )
	ROM_LOAD16_BYTE( "g003a2.u63-1", 0x000001, 0x020000, CRC(0d30725d) SHA1(d4614f9ffb930c4ea36cb3fbacffe63060e92402) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASE00 )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035",  0x0380000, 0x080000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_CONTINUE(             0x0300000, 0x080000)
	ROM_CONTINUE(             0x0280000, 0x080000)
	ROM_CONTINUE(             0x0200000, 0x080000)
	ROM_LOAD( "gp2-302.036",  0x0580000, 0x080000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_CONTINUE(             0x0500000, 0x080000)
	ROM_CONTINUE(             0x0480000, 0x080000)
	ROM_CONTINUE(             0x0400000, 0x080000)
	ROM_LOAD( "gp2-303.037",  0x0780000, 0x080000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_CONTINUE(             0x0700000, 0x080000)
	ROM_CONTINUE(             0x0680000, 0x080000)
	ROM_CONTINUE(             0x0600000, 0x080000)
	ROM_LOAD( "gp2-304.038",  0x0980000, 0x080000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_CONTINUE(             0x0900000, 0x080000)
	ROM_CONTINUE(             0x0880000, 0x080000)
	ROM_CONTINUE(             0x0800000, 0x080000)
	ROM_LOAD( "gp2-305.039",  0x0b80000, 0x080000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_CONTINUE(             0x0b00000, 0x080000)
	ROM_CONTINUE(             0x0a80000, 0x080000)
	ROM_CONTINUE(             0x0a00000, 0x080000)
	ROM_LOAD( "gp2-306.040",  0x0d80000, 0x080000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_CONTINUE(             0x0d00000, 0x080000)
	ROM_CONTINUE(             0x0c80000, 0x080000)
	ROM_CONTINUE(             0x0c00000, 0x080000)
	ROM_LOAD( "gp2-307.041",  0x0f80000, 0x080000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_CONTINUE(             0x0f00000, 0x080000)
	ROM_CONTINUE(             0x0e80000, 0x080000)
	ROM_CONTINUE(             0x0e00000, 0x080000)
	ROM_LOAD( "gp2-308.042",  0x1180000, 0x080000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_CONTINUE(             0x1100000, 0x080000)
	ROM_CONTINUE(             0x1080000, 0x080000)
	ROM_CONTINUE(             0x1000000, 0x080000)
	ROM_LOAD16_WORD_SWAP( "gp2-309a.050", 0x1280000, 0x080000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_CONTINUE(             0x1200000, 0x080000)
	ROM_LOAD( "gp2-309b.051", 0x1380000, 0x080000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_CONTINUE(             0x1300000, 0x080000)
	ROM_LOAD( "gp2-310a.055", 0x1480000, 0x080000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )
	ROM_CONTINUE(             0x1400000, 0x080000)

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.188", 0x400000, 0x080000, CRC(613ad1d5) SHA1(0ea1d4306c3e1eca3d207be2f72214fb36db0d75) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

ROM_START( galpani2e )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000a2.u165-1", 0x000000, 0x080000, CRC(0c6dfe3f) SHA1(22b16eaa3fee7f8f8434c6775255b25c8d960620) )
	ROM_LOAD16_BYTE( "g001a2.u164-1", 0x000001, 0x080000, CRC(b3a5951f) SHA1(78cf2d85a8b3cd46c5e30fd13b474af2ed2ee09b) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002a1-u125-1.bin", 0x000000, 0x020000, CRC(100e76b3) SHA1(24a259ee427cd7a6e487520a712dc7ef632dc5d6) )
	ROM_LOAD16_BYTE( "g003a1-u126-1.bin", 0x000001, 0x020000, CRC(0efe7835) SHA1(c7eecacdf101c0515da504cc77512f27b61b2ab7) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.188", 0x400000, 0x080000, CRC(613ad1d5) SHA1(0ea1d4306c3e1eca3d207be2f72214fb36db0d75) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

// from Single board PCB
ROM_START( galpani2i )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000i1u165", 0x000000, 0x080000, CRC(b802ad64) SHA1(a0bef1f037a72c379f43ff6d22e441d988b68fcc) )
	ROM_LOAD16_BYTE( "g001i1u164", 0x000001, 0x080000, CRC(d342fe5c) SHA1(7add3488d1e8eaec9e1f5cc47e4e7147822923bc) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002a2.u64-1", 0x000000, 0x020000, CRC(c0b94eaf) SHA1(4f3a65b238b31ee8d256b7025253f01eaf6e55d5) )
	ROM_LOAD16_BYTE( "g003a2.u63-1", 0x000001, 0x020000, CRC(0d30725d) SHA1(d4614f9ffb930c4ea36cb3fbacffe63060e92402) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.u188", 0x400000, 0x080000, CRC(ba83c918) SHA1(04a70dc7e33d853d84b88dc82c9b066696475cee) ) // different from Asia set

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	ROM_LOAD( "g104i1u062-0", 0x140000, 0x080000, CRC(117ee59e) SHA1(7deb9b71363ff0bf239f9ad21171ddd9bfc49eb4) )    // $8 x $10000, 1st is just audio data, no header (Italian samples data)

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

// Single board PCB
ROM_START( galpani2gs ) // basically the same as the Italy set but with different region byte and German sample rom
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000g1.u165-2", 0x000000, 0x080000, CRC(9f6952bb) SHA1(d7dde397589978ec88958ce191570d6aa5789bc6) )
	ROM_LOAD16_BYTE( "g001g1.u164-2", 0x000001, 0x080000, CRC(d342fe5c) SHA1(7add3488d1e8eaec9e1f5cc47e4e7147822923bc) ) // == italian set

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g003g1.u65-2", 0x000000, 0x020000, CRC(c0b94eaf) SHA1(4f3a65b238b31ee8d256b7025253f01eaf6e55d5) )
	ROM_LOAD16_BYTE( "g002g1.u64-2", 0x000001, 0x020000, CRC(0d30725d) SHA1(d4614f9ffb930c4ea36cb3fbacffe63060e92402) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.u188", 0x400000, 0x080000, CRC(ba83c918) SHA1(04a70dc7e33d853d84b88dc82c9b066696475cee) ) // different from Asia set

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	ROM_LOAD( "g104g1.u62-0", 0x140000, 0x080000, CRC(03539013) SHA1(36b96c59c59d0e747eb000472c22a30de0810902) )   // $8 x $10000, 1st is just audio data, no header (German sample data)

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

ROM_START( galpani2g )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000g1.u133-0", 0x000000, 0x080000, CRC(5a9c4886) SHA1(6fbc443612e72bafc5cac30de78c72815db20c4c) )  // German version specific
	ROM_LOAD16_BYTE( "g001g1.u134-0", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )  // same as other 2 PCB versions

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002t1.125", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003t1.126", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD16_BYTE( "g300a0.u44-00", 0x0000000, 0x080000, CRC(50406294) SHA1(fc1165b7b31a44ab204cd5ac3e7b2733ed6b1534) )
	ROM_LOAD16_BYTE( "g300a1.u41-00", 0x0000001, 0x080000, CRC(d26b7c4f) SHA1(b491170010977ba1e5111893937cc6bab0539e7d) )
	ROM_LOAD16_BYTE( "g300b0.u45-00", 0x0100000, 0x080000, CRC(9637934c) SHA1(d3b39d9f44825bdf24d4aa39ca32035bc5af4905) )
	ROM_LOAD16_BYTE( "g300b1.u42-00", 0x0100001, 0x080000, CRC(d72e154b) SHA1(e367c8f9af47b999fcba4afcd293565bad2038ec) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204a0.u33-00", 0x400000, 0x040000, CRC(2867cbfd) SHA1(89af600fb33ce72a7a3fbdf9ff05a4916454a205) )
	ROM_LOAD16_BYTE( "g204a1.u27-00", 0x400001, 0x040000, CRC(c50503bc) SHA1(5003aa414660358900857901d5e9eca6739f14e3) )

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	ROM_LOAD( "g104g1.u04-00", 0x140000, 0x080000, CRC(03539013) SHA1(36b96c59c59d0e747eb000472c22a30de0810902) )   // $8 x $10000, 1st is just audio data, no header (German sample data)

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END


ROM_START( galpani2i2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000i1-u133-0.u133", 0x000000, 0x080000, CRC(7df7b759) SHA1(2479a6389649ee6042b175b71d7ed54bc116add5) )   // Italian version specific
	ROM_LOAD16_BYTE( "g001i1-u134-0.u134", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )   // same as other 2 PCB versions

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002i1.125", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003i1.126", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD16_BYTE( "g300a0.u44-00", 0x0000000, 0x080000, CRC(50406294) SHA1(fc1165b7b31a44ab204cd5ac3e7b2733ed6b1534) )
	ROM_LOAD16_BYTE( "g300a1.u41-00", 0x0000001, 0x080000, CRC(d26b7c4f) SHA1(b491170010977ba1e5111893937cc6bab0539e7d) )
	ROM_LOAD16_BYTE( "g300b0.u45-00", 0x0100000, 0x080000, CRC(9637934c) SHA1(d3b39d9f44825bdf24d4aa39ca32035bc5af4905) )
	ROM_LOAD16_BYTE( "g300b1.u42-00", 0x0100001, 0x080000, CRC(d72e154b) SHA1(e367c8f9af47b999fcba4afcd293565bad2038ec) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204a0.u33-00", 0x400000, 0x040000, CRC(2867cbfd) SHA1(89af600fb33ce72a7a3fbdf9ff05a4916454a205) )
	ROM_LOAD16_BYTE( "g204a1.u27-00", 0x400001, 0x040000, CRC(c50503bc) SHA1(5003aa414660358900857901d5e9eca6739f14e3) )

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	//ROM_LOAD( "g104g1.u04-00", 0x140000, 0x080000, CRC(03539013) SHA1(36b96c59c59d0e747eb000472c22a30de0810902) )   // $8 x $10000, 1st is just audio data, no header // this is the german sound data? shouldn't be loaded in an English set??...

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END


ROM_START( galpani2e2 ) // Kaneko Z04G2-003 + Z04G2-SUB3
	ROM_REGION( 0x100000, "maincpu", 0 )            // CPU#1 Code
	ROM_LOAD16_BYTE( "g000e1-u133-0.u133", 0x000000, 0x080000, CRC(47bae233) SHA1(b3827bceeb5d092ae2c19efb17ac418ff11667ec) )   // English version specific
	ROM_LOAD16_BYTE( "g001e1-u134-0.u134", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )   // same as other 2 PCB versions

	ROM_REGION( 0x40000, "sub", 0 )         // CPU#2 Code
	ROM_LOAD16_BYTE( "g002e1-u125-0", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003e1-u126-0", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    // Backgrounds (CPU2)
	ROM_LOAD16_BYTE( "g300a0.u44-00", 0x0000000, 0x080000, CRC(50406294) SHA1(fc1165b7b31a44ab204cd5ac3e7b2733ed6b1534) )
	ROM_LOAD16_BYTE( "g300a1.u41-00", 0x0000001, 0x080000, CRC(d26b7c4f) SHA1(b491170010977ba1e5111893937cc6bab0539e7d) )
	ROM_LOAD16_BYTE( "g300b0.u45-00", 0x0100000, 0x080000, CRC(9637934c) SHA1(d3b39d9f44825bdf24d4aa39ca32035bc5af4905) )
	ROM_LOAD16_BYTE( "g300b1.u42-00", 0x0100001, 0x080000, CRC(d72e154b) SHA1(e367c8f9af47b999fcba4afcd293565bad2038ec) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   // Sprites
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204a0.u33-00", 0x400000, 0x040000, CRC(2867cbfd) SHA1(89af600fb33ce72a7a3fbdf9ff05a4916454a205) )
	ROM_LOAD16_BYTE( "g204a1.u27-00", 0x400001, 0x040000, CRC(c50503bc) SHA1(5003aa414660358900857901d5e9eca6739f14e3) )

	ROM_REGION( 0x1c0000, "oki1", 0 )   // Samples
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   // Samples
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END


ROM_START( galpani2t )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000t1.133", 0x000000, 0x080000, CRC(332048e7) SHA1(1a353d4b29f7a08158fc454309dc496df6b5b108) )  // Taiwan version specific
	ROM_LOAD16_BYTE( "g001t1.134", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )  // same as other 2 PCB versions

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002t1.125", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003t1.126", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204t1.33", 0x400000, 0x040000, CRC(65a1f838) SHA1(ccc3bb4a4f4ea1677caa1a3a51bc0a13b4b619c7) )
	ROM_LOAD16_BYTE( "g204t1.27", 0x400001, 0x040000, CRC(39059f66) SHA1(6bf41738033a13b63d96babf827c73c914323425) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END


ROM_START( galpani2k ) // Kaneko Z04G2-004 single PCB
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "27c040.u165", 0x000000, 0x080000, CRC(152bbddf) SHA1(b5cb78f7aef145bf37c1252959b479e948e09df4) ) // No labels, but likely different to the PCB layout in header
	ROM_LOAD16_BYTE( "27c040.u164", 0x000001, 0x080000, CRC(906d72af) SHA1(fb57d371d6fa3efeee38ca3e89f3d2725dada0bf) ) // No labels, but likely different to the PCB layout in header

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "27c010.u64", 0x000000, 0x020000, CRC(4fa04a29) SHA1(5819b6e6a845e16cc26704bc465b1bb807b4573d) ) // No labels, but likely different to the PCB layout in header
	ROM_LOAD16_BYTE( "27c010.u63", 0x000001, 0x020000, CRC(c74f6763) SHA1(e39063ec1789fbacfaba6e97031247222f7b206d) ) // No labels, but likely different to the PCB layout in header

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD16_BYTE( "gp2-300a-k_0067.u175", 0x000000, 0x100000, CRC(68145adc) SHA1(6d8ab1f344adf2f0ec79aad7bb3c7977c5229651) ) // KANEKO GP2-300A-K 0067
	ROM_LOAD16_BYTE( "gp2-300b-k_0068.u176", 0x000001, 0x100000, CRC(e77af845) SHA1(c88cc93f718b3057120c81403cd0b1659305008b) ) // KANEKO GP2-300B-K 0068
	ROM_LOAD16_BYTE( "gp2-301a-k_0069.u177", 0x200000, 0x100000, CRC(b6610c77) SHA1(8f53360bde92de24fc8154718aece497cd9178d3) ) // KANEKO GP2-301A-K 0069
	ROM_LOAD16_BYTE( "gp2-301b-k_0070.u178", 0x200001, 0x100000, CRC(71969a41) SHA1(337d18caf57bca8025669cdcc11e16c49e7fe854) ) // KANEKO GP2-301B-K 0070
	ROM_LOAD16_BYTE( "gp2-302a_0057.u179",   0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) ) // KANEKO GP2-302A 0057
	ROM_LOAD16_BYTE( "gp2-302b_0058.u180",   0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) ) // KANEKO GP2-302B 0058
	ROM_LOAD16_BYTE( "gp2-303a_0063.u181",   0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) ) // KANEKO GP2-303A 0063
	ROM_LOAD16_BYTE( "gp2-303b_0064.u182",   0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) ) // KANEKO GP2-303B 0064
	/* For this PCB set, GROM21L / GROM21U & GROM22L / GROM22U were unpopulated. Unlike what's shown in the PCB layout in the header */

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200_0046.u189",  0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) ) // KANEKO GP2-200 0046
	ROM_CONTINUE(                   0x000000, 0x080000)
	ROM_LOAD( "gp2-201_0047.u171",  0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) ) // KANEKO GP2-201 0047
	ROM_CONTINUE(                   0x100000, 0x080000)
	ROM_LOAD( "gp2-202_0048.u121",  0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) ) // KANEKO GP2-202 0048
	ROM_CONTINUE(                   0x200000, 0x080000)
	ROM_LOAD( "gp2-203_0049.u102",  0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) ) // KANEKO GP2-203 0049
	ROM_CONTINUE(                   0x300000, 0x080000)
	ROM_LOAD16_BYTE( "27c020.u169", 0x400000, 0x040000, CRC(048c0ea6) SHA1(6585b0f20135e0b731f7fb801fbb6cb538763069) ) // No labels, but likely different to the PCB layout in header
	ROM_LOAD16_BYTE( "27c020.u170", 0x400001, 0x040000, CRC(15f5012d) SHA1(65aa84e25cb83887ac64b0ae27ea08582133430f) ) // No labels, but likely different to the PCB layout in header

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100-k.u61", 0x040000, 0x100000, CRC(665ae0a2) SHA1(fa67a49eb4cb67f4f7f51bc70bce58fe9baff57b) )
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples - Soldered in, NOT dumped from this PCB, taken from the set below */
	ROM_LOAD( "gp2-102_0045.u59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) ) // KANEKO GP2-102 0045
	ROM_LOAD( "gp2-101_0044.u60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) ) // KANEKO GP2-101 0044
ROM_END

/*

Gals Panic II - Quiz Version (c) 1993 Kaneko

CPUs: 68HC000-16 (x2)
Sound: OKI6295 (x2)
Customs: KC-BYO KA05-1068 (x2), KC-TAS KA07-1209, KC-YUU KA06-0041, KC-SHU KA03-1849 (x2), KC-001 (x2), PISCES, KC-002
RAM: 6116 (x6), 52B256 (x4), 42101 (x2), 42426 (x2), 6264 (x2), 431000 (x8)
X1: 27 MHz
X2: 16 MHz
X3: 33.333 MHz
X4: 20 MHz

--- NOTE 07/01/07
this just appears to be a regular japanese version, NOT the quiz version, unless it's using the wrong roms..

*/

ROM_START( galpani2j )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000j2.165", 0x000000, 0x080000, CRC(e0c5a03d) SHA1(e12457400ca8cd78674b44d7f4d664cfc0afc8c9) )
	ROM_LOAD16_BYTE( "g001j2.164", 0x000001, 0x080000, CRC(c8e12223) SHA1(0e0160565e95cb33dc6ad796225e995ed3baf8eb) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002j1.64",  0x000000, 0x020000, CRC(5e523829) SHA1(dad11e4a3348c988ff658609cf78a3fbee58064e) )
	ROM_LOAD16_BYTE( "g003j1.63",  0x000001, 0x020000, CRC(2a0d5f89) SHA1(0a7031c4b8b7bc757da25250dbb5fa1004205aeb) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300j.175", 0x000000, 0x200000, CRC(3a0afc1d) SHA1(91fba9074cc3c28e919053f0ea07b28d88b2ce5f) )
	ROM_LOAD( "gp2-301j.176", 0x200000, 0x200000, CRC(5b6d1709) SHA1(a7d35247fe71895f2b6169409aa0bdaef446804c) )
	ROM_LOAD16_BYTE( "gp2-302a.177", 0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) )
	ROM_LOAD16_BYTE( "gp2-302b.178", 0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) )
	ROM_LOAD16_BYTE( "gp2-303a.179", 0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) )
	ROM_LOAD16_BYTE( "gp2-303b.180", 0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) )

	ROM_REGION( 0x480000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200j.189", 0x000000, 0x200000, CRC(2f81e519) SHA1(c07f4dad15b6f7f1fb867f773c0ada309d172326) )
	ROM_LOAD( "gp2-201j.171", 0x200000, 0x200000, CRC(bbe404e0) SHA1(198db9a6c6ec97ed8fd32d946051ba4d6e4bd354) )
	ROM_LOAD16_BYTE( "g204j0.169", 0x400000, 0x040000, CRC(212d8aab) SHA1(459f556978ef9a103279cf633fcc1cacb367ea61) )
	ROM_LOAD16_BYTE( "g204j1.170", 0x400001, 0x040000, CRC(bfd89343) SHA1(884d17b3302643d86f84a4a4917de850c5bf8924) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100j.61", 0x000000, 0x100000, CRC(60382cbf) SHA1(766c50a3302bc11d54de49a2850522d93fc36ba2) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) )
	ROM_LOAD( "gp2-101.60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) )
ROM_END

ROM_START( gp2se )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000j4.u165", 0x000000, 0x080000, CRC(d8258a7a) SHA1(12991392d7e70bfba394ec4ad49b427959ca019e) )
	ROM_LOAD16_BYTE( "g001j4.u164", 0x000001, 0x080000, CRC(23f706bf) SHA1(960c6e6c17f03072cecabfd52018e0351ff4b661) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002j4.u64",  0x000000, 0x020000, CRC(bcd4edd9) SHA1(17ae6fbf75d8e5333133737de926a36f5cd29661) )
	ROM_LOAD16_BYTE( "g003j4.u63",  0x000001, 0x020000, CRC(2fbe0194) SHA1(52da771ba813b27ec1a996b237c14dab9b33db82) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300-j-0071.u175", 0x000000, 0x200000, CRC(3a0afc1d) SHA1(91fba9074cc3c28e919053f0ea07b28d88b2ce5f) )
	ROM_LOAD( "gp2-301-j-0072.u176", 0x200000, 0x200000, CRC(5b6d1709) SHA1(a7d35247fe71895f2b6169409aa0bdaef446804c) )
	ROM_LOAD16_BYTE( "gp2-302a-0057.u177", 0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) )
	ROM_LOAD16_BYTE( "gp2-302b-0058.u178", 0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) )
	ROM_LOAD16_BYTE( "gp2-303a-0063.u179", 0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) )
	ROM_LOAD16_BYTE( "gp2-303b-0064.u180", 0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) )
	ROM_LOAD16_BYTE( "g304aj4.u158", 0x800000, 0x080000, CRC(3a1c9d53) SHA1(058311da5f47036f5e388895e41abc1757ed8518) )
	ROM_LOAD16_BYTE( "g304bj4.u159", 0x800001, 0x080000, CRC(ae87cf2c) SHA1(a397e9441048ae6b5699b7e45f420c92903e8a96) )
	ROM_LOAD16_BYTE( "g305aj4.u160", 0x900000, 0x080000, CRC(2d4a8fbb) SHA1(8a00e6ba4e061678da4c41446df7278c9b4f26c2) )
	ROM_LOAD16_BYTE( "g305bj4.u161", 0x900001, 0x080000, CRC(53d13974) SHA1(29ca4d36f2a8153228c2eec8e9ef6a6bf712cb59) )

	ROM_REGION( 0x500000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200-j-0073.u189", 0x000000, 0x200000, CRC(2f81e519) SHA1(c07f4dad15b6f7f1fb867f773c0ada309d172326) )
	ROM_LOAD( "gp2-201-j-0074.u171", 0x200000, 0x200000, CRC(bbe404e0) SHA1(198db9a6c6ec97ed8fd32d946051ba4d6e4bd354) )
	ROM_LOAD16_BYTE( "g204aj4.u169", 0x400000, 0x080000, CRC(e5e32820) SHA1(9bdc0717feb8983c0d6d5edaa08bcebad4baace0) )
	ROM_LOAD16_BYTE( "g204bj4.u170", 0x400001, 0x080000, CRC(0bd46a73) SHA1(78b163431648db6bfa453e440584e781063529a9) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	/* no u61 on this one */
	ROM_LOAD( "g104j4.u62", 0x000000, 0x080000, CRC(0546ea41) SHA1(cf351b496d93648a50fc0e84badb5bb855b681b4) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102-0045.u59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) )
	ROM_LOAD( "gp2-101-0044.u60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) )
ROM_END


ROM_START( gp2quiz )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000e3.u165-3", 0x000000, 0x080000, CRC(b6de2653) SHA1(a24daf5e6b6b268f60b1dbb374861c85f642cea5) )
	ROM_LOAD16_BYTE( "g001e3.u164-3", 0x000001, 0x080000, CRC(74e8d0e8) SHA1(d131be9f52ee79e1b82f46721c2ad5d71b3da649) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002e3.u64-3",  0x000000, 0x020000, CRC(5e523829) SHA1(dad11e4a3348c988ff658609cf78a3fbee58064e) )
	ROM_LOAD16_BYTE( "g003e3.u63-3",  0x000001, 0x020000, CRC(2a0d5f89) SHA1(0a7031c4b8b7bc757da25250dbb5fa1004205aeb) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300-j-0071.u175", 0x000000, 0x200000, CRC(3a0afc1d) SHA1(91fba9074cc3c28e919053f0ea07b28d88b2ce5f) )
	ROM_LOAD( "gp2-301-j-0072.u176", 0x200000, 0x200000, CRC(5b6d1709) SHA1(a7d35247fe71895f2b6169409aa0bdaef446804c) )
	ROM_LOAD16_BYTE( "gp2-302a-0057.u177", 0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) )
	ROM_LOAD16_BYTE( "gp2-302b-0058.u178", 0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) )
	ROM_LOAD16_BYTE( "gp2-303a-0063.u179", 0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) )
	ROM_LOAD16_BYTE( "gp2-303b-0064.u180", 0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) )

	ROM_REGION( 0x500000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200-j-0073.u189", 0x000000, 0x200000, CRC(2f81e519) SHA1(c07f4dad15b6f7f1fb867f773c0ada309d172326) )
	ROM_LOAD( "gp2-201-j-0074.u171", 0x200000, 0x200000, CRC(bbe404e0) SHA1(198db9a6c6ec97ed8fd32d946051ba4d6e4bd354) )
	ROM_LOAD16_BYTE( "g204a3.u169-3", 0x400000, 0x080000, CRC(92a837b7) SHA1(f581e1f7754f1fb20255c6c55ffc4e486d867111) )
	ROM_LOAD16_BYTE( "g204a4.u170-3", 0x400001, 0x080000, CRC(3c2dd1cd) SHA1(d5267ad6f51283191174988ac0519c0e0aa6552f) )

	ROM_REGION( 0x180000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100-0043.u61", 0x000000, 0x100000, CRC(a61e8868) SHA1(ad84ae00ebe7c70a36b1aa75e743686a0193e5d9) )
	ROM_LOAD( "g104a3.u62-3", 0x100000, 0x080000, CRC(42b3470e) SHA1(c121ea6c98e6ff452f4bcc49c3a5179e99237128) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102-0045.u59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) )
	ROM_LOAD( "gp2-101-0044.u60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) )
ROM_END

GAME( 1993, galpani2,   0,        galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Asia)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2e,  galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (English)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2e2, galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (English, 2 PCB ver.)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2g,  galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Germany, 2 PCB ver.)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2i2, galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Italy, 2 PCB ver.)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2i,  galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Italy, single PCB)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2gs, galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Germany, single PCB)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2t,  galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Taiwan, 2 PCB ver.)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2k,  galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko (Karam Trading Co., Ltd. license)", "Gals Panic II (Korea, single PCB)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2j,  galpani2, galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // it is a 'quiz edition' but the title screen doesn't say, maybe all Japanese versions have the Quiz

GAME( 1993, gp2quiz,    0,        galpani2, galpani2, galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II - Quiz Version", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // this one has 'quiz edition' on the title screen

GAME( 1994, gp2se,      0,        galpani2, gp2se,    galpani2_state, empty_init, ROT90, "Kaneko", "Gals Panic II' - Special Edition (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
