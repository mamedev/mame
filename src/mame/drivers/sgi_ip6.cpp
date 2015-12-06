// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/****************************************************************************

    drivers/sgi_ip6.c
    SGI 4D/PI IP6 family skeleton driver

    by Ryan Holtz

        0x1fc00000 - 0x1fc3ffff     ROM

    Interrupts:
        R2000:
            NYI

****************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"


struct ip6_regs_t
{
	UINT16 unknown_half_0;
	UINT8 unknown_byte_0;
	UINT8 unknown_byte_1;
};

class sgi_ip6_state : public driver_device
{
public:
	sgi_ip6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	ip6_regs_t m_ip6_regs;
	DECLARE_READ32_MEMBER(ip6_unk1_r);
	DECLARE_WRITE32_MEMBER(ip6_unk1_w);
	DECLARE_READ32_MEMBER(ip6_unk2_r);
	DECLARE_WRITE32_MEMBER(ip6_unk2_w);
	DECLARE_READ32_MEMBER(ip6_unk3_r);
	DECLARE_WRITE32_MEMBER(ip6_unk3_w);
	DECLARE_DRIVER_INIT(sgi_ip6);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_sgi_ip6(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sgi_ip6_vbl);
	inline void ATTR_PRINTF(3,4) verboselog( int n_level, const char *s_fmt, ... );
	required_device<cpu_device> m_maincpu;
};


#define VERBOSE_LEVEL ( 0 )

#define ENABLE_VERBOSE_LOG (1)

inline void ATTR_PRINTF(3,4) sgi_ip6_state::verboselog( int n_level, const char *s_fmt, ... )
{
#if ENABLE_VERBOSE_LOG
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%08x: %s", machine().device("maincpu")->safe_pc(), buf);
	}
#endif
}

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

void sgi_ip6_state::video_start()
{
}

UINT32 sgi_ip6_state::screen_update_sgi_ip6(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/


READ32_MEMBER(sgi_ip6_state::ip6_unk1_r)
{
	UINT32 ret = 0;
	switch(offset)
	{
		case 0x0000/4:
			if(ACCESSING_BITS_16_31)
			{
				verboselog(0, "ip6_unk1_r: Unknown address: %08x & %08x\n", 0x1f880000 + (offset << 2), mem_mask );
			}
			if(ACCESSING_BITS_0_15)
			{
				verboselog(0, "ip6_unk1_r: Unknown Halfword 0: %08x & %08x\n", m_ip6_regs.unknown_half_0, mem_mask );
				ret |= m_ip6_regs.unknown_half_0;
			}
			break;
		default:
			verboselog(0, "ip6_unk1_r: Unknown address: %08x & %08x\n", 0x1f880000 + (offset << 2), mem_mask );
			break;
	}
	return ret;
}

WRITE32_MEMBER(sgi_ip6_state::ip6_unk1_w)
{
	switch(offset)
	{
		case 0x0000/4:
			if(ACCESSING_BITS_16_31)
			{
				verboselog(0, "ip6_unk1_w: Unknown address: %08x = %08x & %08x\n", 0x1f880000 + (offset << 2), data, mem_mask );
			}
			if(ACCESSING_BITS_0_15)
			{
				verboselog(0, "ip6_unk1_w: Unknown Halfword 0 = %04x & %04x\n", data & 0x0000ffff, mem_mask & 0x0000ffff );
				m_ip6_regs.unknown_half_0 = data & 0x0000ffff;
			}
			break;
		default:
			verboselog(0, "ip6_unk1_w: Unknown address: %08x = %08x & %08x\n", 0x1f880000 + (offset << 2), data, mem_mask );
			break;
	}
}

READ32_MEMBER(sgi_ip6_state::ip6_unk2_r)
{
	UINT32 ret = 0;
	switch(offset)
	{
		case 0x0000/4:
			if(!ACCESSING_BITS_24_31)
			{
				verboselog(0, "ip6_unk2_r: Unknown address: %08x & %08x\n", 0x1f880000 + (offset << 2), mem_mask );
			}
			if(ACCESSING_BITS_24_31)
			{
				verboselog(0, "ip6_unk2_r: Unknown Byte 0 = %02x & %02x\n", m_ip6_regs.unknown_byte_0, mem_mask >> 24 );
				ret |= m_ip6_regs.unknown_byte_0 << 24;
			}
			break;
		default:
			verboselog(0, "ip6_unk2_r: Unknown address: %08x & %08x\n", 0x1f880000 + (offset << 2), mem_mask );
			break;
	}
	return ret;
}

WRITE32_MEMBER(sgi_ip6_state::ip6_unk2_w)
{
	switch(offset)
	{
		case 0x0000/4:
			if(!ACCESSING_BITS_24_31)
			{
				verboselog(0, "ip6_unk2_w: Unknown address: %08x = %08x & %08x\n", 0x1f880000 + (offset << 2), data, mem_mask );
			}
			if(ACCESSING_BITS_24_31)
			{
				verboselog(0, "ip6_unk2_w: Unknown Byte 0 = %02x & %02x\n", data >> 24, mem_mask >> 24 );
				m_ip6_regs.unknown_byte_0 = (data & 0xff000000) >> 24;
			}
			break;
		default:
			verboselog(0, "ip6_unk2_w: Unknown address: %08x = %08x & %08x\n", 0x1f880000 + (offset << 2), data, mem_mask );
			break;
	}
}

READ32_MEMBER(sgi_ip6_state::ip6_unk3_r)
{
	UINT32 ret = 0;
	if(ACCESSING_BITS_16_23)
	{
		verboselog(0, "ip6_unk3_r: Unknown Byte 1: %02x & %02x\n", m_ip6_regs.unknown_byte_1, (mem_mask >> 16) & 0x000000ff);
		ret |= m_ip6_regs.unknown_byte_1 << 16;
	}
	else
	{
		verboselog(0, "ip6_unk3_r: Unknown address: %08x & %08x\n", 0x1fb00000 + (offset << 2), mem_mask );
	}
	return ret;
}

WRITE32_MEMBER(sgi_ip6_state::ip6_unk3_w)
{
	verboselog(0, "ip6_unk3_w: Unknown address: %08x = %08x & %08x\n", 0x1fb00000 + (offset << 2), data, mem_mask );
}

INTERRUPT_GEN_MEMBER(sgi_ip6_state::sgi_ip6_vbl)
{
}

void sgi_ip6_state::machine_start()
{
}

void sgi_ip6_state::machine_reset()
{
	m_ip6_regs.unknown_byte_0 = 0x80;
	m_ip6_regs.unknown_byte_1 = 0x80;
	m_ip6_regs.unknown_half_0 = 0;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( sgi_ip6_map, AS_PROGRAM, 32, sgi_ip6_state )
	AM_RANGE( 0x1f880000, 0x1f880003 ) AM_READWRITE(ip6_unk1_r, ip6_unk1_w)
	AM_RANGE( 0x1fb00000, 0x1fb00003 ) AM_READWRITE(ip6_unk3_r, ip6_unk3_w)
	AM_RANGE( 0x1fbc004c, 0x1fbc004f ) AM_READWRITE(ip6_unk2_r, ip6_unk2_w)
	AM_RANGE( 0x1fc00000, 0x1fc3ffff ) AM_ROM AM_REGION( "user1", 0 )
ADDRESS_MAP_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static MACHINE_CONFIG_START( sgi_ip6, sgi_ip6_state )
	MCFG_CPU_ADD( "maincpu", R3041, 20000000 ) // FIXME: Should be R2000
	MCFG_R3000_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_CPU_CONFIG( config )
	MCFG_CPU_PROGRAM_MAP( sgi_ip6_map )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sgi_ip6_state,  sgi_ip6_vbl)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(sgi_ip6_state, screen_update_sgi_ip6)


MACHINE_CONFIG_END

static INPUT_PORTS_START( sgi_ip6 )
	PORT_START("UNUSED") // unused IN0
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

DRIVER_INIT_MEMBER(sgi_ip6_state,sgi_ip6)
{
}

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( sgi_ip6 )
	ROM_REGION32_BE( 0x40000, "user1", 0 )
	ROM_LOAD( "4d202031.bin", 0x000000, 0x040000, CRC(065a290a) SHA1(6f5738e79643f94901e6efe3612468d14177f65b) )
ROM_END

/*     YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT     COMPANY   FULLNAME */
COMP( 1988, sgi_ip6,  0,        0,        sgi_ip6,  sgi_ip6, sgi_ip6_state,  sgi_ip6,  "Silicon Graphics Inc", "4D/PI (R2000, 20MHz)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
