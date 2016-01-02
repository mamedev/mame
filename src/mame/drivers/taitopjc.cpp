// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*
   Taito Power-JC System

   Skeleton driver. Requires TLCS-900 CPU core to make progress.

   Hardware appears sufficiently different to JC system to warrant
   a separate driver.

   PCB Information (incomplete!)
   ===============

   POWER JC MOTHER-G PCB
   K11X0870A
   OPERATION TIGER

   PowerPC 603E
   CXD1176Q

   TMS320C53PQ80
   40MHz osc
   43256 x 2
   E63-03_H.29 (AT27C512 PLCC)
   E63-04_L.28 (AT27C512 PLCC)

   E63-01 PALCE16V8H
   E63-02 PALCE22V10H

   IC41 E63-06 PALCE16V8H
   IC43 E63-07 PALCE16V8H

   uPD4218160 x 2
   uPD4218160 x 2

   uPD482445 x 4

   CY78991
   IS61LV256AH x 3
   Taito TC0780FPA x 2
   Taito TCG010PJC

   MN1020819
   ZOOM ZSG-2
   ZOOM ZFX-2
   MSM514256


   Second PCB
   ----------

   19 ROMs

   TMP95C063F
   25.0000MHz osc
   1.84320MHz osc
*/

/*
    PPC -> TLCS Commands:
        0x5010:            ?                                        RTC?
        0x5020:            ?                                        RTC?
        0x6000:            ?                                        Backup RAM init?
        0x6010:            ?                                        Backup RAM Read. Address in io_shared[0x1d00].
        0x6020:            ?                                        Backup RAM Write. Address in io_shared[0x1d00].
        0x6030:            ?                                        ?
        0x6040:            ?                                        ?
        0x4000:            ?                                        Sound?
        0x4001:            ?
        0x4002:            ?
        0x4003:            ?
        0x4004:            ?
        0xf055:
        0xf0ff:
        0xf000:
        0xf001:
        0xf010:
        0xf020:

    TLCS -> PPC Commands:
        0x7000:                                                     DSP ready
        0xd000:                                                     Vblank

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/tlcs900/tlcs900.h"
#include "cpu/mn10200/mn10200.h"
#include "cpu/tms32051/tms32051.h"
#include "video/tc0780fpa.h"
#include "machine/nvram.h"

#define LOG_TLCS_TO_PPC_COMMANDS        1
#define LOG_PPC_TO_TLCS_COMMANDS        1
#define LOG_DISPLAY_LIST                    0

class taitopjc_state : public driver_device
{
public:
	taitopjc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_iocpu(*this, "iocpu"),
		m_soundcpu(*this, "mn10200"),
		m_dsp(*this, "dsp"),
		m_tc0780fpa(*this, "tc0780fpa"),
		m_palette(*this, "palette"),
		m_polyrom(*this, "poly"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_iocpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_dsp;
	required_device<tc0780fpa_device> m_tc0780fpa;
	required_device<palette_device> m_palette;
	required_memory_region m_polyrom;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_READ64_MEMBER(video_r);
	DECLARE_WRITE64_MEMBER(video_w);
	DECLARE_READ64_MEMBER(ppc_common_r);
	DECLARE_WRITE64_MEMBER(ppc_common_w);
	DECLARE_READ64_MEMBER(dsp_r);
	DECLARE_WRITE64_MEMBER(dsp_w);
	DECLARE_READ8_MEMBER(tlcs_common_r);
	DECLARE_WRITE8_MEMBER(tlcs_common_w);
	DECLARE_READ8_MEMBER(tlcs_sound_r);
	DECLARE_WRITE8_MEMBER(tlcs_sound_w);
	DECLARE_WRITE16_MEMBER(tlcs_unk_w);
	DECLARE_READ16_MEMBER(tms_dspshare_r);
	DECLARE_WRITE16_MEMBER(tms_dspshare_w);
	DECLARE_READ16_MEMBER(dsp_rom_r);
	DECLARE_WRITE16_MEMBER(dsp_roml_w);
	DECLARE_WRITE16_MEMBER(dsp_romh_w);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_taitopjc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(taitopjc_vbi);
	UINT32 videochip_r(offs_t address);
	void videochip_w(offs_t address, UINT32 data);
	void video_exit();
	void print_display_list();
	TILE_GET_INFO_MEMBER(tile_get_info);
	TILEMAP_MAPPER_MEMBER(tile_scan_layer0);
	TILEMAP_MAPPER_MEMBER(tile_scan_layer1);

	DECLARE_DRIVER_INIT(optiger);

	UINT16 m_dsp_ram[0x1000];
	UINT16 m_io_share_ram[0x2000];

	std::unique_ptr<UINT32[]> m_screen_ram;
	std::unique_ptr<UINT32[]> m_pal_ram;

	tilemap_t *m_tilemap[2];

	UINT32 m_video_address;

	UINT32 m_dsp_rom_address;
};

void taitopjc_state::video_exit()
{
#if 0
	FILE *file;
	int i;

	file = fopen("pjc_screen_ram.bin","wb");
	for (i=0; i < 0x40000; i++)
	{
		fputc((UINT8)(m_screen_ram[i] >> 24), file);
		fputc((UINT8)(m_screen_ram[i] >> 16), file);
		fputc((UINT8)(m_screen_ram[i] >> 8), file);
		fputc((UINT8)(m_screen_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("pjc_pal_ram.bin","wb");
	for (i=0; i < 0x8000; i++)
	{
		fputc((UINT8)(m_pal_ram[i] >> 24), file);
		fputc((UINT8)(m_pal_ram[i] >> 16), file);
		fputc((UINT8)(m_pal_ram[i] >> 8), file);
		fputc((UINT8)(m_pal_ram[i] >> 0), file);
	}
	fclose(file);
#endif
}

TILE_GET_INFO_MEMBER(taitopjc_state::tile_get_info)
{
	UINT32 val = m_screen_ram[0x3f000 + (tile_index/2)];

	if (!(tile_index & 1))
		val >>= 16;

	int color = (val >> 12) & 0xf;
	int tile = (val & 0xfff);
	int flags = 0;

	SET_TILE_INFO_MEMBER(0, tile, color, flags);
}

TILEMAP_MAPPER_MEMBER(taitopjc_state::tile_scan_layer0)
{
	/* logical (col,row) -> memory offset */
	return (row * 64) + col;
}

TILEMAP_MAPPER_MEMBER(taitopjc_state::tile_scan_layer1)
{
	/* logical (col,row) -> memory offset */
	return (row * 64) + col + 32;
}

void taitopjc_state::video_start()
{
	static const gfx_layout char_layout =
	{
		16, 16,
		4032,
		8,
		{ 0,1,2,3,4,5,6,7 },
		{ 3*8, 2*8, 1*8, 0*8, 7*8, 6*8, 5*8, 4*8, 11*8, 10*8, 9*8, 8*8, 15*8, 14*8, 13*8, 12*8 },
		{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
		8*256
	};

	m_screen_ram = std::make_unique<UINT32[]>(0x40000);
	m_pal_ram = std::make_unique<UINT32[]>(0x8000);

	m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(taitopjc_state::tile_get_info),this), tilemap_mapper_delegate(FUNC(taitopjc_state::tile_scan_layer0),this), 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(taitopjc_state::tile_get_info),this), tilemap_mapper_delegate(FUNC(taitopjc_state::tile_scan_layer1),this), 16, 16, 32, 32);
	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(1);
	
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, char_layout, (UINT8*)m_screen_ram.get(), 0, m_palette->entries() / 256, 0));

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(taitopjc_state::video_exit), this));
}

UINT32 taitopjc_state::screen_update_taitopjc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x000000, cliprect);

	m_tc0780fpa->draw(bitmap, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0);

	return 0;
}

UINT32 taitopjc_state::videochip_r( offs_t address)
{
	UINT32 r = 0;

	if (address >= 0x10000000 && address < 0x10040000)
	{
		r = m_screen_ram[address - 0x10000000];
	}

	return r;
}

void taitopjc_state::videochip_w(offs_t address, UINT32 data)
{
	if (address >= 0x20000000 && address < 0x20008000)
	{
		m_pal_ram[address - 0x20000000] = data;

		int b = (data >> 16) & 0xff;
		int g = (data >>  8) & 0xff;
		int r = (data >>  0) & 0xff;
		m_palette->set_pen_color(address - 0x20000000, rgb_t(r, g, b));
	}
	else if (address >= 0x10000000 && address < 0x10040000)
	{
		UINT32 addr = address - 0x10000000;
		m_screen_ram[addr] = data;
		
		if (address >= 0x1003f000)
		{
			UINT32 a = address - 0x1003f000;
			m_tilemap[0]->mark_tile_dirty((a*2));
			m_tilemap[0]->mark_tile_dirty((a*2)+1);
		}
		else
		{
			m_gfxdecode->gfx(0)->mark_dirty(addr / 64);
		}
	}
	else
	{
		printf("Address %08X = %08X\n", address, data);
	}
}

READ64_MEMBER(taitopjc_state::video_r)
{
	UINT64 r = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_32_63)
		{
			r |= (UINT64)(videochip_r(m_video_address)) << 32;
		}
	}

	return r;
}

WRITE64_MEMBER(taitopjc_state::video_w)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_32_63)
		{
			//printf("Address %08X = %08X\n", video_address, (UINT32)(data >> 32));
			videochip_w(m_video_address, (UINT32)(data >> 32));
		}
	}
	if (offset == 1)
	{
		if (ACCESSING_BITS_32_63)
		{
			m_video_address = (UINT32)(data >> 32);
		}
	}
}

READ64_MEMBER(taitopjc_state::ppc_common_r)
{
	UINT64 r = 0;
	UINT32 address;

	//logerror("ppc_common_r: %08X, %08X%08X\n", offset, (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));

	address = offset * 2;

	if (ACCESSING_BITS_48_63)
	{
		r |= (UINT64)(m_io_share_ram[address]) << 48;
	}
	if (ACCESSING_BITS_16_31)
	{
		r |= (UINT64)(m_io_share_ram[address+1]) << 16;
	}

	return r;
}

WRITE64_MEMBER(taitopjc_state::ppc_common_w)
{
	UINT32 address = offset * 2;

//  logerror("ppc_common_w: %08X, %I64X, %I64X\n", offset, data, mem_mask);

	if (ACCESSING_BITS_48_63)
	{
		m_io_share_ram[address] = (UINT16)(data >> 48);
	}
	if (ACCESSING_BITS_16_31)
	{
		m_io_share_ram[address+1] = (UINT16)(data >> 16);
	}

	if (offset == 0x7ff && ACCESSING_BITS_48_63)
	{
#if LOG_PPC_TO_TLCS_COMMANDS
		if (m_io_share_ram[0xfff] != 0x0000)
		{
			printf("PPC -> TLCS cmd %04X\n", m_io_share_ram[0xfff]);
		}
#endif

		m_iocpu->set_input_line(TLCS900_INT6, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

READ64_MEMBER(taitopjc_state::dsp_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_48_63)
	{
		int addr = offset * 2;
		r |= (UINT64)(m_dsp_ram[addr+0]) << 48;
	}
	if (ACCESSING_BITS_16_31)
	{
		int addr = offset * 2;
		r |= (UINT64)(m_dsp_ram[addr+1]) << 16;
	}

	return r;
}

void taitopjc_state::print_display_list()
{
	int ptr = 0;

	UINT16 cmd = m_dsp_ram[0xffe];
	if (cmd == 0x5245)
	{
		printf("DSP command RE\n");
		bool end = false;
		do
		{
			UINT16 w = m_dsp_ram[ptr++];
			if (w & 0x8000)
			{
				int count = (w & 0x7fff) + 1;
				UINT16 d = m_dsp_ram[ptr++];
				for (int i=0; i < count; i++)
				{
					UINT16 s = m_dsp_ram[ptr++];
					printf("   %04X -> [%04X]\n", s, d);
					d++;
				}
			}
			else if (w == 0)
			{
				end = true;
			}
			else
			{
				switch (w)
				{
					case 0x406d:
						printf("   Call %04X [%04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1]);
						ptr += 2;
						break;
					case 0x40cd:
						printf("   Call %04X [%04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1]);
						ptr += 2;
						break;
					case 0x40ac:
						printf("   Call %04X [%04X %04X %04X %04X %04X %04X %04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1], m_dsp_ram[ptr+2], m_dsp_ram[ptr+3], m_dsp_ram[ptr+4], m_dsp_ram[ptr+5], m_dsp_ram[ptr+6], m_dsp_ram[ptr+7]);
						ptr += 8;
						break;
					case 0x4774:
						printf("   Call %04X [%04X %04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1], m_dsp_ram[ptr+2]);
						ptr += 3;
						break;
					case 0x47d9:
						printf("   Call %04X [%04X %04X %04X %04X %04X %04X %04X %04X]\n", w, m_dsp_ram[ptr], m_dsp_ram[ptr+1], m_dsp_ram[ptr+2], m_dsp_ram[ptr+3], m_dsp_ram[ptr+4], m_dsp_ram[ptr+5], m_dsp_ram[ptr+6], m_dsp_ram[ptr+7]);
						ptr += 8;
						break;
					default:
					{
						printf("Unknown call %04X\n", w);
						for (int i=0; i < 10; i++)
						{
							printf("%04X\n", m_dsp_ram[ptr++]);
						}
						fatalerror("Unknown call %04X\n", w);
						break;
					}
				}
			}
		} while(!end);
	}
	else
	{
		if (cmd != 0)
			printf("DSP command %04X\n", cmd);
		return;
	}
}

WRITE64_MEMBER(taitopjc_state::dsp_w)
{
	//logerror("dsp_w: %08X, %08X%08X, %08X%08X at %08X\n", offset, (UINT32)(data >> 32), (UINT32)(data), (UINT32)(mem_mask >> 32), (UINT32)(mem_mask), space.device().safe_pc());

	if (offset == 0x7fe)
	{
		#if 0
		{
			int i;
			FILE *f = fopen("dspram.bin", "wb");
			for (i=0; i < 0x1000; i++)
			{
				fputc((dsp_ram[i] >> 0) & 0xff, f);
				fputc((dsp_ram[i] >> 8) & 0xff, f);
			}
			fclose(f);
		}
		#endif

#if LOG_DISPLAY_LIST
		print_display_list();
#endif
	}

	if (ACCESSING_BITS_48_63)
	{
		int addr = offset * 2;
		m_dsp_ram[addr+0] = (data >> 48) & 0xffff;
	}
	if (ACCESSING_BITS_16_31)
	{
		int addr = offset * 2;
		m_dsp_ram[addr+1] = (data >> 16) & 0xffff;
	}
}

// BAT Config:
// IBAT0 U: 0x40000002   L: 0x40000022      (0x40000000...0x4001ffff)
// IBAT1 U: 0x0000007f   L: 0x00000002      (0x00000000...0x003fffff)
// IBAT2 U: 0xc0000003   L: 0xc0000022      (0xc0000000...0xc001ffff)
// IBAT3 U: 0xfe0003ff   L: 0xfe000022      (0xfe000000...0xffffffff)
// DBAT0 U: 0x40000002   L: 0x40000022      (0x40000000...0x4001ffff)
// DBAT1 U: 0x0000007f   L: 0x00000002      (0x00000000...0x003fffff)
// DBAT2 U: 0xc0000003   L: 0xc0000022      (0xc0000000...0xc001ffff)
// DBAT3 U: 0xfe0003ff   L: 0xfe000022      (0xfe000000...0xffffffff)

static ADDRESS_MAP_START( ppc603e_mem, AS_PROGRAM, 64, taitopjc_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM // Work RAM
	AM_RANGE(0x40000000, 0x4000000f) AM_READWRITE(video_r, video_w)
	AM_RANGE(0x80000000, 0x80003fff) AM_READWRITE(dsp_r, dsp_w)
	AM_RANGE(0xc0000000, 0xc0003fff) AM_READWRITE(ppc_common_r, ppc_common_w)
	AM_RANGE(0xfe800000, 0xff7fffff) AM_ROM AM_REGION("gfx1", 0)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END




READ8_MEMBER(taitopjc_state::tlcs_common_r)
{
	if (offset & 1)
	{
		return (UINT8)(m_io_share_ram[offset / 2] >> 8);
	}
	else
	{
		return (UINT8)(m_io_share_ram[offset / 2]);
	}
}

WRITE8_MEMBER(taitopjc_state::tlcs_common_w)
{
	if (offset & 1)
	{
		m_io_share_ram[offset / 2] &= 0x00ff;
		m_io_share_ram[offset / 2] |= (UINT16)(data) << 8;
	}
	else
	{
		m_io_share_ram[offset / 2] &= 0xff00;
		m_io_share_ram[offset / 2] |= data;
	}

	if (offset == 0x1fff)
	{
		m_iocpu->set_input_line(TLCS900_INT6, CLEAR_LINE);
	}

	if (offset == 0x1ffd)
	{
#if LOG_TLCS_TO_PPC_COMMANDS
		if (m_io_share_ram[0xffe] != 0xd000 &&
			m_io_share_ram[0xffe] != 0x7000)
		{
			printf("TLCS -> PPC cmd %04X\n", m_io_share_ram[0xffe]);
		}
#endif

		m_iocpu->set_input_line(TLCS900_INT1, CLEAR_LINE);
		m_iocpu->set_input_line(TLCS900_INT2, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

READ8_MEMBER(taitopjc_state::tlcs_sound_r)
{
	if (offset == 0x17)
	{
		return 0x55;
	}

	return 0;
}

WRITE8_MEMBER(taitopjc_state::tlcs_sound_w)
{
//  printf("tlcs_sound_w: %08X, %02X\n", offset, data);
}

WRITE16_MEMBER(taitopjc_state::tlcs_unk_w)
{
	if (offset == 0xc/2)
	{
		int reset = (data & 0x4) ? ASSERT_LINE : CLEAR_LINE;
		m_maincpu->set_input_line(INPUT_LINE_RESET, reset);
	}
}

// TLCS900 interrupt vectors
// 0xfc0100: reset
// 0xfc00ea: INT0 (dummy)
// 0xfc00eb: INT1 vblank?
// 0xfc00f0: INT2 DSP ready?
// 0xfc00f5: INT3 (dummy)
// 0xfc00f6: INT4 (dummy)
// 0xfc00f7: INT5 (dummy)
// 0xfc00f8: INT6 PPC command
// 0xfc00fd: INT7 (dummy)
// 0xfc00fe: INT8 (dummy)
// 0xfc0663: INTT1
// 0xfc0f7d: INTRX0
// 0xfc0f05: INTTX0
// 0xfc0fb5: INTRX1
// 0xfc0f41: INTTX1

static ADDRESS_MAP_START( tlcs900h_mem, AS_PROGRAM, 16, taitopjc_state )
	AM_RANGE(0x010000, 0x02ffff) AM_RAM     // Work RAM
	AM_RANGE(0x040000, 0x0400ff) AM_READWRITE8(tlcs_sound_r, tlcs_sound_w, 0xffff)
	AM_RANGE(0x044000, 0x045fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x060000, 0x061fff) AM_READWRITE8(tlcs_common_r, tlcs_common_w, 0xffff)
	AM_RANGE(0x06c000, 0x06c00f) AM_WRITE(tlcs_unk_w)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("io_cpu", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START( mn10200_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("mn10200", 0)
ADDRESS_MAP_END



READ16_MEMBER(taitopjc_state::tms_dspshare_r)
{
	return m_dsp_ram[offset];
}

WRITE16_MEMBER(taitopjc_state::tms_dspshare_w)
{
	if (offset == 0xffc)
	{
		m_iocpu->set_input_line(TLCS900_INT2, ASSERT_LINE);
	}
	m_dsp_ram[offset] = data;
}

READ16_MEMBER(taitopjc_state::dsp_rom_r)
{
	assert(m_dsp_rom_address < 0x800000);

	UINT16 data = ((UINT16*)m_polyrom->base())[m_dsp_rom_address];
	m_dsp_rom_address++;
	return data;
}

WRITE16_MEMBER(taitopjc_state::dsp_roml_w)
{
	m_dsp_rom_address &= 0xffff0000;
	m_dsp_rom_address |= data;
}

WRITE16_MEMBER(taitopjc_state::dsp_romh_w)
{
	m_dsp_rom_address &= 0xffff;
	m_dsp_rom_address |= (UINT32)(data) << 16;
}


static ADDRESS_MAP_START( tms_program_map, AS_PROGRAM, 16, taitopjc_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0x4c00, 0xefff) AM_ROM AM_REGION("user2", 0x9800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tms_data_map, AS_DATA, 16, taitopjc_state )
	AM_RANGE(0x4000, 0x6fff) AM_ROM AM_REGION("user2", 0x8000)
	AM_RANGE(0x7000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(tms_dspshare_r, tms_dspshare_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tms_io_map, AS_IO, 16, taitopjc_state )
	AM_RANGE(0x0053, 0x0053) AM_WRITE(dsp_roml_w)
	AM_RANGE(0x0057, 0x0057) AM_WRITE(dsp_romh_w)
	AM_RANGE(0x0058, 0x0058) AM_DEVWRITE("tc0780fpa", tc0780fpa_device, poly_fifo_w)
	AM_RANGE(0x005a, 0x005a) AM_DEVWRITE("tc0780fpa", tc0780fpa_device, tex_w)
	AM_RANGE(0x005b, 0x005b) AM_DEVREADWRITE("tc0780fpa", tc0780fpa_device, tex_addr_r, tex_addr_w)
	AM_RANGE(0x005f, 0x005f) AM_READ(dsp_rom_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( taitopjc )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN1 )            // Coin A
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)        // Service switch
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW)      // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START1 )       // Select 1
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_START2 )       // Select 2
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)       // P1 trigger
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)       // P1 bomb
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)       // P2 trigger
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)       // P2 bomb
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG1")       // Player 1 X
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")       // Player 1 Y
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")       // Player 2 X
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_PLAYER(2) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG4")       // Player 2 Y
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_PLAYER(2) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)
INPUT_PORTS_END


void taitopjc_state::machine_reset()
{
	// halt sound CPU since we don't emulate this yet
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_dsp_rom_address = 0;
}


INTERRUPT_GEN_MEMBER(taitopjc_state::taitopjc_vbi)
{
	m_iocpu->set_input_line(TLCS900_INT1, ASSERT_LINE);
}


static MACHINE_CONFIG_START( taitopjc, taitopjc_state )
	MCFG_CPU_ADD("maincpu", PPC603E, 100000000)
	MCFG_PPC_BUS_FREQUENCY(XTAL_66_6667MHz)    /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	MCFG_CPU_PROGRAM_MAP(ppc603e_mem)

	/* TMP95C063F I/O CPU */
	MCFG_CPU_ADD("iocpu", TMP95C063, 25000000)
	MCFG_TMP95C063_PORT5_READ(IOPORT("INPUTS1"))
	MCFG_TMP95C063_PORTD_READ(IOPORT("INPUTS2"))
	MCFG_TMP95C063_PORTE_READ(IOPORT("INPUTS3"))
	MCFG_CPU_PROGRAM_MAP(tlcs900h_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitopjc_state,  taitopjc_vbi)

	/* TMS320C53 DSP */
	MCFG_CPU_ADD("dsp", TMS32053, 40000000)
	MCFG_CPU_PROGRAM_MAP(tms_program_map)
	MCFG_CPU_DATA_MAP(tms_data_map)
	MCFG_CPU_IO_MAP(tms_io_map)

	MCFG_CPU_ADD("mn10200", MN1020012A, 10000000) /* MN1020819DA sound CPU - NOTE: May have 64kB internal ROM */
	MCFG_CPU_PROGRAM_MAP(mn10200_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(taitopjc_state, screen_update_taitopjc)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_DEVICE_ADD("tc0780fpa", TC0780FPA, 0)

MACHINE_CONFIG_END


DRIVER_INIT_MEMBER(taitopjc_state, optiger)
{
	UINT8 *rom = (UINT8*)memregion("io_cpu")->base();

	// skip sound check
	rom[0x217] = 0x00;
	rom[0x218] = 0x00;

#if 0
	UINT32 *mr = (UINT32*)memregion("user1")->base();
	//mr[(0x23a5c^4)/4] = 0x60000000;
	mr[((0x513b0-0x40000)^4)/4] = 0x38600001;
#endif
}


ROM_START( optiger )
	ROM_REGION64_BE( 0x200000, "user1", 0 )
	ROM_LOAD32_BYTE( "e63-33-1_p-hh.23", 0x000000, 0x080000, CRC(5ab176e2) SHA1(a0a5b7c0e91928d0a49987f88f6ae647f5cb3e34) )
	ROM_LOAD32_BYTE( "e63-32-1_p-hl.22", 0x000001, 0x080000, CRC(cca8bacc) SHA1(e5a081f5c12a52601745f5b67fe3412033581b00) )
	ROM_LOAD32_BYTE( "e63-31-1_p-lh.8",  0x000002, 0x080000, CRC(ad69e649) SHA1(9fc853d2cb6e7cac87dc06bad91048f191b799c5) )
	ROM_LOAD32_BYTE( "e63-30-1_p-ll.7",  0x000003, 0x080000, CRC(a6183479) SHA1(e556c3edf100342079e680ec666f018fca7a82b0) )

	ROM_REGION16_BE( 0x20000, "user2", 0 )
	ROM_LOAD16_BYTE( "e63-04_l.29",  0x000001, 0x010000, CRC(eccae391) SHA1(e5293c16342cace54dc4b6dfb827558e18ac25a4) )
	ROM_LOAD16_BYTE( "e63-03_h.28",  0x000000, 0x010000, CRC(58fce52f) SHA1(1e3d9ee034b25e658ca45a8b900de2aa54b00135) )

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e63-28-1_0.59", 0x000000, 0x020000, CRC(ef41ffaf) SHA1(419621f354f548180d37961b861304c469e43a65) )
	ROM_LOAD16_BYTE( "e63-27-1_1.58", 0x000001, 0x020000, CRC(facc17a7) SHA1(40d69840cfcfe5a509d69824c2994de56a3c6ece) )

	ROM_REGION( 0x80000, "mn10200", 0 )
	ROM_LOAD16_BYTE( "e63-17-1_s-l.18", 0x000000, 0x040000, CRC(2a063d5b) SHA1(a2b2fe4d8bad1aef7d9dcc0be607cc4e5bc4f0eb) )
	ROM_LOAD16_BYTE( "e63-18-1_s-h.19", 0x000001, 0x040000, CRC(2f590881) SHA1(7fb827a676f45b24380558b0068b76cb858314f6) )

	ROM_REGION64_BE( 0x1000000, "gfx1", 0 )
	ROM_LOAD32_WORD_SWAP( "e63-21_c-h.24", 0x000000, 0x400000, CRC(c818b211) SHA1(dce07bfe71a9ba11c3f028a640226c6e59c6aece) )
	ROM_LOAD32_WORD_SWAP( "e63-15_c-l.9",  0x000002, 0x400000, CRC(4ec6a2d7) SHA1(2ee6270cff7ea2459121961a29d42e000cee2921) )
	ROM_LOAD32_WORD_SWAP( "e63-22_m-h.25", 0x800000, 0x400000, CRC(6d895eb6) SHA1(473795da42fd29841a926f18a93e5992f4feb27c) )
	ROM_LOAD32_WORD_SWAP( "e63-16_m-l.10", 0x800002, 0x400000, CRC(d39c1e34) SHA1(6db0ce2251841db3518a9bd9c4520c3c666d19a0) )

	ROM_REGION16_BE( 0xc00000, "poly", 0 )
	ROM_LOAD16_WORD_SWAP( "e63-09_poly0.3", 0x000000, 0x400000, CRC(c3e2b1e0) SHA1(ee71f3f59b46e26dbe2ff724da2c509267c8bf2f) )
	ROM_LOAD16_WORD_SWAP( "e63-10_poly1.4", 0x400000, 0x400000, CRC(f4a56390) SHA1(fc3c51a7f4639479e66ad50dcc94255d94803c97) )
	ROM_LOAD16_WORD_SWAP( "e63-11_poly2.5", 0x800000, 0x400000, CRC(2293d9f8) SHA1(16adaa0523168ee63a7a34b29622c623558fdd82) )
// Poly 3 is not populated

	ROM_REGION( 0x800000, "sound_data", 0 )
	ROM_LOAD( "e63-23_wd0.36", 0x000000, 0x200000, CRC(d69e196e) SHA1(f738bb9e1330f6dabb5e0f0378a1a8eb48a4fa40) )
	ROM_LOAD( "e63-24_wd1.37", 0x200000, 0x200000, CRC(cd55f17b) SHA1(08f847ef2fd592dbaf63ef9e370cdf1f42012f74) )
	ROM_LOAD( "e63-25_wd2.38", 0x400000, 0x200000, CRC(bd35bdac) SHA1(5cde6c1a6b74659507b31fcb88257e65f230bfe2) )
	ROM_LOAD( "e63-26_wd3.39", 0x600000, 0x200000, CRC(346bd413) SHA1(0f6081d22db88eef08180278e7ae97283b5e8452) )

	ROM_REGION( 0x500, "plds", ROMREGION_ERASEFF )
	// TODO: There are 6 PALs in total on the main PCB.
ROM_END

GAME( 1998, optiger, 0, taitopjc, taitopjc, taitopjc_state, optiger, ROT0, "Taito", "Operation Tiger", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
