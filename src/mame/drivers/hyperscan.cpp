// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Mattel HyperScan

        08/17/2013 Skeleton driver by Sandro Ronco

        SPG290 Interrupt:

        Vector      Source
          63        SPU FIQ
          62        SPU Beatirq
          61        SPU Envirq
          60        CD servo
          59        ADC gain overflow / ADC recorder / FIFO overflow
          58        General purpose ADC
          57        Timer base
          56        Timer
          55        TV vblanking start
          54        LCD vblanking start
          53        PPU vblanking start
          52        TV
          51        Sensor frame end
          50        Sensor coordinate hit
          49        Sensor motion frame end
          48        Sensor capture done + sensor debug IRQ
          47        TV coordinate hit
          46        PPU coordinate hit
          45        USB host+device
          44        SIO
          43        SPI
          42        Uart (IrDA)
          41        NAND
          40        SD
          39        I2C master
          38        I2S slave
          37        APBDMA CH1
          36        APBDMA CH2
          35        LDM_DMA
          34        BLN_DMA
          33        APBDMA CH3
          32        APBDMA CH4
          31        Alarm + HMS
          30        MP4
          29        C3 (ECC module)
          28        GPIO
          27        Bufctl (for debug) + TV/PPU vblanking end (for debug)
          26        RESERVED1
          25        RESERVED2
          24        RESERVED3

****************************************************************************/

#include "emu.h"
#include "cpu/score/score.h"


#define LOG_SPG290_REGISTER_ACCESS  (1)



class hyperscan_state : public driver_device
{
public:
	hyperscan_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
		{ }

	required_device<score7_cpu_device> m_maincpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	UINT32 spg290_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ32_MEMBER(spg290_regs_r);
	DECLARE_WRITE32_MEMBER(spg290_regs_w);
	void spg290_timers_update();
	void spg290_vblank_irq(screen_device &screen, bool state);
	inline UINT32 spg290_read_mem(UINT32 offset);
	inline void spg290_write_mem(UINT32 offset, UINT32 data);
	void spg290_argb1555(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 posy, UINT16 posx, UINT16 argb);
	void spg290_rgb565(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 posy, UINT16 posx, UINT16 rgb, UINT32 transrgb);
	void spg290_blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 control, UINT32 attribute, UINT32 *palettes, UINT32 buf_start);
	void spg290_blit_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 control, UINT32 attribute, int posy, int posx, UINT32 nptr, UINT32 buf_start, UINT32 transrgb);
	void spg290_blit_character(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 control, UINT32 attribute, int posy, int posx, UINT32 nptr, UINT32 buf_start, UINT32 transrgb);

private:
	static const device_timer_id TIMER_SPG290 = 0;
	static const device_timer_id TIMER_I2C = 1;

	struct spg290_miu
	{
		UINT32  status;
	};

	struct spg290_ppu
	{
		UINT32  control;
		UINT32  irq_control;
		UINT32  irq_status;
		UINT32  sprite_max;
		UINT32  sprite_buf_start;
		UINT32  frame_buff[3];
		UINT32  palettes[0x200];
		UINT32  tx_hoffset[0x200];
		UINT32  tx_hcomp[0x200];
		UINT32  transrgb;

		struct ppu_spite
		{
			UINT32  control;
			UINT32  attribute;
		} sprites[0x200];

		struct ppu_tx
		{
			UINT32  control;
			UINT32  attribute;
			UINT32  posx;
			UINT32  posy;
			UINT32  nptr;
			UINT32  buf_start[3];
		} txs[3];
	};

	struct spg290_timer
	{
		UINT32  control;
		UINT32  control2;
		UINT16  preload;
		UINT16  counter;
	};

	struct spg290_i2c
	{
		UINT32  config;
		UINT32  irq_control;
		UINT32  clock;
		UINT8   count;
		UINT32  id;
		UINT32  port_addr;
		UINT32  wdata;
		UINT32  rdata;
	};

	spg290_miu      m_miu;
	spg290_ppu      m_ppu;
	spg290_timer    m_timers[6];
	spg290_i2c      m_i2c;
	emu_timer *     m_update_timer;
	emu_timer *     m_i2c_timer;
};



#if LOG_SPG290_REGISTER_ACCESS
static void log_spg290_regs(device_t *device,UINT8 module, UINT16 reg, UINT32 mem_mask, bool write, UINT32 data=0)
{
	static const char *const modules_name[] =
	{
		"CSI", "PPU", "JPG", "TV", "LCD", "SPU", "CD", "MIU", "APBDMA", "BUFCTL","IRQCTL", "GPUBUF", "LDMDMA",
		"BLNDMA", "TPGBUF", "AHBDEC", "GPIO", "SPI", "SIO", "I2C", "I2S", "UART", "TIMERS/RTC", "WDOG", "SD",
		"FLASH", "ADC", "USB device", "USB host", "Reserved", "Reserved", "Reserved", "SFTCFG", "CKG", "MP4",
		"MIU2", "ECC"
	};

	if (module < 0x25)
		device->logerror("SPG: %-10s", modules_name[module]);
	else
		device->logerror("SPG: mod 0x%02x  ", module);

	if (!write)
		device->logerror(" R 0x%04x", reg);
	else
		device->logerror(" W 0x%04x = 0x%08x", reg, data);

	if (mem_mask != 0xffffffffu)
		device->logerror(" (0x%08x)\n",  mem_mask);
	else
		device->logerror("\n");
}
#endif


READ32_MEMBER(hyperscan_state::spg290_regs_r)
{
	UINT32 addr = offset << 2;
	UINT32 data = 0;

	if (addr == 0x010000)           // PPU Control
	{
		data = m_ppu.control;
	}
	else if (addr == 0x010084)      // PPU IRQ Status
	{
		data = m_ppu.irq_status;
	}
	if (addr == 0x07006c)           // MIU Status
	{
		data = m_miu.status;
	}
	else if ((addr & 0xff0fff) == 0x160000)   // Timer X Status
	{
		int idx = (offset>>10) & 0x0f;
		data = m_timers[idx].control;
	}
	else if(addr == 0x130024)       // I2C interrupt
	{
		data = m_i2c.irq_control;
	}
	else if(addr == 0x130034)       // I2C write data
	{
		data = m_i2c.wdata;
	}
	else if(addr == 0x130038)       // I2C data data
	{
		data = m_i2c.rdata;
	}

#if LOG_SPG290_REGISTER_ACCESS
	//else
	{
		if (!space.debugger_access())
			log_spg290_regs(this,(offset >> 14) & 0xff, (offset<<2) & 0xffff, mem_mask, false);
	}
#endif

	return data;
}

WRITE32_MEMBER(hyperscan_state::spg290_regs_w)
{
	UINT32 addr = offset << 2;

	if (addr == 0x010000)               // PPU Control
	{
		COMBINE_DATA(&m_ppu.control);
	}
	else if (addr == 0x010008)          // PPU Max Sprites
	{
		COMBINE_DATA(&m_ppu.sprite_max);
	}
	else if (addr == 0x010010)          // PPU Max Sprites
	{
		COMBINE_DATA(&m_ppu.transrgb);
	}
	else if (addr == 0x0100d0)          // Sprites buffer start
	{
		COMBINE_DATA(&m_ppu.sprite_buf_start);
	}
	else if (addr == 0x010020 || addr == 0x01003c || addr == 0x010058)   // Text Layers x pos
	{
		int idx = (((offset>>3) & 3) | ((offset>>4) & 1)) - 1;
		COMBINE_DATA(&m_ppu.txs[idx].posx);
	}
	else if (addr == 0x010024 || addr == 0x010040 || addr == 0x01005c)   // Text Layers y pos
	{
		int idx = (((offset>>3) & 3) | ((offset>>4) & 1)) - 1;
		COMBINE_DATA(&m_ppu.txs[idx].posy);
	}
	else if (addr == 0x010028 || addr == 0x010044 || addr == 0x010060)   // Text Layers attribute
	{
		int idx = ((offset>>3) & 3) - 1;
		COMBINE_DATA(&m_ppu.txs[idx].attribute);
	}
	else if (addr == 0x01002c || addr == 0x010048 || addr == 0x010064)   // Text Layers control
	{
		int idx = ((offset>>3) & 3) - 1;
		COMBINE_DATA(&m_ppu.txs[idx].control);
	}
	else if (addr == 0x010030 || addr == 0x01004c || addr == 0x010068)   // Text Layers number ptr
	{
		int idx = ((offset>>3) & 3) - 1;
		COMBINE_DATA(&m_ppu.txs[idx].nptr);
	}
	else if (addr == 0x010080)          // PPU IRQ Control
	{
		COMBINE_DATA(&m_ppu.irq_control);
	}
	else if (addr == 0x010084)          // PPU IRQ ack
	{
		if (ACCESSING_BITS_0_7)
			m_ppu.irq_status &= ~data;
	}
	else if (addr >= 0x0100a0 && addr <= 0x0100a8)          // Tx1 buffer start
	{
		COMBINE_DATA(&m_ppu.txs[0].buf_start[offset & 3]);
	}
	else if (addr >= 0x0100ac && addr <= 0x0100b4)          // Tx2 buffer start
	{
		COMBINE_DATA(&m_ppu.txs[1].buf_start[(offset+1) & 3]);
	}
	else if (addr >= 0x0100b8 && addr <= 0x0100c0)          // Tx3 buffer starts
	{
		COMBINE_DATA(&m_ppu.txs[2].buf_start[(offset+2) & 3]);
	}
	else if ((addr & 0xfff000) == 0x011000)                 // Palettes
	{
		COMBINE_DATA(&m_ppu.palettes[offset & 0x01ff]);
	}
	else if ((addr & 0xfff000) == 0x012000)                 // Tx horizontal offset
	{
		COMBINE_DATA(&m_ppu.tx_hoffset[offset & 0x01ff]);
	}
	else if ((addr & 0xfff000) == 0x013000)                 // Tx horizontal compression
	{
		COMBINE_DATA(&m_ppu.tx_hcomp[offset & 0x01ff]);
	}
	else if ((addr & 0xfff000) == 0x014000)                 // Sprite Control Registers
	{
		int idx = (offset>>1) & 0x1ff;
		if (offset & 1)
			COMBINE_DATA(&m_ppu.sprites[idx].attribute);
		else
			COMBINE_DATA(&m_ppu.sprites[idx].control);
	}
	else if (addr == 0x07005c)                  // MIU Status
	{
		COMBINE_DATA(&m_miu.status);
	}
	else if ((addr & 0xff0fff) == 0x160000)     // Timer X Control 1
	{
		int idx = (offset>>10) & 0x07;
		COMBINE_DATA(&m_timers[idx].control);

		if (ACCESSING_BITS_24_31)
			m_timers[idx].control &= ~(data & 0x04000000);  // Timers IRQ ack
	}
	else if ((addr & 0xff0fff) == 0x160004)     // Timer X Control 2
	{
		int idx = (offset>>10) & 0x07;
		COMBINE_DATA(&m_timers[idx].control2);
	}
	else if ((addr & 0xff0fff) == 0x160008)     // Timer X Preload
	{
		int idx = (offset>>10) & 0x07;
		COMBINE_DATA(&m_timers[idx].preload);
	}
	else if (addr == 0x2100e4)                  // Timer Source Clock Selection
	{
		UINT32 timers_clk = XTAL_27MHz / ((data & 0xff) + 1);
		m_update_timer->adjust(attotime::from_hz(timers_clk), 0, attotime::from_hz(timers_clk));
	}
	else if(addr == 0x130020)                   // I2C configuration
	{
		COMBINE_DATA(&m_i2c.config);
	}
	else if(addr == 0x130024)                   // I2C interrupt
	{
		COMBINE_DATA(&m_i2c.irq_control);

		if (ACCESSING_BITS_0_7)
			m_i2c.irq_control &= ~(data & 0x00000001);  // I2C IRQ ack
	}
	else if(addr == 0x130028)                   // I2C clock setting
	{
		COMBINE_DATA(&m_i2c.clock);
		UINT32 i2c_clk = XTAL_27MHz / ((m_i2c.clock & 0x3ff) + 1);
		m_i2c_timer->adjust(attotime::from_hz(i2c_clk), 0, attotime::from_hz(i2c_clk));
	}
	else if(addr == 0x13002c)                   // I2C ID
	{
		COMBINE_DATA(&m_i2c.id);
	}
	else if(addr == 0x130030)                   // I2C port address
	{
		COMBINE_DATA(&m_i2c.port_addr);
	}
	else if(addr == 0x130034)                   // I2C write data
	{
		COMBINE_DATA(&m_i2c.wdata);
	}
	else if(addr == 0x130038)                   // I2C data data
	{
		COMBINE_DATA(&m_i2c.rdata);
	}
	else if(addr == 0x150000)                   // UART data
	{
		if (ACCESSING_BITS_0_7)
			printf("%c", data & 0xff);
	}

#if LOG_SPG290_REGISTER_ACCESS
	//else
	{
		if (!space.debugger_access())
			log_spg290_regs(this,(offset >> 14) & 0xff, (offset<<2) & 0xffff, mem_mask, true, data);
	}
#endif
}

inline UINT32 hyperscan_state::spg290_read_mem(UINT32 offset)
{
	return m_maincpu->space(0).read_dword(offset);
}

inline void hyperscan_state::spg290_write_mem(UINT32 offset, UINT32 data)
{
	return m_maincpu->space(0).write_dword(offset, data);
}

void hyperscan_state::spg290_argb1555(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 posy, UINT16 posx, UINT16 argb)
{
	if (!(argb & 0x8000) && cliprect.contains(posx, posy))
	{
		rgb_t color = rgb_t(pal5bit(argb >> 10), pal5bit(argb >> 5), pal5bit(argb >> 0));
		bitmap.pix32(posy, posx) = color;
	}
}

void hyperscan_state::spg290_rgb565(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 posy, UINT16 posx, UINT16 rgb, UINT32 transrgb)
{
	if ((!(transrgb & 0x10000) || (transrgb & 0xffff) != rgb) && cliprect.contains(posx, posy))
	{
		rgb_t color = rgb_t(pal5bit(rgb >> 11), pal6bit(rgb >> 5), pal5bit(rgb >> 0));
		bitmap.pix32(posy, posx) = color;
	}
}

void hyperscan_state::spg290_blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 control, UINT32 attribute, UINT32 *palettes, UINT32 buf_start)
{
	UINT32 sprite_base = buf_start + ((control & 0xffff) << 8);
	UINT16 sprite_x    = (control >> 16) & 0x3ff;
	UINT16 sprite_y    = (attribute >> 16) & 0x3ff;
	UINT8 sprite_hsize = 8 << ((attribute >> 4) & 0x03);
	UINT8 sprite_vsize = 8 << ((attribute >> 6) & 0x03);
	UINT8 bit_pixel    = ((attribute & 3) + 1) << 1;
	UINT8 pixel_word   = 32 / bit_pixel;
	UINT8 word_line    = sprite_hsize / pixel_word;
	UINT8 sprite_flip  = (attribute >> 2) & 0x03;

	for (int y=0; y < sprite_vsize; y++)
		for (int x=0; x < word_line; x++)
		{
			UINT32 data = spg290_read_mem(sprite_base + (y * pixel_word + x) * 4);

			for (int b=0; b < pixel_word; b++)
			{
				UINT16 pen = ((data >> (b * bit_pixel)) & ((1 << bit_pixel) - 1));
				UINT16 posx;
				if (sprite_flip & 0x01)
					posx = sprite_x + (sprite_hsize - (x * word_line + b));
				else
					posx = sprite_x + x * (word_line) + b;

				UINT16 posy;
				if (sprite_flip & 0x02)
					posy = sprite_y + (sprite_vsize - y);
				else
					posy = sprite_y + y;

				spg290_argb1555(bitmap, cliprect, posy, posx, palettes[pen]);
			}
		}
}

void hyperscan_state::spg290_blit_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 control, UINT32 attribute, int posy, int posx, UINT32 nptr, UINT32 buf_start, UINT32 transrgb)
{
	for (int y=0; y<512; y++)
	{
		int line = (control & 0x04) ? 0 : y;
		UINT32 tx_start = spg290_read_mem(nptr + (posy + line) * 4);

		for (int x=0; x < 1024>>1; x++)
		{
			UINT32 data = spg290_read_mem(buf_start + (tx_start + posx) * 2 + x * 4);

			for (int b=0; b < 2; b++)
			{
				UINT16 posx = x * 2 + b;
				UINT16 posy = y;
				UINT16 pix = (data >> (b*16)) & 0xffff;

				if (control & 0x0080)
					spg290_argb1555(bitmap, cliprect, posy, posx, pix);
				if (control & 0x1000)
					spg290_rgb565(bitmap, cliprect, posy, posx, pix, transrgb);
			}
		}
	}
}

void hyperscan_state::spg290_blit_character(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT32 control, UINT32 attribute, int posy, int posx, UINT32 nptr, UINT32 buf_start, UINT32 transrgb)
{
	// TODO
}


UINT32 hyperscan_state::spg290_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_ppu.control & 0x1000)
	{
		for (int depth=0; depth<4; depth++)
		{
			// draw the bitmap/text layers
			for (int l=0; l<3; l++)
				if ((m_ppu.txs[l].control & 0x08) && ((m_ppu.txs[l].attribute >> 13) & 3) == depth)
				{
					if (m_ppu.txs[l].control & 0x01)
						spg290_blit_bitmap(bitmap, cliprect, m_ppu.txs[l].control, m_ppu.txs[l].attribute, m_ppu.txs[l].posy & 0x1ff, m_ppu.txs[l].posx & 0x1ff, m_ppu.txs[l].nptr, m_ppu.txs[l].buf_start[0], m_ppu.transrgb);
					else
						spg290_blit_character(bitmap, cliprect, m_ppu.txs[l].control, m_ppu.txs[l].attribute, m_ppu.txs[l].posy & 0x1ff, m_ppu.txs[l].posx & 0x1ff, m_ppu.txs[l].nptr, m_ppu.txs[l].buf_start[0], m_ppu.transrgb);
				}

			// draw the sprites
			for (int i=0; i<=(m_ppu.sprite_max & 0x1ff); i++)
			{
				if (((m_ppu.sprites[i].attribute >> 13) & 3) == depth)
					spg290_blit_sprite(bitmap, cliprect, m_ppu.sprites[i].control, m_ppu.sprites[i].attribute, m_ppu.palettes, m_ppu.sprite_buf_start);
			}
		}
	}
	else
	{
		bitmap.fill(rgb_t::black, cliprect);
	}

	return 0;
}

void hyperscan_state::spg290_timers_update()
{
	for(auto & elem : m_timers)
		if (elem.control & 0x80000000)
		{
			if (((elem.control2 >> 30) & 0x03) == 0x00)
			{
				if (elem.counter == 0xffff)
				{
					elem.counter = elem.preload;
					if (elem.control & 0x08000000)
					{
						elem.control |= 0x04000000;
						m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 56);
					}
				}
				else
					elem.counter++;
			}
			else
			{
				// TODO: capture, comparison and PWM mode

			}
		}
}

void hyperscan_state::spg290_vblank_irq(screen_device &screen, bool state)
{
	if (state && m_ppu.irq_control & 0x01)      // VBlanking Start IRQ
	{
		m_ppu.irq_status |= 0x01;
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 53);
	}
	else if (!state && m_ppu.irq_control & 0x02) // VBlanking End IRQ
	{
		m_ppu.irq_status |= 0x02;
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 53);
	}
}


void hyperscan_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SPG290:
		spg290_timers_update();
		break;
	case TIMER_I2C:
		if ((m_i2c.config & 0x40) && (m_i2c.config & 0x01))
		{
			// TODO: replace with real I2C emulation
			m_i2c.rdata = 0;

			m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 39);
		}
		break;
	}
}

static ADDRESS_MAP_START(spg290_mem, AS_PROGRAM, 32, hyperscan_state)
	ADDRESS_MAP_GLOBAL_MASK(0x1fffffff)
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_MIRROR(0x07000000)
	AM_RANGE(0x08000000, 0x09ffffff) AM_READWRITE(spg290_regs_r, spg290_regs_w)
	AM_RANGE(0x0a000000, 0x0a003fff) AM_RAM                         // internal SRAM
	AM_RANGE(0x0b000000, 0x0b007fff) AM_ROM AM_REGION("spg290", 0)  // internal ROM
	AM_RANGE(0x1e000000, 0x1e0fffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x0e000000)
	AM_RANGE(0x1f000000, 0x1f0fffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x0e000000)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( hyperscan )
INPUT_PORTS_END


void hyperscan_state::machine_start()
{
	m_update_timer = timer_alloc(TIMER_SPG290);
	m_i2c_timer = timer_alloc(TIMER_I2C);
}

void hyperscan_state::machine_reset()
{
	memset(&m_ppu, 0, sizeof(spg290_ppu));
	memset(&m_miu, 0, sizeof(spg290_miu));
	memset(&m_i2c, 0, sizeof(m_i2c));
	m_i2c_timer->adjust(attotime::never, 0, attotime::never);

	// disable JTAG
	m_maincpu->set_state_int(SCORE_CR + 29, 0x20000000);
}


static MACHINE_CONFIG_START( hyperscan, hyperscan_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SCORE7, XTAL_27MHz * 4)   // 108MHz S+core 7
	MCFG_CPU_PROGRAM_MAP(spg290_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(hyperscan_state, spg290_screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_VBLANK_DRIVER(hyperscan_state, spg290_vblank_irq)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( hs )
	ROM_REGION( 0x100000, "bios", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("hyperscan.bin", 0x000000, 0x100000, CRC(ce346a14) SHA1(560cb747e7193e6781d4b8b0bd4d7b45d3d28690))

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("spg290.bin", 0x000000, 0x008000, NO_DUMP)     // 256Kbit SPG290 internal ROM
ROM_END


/* Driver */

/*  YEAR  NAME  PARENT  COMPAT   MACHINE    INPUT   INIT    COMPANY   FULLNAME     FLAGS */
COMP( 2006, hs,   0,  0,  hyperscan ,  hyperscan , driver_device,   0, "Mattel",   "HyperScan",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
