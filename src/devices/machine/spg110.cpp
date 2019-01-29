// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    SunPlus SPG110-series SoC peripheral emulation

    It is possible this shares some video features with spg110 and
    can be made a derived device

    0032xx looks like it could be the same as 003d00 on spg2xx
	but the video seems quite different?


**********************************************************************/

#include "emu.h"
#include "spg110.h"

DEFINE_DEVICE_TYPE(SPG110, spg110_device, "spg110", "SPG110 System-on-a-Chip")

spg110_device::spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("spg110", ENDIANNESS_BIG, 16, 32, 0, address_map_constructor(FUNC(spg110_device::map_video), this)),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_bg_videoram(*this, "bg_videoram"),
	m_fg_videoram(*this, "fg_videoram"),
	m_bg_attrram(*this, "bg_attrram"),
	m_fg_attrram(*this, "fg_attrram")
{
}

WRITE16_MEMBER(spg110_device::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(spg110_device::fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(spg110_device::bg_attrram_w)
{
	COMBINE_DATA(&m_bg_attrram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(spg110_device::fg_attrram_w)
{
	COMBINE_DATA(&m_fg_attrram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset/2);
}


/* correct, 4bpp gfxs */
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout charlayout6 =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{ 0,1,2,3,4,5 },
	{ STEP8(0,6) },
	{ STEP8(0,6*8) },
	8*8*6
};

static const gfx_layout char16layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4, 8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4 },
	{ STEP16(0,4*16) },
	16*16*4
};

static const gfx_layout char32layout =
{
	32,32,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP32(0,4) },
	{ STEP32(0,4*32) },
	32*32*4
};



static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( ":maincpu", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( ":maincpu", 0, char16layout, 0, 16 )
	GFXDECODE_ENTRY( ":maincpu", 0, char32layout, 0, 16 )
	GFXDECODE_ENTRY( ":maincpu", 0, charlayout6, 0, 16 ) // correct for lots of the tiles inc. startup text
GFXDECODE_END



MACHINE_CONFIG_START(spg110_device::device_add_mconfig)
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::RGB_565, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::IRGB_4444, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::RGBI_4444, 0x100);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);
MACHINE_CONFIG_END


device_memory_interface::space_config_vector spg110_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

spg110_device::spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_device(mconfig, SPG110, tag, owner, clock)
{
}

// irq source or similar?
READ16_MEMBER(spg110_device::spg110_2063_r)
{
	// checks for bits 0x20 and 0x08 in the IRQ function (all IRQs point to the same place)
	return 0x0008;
}

WRITE16_MEMBER(spg110_device::spg110_2063_w)
{
	// writes 0x28, probably clears the IRQ / IRQ sources? 0x63 is the same offset for this in spg2xx but bits used seem to be different
	m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, CLEAR_LINE);
}

READ16_MEMBER(spg110_device::datasegment_r)
{
	uint16_t val = m_cpu->get_ds();
	return val;
}

WRITE16_MEMBER(spg110_device::datasegment_w)
{
	m_cpu->set_ds(data & 0x3f);
}

WRITE16_MEMBER(spg110_device::spg110_3221_w)
{
	/* first write on startup? */
}

WRITE16_MEMBER(spg110_device::spg110_3223_w) { }
WRITE16_MEMBER(spg110_device::spg110_3225_w) { }

WRITE16_MEMBER(spg110_device::spg110_2010_w) { }
WRITE16_MEMBER(spg110_device::spg110_2011_w) { }
WRITE16_MEMBER(spg110_device::spg110_2012_w) { }
WRITE16_MEMBER(spg110_device::spg110_2013_w) { }
WRITE16_MEMBER(spg110_device::spg110_2014_w) { }
WRITE16_MEMBER(spg110_device::spg110_2015_w) { }
WRITE16_MEMBER(spg110_device::spg110_2016_w) { }
WRITE16_MEMBER(spg110_device::spg110_2017_w) { }
WRITE16_MEMBER(spg110_device::spg110_2018_w) { }
WRITE16_MEMBER(spg110_device::spg110_2019_w) { }
WRITE16_MEMBER(spg110_device::spg110_201a_w) { }
WRITE16_MEMBER(spg110_device::spg110_201b_w) { }
WRITE16_MEMBER(spg110_device::spg110_201c_w) { }
WRITE16_MEMBER(spg110_device::spg110_2020_w) { }
WRITE16_MEMBER(spg110_device::spg110_2042_w) { }
WRITE16_MEMBER(spg110_device::spg110_2031_w) { }
WRITE16_MEMBER(spg110_device::spg110_2032_w) { }
WRITE16_MEMBER(spg110_device::spg110_2033_w) { }
WRITE16_MEMBER(spg110_device::spg110_2034_w) { }
WRITE16_MEMBER(spg110_device::spg110_2035_w) { }
WRITE16_MEMBER(spg110_device::spg110_2036_w) { }
WRITE16_MEMBER(spg110_device::spg110_2039_w) { }
WRITE16_MEMBER(spg110_device::spg110_2037_w) { }
WRITE16_MEMBER(spg110_device::spg110_203c_w) { }
WRITE16_MEMBER(spg110_device::spg110_203d_w) { }
WRITE16_MEMBER(spg110_device::spg110_2045_w) { }


WRITE16_MEMBER(spg110_device::spg110_2028_w) { }
WRITE16_MEMBER(spg110_device::spg110_2029_w) { }

READ16_MEMBER(spg110_device::spg110_2028_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_2029_r) { return 0x0000; }


WRITE16_MEMBER(spg110_device::spg110_2050_w) { }
WRITE16_MEMBER(spg110_device::spg110_2051_w) { }
WRITE16_MEMBER(spg110_device::spg110_2052_w) { }
WRITE16_MEMBER(spg110_device::spg110_2053_w) { }
WRITE16_MEMBER(spg110_device::spg110_2054_w) { }
WRITE16_MEMBER(spg110_device::spg110_2055_w) { }
WRITE16_MEMBER(spg110_device::spg110_2056_w) { }
WRITE16_MEMBER(spg110_device::spg110_2057_w) { }
WRITE16_MEMBER(spg110_device::spg110_2058_w) { }
WRITE16_MEMBER(spg110_device::spg110_2059_w) { }
WRITE16_MEMBER(spg110_device::spg110_205a_w) { }
WRITE16_MEMBER(spg110_device::spg110_205b_w) { }
WRITE16_MEMBER(spg110_device::spg110_205c_w) { }
WRITE16_MEMBER(spg110_device::spg110_205d_w) { }
WRITE16_MEMBER(spg110_device::spg110_205e_w) { }
WRITE16_MEMBER(spg110_device::spg110_205f_w) { }

WRITE16_MEMBER(spg110_device::spg110_2061_w) { COMBINE_DATA(&m_2061_outer); }
WRITE16_MEMBER(spg110_device::spg110_2064_w) { COMBINE_DATA(&m_2064_outer); }
WRITE16_MEMBER(spg110_device::spg110_2067_w) { COMBINE_DATA(&m_2067_outer); }
WRITE16_MEMBER(spg110_device::spg110_2068_w) { COMBINE_DATA(&m_2068_outer); }

WRITE16_MEMBER(spg110_device::spg110_2060_w) { COMBINE_DATA(&m_2060_inner); }
WRITE16_MEMBER(spg110_device::spg110_2066_w) { COMBINE_DATA(&m_2066_inner); }


WRITE16_MEMBER(spg110_device::spg110_2062_w)
{
//	int length = (data - 1) & 0xff;
	int length = data & 0x1fff;

	// this is presumably a counter that underflows to 0x1fff, because that's what the wait loop waits for?
	logerror("%s: trigger (%04x) with values (written outer) %04x %04x %04x %04x | (written inner) %04x (ram source?) %04x\n", machine().describe_context(), data, m_2061_outer, m_2064_outer, m_2067_outer, m_2068_outer, m_2060_inner, m_2066_inner);

	int source = m_2066_inner;
	int dest = m_2060_inner;

	// maybe
	/*
	if (!(m_2068_outer & 1))
	{
		length = length * 4;
	}
	*/
	//logerror("[ ");
	for (int i = 0; i <= length; i++)
	{
		if (m_2068_outer & 1)
		{
			address_space &mem = m_cpu->space(AS_PROGRAM);
			uint16_t val = mem.read_word(m_2066_inner + i);

			this->space(0).write_word(dest * 2, val, 0xffff);
		}
		else // guess, it needs to clear layers somehow
		{
			this->space(0).write_word(dest * 2, 0x0000, 0xffff);
		}

		source++;
		dest++;

		//	logerror("%04x, ", val);
	}
	//	logerror(" ]\n");
}

READ16_MEMBER(spg110_device::spg110_2062_r)
{
	return 0x1fff; // DMA related?
}


READ16_MEMBER(spg110_device::spg110_2013_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_2019_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_2037_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_2042_r) { return 0x0000; }

WRITE16_MEMBER(spg110_device::spg110_3200_w) { }
WRITE16_MEMBER(spg110_device::spg110_3201_w) { }
WRITE16_MEMBER(spg110_device::spg110_3203_w) { }
WRITE16_MEMBER(spg110_device::spg110_3204_w) { }
WRITE16_MEMBER(spg110_device::spg110_3206_w) { }
WRITE16_MEMBER(spg110_device::spg110_3208_w) { }
WRITE16_MEMBER(spg110_device::spg110_3209_w) { }

READ16_MEMBER(spg110_device::spg110_3201_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_3225_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_322c_r) { return 0x0000; }

WRITE16_MEMBER(spg110_device::spg110_3100_w) { }
WRITE16_MEMBER(spg110_device::spg110_3101_w) { }
WRITE16_MEMBER(spg110_device::spg110_3102_w) { }
WRITE16_MEMBER(spg110_device::spg110_3104_w) { }
WRITE16_MEMBER(spg110_device::spg110_3105_w) { }
WRITE16_MEMBER(spg110_device::spg110_3106_w) { }
WRITE16_MEMBER(spg110_device::spg110_3107_w) { }
WRITE16_MEMBER(spg110_device::spg110_3108_w) { }
WRITE16_MEMBER(spg110_device::spg110_3109_w) { }
WRITE16_MEMBER(spg110_device::spg110_310b_w) { }
WRITE16_MEMBER(spg110_device::spg110_310c_w) { }
WRITE16_MEMBER(spg110_device::spg110_310d_w) { }

READ16_MEMBER(spg110_device::spg110_310f_r) { return 0x0000; }

void spg110_device::map(address_map &map)
{
	map(0x000000, 0x000fff).ram();
	
	// vregs are at 2000?
	map(0x002010, 0x002010).w(FUNC(spg110_device::spg110_2010_w));
	map(0x002011, 0x002011).w(FUNC(spg110_device::spg110_2011_w)); // possible scroll register?
	map(0x002012, 0x002012).w(FUNC(spg110_device::spg110_2012_w));
	map(0x002013, 0x002013).rw(FUNC(spg110_device::spg110_2013_r),FUNC(spg110_device::spg110_2013_w));
	map(0x002014, 0x002014).w(FUNC(spg110_device::spg110_2014_w));
	map(0x002015, 0x002015).w(FUNC(spg110_device::spg110_2015_w));
	map(0x002016, 0x002016).w(FUNC(spg110_device::spg110_2016_w));
	map(0x002017, 0x002017).w(FUNC(spg110_device::spg110_2017_w));
	map(0x002018, 0x002018).w(FUNC(spg110_device::spg110_2018_w));
	map(0x002019, 0x002019).rw(FUNC(spg110_device::spg110_2019_r), FUNC(spg110_device::spg110_2019_w));
	map(0x00201a, 0x00201a).w(FUNC(spg110_device::spg110_201a_w));
	map(0x00201b, 0x00201b).w(FUNC(spg110_device::spg110_201b_w));
	map(0x00201c, 0x00201c).w(FUNC(spg110_device::spg110_201c_w)); // possible scroll register?
	
	map(0x002020, 0x002020).w(FUNC(spg110_device::spg110_2020_w));

	map(0x002028, 0x002028).rw(FUNC(spg110_device::spg110_2028_r), FUNC(spg110_device::spg110_2028_w));
	map(0x002029, 0x002029).rw(FUNC(spg110_device::spg110_2029_r), FUNC(spg110_device::spg110_2029_w));

	map(0x002031, 0x002031).w(FUNC(spg110_device::spg110_2031_w)); // sometimes 14a?
	map(0x002032, 0x002032).w(FUNC(spg110_device::spg110_2032_w)); // always 14a?
	map(0x002033, 0x002033).w(FUNC(spg110_device::spg110_2033_w));
	map(0x002034, 0x002034).w(FUNC(spg110_device::spg110_2034_w));
	map(0x002035, 0x002035).w(FUNC(spg110_device::spg110_2035_w));
	map(0x002036, 0x002036).w(FUNC(spg110_device::spg110_2036_w)); // possible scroll register?
	map(0x002037, 0x002037).rw(FUNC(spg110_device::spg110_2037_r), FUNC(spg110_device::spg110_2037_w));

	map(0x002039, 0x002039).w(FUNC(spg110_device::spg110_2039_w));

	map(0x00203c, 0x00203c).w(FUNC(spg110_device::spg110_203c_w));

	map(0x00203d, 0x00203d).w(FUNC(spg110_device::spg110_203d_w)); // possible scroll register?

	map(0x002042, 0x002042).rw(FUNC(spg110_device::spg110_2042_r),FUNC(spg110_device::spg110_2042_w));

	map(0x002045, 0x002045).w(FUNC(spg110_device::spg110_2045_w));

	// seems to be 16 entries for.. something?
	map(0x002050, 0x002050).w(FUNC(spg110_device::spg110_2050_w));
	map(0x002051, 0x002051).w(FUNC(spg110_device::spg110_2051_w));
	map(0x002052, 0x002052).w(FUNC(spg110_device::spg110_2052_w));
	map(0x002053, 0x002053).w(FUNC(spg110_device::spg110_2053_w));
	map(0x002054, 0x002054).w(FUNC(spg110_device::spg110_2054_w));
	map(0x002055, 0x002055).w(FUNC(spg110_device::spg110_2055_w));
	map(0x002056, 0x002056).w(FUNC(spg110_device::spg110_2056_w));
	map(0x002057, 0x002057).w(FUNC(spg110_device::spg110_2057_w));
	map(0x002058, 0x002058).w(FUNC(spg110_device::spg110_2058_w));
	map(0x002059, 0x002059).w(FUNC(spg110_device::spg110_2059_w));
	map(0x00205a, 0x00205a).w(FUNC(spg110_device::spg110_205a_w));
	map(0x00205b, 0x00205b).w(FUNC(spg110_device::spg110_205b_w));
	map(0x00205c, 0x00205c).w(FUNC(spg110_device::spg110_205c_w));
	map(0x00205d, 0x00205d).w(FUNC(spg110_device::spg110_205d_w));
	map(0x00205e, 0x00205e).w(FUNC(spg110_device::spg110_205e_w));
	map(0x00205f, 0x00205f).w(FUNC(spg110_device::spg110_205f_w));

	// everything (dma? and interrupt flag?!)
	map(0x002060, 0x002060).w(FUNC(spg110_device::spg110_2060_w));
	map(0x002061, 0x002061).w(FUNC(spg110_device::spg110_2061_w));
	map(0x002062, 0x002062).rw(FUNC(spg110_device::spg110_2062_r),FUNC(spg110_device::spg110_2062_w));
	map(0x002063, 0x002063).rw(FUNC(spg110_device::spg110_2063_r),FUNC(spg110_device::spg110_2063_w)); // this looks like interrupt stuff and is checked in the irq like an irq source, but why in the middle of what otherwise look like some kind of DMA?
	map(0x002064, 0x002064).w(FUNC(spg110_device::spg110_2064_w));
	map(0x002066, 0x002066).w(FUNC(spg110_device::spg110_2066_w));
	map(0x002067, 0x002067).w(FUNC(spg110_device::spg110_2067_w));
	map(0x002068, 0x002068).w(FUNC(spg110_device::spg110_2068_w));

	map(0x002200, 0x0022ff).ram(); // looks like per-pen brightness or similar? strange because palette isn't memory mapped here
	
	map(0x003000, 0x00307f).ram(); // sound registers? seems to be 8 long entries, only uses up to 0x7f?
	map(0x003080, 0x0030ff).ram(); 

	map(0x003100, 0x003100).w(FUNC(spg110_device::spg110_3100_w));
	map(0x003101, 0x003101).w(FUNC(spg110_device::spg110_3101_w));
	map(0x003102, 0x003102).w(FUNC(spg110_device::spg110_3102_w));

	map(0x003104, 0x003104).w(FUNC(spg110_device::spg110_3104_w));
	map(0x003105, 0x003105).w(FUNC(spg110_device::spg110_3105_w));
	map(0x003106, 0x003106).w(FUNC(spg110_device::spg110_3106_w));
	map(0x003107, 0x003107).w(FUNC(spg110_device::spg110_3107_w));
	map(0x003108, 0x003108).w(FUNC(spg110_device::spg110_3108_w));
	map(0x003109, 0x003109).w(FUNC(spg110_device::spg110_3109_w));

	map(0x00310b, 0x00310b).w(FUNC(spg110_device::spg110_310b_w));
	map(0x00310c, 0x00310c).w(FUNC(spg110_device::spg110_310c_w));
	map(0x00310d, 0x00310d).w(FUNC(spg110_device::spg110_310d_w));

	map(0x00310f, 0x00310f).r(FUNC(spg110_device::spg110_310f_r));

	// 0032xx looks like it could be the same as 003d00 on spg2xx
	map(0x003200, 0x003200).w(FUNC(spg110_device::spg110_3200_w));

	map(0x003201, 0x003201).rw(FUNC(spg110_device::spg110_3201_r),FUNC(spg110_device::spg110_3201_w));

	map(0x003203, 0x003203).w(FUNC(spg110_device::spg110_3203_w));
	map(0x003204, 0x003204).w(FUNC(spg110_device::spg110_3204_w));

	map(0x003206, 0x003206).w(FUNC(spg110_device::spg110_3206_w));

	map(0x003208, 0x003208).w(FUNC(spg110_device::spg110_3208_w));
	map(0x003209, 0x003209).w(FUNC(spg110_device::spg110_3209_w));

	map(0x003221, 0x003221).w(FUNC(spg110_device::spg110_3221_w));
	map(0x003223, 0x003223).w(FUNC(spg110_device::spg110_3223_w));
	map(0x003225, 0x003225).rw(FUNC(spg110_device::spg110_3225_r),FUNC(spg110_device::spg110_3225_w));
	map(0x00322c, 0x00322c).r(FUNC(spg110_device::spg110_322c_r));

	map(0x00322f, 0x00322f).rw(FUNC(spg110_device::datasegment_r),FUNC(spg110_device::datasegment_w));
}

// this seems to be a different, non-cpu mapped space only accessible via the DMA?
void spg110_device::map_video(address_map &map)
{
	// are these addresses hardcoded, or can they move (in which case tilemap system isn't really suitable)
	map(0x00000, 0x00fff).ram().w(FUNC(spg110_device::bg_videoram_w)).share("bg_videoram");
	map(0x01000, 0x017ff).ram().w(FUNC(spg110_device::bg_attrram_w)).share("bg_attrram");
	map(0x01800, 0x027ff).ram().w(FUNC(spg110_device::fg_videoram_w)).share("fg_videoram");
	map(0x02800, 0x02fff).ram().w(FUNC(spg110_device::fg_attrram_w)).share("fg_attrram");

	map(0x04000, 0x04fff).ram(); // seems to be 3 blocks, almost certainly spritelist

	map(0x08000, 0x081ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // probably? format unknown tho
}


/*
TIMER_CALLBACK_MEMBER(spg110_device::test_timer)
{
    //
}
*/

TILE_GET_INFO_MEMBER(spg110_device::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(3, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER(spg110_device::get_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}

void spg110_device::device_start()
{
//  m_test_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg110_device::test_timer), this));
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(spg110_device::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(spg110_device::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

void spg110_device::device_reset()
{
}


uint32_t spg110_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE_LINE_MEMBER(spg110_device::vblank)
{
	if (!state)
	{
		m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, ASSERT_LINE);
	//  m_test_timer->adjust(attotime::from_usec(100), 0);
	}

	return;
}
