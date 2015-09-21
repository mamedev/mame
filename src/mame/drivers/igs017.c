// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia
/************************************************************************************************************

                                        -= IGS017 / IGS031 Based Hardware =-

                             driver by   Pierpaolo Prazzoli, Luca Elia (l.elia@tin.it)

CPU:   Z180 or 68000
Video: IGS017 or IGS031 (2 tilemaps, variable size sprites, protection)
Other: IGS025 (8255), IGS022 (protection, MCU), IGS029 (protection)
Sound: M6295 + [YM2413]

-------------------------------------------------------------------------------------------------------------
Year + Game                     PCB        CPU    Sound         Custom                Other
-------------------------------------------------------------------------------------------------------------
96  Shu Zi Le Yuan              NO-0131-4  Z180   M6295 YM2413  IGS017 8255           Battery
97  Mj Super Da Man Guan II     NO-0147-6  68000  M6295         IGS031 8255           Battery
97  Mj Tian Jiang Shen Bing     NO-0157-2  Z180   M6295 YM2413  IGS017 IGS025         Battery
97  Mj Man Guan Da Heng         NO-0252    68000  M6295         IGS031 IGS025 IGS???* Battery
98  Mj Long Hu Zheng Ba 2       NO-0206    68000  M6295         IGS031 IGS025 IGS022* Battery
98  Mj Shuang Long Qiang Zhu 2  NO-0207    68000  M6295         IGS031 IGS025 IGS022  Battery
98  Mj Man Guan Cai Shen        NO-0192-1  68000  M6295         IGS017 IGS025 IGS029  Battery
99? Tarzan (V107)?              NO-0248-1  Z180   M6295         IGS031 IGS025         Battery
99? Tarzan (V109C)?             NO-0228?   Z180   M6295         IGS031 IGS025 IGS029  Battery
00? Super Tarzan (V100I)        NO-0230-1  Z180   M6295         IGS031 IGS025         Battery
??  Super Poker / Formosa       NO-0187    Z180   M6295 YM2413  IGS017 IGS025         Battery
-------------------------------------------------------------------------------------------------------------
                                                                                    * not present in one set
To Do:

- Protection emulation, instead of patching the roms.
- NVRAM.
- iqblockf: protection.
- mgcs: implement joystick inputs. Finish IGS029 protection simulation.

Notes:

- iqblocka: keep start or test pressed during boot to enter test mode A or B.
- lhzb2, mgcs, tjsb: press service + stats during test mode for sound test.
- mgdh: press A + B during test mode for sound test (B1+B2+B3 when using a joystick).
- mgdh: test mode is accessed by keeping test pressed during boot (as usual), but pressing F2+F3 in MAME
  does not actually work. It does work if F2 is pressed in the debug window at boot, and held while closing it.

************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"
#include "machine/igs025.h"
#include "machine/igs022.h"
#include "machine/ticket.h"
#include "video/igs017_igs031.h"

class igs017_state : public driver_device
{
public:
	igs017_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_input_addr(-1),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_hopperdev(*this, "hopper"),
		m_igs025(*this,"igs025"),
		m_igs022(*this,"igs022"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_igs017_igs031(*this, "igs017_igs031")
	{ }

	int m_input_addr;
	required_device<cpu_device> m_maincpu;

	required_device<okim6295_device> m_oki;
	optional_device<ticket_dispenser_device> m_hopperdev;
	optional_device<igs025_device> m_igs025; // Mj Shuang Long Qiang Zhu 2
	optional_device<igs022_device> m_igs022; // Mj Shuang Long Qiang Zhu 2
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;
	required_device<igs017_igs031_device> m_igs017_igs031;


	void igs025_to_igs022_callback( void );



	UINT8 m_input_select;
	UINT8 m_hopper;
	UINT16 m_igs_magic[2];
	UINT8 m_scramble_data;

	UINT8 m_dsw_select;

	// lhzb2a protection:
	UINT16 m_prot_regs[2], m_prot_val, m_prot_word, m_prot_m3, m_prot_mf;
	UINT8 m_prot2;

	// IGS029 protection (communication)
	UINT8 m_igs029_send_data, m_igs029_recv_data;
	UINT8 m_igs029_send_buf[256], m_igs029_recv_buf[256];
	int m_igs029_send_len, m_igs029_recv_len;
	// IGS029 protection (mgcs)
	UINT32 m_igs029_mgcs_long;


	DECLARE_WRITE8_MEMBER(input_select_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE16_MEMBER(mgcs_magic_w);
	DECLARE_READ16_MEMBER(mgcs_magic_r);

	UINT16 mgcs_palette_bitswap(UINT16 bgr);
	UINT16 lhzb2a_palette_bitswap(UINT16 bgr);
	UINT16 tjsb_palette_bitswap(UINT16 bgr);
	UINT16 slqz2_palette_bitswap(UINT16 bgr);

	DECLARE_READ8_MEMBER(sdmg2_keys_r);
	DECLARE_WRITE16_MEMBER(sdmg2_magic_w);
	DECLARE_READ16_MEMBER(sdmg2_magic_r);
	DECLARE_READ8_MEMBER(mgdh_keys_r);
	DECLARE_WRITE16_MEMBER(mgdha_magic_w);
	DECLARE_READ16_MEMBER(mgdha_magic_r);

	DECLARE_WRITE8_MEMBER(tjsb_output_w);
	DECLARE_READ8_MEMBER(tjsb_input_r);
	DECLARE_READ8_MEMBER(spkrform_input_r);

	DECLARE_READ16_MEMBER(lhzb2a_input_r);
	DECLARE_WRITE16_MEMBER(lhzb2a_input_addr_w);
	DECLARE_WRITE16_MEMBER(lhzb2a_input_select_w);

	DECLARE_WRITE16_MEMBER(lhzb2a_prot_w);
	DECLARE_READ16_MEMBER(lhzb2a_prot_r);

	DECLARE_WRITE16_MEMBER(lhzb2a_prot2_reset_w);
	DECLARE_WRITE16_MEMBER(lhzb2a_prot2_inc_w);
	DECLARE_WRITE16_MEMBER(lhzb2a_prot2_dec_w);
	DECLARE_READ16_MEMBER(lhzb2a_prot2_r);

	DECLARE_WRITE16_MEMBER(lhzb2_magic_w);
	DECLARE_READ16_MEMBER(lhzb2_magic_r);

	DECLARE_WRITE16_MEMBER(slqz2_magic_w);
	DECLARE_READ16_MEMBER(slqz2_magic_r);
	DECLARE_READ8_MEMBER(mgcs_keys_r);
	DECLARE_DRIVER_INIT(iqblocka);
	DECLARE_DRIVER_INIT(mgdh);
	DECLARE_DRIVER_INIT(slqz2);
	DECLARE_DRIVER_INIT(lhzb2);
	DECLARE_DRIVER_INIT(starzan);
	DECLARE_DRIVER_INIT(mgcs);
	DECLARE_DRIVER_INIT(tjsb);
	DECLARE_DRIVER_INIT(spkrform);
	DECLARE_DRIVER_INIT(iqblockf);
	DECLARE_DRIVER_INIT(sdmg2);
	DECLARE_DRIVER_INIT(tarzan);
	DECLARE_DRIVER_INIT(tarzana);
	DECLARE_DRIVER_INIT(lhzb2a);
	DECLARE_DRIVER_INIT(mgdha);

	virtual void video_start();
	virtual void machine_reset();
	DECLARE_MACHINE_RESET(iqblocka);
	DECLARE_MACHINE_RESET(mgcs);
	DECLARE_MACHINE_RESET(lhzb2a);
	UINT32 screen_update_igs017(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(iqblocka_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mgcs_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mgdh_interrupt);


	void decrypt_program_rom(int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0);
	void iqblocka_patch_rom();
	void tjsb_decrypt_sprites();
	void mgcs_decrypt_program_rom();
	void mgcs_decrypt_tiles();
	void mgcs_flip_sprites();
	void mgcs_patch_rom();
	void mgcs_igs029_run();
	void starzan_decrypt(UINT8 *ROM, int size, bool isOpcode);
	void lhzb2_patch_rom();
	void lhzb2_decrypt_tiles();
	void lhzb2_decrypt_sprites();
	void slqz2_patch_rom();
	void slqz2_decrypt_tiles();
	void spkrform_decrypt_sprites();
};

void igs017_state::machine_reset()
{
	m_igs029_send_len = m_igs029_recv_len = 0;
}

/***************************************************************************
                                Video Hardware
***************************************************************************/


void igs017_state::video_start()
{
	m_igs017_igs031->video_start();
}

UINT32 igs017_state::screen_update_igs017(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_igs017_igs031->screen_update_igs017(screen, bitmap, cliprect);
	return 0;
}

// palette bitswap callbacks
UINT16 igs017_state::mgcs_palette_bitswap(UINT16 bgr)
{
	bgr = ((bgr & 0xff00) >> 8) | ((bgr & 0x00ff) << 8);

	return BITSWAP16(bgr, 7, 8, 9, 2, 14, 3, 13, 15, 12, 11, 10, 0, 1, 4, 5, 6);
}

UINT16 igs017_state::lhzb2a_palette_bitswap(UINT16 bgr)
{
//	bgr = ((bgr & 0xff00) >> 8) | ((bgr & 0x00ff) << 8);
	return BITSWAP16(bgr, 15,9,13,12,11,5,4,8,7,6,0,14,3,2,1,10);
}

UINT16 igs017_state::tjsb_palette_bitswap(UINT16 bgr)
{
	// bitswap
	return BITSWAP16(bgr, 15,12,3,6,10,5,4,2,9,13,8,7,11,1,0,14);
}

UINT16 igs017_state::slqz2_palette_bitswap(UINT16 bgr)
{
	return BITSWAP16(bgr, 15,14,9,4,11,10,12,3,7,6,5,8,13,2,1,0);
}




/***************************************************************************
                                Decryption
***************************************************************************/

void igs017_state::decrypt_program_rom(int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0)
{
	int length = memregion("maincpu")->bytes();
	UINT8 *rom = memregion("maincpu")->base();
	UINT8 *tmp = auto_alloc_array(machine(), UINT8, length);
	int i;

	// decrypt the program ROM

	// XOR layer
	for (i = 0;i < length;i++)
	{
		if(i & 0x2000)
		{
			if((i & mask) == mask)
				rom[i] ^= 0x01;
		}
		else
		{
			if(i & 0x0100)
			{
				if((i & mask) == mask)
					rom[i] ^= 0x01;
			}
			else
			{
				if(i & 0x0080)
				{
					if((i & mask) == mask)
						rom[i] ^= 0x01;
				}
				else
				{
					if((i & mask) != mask)
						rom[i] ^= 0x01;
				}
			}
		}
	}

	memcpy(tmp,rom,length);

	// address lines swap
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,a7,a6,a5,a4,a3,a2,a1,a0);
		rom[i] = tmp[addr];
	}

#if 0
	FILE *f = fopen("igs017_decrypted.bin", "wb");
	fwrite(rom, 1, length, f);
	fclose(f);
#endif
}


// iqblocka

void igs017_state::iqblocka_patch_rom()
{
	UINT8 *rom = memregion("maincpu")->base();

//  rom[0x7b64] = 0xc9;

	rom[0x010c7] = 0x18;

	// CBEF bank 0a
	rom[0x16bef] = 0x18;

	// C1BD bank 24
	rom[0x301bd] = 0x18;

	// C21B bank 2e
	rom[0x3a21b] = 0x18;

	// DCA9 bank 2e
	rom[0x3bca9] = 0x18;

	// maybe
//  rom[0x18893] = 0x18;
//  rom[0x385b1] = 0x18;
}

DRIVER_INIT_MEMBER(igs017_state,iqblocka)
{
	decrypt_program_rom(0x11, 7, 6, 5, 4, 3, 2, 1, 0);
	iqblocka_patch_rom();
}

// iqblockf

DRIVER_INIT_MEMBER(igs017_state,iqblockf)
{
	decrypt_program_rom(0x11, 7, 6, 5, 4, 3, 2, 1, 0);
//  iqblockf_patch_rom();
}

// tjsb

void igs017_state::tjsb_decrypt_sprites()
{
	int length = memregion("sprites")->bytes();
	UINT8 *rom = memregion("sprites")->base();
	UINT8 *tmp = auto_alloc_array(machine(), UINT8, length);
	int i, addr;

	// address lines swap
	memcpy(tmp, rom, length);
	for (i = 0; i < length; i++)
	{
		addr = (i & ~0xff) | BITSWAP8(i,7,6,5,2,1,4,3,0);
		rom[i] = tmp[addr];
	}

	// data lines swap
	for (i = 0; i < length; i += 2)
	{
		UINT16 data = (rom[i+1] << 8) | rom[i+0];   // x-22222-11111-00000
		data = BITSWAP16(data, 15, 14,13,12,11,10, 9,1,7,6,5, 4,3,2,8,0);
		rom[i+0] = data;
		rom[i+1] = data >> 8;
	}
}

DRIVER_INIT_MEMBER(igs017_state,tjsb)
{
	decrypt_program_rom(0x05, 7, 6, 3, 2, 5, 4, 1, 0);

	tjsb_decrypt_sprites();
}


// mgcs

void igs017_state::mgcs_decrypt_program_rom()
{
	int i;
	UINT16 *src = (UINT16 *)memregion("maincpu")->base();

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 8 xor layer */

		if( (i & 0x2000/2) || !(i & 0x80/2) )
		{
			if( i & 0x100/2 )
			{
				if( !(i & 0x20/2) || (i & 0x400/2) )
				{
					x ^= 0x0100;
				}
			}
		}
		else
		{
			x ^= 0x0100;
		}

		src[i] = x;
	}
}

void igs017_state::mgcs_decrypt_tiles()
{
	int length = memregion("tilemaps")->bytes();
	UINT8 *rom = memregion("tilemaps")->base();
	dynamic_buffer tmp(length);
	int i;

	memcpy(&tmp[0],rom,length);
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xffff) | BITSWAP16(i,15,14,13,12,11,10,6,7,8,9,5,4,3,2,1,0);
		rom[i^1] = BITSWAP8(tmp[addr],0,1,2,3,4,5,6,7);
	}
}

void igs017_state::mgcs_flip_sprites()
{
	int length = memregion("sprites")->bytes();
	UINT8 *rom = memregion("sprites")->base();
	int i;

	for (i = 0;i < length;i+=2)
	{
		UINT16 pixels = (rom[i+1] << 8) | rom[i+0];

		// flip bits
		pixels = BITSWAP16(pixels,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);

		// flip pixels
		pixels = BITSWAP16(pixels,15, 0,1,2,3,4, 5,6,7,8,9, 10,11,12,13,14);

		rom[i+0] = pixels;
		rom[i+1] = pixels >> 8;
	}
}

void igs017_state::mgcs_patch_rom()
{
//  UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

//  rom[0x20666/2] = 0x601e;    // 020666: 671E    beq $20686 (rom check)

	// IGS029 send command
//  rom[0x4dfce/2] = 0x6010;    // 04DFCE: 6610    bne $4dfe0
//  rom[0x4e00e/2] = 0x4e75;
//  rom[0x4e036/2] = 0x6006;    // 04E036: 6306    bls     $4e03e
}

DRIVER_INIT_MEMBER(igs017_state,mgcs)
{
	mgcs_decrypt_program_rom();
	mgcs_patch_rom();

	mgcs_decrypt_tiles();
	mgcs_flip_sprites();
}


// tarzan, tarzana

// decryption is incomplete, the first part of code doesn't seem right.
DRIVER_INIT_MEMBER(igs017_state,tarzan)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();
	int i;
	int size = 0x40000;

	for(i=0; i<size/2; i++)
	{
		UINT16 x = ROM[i];

		if((i & 0x10c0) == 0x0000)
			x ^= 0x0001;

		if((i & 0x0010) == 0x0010 || (i & 0x0130) == 0x0020)
			x ^= 0x0404;

		if((i & 0x00d0) != 0x0010)
			x ^= 0x1010;

		if(((i & 0x0008) == 0x0008)^((i & 0x10c0) == 0x0000))
			x ^= 0x0100;

		ROM[i] = x;
	}
}
// by iq_132
DRIVER_INIT_MEMBER(igs017_state,tarzana)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;
	int size = 0x80000;

	for (i = 0; i < size; i++)
	{
		UINT8 x = 0;
		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x001a0) != 0x00020) x ^= 0x20;
		if ((i & 0x00260) != 0x00200) x ^= 0x40;
		if ((i & 0x00060) != 0x00000 && (i & 0x00260) != 0x00240)   x ^= 0x80;
		ROM[i] ^= x;
	}
}


// starzan

// decryption is incomplete: data decryption is correct but opcodes are encrypted differently.

void igs017_state::starzan_decrypt(UINT8 *ROM, int size, bool isOpcode)
{
	for(int i=0; i<size; i++)
	{
#if 1
		UINT8 x = ROM[i];

		// this seems ok for opcodes too
		if ( (i & 0x10) && (i & 0x01) )
		{
			if ( !(!(i & 0x2000) && !(i & 0x100) && !(i & 0x80)) )
				x ^= 0x01;
		}
		else
		{
			if ( !(i & 0x2000) && !(i & 0x100) && !(i & 0x80) )
				x ^= 0x01;
		}

		// 2x no xor (opcode)
		// 3x no xor (opcode)
		// 60-66 no xor (opcode)
		if ( !(i & 0x100) || (i & 0x80) || (i & 0x20) )
			x ^= 0x20;

		// 2x needs xor (opcode)
		// 3x needs xor (opcode)
		if ( (i & 0x200) || (i & 0x40) || !(i & 0x20) )
			x ^= 0x40;

		if ( (!(i & 0x100) && (i & 0x80)) || (i & 0x20) )
			x ^= 0x80;

#else
		// by iq_132
		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x000a0) != 0x00000) x ^= 0x20;
		if ((i & 0x001a0) == 0x00000) x ^= 0x20;
		if ((i & 0x00060) != 0x00020) x ^= 0x40;
		if ((i & 0x00260) == 0x00220) x ^= 0x40;
		if ((i & 0x00020) == 0x00020) x ^= 0x80;
		if ((i & 0x001a0) == 0x00080) x ^= 0x80;
#endif
		ROM[i] = x;
	}
}

DRIVER_INIT_MEMBER(igs017_state,starzan)
{
	int size = 0x040000;

	UINT8 *data = memregion("maincpu")->base();
	UINT8 *code = m_decrypted_opcodes;
	memcpy(code, data, size);

	starzan_decrypt(data, size, false); // data
	starzan_decrypt(code, size, true);  // opcodes

	mgcs_flip_sprites();
}


// sdmg2

DRIVER_INIT_MEMBER(igs017_state,sdmg2)
{
	int i;
	UINT16 *src = (UINT16 *)memregion("maincpu")->base();

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 9 xor layer */

		if( i & 0x20000/2 )
		{
			x ^= 0x0200;
		}
		else
		{
			if( !(i & 0x400/2) )
			{
				x ^= 0x0200;
			}
		}

		/* bit 12 xor layer */

		if( i & 0x20000/2 )
		{
			x ^= 0x1000;
		}

		src[i] = x;
	}
}


// mgdh, mgdha

DRIVER_INIT_MEMBER(igs017_state,mgdha)
{
	int i;
	UINT16 *src = (UINT16 *)memregion("maincpu")->base();

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		if( (i & 0x20/2) && (i & 0x02/2) )
		{
			if( (i & 0x300/2) || (i & 0x4000/2) )
				x ^= 0x0001;
		}
		else
		{
			if( !(i & 0x300/2) && !(i & 0x4000/2) )
				x ^= 0x0001;
		}

		if( (i & 0x60000/2) )
			x ^= 0x0100;

		if( (i & 0x1000/2) || ((i & 0x4000/2) && (i & 0x40/2) && (i & 0x80/2)) || ((i & 0x2000/2) && (i & 0x400/2)) )
			x ^= 0x0800;

		src[i] = x;
	}

	mgcs_flip_sprites();
}

DRIVER_INIT_MEMBER(igs017_state,mgdh)
{
	DRIVER_INIT_CALL(mgdha);

	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

	// additional protection
	rom[0x4ad50/2] = 0x4e71;
}


// lhzb2


void igs017_state::lhzb2_patch_rom()
{
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

	// Prot. checks:
	rom[0x14786/2] = 0x6044;    // 014786: 6744    beq $147cc

	// ROM check:
	rom[0x0b48a/2] = 0x604e;    // 00B48A: 674E    beq $b4da
}

void igs017_state::lhzb2_decrypt_tiles()
{
	int length = memregion("tilemaps")->bytes();
	UINT8 *rom = memregion("tilemaps")->base();
	dynamic_buffer tmp(length);
	int i;

	int addr;
	memcpy(&tmp[0], rom, length);
	for (i = 0; i < length; i++)
	{
		addr = (i & ~0xffffff) | BITSWAP24(i,23,22,21,20,19,18,17,1,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0);
		rom[i] = tmp[addr];
	}
}

void igs017_state::lhzb2_decrypt_sprites()
{
	int length = memregion("sprites")->bytes();
	UINT8 *rom = memregion("sprites")->base();
	UINT8 *tmp = auto_alloc_array(machine(), UINT8, length);
	int i, addr;

	// address lines swap
	memcpy(tmp, rom, length);
	for (i = 0; i < length; i++)
	{
		addr = (i & ~0xffff) | BITSWAP16(i,15,14,13,6,7,10,9,8,11,12,5,4,3,2,1,0);
		rom[i] = tmp[addr];
	}

	// data lines swap
	for (i = 0;i < length;i+=2)
	{
		UINT16 data = (rom[i+1] << 8) | rom[i+0];   // x-22222-11111-00000
		data = BITSWAP16(data, 15, 7,6,5,4,3, 2,1,0,14,13, 12,11,10,9,8);
		rom[i+0] = data;
		rom[i+1] = data >> 8;
	}
}

void igs017_state::igs025_to_igs022_callback( void )
{
	m_igs022->IGS022_handle_command();
}



DRIVER_INIT_MEMBER(igs017_state,lhzb2)
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 13 xor layer */

		if( !(i & 0x1000/2) )
		{
			if( i & 0x2000/2 )
			{
				if( i & 0x8000/2 )
				{
					if( !(i & 0x100/2) )
					{
						if( i & 0x200/2 )
						{
							if( !(i & 0x40/2) )
							{
								x ^= 0x2000;
							}
						}
						else
						{
							x ^= 0x2000;
						}
					}
				}
				else
				{
					if( !(i & 0x100/2) )
					{
						x ^= 0x2000;
					}
				}
			}
			else
			{
				if( i & 0x8000/2 )
				{
					if( i & 0x200/2 )
					{
						if( !(i & 0x40/2) )
						{
							x ^= 0x2000;
						}
					}
					else
					{
						x ^= 0x2000;
					}
				}
				else
				{
					x ^= 0x2000;
				}
			}
		}

		src[i] = x;
	}

	lhzb2_decrypt_tiles();
	lhzb2_decrypt_sprites();
	lhzb2_patch_rom();

	// install and configure protection device(s)
//  m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xda5610, 0xda5613, read16_delegate(FUNC(igs025_device::killbld_igs025_prot_r), (igs025_device*)m_igs025), write16_delegate(FUNC(igs025_device::killbld_igs025_prot_w), (igs025_device*)m_igs025));
//  m_igs022->m_sharedprotram = m_sharedprotram;
//  m_igs025->m_kb_source_data = dw3_source_data;
//  m_igs025->m_kb_source_data_offset = 0;
//  m_igs025->m_kb_game_id = 0x00060000;
}


//lhzb2a

DRIVER_INIT_MEMBER(igs017_state,lhzb2a)
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 5 xor layer */

		if( i & 0x4000/2 )
		{
			if( i & 0x8000/2 )
			{
				if( i & 0x2000/2 )
				{
					if( i & 0x200/2 )
					{
						if( !(i & 0x40/2) || (i & 0x800/2) )
						{
							x ^= 0x0020;
						}
					}
				}
			}
			else
			{
				if( !(i & 0x40/2) || (i & 0x800/2) )
				{
					x ^= 0x0020;
				}
			}
		}

		src[i] = x;
	}

	lhzb2_decrypt_tiles();
	lhzb2_decrypt_sprites();
}


//slqz2

void igs017_state::slqz2_patch_rom()
{
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

	// Prot. checks:
	rom[0x1489c/2] = 0x6044;    // 01489C: 6744    beq $148e2

	// ROM check:
	rom[0x0b77a/2] = 0x604e;    // 00B77A: 674E    beq $b7ca
}

void igs017_state::slqz2_decrypt_tiles()
{
	int length = memregion("tilemaps")->bytes();
	UINT8 *rom = memregion("tilemaps")->base();
	dynamic_buffer tmp(length);
	int i;

	memcpy(&tmp[0],rom,length);
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,7,4,5,6,3,2,1,0);
		rom[i] = tmp[addr];
	}
}

DRIVER_INIT_MEMBER(igs017_state,slqz2)
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base());

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 14 xor layer */

		if( i & 0x1000/2 )
		{
			if( i & 0x800/2 )
			{
				x ^= 0x4000;
			}
			else
			{
				if( i & 0x200/2 )
				{
					if( !(i & 0x100/2) )
					{
						if( i & 0x40/2 )
						{
							x ^= 0x4000;
						}
					}
				}
				else
				{
					x ^= 0x4000;
				}
			}
		}
		else
		{
			if( i & 0x800/2 )
			{
				x ^= 0x4000;
			}
			else
			{
				if( !(i & 0x100/2) )
				{
					if( i & 0x40/2 )
					{
						x ^= 0x4000;
					}
				}
			}
		}

		src[i] = x;
	}

	slqz2_decrypt_tiles();
	lhzb2_decrypt_sprites();
	slqz2_patch_rom();

	// install and configure protection device(s)
//  m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xda5610, 0xda5613, read16_delegate(FUNC(igs025_device::killbld_igs025_prot_r), (igs025_device*)m_igs025), write16_delegate(FUNC(igs025_device::killbld_igs025_prot_w), (igs025_device*)m_igs025));
//  m_igs022->m_sharedprotram = m_sharedprotram;
//  m_igs025->m_kb_source_data = dw3_source_data;
//  m_igs025->m_kb_source_data_offset = 0;
//  m_igs025->m_kb_game_id = 0x00060000;
}

// spkrform

void igs017_state::spkrform_decrypt_sprites()
{
	int length = memregion("sprites")->bytes();
	UINT8 *rom = memregion("sprites")->base();
	UINT8 *tmp = auto_alloc_array(machine(), UINT8, length);
	int i, addr;

	// address lines swap
	memcpy(tmp, rom, length);
	for (i = 0; i < length; i++)
	{
		if (i & 0x80000)
			addr = (i & ~0xff) | BITSWAP8(i,7,6,3,4,5,2,1,0);
		else
			addr = (i & ~0xffff) | BITSWAP16(i,15,14,13,12,11,10, 4, 8,7,6,5, 9,3,2,1,0);

		rom[i] = tmp[addr];
	}
}

DRIVER_INIT_MEMBER(igs017_state,spkrform)
{
	decrypt_program_rom(0x14, 7, 6, 5, 4, 3, 0, 1, 2);

	spkrform_decrypt_sprites();
}

/***************************************************************************
                                Memory Maps
***************************************************************************/


// iqblocka


static ADDRESS_MAP_START( iqblocka_map, AS_PROGRAM, 8, igs017_state )
	AM_RANGE( 0x00000, 0x0dfff ) AM_ROM
	AM_RANGE( 0x0e000, 0x0efff ) AM_RAM
	AM_RANGE( 0x0f000, 0x0ffff ) AM_RAM
	AM_RANGE( 0x10000, 0x3ffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, igs017_state )
	AM_RANGE( 0x00000, 0x3ffff ) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END



WRITE8_MEMBER(igs017_state::input_select_w)
{
	m_input_select = data;
}

READ8_MEMBER(igs017_state::input_r)
{
	switch (m_input_select)
	{
		case 0x00:  return ioport("PLAYER1")->read();
		case 0x01:  return ioport("PLAYER2")->read();
		case 0x02:  return ioport("COINS")->read();

		case 0x03:  return 01;

		case 0x20:  return 0x49;
		case 0x21:  return 0x47;
		case 0x22:  return 0x53;

		case 0x24:  return 0x41;
		case 0x25:  return 0x41;
		case 0x26:  return 0x7f;
		case 0x27:  return 0x41;
		case 0x28:  return 0x41;

		case 0x2a:  return 0x3e;
		case 0x2b:  return 0x41;
		case 0x2c:  return 0x49;
		case 0x2d:  return 0xf9;
		case 0x2e:  return 0x0a;

		case 0x30:  return 0x26;
		case 0x31:  return 0x49;
		case 0x32:  return 0x49;
		case 0x33:  return 0x49;
		case 0x34:  return 0x32;

		default:
			logerror("%s: input %02x read\n", machine().describe_context(), m_input_select);
			return 0xff;
	}
}

static ADDRESS_MAP_START( iqblocka_io, AS_IO, 8, igs017_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs

	AM_RANGE( 0x0000, 0x7fff ) AM_DEVREADWRITE("igs017_igs031", igs017_igs031_device, read,write)

	AM_RANGE( 0x8000, 0x8000 ) AM_WRITE(input_select_w )
	AM_RANGE( 0x8001, 0x8001 ) AM_READ(input_r )

	AM_RANGE( 0x9000, 0x9000 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE( 0xa000, 0xa000 ) AM_READ_PORT( "BUTTONS" )

	AM_RANGE( 0xb000, 0xb001 ) AM_DEVWRITE("ymsnd", ym2413_device, write)
ADDRESS_MAP_END


// mgcs

// IGS029 appears to be an MCU that receives commands (write port with value, read port, etc.)
// Sound banking and DSW are accessed through it. It also performs some game specific calculations.
void igs017_state::mgcs_igs029_run()
{
	logerror("%s: running igs029 command ", machine().describe_context());
	for (int i = 0; i < m_igs029_send_len; i++)
		logerror("%02x ", m_igs029_send_buf[i]);

	if (m_igs029_send_buf[0] == 0x05 && m_igs029_send_buf[1] == 0x5a)
	{
		UINT8 data = m_igs029_send_buf[2];
		UINT8 port = m_igs029_send_buf[3];

		logerror("PORT %02x = %02x\n", port, data);

		switch (port)
		{
			case 0x01:
				m_oki->set_bank_base((data & 0x10) ? 0x40000 : 0);
				coin_counter_w(machine(), 0, (~data) & 0x20);   // coin in
				coin_counter_w(machine(), 1, (~data) & 0x40);   // coin out

//              popmessage("PORT1 %02X", data);

				if ( data & ~0x70 )
					logerror("%s: warning, unknown bits written in port %02x = %02x\n", machine().describe_context(), port, data);

				break;

			case 0x03:
				m_dsw_select = data;

//              popmessage("PORT3 %02X", data);

				if ( data & ~0x03 )
					logerror("%s: warning, unknown bits written in port %02x = %02x\n", machine().describe_context(), port, data);

				break;

			default:
				logerror("%s: warning, unknown port %02x written with %02x\n", machine().describe_context(), port, data);
		}

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;
	}
	else if (m_igs029_send_buf[0] == 0x03 && m_igs029_send_buf[1] == 0x55)
	{
		logerror("MIN BET?\n");

		// No inputs. Returns 1 long

		UINT8 min_bets[4] = {1, 2, 3, 5};

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = min_bets[ (~ioport("DSW2")->read()) & 3 ];
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x05;
	}
	else if (m_igs029_send_buf[0] == 0x03 && m_igs029_send_buf[1] == 0x39)
	{
		logerror("READ DSW\n");

		UINT8 ret;
		if      (~m_dsw_select & 0x01)  ret = ioport("DSW1")->read();
		else if (~m_dsw_select & 0x02)  ret = ioport("DSW2")->read();
		else
		{
			logerror("%s: warning, reading dsw with dsw_select = %02x\n", machine().describe_context(), m_dsw_select);
			ret = 0xff;
		}

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = ret;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x02;
	}
	else if (m_igs029_send_buf[0] == 0x07 && m_igs029_send_buf[1] == 0x2c)
	{
		logerror("?? (2C)\n");  // ??

		// 4 inputs. Returns 1 long

		// called when pressing start without betting.
		// Returning high values produces an overflow causing a division by 0, and then the game hangs.
		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;  // ??
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x05;
	}
	else if (m_igs029_send_buf[0] == 0x07 && m_igs029_send_buf[1] == 0x15)
	{
		logerror("SET LONG\n");

		m_igs029_mgcs_long = (m_igs029_send_buf[2] << 24) | (m_igs029_send_buf[3] << 16) | (m_igs029_send_buf[4] << 8) | m_igs029_send_buf[5];

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;
	}
	else if (m_igs029_send_buf[0] == 0x03 && m_igs029_send_buf[1] == 0x04)
	{
		logerror("GET LONG\n");

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = (m_igs029_mgcs_long >>  0) & 0xff;
		m_igs029_recv_buf[m_igs029_recv_len++] = (m_igs029_mgcs_long >>  8) & 0xff;
		m_igs029_recv_buf[m_igs029_recv_len++] = (m_igs029_mgcs_long >> 16) & 0xff;
		m_igs029_recv_buf[m_igs029_recv_len++] = (m_igs029_mgcs_long >> 24) & 0xff;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x05;
	}
	else
	{
		logerror("UNKNOWN\n");

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;
	}

	m_igs029_send_len = 0;
}

WRITE16_MEMBER(igs017_state::mgcs_magic_w)
{
	COMBINE_DATA(&m_igs_magic[offset]);

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
			{
				bool igs029_irq = !(m_input_select & 0x04) &&  (data & 0x04);   // 0->1

				// 7654 3--- Keys
				// ---- -2-- IRQ on IGS029
				// ---- --1-
				// ---- ---0 Hopper Motor
				m_input_select = data & 0xff;

				m_hopperdev->write(space, 0, (data & 0x0001) ? 0x80 : 0x00);

				if (igs029_irq)
				{
					if (!m_igs029_recv_len)
					{
						// SEND
						if (m_igs029_send_len < sizeof(m_igs029_send_buf))
							m_igs029_send_buf[m_igs029_send_len++] = m_igs029_send_data;

						logerror("%s: igs029 send ", machine().describe_context());
						for (int i = 0; i < m_igs029_send_len; i++)
							logerror("%02x ", m_igs029_send_buf[i]);
						logerror("\n");

						if (m_igs029_send_buf[0] == m_igs029_send_len)
							mgcs_igs029_run();
					}

					if (m_igs029_recv_len)
					{
						// RECV
						logerror("%s: igs029 recv ", machine().describe_context());
						for (int i = 0; i < m_igs029_recv_len; i++)
							logerror("%02x ", m_igs029_recv_buf[i]);
						logerror("\n");

						if (m_igs029_recv_len)
							--m_igs029_recv_len;

						m_igs029_recv_data = m_igs029_recv_buf[m_igs029_recv_len];
					}
				}
			}

			if ( m_input_select & ~0xfd )
				logerror("%s: warning, unknown bits written in input_select = %02x\n", machine().describe_context(), m_input_select);

			break;

		case 0x01:
			if (ACCESSING_BITS_0_7)
			{
				m_scramble_data = data & 0xff;
//              logerror("%s: writing %02x to igs_magic = %02x\n", machine().describe_context(), data & 0xff, m_igs_magic[0]);
			}
			break;

		// case 0x02: ?

		case 0x03:
			if (ACCESSING_BITS_0_7)
			{
				m_igs029_send_data = data & 0xff;
//              logerror("%s: writing %02x to igs_magic = %02x\n", machine().describe_context(), data & 0xff, m_igs_magic[0]);
			}
			break;

		default:
			logerror("%s: warning, writing to igs_magic %02x = %02x\n", machine().describe_context(), m_igs_magic[0], data);
	}
}

READ16_MEMBER(igs017_state::mgcs_magic_r)
{
	if (offset == 0)
		return m_igs_magic[0];

	switch(m_igs_magic[0])
	{
		case 0x00:
			return m_input_select | 0x02;

		case 0x01:
		{
			UINT16 ret = BITSWAP8( (BITSWAP8(m_scramble_data, 0,1,2,3,4,5,6,7) + 1) & 3, 4,5,6,7, 0,1,2,3);
			logerror("%s: reading %02x from igs_magic = %02x\n", machine().describe_context(), ret, m_igs_magic[0]);
			return ret;
		}

		case 0x02:
		{
			UINT8 ret = m_igs029_recv_data;
			logerror("%s: reading %02x from igs_magic = %02x\n", machine().describe_context(), ret, m_igs_magic[0]);
			return ret;
		}

//      case 0x05: ???

		default:
			logerror("%s: warning, reading with igs_magic = %02x\n", machine().describe_context(), m_igs_magic[0]);
			break;
	}

	return 0xffff;
}

READ8_MEMBER(igs017_state::mgcs_keys_r)
{
	if (~m_input_select & 0x08) return ioport("KEY0")->read();
	if (~m_input_select & 0x10) return ioport("KEY1")->read();
	if (~m_input_select & 0x20) return ioport("KEY2")->read();
	if (~m_input_select & 0x40) return ioport("KEY3")->read();
	if (~m_input_select & 0x80) return ioport("KEY4")->read();

	logerror("%s: warning, reading key with input_select = %02x\n", machine().describe_context(), m_input_select);
	return 0xff;
}



static ADDRESS_MAP_START( mgcs, AS_PROGRAM, 16, igs017_state )
	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x300000, 0x303fff ) AM_RAM
	AM_RANGE( 0x49c000, 0x49c003 ) AM_WRITE(mgcs_magic_w ) AM_READ(mgcs_magic_r )

	AM_RANGE( 0xa00000, 0xa0ffff ) AM_DEVREADWRITE8("igs017_igs031", igs017_igs031_device, read,write, 0x00ff)

	AM_RANGE( 0xa12000, 0xa12001 ) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	// oki banking through protection (code at $1a350)?
ADDRESS_MAP_END


// sdmg2

READ8_MEMBER(igs017_state::sdmg2_keys_r)
{
	if (~m_input_select & 0x01) return ioport("KEY0")->read();
	if (~m_input_select & 0x02) return ioport("KEY1")->read();
	if (~m_input_select & 0x04) return ioport("KEY2")->read();
	if (~m_input_select & 0x08) return ioport("KEY3")->read();
	if (~m_input_select & 0x10) return ioport("KEY4")->read();

	if (m_input_select == 0x1f) return ioport("KEY0")->read();  // in joystick mode

	logerror("%s: warning, reading key with input_select = %02x\n", machine().describe_context(), m_input_select);
	return 0xff;
}

WRITE16_MEMBER(igs017_state::sdmg2_magic_w)
{
	COMBINE_DATA(&m_igs_magic[offset]);

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		// case 0x00: ? 0x80

		case 0x01:
			if (ACCESSING_BITS_0_7)
			{
				m_input_select  =   data & 0x1f;
				coin_counter_w(machine(), 0,    data & 0x20);
				//  coin out        data & 0x40
				m_hopper            =   data & 0x80;
			}
			break;

		case 0x02:
			if (ACCESSING_BITS_0_7)
			{
				m_oki->set_bank_base((data & 0x80) ? 0x40000 : 0);
			}
			break;

		default:
			logerror("%s: warning, writing to igs_magic %02x = %02x\n", machine().describe_context(), m_igs_magic[0], data);
	}
}

READ16_MEMBER(igs017_state::sdmg2_magic_r)
{
	switch(m_igs_magic[0])
	{
		case 0x00:
		{
			UINT16 hopper_bit = (m_hopper && ((m_screen->frame_number()/10)&1)) ? 0x0000 : 0x0001;
			return ioport("COINS")->read() | hopper_bit;
		}

		case 0x02:
			return sdmg2_keys_r(space, 0);

		default:
			logerror("%s: warning, reading with igs_magic = %02x\n", machine().describe_context(), m_igs_magic[0]);
			break;
	}

	return 0xffff;
}

static ADDRESS_MAP_START( sdmg2, AS_PROGRAM, 16, igs017_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x1f0000, 0x1fffff) AM_RAM

	AM_RANGE(0x200000, 0x20ffff ) AM_DEVREADWRITE8("igs017_igs031", igs017_igs031_device, read,write, 0x00ff)

	AM_RANGE(0x210000, 0x210001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE(0x300000, 0x300003) AM_WRITE(sdmg2_magic_w )
	AM_RANGE(0x300002, 0x300003) AM_READ(sdmg2_magic_r )
ADDRESS_MAP_END


// mgdh, mgdha


READ8_MEMBER(igs017_state::mgdh_keys_r)
{
	if (~m_input_select & 0x04) return ioport("KEY0")->read();
	if (~m_input_select & 0x08) return ioport("KEY1")->read();
	if (~m_input_select & 0x10) return ioport("KEY2")->read();
	if (~m_input_select & 0x20) return ioport("KEY3")->read();
	if (~m_input_select & 0x40) return ioport("KEY4")->read();

	if ((m_input_select & 0xfc) == 0xfc)    return ioport("DSW1")->read();

	logerror("%s: warning, reading key with input_select = %02x\n", machine().describe_context(), m_input_select);
	return 0xff;
}

WRITE16_MEMBER(igs017_state::mgdha_magic_w)
{
	COMBINE_DATA(&m_igs_magic[offset]);

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
			{
				//  coin out     data & 0x40
				coin_counter_w(machine(), 0, data & 0x80);
			}

			if ( data & ~0xc0 )
				logerror("%s: warning, unknown bits written to igs_magic 00 = %02x\n", machine().describe_context(), data);

			break;

		case 0x01:
			if (ACCESSING_BITS_0_7)
			{
				m_input_select = data & 0xff;
				m_hopper = data & 0x01;
			}

			if ( m_input_select & ~0xfd )
				logerror("%s: warning, unknown bits written in input_select = %02x\n", machine().describe_context(), m_input_select);

			break;

		case 0x03:
			if (ACCESSING_BITS_0_7)
			{
				// bit 7?
				m_oki->set_bank_base((data & 0x40) ? 0x40000 : 0);
			}
			break;

		default:
/*
            04aba0: warning, writing to igs_magic 08 = d0
            04abb0: warning, writing to igs_magic 09 = 76
            04abc0: warning, writing to igs_magic 0a = 97
            04abd0: warning, writing to igs_magic 0b = bf
            04abe0: warning, writing to igs_magic 0c = ff
            04abf0: warning, writing to igs_magic 04 = 3f
            04ac00: warning, writing to igs_magic 05 = 82
            04ac10: warning, writing to igs_magic 06 = ff
            04ac20: warning, writing to igs_magic 07 = 3f
*/
			logerror("%s: warning, writing to igs_magic %02x = %02x\n", machine().describe_context(), m_igs_magic[0], data);
	}
}

READ16_MEMBER(igs017_state::mgdha_magic_r)
{
	switch(m_igs_magic[0])
	{
		case 0x00:
			return mgdh_keys_r(space, 0);

		case 0x01:
			return ioport("BUTTONS")->read();

		case 0x02:
			return BITSWAP8(ioport("DSW2")->read(), 0,1,2,3,4,5,6,7);

		case 0x03:
		{
			UINT16 hopper_bit = (m_hopper && ((m_screen->frame_number()/10)&1)) ? 0x0000 : 0x0001;
			return ioport("COINS")->read() | hopper_bit;
		}

		default:
			logerror("%s: warning, reading with igs_magic = %02x\n", machine().describe_context(), m_igs_magic[0]);
			break;
	}

	return 0xffff;
}

static ADDRESS_MAP_START( mgdha_map, AS_PROGRAM, 16, igs017_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x600000, 0x603fff) AM_RAM
	AM_RANGE(0x876000, 0x876003) AM_WRITE(mgdha_magic_w )
	AM_RANGE(0x876002, 0x876003) AM_READ(mgdha_magic_r )

	AM_RANGE(0xa00000, 0xa0ffff ) AM_DEVREADWRITE8("igs017_igs031", igs017_igs031_device, read,write, 0x00ff)

	AM_RANGE(0xa10000, 0xa10001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
ADDRESS_MAP_END


// tjsb


WRITE8_MEMBER(igs017_state::tjsb_output_w)
{
	switch(m_input_select)
	{
		case 0x00:
			coin_counter_w(machine(), 0,    data & 0x80);   // coin in
			if (!(data & ~0x80))
				return;
			break;

		case 0x01:
			coin_counter_w(machine(), 1,    data & 0x01);   // coin out
			if (!(data & ~0x01))
				return;
			break;

		case 0x02:
			m_oki->set_bank_base((data & 0x10) ? 0x40000 : 0);   // oki bank (0x20/0x30)
			if (!(data & ~0x30))
				return;
			break;

		case 0x03:
			m_hopper = data & 0x40;
			if (!(data & ~0x40))
				return;
			break;
	}
	logerror("%s: warning, writing to igs_magic %02x = %02x\n", machine().describe_context(), m_input_select, data);
}

READ8_MEMBER(igs017_state::tjsb_input_r)
{
	switch (m_input_select)
	{
		case 0x00:  return ioport("PLAYER1")->read();
		case 0x01:  return ioport("PLAYER2")->read();
		case 0x02:  return ioport("COINS")->read();
		case 0x03:
		{
			UINT8 hopper_bit = (m_hopper && ((m_screen->frame_number()/10)&1)) ? 0x00 : 0x20;
			return ioport("HOPPER")->read() | hopper_bit;
		}

		default:
			logerror("%s: input %02x read\n", machine().describe_context(), m_input_select);
			return 0xff;
	}
}

static ADDRESS_MAP_START( tjsb_map, AS_PROGRAM, 8, igs017_state )
	AM_RANGE( 0x00000, 0x0dfff ) AM_ROM
	AM_RANGE( 0x0e000, 0x0e000 ) AM_WRITE(input_select_w )
	AM_RANGE( 0x0e001, 0x0e001 ) AM_READWRITE(tjsb_input_r, tjsb_output_w )
	AM_RANGE( 0x0e002, 0x0efff ) AM_RAM
	AM_RANGE( 0x0f000, 0x0ffff ) AM_RAM
	AM_RANGE( 0x10000, 0x3ffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tjsb_io, AS_IO, 8, igs017_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs

	AM_RANGE( 0x0000, 0x7fff ) AM_DEVREADWRITE("igs017_igs031", igs017_igs031_device, read,write)


	AM_RANGE( 0x9000, 0x9000 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE( 0xb000, 0xb001 ) AM_DEVWRITE("ymsnd", ym2413_device, write)
ADDRESS_MAP_END


// spkrform


static ADDRESS_MAP_START( spkrform_map, AS_PROGRAM, 8, igs017_state )
	AM_RANGE( 0x00000, 0x0dfff ) AM_ROM
	AM_RANGE( 0x0e9bf, 0x0e9bf ) AM_NOP // hack: uncomment to switch to Formosa
	AM_RANGE( 0x0e000, 0x0efff ) AM_RAM
	AM_RANGE( 0x0f000, 0x0ffff ) AM_RAM
	AM_RANGE( 0x10000, 0x3ffff ) AM_ROM
ADDRESS_MAP_END

READ8_MEMBER(igs017_state::spkrform_input_r)
{
	switch (m_input_select)
	{
		case 0x00:  return ioport("PLAYER1")->read();
		case 0x01:  return ioport("PLAYER2")->read();
		case 0x02:  return ioport("COINS")->read();
		case 0x03:
		{
			return ioport("BUTTONS")->read();
		}

		default:
			logerror("%s: input %02x read\n", machine().describe_context(), m_input_select);
			return 0xff;
	}
}

static ADDRESS_MAP_START( spkrform_io, AS_IO, 8, igs017_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs

	AM_RANGE( 0x0000, 0x7fff ) AM_DEVREADWRITE("igs017_igs031", igs017_igs031_device, read,write)

	AM_RANGE( 0x8000, 0x8000 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE( 0x9000, 0x9001 ) AM_DEVWRITE("ymsnd", ym2413_device, write)

	AM_RANGE( 0xa000, 0xa000 ) AM_READ_PORT( "A000" )   // Game selection
	AM_RANGE( 0xa001, 0xa001 ) AM_READ_PORT( "A001" )

	AM_RANGE( 0xb000, 0xb000 ) AM_WRITE(input_select_w )
	AM_RANGE( 0xb001, 0xb001 ) AM_READ(spkrform_input_r )
ADDRESS_MAP_END


// lhzb2


WRITE16_MEMBER(igs017_state::lhzb2_magic_w)
{
	COMBINE_DATA(&m_igs_magic[offset]);

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
			{
				m_input_select = data & 0xff;
			}

			if ( m_input_select & ~0x1f )
				logerror("%s: warning, unknown bits written in input_select = %02x\n", machine().describe_context(), m_input_select);
			break;

		case 0x01:
			if (ACCESSING_BITS_0_7)
			{
				m_oki->set_bank_base((data & 0x80) ? 0x40000 : 0);

				if ( data & 0x7f )
					logerror("%s: warning, unknown bits written in oki bank = %04x\n", machine().describe_context(), data);
			}
			break;

		default:
			logerror("%s: warning, writing to igs_magic %02x = %02x\n", machine().describe_context(), m_igs_magic[0], data);
	}
}

READ16_MEMBER(igs017_state::lhzb2_magic_r)
{
	switch(m_igs_magic[0])
	{
		case 0x01:
		{
			if (~m_input_select & 0x01) return ioport("KEY0")->read();
			if (~m_input_select & 0x02) return ioport("KEY1")->read();
			if (~m_input_select & 0x04) return ioport("KEY2")->read();
			if (~m_input_select & 0x08) return ioport("KEY3")->read();
			if (~m_input_select & 0x10) return ioport("KEY4")->read();

			logerror("%s: warning, reading key with input_select = %02x\n", machine().describe_context(), m_input_select);
			return 0xffff;
		}

		default:
			logerror("%s: warning, reading with igs_magic = %02x\n", machine().describe_context(), m_igs_magic[0]);
			break;
	}

	return 0xffff;
}

static ADDRESS_MAP_START( lhzb2, AS_PROGRAM, 16, igs017_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x500000, 0x503fff) AM_RAM
	AM_RANGE(0x910000, 0x910003) AM_WRITE( lhzb2_magic_w )
	AM_RANGE(0x910002, 0x910003) AM_READ( lhzb2_magic_r )

	AM_RANGE( 0xb00000, 0xb0ffff ) AM_DEVREADWRITE8("igs017_igs031", igs017_igs031_device, read,write, 0x00ff)

	AM_RANGE(0xb10000, 0xb10001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
ADDRESS_MAP_END


// lhzb2a
// To do: what devices are on this PCB?

/***************************************************************************

    LHZB2A Protection (similar to that found in igs011.c)

    ---- Protection 1 (parametric bitswaps) ----

    An address base register (xx = F0 at reset) determines where this protection device,
    as well as game inputs and the address base register itself are mapped in memory:
    inputs are mapped at xx8000, protection at xx4000 and base register at xxc000.

    The protection involves an internal 16-bit value (val), two mode registers
    (mode_f = 0..f, mode_3 = 0..3) and 8 x 16-bit registers (word).

    The two modes affect the bitswap, and are set by loading the (same) mode-specific value
    to all the word registers, and then writing to the mode_f or mode_3 trigger register.

    The bitswap of the internal value is then performed writing to one of 8 trigger registers,
    according to the modes, trigger register and value written.

    The result is read through a fixed bitswap of the internal value.

    ---- Protection 2 (simple inc,dec + bitswapped read) ----

    The chip holds an internal 8-bit value. It is manipulated by issuing commands,
    where each command is assigned a specific address range, and is triggered
    by writing FF to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - RESET: value = 0

    The protection value is read from an additional address range:

    - READ:  read bitswap(value). Only 4 bits are checked.

***************************************************************************/

// Bitswap protection

WRITE16_MEMBER(igs017_state::lhzb2a_prot_w)
{
	COMBINE_DATA(&m_prot_regs[offset]);

	if (offset == 0)
		return;

	switch(m_prot_regs[0])
	{
		case 0x40:  // prot_word
		{
			m_prot_word = (m_prot_word << 8) | (m_prot_regs[1] & 0xff);
			break;
		}
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			// same value as reg 0x40
			break;

		case 0x48:  // mode_f
		{
			switch (m_prot_word)
			{
				case 0x9a96:    m_prot_mf = 0x0;    break;
				case 0x9a06:    m_prot_mf = 0x1;    break;
				case 0x9a90:    m_prot_mf = 0x2;    break;
				case 0x9a00:    m_prot_mf = 0x3;    break;
				case 0x0a96:    m_prot_mf = 0x4;    break;
				case 0x0a06:    m_prot_mf = 0x5;    break;
				case 0x0a90:    m_prot_mf = 0x6;    break;
				case 0x0a00:    m_prot_mf = 0x7;    break;
				case 0x9096:    m_prot_mf = 0x8;    break;
				case 0x9006:    m_prot_mf = 0x9;    break;
				case 0x9090:    m_prot_mf = 0xa;    break;
				case 0x9000:    m_prot_mf = 0xb;    break;
				case 0x0096:    m_prot_mf = 0xc;    break;
				case 0x0006:    m_prot_mf = 0xd;    break;
				case 0x0090:    m_prot_mf = 0xe;    break;
				case 0x0000:    m_prot_mf = 0xf;    break;
				default:
					m_prot_mf = 0;
					logerror("%s: warning, setting mode_f with unknown prot_word = %02x\n", machine().describe_context(), m_prot_word);
					return;
			}

			logerror("%s: mode_f = %02x\n", machine().describe_context(), m_prot_mf);
			break;
		}

		case 0x50:  // mode_3
		{
			switch (m_prot_word & 0xff)
			{
				case 0x53:  m_prot_m3 = 0x0;    break;
				case 0x03:  m_prot_m3 = 0x1;    break;
				case 0x50:  m_prot_m3 = 0x2;    break;
				case 0x00:  m_prot_m3 = 0x3;    break;
				default:
					m_prot_m3 = 0;
					logerror("%s: warning, setting mode_3 with unknown prot_word = %02x\n", machine().describe_context(), m_prot_word);
					return;
			}

			logerror("%s: mode_3 = %02x\n", machine().describe_context(), m_prot_m3);
			break;
		}

		case 0x80:  // do bitswap
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		{
			UINT16 x  = m_prot_val;
			UINT16 mf = m_prot_mf;

			UINT16 bit0 = 0;
			switch (m_prot_m3)
			{
				case 0: bit0 = BIT(~x,12) ^ BIT(~x,15) ^ BIT( x, 8) ^ BIT(~x, 3);   break;
				case 1: bit0 = BIT(~x, 6) ^ BIT(~x,15) ^ BIT(~x, 3) ^ BIT(~x, 9);   break;
				case 2: bit0 = BIT(~x, 3) ^ BIT(~x,15) ^ BIT(~x, 5) ^ BIT( x, 4);   break;
				case 3: bit0 = BIT(~x,15) ^ BIT(~x, 9) ^ BIT( x,12) ^ BIT(~x,11);   break;
			}

			UINT16 xor0 = BIT(m_prot_regs[1], m_prot_regs[0] - 0x80);
			bit0 ^= xor0;

			m_prot_val  =   (    BIT( x,14)                 << 15   ) |
							(   (BIT(~x,13) ^ BIT(mf,3))    << 14   ) |
							(    BIT( x,12)                 << 13   ) |
							(    BIT(~x,11)                 << 12   ) |
							(   (BIT( x,10) ^ BIT(mf,2))    << 11   ) |
							(    BIT( x, 9)                 << 10   ) |
							(    BIT( x, 8)                 <<  9   ) |
							(   (BIT(~x, 7) ^ BIT(mf,1))    <<  8   ) |
							(    BIT( x, 6)                 <<  7   ) |
							(    BIT( x, 5)                 <<  6   ) |
							(   (BIT(~x, 4) ^ BIT(mf,0))    <<  5   ) |
							(    BIT(~x, 3)                 <<  4   ) |
							(    BIT( x, 2)                 <<  3   ) |
							(    BIT(~x, 1)                 <<  2   ) |
							(    BIT( x, 0)                 <<  1   ) |
							(    bit0                       <<  0   ) ;

			logerror("%s: exec bitswap - mode_3 %02x, mode_f %02x, xor0 %x, val %04x -> %04x\n", machine().describe_context(), m_prot_m3, m_prot_mf, xor0, x, m_prot_val);

			break;
		}

		case 0xa0:  // reset
		{
			m_prot_val = 0;
			break;
		}

		default:
			logerror("%s: warning, writing to prot_reg %02x = %02x\n", machine().describe_context(), m_prot_regs[0], m_prot_regs[1]);
	}
}

READ16_MEMBER(igs017_state::lhzb2a_prot_r)
{
	switch(m_prot_regs[0])
	{
		case 0x03:  // result
		{
			UINT16 x = m_prot_val;
			UINT16 res  =   (BIT(x, 5) << 7) |
							(BIT(x, 2) << 6) |
							(BIT(x, 9) << 5) |
							(BIT(x, 7) << 4) |
							(BIT(x,10) << 3) |
							(BIT(x,13) << 2) |
							(BIT(x,12) << 1) |
							(BIT(x,15) << 0) ;

			logerror("%s: read bitswap - val %04x -> %02x\n", machine().describe_context(), m_prot_val, res);
			return res;
		}

		default:
			logerror("%s: warning, reading with prot_reg = %02x\n", machine().describe_context(), m_prot_regs[0]);
			break;
	}

	return 0xffff;
}

// Protection 2

WRITE16_MEMBER(igs017_state::lhzb2a_prot2_reset_w)
{
	m_prot2 = 0x00;
	logerror("%s: prot2 reset -> %02x\n", machine().describe_context(), m_prot2);
}

WRITE16_MEMBER(igs017_state::lhzb2a_prot2_inc_w)
{
	m_prot2++;
	logerror("%s: prot2 inc -> %02x\n", machine().describe_context(), m_prot2);
}

WRITE16_MEMBER(igs017_state::lhzb2a_prot2_dec_w)
{
	m_prot2--;
	logerror("%s: prot2 dec -> %02x\n", machine().describe_context(), m_prot2);
}

READ16_MEMBER(igs017_state::lhzb2a_prot2_r)
{
	UINT8 x     =   m_prot2;
	UINT8 res   =   (BIT(x, 0) << 7) |
					(BIT(x, 3) << 5) |
					(BIT(x, 2) << 4) |
					(BIT(x, 1) << 2) ;

	logerror("%s: prot2 read, %02x -> %02x\n", machine().describe_context(), m_prot2, res);
	return res;
}



READ16_MEMBER(igs017_state::lhzb2a_input_r)
{
	switch (offset*2)
	{
		case 0x00:  // Keys
		{
			if (~m_input_select & 0x01) return ioport("KEY0")->read() << 8;
			if (~m_input_select & 0x02) return ioport("KEY1")->read() << 8;
			if (~m_input_select & 0x04) return ioport("KEY2")->read() << 8;
			if (~m_input_select & 0x08) return ioport("KEY3")->read() << 8;
			if (~m_input_select & 0x10) return ioport("KEY4")->read() << 8;

			logerror("%s: warning, reading key with input_select = %02x\n", machine().describe_context(), m_input_select);
			return 0xffff;
		}

		case 0x02:
		{
			UINT16 hopper_bit = (m_hopper && ((m_screen->frame_number()/10)&1)) ? 0x0000 : 0x0002;
			return (ioport("DSW1")->read() << 8) | ioport("COINS")->read() | hopper_bit;
		}

		case 0x04:
			return ioport("DSW2")->read();
	}

	return 0xffff;
}

WRITE16_MEMBER(igs017_state::lhzb2a_input_addr_w)
{
	// Unmap previous address ranges
	if (m_input_addr != -1)
	{
		space.unmap_readwrite(m_input_addr * 0x10000 + 0x4000, m_input_addr * 0x10000 + 0x4003);
		space.unmap_read     (m_input_addr * 0x10000 + 0x8000, m_input_addr * 0x10000 + 0x8005);
		space.unmap_write    (m_input_addr * 0x10000 + 0xc000, m_input_addr * 0x10000 + 0xc001);
	}

	m_input_addr = data & 0xff;

	// Add new memory ranges
	space.install_readwrite_handler (m_input_addr * 0x10000 + 0x4000, m_input_addr * 0x10000 + 0x4003, read16_delegate (FUNC(igs017_state::lhzb2a_prot_r),      this), write16_delegate (FUNC(igs017_state::lhzb2a_prot_w),      this));
	space.install_read_handler      (m_input_addr * 0x10000 + 0x8000, m_input_addr * 0x10000 + 0x8005, read16_delegate (FUNC(igs017_state::lhzb2a_input_r),      this));
	space.install_write_handler     (m_input_addr * 0x10000 + 0xc000, m_input_addr * 0x10000 + 0xc001, write16_delegate(FUNC(igs017_state::lhzb2a_input_addr_w), this));

	logerror("%s: inputs and protection remapped at %02xxxxx\n", machine().describe_context(), m_input_addr);
}

WRITE16_MEMBER(igs017_state::lhzb2a_input_select_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_input_select      =           data & 0x1f;    // keys
		m_hopper            =           data & 0x20;    // hopper motor
		coin_counter_w(machine(), 1,    data & 0x40);   // coin out counter
		coin_counter_w(machine(), 0,    data & 0x80);   // coin in  counter
	}
	if (ACCESSING_BITS_8_15)
	{
		m_oki->set_bank_base((data & 0x0100) ? 0x40000 : 0);

		if ( data & 0x0fe00 )
			logerror("%s: warning, unknown bits written in input_select = %04x\n", machine().describe_context(), data);
	}
}

static ADDRESS_MAP_START( lhzb2a, AS_PROGRAM, 16, igs017_state )
	// prot2
	AM_RANGE(0x003200, 0x003201) AM_WRITE( lhzb2a_prot2_reset_w )
	AM_RANGE(0x003202, 0x003203) AM_WRITE( lhzb2a_prot2_dec_w )
	AM_RANGE(0x003206, 0x003207) AM_WRITE( lhzb2a_prot2_inc_w )
	AM_RANGE(0x00320a, 0x00320b) AM_READ( lhzb2a_prot2_r )

	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x500000, 0x503fff) AM_RAM
//  AM_RANGE(0x910000, 0x910003) accesses appear to be from leftover code where the final checks were disabled

	AM_RANGE( 0xb00000, 0xb0ffff ) AM_DEVREADWRITE8("igs017_igs031", igs017_igs031_device, read,write, 0x00ff)

	AM_RANGE(0xb10000, 0xb10001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE(0xb12000, 0xb12001) AM_WRITE( lhzb2a_input_select_w )
//  Inputs dynamically mapped at xx8000, protection at xx4000 (xx = f0 initially). xx written to xxc000
ADDRESS_MAP_END


// slqz2


WRITE16_MEMBER(igs017_state::slqz2_magic_w)
{
	COMBINE_DATA(&m_igs_magic[offset]);

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
			{
				m_oki->set_bank_base((data & 0x01) ? 0x40000 : 0);

//              m_hopper            =           data & 0x20;    // hopper motor
//              coin_counter_w(machine(), 1,    data & 0x40);   // coin out counter
				coin_counter_w(machine(), 0,    data & 0x80);   // coin in  counter

				if ( data & 0x7e )
					logerror("%s: warning, unknown bits written in oki bank = %04x\n", machine().describe_context(), data);
			}
			break;

		default:
			logerror("%s: warning, writing to igs_magic %02x = %02x\n", machine().describe_context(), m_igs_magic[0], data);
	}
}

READ16_MEMBER(igs017_state::slqz2_magic_r)
{
	switch(m_igs_magic[0])
	{
		case 0x00:
			return ioport("PLAYER2")->read();
		case 0x01:
			return ioport("PLAYER1")->read();
		case 0x02:
			return ioport("BUTTONS")->read();

		default:
			logerror("%s: warning, reading with igs_magic = %02x\n", machine().describe_context(), m_igs_magic[0]);
			break;
	}

	return 0xffff;
}

static ADDRESS_MAP_START( slqz2, AS_PROGRAM, 16, igs017_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x602000, 0x602003) AM_WRITE( slqz2_magic_w )
	AM_RANGE(0x602002, 0x602003) AM_READ( slqz2_magic_r )

	AM_RANGE(0x900000, 0x90ffff ) AM_DEVREADWRITE8("igs017_igs031", igs017_igs031_device, read,write, 0x00ff)


	AM_RANGE(0x910000, 0x910001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( iqblocka )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hold Mode" )
	PORT_DIPSETTING(    0x02, "In Win" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x04, 0x04, "Max Credit" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x38, "Cigarette Bet" )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x28, "20" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x08, "120" )
	PORT_DIPSETTING(    0x00, "150" )
	PORT_DIPNAME( 0xc0, 0xc0, "Min Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )
	PORT_DIPSETTING(    0x07, "10" )
	PORT_DIPSETTING(    0x06, "20" )
	PORT_DIPSETTING(    0x05, "40" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "250" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x08, 0x08, "Key Out" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" )
	PORT_DIPSETTING(    0x10, "Gaming" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Game" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bonus Base" )
	PORT_DIPSETTING(    0xc0, "100" )
	PORT_DIPSETTING(    0x80, "200" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x00, "400" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Win Up Pool" )
	PORT_DIPSETTING(    0x03, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "800" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Double Up" )
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x10, 0x10, "Cards" )
	PORT_DIPSETTING(    0x10, "A,J,Q,K" )
	PORT_DIPSETTING(    0x00, "Number" )
	PORT_DIPNAME( 0x20, 0x20, "Title Name" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Double" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "CG Select" )
	PORT_DIPSETTING(    0x80, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME( "Start / Test" ) // keep pressed while booting
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_NAME( "Help / Big" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2         ) PORT_NAME( "Bet" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1     ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2     ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW  )  // keep pressed while booting
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )  // this seems to toggle between videogame and gambling

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Small" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3   ) PORT_IMPULSE(2)    // coin error
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME( "Score -> Time" )    // converts score into time
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( lhzb2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" )
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, "Symbols" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // pigs, apples
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL   )  // hopper switch (unimplemented)
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW ) // service mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1  ) PORT_NAME("Statistics") // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1     ) PORT_IMPULSE(5) // coin error otherwise
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE3  ) // ? (shown in service mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( lhzb2a )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" )
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, "Symbols" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // pigs, apples
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL   ) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x04,   IP_ACTIVE_LOW ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1  ) PORT_NAME("Statistics") // press with the above for sound test
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN1     ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE3  ) // shown in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( mgcs )

	// DSWs are read through a protection device (IGS029). See code at 1CF16

	PORT_START("DSW1")  // $3009e2
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Limit" )
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )

	PORT_START("DSW2")  // $3009e3
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "Double Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x10, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" )
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Tile" )
	PORT_DIPNAME( 0x40, 0x40, "Double Up Type" )
	PORT_DIPSETTING(    0x40, "Double" )
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPNAME( 0x80, 0x80, "Bet Number" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// the top 2 bits of COINS (port A) and KEYx (port B) are read and combined with the bottom 4 bits read from port C (see code at 1C83A)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW ) // service mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME("Statistics")  // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1    ) PORT_IMPULSE(5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE3 ) // ? must be high to display numbers (shown in service mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( sdmg2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x00, "29999" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x40, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x80, 0x80, "Number Type" )
	PORT_DIPSETTING(    0x80, "Number" )
	PORT_DIPSETTING(    0x00, "Tile" )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL   ) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE2  ) // shown in test mode
	PORT_SERVICE_NO_TOGGLE( 0x04,   IP_ACTIVE_LOW ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1  ) PORT_NAME("Statistics")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN1     )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)

	// Keyboard mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE3  ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40) // shown in test mode
	// Joystick mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON3   ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)

	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_START("KEY0")
	// Keyboard mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A         ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E         ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I         ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M         ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN       ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1            ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	// Joystick mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1            ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP       ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN     ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT     ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT    ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1           ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2           ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( mgdh )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Credits Per Note" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Max Note Credits" )
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x10, 0x10, "Money Type" )
	PORT_DIPSETTING(    0x10, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x20, 0x20, "Pay Out Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0xc0, 0xc0, "Min Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")  // bitswapped
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x02, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "5" )
	PORT_DIPSETTING(    0x80, "6" )
	PORT_DIPSETTING(    0x60, "7" )
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x20, "9" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL   ) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW ) // service mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1  ) PORT_NAME("Statistics") // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1     ) PORT_IMPULSE(5) // coin error otherwise
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE3  ) // ? (shown in service mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_START("KEY0")
	// Keyboard mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A         ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E         ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I         ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M         ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN       ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1            ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	// Joystick mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1            ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP       ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN     ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT     ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT    ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1           ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN           ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

INPUT_PORTS_END

static INPUT_PORTS_START( slqz2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" )
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, "Symbols" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // pigs, apples
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SPECIAL   ) // hopper switch (unimplemented)
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW ) // service mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1  ) PORT_NAME("Statistics") // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1     ) PORT_IMPULSE(5) // coin error otherwise
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER     ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2   )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3   )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("Start / Don Den")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_NAME( "Help / Big" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2         ) PORT_NAME( "Bet" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_8_PAD)

INPUT_PORTS_END

static INPUT_PORTS_START( tjsb )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )  // 2/4
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "4000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" )   // show bonus round in demo mode -> protection check
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" )
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  // check: (val^ff) & 9a == 0a
	PORT_DIPNAME( 0xff, 0xf5, "Bonus Round Protection Check" )
	PORT_DIPSETTING(    0xf5, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xff, DEF_STR( On ) )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("Start / Don Den")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) // choose
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) // bet
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE1  ) PORT_NAME("Statistics")
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW ) // keep pressed while booting
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_COIN1     )

	PORT_START("HOPPER")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? shown in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_SPECIAL ) // hopper switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

// to do:
static INPUT_PORTS_START( spkrform )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Win Up Pool" )
	PORT_DIPSETTING(    0x03, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "800" )
	PORT_DIPSETTING(    0x00, "800" )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) // ?? exit poker
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) // start (formosa)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) // up
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) // down / start
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)  // right / bet
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)  // button1 / hold1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)  // hold2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1    )  // left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2    )  // hold3
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3    )  // credit in
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4    )  // credit out
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1     )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2     )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4     )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )   // coin (coin error)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )   // coin (coin error)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )   // hopper error
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)  // record
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)

	PORT_START("A000")
	PORT_DIPNAME( 0xff, 0xff, "A000" )
	PORT_DIPSETTING(    0xff, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("A001")
	PORT_DIPNAME( 0xff, 0xff, "A001" )
	PORT_DIPSETTING(    0xff, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END



/***************************************************************************
                                Machine Drivers
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(igs017_state::iqblocka_interrupt)
{
	int scanline = param;

	if(scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(0, HOLD_LINE);

	if(scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


MACHINE_RESET_MEMBER(igs017_state,iqblocka)
{
	m_input_select = 0;
}

static MACHINE_CONFIG_START( iqblocka, igs017_state )
	MCFG_CPU_ADD("maincpu", Z180, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(iqblocka_map)
	MCFG_CPU_IO_MAP(iqblocka_io)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, iqblocka_interrupt, "screen", 0, 1)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW3"))

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,iqblocka)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz / 16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( starzan, iqblocka )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END


// mgcs

TIMER_DEVICE_CALLBACK_MEMBER(igs017_state::mgcs_interrupt)
{
	int scanline = param;

	if(scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->set_input_line(2, HOLD_LINE);
}

MACHINE_RESET_MEMBER(igs017_state,mgcs)
{
	MACHINE_RESET_CALL_MEMBER( iqblocka );

	m_scramble_data = 0;
	memset(m_igs_magic, 0, sizeof(m_igs_magic));
}

static MACHINE_CONFIG_START( mgcs, igs017_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22MHz / 2)
	MCFG_CPU_PROGRAM_MAP(mgcs)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, mgcs_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,mgcs)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("COINS"))
	MCFG_I8255_IN_PORTB_CB(READ8(igs017_state, mgcs_keys_r))

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(50), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_PALETTE_SCRAMBLE_CB( igs017_state, mgcs_palette_bitswap )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_8MHz / 8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



// lhzb2

static MACHINE_CONFIG_START( lhzb2, igs017_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22MHz / 2)
	MCFG_CPU_PROGRAM_MAP(lhzb2)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, mgcs_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,mgcs)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("COINS"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")


	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_PALETTE_SCRAMBLE_CB( igs017_state, lhzb2a_palette_bitswap )

	// protection
	MCFG_DEVICE_ADD("igs025", IGS025, 0)
	MCFG_IGS025_SET_EXTERNAL_EXECUTE( igs017_state, igs025_to_igs022_callback )

	MCFG_DEVICE_ADD("igs022", IGS022, 0)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_8MHz / 8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



// lhzb2a

MACHINE_RESET_MEMBER(igs017_state,lhzb2a)
{
	MACHINE_RESET_CALL_MEMBER( mgcs );
	lhzb2a_input_addr_w(m_maincpu->space(AS_PROGRAM), 0, 0xf0);
}

static MACHINE_CONFIG_START( lhzb2a, igs017_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22MHz/2)
	MCFG_CPU_PROGRAM_MAP(lhzb2a)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, mgcs_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,lhzb2a)

//  MCFG_I8255A_ADD( "ppi8255", sdmg2_ppi8255_intf )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)    // VSync 60Hz, HSync 15.3kHz
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_PALETTE_SCRAMBLE_CB( igs017_state, lhzb2a_palette_bitswap )


	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_22MHz / 22, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



// slqz2

static MACHINE_CONFIG_START( slqz2, igs017_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22MHz / 2)
	MCFG_CPU_PROGRAM_MAP(slqz2)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, mgcs_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,mgcs)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("COINS"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_PALETTE_SCRAMBLE_CB( igs017_state, slqz2_palette_bitswap )

	// protection
	MCFG_DEVICE_ADD("igs025", IGS025, 0)
	MCFG_IGS025_SET_EXTERNAL_EXECUTE( igs017_state, igs025_to_igs022_callback )

	MCFG_DEVICE_ADD("igs022", IGS022, 0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_8MHz / 8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



// sdmg2

static MACHINE_CONFIG_START( sdmg2, igs017_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22MHz/2)
	MCFG_CPU_PROGRAM_MAP(sdmg2)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, mgcs_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,mgcs)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)    // VSync 60Hz, HSync 15.3kHz
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")


	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_22MHz / 22, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


// mgdh

TIMER_DEVICE_CALLBACK_MEMBER(igs017_state::mgdh_interrupt)
{
	int scanline = param;

	if(scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->set_input_line(3, HOLD_LINE); // lev 3 instead of 2
}

static MACHINE_CONFIG_START( mgdha, igs017_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_22MHz / 2)
	MCFG_CPU_PROGRAM_MAP(mgdha_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, mgdh_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,mgcs)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")


	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_22MHz / 22, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


// tjsb

static MACHINE_CONFIG_START( tjsb, igs017_state )
	MCFG_CPU_ADD("maincpu", Z180, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(tjsb_map)
	MCFG_CPU_IO_MAP(tjsb_io)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, iqblocka_interrupt, "screen", 0, 1)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW3"))

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,iqblocka)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_PALETTE_SCRAMBLE_CB( igs017_state, tjsb_palette_bitswap )


	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz / 16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


// spkrform

static MACHINE_CONFIG_START( spkrform, igs017_state )
	MCFG_CPU_ADD("maincpu", Z180, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(spkrform_map)
	MCFG_CPU_IO_MAP(spkrform_io)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", igs017_state, iqblocka_interrupt, "screen", 0, 1)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW3"))

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state,iqblocka)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs017_state, screen_update_igs017)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100*2)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("igs017_igs031", IGS017_IGS031, 0)
	MCFG_GFX_PALETTE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz / 16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

IQ Block (alt hardware)
IGS, 1996

PCB Layout
----------

IGS PCB N0- 0131-4
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                               AR17961 |
|   HD64180RP8                          |
|  16MHz                         BATTERY|
|                                       |
|                         SPEECH.U17    |
|                                       |
|J                        6264          |
|A                                      |
|M      8255              V.U18         |
|M                                      |
|A                                      |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |        |
|       CG.U7          |IGS017 |        |
|                      |       |        |
|       TEXT.U8        |-------|   PAL  |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.31kHz

***************************************************************************/

ROM_START( iqblocka )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v.u18", 0x00000, 0x40000, CRC(2e2b7d43) SHA1(cc73f4c8f9a6e2219ee04c9910725558a80b4eb2) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )   // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0)
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) )
ROM_END

ROM_START( iqblockf )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v113fr.u18", 0x00000, 0x40000, CRC(346c68af) SHA1(ceae4c0143c288dc9c1dd1e8a51f1e3371ffa439) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )   // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sp.u17", 0x00000, 0x40000, CRC(71357845) SHA1(25f4f7aebdcc0706018f041d3696322df569b0a3) )
ROM_END

/***************************************************************************

Mahjong Tian Jiang Shen Bing
IGS, 1997

This PCB is almost the same as IQBlock (IGS, 1996)
but the 8255 has been replaced with the IGS025 IC

PCB Layout
----------

IGS PCB N0- 0157-2
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                         AR17961       |
|   HD64180RP8                   SPDT_SW|
|  16MHz                         BATTERY|
|                                       |
|                         S0703.U15     |
|                                       |
|J     |-------|          6264          |
|A     |       |                        |
|M     |IGS025 |          P0700.U16     |
|M     |       |                        |
|A     |-------|                        |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |   PAL  |
|       A0701.U3       |IGS017 |        |
|                      |       |   PAL  |
|       TEXT.U6        |-------|        |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      IGS025  - Custom IGS IC (PLCC68)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.30kHz

***************************************************************************/

ROM_START( tjsb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "p0700.u16", 0x00000, 0x40000,CRC(1b2a50df) SHA1(95a272e624f727df9523667864f933118d9e633c) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "a0701.u3", 0x00000, 0x400000, CRC(27502a0a) SHA1(cca79e253697f47b688ef781b1b6de9d2945f199) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x00000, 0x80000,  CRC(3be886b8) SHA1(15b3624ed076640c1828d065b01306a8656f5a9b) )  // BADADDR --xxxxxxxxxxxxxxxxx

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0703.u15", 0x00000, 0x80000,  CRC(c6f94d29) SHA1(ec413580240711fc4977dd3c96c288501aa7ef6c) )
ROM_END

/***************************************************************************

Mahjong Man Guan Cai Shen
IGS, 1998


PCB Layout
----------

IGS PCB NO-0192-1
|---------------------------------------|
|              JAMMA            UPC1242 |
|                                       |
|               S1502.U10               |
|                          K668    VOL  |
|                                       |
|                                       |
|                       22MHz           |
|1     61256                            |
|8              |-------|      TEXT.U25 |
|W     PAL      |       |               |
|A              |IGS017 |               |
|Y              |       |      M1501.U23|
|               |-------|               |
|   |-------|                           |
|   |       |                           |
|   |IGS025 |   P1500.U8                |
|   |       |              PAL    6264  |
|1  |-------|                           |
|0  |----|                 PAL    6264  |
|W  |IGS |                 PAL          |
|A  |029 |  8MHz                 SPDT_SW|
|Y  |----|                 68000        |
|T DSW1  DSW2                   BATTERY |
|---------------------------------------|
Notes:
      Uses JAMMA & common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( mgcs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1500.u8", 0x00000, 0x80000, CRC(a8cb5905) SHA1(37be7d926a1352869632d43943763accd4dec4b7) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "m1501.u23", 0x000000, 0x400000, CRC(96fce058) SHA1(6b87f47d646bad9b3061bdc8a9af65467fdbbc9f) )   // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u25", 0x00000, 0x80000, CRC(a37f9613) SHA1(812f060ca98a34540c48a180c359c3d0f1c0b5bb) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1502.u10", 0x00000, 0x80000, CRC(a8a6ba58) SHA1(59276a8ab4a31812600816c2a43b74bd71394419) )
ROM_END

/***************************************************************************

Mahjong Super Da Man Guan 2
IGS, 1997


PCB Layout
----------

IGS PCB NO-0147-6
|---------------------------------------|
| UPC1242H          S0903.U15   BATTERY |
|          VOL               SPDT_SW    |
|                                       |
|        K668                    6264   |
|                                       |
|                                6264   |
|                   PAL                 |
|1   8255                               |
|8                            P0900.U25 |
|W                                      |
|A                                      |
|Y                                      |
|                                68000  |
|                                       |
|    M0902.U4       PAL                 |
|                                       |
|                                 PAL   |
|1   M0901.U5       |-------|           |
|0                  |       |     PAL   |
|W                  |IGS031 |           |
|A   TEXT.U6        |       |           |
|Y                  |-------|     62256 |
|T         22MHz  DSW1 DSW2             |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668 = M6295. clock 1.000MHz [22/22]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( sdmg2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p0900.u25", 0x00000, 0x80000,CRC(43366f51) SHA1(48dd965dceff7de15b43c2140226a8b17a792dbc) )

	ROM_REGION( 0x280000, "sprites", 0 )
	ROM_LOAD( "m0901.u5", 0x000000, 0x200000, CRC(9699db24) SHA1(50fc2f173c20b48d10595f01f1e9545f1b13a61b) )    // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_LOAD( "m0902.u4", 0x200000, 0x080000, CRC(3298b13b) SHA1(13b21ddeed368b7f4fea1408c8fc511244342faf) )    // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x20000, "tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x000000, 0x020000, CRC(cb34cbc0) SHA1(ceedbdda085fd1acc9a575502bdf7cf998f54f05) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0903.u15", 0x00000, 0x80000, CRC(ae5a441c) SHA1(923774ef73ab0f70e0db1738a4292dcbd70d2384) )
ROM_END

/***************************************************************************

Mahjong Long Hu Zheng Ba 2
IGS, 1998

PCB Layout
----------

IGS PCB NO-0206
|---------------------------------------|
|    6264             |-------|         |
|    6264      |----| |       |         |
|              |IGS | |IGS025 |         |
|              |022 | |       |  PAL    |
|              |----| |-------|         |
|                       PAL             |
|    M1104.U11          PAL    68000    |
|1                                      |
|8                                      |
|W                                      |
|A   M1101.U6  8MHz          P1100.U30  |
|Y                                      |
|                                  6264 |
|                                       |
|              |-------|                |
|              |       |                |
|              |IGS031 |           61256|
|1             |       |                |
|0   M1103.U8  |-------|                |
|W      22MHz                           |
|A             DSW1   DSW2              |
|Y            K668     BATTERY          |
|TDA1020 VOL          S1102.U23  SPDT_SW|
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( lhzb2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1100.u30", 0x00000, 0x80000, CRC(68102b25) SHA1(6c1e8d204be0efda0e9b6c2f49b5c6760712475f) )

	ROM_REGION( 0x10000, "igs022data", 0 )  // INTERNATIONAL GAMES SYSTEM CO.,LTD
	ROM_LOAD( "m1104.u11",0x0000, 0x10000, CRC(794d0276) SHA1(ac903d2faa3fb315438dc8da22c5337611a8790d) )

	ROM_REGION( 0x400000, "sprites", 0 )    // adddress scrambling
	ROM_LOAD16_WORD_SWAP( "m1101.u6", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) )    // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )    // adddress scrambling
	ROM_LOAD16_WORD_SWAP( "m1103.u8", 0x00000, 0x80000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )
ROM_END

/* alt hardware, no IGS022 (protection) chip */

ROM_START( lhzb2a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p-4096", 0x00000, 0x80000, CRC(41293f32) SHA1(df4e993f4a458729ade13981e58f32d8116c0082) )

	ROM_REGION( 0x400000, "sprites", 0 )    // adddress scrambling
	ROM_LOAD16_WORD_SWAP( "m1101.u6", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) )    // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )    // adddress scrambling
	ROM_LOAD16_WORD_SWAP( "m1103.u8", 0x00000, 0x80000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )
ROM_END

/***************************************************************************

Mahjong Shuang Long Qiang Zhu 2
IGS, 1998

PCB Layout
----------

IGS PCB NO-0207
|---------------------------------------|
|                   K668  S1102.U20     |
|     PAL                               |
| 8MHz                     6264         |
|                                       |
|    |----|                6264         |
|    |IGS |                             |
|    |022 |  M1103.U12       PAL        |
|J   |----|                    PAL      |
|A                                      |
|M                                      |
|M      |-------|                       |
|A      |       |                       |
|       |IGS025 |   68000               |
|       |       |                       |
|       |-------|                       |
|                            P1100.U28  |
|                 PAL                   |
|  M1101.U4       |-------|             |
|                 |       |             |
|                 |IGS031 |      6264   |
|  TEXT.U6        |       |             |
|                 |-------|      62256  |
|SPDT_SW   22MHz   DSW1  DSW2  BATTERY  |
|---------------------------------------|
Notes:
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1.000MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( slqz2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1100.u28", 0x00000, 0x80000, CRC(0b8e5c9e) SHA1(16572bd1163bba4da8a76b10649d2f71e50ad369) )

	ROM_REGION( 0x10000, "igs022data", 0 )  // INTERNATIONAL GAMES SYSTEM CO.,LTD
	ROM_LOAD( "m1103.u12", 0x00000, 0x10000, CRC(9f3b8d65) SHA1(5ee1ad025474399c2826f21d970e76f25d0fa1fd) )

	ROM_REGION( 0x400000, "sprites", 0 )    // adddress scrambling
	ROM_LOAD16_WORD_SWAP( "m1101.u4", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) )    // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )    // light adddress scrambling
	ROM_LOAD( "text.u6", 0x00000, 0x80000, CRC(40d21adf) SHA1(18b202d6330ac89026bec2c9c8224b52540dd48d) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u20", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) ) // = s1102.u23 Mahjong Long Hu Zheng Ba 2
ROM_END

/***************************************************************************

Mahjong Man Guan Da Heng (V123T1)
(c) 1997 IGS

PCB Layout
----------

IGS PCB NO- 0252
|----------------------------------|
|    S1002.U22   6264   62256  SW  |
|TDA1020  FLASH.U19     PAL    BATT|
|  LM7805 M6295                    |
|   VOL                            |
|ULN2004                           |
|J             68000      IGS031   |
|A    DSW2                         |
|M                                 |
|M                                 |
|A           PAL PAL               |
|     IGS025                       |
|                                  |
|               M1001.U4      22MHz|
|                       TEXT.U6    |
|                              DSW1|
|    18WAY               10WAY     |
|----------------------------------|
Notes:
      68000     - Clock 11.000MHz [22/2]
      M6295     - Clock 1.000MHz [22/22]. Pin 7 HIGH
      DSW1/2    - 8-position Dip Switches
      SW        - Backup RAM Clear and Reset Switch
      FLASH.U19 - MX29F400 4M TSOP48 mounted onto a DIP adapter and plugged into a
                  socket. Under the socket is written '27C4096'
      M1001.U4  - 32M DIP42 Mask ROM
      S1002.U22 - 4M DIP32 Mask ROM
      TEXT.U6   - 27C1024 EPROM

***************************************************************************/

ROM_START( mgdha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "flash.u19", 0x00000, 0x80000, CRC(ff3aed2c) SHA1(829140e6fc7e4dfc039b0e7b647ce26d59b23b3d) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "m1001.u4", 0x000000, 0x400000, CRC(0cfb60d6) SHA1(e099aca730e7fd91a72915c27e569ad3d21f0d8f) )    // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x20000, "tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "text.u6", 0x00000, 0x20000, CRC(db50f8fc) SHA1(e2ce4a42f5bdc0b4b7988ad9e8d14661f17c3d51) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1002.u22", 0x00000, 0x80000, CRC(ac6b55f2) SHA1(7ff91fd1107272ad6bce071dc9ae2f374ebf5e3e) )
ROM_END

/***************************************************************************

Mahjong Man Guan Da Heng (V125T1)
(c) 1997 IGS

No hardware info, no sprites rom for this set.
It has additional protection.

***************************************************************************/

ROM_START( mgdh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "igs_f4bd.125", 0x00000, 0x80000, CRC(8bb0b870) SHA1(f0313f0b8b7575f4fff1feb99d48699d50556ef5) )

	ROM_REGION( 0x400000, "sprites", 0 )
	// not in this set
	ROM_LOAD( "m1001.u4", 0x000000, 0x400000, CRC(0cfb60d6) SHA1(e099aca730e7fd91a72915c27e569ad3d21f0d8f) )    // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x20000, "tilemaps", 0 )
	ROM_LOAD( "igs_512e.u6", 0x00000, 0x20000, CRC(db50f8fc) SHA1(e2ce4a42f5bdc0b4b7988ad9e8d14661f17c3d51) )   // == text.u6

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ig2_8836.u14", 0x00000, 0x80000, CRC(ac1f4da8) SHA1(789a2e0b58750292909dabca42c7e5ad72af3db5) )
ROM_END

/***************************************************************************

Tarzan

***************************************************************************/

// IGS NO-0248-1? Mislabeled?
ROM_START( tarzan )
	ROM_REGION( 0x40000, "maincpu", 0 ) // V109C TARZAN C
	ROM_LOAD( "0228-u16.bin", 0x00000, 0x40000, CRC(e6c552a5) SHA1(f156de9459833474c85a1f5b35917881b390d34c) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "a2104_cg_v110.u15", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "0228-u6.bin", 0x00000, 0x80000, CRC(55e94832) SHA1(b15409f4f1264b6d1218d5dc51c5bd1de2e40284) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "sound.u14", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x2dd * 2, "plds", 0 )
	ROM_LOAD( "eg.u20", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "eg.u21", 0x2dd, 0x2dd, NO_DUMP )
ROM_END

// IGS NO-0228?
ROM_START( tarzana )
	ROM_REGION( 0x80000, "maincpu", 0 ) // V107 TAISAN
	ROM_LOAD( "0228-u21.bin", 0x00000, 0x80000, CRC(80aaece4) SHA1(07cad92492c5de36c3915867ed4c6544b1a30c07) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "sprites.u17", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "sound.u16", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x2dd * 2, "plds", 0 )
	ROM_LOAD( "pal1", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "pal2", 0x2dd, 0x2dd, NO_DUMP )
ROM_END

/***************************************************************************

Super Tarzan (Italy, V100I)
IGS PCB NO-0230-1

      ----|     10/WAY CONN     |--------------------------------|
      |   |---------------------|                       |------| |
     _|                                                 | U23  | |
    |_|  [  U1  ][  U5  ] |--------------|   |-----|    | TDA  | |
    |_|                   | U8 IGS S2102 |   | U17 |    | 1020 | |
    |_|          [  U6  ] |--------------|   |-----|    |------| |
    |_|                   |--------------|                       |
    |_|                   | U9 SP V100I  | [   U15   ]   LM7805  |
 3  |_|                   |--------------| |-------------------| |
 6  |_|       |----------|                 |  U16 Z8018008PSC  | |
 W  |_|       | U4       | [ TAR97 U10-1 ] |-------------------| |
 A  |_|       | IGS025   |                    OSC                |
 Y  |_|       | S_TARZAN |     [   U13   ] 16.0Mhz [ TAR97 U20 ] |
    |_|       |          |                                       |
 C  |_|       |----------|                      [ U18 ]  [ U20 ] |
 O  |_|                    |-----------|                         |
 N  |_|                    | U12       |                   SW1   |
 N  |_|                    | IGS 031   |                         |
    |_|                    | F00030142 |                   SW2   |
    |_|                    |           |        [ U19 ]          |
    |_|                    |-----------|                   SW3   |
    |_| |----------------|                22.00 Mhz              |
    |_| | U2  TBM27C4096 |                                       |
    |_| |----------------|   [ R20 Ohm 5W ]        Battery       |
    |_| |----------------| |----------------------| (---)  Reset |
      | | U3  C0057209   | | U11 IGST2105 CG V110 | (3.6)  SW4   |
      | |----------------| |----------------------| (---)    \   |
      |----------------------------------------------------------|

      U1,U5,U6  ULN2004A               SW4 1pos switch for reset
            U2  TBM TB27C4096          Sw1-2-3  8x2 DSW
            U4  IGS025 (protection?)
            U8  IGS S2102 SP V102 1P1327A6 C000538
           U11  IGS T2105 CG V110 1P1379C1 S000938
           U12  IGS031 F00030142 (graphic array?)
           U17  K668 = Oki M6295 (QFP44). Clock 1.000MHz [8/8]. pin7 = High
           U16  Zilog Z8018008PSC Z180 MPU
           U18  DN74LS14N
           U19  SN74HC132N
           U20  HD74LS161AP
   U10-1,U20-1  PALCE22V10H (read protected)

***************************************************************************/

ROM_START( starzan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v100i.u9", 0x00000, 0x40000, CRC(64180bff) SHA1(b08dbe8a17ca33024442ebee41f111c8f98a2109) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "cg.u2",             0x00000, 0x80000, CRC(884f95f5) SHA1(2e526aa966e90dc696a8b392a5a99e14f03c4bd4) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "t2105_cg_v110.u11", 0x80000, 0x80000, NO_DUMP )

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "c0057209.u3", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "s2102_sp_v102.u8", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x2dd * 2, "plds", 0 )
	ROM_LOAD( "palce22v10h_tar97_u10-1.u10", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "palce22v10h_tar97_u20.u20",   0x2dd, 0x2dd, NO_DUMP )
ROM_END

/***************************************************************************

Super Poker (v100xD03) / Formosa

PCB NO-0187

CPU Z8018008psc
IGS017
IGS025
K668 (AD-65)
UM3567 (YM2413)
Audio Xtal 3.579545
CPU Xtal 16Mhz
3 x DSW8

***************************************************************************/

ROM_START( spkrform )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "super2in1-v100xd03.u29", 0x00000, 0x40000, CRC(e8f7476c) SHA1(e20241d68d22ee01a65f5d7921fe2291077f081f) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "super2in1.u26", 0x00000, 0x80000, CRC(af3b1d9d) SHA1(ce84b076939d2c9d959cd430d4f5664f32735d60) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_LOAD( "super2in1.u25", 0x80000, 0x80000, CRC(7ebaf0a0) SHA1(c278810742cd7e1daa89a93fd7fe82495543ccbf) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "super2in1.u24", 0x00000, 0x40000, CRC(54d68c49) SHA1(faad78779c3a5b4ecb1c733192d9477ce3324f71) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "super2in1sp.u28", 0x00000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END


GAME( 1996,  iqblocka, iqblock,  iqblocka, iqblocka, igs017_state, iqblocka, ROT0, "IGS",              "Shu Zi Le Yuan (V127M)",                      MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1996,  iqblockf, iqblock,  iqblocka, iqblocka, igs017_state, iqblockf, ROT0, "IGS",              "Shu Zi Le Yuan (V113FR)",                     MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1997,  mgdh,     0,        mgdha,    mgdh,     igs017_state, mgdh,     ROT0, "IGS",              "Mahjong Man Guan Da Heng (Taiwan, V125T1)",   0 )
GAME( 1997,  mgdha,    mgdh,     mgdha,    mgdh ,    igs017_state, mgdha,    ROT0, "IGS",              "Mahjong Man Guan Da Heng (Taiwan, V123T1)",   0 )
GAME( 1997,  sdmg2,    0,        sdmg2,    sdmg2,    igs017_state, sdmg2,    ROT0, "IGS",              "Mahjong Super Da Man Guan II (China, V754C)", 0 )
GAME( 1997,  tjsb,     0,        tjsb,     tjsb,     igs017_state, tjsb,     ROT0, "IGS",              "Mahjong Tian Jiang Shen Bing (V137C)",        MACHINE_UNEMULATED_PROTECTION )
GAME( 1998,  mgcs,     0,        mgcs,     mgcs,     igs017_state, mgcs,     ROT0, "IGS",              "Mahjong Man Guan Cai Shen (V103CS)",          MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND )
GAME( 1998,  lhzb2,    0,        lhzb2,    lhzb2,    igs017_state, lhzb2,    ROT0, "IGS",              "Mahjong Long Hu Zheng Ba 2 (set 1)",          MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1998,  lhzb2a,   lhzb2,    lhzb2a,   lhzb2a,   igs017_state, lhzb2a,   ROT0, "IGS",              "Mahjong Long Hu Zheng Ba 2 (VS221M)",         0 )
GAME( 1998,  slqz2,    0,        slqz2,    slqz2,    igs017_state, slqz2,    ROT0, "IGS",              "Mahjong Shuang Long Qiang Zhu 2 (VS203J)",    MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1999?, tarzan,   0,        iqblocka, iqblocka, igs017_state, tarzan,   ROT0, "IGS",              "Tarzan (V109C)",                              MACHINE_NOT_WORKING )
GAME( 1999?, tarzana,  tarzan,   iqblocka, iqblocka, igs017_state, tarzana,  ROT0, "IGS",              "Tarzan (V107)",                               MACHINE_NOT_WORKING )
GAME( 2000?, starzan,  0,        starzan,  iqblocka, igs017_state, starzan,  ROT0, "IGS / G.F. Gioca", "Super Tarzan (Italy, V100I)",                 MACHINE_NOT_WORKING )
GAME( ????,  spkrform, spk116it, spkrform, spkrform, igs017_state, spkrform, ROT0, "IGS",              "Super Poker (v100xD03) / Formosa",            MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
