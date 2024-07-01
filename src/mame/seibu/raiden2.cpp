// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/********************************************************************************************************

    Seibu Protected 1993-94 era hardware, V30 based (sequel to the SYS68C hardware)

    TODO:
    * zeroteam - sort-DMA doesn't seem to work too well, sprite-sprite priorities are broken as per now

    * xsedae - it does an "8-liner"-style scroll during attract, doesn't work too well.

    * sprite chip is the same as seibuspi.cpp and feversoc.cpp, needs device-ification and merging.

    * sprite chip also uses first entry for "something" that isn't sprite, some of them looks clipping
      regions (150 - ff in zeroteam, 150 - 0 and 150 - 80 in raiden2). Latter probably do double buffering
      on odd/even frames, by updating only top or bottom part of screen.

===========================================================================================================

raiden 2 board test note 17/04/08 (based on test by dox)

 ROM banking is at 6c9, bit 0x80
  -- the game only writes this directly at startup, must be written indirectly by
     one of the protection commands? or mirrored?
  value of 0x80 puts 0x00000-0x1ffff at 0x20000 - 0x3ffff
  value of 0x00 puts 0x20000-0x3ffff at 0x20000 - 0x3ffff


===========================================================================================================

Raiden DX
Seibu Kaihatsu, 1994

This readme covers Raiden DX and to some extent Raiden II
which uses an almost identical PCB.

PCB Layout
----------

(C) 1993 RAIDEN II DX SEIBU KAIHATSU INC.,o
|----------------------------------------------------------|
|      1    2   3   4   5   6    7      8      9     10    |
|LA4460    M6295  PCM  Z80     6116                       A|
|   YM2151 M6295   6    5      6116    28.63636MHz        B|
|     VOL   YM3012                                         |
|HB-45A            |------|                               C|
|HB-2       4560   |SIE150| 6116      |---------|          |
|RC220             |      | 6116      | SEI252  |         D|
|                  |------| 6116      |SB05-106 |          |
|                           6116      |(QFP208) |         E|
|J                                    |         |         F|
|A    DSW2(8)                         |---------|          |
|M                                                        G|
|M    DSW1(8)                                   CXK58258   |
|A          |---------|OBJ-1    OBJ-2           CXK58258  H|
|           | SEI360  |                         CXK58258  J|
|           |SB06-1937|DX_OBJ-3 DX_OBJ-4        CXK58258  K|
|           |(QFP160) |  PAL1               |---------|   L|
|           |         |                     |SEI1000  |   M|
| |------|  |---------|  1H      3H         |SB01-001 |   N|
| |SEI200|         32MHz                    |(QFP184) |    |
| |      |CY7C185        2H      4H         |         |   P|
| |------|CY7C185                           |---------|    |
|                                                         Q|
|                        PAL2 PAL3             |----|     R|
|                                              |V30 |      |
| DX_BACK-1  DX_BACK-2   7   COPX-D2           |----|     S|
|----------------------------------------------------------|
Notes:
      V30 clock    - 16.000MHz [32/2]. Chip is stamped "NEC D70116HG-16 V30 NEC '84" (QFP52)
      Z80 clock    - 3.579545MHz [28.63636/8]. /NMI, /BUSREQ and /WAIT tied high/unused.
      YM2151 clock - 3.579545MHz [28.63636/8]
      M6295 clocks - 1.022MHz [28.63636/28] and pin 7 HIGH (both)
      CXK58258     - Sony CXK58258 32k x8 SRAM (= 62256)
      CY7C185      - Cypress CY7C185 8k x8 SRAM (= 6264)
      6116         - 2k x8 SRAM
      HB-45A       - Seibu custom ceramic module sound DAC (SIP20)
      HB-2         - Seibu custom ceramic module connected to coin counters (SIP10)
      RC220        - Custom resistor network module used for inputs (SIP14)
      VSync        - 55.4859Hz  \
      HSync        - 15.5586kHz / measured via EL4583
      PAL1         - AMI 18CV8 stamped 'JJ5004' (DIP20)
      PAL2         - AMI 18CV8 stamped 'JJ5002' (DIP20)
      PAL3         - AMI 18CV8 stamped 'JJ5001' (DIP20)
      ROMs         - *PCM      - 2M MaskROM stamped 'RAIDEN 2 PCM' at location U1018 (DIP32)
                     6         - 27C020 EPROM labelled 'SEIBU 6' at location U1017 (DIP32)
                     5         - 27C512 EPROM labelled 'SEIBU 5' at location U1110 (DIP28)
                     *OBJ-1    - 16M MaskROM stamped 'RAIDEN 2 OBJ-1' at location U0811 (DIP42)
                     *OBJ-2    - 16M MaskROM stamped 'RAIDEN 2 OBJ-2' at location U082 (DIP42)
                     DX_OBJ-3  - 16M MaskROM stamped 'DX OBJ-3' at location U0837 (DIP42)
                     DX_OBJ-4  - 16M MaskROM stamped 'DX OBJ-4' at location U0836 (DIP42)
                     1H        - 27C4001 EPROM labelled 'SEIBU 1H' at location U1210 (DIP32)
                     2H        - 27C4001 EPROM labelled 'SEIBU 2H' at location U1211 (DIP32)
                     3H        - 27C4001 EPROM labelled 'SEIBU 3H' at location U129 (DIP32)
                     4H        - 27C4001 EPROM labelled 'SEIBU 4H' at location U1212 (DIP32)
                     DX_BACK-1 - 16M MaskROM stamped 'DX BACK-1' at location U075 (DIP42)
                     DX_BACK-2 - 16M MaskROM stamped 'DX BACK-2' at location U0714 (DIP42)
                     7         - 27C210 EPROM labelled 'SEIBU 7' at location U0724 (DIP40)
                     *COPX-D2  - 2M MaskROM stamped 'COPX-D2' at location U0313 (DIP40)

                     * = these ROMs are soldered-in and match ROMs from the original Raiden II PCB

      SEIBU Custom ICs -
                        SIE150 (QFP100)
                        SEI252 SB05-106 (QFP208)
                        SEI0200 TC110G21AF 0076 (QFP100)
                        SEI360 SB06-1937 (QFP160)
                        SEI1000 SB01-001 (QFP184)


Games on this PCB / Similar PCBs
 Raiden 2
 Raiden DX
 Zero Team
 X Se Dae Quiz

 + variants

Some of these games were also released on updated PCBs
which usually featured vastly inferior sound hardware
 (see the V33 based version of Raiden II/DX New)

All games on this hardware have a startup routine that reads a few of the highest bytes of ROM
and stores their adjusted values in RAM. Which bytes are read differs from set to set, but one
is always FFFFB, which determines the region and/or licensee.

             Raiden II/DX       Zero Team
             -------------      ----------
    00/FF    Japan/Default      Japan/Default
    01       U.S. (Fabtek)      U.S. (Fabtek)
    02       Taiwan (Liang Hwa) Korea (Dream Soft)
    03       HK (Metrotainment) Taiwan (Liang Hwa)
    04       Korea?             "Selection"
    05       Germany (Tuning)
    06       Austria
    07       Belgium
    08       Denmark
    09       Finland
    0A       France
    0B       Great Britain
    0C       Greece
    0D       Holland
    0E       Italy
    0F       Norway
    10       Portugal
    11       Spain
    12       Sweden
    13       Switzerland
    14       Australia
    15       New Zealand

Most sets of Raiden II and Raiden DX display "USE IN JAPAN ONLY" when the region byte is set
to 00 or FF. This string is absent from the 'easier' sets of Raiden II, and Zero Team has
nothing of the sort. (The obviously Korean-only X Se Dae Quiz still reads the byte.)

Region 04 for Raiden II and Raiden DX is presumably Korea, but the notice that would confirm
this does not seem to be present in any of the sets.


Protection Notes:
 These games use the 2nd (and 3rd) generation of Seibu's 'COP' protection,
 utilizing the external 'COPX_D2' and 'COPX_D3' lookup ROMs (probably for
 math operations)  These chips, marked (c)1992 RISE Corp. are not thought
 to be the actual MCU which is probably internal to one of the Seibu
 customs.

 The games in legionna.cpp use (almost?) the same protection chips.

********************************************************************************************************/

#include "emu.h"
#include "raiden2.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"
#include "r2crypt.h"

#include "debugger.h"
#include "speaker.h"


void raiden2_state::common_save_state()
{
	save_item(NAME(m_bg_bank));
	save_item(NAME(m_fg_bank));
	save_item(NAME(m_mid_bank));
	save_item(NAME(m_tx_bank));
	save_item(NAME(m_tilemap_enable));
	save_item(NAME(m_prg_bank));
	save_item(NAME(m_cop_bank));

	save_item(NAME(m_sprite_prot_x));
	save_item(NAME(m_sprite_prot_y));
	save_item(NAME(m_dst1));
	save_item(NAME(m_cop_spr_maxx));
	save_item(NAME(m_cop_spr_off));

	save_item(NAME(m_scrollvals));

	save_item(NAME(m_sprite_prot_src_addr));
}

void raiden2_state::machine_start()
{
	common_save_state();

	save_item(NAME(m_sprcpt_adr));
	save_item(NAME(m_sprcpt_idx));
	save_item(NAME(m_sprcpt_val));
	save_item(NAME(m_sprcpt_flags1));
	save_item(NAME(m_sprcpt_flags2));
	save_item(NAME(m_sprcpt_data_1));
	save_item(NAME(m_sprcpt_data_2));
	save_item(NAME(m_sprcpt_data_3));
	save_item(NAME(m_sprcpt_data_4));
}

/*
u16 raiden2_state::rps()
{
    return m_maincpu->state_int(NEC_CS);
}

u16 raiden2_state::rpc()
{
    return m_maincpu->state_int(NEC_IP);
}
*/


void raiden2_state::combine32(u32 *val, offs_t offset, u16 data, u16 mem_mask)
{
	u16 *dest = (u16 *)val + BYTE_XOR_LE(offset);
	COMBINE_DATA(dest);
}


/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(raiden2_state::interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc0/4);   // V30 - VBL
}


// Sprite encryption key upload

void raiden2_state::sprcpt_init()
{
	std::fill(std::begin(m_sprcpt_data_1), std::end(m_sprcpt_data_1), 0);
	std::fill(std::begin(m_sprcpt_data_2), std::end(m_sprcpt_data_2), 0);
	std::fill(std::begin(m_sprcpt_data_3), std::end(m_sprcpt_data_3), 0);
	std::fill(std::begin(m_sprcpt_data_4), std::end(m_sprcpt_data_4), 0);

	m_sprcpt_adr = 0;
	m_sprcpt_idx = 0;
}


void raiden2_state::sprcpt_adr_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(&m_sprcpt_adr, offset, data, mem_mask);
}

void raiden2_state::sprcpt_data_1_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(m_sprcpt_data_1+m_sprcpt_adr, offset, data, mem_mask);
}

void raiden2_state::sprcpt_data_2_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(m_sprcpt_data_2+m_sprcpt_adr, offset, data, mem_mask);
}

void raiden2_state::sprcpt_data_3_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(m_sprcpt_data_3+m_sprcpt_idx, offset, data, mem_mask);
	if (offset == 1)
	{
		m_sprcpt_idx++;
		if (m_sprcpt_idx == 6)
			m_sprcpt_idx = 0;
	}
}

void raiden2_state::sprcpt_data_4_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(m_sprcpt_data_4+m_sprcpt_idx, offset, data, mem_mask);
	if (offset == 1)
	{
		m_sprcpt_idx++;
		if (m_sprcpt_idx == 4)
			m_sprcpt_idx = 0;
	}
}

void raiden2_state::sprcpt_val_1_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(m_sprcpt_val+0, offset, data, mem_mask);
}

void raiden2_state::sprcpt_val_2_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(m_sprcpt_val+1, offset, data, mem_mask);
}

void raiden2_state::sprcpt_flags_1_w(offs_t offset, u16 data, u16 mem_mask)
{
	combine32(&m_sprcpt_flags1, offset, data, mem_mask);
	if (offset == 1)
	{
		// bit 31: 1 = allow write on sprcpt data

		if (!(m_sprcpt_flags1 & 0x80000000U))
		{
			// Upload finished
			if (1)
			{
				int i;
				logerror("sprcpt_val 1: %08x\n", m_sprcpt_val[0]);
				logerror("sprcpt_val 2: %08x\n", m_sprcpt_val[1]);
				logerror("sprcpt_data 1:\n");
				for (i = 0; i < 0x100; i++)
				{
					logerror(" %08x", m_sprcpt_data_1[i]);
					if (!((i + 1) & 7))
						logerror("\n");
				}
				logerror("sprcpt_data 2:\n");
				for (i = 0; i < 0x40; i++)
				{
					logerror(" %08x", m_sprcpt_data_2[i]);
					if (!((i + 1) & 7))
						logerror("\n");
				}
			}
		}
	}
}

void raiden2_state::sprcpt_flags_2_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_sprcpt_flags2);
	if (offset == 0)
	{
		if (m_sprcpt_flags2 & 0x8000)
		{
			// Reset decryption -> redo it
		}
	}
}


void raiden2_state::bank_reset(int bgbank, int fgbank, int midbank, int txbank)
{
	m_bg_bank  = bgbank;
	m_fg_bank  = fgbank;
	m_mid_bank = midbank;
	m_tx_bank  = txbank;
}

MACHINE_RESET_MEMBER(raiden2_state,raiden2)
{
	bank_reset(0,6,1,0);
	sprcpt_init();

	m_mainbank->set_entry(1);

	m_prg_bank = 0;
	//cop_init();
}

MACHINE_RESET_MEMBER(raiden2_state,raidendx)
{
	bank_reset(0,6,1,0);
	sprcpt_init();

	m_mainbank->set_entry(0);

	m_prg_bank = 0x08;

	//cop_init();
}

MACHINE_RESET_MEMBER(raiden2_state,zeroteam)
{
	bank_reset(0,2,1,0);
	sprcpt_init();

	m_mainbank->set_entry(1);

	m_prg_bank = 0;
	//cop_init();
}

MACHINE_RESET_MEMBER(raiden2_state,xsedae)
{
	bank_reset(0,2,1,0);
	sprcpt_init();
}

void raiden2_state::raiden2_bank_w(u8 data)
{
	const int bb = BIT(~data, 7);
	logerror("select bank %d %04x\n", BIT(data, 7), data);
	m_mainbank->set_entry(bb);
	m_prg_bank = BIT(data, 7);
}


void raiden2_state::sprite_prot_x_w(u16 data)
{
	m_sprite_prot_x = data;
	//popmessage("%04x %04x",m_sprite_prot_x,m_sprite_prot_y);
}

void raiden2_state::sprite_prot_y_w(u16 data)
{
	m_sprite_prot_y = data;
	//popmessage("%04x %04x",m_sprite_prot_x,m_sprite_prot_y);
}

void raiden2_state::sprite_prot_src_seg_w(u16 data)
{
	m_sprite_prot_src_addr[0] = data;
}

u16 raiden2_state::sprite_prot_src_seg_r()
{
	return m_sprite_prot_src_addr[0];
}

void raiden2_state::sprite_prot_src_w(address_space &space, u16 data)
{
	m_sprite_prot_src_addr[1] = data;
	const u32 src = (m_sprite_prot_src_addr[0]<<4)+m_sprite_prot_src_addr[1];

	const int x = s16((space.read_dword(src + 0x08) >> 16) - (m_sprite_prot_x));
	const int y = s16((space.read_dword(src + 0x04) >> 16) - (m_sprite_prot_y));

	const u16 head1 = space.read_word(src + m_cop_spr_off);
	const u16 head2 = space.read_word(src + m_cop_spr_off+2);

	const int w = (((head1 >> 8 ) & 7) + 1) << 4;
	const int h = (((head1 >> 12) & 7) + 1) << 4;

	u16 flag = x-w/2 > -w && x-w/2 < m_cop_spr_maxx+w && y-h/2 > -h && y-h/2 < 256+h ? 1 : 0;

	flag = (space.read_word(src) & 0xfffe) | flag;
	space.write_word(src, flag);

	if (flag & 1)
	{
		space.write_word(m_dst1,   head1);
		space.write_word(m_dst1+2, head2);
		space.write_word(m_dst1+4, x-w/2);
		space.write_word(m_dst1+6, y-h/2);

		m_dst1 += 8;
	}
	//printf("[%08x] %08x %08x %04x %04x\n",src,dx,dy,m_dst1,dst2);
	//machine().debug_break();
}

u16 raiden2_state::sprite_prot_dst1_r()
{
	return m_dst1;
}

u16 raiden2_state::sprite_prot_maxx_r()
{
	return m_cop_spr_maxx;
}

u16 raiden2_state::sprite_prot_off_r()
{
	return m_cop_spr_off;
}

void raiden2_state::sprite_prot_dst1_w(u16 data)
{
	m_dst1 = data;
}

void raiden2_state::sprite_prot_maxx_w(u16 data)
{
	m_cop_spr_maxx = data;
}

void raiden2_state::sprite_prot_off_w(u16 data)
{
	m_cop_spr_off = data;
}

// MEMORY MAPS
void raiden2_state::raiden2_cop_mem(address_map &map)
{
	map(0x0041c, 0x0041d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_angle_target_w)); // angle target (for 0x6200 COP macro)
	map(0x0041e, 0x0041f).w(m_raiden2cop, FUNC(raiden2cop_device::cop_angle_step_w));   // angle step   (for 0x6200 COP macro)
	map(0x00420, 0x00421).w(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_low_w));
	map(0x00422, 0x00423).w(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_high_w));
	map(0x00424, 0x00425).w(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_mode_w));
	map(0x00428, 0x00429).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_v1_w));
	map(0x0042a, 0x0042b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_v2_w));
	map(0x0042c, 0x0042d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_prng_maxvalue_w));
	map(0x00432, 0x00433).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_data_w));
	map(0x00434, 0x00435).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_addr_w));
	map(0x00436, 0x00437).w(m_raiden2cop, FUNC(raiden2cop_device::cop_hitbox_baseadr_w));
	map(0x00438, 0x00439).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_value_w));
	map(0x0043a, 0x0043b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_mask_w));
	map(0x0043c, 0x0043d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pgm_trigger_w));
	map(0x00444, 0x00445).w(m_raiden2cop, FUNC(raiden2cop_device::cop_scale_w));
	map(0x00450, 0x00451).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_ram_addr_hi_w));
	map(0x00452, 0x00453).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_ram_addr_lo_w));
	map(0x00454, 0x00455).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_lookup_hi_w));
	map(0x00456, 0x00457).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_lookup_lo_w));
	map(0x00458, 0x00459).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_param_w));
	map(0x0045a, 0x0045b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pal_brightness_val_w)); //palette DMA brightness val, used by X Se Dae / Zero Team
	map(0x0045c, 0x0045d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_pal_brightness_mode_w));  //palette DMA brightness mode, used by X Se Dae / Zero Team (sets to 5)
	map(0x00470, 0x00471).rw(FUNC(raiden2_state::cop_tile_bank_2_r), FUNC(raiden2_state::cop_tile_bank_2_w)); // implementaton of this varies between games, external hookup?

	map(0x00476, 0x00477).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_adr_rel_w));
	map(0x00478, 0x00479).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_src_w));
	map(0x0047a, 0x0047b).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_size_w));
	map(0x0047c, 0x0047d).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_dst_w));
	map(0x0047e, 0x0047f).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_mode_r), FUNC(raiden2cop_device::cop_dma_mode_w));
	map(0x004a0, 0x004ad).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_reg_high_r), FUNC(raiden2cop_device::cop_reg_high_w));
	map(0x004c0, 0x004cd).rw(m_raiden2cop, FUNC(raiden2cop_device::cop_reg_low_r), FUNC(raiden2cop_device::cop_reg_low_w));
	map(0x00500, 0x00505).w(m_raiden2cop, FUNC(raiden2cop_device::cop_cmd_w));
	map(0x00580, 0x00581).r(m_raiden2cop, FUNC(raiden2cop_device::cop_collision_status_r));
	map(0x00582, 0x00587).r(m_raiden2cop, FUNC(raiden2cop_device::cop_collision_status_val_r));
	map(0x00588, 0x00589).r(m_raiden2cop, FUNC(raiden2cop_device::cop_collision_status_stat_r));
	map(0x00590, 0x00599).r(m_raiden2cop, FUNC(raiden2cop_device::cop_itoa_digits_r));
	map(0x005a0, 0x005a7).r(m_raiden2cop, FUNC(raiden2cop_device::cop_prng_r)); // zeroteam reads from 5a4
	map(0x005b0, 0x005b1).r(m_raiden2cop, FUNC(raiden2cop_device::cop_status_r));
	map(0x005b2, 0x005b3).r(m_raiden2cop, FUNC(raiden2cop_device::cop_dist_r));
	map(0x005b4, 0x005b5).r(m_raiden2cop, FUNC(raiden2cop_device::cop_angle_r));

	map(0x00600, 0x0063f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	//map(0x00640, 0x006bf).rw("obj", FUNC(seibu_encrypted_sprite_device::read), FUNC(seibu_encrypted_sprite_device::write));
	map(0x006a0, 0x006a3).w(FUNC(raiden2_state::sprcpt_val_1_w));
	map(0x006a4, 0x006a7).w(FUNC(raiden2_state::sprcpt_data_3_w));
	map(0x006a8, 0x006ab).w(FUNC(raiden2_state::sprcpt_data_4_w));
	map(0x006ac, 0x006af).w(FUNC(raiden2_state::sprcpt_flags_1_w));
	map(0x006b0, 0x006b3).w(FUNC(raiden2_state::sprcpt_data_1_w));
	map(0x006b4, 0x006b7).w(FUNC(raiden2_state::sprcpt_data_2_w));
	map(0x006b8, 0x006bb).w(FUNC(raiden2_state::sprcpt_val_2_w));
	map(0x006bc, 0x006bf).w(FUNC(raiden2_state::sprcpt_adr_w));
	map(0x006c0, 0x006c1).rw(FUNC(raiden2_state::sprite_prot_off_r), FUNC(raiden2_state::sprite_prot_off_w));
	map(0x006c2, 0x006c3).rw(FUNC(raiden2_state::sprite_prot_src_seg_r), FUNC(raiden2_state::sprite_prot_src_seg_w));
	map(0x006c4, 0x006c5).nopw(); // constant value written along with 0x6c0
	map(0x006c6, 0x006c7).w(FUNC(raiden2_state::sprite_prot_dst1_w));
	map(0x006cb, 0x006cb).w(FUNC(raiden2_state::raiden2_bank_w));
	map(0x006cc, 0x006cc).w(FUNC(raiden2_state::tile_bank_01_w));
	map(0x006ce, 0x006cf).w(FUNC(raiden2_state::sprcpt_flags_2_w));
	map(0x006d8, 0x006d9).w(FUNC(raiden2_state::sprite_prot_x_w));
	map(0x006da, 0x006db).w(FUNC(raiden2_state::sprite_prot_y_w));
	map(0x006dc, 0x006dd).rw(FUNC(raiden2_state::sprite_prot_maxx_r), FUNC(raiden2_state::sprite_prot_maxx_w));
	map(0x006de, 0x006df).w(FUNC(raiden2_state::sprite_prot_src_w));
	// end video block

	map(0x006fc, 0x006fd).w(m_raiden2cop, FUNC(raiden2cop_device::cop_dma_trigger_w));
	map(0x006fe, 0x006ff).w(m_raiden2cop, FUNC(raiden2cop_device::cop_sort_dma_trig_w)); // sort-DMA trigger

	map(0x00762, 0x00763).r(FUNC(raiden2_state::sprite_prot_dst1_r));
}

void raiden2_state::raiden2_mem(address_map &map)
{
	map(0x00000, 0x003ff).ram();

	raiden2_cop_mem(map);

	map(0x0068e, 0x0068f).w(m_spriteram, FUNC(buffered_spriteram16_device::write));

	map(0x00700, 0x0071f).lrw8(
							   NAME([this](offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
							   NAME([this](offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);

	map(0x00740, 0x00741).portr("DSW");
	map(0x00744, 0x00745).portr("P1_P2");
	map(0x0074c, 0x0074d).portr("SYSTEM");

	map(0x00800, 0x0bfff).ram();

	map(0x0c000, 0x0cfff).ram().share("spriteram");
	map(0x0d000, 0x0d7ff).ram(); // .w(FUNC(raiden2_state::background_w)).share("back_data");
	map(0x0d800, 0x0dfff).ram(); // .w(FUNC(raiden2_state::foreground_w).share("fore_data");
	map(0x0e000, 0x0e7ff).ram(); // .w(FUNC(raiden2_state::midground_w).share("mid_data");
	map(0x0e800, 0x0f7ff).ram(); // .w(FUNC(raiden2_state::text_w).share("text_data");
	map(0x0f800, 0x0ffff).ram(); // Stack area

	map(0x10000, 0x1efff).ram();
	map(0x1f000, 0x1ffff).ram(); //.w("palette", palette_device, write).share("palette");

	map(0x20000, 0x3ffff).bankr(m_mainbank);
	map(0x40000, 0xfffff).rom().region("maincpu", 0x40000);
}

void raiden2_state::raidendx_mem(address_map &map)
{
	raiden2_mem(map);
	map(0x00470, 0x00471).rw(FUNC(raiden2_state::cop_tile_bank_2_r), FUNC(raiden2_state::raidendx_cop_bank_2_w));
	map(0x004d0, 0x004d7).ram(); //???
	map(0x00600, 0x0063f).rw("crtc", FUNC(seibu_crtc_device::read_alt), FUNC(seibu_crtc_device::write_alt));
//  map(0x006ca, 0x006cb).nopw();

	map(0x20000, 0x2ffff).bankr(m_mainbank);
	map(0x30000, 0xfffff).rom().region("maincpu", 0x30000);
}

void raiden2_state::zeroteam_mem(address_map &map)
{
	map(0x00000, 0x003ff).ram();

	raiden2_cop_mem(map);

	map(0x00470, 0x00471).nopw();
	map(0x006cc, 0x006cd).nopw();

	map(0x0068e, 0x0068f).w(m_spriteram, FUNC(buffered_spriteram16_device::write));

	map(0x00700, 0x0071f).lrw8(
							   NAME([this](offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
							   NAME([this](offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);

	map(0x00740, 0x00741).portr("DSW");
	map(0x00744, 0x00745).portr("P1_P2");
	map(0x00748, 0x00749).portr("P3_P4");
	map(0x0074c, 0x0074d).portr("SYSTEM");

	map(0x00800, 0x0b7ff).ram();
	map(0x0b800, 0x0bfff).ram(); // .w(FUNC(raiden2_state::background_w)).share("back_data");
	map(0x0c000, 0x0c7ff).ram(); // .w(FUNC(raiden2_state::foreground_w).share("fore_data");
	map(0x0c800, 0x0cfff).ram(); // .w(FUNC(raiden2_state::midground_w).share("mid_data");
	map(0x0d000, 0x0dfff).ram(); // .w(FUNC(raiden2_state::text_w).share("text_data");
	map(0x0e000, 0x0efff).ram(); // .w("palette", palette_device, write).share("palette");
	map(0x0f000, 0x0ffff).ram().share("spriteram");
	map(0x10000, 0x1ffff).ram();

	map(0x20000, 0x3ffff).bankr(m_mainbank);
	map(0x40000, 0xfffff).rom().region("maincpu", 0x40000);
}

void raiden2_state::xsedae_mem(address_map &map)
{
	map(0x00000, 0x003ff).ram();

	raiden2_cop_mem(map);

	map(0x00470, 0x00471).nopw();
	map(0x006cc, 0x006cd).nopw();

	map(0x0068e, 0x0068f).w(m_spriteram, FUNC(buffered_spriteram16_device::write));

	map(0x00700, 0x0071f).lrw8(
							   NAME([this](offs_t offset) { return m_seibu_sound->main_r(offset >> 1); }),
							   NAME([this](offs_t offset, u8 data) { m_seibu_sound->main_w(offset >> 1, data); })).umask16(0x00ff);

	map(0x00740, 0x00741).portr("DSW");
	map(0x00744, 0x00745).portr("P1_P2");
	map(0x00748, 0x00749).portr("P3_P4");
	map(0x0074c, 0x0074d).portr("SYSTEM");

	map(0x00800, 0x0b7ff).ram();
	map(0x0b800, 0x0bfff).ram(); // .w(FUNC(raiden2_state::background_w)).share("back_data");
	map(0x0c000, 0x0c7ff).ram(); // .w(FUNC(raiden2_state::foreground_w).share("fore_data");
	map(0x0c800, 0x0cfff).ram(); // .w(FUNC(raiden2_state::midground_w).share("mid_data");
	map(0x0d000, 0x0dfff).ram(); // .w(FUNC(raiden2_state::text_w).share("text_data");
	map(0x0e000, 0x0efff).ram(); // .w("palette", palette_device, write).share("palette");
	map(0x0f000, 0x0ffff).ram().share("spriteram");

	map(0x10000, 0x1ffff).ram();

	map(0x20000, 0xfffff).rom().region("maincpu", 0x20000);
}

void raiden2_state::raiden2_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w(m_seibu_sound, FUNC(seibu_sound_device::pending_w));
	map(0x4001, 0x4001).w(m_seibu_sound, FUNC(seibu_sound_device::irq_clear_w));
	map(0x4002, 0x4002).w(m_seibu_sound, FUNC(seibu_sound_device::rst10_ack_w));
	map(0x4003, 0x4003).w(m_seibu_sound, FUNC(seibu_sound_device::rst18_ack_w));
	map(0x4004, 0x4004).noprw();
	map(0x4008, 0x4009).rw(m_seibu_sound, FUNC(seibu_sound_device::ym_r), FUNC(seibu_sound_device::ym_w));
	map(0x4010, 0x4011).r(m_seibu_sound, FUNC(seibu_sound_device::soundlatch_r));
	map(0x4012, 0x4012).r(m_seibu_sound, FUNC(seibu_sound_device::main_data_pending_r));
	map(0x4013, 0x4013).portr("COIN");
	map(0x4018, 0x4019).w(m_seibu_sound, FUNC(seibu_sound_device::main_data_w));
	map(0x401a, 0x401a).w(m_seibu_sound, FUNC(seibu_sound_device::bank_w));
	map(0x401b, 0x401b).w(m_seibu_sound, FUNC(seibu_sound_device::coin_w));
	map(0x6000, 0x6000).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x6002, 0x6002).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x8000, 0xffff).bankr("seibu_bank1");
}

void raiden2_state::zeroteam_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w(m_seibu_sound, FUNC(seibu_sound_device::pending_w));
	map(0x4001, 0x4001).w(m_seibu_sound, FUNC(seibu_sound_device::irq_clear_w));
	map(0x4002, 0x4002).w(m_seibu_sound, FUNC(seibu_sound_device::rst10_ack_w));
	map(0x4003, 0x4003).w(m_seibu_sound, FUNC(seibu_sound_device::rst18_ack_w));
	map(0x4008, 0x4009).rw(m_seibu_sound, FUNC(seibu_sound_device::ym_r), FUNC(seibu_sound_device::ym_w));
	map(0x4010, 0x4011).r(m_seibu_sound, FUNC(seibu_sound_device::soundlatch_r));
	map(0x4012, 0x4012).r(m_seibu_sound, FUNC(seibu_sound_device::main_data_pending_r));
	map(0x4013, 0x4013).portr("COIN");
	map(0x4018, 0x4019).w(m_seibu_sound, FUNC(seibu_sound_device::main_data_w));
	map(0x401a, 0x401a).w(m_seibu_sound, FUNC(seibu_sound_device::bank_w));
	map(0x401b, 0x401b).w(m_seibu_sound, FUNC(seibu_sound_device::coin_w));
	map(0x6000, 0x6000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x8000, 0xffff).bankr("seibu_bank1");
}


// INPUT PORTS

static INPUT_PORTS_START( raiden2 )
	SEIBU_COIN_INPUTS_INVERT    // coin inputs read through sound cpu

	PORT_START("P1_P2") // IN0/1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")   // Dip switches 
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) ) // dipsw sheets say this is hard but service mode says easy
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) ) // vice versa of above
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x3000, "200000 500000" )
	PORT_DIPSETTING(      0x2000, "400000 1000000" )
	PORT_DIPSETTING(      0x1000, "1000000 3000000" )
	PORT_DIPSETTING(      0x0000, "No Extend" )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:!8") // Test Mode

	PORT_START("SYSTEM")    // START BUTTONS
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( raidendx )
	PORT_INCLUDE( raiden2 )

	PORT_MODIFY("DSW")  // Dip switches 
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!5") // Manual shows "Not Used"
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!6") // Manual shows "Not Used"
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( zeroteam )
	PORT_INCLUDE( raiden2 )

	PORT_MODIFY("COIN")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(4)

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P3_P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) // read in unused dead code - debugging leftover?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) // read in unused dead code - debugging leftover?

	PORT_MODIFY("DSW") // not the same as raiden2/dx: coins, difficulty, lives and bonus lives all differ!
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x3000, "1000000" )
	PORT_DIPSETTING(      0x2000, "2000000" )
	PORT_DIPSETTING(      0x1000, "Every 1000000" )
	PORT_DIPSETTING(      0x0000, "No Extend" )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!8") // marked as unused
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0700, 0x0700, "Cabinet Setting" ) PORT_DIPLOCATION("SW3:!1,!2,!3")
	PORT_DIPSETTING(    0x0700, "2P" )
	PORT_DIPSETTING(    0x0600, "3P 3Slot" )
	PORT_DIPSETTING(    0x0500, "4P 4Slot" )
	PORT_DIPSETTING(    0x0400, "3P 2Slot" )
	PORT_DIPSETTING(    0x0300, "2P x2" )
	PORT_DIPSETTING(    0x0200, "4P 2Slot" )
	PORT_DIPSETTING(    0x0100, "2P Freeplay" )
	PORT_DIPSETTING(    0x0000, "4P Freeplay" )
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW3:!4") // marked as test mode
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!5") // marked as unused
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!6") // marked as unused
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!7") // marked as unused
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!8") // marked as unused
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xsedae )
	PORT_INCLUDE( raiden2 )

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW0" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{ 3,2,1,0,19,18,17,16 },
	{ STEP8(0,32) },
	32*8
};


static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{
		3,2,1,0,
		19,18,17,16,
		3+64*8, 2+64*8, 1+64*8, 0+64*8,
		19+64*8,18+64*8,17+64*8,16+64*8,
	},
	{ STEP16(0,32) },
	128*8
};

GFXDECODE_START( raiden2_state::gfx_raiden2 )
	GFXDECODE_ENTRY( "chars",   0, charlayout,             0x700, 0x10 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,             0x400, 0x30 )
GFXDECODE_END

GFXDECODE_START( raiden2_state::gfx_raiden2_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_lsb, 0x000, 0x40 ) // really 128, but using the top bits for priority
GFXDECODE_END


// MACHINE DRIVERS

void raiden2_state::raiden2(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, XTAL(32'000'000)/2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden2_state::raiden2_mem);
	m_maincpu->set_vblank_int("screen", FUNC(raiden2_state::interrupt));

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,raiden2)

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(28'636'363)/8));
	audiocpu.set_addrmap(AS_PROGRAM, &raiden2_state::raiden2_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_raw(XTAL(32'000'000)/4, 512, 0, 40*8, 282, 0, 30*8); // hand-tuned to match ~55.47
	screen.set_screen_update(FUNC(raiden2_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, raiden2_state::gfx_raiden2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(raiden2_state::tilemap_enable_w));
	crtc.layer_scroll_callback().set(FUNC(raiden2_state::tile_scroll_w));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	SEI25X_RISE1X(config, m_spritegen, 0, m_palette, raiden2_state::gfx_raiden2_spr);
	m_spritegen->set_screen("screen");
	m_spritegen->set_pix_raw_shift(4);
	m_spritegen->set_pri_raw_shift(14);
	m_spritegen->set_transpen(15);

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(raiden2_state::m_videoram_private_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(28'636'363)/8));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(28'636'363)/28, okim6295_device::PIN7_HIGH));
	oki1.add_route(ALL_OUTPUTS, "mono", 0.40);

	okim6295_device &oki2(OKIM6295(config, "oki2", XTAL(28'636'363)/28, okim6295_device::PIN7_HIGH));
	oki2.add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank1");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));
}

void raiden2_state::raidendx(machine_config &config)
{
	raiden2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden2_state::raidendx_mem);

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,raidendx)
}

void raiden2_state::zeroteam(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, XTAL(32'000'000)/2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden2_state::zeroteam_mem);
	m_maincpu->set_vblank_int("screen", FUNC(raiden2_state::interrupt));

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,zeroteam)

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(28'636'363)/8));
	audiocpu.set_addrmap(AS_PROGRAM, &raiden2_state::zeroteam_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
//  screen.set_refresh_hz(55.47);    // verified on pcb
	screen.set_raw(XTAL(32'000'000)/4, 512, 0, 40*8, 282, 0, 32*8); // hand-tuned to match ~55.47
	screen.set_screen_update(FUNC(raiden2_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, raiden2_state::gfx_raiden2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(raiden2_state::tilemap_enable_w));
	crtc.layer_scroll_callback().set(FUNC(raiden2_state::tile_scroll_w));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	SEI25X_RISE1X(config, m_spritegen, 0, m_palette, raiden2_state::gfx_raiden2_spr);
	m_spritegen->set_screen("screen");
	m_spritegen->set_pix_raw_shift(4);
	m_spritegen->set_pri_raw_shift(14);
	m_spritegen->set_transpen(15);

	RAIDEN2COP(config, m_raiden2cop, 0);
	m_raiden2cop->videoramout_cb().set(FUNC(raiden2_state::m_videoram_private_w));
	m_raiden2cop->paletteramout_cb().set(m_palette, FUNC(palette_device::write16));
	m_raiden2cop->set_host_cpu_tag(m_maincpu);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(28'636'363)/8));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(28'636'363)/28, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank1");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void raiden2_state::xsedae(machine_config &config)
{
	zeroteam(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden2_state::xsedae_mem);

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,xsedae)

	subdevice<screen_device>("screen")->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);

	ym2151_device &ymsnd(YM2151(config.replace(), "ymsnd", XTAL(28'636'363)/8));
	ymsnd.irq_handler().set(m_seibu_sound, FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));
}

// ROM LOADING
/*
Raiden II

(C) 1993 RAIDEN II SEIBU KAIHATSU INC.,o
|----------------------------------------------------------|
|      1    2   3   4   5   6    7      8      9     10    |
|LA4460    M6295  PCM  Z8400A  6116    BATTERY3.6v        A|
|   YM2151 M6295   6    5      6116    28.6360 MHz        B|
|     VOL   YM3014                                         |
|HB-45A     YM3012 |------|                               C|
|HB-2      NJM4560 |SIE150| LH5116    |---------|          |
|RC220             |      | LH5116    | SEI252  |         D|
|RC220             |------| LH5116    |SB05-106 |          |
|RC220                      LH5116    |(QFP208) |         E|
|J                                    |         |         F|
|A                                    |---------|          |
|M    DSW2(8)                                             G|
|M    DSW1(8)                                   LH522258   |
|A          |---------|OBJ-1    OBJ-2           LH522258  H|
|           | SEI360  |                         LH522258  J|
|           |SB06-1937|OBJ-3    OBJ-4           LH522258  K|
|           |(QFP160) |                     |---------|   L|
|           |         |      1              |SEI1000  |   M|
| |------|  |---------|  1x      3x         |SB01-001 |   N|
| |SEI200|         32MHz     2              |(QFP184) |    |
| |      |CXK5863        2x      4x         |         |   P|
| |------|CXK5863                           |---------|    |
|                                                         Q|
|                        PAL2 PAL1             |----|     R|
|                                              |V30 |      |
|      BG-1       BG-2   7   COPX-D2           |----|     S|
|----------------------------------------------------------|
Notes:
      V30 clock    - 16.000MHz [32/2]. Chip is stamped "NEC D70116HG-16 V30 NEC '84" (QFP52)
      Z80 clock    - 3.579545MHz [28.63636/8]
      YM2151 clock - 3.579545MHz [28.63636/8]
      Yamaha DAC   -
        early boards: ym3014 mono dac, no NJM4560
        later boards: ym3012 stereo dac plus NJM4560, each with a capacitor on top
      M6295 clocks - 1.022MHz [28.63636/28] and pin 7 HIGH (both)
      LH52258      - Sharp LH52258 32k x8 SRAM (= 62256)
      CXK5863      - Sony CXK5863 8k x8 SRAM (= 6264)
      6116         - 2k x8 SRAM
      LH5116       - 2k x8 SRAM
      HB-45A       - Seibu custom ceramic module sound DAC (SIP20)
      HB-2         - Seibu custom ceramic module connected to coin counters (SIP10)
      RC220        - Custom resistor network module used for inputs (SIP14)
      VSync        - 55.4859Hz  \
      HSync        - 15.5586kHz / not measured but assumed same as R2DX
      PAL1         - MMIPAL16L8B stamped 'JJ4B01' (DIP20)
      PAL2         - AMI 18CV8 stamped 'JJ4B02' (DIP20)
      ROMs         - *PCM      - 2M MaskROM stamped 'RAIDEN 2 PCM' at location U1018 (DIP32), pcb labeled VOI2
                     6         - 23C020 mask ROM labelled 'SEIBU 6' at location U1017 (DIP32), pcb labeled VOI1
                     5         - 27C512 EPROM labelled 'SEIBU 5' at location U1110 (DIP28)
                     *OBJ-1     - 16M mask ROM stamped 'RAIDEN 2 OBJ-1' at location U0811 (DIP42)
                     *OBJ-2     - 16M mask ROM stamped 'RAIDEN 2 OBJ-2' at location U082 (DIP42)
                     *OBJ-3     - 16M mask ROM stamped 'RAIDEN 2 OBJ-3' at location U0837 (DIP42)
                     *OBJ-4     - 16M mask ROM stamped 'RAIDEN 2 OBJ-4' at location U0836 (DIP42)
                 /   1x        - 27C2001 EPROM labelled 'SEIBU 1' at location U1210 (DIP32)
     Early boards|   2x        - 27C2001 EPROM labelled 'SEIBU 2' at location U1211 (DIP32)
                 |   3x        - 27C2001 EPROM labelled 'SEIBU 3' at location U129 (DIP32)
                 \   4x        - 27C2001 EPROM labelled 'SEIBU 4' at location U1212 (DIP32)
     Later boards/   1         - 27C402 or 27C4096 EPROM labelled 'SEIBU 1' at location U0211 (DIP40)
                 \   2         - 27C402 or 27C4096 EPROM labelled 'SEIBU 2' at location U0212 (DIP40)
                     *BG-1      - 16M MaskROM stamped 'RAIDEN 2 BG-1' at location U075 (DIP42)
                     *BG-2      - 16M MaskROM stamped 'RAIDEN 2 BG-2' at location U0714 (DIP42)
                     7         - 27C210 EPROM labelled 'SEIBU 7' at location U0724 (DIP40)
                     *COPX-D2  - 2M MaskROM stamped 'COPX-D2' at location U0313 (DIP40)

                     * = these ROMs are soldered-in

      SEIBU Custom ICs -
                        SIE150 (QFP100) - z80 interface
                        SEI252 SB05-106 (QFP208) - fg/sprite gfx and its decryption
                        SEI0200 TC110G21AF 0076 (QFP100) - bg gfx
                        SEI360 SB06-1937 (QFP160) - logic and i/o array
                        SEI1000 SB01-001 (QFP184) - main protection

*/

/* Note: some raiden 2 fabtek usa boards (the one Hammad sent to LN and Balrog, at least) have the
    ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
    z80 sound ROM as used in raiden2hk instead of the
    ROM_LOAD( "snd.u1110",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
    ROM from raiden2. Slight version difference, and I don't know which is older/newer. - LN

ROMSET organization:
Note: type numbers are NOT NECESSARILY in chronological version order YET.
SETNAME   LONGNAME       PRG TYPES   SND(u1110) TYPE   VOICE(u1017) TYPE  FX0(u0724) TYPE  Notes
raiden2   (set 1 fabtek) 1 1'        1(f51a28f9)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
raiden2hk (set 2 metro)  1 2'        2(8f130589)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
raiden2j  (set 3 japan)  1 3'        2(8f130589)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
raiden2i  (set 4 italy)  2 4'        3(5db9f922)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
(trap15: one of these four above has aama serial 0587600)
raiden2e  (set 5 easy)   3 5'        4(6bad0a3e)       2(488d050f)        2(c709bdf6)      red fighter on hiscore
raiden2ea (set 6 easy)   4 6'        5(f5f835af)       3(fab9f8e4)        3(c7aa4d00)      red fighter on hiscore
raiden2eu (set 7 easy fabtek) 4 7'   5(f5f835af)       3(fab9f8e4)        3(c7aa4d00)      red fighter on hiscore
raiden2eua (set 8 easy fabtek) 3 8'   6(6d362472)       3(fab9f8e4)        3(c7aa4d00)      red fighter on hiscore, sn 0003068, aama 0557135
^ this set has 4 program ROMs: 1 and 3 correspond to seibu1/prg0 and 2 and 4 correspond to seibu2/prg1
balrog+ln (set x fabtek) 1 1'        2(8f130589)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore, sn 0012739, aama 0600565, not in mame yet due to ROMs matching mix of sets 1 and 2

differences amongst SND/u1110 ROMs:
   First half end, last byte before ff fill ending at 7fff
   |     Last byte before ff fill ending at 8fff
   |     |     Last byte before ff fill ending at ffff
   |     |     |
1: 62e8  8faf  f56b
2: 62b8  8faf  f56b
3: 62a9  8faf  f56b
4: 623e  8ee7  f4dd
5: 620a  8ee7  f4d7
6: 64b8  8e1f  f4db
<LordNlptp> btw my guess is the code versions go from newest to oldest, 1 to 6, though I need more serial numbers to be sure
<LordNlptp> 6 has a larger main code chunk because i think they accidentally included some stuff they didn't actually use, which was removed on later versions
<LordNlptp> and it would not surprise me in the least if the code/player data is ALMOST the same as the zt version but with support for the second msm6295

*/

ROM_START( raiden2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("prg1.u0212",   0x000001, 0x80000, CRC(4609b5f2) SHA1(272d2aa75b8ea4d133daddf42c4fc9089093df2e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "snd.u1110",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2g )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1 - same code base as raiden2
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("prg1g.u0212",   0x000001, 0x80000, CRC(41001d2e) SHA1(06bece44c081ecbb3b8dac5c515e30c5a5ffc1bf) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "snd.u1110",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0 - sldh w/raiden2u

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

/*

---------------------------------------
Raiden II by SEIBU KAIHATSU INC. (1993)
---------------------------------------
malcor

Location      Type      File ID    Checksum
-------------------------------------------
M6 U0211     27C240      ROM1        F9A9
M6 U0212     27C240      ROM2e       13B3    [ English  ]
M6 U0212     27C240      ROM2J       14BF    [ Japanese ]
B5 U1110     27C512      ROM5        1223
B3 U1017     27C2000     ROM6        DE25
S5 U0724     27C1024     ROM7        966D

*/

ROM_START( raiden2hk )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1 - same code base as raiden2
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2e.u0212",  0x000001, 0x80000, CRC(458d619c) SHA1(842bf0eeb5d192a6b188f4560793db8dad697683) )
	ROM_RELOAD(0x100001, 0x80000)


	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) ) // sldh w/raiden2u
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

/*

Raiden II (Japan version)
(c) 1993 Seibu Kaihatsu Inc.,

CPU:          D70116HG-16 V30/Z8400AB1 Z80ACPU
SOUND:        YM2151
VOICE:        M6295 x2
OSC:          32.000/28.6364MHz
CUSTOM:       SEI150
              SEI252
              SEI360
              SEI1000
              SEI0200
              COPX-D2 ((c)1992 RISE CORP)

---------------------------------------------------
 filemanes          devices       kind
---------------------------------------------------
 RD2_1.211          27C4096       V30 main prg.
 RD2_2.212          27C4096       V30 main prg.
 RD2_5.110          27C512        Z80 sound prg.
 RD2_PCM.018        27C2001       M6295 data
 RD2_6.017          27C2001       M6295 data
 RD2_7.724          27C1024       fix chr.
 RD2_BG1.075        57C16200      bg  chr.
 RD2_BG2.714        57C16200      bg  chr.
 RD2_OBJ1.811       57C16200      obj chr.
 RD2_OBJ2.082       57C16200      obj chr.
 RD2_OBJ3.837       57C16200      obj chr.
 RD2_OBJ4.836       57C16200      obj chr.
---------------------------------------------------

*/

ROM_START( raiden2j )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1 - same code base as raiden2
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2j.u0212",  0x000001, 0x80000, CRC(e4e4fb4c) SHA1(7ccf33fe9a1cddf0c7e80d7ed66d615a828b3bb9) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1 - sldh w/raiden2u

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2sw ) // original board with serial # 0008307
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("seibu_1.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1 - same code base as raiden2 - sldh w/raiden2eu
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu_2.u0212",   0x000001, 0x80000, CRC(59abc2ec) SHA1(45f2dbd2dd46f5da07dae0dc486772f8e61f4c43) ) // sldh w/raiden2eu
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(c2028ba2) SHA1(f6a9322b669ff82dea6ecf52ad3bd5d0901cce1b) ) // 99.993896% match
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2f ) // original board with serial # 12476 that matches raiden2nl set except the region and Audio CPU
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("1_u0211.bin",   0x000000, 0x80000, CRC(53be3dd0) SHA1(304d118423e4085eea3b883bd625d90d21bb2054) )   // same code base as raiden2nl & raiden2es
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu2_u0212.bin",  0x000001, 0x80000, CRC(8dcd8a8d) SHA1(be0681d5867d8b4f5fb78946a896d89827a71e8e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu5_u1110.bin",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )   // == raiden2
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "6_u1017.bin", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2nl )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("1_u0211.bin",   0x000000, 0x80000, CRC(53be3dd0) SHA1(304d118423e4085eea3b883bd625d90d21bb2054) )   // same code base as raiden2f & raiden2es
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("2_u0212.bin",  0x000001, 0x80000, CRC(88829c08) SHA1(ecdfbafeeffcd009bbc4cf5bf797bcd4b5bfcf50) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5_u1110.bin",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "6_u1017.bin", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2es )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("1_u0211.bin",   0x000000, 0x80000, CRC(53be3dd0) SHA1(304d118423e4085eea3b883bd625d90d21bb2054) )   // same code base as raiden2f & raiden2nl
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("2_u0212.rom",  0x000001, 0x80000, CRC(9dbec61c) SHA1(59ed06d9f97d93486dec2c0d8c0f42f59fb19db0) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5_u1110.bin",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "6_u1017.bin", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2u )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("1.u0211",  0x000000, 0x80000, CRC(b16df955) SHA1(9b7fd85cf2f2c9fea657f3c38abafa93673b3933) ) // unique unknown code base
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("2.u0212",  0x000001, 0x80000, CRC(2a14b112) SHA1(84cd9891b5be0b71b2bae3487ad38bed3045305e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k code for sound Z80
	ROM_LOAD( "seibu5.u1110", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) ) // sldh w/raiden2hk
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) ) // sldh w/raiden2g

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) ) // sldh w/raiden2j

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2i )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("seibu1.u0211",   0x000000, 0x80000, CRC(c1fc70f5) SHA1(a054f5ae9583972c406d9cf871340d5e072d71a3) ) // Italian set  - unique unknown code base
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu2.u0212",   0x000001, 0x80000, CRC(28d5365f) SHA1(21efe29c2d373229c2ff302d86e59c2c94fa6d03) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu5.c.u1110",  0x000000, 0x08000, CRC(5db9f922) SHA1(8257aab98657fe44df19d2a48d85fcf65b3d98c6) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END


ROM_START( raiden2au )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("1.u0211",   0x000000, 0x80000, CRC(c1fc70f5) SHA1(a054f5ae9583972c406d9cf871340d5e072d71a3) )
	ROM_RELOAD(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE("2.u0212",   0x000001, 0x80000, CRC(396410f9) SHA1(a3f737ae1fb6229388d3ebec93e880218d09fed5) ) // Australian set
	ROM_RELOAD(                  0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF )
	ROM_LOAD( "5.u1110",  0x000000, 0x08000, CRC(c2028ba2) SHA1(f6a9322b669ff82dea6ecf52ad3bd5d0901cce1b) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "voice_2.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END


ROM_START( raiden2k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("k1.u0211",   0x000000, 0x80000, CRC(1fcc08cf) SHA1(bff7076ced189120166217d71e2762bb98aad7c8) ) // hand-written
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("k2.u0212",   0x000001, 0x80000, CRC(59a744ca) SHA1(5fdd7dd4049f944df23371e2e2d3133b10c66ab8) ) // hand-written
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "snd.u1110",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) // PCB silkscreened VOICE1

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

/*

Raiden 2, Seibu License, Easy Version

According to DragonKnight Zero's excellent Raiden 2
FAQ this PCB is the easy version.

The different versions may be identified by the high score
screen. The easy version has the Raiden MK-II in colour
on a black background whereas the hard version has a sepia shot
of an ascending fighter.

The entire FAQ is available here:
http://www.gamefaqs.com/coinop/arcade/game/10729.html

*/

ROM_START( raiden2e )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("r2_prg_0.u0211",   0x000000, 0x80000, CRC(2abc848c) SHA1(1df4276d0074fcf1267757fa0b525a980a520f3d) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2_prg_1.u0212",   0x000001, 0x80000, CRC(509ade43) SHA1(7cdee7bb00a6a1c7899d10b96385d54c261f6f5a) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "r2_snd.u1110", 0x000000, 0x08000, CRC(6bad0a3e) SHA1(eb7ae42353e1984cd60b569c26cdbc3b025a7da6) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "r2_fx0.u0724",   0x000000, 0x020000, CRC(c709bdf6) SHA1(0468d90412b7590d67eaadc0a5e3537cd5e73943) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "r2_voi1.u1017", 0x00000, 0x40000, CRC(488d050f) SHA1(fde2fd64fea6bc39e1a42885d21d362bc6be2ac2) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2ea )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("r2.1.u0211",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2.2.u0212",  0x000001, 0x80000, CRC(bf7577ec) SHA1(98576af78760b8aef1ef3efe1ba963977c89d225) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k code for sound Z80
	ROM_LOAD( "r2.5.u1110", 0x000000, 0x08000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "r2.7.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "r2.6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2eu ) // same as raiden2ea, different region
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("seibu_1.u0211",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) ) // sldh w/raiden2sw
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu_2.u0212",  0x000001, 0x80000, CRC(beb71ddb) SHA1(471399ead1cdc27ac2a1139f9616f828efd14626) ) // sldh w/raiden2sw
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k code for sound Z80
	ROM_LOAD( "r2.5.u1110", 0x000000, 0x08000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "r2.7.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "r2.6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2eua ) // sort of a mixture of raiden2e easy set with voice ROM of raiden2ea and 2f and a unique sound ROM
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("seibu__1.27c020j.u1210",   0x000000, 0x40000, CRC(ed1514e3) SHA1(296125bfe3c4f3033f7aa319dd8554bc978c4a00) )
	ROM_RELOAD(0x100000, 0x40000)
	ROM_LOAD32_BYTE("seibu__2.27c2001.u1211",   0x000001, 0x40000, CRC(bb6ecf2a) SHA1(d4f628e9d0ed2897654f05a8a2541e1ed3faf8dd) )
	ROM_RELOAD(0x100001, 0x40000)
	ROM_LOAD32_BYTE("seibu__3.27c2001.u129",    0x000002, 0x40000, CRC(6a01d52c) SHA1(983b914592ab9d9c058bebb5bccf5c882e2b82de) )
	ROM_RELOAD(0x100002, 0x40000)
	ROM_LOAD32_BYTE("seibu__4.27c2001.u1212",   0x000003, 0x40000, CRC(e54bfa37) SHA1(4fabb23503fd9245a10cded15a6880415ca5ffd7) )
	ROM_RELOAD(0x100003, 0x40000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k code for sound Z80
	ROM_LOAD( "seibu__5.27c512.u1110", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu__7.fx0.27c210.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu__6.voice1.23c020.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2eg ) // this is the same code revision as raiden2eua but a german region
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("raiden_2_1.u1210",   0x000000, 0x40000, CRC(ed1514e3) SHA1(296125bfe3c4f3033f7aa319dd8554bc978c4a00) )
	ROM_RELOAD(0x100000, 0x40000)
	ROM_LOAD32_BYTE("raiden_2_2.u1211",   0x000001, 0x40000, CRC(bb6ecf2a) SHA1(d4f628e9d0ed2897654f05a8a2541e1ed3faf8dd) )
	ROM_RELOAD(0x100001, 0x40000)
	ROM_LOAD32_BYTE("raiden_2_3.u129",    0x000002, 0x40000, CRC(6a01d52c) SHA1(983b914592ab9d9c058bebb5bccf5c882e2b82de) )
	ROM_RELOAD(0x100002, 0x40000)
	ROM_LOAD32_BYTE("raiden_2_4.u1212",   0x000003, 0x40000, CRC(81273f33) SHA1(074cedf44cc5286649cc101bce0b48d40234e472) )
	ROM_RELOAD(0x100003, 0x40000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k code for sound Z80
	ROM_LOAD( "raiden_2_5.bin", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "raiden_2_7.bin", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_6.bin", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2eup )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("prg0 11-16.u1210",   0x000000, 0x40000, CRC(0a68a400) SHA1(7b3baae086ea9604af29eabde358da358d43591f) )
	ROM_RELOAD(0x100000, 0x40000)
	ROM_LOAD32_BYTE("prg1 11-16.u1211",   0x000001, 0x40000, CRC(45a01ebe) SHA1(8b114198130ead8a70cc64b195fba5a3507ff6cb) )
	ROM_RELOAD(0x100001, 0x40000)
	ROM_LOAD32_BYTE("prg2 11-16.u129",    0x000002, 0x40000, CRC(9a6e33d7) SHA1(fadaec486cd4163d4b8f41aac171baaac3505e30) )
	ROM_RELOAD(0x100002, 0x40000)
	ROM_LOAD32_BYTE("prg3a 11-16.u1212",  0x000003, 0x40000, CRC(42ce511b) SHA1(59f199cb52cb315ddea0373b978541b8efeb57a3) )
	ROM_RELOAD(0x100003, 0x40000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k code for sound Z80
	ROM_LOAD( "sound 11-12.bin", 0x000000, 0x08000, CRC(4fac206c) SHA1(47b78b2efe88729df07033a8674ba203cdf2e44c) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "fix.bin", 0x000000, 0x020000, CRC(6a6fa0de) SHA1(4e2ba9e84f5f5684b480e6d7f86bfc08ac9f5337) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "pcm 11-11.bin", 0x00000, 0x40000, CRC(c541c729) SHA1(23f0b454d8d813bfcdb831ee5a8547178892231f) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2eub )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD16_BYTE("r2_prg_0.u0211",   0x000000, 0x80000, CRC(2abc848c) SHA1(1df4276d0074fcf1267757fa0b525a980a520f3d) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2_prg_1.u0212",   0x000001, 0x80000, CRC(56511ca8) SHA1(1073f9a5dab185b161306d1f0db44ae84865294b) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "r2_snd.u1110", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "r2.7.u0724",   0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "r2.6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	// Common Raiden II PALs below
	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342",   0x0000, 0x155, CRC(057a9cdc) SHA1(8b46f6673ddf11efbc3394ae423ec89d4a1283bf) )
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341", 0x0000, 0x117, CRC(20931f21) SHA1(95ce9cfbfb280dfc6a326e378684eff3c6f54701) )

	// Common Raiden II soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2dx ) // this set is very weird, it's Raiden II on a Raiden DX board, I'm assuming for now that it uses Raiden DX graphics, but could be wrong.
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("u1210.bin",      0x000000, 0x80000, CRC(413241e0) SHA1(50fa501db91412baea474a8faf8ad483f3a119c7) )
	ROM_LOAD32_BYTE("prg1_u1211.bin", 0x000001, 0x80000, CRC(93491f56) SHA1(2239980fb7267906e4c3985703c2dc2932b23705) )
	ROM_LOAD32_BYTE("u129.bin",       0x000002, 0x80000, CRC(e0932b6c) SHA1(04f1ca885d220e802023042438f63e40e4106696) )
	ROM_LOAD32_BYTE("u1212.bin",      0x000003, 0x80000, CRC(505423f4) SHA1(d8e65580deec05dd84c4cf3074cb690e3764c625) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "u1110.bin",  0x000000, 0x08000,  CRC(b8ad8fe7) SHA1(290896f811f717ef6e3ec2152d4db98a9fe9b310) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	//ROM_LOAD( "fx0_u0724.bin",    0x000000,   0x020000,   CRC(ded3c718) SHA1(c722ec45cd1b2dab23aac14e9113e0e9697830d3) ) // bad dump
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // PCB silkscreened FX0

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX(!) soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Shared with original Raiden 2
ROM_END

// Raiden DX sets

ROM_START( raidendx )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1d.4n",   0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.4p",   0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.6n",   0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.6p",   0x000003, 0x80000, CRC(2a2003e8) SHA1(f239b351759babe4683d16e745a5ac2f3c2ab06b) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxg )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1d.u1210", 0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.u1211", 0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.u129",  0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.u1212", 0x000003, 0x80000, CRC(6bde6edc) SHA1(c3565a55b858c10659fd9b93b1cd92bc39e6446d) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxpt )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("seibu_1d.u1210", 0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("seibu_2d.u1211", 0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("seibu_3d.u129",  0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("seibu_4d.u1212", 0x000003, 0x80000, CRC(e4fa53ac) SHA1(640257420beba6acaeaf687fde34dd20aef7c41a) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxa1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("dx_1h.4n",   0x000000, 0x80000, BAD_DUMP CRC(7624c36b) SHA1(84c17f2988031210d06536710e1eac558f4290a1) ) // bad
	ROM_LOAD32_BYTE("dx_2h.4p",   0x000001, 0x80000, CRC(4940fdf3) SHA1(c87e307ed7191802583bee443c7c8e4f4e33db25) )
	ROM_LOAD32_BYTE("dx_3h.6n",   0x000002, 0x80000, CRC(6c495bcf) SHA1(fb6153ecc443dabc829dda6f8d11234ad48de88a) )
	ROM_LOAD32_BYTE("dx_4h.6k",   0x000003, 0x80000, CRC(9ed6335f) SHA1(66975204b120915f23258a431e19dbc017afd912) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxa2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1d.bin",   0x000000, 0x80000, CRC(22b155ae) SHA1(388151e2c8fb301bd5bc66a974e9fe16816ae0bc) )
	ROM_LOAD32_BYTE("2d.bin",   0x000001, 0x80000, CRC(2be98ca8) SHA1(491e990405b0ad3de45bdbcc2453af9215ae19c8) )
	ROM_LOAD32_BYTE("3d.bin",   0x000002, 0x80000, CRC(b4785576) SHA1(aa5eee7b0c635c6d18a7fc1e037bf570a677dd90) )
	ROM_LOAD32_BYTE("4d.bin",   0x000003, 0x80000, CRC(5a77f7b4) SHA1(aa757e6308893ca63963170c5b1743de7c7ab034) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxk )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("rdxj_1.bin",   0x000000, 0x80000, CRC(b5b32885) SHA1(fb3c592b2436d347103c17bd765176062be95fa2) )
	ROM_LOAD32_BYTE("rdxj_2.bin",   0x000001, 0x80000, CRC(7efd581d) SHA1(4609a0d8afb3d62a38b461089295efed47beea91) )
	ROM_LOAD32_BYTE("rdxj_3.bin",   0x000002, 0x80000, CRC(55ec0e1d) SHA1(6be7f268df51311a817c1c329a578b38abb659ae) )
	ROM_LOAD32_BYTE("rdxj_4.bin",   0x000003, 0x80000, CRC(f8fb31b4) SHA1(b72fd7cbbebcf3d1b2253c309fcfa60674776467) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxu )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1a.u1210", 0x000000, 0x80000, CRC(53e63194) SHA1(a957330e14649cf46ad27fb99c460576c59e60b1) )
	ROM_LOAD32_BYTE("2a.u1211", 0x000001, 0x80000, CRC(ec8d1647) SHA1(5ceae132c6c09d6bb8565e9141ee1170bbdfd5fc) )
	ROM_LOAD32_BYTE("3a.u129",  0x000002, 0x80000, CRC(7dbfd73d) SHA1(43cb1dbc3ccbded64fc300c262d1fd528e0391a2) )
	ROM_LOAD32_BYTE("4a.u1212", 0x000003, 0x80000, CRC(cb41a459) SHA1(532f0ed00a5b50a7537e5f48884d632aa5b92fb0) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxnl )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("u1210_4n.bin", 0x000000, 0x80000, CRC(c589019a) SHA1(9bdd7f7d0bca16d67ba234d8a1fed5d2c8ab7191) )
	ROM_LOAD32_BYTE("u1211_4p.bin", 0x000001, 0x80000, CRC(b2222254) SHA1(b0e41d88111a96f0c0fb11b20ea99f436e8d493d) )
	ROM_LOAD32_BYTE("u129_6n.bin",  0x000002, 0x80000, CRC(60f04634) SHA1(50f1b721a017d879838d920cf5d5355aa024e09b) )
	ROM_LOAD32_BYTE("u1212_6p.bin", 0x000003, 0x80000, CRC(21ec37cc) SHA1(6da629e2bb5bd4c2192156af017148e99e274544) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxj )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("rdxj_1.u1211", 0x000000, 0x80000, CRC(5af382e1) SHA1(a11fc181da322f484815f55a510ce7e6c7df2d60) )
	ROM_LOAD32_BYTE("rdxj_2.u0212", 0x000001, 0x80000, CRC(899966fc) SHA1(0f91c2b05a44afb4c4b74e115a8fa530fb6d6414) )
	ROM_LOAD32_BYTE("rdxj_3.u129",  0x000002, 0x80000, CRC(e7f08013) SHA1(1f99672d8fdbda847c6552da210c417b21ca78ac) )
	ROM_LOAD32_BYTE("rdxj_4.u1212", 0x000003, 0x80000, CRC(78037e1f) SHA1(8d9c4188ca808e670e330e70e906bb1d27e36492) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "rdxj_7.u0724", 0x000000, 0x020000, CRC(ec31fa10) SHA1(e39c9d95699dbeb21e3661d863eee503c9011bbc) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxja )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1.bin", 0x000000, 0x80000, CRC(247e21c7) SHA1(69e9a084407f57e8433f832a592cfbba2757631f) )
	ROM_LOAD32_BYTE("2.bin", 0x000001, 0x80000, CRC(f2e9855a) SHA1(e27ac94cdb4ce8e8a98b342fbaf0fde11d9ab9ef) )
	ROM_LOAD32_BYTE("3.bin", 0x000002, 0x80000, CRC(fbab727f) SHA1(5415ff87dd967e52b5cf7d754c046223cfa1147b) )
	ROM_LOAD32_BYTE("4.bin", 0x000003, 0x80000, CRC(a08d5838) SHA1(ec07770503eefa5f22d30f40e3df2a02813908e4) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "rdxj_7.u0724", 0x000000, 0x020000, CRC(ec31fa10) SHA1(e39c9d95699dbeb21e3661d863eee503c9011bbc) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END

ROM_START( raidendxch )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("rdxc_1.u1210", 0x000000, 0x80000, CRC(2154c6ae) SHA1(dc794f8ddbd8a6267db37fe4e3ed44e06e9b84b7) )
	ROM_LOAD32_BYTE("rdxc_2.u1211", 0x000001, 0x80000, CRC(73bb74b7) SHA1(2f197adbe89d96c9e75054c568c380fdd2e80162))
	ROM_LOAD32_BYTE("rdxc_3.u129",  0x000002, 0x80000, CRC(50f0a6aa) SHA1(68579f8e73fe06b458368ac9cac0b33370cf3b4e))
	ROM_LOAD32_BYTE("rdxc_4.u1212", 0x000003, 0x80000, CRC(00071e70) SHA1(8a03ea0e650936e48cdd21ff84132742649920fe) )

	// no other ROMs present with this set, so the ones below could be wrong
	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD( "seibu_7.u0724",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x100000, "oki1", 0 )   // ADPCM samples
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	// Common Raiden DX soldered mask ROMs below
	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) // Shared with original Raiden 2

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "sprites", 0 ) // sprite gfx (encrypted)
	ROM_LOAD32_WORD( "raiden_2_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "raiden_2_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) // Shared with original Raiden 2
	ROM_LOAD32_WORD( "dx_obj-3.u0837",       0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836",       0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki2", 0 )   // ADPCM samples
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) // Mask ROM - Shared with original Raiden 2
ROM_END



// Zero Team sets
/* Zero team is slightly older hardware (early 93 instead of late 93) but
almost identical to raiden 2 with a few key differences:
Zero Team:                     Raiden 2:
BG/FG ROMs marked MUSHA        BG/FG ROMs marked RAIDEN 2
SEI251 fg/sprite gate array    SEI252 fg/sprite gate array
about 15 74xx logic chips      SEI360 gate array
3x dipswitch arrays            2x dipswitch arrays
4x 8bit program ROMs           2x 16bit program ROMs (some older pcbs have 4x 8bit like zt)
YM3812 plus Y3014              YM2151 plus Y3012 plus NJM4550 (some older pcbs have YM2151, Y3014)
1x OKI M6295 & voice ROM       2x OKI M6295s & 2x voice ROMs
2x 8bit licensee bg ROMs       1x 16bit licensee bg ROM
2x fg/sprite mask ROMs         4x fg/sprite mask ROMs
4x pals (two are stacked)      2x pals
*/
/* ZERO TEAM Seibu Kaihatsu 1993

(C) 1993 ZERO TEAM SEIBU KAIHATSU INC.,o
|----------------------------------------------------------|
|      1    2   3   4   5   6    7      8      9     10    |
|LA4460    M6295   6   Z8400A          BATTERY3.6v        A|
|   YM3812       LH5116 5                                 B|
|     VOL   YM3014                                         |
|HB-45A         |------|                               C|
|HB-2           |SIE150|    LH5116    |---------|          |
|RC220          |      |    LH5116    | SEI251  | 28.6360 D|
|RC220          |------|              |SB03-012 | MHz      |
|RC220                                |(QFP208) |         E|
|J                     OBJ-2    OBJ-1 |         |         F|
|A                                    |---------|          |
|M                                                        G|
|M                                              LH522258   |
|A                                              LH522258  H|
|                                               LH522258  J|
|               DSW1(8) PAL2  1       2         LH522258  K|
|           DSW2(8)     PAL14 4       3     |---------|   L|
|           DSW3(8) PAL3                    |SEI1000  |   M|
|                           COPX-D2         |SB01-001 |   N|
|                                           |(QFP184) |    |
|         CXK5863                           |         |   P|
|         CXK5863      |------|             |---------|    |
|                      |SEI200|                           Q|
|                      |      |  8             |----|     R|
|                      |------|                |V30 |      |
|       BACK-2     BACK-1        7             |----|     S|
|----------------------------------------------------------|
Notes:
      V30 clock    - 16.000MHz [32/2]. Chip is stamped "NEC D70116HG-16 V30 NEC '84" (QFP52)
      Z80 clock    - 3.579545MHz [28.63636/8]
      YM3812 clock - 3.579545MHz [28.63636/8]
      Yamaha DAC   - ym3014 mono dac
      M6295 clocks - 1.022MHz [28.63636/28] and pin 7 HIGH
      LH52258      - Sharp LH52258 32k x8 SRAM (= 62256)
      CXK5863      - Sony CXK5863 8k x8 SRAM (= 6264)
      6116         - 2k x8 SRAM
      LH5116       - 2k x8 SRAM
      HB-45A       - Seibu custom ceramic module sound DAC (SIP20)
      HB-2         - Seibu custom ceramic module connected to coin counters (SIP10)
      RC220        - Custom resistor network module used for inputs (SIP14)
      VSync        - 55.4859Hz  \
      HSync        - 15.5586kHz / not measured but assumed same as Raiden 2 DX
      PAL14        - Two pals in a stack, along with a resistor and wires to sei0200 and the sie150
                     'V3C004X'  (DIP20), has a resistor between one pin and gnd
                     <unknown, maybe V3C001, under above pal> (DIP20) u0310
      PAL2         - TIBPAL16L8-25CN stamped 'V3C002' (DIP20) u0322
      PAL3         - AMI 18CV8P-15 stamped 'V3C003' (DIP20) u0619
      ROMs         - 6         - 27C020 EPROM labelled 'SEIBU 6' at location U105 (DIP32), pcb labeled VOICE
                     5         - 27C512 EPROM labelled 'SEIBU 5' at location U1110 (DIP28)
                     *OBJ-1    - 16Mbit TC5316200BP MaskROM stamped 'MUSHA OBJ-1' at location U0811 (DIP42)
                     *OBJ-2    - 16Mbit TC5316200BP MaskROM stamped 'MUSHA OBJ-2' at location U082 (DIP42)
                     1         - 27C020 EPROM labelled 'SEIBU 1' at location U024 (DIP32)
                     2         - 27C020 EPROM labelled 'SEIBU 2' at location U025 (DIP32)
                     3         - 27C020 EPROM labelled 'SEIBU 3' at location U023 (DIP32)
                     4         - 27C020 EPROM labelled 'SEIBU 4' at location U026 (DIP32)
                     *BACK-1   - 8Mbit TC538200AP MaskROM stamped 'MUSHA BACK-1' at location U075 (DIP42)
                     *BACK-2   - 4Mbit TC534200AP MaskROM stamped 'MUSHA BACK-2' at location U0714 (DIP40)
                     7         - 27C512 EPROM labelled 'SEIBU 7' at location U072 (DIP28)
                     8         - 27C512 EPROM labelled 'SEIBU 8' at location U077 (DIP28)
                     *COPX-D2  - 2M MaskROM stamped 'COPX-D2' at location U0313 (DIP40)

                     * = these ROMs are soldered-in

      SEIBU Custom ICs -
                        SIE150 (QFP100) - z80 interface
                        SEI251 SB03-012 (QFP208) - fg/sprite gfx and its decryption
                        SEI0200 TC110G21AF 0076 (QFP100) - bg gfx
                        SEI1000 SB01-001 (QFP184) - main protection

*/


ROM_START( zeroteam ) // Fabtek, US licensee, displays 'USA' under zero team logo, board had serial 'Seibu Kaihatsu No. 0001468' on it, as well as AAMA 0458657
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("seibu__1.u024.5k",   0x000000, 0x40000, CRC(25aa5ba4) SHA1(40e6047620fbd195c87ac3763569af099096eff9) ) // alternate label "1"
	ROM_LOAD32_BYTE("seibu__3.u023.6k",   0x000002, 0x40000, CRC(ec79a12b) SHA1(515026a2fca92555284ac49818499af7395783d3) ) // alternate label "3"
	ROM_LOAD32_BYTE("seibu__2.u025.6l",   0x000001, 0x40000, CRC(54f3d359) SHA1(869744185746d55c60d2f48eabe384a8499e00fd) ) // alternate label "2"
	ROM_LOAD32_BYTE("seibu__4.u026.5l",   0x000003, 0x40000, CRC(a017b8d0) SHA1(4a93ff1ab18f4b61c7ef580995f64840c19ce6b9) ) // alternate label "4"

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu__5.u1110.5b",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // // alternate label "5"
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "seibu__7.u072.5s",    0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) ) // alternate label "7"
	ROM_LOAD16_BYTE( "seibu__8.u077.5r",    0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) ) // alternate label "8"

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "seibu__6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // alternate label "6"

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteama ) // No licensee, original japan?
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1.u024.5k",   0x000000, 0x40000, CRC(bd7b3f3a) SHA1(896413901a429d0efa3290f61920063c81730e9b) )
	ROM_LOAD32_BYTE("3.u023.6k",   0x000002, 0x40000, CRC(19e02822) SHA1(36c9b887eaa9b9b67d65c55e8f7eefd08fe0be15) )
	ROM_LOAD32_BYTE("2.u025.6l",   0x000001, 0x40000, CRC(0580b7e8) SHA1(d4416264aa5acdaa781ebcf51f128b3e665cc903) )
	ROM_LOAD32_BYTE("4.u026.5l",   0x000003, 0x40000, CRC(cc666385) SHA1(23a8878315b6009dcc1f27e49572e5be29f6a1a6) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5.a.u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "7.a.u072.5s", 0x000000,   0x010000, CRC(eb10467f) SHA1(fc7d576dc41bc878ff20f0370e669e19d54fcefb) )
	ROM_LOAD16_BYTE( "8.a.u077.5r", 0x000001,   0x010000, CRC(a0b2a09a) SHA1(9b1f6c732000b84b1ad635f332ebead5d65cc491) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

/* this set, consisting of updated program ROMs, is a later version or hack of zero team to incorporate the writing
of the fg sei251 'key data' to the pcb on bootup (like raiden 2 does) rather than relying on the sram to hold the
keys as programmed from factory (or via the suicide revival kit below); hence this ROMset is immune to the common
problem of the 3.6v lithium battery dying and the missing keys to cause the sprites to show up as gibberish */
// note: it is possible *but not proven* that this specific set in mame is a frankenstein-hybrid of the japan and us
// sets, using the sound and char ROMs from us set and code from later japan set. This would make sense if it was dumped
// from a 'fixed, suicide free' modified us board where someone swapped in the later suicideless japan code ROMs.
ROM_START( zeroteamb ) // No licensee, later japan?
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1b.u024.5k",   0x000000, 0x40000, CRC(157743d0) SHA1(f9c84c9025319f76807ef0e79f1ee1599f915b45) )
	ROM_LOAD32_BYTE("3b.u023.6k",   0x000002, 0x40000, CRC(fea7e4e8) SHA1(08c4bdff82362ae4bcf86fa56fcfc384bbf82b71) )
	ROM_LOAD32_BYTE("2b.u025.6l",   0x000001, 0x40000, CRC(21d68f62) SHA1(8aa85b38e8f36057ef6c7dce5a2878958ce93ce8) )
	ROM_LOAD32_BYTE("4b.u026.5l",   0x000003, 0x40000, CRC(ce8fe6c2) SHA1(69627867c7866e43e771ab85014553117044d18d) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5.u1110.5b",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "7.u072.5s",   0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.u077.5r",   0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteamc ) // Liang Hwa, Taiwan licensee, no special word under logo on title
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("b1.u024.5k",   0x000000, 0x40000, CRC(528de3b9) SHA1(9ca8cdc0212f2540e852d20ab4c04f68b967d024) )
	ROM_LOAD32_BYTE("b3.u023.6k",   0x000002, 0x40000, CRC(3688739a) SHA1(f98f461fb8e7804b3b4020a5e3762d36d6458a62) )
	ROM_LOAD32_BYTE("b2.u025.6l",   0x000001, 0x40000, CRC(5176015e) SHA1(6b372564b2f1b1f56cae0c98f4ca588b784bfa3d) )
	ROM_LOAD32_BYTE("b4.u026.5l",   0x000003, 0x40000, CRC(c79925cb) SHA1(aaff9f626ec61bc0ff038ebd722fe361dccc49fb) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5.c.u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "b7.u072.5s",  0x000000,   0x010000, CRC(30ec0241) SHA1(a0d0be9458bf97cb9764fb85c988bb816710475e) )
	ROM_LOAD16_BYTE( "b8.u077.5r",  0x000001,   0x010000, CRC(e18b3a75) SHA1(3d52bba8d47d0d9108ee79014fd64d6e856a3fde) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "6.c.u105.4a", 0x00000, 0x40000,  CRC(b4a6e899) SHA1(175ab656db3c3258ff10eede89890f62435d2298) )

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteamd ) // Dream Soft, Korea licensee, no special word under logo on title; board had serial 'no 1041' on it.
	// this is weird, on other zt sets the ROM order is 1 3 2 4, but this one is 1 3 4 2. blame seibu or whoever marked the ROMs, which were labeled in pen
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1.d.u024.5k",   0x000000, 0x40000, CRC(6cc279be) SHA1(63143ba3105d24d133e60ffdb3edc2ceb2d5dc5b) )
	ROM_LOAD32_BYTE("3.d.u023.6k",   0x000002, 0x40000, CRC(0212400d) SHA1(28f77b5fddb9d724b735c3ff2255bd518b166e67) )
	ROM_LOAD32_BYTE("4.d.u025.6l",   0x000001, 0x40000, CRC(08813ebb) SHA1(454779cec2fd0e71b72f7161e7d9334893ee42de) )
	ROM_LOAD32_BYTE("2.d.u026.5l",   0x000003, 0x40000, CRC(9236129d) SHA1(8561ab62e3593cd9353d9ffddedbdb77e9ae2c45) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "512kb.u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) ) // this is a soldered mask ROM on this pcb version! the contents match the taiwan version EPROM; the mask ROM has no label
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "512kb.u072.5s",   0x000000,   0x010000, CRC(30ec0241) SHA1(a0d0be9458bf97cb9764fb85c988bb816710475e) ) // this is a soldered mask ROM on this pcb version! the contents match the taiwan version EPROM; the mask ROM has no label
	ROM_LOAD16_BYTE( "512kb.u077.5r",   0x000001,   0x010000, CRC(e18b3a75) SHA1(3d52bba8d47d0d9108ee79014fd64d6e856a3fde) ) // this is a soldered mask ROM on this pcb version! the contents match the taiwan version EPROM; the mask ROM has no label

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "8.u105.4a", 0x00000, 0x40000,  CRC(b4a6e899) SHA1(175ab656db3c3258ff10eede89890f62435d2298) ) // same ROM as '6' labeled one in zeroteamc above but has '8' written on label in pen

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END
// A version of the above exists (which dr.kitty used to own) which DOES have 'Korea' under the logo on title, needs dumping

ROM_START( zeroteame ) // shares chars ROMs with zeroteama
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("seibu_1_u024.5k",   0x000000, 0x40000, CRC(76e69af5) SHA1(c5a4b2491cee3f4f9694e454f3828fda33fff3ab) )
	ROM_LOAD32_BYTE("seibu_3_u023.6k",   0x000002, 0x40000, CRC(4a904880) SHA1(4a85a62446ea11d57369dfd65f143475063abc31) )
	ROM_LOAD32_BYTE("seibu_2_u025.6l",   0x000001, 0x40000, CRC(b97ab448) SHA1(82d9e7aa6b69c214ad82e5ff30a049cecab607c0) )
	ROM_LOAD32_BYTE("seibu_4_u026.5l",   0x000003, 0x40000, CRC(1d43b9c1) SHA1(b1144d49f8fddf416fe2e3eae4040b369eb212cf) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "seibu_5_u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "seibu_7_u072.5s", 0x000000,   0x010000, CRC(eb10467f) SHA1(fc7d576dc41bc878ff20f0370e669e19d54fcefb) )
	ROM_LOAD16_BYTE( "seibu_8_u077.5r", 0x000001,   0x010000, CRC(a0b2a09a) SHA1(9b1f6c732000b84b1ad635f332ebead5d65cc491) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "seibu_6_u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteams ) // No license, displays 'Selection' under logo
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1_sel.bin",   0x000000, 0x40000, CRC(d99d6273) SHA1(21dccd5d71c720b8364406835812b3c9defaff6c) )
	ROM_LOAD32_BYTE("3_sel.bin",   0x000002, 0x40000, CRC(0a9fe0b1) SHA1(3588fe19788f77d07e9b5ab8182b94362ffd0024) )
	ROM_LOAD32_BYTE("2_sel.bin",   0x000001, 0x40000, CRC(4e114e74) SHA1(fcccbb68c6b7ffe8d109ed3a1ec9120d338398f9) )
	ROM_LOAD32_BYTE("4_sel.bin",   0x000003, 0x40000, CRC(0df8ba94) SHA1(e07dce6cf3c3cfe1ea3b7f01e18833c1da5ed1dc) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5_sel.bin",  0x000000, 0x08000, CRC(ed91046c) SHA1(de815c999aeeb814d3f091d5a9ac34ea9a388ddb) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "7.u072.5s",   0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.u077.5r",   0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

/*
"Zero Team Suicide Revival Kit"

As the name implies, this is used to give life again to a "suicided" ZT PCB, where the 3.6v
lithium battery which backs up the FG/sprite encryption keys has died, and the sprites display
as garbage blocks.
To use: replace the 3.6v battery with a working one, and then remove the normal four code ROMs
and install these instead.
Boot the pcb, it should rewrite the sei251 decryption keys and display a message on screen.
Next, turn off power and reinsert the old code ROMs, and the pcb should now have working sprites.
*/

ROM_START( zeroteamsr )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("zteam1.u24",   0x000000, 0x40000, CRC(c531e009) SHA1(731881fca3dc0a8269ecdd295ba7119d93c892e7) )
	ROM_LOAD32_BYTE("zteam3.u23",   0x000002, 0x40000, CRC(1f988808) SHA1(b1fcb8c96e57c4942bc032d42408d7289c6a3681) )
	ROM_LOAD32_BYTE("zteam2.u25",   0x000001, 0x40000, CRC(b7234b93) SHA1(35bc093e8ad4bce1d2130a392ed1b9487a5642a1) )
	ROM_LOAD32_BYTE("zteam4.u26",   0x000003, 0x40000, CRC(c2d26708) SHA1(d65191b40f5dd7cdbbc004e2de10134db6092fd1) )

	ROM_REGION( 0x40000, "math", 0 )   // COPX
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "5.5c",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "7.u072.5s",   0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.u077.5r",   0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (encrypted) (diff encrypt to raiden2? )
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a

	ROM_REGION( 0x10000, "pals", 0 )    // PALS
	ROM_LOAD( "v3c001.pal.u0310",            0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322",  0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619",   0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END


/*

X Se Dae Quiz
Seibu/Dream Island, 1995

This game runs on a Zero Team PCB

PCB Layout
ZERO TEAM-V2 SEIBU KAIHATSU INC.
|----------------------------------------|
|LA4460 YM2151 M6295 9  Z80    Y         |
|HB-46A1 YM3014 SEI150 8                 |
|VOL      6116     6116  28.6362MHz 6116 |
|                  6116   SEI251    6116 |
|J HB-2                            62256 |
|A                  OBJ-2  OBJ-1   62256 |
|M                                 62256 |
|M      SW1(8) PAL    1      3     62256 |
|A             PAL    4      2  SEI1000  |
|   SW2(8)     6264                      |
|          PAL 6264          X           |
|   SW3(8)       SEI0200  5    D71011    |
|         7     BG-1      6         V30  |
|----------------------------------------|
Notes:
      PCB is identical to standard Zero Team PCB
      with the following differences....
      1. X - location for COPX ROM, not populated
      2. Y - location for battery, not populated
      3. NEC V30 and NEC D71011 are located on a sub board and
         the surface-mounted V30 (UPD70116) is not populated
      4. ROM7 is located in a 8M-DIP42 to 4M-DIP40 adapter and is a 27C4002 EPROM
      5. ROM8 has the top 4 pins hanging out of the DIP28 socket and is a 27C1001
         EPROM. Pins 30,31 & 32 are tied together and pin 2 is tied to the SEI150
         with a wire.
*/

ROM_START( xsedae )
	ROM_REGION( 0x200000, "maincpu", 0 ) // v30 main cpu
	ROM_LOAD32_BYTE("1.u024",   0x000000, 0x40000, CRC(185437f9) SHA1(e46950b6a549d11dc57105dd7d9cb512a8ecbe70) )
	ROM_LOAD32_BYTE("2.u025",   0x000001, 0x40000, CRC(a2b052df) SHA1(e8bf9ab3d5d4e601ea9386e1f2d4e017b025407e) )
	ROM_LOAD32_BYTE("3.u023",   0x000002, 0x40000, CRC(293fd6c1) SHA1(8b1a231f4bedbf9c0f347330e13fdf092b9888b4) )
	ROM_LOAD32_BYTE("4.u026",   0x000003, 0x40000, CRC(5adf20bf) SHA1(42a0bb5a460c656675b2c432c043fc61a9049276) )

	ROM_REGION( 0x40000, "math", ROMREGION_ERASEFF )   // COPX
	// Not populated

	ROM_REGION( 0x30000, "audiocpu", ROMREGION_ERASEFF ) // 64k code for sound Z80
	ROM_LOAD( "8.u1110",  0x000000, 0x08000, CRC(2dc2f81a) SHA1(0f6605042e0e295b4256b43dbdf5d53daebe1a9a) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_CONTINUE(0x20000,0x10000) // TODO
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "chars", 0 ) // chars
	ROM_LOAD16_BYTE( "6.u072.5s",   0x000000,   0x010000, CRC(a788402d) SHA1(8a1ac4760cf75cd2e32c1d15f36ad15cce3d411b) )
	ROM_LOAD16_BYTE( "5.u077.5r",   0x000001,   0x010000, CRC(478deced) SHA1(88cd72cb76bbc1c4255c3dfae4b9a10af9b050b2) )

	ROM_REGION( 0x400000, "tiles", 0 ) // background gfx
	ROM_LOAD( "bg-1.u075",   0x000000, 0x100000, CRC(ac087560) SHA1(b6473b20c55ec090961cfc46a024b3c5b707ec25) )
	ROM_LOAD( "7.u0714",     0x100000, 0x080000, CRC(296105dc) SHA1(c2b80d681646f504b03c2dde13e37b1d820f82d2) )

	ROM_REGION32_LE( 0x800000, "sprites", ROMREGION_ERASEFF ) // sprite gfx (not encrypted)
	ROM_LOAD32_WORD( "obj-1.u0811",  0x000000, 0x200000, CRC(6ae993eb) SHA1(d9713c79eacb4b3ce5e82dd3ce39003e3a433d8f) )
	ROM_LOAD32_WORD( "obj-2.u082",   0x000002, 0x200000, CRC(26c806ee) SHA1(899a76a1b3f933c6f5cb6b5dcdf5b58e1b7e49c6) )

	ROM_REGION( 0x100000, "oki", 0 )   // ADPCM samples
	ROM_LOAD( "9.u105.4a", 0x00000, 0x40000, CRC(a7a0c5f9) SHA1(7882681ac152642aa4f859071f195842068b214b) )
ROM_END

const u16 raiden2_state::raiden_blended_colors[] = {
	// bridge tunnel entrance shadow
	0x380,

	// cloud
	0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3c6, 0x3c7, 0x3c8, 0x3c9, 0x3ca, 0x3cb, 0x3cc, 0x3cd, 0x3ce,

	// engine
	0x3d0, 0x3d1, 0x3d2, 0x3d3, 0x3d4, 0x3d5, 0x3d6, 0x3d7, 0x3d8, 0x3d9, 0x3da, 0x3db, 0x3dc, 0x3dd, 0x3de,

	// level 1 boss legs
	0x3f0, 0x3f1, 0x3f2, 0x3f3, 0x3f4, 0x3f5, 0x3f6, 0x3f7, 0x3f8, 0x3f9, 0x3fa, 0x3fb, 0x3fc, 0x3fd, 0x3fe,

	// water
	0x4f8, 0x4f9, 0x4fa, 0x4fb, 0x4fc, 0x4fd, 0x4fe,
	0x5c8, 0x5c9, 0x5ca, 0x5cb, 0x5cc, 0x5cd, 0x5ce,

	// wall shadow
	0x5de,

	// glass roof
	0x5e8, 0x5e9, 0x5ea, 0x5eb, 0x5ec, 0x5ed, 0x5ee,

	// house shadow plus stage 3 boss green pools
	0x5f8, 0x5f9, 0x5fa, 0x5fb, 0x5fc, 0x5fd, 0x5fe,

	// water and trees
	0x6c8, 0x6c9, 0x6ca, 0x6cb, 0x6cc, 0x6cd, 0x6ce,
	0x6d8, 0x6d9, 0x6da, 0x6db, 0x6dc, 0x6dd, 0x6de,
	0x6e8, 0x6e9, 0x6ea, 0x6eb, 0x6ec, 0x6ed, 0x6ee,
	0x6f8, 0x6f9, 0x6fa, 0x6fb, 0x6fc, 0x6fd, 0x6fe,

	// stage end panel, raiden dx logo plus misc stuff
	0x70d, 0x70e,
	0x71c, 0x71d, 0x71e,
	0x72d, 0x72e,
	0x73d, 0x73e,
	0x74d, 0x74e,
	0x75c,
	0x76c, 0x76d, 0x76e,
	0x77d, 0x77e,

	// logo in attract mode
	0x7c8, 0x7c9, 0x7ca, 0x7cb, 0x7cc, 0x7cd, 0x7ce,

	0xffff,
};

void raiden2_state::init_blending(const u16 *table)
{
	for (auto & elem : m_blend_active)
		elem = false;
	while (*table != 0xffff)
		m_blend_active[*table++] = true;
}

void raiden2_state::init_raiden2()
{
	init_blending(raiden_blended_colors);
	static const int spri[5] = { 0, 1, 2, 3, -1 };
	m_cur_spri = spri;
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x20000);
	raiden2_decrypt_sprites(machine());
}

void raiden2_state::init_raidendx()
{
	init_blending(raiden_blended_colors);
	static const int spri[5] = { 0, 1, 2, 3, -1 };
	m_cur_spri = spri;
	m_mainbank->configure_entries(0, 0x10, memregion("maincpu")->base() + 0x100000, 0x10000);
	raiden2_decrypt_sprites(machine());
}

const u16 raiden2_state::xsedae_blended_colors[] = {
	0xffff,
};

void raiden2_state::init_xsedae()
{
	init_blending(xsedae_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	m_cur_spri = spri;
	// doesn't have banking
}

const u16 raiden2_state::zeroteam_blended_colors[] = {
	// Player selection
	0x37e,
	// Boss spear shadow
	0x38e,
	// Scaffolding shadow
	0x52e,
	// Road brightening
	0x5de,

	0xffff
};


void raiden2_state::init_zeroteam()
{
	init_blending(zeroteam_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	m_cur_spri = spri;
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x20000);
	zeroteam_decrypt_sprites(machine());
}

// GAME DRIVERS

/* The Raiden 2 / DX sets are sorted by the checksums of the non-regional ROMs (the final program ROM contains the region byte)

   it's interesting to note that most Raiden DX sets are unique, very few differ only by region byte, but for Raiden 2 there are many where the program ROMs only differ by region byte
   both Raiden 2 'harder' sets currently dumped are Korea region, but the Korea region byte does not determine the difficulty.
*/

// Raiden 2 sets

// Regular version - Sepia high score table background, regular tanks on first bridge

// code rev with first ROM having checksum 09475ec4
GAME( 1993, raiden2,    0,        raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (US, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2g,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden II (Germany)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2hk,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden II (Hong Kong)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2j,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2sw,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Switzerland)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first ROM having checksum b16df955
GAME( 1993, raiden2u,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (US, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first ROM having checksum 53be3dd0
GAME( 1993, raiden2f,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (France)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2nl,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Holland)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2es,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Spain)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first ROM having checksum c1fc70f5
GAME( 1993, raiden2i,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Italy)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2au,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Australia)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

// Easy version - Coloured high score table background, different enemy placement

// code rev with first ROM having checksum 2abc848c
GAME( 1993, raiden2e,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (easier, Korea)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // (Region 0x04) - Korea, if regions are the same as RDX, no license or region message tho
GAME( 1993, raiden2eub, raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (easier, US set 3)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // (Region 0x01) - PRG0 is same as raiden2e, but PRG1 has region byte different than raiden2e, other ROMs match raiden2u
// code rev with first ROM having checksum ed1514e3 (using 4x program ROM configuration, not 2) would have crc 2abc848c in 2 ROM config, so same rev as above
GAME( 1993, raiden2eua, raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (easier, US set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2eg,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden II (easier, Germany)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

// unique revision (4x program ROM configuration)
GAME( 1993, raiden2eup, raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (easier, US, prototype? 11-16)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // program ROMs had 11-16 date

// code rev with first ROM having checksum d7041be4
GAME( 1993, raiden2ea,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (easier, Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // (Region 0x00) - Japan, but the easy sets have no 'FOR USE IN JAPAN ONLY' display even when region is 00
GAME( 1993, raiden2eu,  raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (easier, US set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) //  ^

// Harder version - Sepia high score table background, red tanks on first bridge

// code rev with first ROM having checksum 1fcc08cf
GAME( 1993, raiden2k,   raiden2,  raiden2,  raiden2,  raiden2_state, init_raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (harder, Korea)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // (Region 0x04) - Korea, no message displayed tho
// code rev with first ROM having checksum 413241e0 (using 4x program ROM configuration, not 2, on Raiden DX hardware)
GAME( 1993, raiden2dx,  raiden2,  raidendx, raiden2,  raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden II (harder, Raiden DX hardware, Korea)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // ^


// Raiden DX sets

// code rev with the first 3 ROMs having checksums 14d725fc, 5e7e45cb, f0a47e67
GAME( 1994, raidendx,   0,        raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (UK)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxg,  raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden DX (Germany)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxpt, raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Portugal)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums 7624c36b, 4940fdf3, 6c495bcf
GAME( 1994, raidendxa1, raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden DX (Hong Kong, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums 22b155ae, 2be98ca8, b4785576
GAME( 1994, raidendxa2, raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden DX (Hong Kong, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums b5b32885, 7efd581d, 55ec0e1d
GAME( 1994, raidendxk,  raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Korea)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums 53e63194, ec8d1647, 7dbfd73d
GAME( 1994, raidendxu,  raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden DX (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums c589019a, b2222254, 60f04634
GAME( 1994, raidendxnl, raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Holland)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums 5af382e1, 899966fc, e7f08013
GAME( 1994, raidendxj,  raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Japan, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums 247e21c7, f2e9855a, fbab727f
GAME( 1994, raidendxja, raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Japan, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// code rev with first 3 ROMs having checksums 2154c6ae, 73bb74b7, 50f0a6aa
GAME( 1994, raidendxch, raidendx, raidendx, raidendx, raiden2_state, init_raidendx, ROT270, "Seibu Kaihatsu (Ideal International Development Corp license)", "Raiden DX (China)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // Region byte is 0x16, defined as "MAIN LAND CHINA" for this set only


// Zero Team sets

GAME( 1993, zeroteam,   0,        zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu (Fabtek license)", "Zero Team USA (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteama,  zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team (Japan?, earlier?, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteamb,  zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team (Japan?, later batteryless)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // reprograms the sprite decrypt data of the SEI251 on every boot, like raiden2 does. hack?
GAME( 1993, zeroteamc,  zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu (Liang Hwa license)", "Zero Team (Taiwan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteamd,  zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu (Dream Soft license)", "Zero Team (Korea)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteame,  zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team (Japan?, earlier?, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteams,  zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team Selection", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteamsr, zeroteam, zeroteam, zeroteam, raiden2_state, init_zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team Suicide Revival Kit", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // reprograms the sprite decrypt data of the SEI251 only, no game code


// X Se Dae Quiz sets

GAME( 1995, xsedae,     0,        xsedae,   xsedae,   raiden2_state, init_xsedae,   ROT0,   "Dream Island", "X Se Dae Quiz (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
