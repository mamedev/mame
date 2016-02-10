// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "machine/megacd.h"
#include "machine/nvram.h"
#include "megacd.lh"


const device_type SEGA_SEGACD_US = &device_creator<sega_segacd_us_device>;
const device_type SEGA_SEGACD_JAPAN = &device_creator<sega_segacd_japan_device>;
const device_type SEGA_SEGACD_EUROPE = &device_creator<sega_segacd_europe_device>;


/* Callback when the genesis enters interrupt code */
IRQ_CALLBACK_MEMBER(sega_segacd_device::segacd_sub_int_callback)
{
	if (irqline==2)
	{
		// clear this bit
		m_a12000_halt_reset_reg &= ~0x0100;
		m_scdcpu->set_input_line(2, CLEAR_LINE);
	}

	return (0x60+irqline*4)/4; // vector address
}


TIMER_DEVICE_CALLBACK_MEMBER( sega_segacd_device::irq3_timer_callback )
{
	CHECK_SCD_LV3_INTERRUPT
	m_irq3_timer->adjust(SEGACD_IRQ3_TIMER_SPEED);
}


TIMER_DEVICE_CALLBACK_MEMBER( sega_segacd_device::stamp_timer_callback )
{
	//printf("stamp_timer_callback\n");

	CHECK_SCD_LV1_INTERRUPT

	segacd_conversion_active = 0;

	// this ends up as 0 after processing (soniccd bonus stage)
	segacd_imagebuffer_vdot_size = 0;
}


static ADDRESS_MAP_START( segacd_map, AS_PROGRAM, 16, sega_segacd_device )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_SHARE("prgram")
	AM_RANGE(0x080000, 0x0bffff) AM_READWRITE(segacd_sub_dataram_part1_r, segacd_sub_dataram_part1_w) AM_SHARE("dataram")
	AM_RANGE(0x0c0000, 0x0dffff) AM_READWRITE(segacd_sub_dataram_part2_r, segacd_sub_dataram_part2_w) //AM_SHARE("dataram2")

	AM_RANGE(0xfe0000, 0xfe3fff) AM_READWRITE8(backupram_r, backupram_w, 0x00ff) // backup RAM, odd bytes only!

	AM_RANGE(0xff0000, 0xff001f) AM_DEVWRITE8("rfsnd", rf5c68_device, rf5c68_w, 0x00ff)  // PCM, RF5C164
	AM_RANGE(0xff0020, 0xff003f) AM_DEVREAD8("rfsnd", rf5c68_device, rf5c68_r, 0x00ff)
	AM_RANGE(0xff2000, 0xff3fff) AM_DEVREADWRITE8("rfsnd", rf5c68_device, rf5c68_mem_r, rf5c68_mem_w,0x00ff)  // PCM, RF5C164

	AM_RANGE(0xff8000 ,0xff8001) AM_READWRITE(segacd_sub_led_ready_r, segacd_sub_led_ready_w)
	AM_RANGE(0xff8002 ,0xff8003) AM_READWRITE(segacd_sub_memory_mode_r, segacd_sub_memory_mode_w)

	AM_RANGE(0xff8004 ,0xff8005) AM_DEVREADWRITE("tempcdc",lc89510_temp_device, segacd_cdc_mode_address_r, segacd_cdc_mode_address_w)
	AM_RANGE(0xff8006 ,0xff8007) AM_DEVREADWRITE("tempcdc",lc89510_temp_device,segacd_cdc_data_r, segacd_cdc_data_w)
	AM_RANGE(0xff8008, 0xff8009) AM_DEVREAD("tempcdc",lc89510_temp_device, cdc_data_sub_r)
	AM_RANGE(0xff800a, 0xff800b) AM_READWRITE(segacd_dmaaddr_r,segacd_dmaaddr_w) // DMA Address (not CDC, used in conjunction with)
	AM_RANGE(0xff800c, 0xff800d) AM_READWRITE(segacd_stopwatch_timer_r, segacd_stopwatch_timer_w)// Stopwatch timer
	AM_RANGE(0xff800e ,0xff800f) AM_READWRITE(segacd_comms_flags_r, segacd_comms_flags_subcpu_w)
	AM_RANGE(0xff8010 ,0xff801f) AM_READWRITE(segacd_comms_sub_part1_r, segacd_comms_sub_part1_w)
	AM_RANGE(0xff8020 ,0xff802f) AM_READWRITE(segacd_comms_sub_part2_r, segacd_comms_sub_part2_w)
	AM_RANGE(0xff8030, 0xff8031) AM_READWRITE(segacd_irq3timer_r, segacd_irq3timer_w) // Timer W/INT3
	AM_RANGE(0xff8032, 0xff8033) AM_DEVREADWRITE("tempcdc",lc89510_temp_device,segacd_irq_mask_r,segacd_irq_mask_w)
	AM_RANGE(0xff8034, 0xff8035) AM_DEVREADWRITE("tempcdc",lc89510_temp_device,segacd_cdfader_r,segacd_cdfader_w) // CD Fader
	AM_RANGE(0xff8036, 0xff8037) AM_DEVREADWRITE("tempcdc",lc89510_temp_device,segacd_cdd_ctrl_r,segacd_cdd_ctrl_w)
	AM_RANGE(0xff8038, 0xff8041) AM_DEVREAD8("tempcdc",lc89510_temp_device,segacd_cdd_rx_r,0xffff)
	AM_RANGE(0xff8042, 0xff804b) AM_DEVWRITE8("tempcdc",lc89510_temp_device,segacd_cdd_tx_w,0xffff)
	AM_RANGE(0xff804c, 0xff804d) AM_READWRITE8(font_color_r, font_color_w, 0x00ff)
	AM_RANGE(0xff804e, 0xff804f) AM_RAM AM_SHARE("font_bits")
	AM_RANGE(0xff8050, 0xff8057) AM_READ(font_converted_r)
	AM_RANGE(0xff8058, 0xff8059) AM_READWRITE(segacd_stampsize_r, segacd_stampsize_w) // Stamp size
	AM_RANGE(0xff805a, 0xff805b) AM_READWRITE(segacd_stampmap_base_address_r, segacd_stampmap_base_address_w) // Stamp map base address
	AM_RANGE(0xff805c, 0xff805d) AM_READWRITE(segacd_imagebuffer_vcell_size_r, segacd_imagebuffer_vcell_size_w)// Image buffer V cell size
	AM_RANGE(0xff805e, 0xff805f) AM_READWRITE(segacd_imagebuffer_start_address_r, segacd_imagebuffer_start_address_w) // Image buffer start address
	AM_RANGE(0xff8060, 0xff8061) AM_READWRITE(segacd_imagebuffer_offset_r, segacd_imagebuffer_offset_w)
	AM_RANGE(0xff8062, 0xff8063) AM_READWRITE(segacd_imagebuffer_hdot_size_r, segacd_imagebuffer_hdot_size_w) // Image buffer H dot size
	AM_RANGE(0xff8064, 0xff8065) AM_READWRITE(segacd_imagebuffer_vdot_size_r, segacd_imagebuffer_vdot_size_w ) // Image buffer V dot size
	AM_RANGE(0xff8066, 0xff8067) AM_WRITE(segacd_trace_vector_base_address_w)// Trace vector base address
//  AM_RANGE(0xff8068, 0xff8069) // Subcode address

//  AM_RANGE(0xff8100, 0xff817f) // Subcode buffer area
//  AM_RANGE(0xff8180, 0xff81ff) // mirror of subcode buffer area

ADDRESS_MAP_END


// the tiles in RAM are 8x8 tiles
// they are referenced in the cell look-up map as either 16x16 or 32x32 tiles (made of 4 / 16 8x8 tiles)

#define SEGACD_BYTES_PER_TILE16 (128)
#define SEGACD_BYTES_PER_TILE32 (512)

#define SEGACD_NUM_TILES16 (0x40000/SEGACD_BYTES_PER_TILE16)
#define SEGACD_NUM_TILES32 (0x40000/SEGACD_BYTES_PER_TILE32)

#define _16x16_SEQUENCE_1  { STEP8(0, 4), STEP8(512, 4) },
#define _16x16_SEQUENCE_1_FLIP  { STEP8(512+28, -4), STEP8(28, -4) },

#define _16x16_SEQUENCE_2  { STEP16(0, 32) },
#define _16x16_SEQUENCE_2_FLIP  { STEP16(15*32, -32) },


#define _16x16_START \
{ \
	16,16, \
	SEGACD_NUM_TILES16, \
	4, \
	{ 0,1,2,3 },
#define _16x16_END \
		8*128 \
};
#define _32x32_START \
{ \
	32,32, \
	SEGACD_NUM_TILES32, \
	4, \
	{ 0,1,2,3 },

#define _32x32_END \
	8*512 \
};

#define _32x32_SEQUENCE_1 { STEP8(0, 4), STEP8(1024, 4), STEP8(2048, 4), STEP8(3072, 4) },
#define _32x32_SEQUENCE_1_FLIP { STEP8(3072+28, -4), STEP8(2048+28, -4), STEP8(1024+28, -4), STEP8(28, -4) },

#define _32x32_SEQUENCE_2 { STEP32(0, 32) },
#define _32x32_SEQUENCE_2_FLIP { STEP32(31*32, -32) },


/* 16x16 decodes */
static const gfx_layout sega_16x16_r00_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1_FLIP
_16x16_END

static const gfx_layout sega_16x16_r10_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r00_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r10_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1_FLIP
_16x16_END

/* 32x32 decodes */
static const gfx_layout sega_32x32_r00_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1_FLIP
_32x32_END

static const gfx_layout sega_32x32_r10_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r00_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r10_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1_FLIP
_32x32_END

static GFXDECODE_START( segacd )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r00_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r01_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r10_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r11_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r00_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r11_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r10_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_16x16_r01_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r00_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r01_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r10_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r11_f0_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r00_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r11_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r10_f1_layout, 0, 0 )
	GFXDECODE_DEVICE_RAM( "dataram", 0, sega_32x32_r01_f1_layout, 0, 0 )
GFXDECODE_END


static MACHINE_CONFIG_FRAGMENT( segacd_fragment )

	MCFG_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MCFG_CPU_PROGRAM_MAP(segacd_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, sega_segacd_device,segacd_sub_int_callback)

	MCFG_DEVICE_ADD("cdc", LC89510, 0) // cd controller

	// temporary until things are cleaned up
	MCFG_DEVICE_ADD("tempcdc", LC89510_TEMP, 0) // cd controller
	MCFG_SEGACD_HACK_SET_CDC_DO_DMA( sega_segacd_device, SegaCD_CDC_Do_DMA ) // hack

	MCFG_TIMER_ADD_NONE("sw_timer") //stopwatch timer
	MCFG_TIMER_DRIVER_ADD("stamp_timer", sega_segacd_device, stamp_timer_callback)
	MCFG_TIMER_DRIVER_ADD("irq3_timer", sega_segacd_device, irq3_timer_callback)
	MCFG_TIMER_DRIVER_ADD("dma_timer", sega_segacd_device, dma_timer_callback)

	MCFG_DEFAULT_LAYOUT( layout_megacd )

	MCFG_RF5C68_ADD("rfsnd", SEGACD_CLOCK) // RF5C164!
	MCFG_SOUND_ROUTE( 0, ":lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, ":rspeaker", 0.50 )

	MCFG_NVRAM_ADD_0FILL("backupram")

	MCFG_QUANTUM_PERFECT_CPU("segacd_68k") // perfect sync to the fastest cpu
MACHINE_CONFIG_END



machine_config_constructor sega_segacd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( segacd_fragment );
}


sega_segacd_device::sega_segacd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_gfx_interface(mconfig, *this, GFXDECODE_NAME( segacd )),
		m_scdcpu(*this, "segacd_68k"),
		m_rfsnd(*this, "rfsnd"),
		m_lc89510_temp(*this, "tempcdc"),
		m_stopwatch_timer(*this, "sw_timer"),
		m_stamp_timer(*this, "stamp_timer"),
		m_irq3_timer(*this, "irq3_timer"),
		m_dma_timer(*this, "dma_timer"),
		m_prgram(*this, "prgram"),
		m_dataram(*this, "dataram"),
		m_font_bits(*this, "font_bits")
{
}

sega_segacd_us_device::sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_segacd_device(mconfig, SEGA_SEGACD_US, "sega_segacd_us", tag, owner, clock, "sega_segacd_us", __FILE__)
{
}

sega_segacd_japan_device::sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_segacd_device(mconfig, SEGA_SEGACD_JAPAN, "sega_segacd_japan", tag, owner, clock, "sega_segacd_japan", __FILE__)
{
}

sega_segacd_europe_device::sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_segacd_device(mconfig, SEGA_SEGACD_EUROPE, "sega_segacd_europe", tag, owner, clock, "sega_segacd_europe", __FILE__)
{
}


inline void sega_segacd_device::write_pixel(UINT8 pix, int pixeloffset)
{
	int shift = 12-(4*(pixeloffset&0x3));
	UINT16 datamask = (0x000f) << shift;

	int offset = pixeloffset>>3;
	if (pixeloffset&4) offset++;

	offset &=0x1ffff;

	switch (segacd_memory_priority_mode)
	{
		case 0x00: // normal write, just write the data
			m_dataram[offset] = m_dataram[offset] &~ datamask;
			m_dataram[offset] |= pix << shift;
			break;

		case 0x01: // underwrite, only write if the existing data is 0
			if ((m_dataram[offset]&datamask) == 0x0000)
			{
				m_dataram[offset] = m_dataram[offset] &~ datamask;
				m_dataram[offset] |= pix << shift;
			}
			break;

		case 0x02: // overwrite, only write non-zero data
			if (pix)
			{
				m_dataram[offset] = m_dataram[offset] &~ datamask;
				m_dataram[offset] |= pix << shift;
			}
			break;

		default:
		case 0x03:
			pix = machine().rand() & 0x000f;
			m_dataram[offset] = m_dataram[offset] &~ datamask;
			m_dataram[offset] |= pix << shift;
			break;

	}
}

// 1meg / 2meg swap is interleaved swap, not half/half of the ram?
// Wily Beamish and Citizen X appear to rely on this
// however, it breaks the megacdj bios (megacd2j still works!)
//  (maybe that's a timing issue instead?)
UINT16 sega_segacd_device::segacd_1meg_mode_word_read(int offset, UINT16 mem_mask)
{
	offset *= 2;

	if ((offset&0x20000))
		offset +=1;

	offset &=0x1ffff;

	return m_dataram[offset];
}


void sega_segacd_device::segacd_1meg_mode_word_write(int offset, UINT16 data, UINT16 mem_mask, int use_pm)
{
	offset *= 2;

	if ((offset&0x20000))
		offset +=1;

	offset &=0x1ffff;

	if (use_pm)
	{
		// priority mode can apply when writing with the double up buffer mode
		// Jaguar XJ220 relies on this
		switch (segacd_memory_priority_mode)
		{
			case 0x00: // normal write, just write the data
				COMBINE_DATA(&m_dataram[offset]);
				break;

			case 0x01: // underwrite, only write if the existing data is 0
				if (ACCESSING_BITS_8_15)
				{
					if ((m_dataram[offset]&0xf000) == 0x0000) m_dataram[offset] |= (data)&0xf000;
					if ((m_dataram[offset]&0x0f00) == 0x0000) m_dataram[offset] |= (data)&0x0f00;
				}
				if (ACCESSING_BITS_0_7)
				{
					if ((m_dataram[offset]&0x00f0) == 0x0000) m_dataram[offset] |= (data)&0x00f0;
					if ((m_dataram[offset]&0x000f) == 0x0000) m_dataram[offset] |= (data)&0x000f;
				}
				break;

			case 0x02: // overwrite, only write non-zero data
				if (ACCESSING_BITS_8_15)
				{
					if ((data)&0xf000) m_dataram[offset] = (m_dataram[offset] & 0x0fff) | ((data)&0xf000);
					if ((data)&0x0f00) m_dataram[offset] = (m_dataram[offset] & 0xf0ff) | ((data)&0x0f00);
				}
				if (ACCESSING_BITS_0_7)
				{
					if ((data)&0x00f0) m_dataram[offset] = (m_dataram[offset] & 0xff0f) | ((data)&0x00f0);
					if ((data)&0x000f) m_dataram[offset] = (m_dataram[offset] & 0xfff0) | ((data)&0x000f);
				}
				break;

			default:
			case 0x03: // invalid?
				COMBINE_DATA(&m_dataram[offset]);
				break;

		}
	}
	else
	{
		COMBINE_DATA(&m_dataram[offset]);
	}
}




WRITE16_MEMBER( sega_segacd_device::scd_a12000_halt_reset_w )
{
	UINT16 old_halt = m_a12000_halt_reset_reg;

	COMBINE_DATA(&m_a12000_halt_reset_reg);

	if (ACCESSING_BITS_0_7)
	{
		// reset line
		if (m_a12000_halt_reset_reg & 0x0001)
		{
			m_scdcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			if (!(old_halt&0x0001)) printf("clear reset slave\n");
		}
		else
		{
			m_scdcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			if ((old_halt&0x0001)) printf("assert reset slave\n");
		}

		// request BUS
		if (m_a12000_halt_reset_reg & 0x0002)
		{
			m_scdcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			if (!(old_halt&0x0002)) printf("halt slave\n");
		}
		else
		{
			m_scdcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			if ((old_halt&0x0002)) printf("resume slave\n");
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		if (m_a12000_halt_reset_reg & 0x0100)
		{
			CHECK_SCD_LV2_INTERRUPT
		}

		if (m_a12000_halt_reset_reg & 0x8000)
		{
			// not writable.. but can read irq mask here?
			//printf("m_a12000_halt_reset_reg & 0x8000 set\n"); // irq2 mask?
		}


	}
}

READ16_MEMBER( sega_segacd_device::scd_a12000_halt_reset_r )
{
	return m_a12000_halt_reset_reg;
}


/********************************************************************************
 MEMORY MODE CONTROL
  - main / sub sides differ!
********************************************************************************/

//
// we might need a delay on the segacd_maincpu_has_ram_access registers, as they actually indicate requests being made
// so probably don't change instantly...
//


READ16_MEMBER( sega_segacd_device::scd_a12002_memory_mode_r )
{
	int temp = scd_rammode;
	int temp2 = 0;

	temp2 |= (scd_mode_dmna_ret_flags>>(temp*4))&0x7;

	return (segacd_ram_writeprotect_bits << 8) |
			(segacd_4meg_prgbank << 6) |
			temp2;

}


/* I'm still not 100% clear how this works, the sources I have are a bit vague,
   it might still be incorrect in both modes

  for a simple way to swap blocks of ram between cpus this is stupidly convoluted

 */

// DMNA = Decleration Mainram No Access (bit 0)
// RET = Return access (bit 1)


WRITE8_MEMBER( sega_segacd_device::scd_a12002_memory_mode_w_8_15 )
{
	if (data & 0xff00)
	{
		printf("write protect bits set %02x\n", data);
	}

	segacd_ram_writeprotect_bits = data;
}


WRITE8_MEMBER( sega_segacd_device::scd_a12002_memory_mode_w_0_7 )
{
	//printf("scd_a12002_memory_mode_w_0_7 %04x\n",data);

	segacd_4meg_prgbank = (data&0x00c0)>>6;

	if (scd_rammode&0x2)
	{ // ==0x2 (1 meg mode)
		if (!(data&2)) // check DMNA bit
		{
			scd_mode_dmna_ret_flags |= 0x2200;
		}
	}
	else // == 0x0 (2 meg mode)
	{
		if (data&2) // check DMNA bit
		{
			scd_rammode = 1;
		}
	}
}


WRITE16_MEMBER( sega_segacd_device::scd_a12002_memory_mode_w )
{
	if (ACCESSING_BITS_8_15)
		scd_a12002_memory_mode_w_8_15(space, 0, data>>8, mem_mask>>8);

	if (ACCESSING_BITS_0_7)
		scd_a12002_memory_mode_w_0_7(space, 0, data&0xff, mem_mask&0xff);
}




READ16_MEMBER( sega_segacd_device::segacd_sub_memory_mode_r )
{
	int temp = scd_rammode;
	int temp2 = 0;

	temp2 |= (scd_mode_dmna_ret_flags>>(temp*4))&0x7;

	return (segacd_ram_writeprotect_bits << 8) |
			(segacd_memory_priority_mode << 3) |
			temp2;
}


WRITE8_MEMBER( sega_segacd_device::segacd_sub_memory_mode_w_8_15 )
{
	/* setting write protect bits from sub-cpu has no effect? */
}



WRITE8_MEMBER( sega_segacd_device::segacd_sub_memory_mode_w_0_7 )
{
	segacd_memory_priority_mode = (data&0x0018)>>3;

	// If the mode bit is 0 then we're requesting a change to
	// 2Meg mode?

	//printf("segacd_sub_memory_mode_w_0_7 %04x\n",data);

	if (!(data&4)) // check ram mode bit
	{   // == 0x0 - 2 meg mode
		scd_mode_dmna_ret_flags &= 0xddff;

		if (data&1) // check RET
		{
			// If RET is set and the Mode bit in the write is set to 2M then we want to change to 2M mode
			// If we're already in 2M mode it has no effect
			scd_rammode = 0;

		}
		else
		{
			// == 0x4 - 1 meg mode

			int temp = scd_rammode;
			if (temp&2) // check ram mode
			{ // == 0x2 - 1 meg mode
				scd_mode_dmna_ret_flags &= 0xffde;
				scd_rammode = temp &1;
			}
		}
	}
	else
	{   // == 0x4 - 1 meg mode
		data &=1;
		int temp = data;
		int scd_rammode_old = scd_rammode;
		data |=2;

		temp ^= scd_rammode_old;
		scd_rammode = data;

		if (scd_rammode_old & 2)
		{ // == 0x2 - already in 1 meg mode
			if (temp & 1) // ret bit
			{
				scd_mode_dmna_ret_flags &= 0xddff;
			}
		}
		else
		{ // == 0x0 - currently in 2 meg mode
			scd_mode_dmna_ret_flags &= 0xddff;
		}
	}
}

WRITE16_MEMBER( sega_segacd_device::segacd_sub_memory_mode_w )
{
	//printf("segacd_sub_memory_mode_w %04x %04x\n", data, mem_mask);


	if (ACCESSING_BITS_8_15)
		segacd_sub_memory_mode_w_8_15(space, 0, data>>8, mem_mask>>8);

	if (ACCESSING_BITS_0_7)
		segacd_sub_memory_mode_w_0_7(space, 0, data&0xff, mem_mask&0xff);
}


/********************************************************************************
 END MEMORY MODE CONTROL
********************************************************************************/

/********************************************************************************
 COMMUNICATION FLAGS
  - main / sub sides differ in which bits are write only
********************************************************************************/


READ16_MEMBER( sega_segacd_device::segacd_comms_flags_r )
{
	return segacd_comms_flags;
}

WRITE16_MEMBER( sega_segacd_device::segacd_comms_flags_subcpu_w )
{
	if (ACCESSING_BITS_8_15) // Dragon's Lair
	{
		segacd_comms_flags = (segacd_comms_flags & 0xff00) | ((data >> 8) & 0x00ff);
	}

	// flashback needs low bits to take priority in word writes
	if (ACCESSING_BITS_0_7)
	{
		segacd_comms_flags = (segacd_comms_flags & 0xff00) | (data & 0x00ff);
	}
}

WRITE16_MEMBER( sega_segacd_device::segacd_comms_flags_maincpu_w )
{
	if (ACCESSING_BITS_8_15)
	{
		segacd_comms_flags = (segacd_comms_flags & 0x00ff) | (data & 0xff00);
	}

	// flashback needs low bits to take priority in word writes
	if (ACCESSING_BITS_0_7)
	{
		segacd_comms_flags = (segacd_comms_flags & 0x00ff) | ((data << 8) & 0xff00);
	}
}

READ16_MEMBER( sega_segacd_device::scd_4m_prgbank_ram_r )
{
	UINT16 realoffset = ((segacd_4meg_prgbank * 0x20000)/2) + offset;
	return m_prgram[realoffset];

}

WRITE16_MEMBER( sega_segacd_device::scd_4m_prgbank_ram_w )
{
	UINT16 realoffset = ((segacd_4meg_prgbank * 0x20000)/2) + offset;

	// todo:
	// check for write protection? (or does that only apply to writes on the SubCPU side?

	COMBINE_DATA(&m_prgram[realoffset]);

}



READ16_MEMBER( sega_segacd_device::segacd_comms_main_part1_r )
{
	return segacd_comms_part1[offset];
}

WRITE16_MEMBER( sega_segacd_device::segacd_comms_main_part1_w )
{
	COMBINE_DATA(&segacd_comms_part1[offset]);
}

READ16_MEMBER( sega_segacd_device::segacd_comms_main_part2_r )
{
	return segacd_comms_part2[offset];
}

WRITE16_MEMBER( sega_segacd_device::segacd_comms_main_part2_w )
{
	printf("Sega CD main CPU attempting to write to read only comms regs\n");
}


READ16_MEMBER( sega_segacd_device::segacd_comms_sub_part1_r )
{
	return segacd_comms_part1[offset];
}

WRITE16_MEMBER( sega_segacd_device::segacd_comms_sub_part1_w )
{
	printf("Sega CD sub CPU attempting to write to read only comms regs\n");
}

READ16_MEMBER( sega_segacd_device::segacd_comms_sub_part2_r )
{
	return segacd_comms_part2[offset];
}

WRITE16_MEMBER( sega_segacd_device::segacd_comms_sub_part2_w )
{
	COMBINE_DATA(&segacd_comms_part2[offset]);
}


READ16_MEMBER( sega_segacd_device::segacd_main_dataram_part1_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (!(scd_rammode&1))
		{
			//printf("segacd_main_dataram_part1_r in mode 0 %08x %04x\n", offset*2, m_dataram[offset]);

			return m_dataram[offset];

		}
		else
		{
			//printf("Illegal: segacd_main_dataram_part1_r in mode 0 without permission\n");
			return 0xffff;
		}

	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		if (offset<0x20000/2)
		{
			// wordram accees
			//printf("Unspported: segacd_main_dataram_part1_r (word RAM) in mode 1\n");

			// ret bit set by sub cpu determines which half of WorkRAM we have access to?
			if (scd_rammode&1)
			{
				return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
			}
			else
			{
				return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
			}

		}
		else
		{
			// converts data stored in bitmap format (in dataram) to be read out as tiles (for dma->vram purposes)
			// used by Heart of the Alien

			if(offset<0x30000/2)        /* 0x20000 - 0x2ffff */ // 512x256 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,8,7,6,5,4,3,2,1,14,13,12,11,10,9,0);
			else if(offset<0x38000/2)   /* 0x30000 - 0x37fff */  // 512x128 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,7,6,5,4,3,2,1,13,12,11,10,9,8,0);
			else if(offset<0x3c000/2)   /* 0x38000 - 0x3bfff */  // 512x64 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,13,6,5,4,3,2,1,12,11,10,9,8,7,0);
			else  /* 0x3c000 - 0x3dfff and 0x3e000 - 0x3ffff */  // 512x32 bitmap (x2) -> tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,13,12,5,4,3,2,1,11,10,9,8,7,6,0);

			offset &=0xffff;
			// HOTA cares about this
			if (!(scd_rammode&1))
			{
				return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
			}
			else
			{
				return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
			}
		}
	}

	return 0x0000;
}

WRITE16_MEMBER( sega_segacd_device::segacd_main_dataram_part1_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (!(scd_rammode&1))
		{
			COMBINE_DATA(&m_dataram[offset]);
			segacd_mark_tiles_dirty(offset);
		}
		else
		{
			//printf("Illegal: segacd_main_dataram_part1_w in mode 0 without permission\n");
		}

	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		if (offset<0x20000/2)
		{
			//printf("Unspported: segacd_main_dataram_part1_w (word RAM) in mode 1\n");
			// wordram accees

			// ret bit set by sub cpu determines which half of WorkRAM we have access to?
			if (scd_rammode&1)
			{
				segacd_1meg_mode_word_write(offset+0x20000/2, data, mem_mask, 0);
			}
			else
			{
				segacd_1meg_mode_word_write(offset+0x00000/2, data, mem_mask, 0);
			}
		}
		else
		{
		//  printf("Unspported: segacd_main_dataram_part1_w (Cell rearranged RAM) in mode 1 (illega?)\n"); // is this legal??
		}
	}
}

READ16_MEMBER( sega_segacd_device::scd_hint_vector_r )
{
//  printf("read HINT offset %d\n", offset);

	switch (offset&1)
	{
		case 0x00:
			//return 0x00ff; // doesn't make much sense..
			return 0xffff;
		case 0x01:
			return segacd_hint_register;
	}

	return 0;

}

READ16_MEMBER( sega_segacd_device::scd_a12006_hint_register_r )
{
	return segacd_hint_register;
}

WRITE16_MEMBER( sega_segacd_device::scd_a12006_hint_register_w )
{
	COMBINE_DATA(&segacd_hint_register);
}


void sega_segacd_device::segacd_mark_tiles_dirty(int offset)
{
	gfx(0)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(1)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(2)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(3)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(4)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(5)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(6)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx(7)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE16));

	gfx(8)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(9)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(10)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(11)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(12)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(13)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(14)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx(15)->mark_dirty((offset*2)/(SEGACD_BYTES_PER_TILE32));
}


// mame specific.. map registers to which tilemap cache we use
int sega_segacd_device::segacd_get_active_stampmap_tilemap(void)
{
	return (segacd_stampsize & 0x6)>>1;
}



void sega_segacd_device::SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 0; // 16x16 tiles
	int tile_base = (segacd_stampmap_base_address & 0xff80) * 4;

	int tiledat = m_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = tiledat & 0x07ff;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}

void sega_segacd_device::SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 8; // 32x32 tiles
	int tile_base = (segacd_stampmap_base_address & 0xffe0) * 4;

	int tiledat = m_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = (tiledat & 0x07fc)>>2;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}


void sega_segacd_device::SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 0; // 16x16 tiles
	int tile_base = (0x8000) * 4; // fixed address in this mode

	int tiledat = m_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = tiledat & 0x07ff;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}


void sega_segacd_device::SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 8; // 32x32 tiles
	int tile_base = (segacd_stampmap_base_address & 0xe000) * 4;

	int tiledat = m_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = (tiledat & 0x07fc)>>2;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}

/* Tilemap callbacks (we don't actually use the tilemaps due to the excessive overhead */



TILE_GET_INFO_MEMBER( sega_segacd_device::get_stampmap_16x16_1x1_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_1x1(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER( sega_segacd_device::get_stampmap_32x32_1x1_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_1x1(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}


TILE_GET_INFO_MEMBER( sega_segacd_device::get_stampmap_16x16_16x16_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_16x16(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER( sega_segacd_device::get_stampmap_32x32_16x16_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_16x16(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO_MEMBER(tile_region, tileno, 0, 0);
}

// non-tilemap functions to get a pixel from a 'tilemap' based on the above, but looking up each pixel, as to avoid the heavy cache bitmap

inline UINT8 sega_segacd_device::get_stampmap_16x16_1x1_tile_info_pixel(int xpos, int ypos)
{
	const int tilesize = 4; // 0xf pixels
	const int tilemapsize = 0x0f;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_1x1(tile_region,tileno,(int)tile_index);

	tileno %= gfx(tile_region)->elements();

	if (tileno==0) return 0x00;

	const UINT8* srcdata = gfx(tile_region)->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

inline UINT8 sega_segacd_device::get_stampmap_32x32_1x1_tile_info_pixel(int xpos, int ypos)
{
	const int tilesize = 5; // 0x1f pixels
	const int tilemapsize = 0x07;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_1x1(tile_region,tileno,(int)tile_index);

	tileno %= gfx(tile_region)->elements();

	if (tileno==0) return 0x00; // does this apply in this mode?

	const UINT8* srcdata = gfx(tile_region)->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

inline UINT8 sega_segacd_device::get_stampmap_16x16_16x16_tile_info_pixel(int xpos, int ypos)
{
	const int tilesize = 4; // 0xf pixels
	const int tilemapsize = 0xff;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_16x16(tile_region,tileno,(int)tile_index);

	tileno %= gfx(tile_region)->elements();

	if (tileno==0) return 0x00; // does this apply in this mode

	const UINT8* srcdata = gfx(tile_region)->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

inline UINT8 sega_segacd_device::get_stampmap_32x32_16x16_tile_info_pixel(int xpos, int ypos)
{
	const int tilesize = 5; // 0x1f pixels
	const int tilemapsize = 0x7f;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_16x16(tile_region,tileno,(int)tile_index);

	tileno %= gfx(tile_region)->elements();

	if (tileno==0) return 0x00;

	const UINT8* srcdata = gfx(tile_region)->get_data(tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}



WRITE16_MEMBER( sega_segacd_device::segacd_stopwatch_timer_w )
{
	if(data == 0)
		m_stopwatch_timer->reset();
	else
		printf("Stopwatch timer %04x\n",data);
}

READ16_MEMBER( sega_segacd_device::segacd_stopwatch_timer_r )
{
	INT32 result = (m_stopwatch_timer->time_elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(30.72))).as_double();

	return result & 0xfff;
}







READ16_MEMBER( sega_segacd_device::segacd_sub_led_ready_r )
{
	UINT16 retdata = 0x0000;

	if (ACCESSING_BITS_0_7)
	{
		retdata |= segacd_ready;
	}

	if (ACCESSING_BITS_8_15)
	{
		retdata |= segacd_redled << 8;
		retdata |= segacd_greenled << 9;
	}

	return retdata;
}

WRITE16_MEMBER( sega_segacd_device::segacd_sub_led_ready_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if ((data&0x01) == 0x00)
		{
			// reset CD unit
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		segacd_redled = (data >> 8)&1;
		segacd_greenled = (data >> 9)&1;

		machine().output().set_value("red_led",segacd_redled ^ 1);
		machine().output().set_value("green_led",segacd_greenled ^ 1);

		//popmessage("%02x %02x",segacd_greenled,segacd_redled);
	}
}



READ16_MEMBER( sega_segacd_device::segacd_sub_dataram_part1_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (scd_rammode&1)
			return m_dataram[offset];
		else
		{
			//printf("Illegal: segacd_sub_dataram_part1_r in mode 0 without permission\n");
			return 0x0000;
		}
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
//      printf("Unspported: segacd_sub_dataram_part1_r in mode 1 (Word RAM Expander - 1 Byte Per Pixel)\n");
		UINT16 data;

		if (scd_rammode&1)
		{
			data = segacd_1meg_mode_word_read(offset/2+0x00000/2, 0xffff);
		}
		else
		{
			data = segacd_1meg_mode_word_read(offset/2+0x20000/2, 0xffff);
		}

		if (offset&1)
		{
			return ((data & 0x00f0) << 4) | ((data & 0x000f) << 0);
		}
		else
		{
			return ((data & 0xf000) >> 4) | ((data & 0x0f00) >> 8);
		}


	}

	return 0x0000;
}

WRITE16_MEMBER( sega_segacd_device::segacd_sub_dataram_part1_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (scd_rammode&1)
		{
			COMBINE_DATA(&m_dataram[offset]);
			segacd_mark_tiles_dirty(offset);
		}
		else
		{
			//printf("Illegal: segacd_sub_dataram_part1_w in mode 0 without permission\n");
		}
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//if (mem_mask==0xffff)
		//  printf("Unspported: segacd_sub_dataram_part1_w in mode 1 (Word RAM Expander - 1 Byte Per Pixel) %04x %04x\n", data, mem_mask);

		data = (data & 0x000f) | (data & 0x0f00)>>4;
		mem_mask = (mem_mask & 0x000f) | (mem_mask & 0x0f00)>>4;

//      data = ((data & 0x00f0) >>4) | (data & 0xf000)>>8;
//      mem_mask = ((mem_mask & 0x00f0)>>4) | ((mem_mask & 0xf000)>>8);


		if (!(offset&1))
		{
			data <<=8;
			mem_mask <<=8;
		}

		if (scd_rammode&1)
		{
			segacd_1meg_mode_word_write(offset/2+0x00000/2, data , mem_mask, 1);
		}
		else
		{
			segacd_1meg_mode_word_write(offset/2+0x20000/2, data, mem_mask, 1);
		}

	//  printf("Unspported: segacd_sub_dataram_part1_w in mode 1 (Word RAM Expander - 1 Byte Per Pixel) %04x\n", data);
	}
}

READ16_MEMBER( sega_segacd_device::segacd_sub_dataram_part2_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		//printf("ILLEGAL segacd_sub_dataram_part2_r in mode 0\n"); // not mapped to anything in mode 0
		return 0x0000;
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//printf("Unsupported: segacd_sub_dataram_part2_r in mode 1 (Word RAM)\n");
		// ret bit set by sub cpu determines which half of WorkRAM we have access to?
		if (scd_rammode&1)
		{
			return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
		}
		else
		{
			return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
		}

	}

	return 0x0000;
}

WRITE16_MEMBER( sega_segacd_device::segacd_sub_dataram_part2_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		printf("ILLEGAL segacd_sub_dataram_part2_w in mode 0\n"); // not mapped to anything in mode 0
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//printf("Unsupported: segacd_sub_dataram_part2_w in mode 1 (Word RAM)\n");
		// ret bit set by sub cpu determines which half of WorkRAM we have access to?
		if (scd_rammode&1)
		{
			segacd_1meg_mode_word_write(offset+0x00000/2, data, mem_mask, 0);
		}
		else
		{
			segacd_1meg_mode_word_write(offset+0x20000/2, data, mem_mask, 0);
		}

	}
}



READ16_MEMBER( sega_segacd_device::segacd_stampsize_r )
{
	UINT16 retdata = 0x0000;

	retdata |= segacd_conversion_active<<15;

	retdata |= segacd_stampsize & 0x7;

	return retdata;

}

WRITE16_MEMBER( sega_segacd_device::segacd_stampsize_w )
{
	//printf("segacd_stampsize_w %04x %04x\n",data, mem_mask);
	if (ACCESSING_BITS_0_7)
	{
		segacd_stampsize = data & 0x07;
		//if (data & 0xf8)
		//  printf("    unused bits (LSB) set in stampsize!\n");

		//if (data&1) printf("    Repeat On\n");
		//else printf("    Repeat Off\n");

		//if (data&2) printf("    32x32 dots\n");
		//else printf("    16x16 dots\n");

		//if (data&4) printf("    16x16 screens\n");
		//else printf("    1x1 screen\n");
	}

	if (ACCESSING_BITS_8_15)
	{
		//if (data&0xff00) printf("    unused bits (MSB) set in stampsize!\n");
	}
}

// these functions won't cope if
//
// the lower 3 bits of segacd_imagebuffer_hdot_size are set

// this really needs to be doing it's own lookups rather than depending on the inefficient MAME cache..
inline UINT8 sega_segacd_device::read_pixel_from_stampmap(bitmap_ind16* srcbitmap, int x, int y)
{
/*
    if (!srcbitmap)
    {
        return machine.rand();
    }

    if (x >= srcbitmap->width) return 0;
    if (y >= srcbitmap->height) return 0;

    UINT16* cacheptr = &srcbitmap->pix16(y, x);

    return cacheptr[0] & 0xf;
*/

	switch (segacd_get_active_stampmap_tilemap()&3)
	{
		case 0x00: return get_stampmap_16x16_1x1_tile_info_pixel( x, y );
		case 0x01: return get_stampmap_32x32_1x1_tile_info_pixel( x, y );
		case 0x02: return get_stampmap_16x16_16x16_tile_info_pixel( x, y );
		case 0x03: return get_stampmap_32x32_16x16_tile_info_pixel( x, y );
	}

	return 0;
}





// this triggers the conversion operation, which will cause an IRQ1 when finished
WRITE16_MEMBER( sega_segacd_device::segacd_trace_vector_base_address_w )
{
	if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		printf("ILLEGAL: segacd_trace_vector_base_address_w %04x %04x in mode 1!\n",data,mem_mask);
	}

	//printf("segacd_trace_vector_base_address_w %04x %04x\n",data,mem_mask);

	{
		int base = (data & 0xfffe) * 4;

		//printf("actual base = %06x\n", base + 0x80000);

		// nasty nasty nasty
		//segacd_mark_stampmaps_dirty();

		segacd_conversion_active = 1;

		// todo: proper time calculation
		m_stamp_timer->adjust(attotime::from_nsec(30000));



		int line;
		//bitmap_ind16 *srcbitmap = segacd_stampmap[segacd_get_active_stampmap_tilemap(->pixmap()]);
		bitmap_ind16 *srcbitmap = nullptr;
		UINT32 bufferstart = ((segacd_imagebuffer_start_address&0xfff8)*2)<<3;

		for (line=0;line<segacd_imagebuffer_vdot_size;line++)
		{
			int currbase = base + line * 0x8;

			// are the 256x256 tile modes using the same sign bits?
			INT16 tilemapxoffs,tilemapyoffs;
			INT16 deltax,deltay;

			tilemapxoffs = m_dataram[(currbase+0x0)>>1];
			tilemapyoffs = m_dataram[(currbase+0x2)>>1];
			deltax = m_dataram[(currbase+0x4)>>1]; // x-zoom
			deltay = m_dataram[(currbase+0x6)>>1]; // rotation

			//printf("%06x:  %04x (%d) %04x (%d) %04x %04x\n", currbase, tilemapxoffs, tilemapxoffs>>3, tilemapyoffs, tilemapyoffs>>3, deltax, deltay);

			int xbase = tilemapxoffs * 256;
			int ybase = tilemapyoffs * 256;
			int count;

			for (count=0;count<(segacd_imagebuffer_hdot_size);count++)
			{
				//int i;
				UINT8 pix = 0x0;

				pix = read_pixel_from_stampmap(srcbitmap, xbase>>(3+8), ybase>>(3+8));

				xbase += deltax;
				ybase += deltay;

				// clamp to 24-bits, seems to be required for all the intro effects to work
				xbase &= 0xffffff;
				ybase &= 0xffffff;

				int countx = count + (segacd_imagebuffer_offset&0x7);

				UINT32 offset;

				offset = bufferstart+((((segacd_imagebuffer_vcell_size+1)*0x10)*(countx>>3))<<3);

				offset+= ((line*2)<<3);
				offset+=(segacd_imagebuffer_offset&0x38)<<1;

				offset+=countx & 0x7;

				write_pixel( pix, offset );

				segacd_mark_tiles_dirty(offset>>3);
				segacd_mark_tiles_dirty((offset>>3)+1);

			}

		}
	}

}

// actually just the low 8 bits?
READ16_MEMBER( sega_segacd_device::segacd_imagebuffer_vdot_size_r )
{
	return segacd_imagebuffer_vdot_size;
}

WRITE16_MEMBER( sega_segacd_device::segacd_imagebuffer_vdot_size_w )
{
	//printf("segacd_imagebuffer_vdot_size_w %04x %04x\n",data,mem_mask);
	COMBINE_DATA(&segacd_imagebuffer_vdot_size);
}


// basically the 'tilemap' base address, for the 16x16 / 32x32 source tiles
READ16_MEMBER( sega_segacd_device::segacd_stampmap_base_address_r )
{
	// different bits are valid in different modes, but I'm guessing the register
	// always returns all the bits set, even if they're not used?
	return segacd_stampmap_base_address;

}

WRITE16_MEMBER( sega_segacd_device::segacd_stampmap_base_address_w )
{ // WORD ACCESS

	// low 3 bitsa aren't used, are they stored?
	COMBINE_DATA(&segacd_stampmap_base_address);
}

// destination for 'rendering' the section of the tilemap(stampmap) requested
READ16_MEMBER( sega_segacd_device::segacd_imagebuffer_start_address_r )
{
	return segacd_imagebuffer_start_address;
}

WRITE16_MEMBER( sega_segacd_device::segacd_imagebuffer_start_address_w )
{
	COMBINE_DATA(&segacd_imagebuffer_start_address);

	//int base = (segacd_imagebuffer_start_address & 0xfffe) * 4;
	//printf("segacd_imagebuffer_start_address_w %04x %04x (actual base = %06x)\n", data, segacd_imagebuffer_start_address, base);
}

READ16_MEMBER( sega_segacd_device::segacd_imagebuffer_offset_r )
{
	return segacd_imagebuffer_offset;
}

WRITE16_MEMBER( sega_segacd_device::segacd_imagebuffer_offset_w )
{
	COMBINE_DATA(&segacd_imagebuffer_offset);
//  printf("segacd_imagebuffer_offset_w %04x\n", segacd_imagebuffer_offset);
}

READ16_MEMBER( sega_segacd_device::segacd_imagebuffer_vcell_size_r )
{
	return segacd_imagebuffer_vcell_size;
}

WRITE16_MEMBER( sega_segacd_device::segacd_imagebuffer_vcell_size_w )
{
	COMBINE_DATA(&segacd_imagebuffer_vcell_size);
}


READ16_MEMBER( sega_segacd_device::segacd_imagebuffer_hdot_size_r )
{
	return segacd_imagebuffer_hdot_size;
}

WRITE16_MEMBER( sega_segacd_device::segacd_imagebuffer_hdot_size_w )
{
	COMBINE_DATA(&segacd_imagebuffer_hdot_size);
}



READ16_MEMBER( sega_segacd_device::segacd_irq3timer_r )
{
	return m_irq3_timer_reg; // always returns value written, not current counter!
}


WRITE16_MEMBER( sega_segacd_device::segacd_irq3timer_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_irq3_timer_reg = data & 0xff;

		// time = reg * 30.72 us

		if (m_irq3_timer_reg)
			m_irq3_timer->adjust(SEGACD_IRQ3_TIMER_SPEED);
		else
			m_irq3_timer->adjust(attotime::never);

		//printf("segacd_irq3timer_w %02x\n", segacd_m_irq3_timer_reg);
	}
}


READ8_MEMBER( sega_segacd_device::backupram_r )
{
	return m_backupram[offset];
}

WRITE8_MEMBER( sega_segacd_device::backupram_w )
{
	m_backupram[offset] = data;
}

READ8_MEMBER( sega_segacd_device::font_color_r )
{
	return m_font_color;
}

WRITE8_MEMBER( sega_segacd_device::font_color_w )
{
	m_font_color = data;
}

READ16_MEMBER( sega_segacd_device::font_converted_r )
{
	int scbg = (m_font_color & 0x0f);
	int scfg = (m_font_color & 0xf0)>>4;
	UINT16 retdata = 0;
	int bit;

	for (bit=0;bit<4;bit++)
	{
		if (*m_font_bits&((0x1000>>offset*4)<<bit))
			retdata |= scfg << (bit*4);
		else
			retdata |= scbg << (bit*4);
	}

	return retdata;
}





void sega_segacd_device::device_start()
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	m_backupram.resize(0x2000);
	subdevice<nvram_device>("backupram")->set_base(&m_backupram[0], 0x2000);

	segacd_4meg_prgbank = 0;

	space.unmap_readwrite        (0x020000,0x3fffff);

	space.install_read_handler (0x0020000, 0x003ffff, read16_delegate(FUNC(sega_segacd_device::scd_4m_prgbank_ram_r),this) );
	space.install_write_handler (0x0020000, 0x003ffff, write16_delegate(FUNC(sega_segacd_device::scd_4m_prgbank_ram_w),this) );


	space.install_readwrite_handler(0x200000, 0x23ffff, read16_delegate(FUNC(sega_segacd_device::segacd_main_dataram_part1_r),this), write16_delegate(FUNC(sega_segacd_device::segacd_main_dataram_part1_w),this)); // RAM shared with sub
	space.install_readwrite_handler(0xa12000, 0xa12001, read16_delegate(FUNC(sega_segacd_device::scd_a12000_halt_reset_r),this), write16_delegate(FUNC(sega_segacd_device::scd_a12000_halt_reset_w),this)); // sub-cpu control
	space.install_readwrite_handler(0xa12002, 0xa12003, read16_delegate(FUNC(sega_segacd_device::scd_a12002_memory_mode_r),this), write16_delegate(FUNC(sega_segacd_device::scd_a12002_memory_mode_w),this)); // memory mode / write protect
	//space.install_readwrite_handler(0xa12004, 0xa12005, read16_delegate(FUNC(sega_segacd_device::segacd_cdc_mode_address_r),this), write16_delegate(FUNC(sega_segacd_device::segacd_cdc_mode_address_w),this));
	space.install_readwrite_handler(0xa12006, 0xa12007, read16_delegate(FUNC(sega_segacd_device::scd_a12006_hint_register_r),this), write16_delegate(FUNC(sega_segacd_device::scd_a12006_hint_register_w),this)); // where HINT points on main CPU
	//space.install_read_handler     (0xa12008, 0xa12009, read16_delegate(FUNC(sega_segacd_device::cdc_data_main_r),this));


	space.install_readwrite_handler(0xa1200c, 0xa1200d, read16_delegate(FUNC(sega_segacd_device::segacd_stopwatch_timer_r),this), write16_delegate(FUNC(sega_segacd_device::segacd_stopwatch_timer_w),this)); // starblad

	space.install_readwrite_handler(0xa1200e, 0xa1200f, read16_delegate(FUNC(sega_segacd_device::segacd_comms_flags_r),this), write16_delegate(FUNC(sega_segacd_device::segacd_comms_flags_maincpu_w),this)); // communication flags block

	space.install_readwrite_handler(0xa12010, 0xa1201f, read16_delegate(FUNC(sega_segacd_device::segacd_comms_main_part1_r),this), write16_delegate(FUNC(sega_segacd_device::segacd_comms_main_part1_w),this));
	space.install_readwrite_handler(0xa12020, 0xa1202f, read16_delegate(FUNC(sega_segacd_device::segacd_comms_main_part2_r),this), write16_delegate(FUNC(sega_segacd_device::segacd_comms_main_part2_w),this));



	space.install_read_handler (0x0000070, 0x0000073, read16_delegate(FUNC(sega_segacd_device::scd_hint_vector_r),this) );

	segacd_stampmap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(sega_segacd_device::get_stampmap_16x16_1x1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	segacd_stampmap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(sega_segacd_device::get_stampmap_32x32_1x1_tile_info),this), TILEMAP_SCAN_ROWS, 32, 32, 8, 8);
	segacd_stampmap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(sega_segacd_device::get_stampmap_16x16_16x16_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 256, 256); // 128kb!
	segacd_stampmap[3] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(sega_segacd_device::get_stampmap_32x32_16x16_tile_info),this), TILEMAP_SCAN_ROWS, 32, 32, 128, 128); // 32kb!

	// todo register save state stuff
}

READ16_MEMBER( sega_segacd_device::segacd_dmaaddr_r )
{
	return m_dmaaddr;
}

WRITE16_MEMBER( sega_segacd_device::segacd_dmaaddr_w )
{
	COMBINE_DATA(&m_dmaaddr);
}

void sega_segacd_device::device_reset()
{
	m_scdcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_scdcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	segacd_hint_register = 0xffff; // -1

	m_a12000_halt_reset_reg = 0x0000;

	scd_rammode = 0;
	scd_mode_dmna_ret_flags = 0x5421;

	m_lc89510_temp->reset_cd();
	m_dmaaddr = 0;
	m_dma_timer->adjust(attotime::zero);

	m_total_scanlines = 262;

	// HACK!!!! timegal, anettfut, roadaven end up with the SubCPU waiting in a loop for *something*
	// overclocking the CPU, even at the point where the game is hung, allows them to continue and boot
	// I'm not sure what the source of this timing problem is, it's not using IRQ3 or StopWatch at the
	// time.  Changing the CDHock timer to 50hz from 75hz also stops the hang, but then the video is
	// too slow and has bad sound.  -- Investigate!

	m_scdcpu->set_clock_scale(1.5000f);


	// initialize some stuff on reset

	segacd_ram_writeprotect_bits = 0;
	segacd_4meg_prgbank = 0;
	segacd_memory_priority_mode = 0;
	segacd_stampsize = 0;

	segacd_imagebuffer_vdot_size = 0;
	segacd_imagebuffer_vcell_size = 0;
	segacd_imagebuffer_hdot_size = 0;

	segacd_conversion_active = 0;
	segacd_stampmap_base_address = 0;
	segacd_imagebuffer_start_address = 0;
	segacd_imagebuffer_offset = 0;

	segacd_comms_flags = 0x0000;

	segacd_redled = 0;
	segacd_greenled = 0;
	segacd_ready = 1; // actually set 100ms after startup?
	m_irq3_timer_reg = 0;

	m_stamp_timer->adjust(attotime::never);
	m_irq3_timer->adjust(attotime::never);

}


// todo: tidy up
TIMER_DEVICE_CALLBACK_MEMBER( sega_segacd_device::dma_timer_callback )
{
	// todo: accurate timing of this!

	#define RATE 256
	m_lc89510_temp->CDC_Do_DMA(machine(), RATE);

	// timed reset of flags
	scd_mode_dmna_ret_flags |= 0x0021;

	m_dma_timer->adjust(attotime::from_hz(m_framerate) / m_total_scanlines);

}

// todo: tidy up, too many CDC internals here
void sega_segacd_device::SegaCD_CDC_Do_DMA(int &dmacount, UINT8 *CDC_BUFFER, UINT16 &dma_addrc, UINT16 &destination )
{
	int length = dmacount;
	UINT16 *dest;
	int srcoffset = 0;
	int dstoffset = 0;
	address_space& space = m_scdcpu->space(AS_PROGRAM);

	bool PCM_DMA = false;

	if (destination==DMA_PCM)
	{
		dstoffset = (m_dmaaddr & 0x03FF) << 2;
		PCM_DMA = true;
	}
	else
	{
		dstoffset = (m_dmaaddr & 0xFFFF) << 3;
	}


	while (dmacount--)
	{
		UINT16 data = (CDC_BUFFER[dma_addrc+srcoffset]<<8) | CDC_BUFFER[dma_addrc+srcoffset+1];

		if (destination==DMA_PRG)
		{
			dest = m_prgram;
		}
		else if (destination==DMA_WRAM)
		{
			dest = m_dataram;
		}
		else if (destination==DMA_PCM)
		{
			dest = nullptr;//fatalerror("PCM RAM DMA unimplemented!\n");
		}
		else
		{
			fatalerror("Unknown DMA Destination!!\n");
		}

		if (PCM_DMA)
		{
			m_rfsnd->rf5c68_mem_w(space, dstoffset & 0xfff, data >> 8);
			m_rfsnd->rf5c68_mem_w(space, (dstoffset+1) & 0xfff, data);
		//  printf("PCM_DMA writing %04x %04x\n",0xff2000+(dstoffset*2), data);
		}
		else
		{
			if (dest)
			{
				if (destination==DMA_WRAM)
				{
					if ((scd_rammode&2)==RAM_MODE_2MEG)
					{
						dstoffset &= 0x3ffff;

						dest[dstoffset/2] = data;

						segacd_mark_tiles_dirty(dstoffset/2);
					}
					else
					{
						dstoffset &= 0x1ffff;

						if (!(scd_rammode & 1))
						{
							segacd_1meg_mode_word_write((dstoffset+0x20000)/2, data, 0xffff, 0);
						}
						else
						{
							segacd_1meg_mode_word_write((dstoffset+0x00000)/2, data, 0xffff, 0);
						}
					}

				}
				else
				{
					// main ram
					dest[dstoffset/2] = data;
				}

			}
		}

		srcoffset += 2;
		dstoffset += 2;
	}


	if (PCM_DMA)
	{
		m_dmaaddr += length >> 1;
	}
	else
	{
		m_dmaaddr += length >> 2;
	}
}
