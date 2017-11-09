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
     R5F21256SN(extra MCU for protection and ICcard communication)
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
    The King of Fighters '98 - Ultimate Match - Hero  (NOT DUMPED)
    Knights of Valour 2 New Legend
    Dodonpachi Daioujou Tamashii
    Knights of Valour 3

    These were only released as single board PGM2 based hardware, seen for sale in Japan for around $250-$300

    Jigsaw World Arena
    Puzzle of Ocha / Ochainu No Pazuru


	ToDo (emulation issues):

	Support remaining games (need IGS036 dumps)
	Identify which regions each game was released in and either dump alt. internal ROMs for each region, or
	  create them until that can be done.
	RTC
	Memory Card system (there's an MCU on the motherboard that will need simulating or dumping somehow)
	Sprite Zoom (Dodonpachi Daioujou Tamashii will likely provide test case for this)
	Simplify IGS036 encryption based on tables in internal roms
	Fix ARM? bug that means Oriental Legend 2 needs a patch
	Fix Save States (is this a driver problem or an ARM core problem, they don't work unless you get through the startup tests)

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/ymz770.h"
#include "machine/igs036crypt.h"
#include "screen.h"
#include "speaker.h"
#include "machine/nvram.h"
#include "machine/timer.h"

// see http://sam7-ex256.narod.ru/include/HTML/AT91SAM7X256_AIC.html
DECLARE_DEVICE_TYPE(ARM_AIC, arm_aic_device)

#define MCFG_ARM_AIC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ARM_AIC, 0)

#define MCFG_IRQ_LINE_CB(_devcb) \
	devcb = &arm_aic_device::set_line_callback(*device, DEVCB_##_devcb);

class arm_aic_device : public device_t
{
public:
	// construction/destruction
	arm_aic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, ARM_AIC, tag, owner, clock),
		m_irq_out(*this)
	{ 
	}

	// configuration
	template <class Object> static devcb_base &set_line_callback(device_t &device, Object &&cb) { return downcast<arm_aic_device &>(device).m_irq_out.set_callback(std::forward<Object>(cb)); }

	DECLARE_ADDRESS_MAP(regs_map, 32);

	DECLARE_READ32_MEMBER(irq_vector_r);
	DECLARE_READ32_MEMBER(firq_vector_r);

	// can't use AM_RAM and AM_SHARE in device submaps
	DECLARE_READ32_MEMBER(aic_smr_r) { return m_aic_smr[offset]; };
	DECLARE_READ32_MEMBER(aic_svr_r) { return m_aic_svr[offset]; };
	DECLARE_WRITE32_MEMBER(aic_smr_w) { COMBINE_DATA(&m_aic_smr[offset]); };
	DECLARE_WRITE32_MEMBER(aic_svr_w) { COMBINE_DATA(&m_aic_svr[offset]); };

	DECLARE_WRITE32_MEMBER(aic_iecr_w) { /*logerror("%s: aic_iecr_w  %08x (Interrupt Enable Command Register)\n", machine().describe_context().c_str(), data);*/ COMBINE_DATA(&m_irqs_enabled); };
	DECLARE_WRITE32_MEMBER(aic_idcr_w) { /*logerror("%s: aic_idcr_w  %08x (Interrupt Disable Command Register)\n", machine().describe_context().c_str(), data);*/ };
	DECLARE_WRITE32_MEMBER(aic_iccr_w);
	DECLARE_WRITE32_MEMBER(aic_eoicr_w){ /*logerror("%s: aic_eoicr_w (End of Interrupt Command Register)\n", machine().describe_context().c_str());*/ }; // value doesn't matter

	void set_irq(int identity);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	
private:
	uint32_t m_irqs_enabled;
	uint32_t m_current_irq_vector;
	uint32_t m_current_firq_vector;

	uint32_t m_aic_smr[32];
	uint32_t m_aic_svr[32];

	devcb_write_line    m_irq_out;
};

DEFINE_DEVICE_TYPE(ARM_AIC, arm_aic_device, "arm_aic", "ARM Advanced Interrupt Controller")

DEVICE_ADDRESS_MAP_START( regs_map, 32, arm_aic_device )
	AM_RANGE(0x000, 0x07f) AM_READWRITE(aic_smr_r, aic_smr_w) // AIC_SMR[32] (AIC_SMR)	Source Mode Register
	AM_RANGE(0x080, 0x0ff) AM_READWRITE(aic_svr_r, aic_svr_w) // AIC_SVR[32] (AIC_SVR)	Source Vector Register
	AM_RANGE(0x100, 0x103) AM_READ(irq_vector_r)      // AIC_IVR	IRQ Vector Register
	AM_RANGE(0x104, 0x107) AM_READ(firq_vector_r)     // AIC_FVR	FIQ Vector Register
// 0x108	AIC_ISR	Interrupt Status Register
// 0x10C	AIC_IPR	Interrupt Pending Register
// 0x110	AIC_IMR	Interrupt Mask Register
// 0x114	AIC_CISR	Core Interrupt Status Register
	AM_RANGE(0x120, 0x123) AM_WRITE(aic_iecr_w) // 0x120	AIC_IECR	Interrupt Enable Command Register
	AM_RANGE(0x124, 0x127) AM_WRITE(aic_idcr_w) // 0x124	AIC_IDCR	Interrupt Disable Command Register
	AM_RANGE(0x128, 0x12b) AM_WRITE(aic_iccr_w) // 0x128	AIC_ICCR	Interrupt Clear Command Register
// 0x12C	AIC_ISCR	Interrupt Set Command Register
	AM_RANGE(0x130, 0x133) AM_WRITE(aic_eoicr_w) // 0x130	AIC_EOICR	End of Interrupt Command Register
// 0x134	AIC_SPU	Spurious Vector Register
// 0x138	AIC_DCR	Debug Control Register (Protect)
// 0x140	AIC_FFER	Fast Forcing Enable Register
// 0x144	AIC_FFDR	Fast Forcing Disable Register
// 0x148	AIC_FFSR	Fast Forcing Status Register
ADDRESS_MAP_END


READ32_MEMBER(arm_aic_device::irq_vector_r)
{
	return m_current_irq_vector;
}

READ32_MEMBER(arm_aic_device::firq_vector_r)
{
	return m_current_firq_vector;
}

void arm_aic_device::device_start()
{
	m_irq_out.resolve_safe();

	save_item(NAME(m_irqs_enabled));
	save_item(NAME(m_current_irq_vector));
	save_item(NAME(m_current_firq_vector));

	save_item(NAME(m_aic_smr));
	save_item(NAME(m_aic_svr));

}

void arm_aic_device::device_reset()
{
	m_irqs_enabled = 0;
	m_current_irq_vector = 0;
	m_current_firq_vector = 0;

	for(auto & elem : m_aic_smr) { elem = 0; }
	for(auto & elem : m_aic_svr) { elem = 0; }
}

void arm_aic_device::set_irq(int identity)
{
	for (int i = 0;i < 32;i++)
	{
		if (m_aic_smr[i] == identity)
		{
			if ((m_irqs_enabled >> i) & 1)
			{
				m_current_irq_vector = m_aic_svr[i];
				m_irq_out(ASSERT_LINE);
				return;
			}
		}
	}
}

WRITE32_MEMBER(arm_aic_device::aic_iccr_w)
{
	//logerror("%s: aic_iccr_w  %08x (Interrupt Clear Command Register)\n", machine().describe_context().c_str(), data);
	m_irq_out(CLEAR_LINE);
};



class pgm2_state : public driver_device
{
public:
	pgm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_lineram(*this, "lineram"),
		m_sp_zoom(*this, "sp_zoom"),
		m_mainram(*this, "mainram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_vid_regs(*this, "vid_regs"),
		m_gfxdecode2(*this, "gfxdecode2"),
		m_gfxdecode3(*this, "gfxdecode3"),
		m_arm_aic(*this, "arm_aic"),
		m_sprites_mask(*this, "sprites_mask"),
		m_sprites_colour(*this, "sprites_colour"),
		m_sp_palette(*this, "sp_palette"),
		m_bg_palette(*this, "bg_palette"),
		m_tx_palette(*this, "tx_palette")
	{ }

	DECLARE_READ32_MEMBER(unk_startup_r);
	DECLARE_WRITE32_MEMBER(fg_videoram_w);
	DECLARE_WRITE32_MEMBER(bg_videoram_w);

	DECLARE_READ32_MEMBER(orleg2_speedup_r);

	DECLARE_DRIVER_INIT(kov2nl);
	DECLARE_DRIVER_INIT(orleg2);
	DECLARE_DRIVER_INIT(ddpdojh);
	DECLARE_DRIVER_INIT(kov3);
	DECLARE_DRIVER_INIT(kov3_104);
	DECLARE_DRIVER_INIT(kov3_102);
	DECLARE_DRIVER_INIT(kov3_100);

	uint32_t screen_update_pgm2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_pgm2);
	DECLARE_WRITE_LINE_MEMBER(irq);

	INTERRUPT_GEN_MEMBER(igs_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(igs_interrupt2);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void pgm_create_dummy_internal_arm_region();
	void decrypt_kov3_module(uint32_t addrxor, uint16_t dataxor);

	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;

	std::unique_ptr<uint32_t[]>     m_spritebufferram; // buffered spriteram

	bitmap_ind16 m_sprite_bitmap;

	void draw_sprites(screen_device &screen, const rectangle &cliprect, uint32_t* spriteram);
	void copy_sprites_from_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_lineram;
	required_shared_ptr<uint32_t> m_sp_zoom;
	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_fg_videoram;
	required_shared_ptr<uint32_t> m_bg_videoram;
	required_shared_ptr<uint32_t> m_sp_videoram;
	required_shared_ptr<uint32_t> m_vid_regs;
	required_device<gfxdecode_device> m_gfxdecode2;
	required_device<gfxdecode_device> m_gfxdecode3;
	required_device<arm_aic_device> m_arm_aic;
	required_region_ptr<uint8_t> m_sprites_mask;
	required_region_ptr<uint8_t> m_sprites_colour;
	required_device<palette_device> m_sp_palette;
	required_device<palette_device> m_bg_palette;
	required_device<palette_device> m_tx_palette;
};

// checked on startup, or doesn't boot
READ32_MEMBER(pgm2_state::unk_startup_r)
{
	logerror("%s: unk_startup_r\n", machine().describe_context().c_str());
	return 0xffffffff;
}

INTERRUPT_GEN_MEMBER(pgm2_state::igs_interrupt)
{
	m_arm_aic->set_irq(0x47);
}

TIMER_DEVICE_CALLBACK_MEMBER(pgm2_state::igs_interrupt2)
{
	int scanline = param;

	if(scanline == 0)
		 m_arm_aic->set_irq(0x46);
}

static ADDRESS_MAP_START( pgm2_map, AS_PROGRAM, 32, pgm2_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM //AM_REGION("user1", 0x00000) // internal ROM

	AM_RANGE(0x02000000, 0x0200ffff) AM_RAM AM_SHARE("sram") // 'battery ram'
	
	AM_RANGE(0x03600000, 0x03600003) AM_WRITENOP
	AM_RANGE(0x03620000, 0x03620003) AM_WRITENOP
	AM_RANGE(0x03640000, 0x03640003) AM_WRITENOP
	AM_RANGE(0x03660000, 0x03660003) AM_READ_PORT("UNK1")
	AM_RANGE(0x03680000, 0x03680003) AM_READ_PORT("UNK2")
	AM_RANGE(0x036a0000, 0x036a0003) AM_WRITENOP

	AM_RANGE(0x03900000, 0x03900003) AM_READ_PORT("INPUTS0")
	AM_RANGE(0x03a00000, 0x03a00003) AM_READ_PORT("INPUTS1")

	AM_RANGE(0x10000000, 0x107fffff) AM_ROM AM_REGION("user1", 0) // external ROM
	AM_RANGE(0x20000000, 0x207fffff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0x30000000, 0x30001fff) AM_RAM AM_SHARE("sp_videoram") // spriteram ('move' ram in test mode)

	AM_RANGE(0x30020000, 0x30021fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x30040000, 0x30045fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")

	AM_RANGE(0x30060000, 0x30063fff) AM_RAM_DEVWRITE("sp_palette", palette_device, write) AM_SHARE("sp_palette")

	AM_RANGE(0x30080000, 0x30081fff) AM_RAM_DEVWRITE("bg_palette", palette_device, write) AM_SHARE("bg_palette") 

	AM_RANGE(0x300a0000, 0x300a07ff) AM_RAM_DEVWRITE("tx_palette", palette_device, write) AM_SHARE("tx_palette") 

	AM_RANGE(0x300c0000, 0x300c01ff) AM_RAM AM_SHARE("sp_zoom") // sprite zoom table?
	AM_RANGE(0x300e0000, 0x300e03bf) AM_RAM AM_SHARE("lineram") // linescroll - 0x3bf is enough bytes for 240 lines if each rowscroll value was 8 bytes, but each row is 4, so only half of this is used? or tx can do it too (unlikely, as orl2 writes 256 lines of data) maybe just bad mem check bounds on orleg2, it reports pass even if it fails the first byte!

	AM_RANGE(0x30100000, 0x301000ff) AM_RAM // unknown 

	AM_RANGE(0x30120000, 0x3012003f) AM_RAM AM_SHARE("vid_regs") // scroll etc.?

	AM_RANGE(0x40000000, 0x40000003) AM_DEVREADWRITE8("ymz774", ymz774_device, read, write, 0xffffffff)
	
	// internal to IGS036? - various other writes down here on startup too
	AM_RANGE(0xffffec00, 0xffffec5f) AM_RAM // other encryption data?
	AM_RANGE(0xfffffc00, 0xfffffcff) AM_RAM // encryption table (see code at 3950)

	AM_RANGE(0xfffff000, 0xfffff14b) AM_DEVICE("arm_aic", arm_aic_device, regs_map)

	AM_RANGE(0xfffff430, 0xfffff433) AM_WRITENOP // often
	AM_RANGE(0xfffff434, 0xfffff437) AM_WRITENOP // often

	AM_RANGE(0xfffffd28, 0xfffffd2b) AM_READNOP // often

//	AM_RANGE(0xfffffa08, 0xfffffa0b) AM_WRITE(table_done_w) // after uploading encryption? table might actually send it or enable external ROM?
	AM_RANGE(0xfffffa0c, 0xfffffa0f) AM_READ(unk_startup_r)
ADDRESS_MAP_END




static INPUT_PORTS_START( pgm2 )
// probably not inputs
	PORT_START("UNK1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

// probably not inputs
	PORT_START("UNK2")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 ) // test key p1+p2
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNUSED ) // should be test key p3+p4 but doesn't work in test mode?
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SERVICE3 ) // service key p1+p2
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE2 ) // service key p3+p4
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
	PORT_DIPNAME( 0x80000000, 0x80000000, DEF_STR( Unused ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(          0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
INPUT_PORTS_END




void pgm2_state::draw_sprites(screen_device &screen, const rectangle &cliprect, uint32_t* spriteram)
{
	m_sprite_bitmap.fill(0x8000, cliprect);

	int endoflist = -1;

//	printf("frame\n");

	for (int i = 0;i < 0x2000 / 4;i++)
	{
		if (spriteram[i] == 0x80000000)
		{
			endoflist = i;
			i = 0x2000;
		}
	}

	if (endoflist != -1)
	{
		uint16_t* dstptr_bitmap;

		for (int i = 0; i < endoflist-2; i += 4)
		{
			//printf("sprite with %08x %08x %08x %08x\n", spriteram[i + 0], spriteram[i + 1], spriteram[i + 2], spriteram[i + 3]);
		
			int x =     (spriteram[i + 0] & 0x000007ff) >> 0;
			int y =     (spriteram[i + 0] & 0x003ff800) >> 11;
			int pal =   (spriteram[i + 0] & 0x0fc00000) >> 22;
			int pri =   (spriteram[i + 0] & 0x80000000) >> 31;

			int sizex = (spriteram[i + 1] & 0x0000003f) >> 0;
			int sizey = (spriteram[i + 1] & 0x00007fc0) >> 6;
			int flipx = (spriteram[i + 1] & 0x00800000) >> 23;
			int flipy = (spriteram[i + 1] & 0x80000000) >> 31; // more of a 'reverse entire drawing' flag than y-flip, but used for that purpose
			//int zoomx = (spriteram[i + 1] & 0x001f0000) >> 16;
			//int zoomy = (spriteram[i + 1] & 0x1f000000) >> 24;

			int mask_offset = (spriteram[i + 2]<<1);
			int palette_offset = (spriteram[i + 3]);

			if (x & 0x400) x -=0x800;
			if (y & 0x400) y -=0x800;

			if (flipy)
				mask_offset -= 2;

			mask_offset &= 0x3ffffff;
			palette_offset &= 0x7ffffff;

			pal |= (pri << 6); // encode priority with the palette for manual mixing later

			for (int ydraw = 0; ydraw < sizey;ydraw++)
			{
				int realy = ydraw + y;

				for (int xdraw = 0; xdraw < sizex;xdraw++)
				{
					uint32_t maskdata = m_sprites_mask[mask_offset+0] << 24;
					maskdata |= m_sprites_mask[mask_offset+1] << 16;
					maskdata |= m_sprites_mask[mask_offset+2] << 8;
					maskdata |= m_sprites_mask[mask_offset+3] << 0;
				
					if (flipy)
					{
						mask_offset -= 4;
					}
					else if (!flipy)
					{
						mask_offset += 4;
					}
				
					
					mask_offset &= 0x3ffffff;

					for (int xchunk = 0;xchunk < 32;xchunk++)
					{
						int realx, pix;
				
						if (!flipx)
						{
							if (!flipy) realx = x + (xdraw * 32) + xchunk;
							else realx = ((x + sizex * 32) - 1) - ((xdraw * 32) + xchunk);
						}
						else
						{
							if (!flipy) realx = ((x + sizex * 32) - 1) - ((xdraw * 32) + xchunk);
							else realx = x + (xdraw * 32) + xchunk;
						}

						if (!flipy)
						{
							pix = (maskdata >> (31 - xchunk)) & 1;
						}
						else
						{
							pix = (maskdata >> xchunk) & 1;
						}

						if (pix)
						{
							if (cliprect.contains(realx, realy))
							{
								uint16_t pix = m_sprites_colour[palette_offset] & 0x3f; // there are some stray 0xff bytes in some roms, so mask

								uint16_t pendat = pix + (pal * 0x40);

								dstptr_bitmap = &m_sprite_bitmap.pix16(realy);
								dstptr_bitmap[realx] = pendat;
							}
						

							if (!flipy)
							{
								palette_offset++;
							}
							else
							{
								palette_offset--;
							}
								
							palette_offset &= 0x7ffffff;
						}					
					}
				}
			}
		}

	}
}

void pgm2_state::copy_sprites_from_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
	pri <<= 12;

	const pen_t *paldata = m_sp_palette->pens();
	uint16_t* srcptr_bitmap;
	uint32_t* dstptr_bitmap;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		srcptr_bitmap = &m_sprite_bitmap.pix16(y);
		dstptr_bitmap = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t pix = srcptr_bitmap[x];

			if (pix != 0x8000)
			{
				if ((pix&0x1000) == pri)
					dstptr_bitmap[x] = paldata[pix & 0xfff];
			}
		}

	}
}



uint32_t pgm2_state::screen_update_pgm2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly(0, (m_vid_regs[0x0/4] & 0xffff0000)>>16 );

	for (int y = 0; y < 224; y++)
	{
		uint16_t linescroll = (y & 1) ? ((m_lineram[(y >> 1)] & 0xffff0000) >> 16) : (m_lineram[(y >> 1)] & 0x0000ffff);
		m_bg_tilemap->set_scrollx((y + ((m_vid_regs[0x0 / 4] & 0xffff0000) >> 16)) & 0x3ff, ((m_vid_regs[0x0 / 4] & 0x0000ffff) >> 0) + linescroll);
	}

	const pen_t *paldata = m_bg_palette->pens();

	bitmap.fill(paldata[0], cliprect); // are there any places bg pen is showing so we know what it should be?

	draw_sprites(screen, cliprect, m_spritebufferram.get());

	copy_sprites_from_bitmap(screen, bitmap, cliprect, 1);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	copy_sprites_from_bitmap(screen, bitmap, cliprect, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE_LINE_MEMBER(pgm2_state::screen_vblank_pgm2)
{
	// rising edge
	if (state)
	{
		memcpy(m_spritebufferram.get(), m_sp_videoram, 0x2000);
	}
}

WRITE_LINE_MEMBER(pgm2_state::irq)
{
//	printf("irq\n");
	if (state == ASSERT_LINE) m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
	else m_maincpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);

}

WRITE32_MEMBER(pgm2_state::fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(pgm2_state::get_fg_tile_info)
{
	int tileno = (m_fg_videoram[tile_index] & 0x0003ffff) >> 0;
	int colour = (m_fg_videoram[tile_index] & 0x007c0000) >> 18; // 5 bits
	int flipxy = (m_fg_videoram[tile_index] & 0x01800000) >> 23;

	SET_TILE_INFO_MEMBER(0, tileno, colour, TILE_FLIPXY(flipxy));
}


WRITE32_MEMBER(pgm2_state::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(pgm2_state::get_bg_tile_info)
{
	int tileno = (m_bg_videoram[tile_index] & 0x0003ffff) >> 0;
	int colour = (m_bg_videoram[tile_index] & 0x003c0000) >> 18; // 4 bits
	int flipxy = (m_bg_videoram[tile_index] & 0x01800000) >> 23;

	SET_TILE_INFO_MEMBER(0, tileno, colour, TILE_FLIPXY(flipxy));
}

void pgm2_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode2, tilemap_get_info_delegate(FUNC(pgm2_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 96, 48); // 0x4800 bytes
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode3, tilemap_get_info_delegate(FUNC(pgm2_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 32, 32, 64, 32); // 0x2000 bytes
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_rows(32 * 32);

	m_spritebufferram = make_unique_clear<uint32_t[]>(0x2000/4);

	m_screen->register_screen_bitmap(m_sprite_bitmap);

	save_pointer(NAME(m_spritebufferram.get()), 0x2000/4);
}

void pgm2_state::machine_start()
{
}

void pgm2_state::machine_reset()
{
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tiles32x32x8_layout =
{
	32,32,
	RGN_FRAC(1,1),
	7,
	{ 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8, 24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256,
		16*256, 17*256, 18*256, 19*256, 20*256, 21*256, 22*256, 23*256, 24*256, 25*256, 26*256, 27*256, 28*256, 29*256, 30*256, 31*256
	},
	256*32
};

static GFXDECODE_START( pgm2_tx )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 0x800/4/0x10 )
GFXDECODE_END

static GFXDECODE_START( pgm2_bg )
	GFXDECODE_ENTRY( "bgtile", 0, tiles32x32x8_layout, 0, 0x2000/4/0x80 )
GFXDECODE_END


void pgm2_state::pgm_create_dummy_internal_arm_region()
{
	uint16_t *temp16 = (uint16_t *)memregion("maincpu")->base();
	int i;
	for (i = 0;i < 0x4000 / 2;i += 2)
	{
		temp16[i] = 0xFFFE;
		temp16[i + 1] = 0xEAFF;

	}
	int base = 0;
	int addr = 0x10000000;

	// just do a jump to 0x10000000 because that looks possibly correct.
	// we probably should be setting up stack and other things too, but we
	// don't really know that info yet.

	temp16[(base) / 2] = 0x0004; base += 2;
	temp16[(base) / 2] = 0xe59f; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;
	temp16[(base) / 2] = 0xe590; base += 2;
	temp16[(base) / 2] = 0xff10; base += 2;
	temp16[(base) / 2] = 0xe12f; base += 2;
	temp16[(base) / 2] = 0x0010; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;

	temp16[(base) / 2] = addr & 0xffff; base += 2;
	temp16[(base) / 2] = (addr >> 16) & 0xffff; base += 2;

	/* debug code to find jumps to the internal ROM and put jumps back to where we came from in the internal ROM space */

	uint16_t *gamerom = (uint16_t *)memregion("user1")->base();
	int gameromlen = memregion("user1")->bytes() / 2;

	for (int i = 0;i < gameromlen - 3;i++)
	{
		uint16_t val1 = gamerom[i];
		uint16_t val2 = gamerom[i + 1];

		if ((val1 == 0xF004) && (val2 == 0xe51f))
		{
			uint16_t val3 = gamerom[i + 2];
			uint16_t val4 = gamerom[i + 3];
			uint32_t jump = (val4 << 16) | val3;

			if (jump < 0x10000000) // jumps to internal ROM
			{
				logerror("internal ROM jump found at %08x (jump is %08x)", i * 2, jump);
				if (jump & 1)
				{
					logerror(" (To THUMB)");
					jump &= ~1;
					// put a BX R14 there
					temp16[(jump / 2)] = 0x4770;
				}
				else
				{
					// put a BX R14 there
					temp16[(jump / 2)] = 0xFF1E;
					temp16[(jump / 2) + 1] = 0xE12F;
				}
				logerror("\n");
			}
			else if (jump < 0x20000000) // jumps to RAM
			{
				logerror("RAM jump found at %08x (jump is %08x)", i * 2, jump);
				if (jump & 1) logerror(" (To THUMB)");
				logerror("\n");
			}
			else // anything else is to ROM
			{
				logerror("ROM jump found at %08x (jump is %08x)", i * 2, jump);
				if (jump & 1) logerror(" (To THUMB)");
				logerror("\n");
			}

		}
	}
}

static MACHINE_CONFIG_START( pgm2 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", IGS036, 100000000) // ?? ARM based CPU, has internal ROM.
	MCFG_CPU_PROGRAM_MAP(pgm2_map)

	MCFG_CPU_VBLANK_INT_DRIVER("screen", pgm2_state,  igs_interrupt)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pgm2_state, igs_interrupt2, "screen", 0, 1)


	MCFG_ARM_AIC_ADD("arm_aic")
	MCFG_IRQ_LINE_CB(WRITELINE(pgm2_state, irq))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 448-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(pgm2_state, screen_update_pgm2)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(pgm2_state, screen_vblank_pgm2))

	MCFG_GFXDECODE_ADD("gfxdecode2", "tx_palette", pgm2_tx)
	
	MCFG_GFXDECODE_ADD("gfxdecode3", "bg_palette", pgm2_bg)

	MCFG_PALETTE_ADD("sp_palette", 0x4000/4) // sprites
	MCFG_PALETTE_FORMAT(XRGB)
	
	MCFG_PALETTE_ADD("tx_palette", 0x800/4) // text
	MCFG_PALETTE_FORMAT(XRGB)

	MCFG_PALETTE_ADD("bg_palette", 0x2000/4) // bg
	MCFG_PALETTE_FORMAT(XRGB)

	MCFG_NVRAM_ADD_0FILL("sram")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_YMZ774_ADD("ymz774", 16384000) // is clock correct ?
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/* using macros for the video / sound roms because the locations never change between sets, and
   we're going to have a LOT of clones to cover all the internal rom regions and external rom revision
   combinations, so it keeps things readable */

#define ORLEG2_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF ) \
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
	ROM_LOAD32_WORD( "ig-a_cgl.u18",     0x00000000, 0x2000000, BAD_DUMP CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) ) \
	ROM_LOAD32_WORD( "ig-a_cgh.u26",     0x00000002, 0x2000000, BAD_DUMP CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) ) \
	\
	ROM_REGION( 0x1000000, "ymz774", ROMREGION_ERASEFF ) /* ymz770 */ \
	ROM_LOAD16_WORD_SWAP( "ig-a_sp.u2",  0x00000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) ) \
	\
	ROM_REGION( 0x10000, "sram", 0 ) \
	ROM_LOAD( "xyj2_nvram",            0x00000000, 0x10000, CRC(ccccc71c) SHA1(585b5ccbf89dd28d8532da785d7c8af12f31c6d6) )

#define ORLEG2_PROGRAM_104 \
	ROM_REGION( 0x800000, "user1", 0 ) \
	ROM_LOAD( "xyj2_v104cn.u7",          0x00000000, 0x0800000, CRC(7c24a4f5) SHA1(3cd9f9264ef2aad0869afdf096e88eb8d74b2570) )

#define ORLEG2_PROGRAM_103 \
	ROM_REGION( 0x800000, "user1", 0 ) \
	ROM_LOAD( "xyj2_v103cn.u7",  0x000000, 0x800000, CRC(21c1fae8) SHA1(36eeb7a5e8dc8ee7c834f3ff1173c28cf6c2f1a3) )

#define ORLEG2_PROGRAM_101 \
	ROM_REGION( 0x800000, "user1", 0 ) \
	ROM_LOAD( "xyj2_v101cn.u7",  0x000000, 0x800000, CRC(45805b53) SHA1(f2a8399c821b75fadc53e914f6f318707e70787c) )

#define ORLEG2_INTERNAL_CHINA \
	ROM_REGION( 0x04000, "maincpu", 0 ) \
	/* offset 3cb8 of the internal rom controls the region, however only China is dumped at the moment and it appears Overseas and Japan at least need different external ROMs or will crash later in game */ \
	ROM_LOAD( "xyj2_igs036_china.rom", 0x00000000, 0x0004000, CRC(bcce7641) SHA1(c3b5cf6e9f6eae09b6785314777a52b34c3c7657) )

ROM_START( orleg2 )
	ORLEG2_INTERNAL_CHINA
	ORLEG2_PROGRAM_104
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_103 )
	ORLEG2_INTERNAL_CHINA
	ORLEG2_PROGRAM_103
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

ROM_START( orleg2_101 )
	ORLEG2_INTERNAL_CHINA
	ORLEG2_PROGRAM_101
	ORLEG2_VIDEO_SOUND_ROMS
ROM_END

#define KOV2NL_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF ) \
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
	ROM_REGION( 0x2000000, "ymz774", ROMREGION_ERASEFF ) /* ymz770 */ \
	ROM_LOAD16_WORD_SWAP( "ig-a3_sp.u37",            0x00000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) )


ROM_START( kov2nl )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "gsyx_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "gsyx_v302cn.u7",          0x00000000, 0x0800000, CRC(b19cf540) SHA1(25da5804bbfd7ef2cdf5cc5aabaa803d18b98929) )

	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov2nlo )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "gsyx_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "gsyx_v301cn.u7",  0x000000, 0x800000, CRC(c4595c2c) SHA1(09e379556ef76f81a63664f46d3f1415b315f384) )

	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov2nloa )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "gsyx_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "kov2nl_gsyx_v300tw.u7",  0x000000, 0x800000, CRC(08da7552) SHA1(303b97d7694405474c8133a259303ccb49db48b1) )

	KOV2NL_VIDEO_SOUND_ROMS
ROM_END

#define DDPDOJH_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF ) \
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
	ROM_REGION( 0x1000000, "ymz774", ROMREGION_ERASEFF ) /* ymz770 */ \
	ROM_LOAD16_WORD_SWAP( "ddpdoj_wave0.u12",        0x00000000, 0x1000000, CRC(2b71a324) SHA1(f69076cc561f40ca564d804bc7bd455066f8d77c) )



ROM_START( ddpdojh )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "ddpdoj_igs036.rom",       0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "ddpdoj_v201cn.u4",        0x00000000, 0x0200000, CRC(89e4b760) SHA1(9fad1309da31d12a413731b416a8bbfdb304ed9e) )

	DDPDOJH_VIDEO_SOUND_ROMS
ROM_END

/*
   The Kov3 Program rom is a module consisting of a NOR flash and a FPGA, this provides an extra layer of encryption on top of the usual
   that is only unlocked when the correct sequence is recieved from the ARM MCU (IGS036)

   Newer gambling games use the same modules.
*/

#define KOV3_VIDEO_SOUND_ROMS \
	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF ) \
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
	ROM_REGION( 0x4000000, "ymz774", ROMREGION_ERASEFF ) /* ymz770 */ \
	ROM_LOAD16_WORD_SWAP( "kov3_wave0.u13",              0x00000000, 0x4000000, CRC(aa639152) SHA1(2314c6bd05524525a31a2a4668a36a938b924ba4) )


ROM_START( kov3 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "kov3_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "kov3_v104cn_raw.bin",         0x00000000, 0x0800000, CRC(1b5cbd24) SHA1(6471d4842a08f404420dea2bd1c8b88798c80fd5) )

	KOV3_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov3_102 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "kov3_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "kov3_v102cn_raw.bin",         0x00000000, 0x0800000, CRC(61d0dabd) SHA1(959b22ef4e342ca39c2386549ac7274f9d580ab8) )

	KOV3_VIDEO_SOUND_ROMS
ROM_END

ROM_START( kov3_100 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "kov3_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "kov3_v100cn_raw.bin",         0x00000000, 0x0800000, CRC(93bca924) SHA1(ecaf2c4676eb3d9f5e4fdbd9388be41e51afa0e4) )

	KOV3_VIDEO_SOUND_ROMS
ROM_END


static void iga_u16_decode(uint16_t *rom, int len, int ixor)
{
	int i;

	for (i = 1; i < len / 2; i+=2)
	{
		uint16_t x = ixor;

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
		rom[i] = BITSWAP16(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static void iga_u12_decode(uint16_t* rom, int len, int ixor)
{
	int i;

	for (i = 0; i < len / 2; i+=2)
	{
		uint16_t x = ixor;

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
		rom[i] = BITSWAP16(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static void sprite_colour_decode(uint16_t* rom, int len)
{
	int i;

	for (i = 0; i < len / 2; i++)
	{
		rom[i] = BITSWAP16(rom[i], 15, 14, /* unused - 6bpp */
			                       13, 12, 11,
			                       5, 4, 3,
			                       7, 6, /* unused - 6bpp */
			                       10, 9, 8,  
			                       2, 1, 0  );
	}
}

READ32_MEMBER(pgm2_state::orleg2_speedup_r)
{
	int pc = space.device().safe_pc();
	if ((pc == 0x1002faec) || (pc == 0x1002f9b8))
	{
		if ((m_mainram[0x20114 / 4] == 0x00) && (m_mainram[0x20118 / 4] == 0x00))
			space.device().execute().spin_until_interrupt();
	}
	/*else
	{
		printf("pc is %08x\n", pc);
	}*/

	return m_mainram[0x20114 / 4];
}



DRIVER_INIT_MEMBER(pgm2_state,orleg2)
{
	uint16_t *src = (uint16_t *)memregion("sprites_mask")->base();

	iga_u12_decode(src, 0x2000000, 0x4761);
	iga_u16_decode(src, 0x2000000, 0xc79f);

	src = (uint16_t *)memregion("sprites_colour")->base();
	sprite_colour_decode(src, 0x4000000);

	igs036_decryptor decrypter(orleg2_key);
	decrypter.decrypter_rom(memregion("user1"));

	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0x20020114, 0x20020117, read32_delegate(FUNC(pgm2_state::orleg2_speedup_r),this));

	/* HACK!
	   patch out an ingame assert that ends up being triggered after the 5 element / fire chariot boss due to an invalid value in R3 
	   todo: why does this happen? ARM core bug? is patching out the assert actually safe? game continues as normal like this, but there could be memory corruption.
	*/
	uint16_t* rom;
	rom = (uint16_t*)memregion("user1")->base();
	int hackaddress = -1;

	if (rom[0x12620 / 2] == 0xd301) // 104 / 103
	{
		hackaddress = 0x12620;
	}
	else if (rom[0x1257C / 2] == 0xd301) // 101
	{
		hackaddress = 0x1257C;
	}

	if (hackaddress != -1)
	{
		rom[(hackaddress + 2) / 2] = 0x0009;
		rom[(hackaddress + 4) / 2] = 0x0009;
		rom = (uint16_t*)memregion("maincpu")->base(); // BEQ -> BNE for checksum
		rom[0x39f2 / 2] = 0x1a00;

		// set a breakpoint on 0x10000000 + hackaddress to see when it triggers
	}
	else
	{
		fatalerror("no patch for this set \n");
	}

}

DRIVER_INIT_MEMBER(pgm2_state,kov2nl)
{
	uint16_t *src = (uint16_t *)memregion("sprites_mask")->base();

	iga_u12_decode(src, 0x2000000, 0xa193);
	iga_u16_decode(src, 0x2000000, 0xb780);

	igs036_decryptor decrypter(kov2_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region();
}

DRIVER_INIT_MEMBER(pgm2_state,ddpdojh)
{
	uint16_t *src = (uint16_t *)memregion("sprites_mask")->base();

	iga_u12_decode(src, 0x800000, 0x1e96);
	iga_u16_decode(src, 0x800000, 0x869c);

	igs036_decryptor decrypter(ddpdoj_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region();
}

DRIVER_INIT_MEMBER(pgm2_state,kov3)
{
	uint16_t *src = (uint16_t *)memregion("sprites_mask")->base();

	iga_u12_decode(src, 0x2000000, 0x956d);
	iga_u16_decode(src, 0x2000000, 0x3d17);

	igs036_decryptor decrypter(kov3_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region();
}

void pgm2_state::decrypt_kov3_module(uint32_t addrxor, uint16_t dataxor)
{
	uint16_t *src = (uint16_t *)memregion("user1")->base();
	uint32_t size = memregion("user1")->bytes();

	std::vector<uint16_t> buffer(size/2);

	for (int i = 0; i < size/2; i++)
		buffer[i] = src[i^addrxor]^dataxor;

	memcpy(src, &buffer[0], size);
}

DRIVER_INIT_MEMBER(pgm2_state, kov3_104)
{
	decrypt_kov3_module(0x18ec71, 0xb89d);
	DRIVER_INIT_CALL(kov3);
}

DRIVER_INIT_MEMBER(pgm2_state, kov3_102)
{
	decrypt_kov3_module(0x021d37, 0x81d0);
	DRIVER_INIT_CALL(kov3);
}

DRIVER_INIT_MEMBER(pgm2_state, kov3_100)
{
	decrypt_kov3_module(0x3e8aa8, 0xc530);
	DRIVER_INIT_CALL(kov3);
}


/* PGM2 */

// Oriental Legend 2 - should be a V102 and V100 too
GAME( 2007, orleg2,       0,         pgm2,    pgm2, pgm2_state,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V104, China)", MACHINE_NOT_WORKING )
GAME( 2007, orleg2_103,   orleg2,    pgm2,    pgm2, pgm2_state,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V103, China)", MACHINE_NOT_WORKING )
GAME( 2007, orleg2_101,   orleg2,    pgm2,    pgm2, pgm2_state,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V101, China)", MACHINE_NOT_WORKING )

// Knights of Valour 2 New Legend 
GAME( 2008, kov2nl,       0,         pgm2,    pgm2, pgm2_state,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V302, China)", MACHINE_NOT_WORKING )
GAME( 2008, kov2nlo,      kov2nl,    pgm2,    pgm2, pgm2_state,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V301, China)", MACHINE_NOT_WORKING )
GAME( 2008, kov2nloa,     kov2nl,    pgm2,    pgm2, pgm2_state,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V300, Taiwan)", MACHINE_NOT_WORKING )

// Dodonpachi Daioujou Tamashii - should be a V200 too
GAME( 2010, ddpdojh,      0,    pgm2,    pgm2, pgm2_state,     ddpdojh,    ROT270, "IGS", "Dodonpachi Daioujou Tamashii (V201, China)", MACHINE_NOT_WORKING )

// Knights of Valour 3 - should be a V103 and V101 too
GAME( 2011, kov3,         0,    pgm2,    pgm2, pgm2_state,     kov3_104,   ROT0, "IGS", "Knights of Valour 3 (V104, China)", MACHINE_NOT_WORKING )
GAME( 2011, kov3_102,     kov3, pgm2,    pgm2, pgm2_state,     kov3_102,   ROT0, "IGS", "Knights of Valour 3 (V102, China)", MACHINE_NOT_WORKING )
GAME( 2011, kov3_100,     kov3, pgm2,    pgm2, pgm2_state,     kov3_100,   ROT0, "IGS", "Knights of Valour 3 (V100, China)", MACHINE_NOT_WORKING )

// The King of Fighters '98 - Ultimate Match - Hero

// Jigsaw World Arena

// Puzzle of Ocha / Ochainu No Pazuru


