// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, Philip Bennett, hap
/***************************************************************************

    Midnight Landing

    driver by Tomasz Slanina, Phil Bennett & Angelo Salese
    Based on early work by David Haywood

    CPU Board quick layout:
    |------------------------------------|
    |    68000P8             DSW  DSW    |
    |                                  J|--|
    |                          uPD4701? |--|
    |    x   x        TMS32025 uPD4701? |--|
    |                                   |--|
    |                        TC0060DCA? |--|
    |A                                  |--|
    |                                   |--|
    |                                   |--|
    |                                   |--|
    |                                    |
    | XTAL       x    YM2151           R |
    |B           x                       |
    |    x   x   x    5205 5205          |
    |    x   x   x                       |
    |    x   x   x           x           |
    |                        PC060HA     |
    |                                    |
    |                 x      Z80 CTC     |
    |    68000P8      Z80                |
    |------------------------------------|
        * A, B, R are flatcable connectors, and J is for Jamma
        * XTAL is assumed to be around 32MHz
        * x are ROM chips, PCB photo was too small to determine which


    To do:
        * Find Japanese version
        * Determine correct CPU and video timings
        * Unknown sound writes (volume and body sonic control?)
        * Better document mecha drive CPU

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32025/tms32025.h"
#include "cpu/z80/z80.h"
#include "includes/taitoipt.h"
#include "machine/z80ctc.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"



/*************************************
 *
 *  Driver state
 *
 *************************************/

class mlanding_state : public driver_device
{
public:
	enum
	{
		TIMER_DMA_COMPLETE
	};

	static const UINT32 c_dma_bank_words = 0x2000;

	mlanding_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_dsp(*this, "dsp"),
		m_audiocpu(*this, "audiocpu"),
		m_mechacpu(*this, "mechacpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_ctc(*this, "ctc"),
		m_dma_bank(*this, "dma_ram"),
		m_msm1_rom(*this, "adpcm1"),
		m_msm2_rom(*this, "adpcm2"),
		m_g_ram(*this, "g_ram"),
		m_cha_ram(*this, "cha_ram"),
		m_dot_ram(*this, "dot_ram"),
		m_power_ram(*this, "power_ram"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_dsp;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mechacpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<z80ctc_device> m_ctc;

	required_memory_bank    m_dma_bank;
	required_region_ptr<UINT8> m_msm1_rom;
	required_region_ptr<UINT8> m_msm2_rom;

	required_shared_ptr<UINT16> m_g_ram;
	required_shared_ptr<UINT16> m_cha_ram;
	required_shared_ptr<UINT16> m_dot_ram;
	required_shared_ptr<UINT8>  m_power_ram;

	required_device<palette_device> m_palette;

	std::unique_ptr<UINT16[]> m_dma_ram;
	UINT8   m_dma_cpu_bank;
	UINT8   m_dma_busy;
	UINT16  m_dsp_hold_signal;

	UINT32  m_msm_pos[2];
	UINT8   m_msm_reset[2];
	UINT8   m_msm_nibble[2];
	UINT8   m_msm2_vck;
	UINT8   m_msm2_vck2;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE16_MEMBER(dma_start_w);
	DECLARE_WRITE16_MEMBER(dma_stop_w);
	DECLARE_WRITE16_MEMBER(output_w);
	DECLARE_READ16_MEMBER(input_r);
	DECLARE_READ16_MEMBER(analog1_lsb_r);
	DECLARE_READ16_MEMBER(analog2_lsb_r);
	DECLARE_READ16_MEMBER(analog3_lsb_r);
	DECLARE_READ16_MEMBER(analog1_msb_r);
	DECLARE_READ16_MEMBER(analog2_msb_r);
	DECLARE_READ16_MEMBER(analog3_msb_r);
	DECLARE_READ16_MEMBER(power_ram_r);
	DECLARE_WRITE16_MEMBER(power_ram_w);

	DECLARE_WRITE16_MEMBER(dsp_control_w);
	DECLARE_READ16_MEMBER(dsp_hold_signal_r);

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(msm5205_1_start_w);
	DECLARE_WRITE8_MEMBER(msm5205_1_stop_w);
	DECLARE_WRITE8_MEMBER(msm5205_1_addr_lo_w);
	DECLARE_WRITE8_MEMBER(msm5205_1_addr_hi_w);
	DECLARE_WRITE8_MEMBER(msm5205_2_start_w);
	DECLARE_WRITE8_MEMBER(msm5205_2_stop_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_1_vck);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to0);

	DECLARE_READ8_MEMBER(motor_r);

	UINT32 screen_update_mlanding(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 exec_dma();
	void msm5205_update(int chip);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};



/*************************************
 *
 *  Initialization
 *
 *************************************/

void mlanding_state::machine_start()
{
	// Allocate two DMA RAM banks
	m_dma_ram = std::make_unique<UINT16[]>(c_dma_bank_words * 2);
	m_dma_bank->configure_entries(0, 2, m_dma_ram.get(), c_dma_bank_words * 2);

	// Register state for saving
	save_pointer(NAME(m_dma_ram.get()), c_dma_bank_words * 2);
	save_item(NAME(m_dma_cpu_bank));
	save_item(NAME(m_dma_busy));
	save_item(NAME(m_dsp_hold_signal));
	save_item(NAME(m_msm_pos));
	save_item(NAME(m_msm_reset));
	save_item(NAME(m_msm_nibble));
	save_item(NAME(m_msm2_vck));
	save_item(NAME(m_msm2_vck2));
}


void mlanding_state::machine_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_mechacpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_dma_cpu_bank = 0;
	m_dma_bank->set_entry(m_dma_cpu_bank);

	m_dsp_hold_signal = 0;

	m_msm_reset[0] = 0;
	m_msm_reset[1] = 0;
	m_msm1->reset_w(1);
	m_msm2->reset_w(1);
	m_msm2_vck = 0;
	m_msm2_vck2 = 0;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 mlanding_state::screen_update_mlanding(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();

	for (UINT32 y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT16 *src = &m_g_ram[(112 + y) * 512 + cliprect.min_x];
		UINT16 *dst = &bitmap.pix16(y, cliprect.min_x);

		for (UINT32 x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			*dst++ = pens[*src++ & 0x3fff];
		}
	}

	return 0;
}



/*************************************
 *
 *  Video DMA
 *
 *************************************/

WRITE16_MEMBER(mlanding_state::dma_start_w)
{
	m_dma_cpu_bank ^= 1;
	membank("dma_ram")->set_entry(m_dma_cpu_bank);

	UINT32 pixels = exec_dma();

	if (pixels)
	{
		m_dma_busy = 1;

		// This is a rather crude estimate!
		timer_set(attotime::from_hz(16000000) * pixels, TIMER_DMA_COMPLETE);
	}
}


WRITE16_MEMBER(mlanding_state::dma_stop_w)
{
	m_dma_busy = 0;
	timer_set(attotime::never);
}


/*
        FEDCBA9876543210

    0   ...xxxxx xxxxxxxx       Tile index
        ..x..... ........       Clear mode
        .x...... ........       Clear pixel/palette data
        x....... ........       Transparent/opaque mode

    1   .......x xxxxxxxx       X Coordinate
        xxxxx... ........       Width in 8x8 tiles

    2   .......x xxxxxxxx       Y Coordinate
        .....xx. ........       Unused
        xxxxx... ........       Height in 8x8 tiles

    3   ........ ....xxxx       Colour
*/
UINT32 mlanding_state::exec_dma()
{
	UINT32 pixcnt = 0;
	UINT32 gram_mask = m_g_ram.bytes() - 1;
	UINT16 *dma_ram = &m_dma_ram[(m_dma_cpu_bank ^ 1) * c_dma_bank_words];

	// Process the entries in DMA RAM
	for (UINT32 offs = 0; offs < c_dma_bank_words; offs += 4)
	{
		UINT16 attr = dma_ram[offs];

		if (attr == 0)
			continue;

		UINT16 code = attr & 0x1fff;

		UINT16 xword = dma_ram[offs + 1];
		UINT16 yword = dma_ram[offs + 2];

		UINT16 x = xword & 0x1ff;
		UINT16 y = yword & 0x1ff;
		UINT16 sx = ((xword >> 11) & 0x1f) + 1;
		UINT16 sy = ((yword >> 11) & 0x1f) + 1;

		UINT8 colour = dma_ram[offs + 3] & 0xff;

		if ((attr & 0x2000) == 0)
		{
			// Normal draw mode
			UINT8 basepix = colour << 4;

			for (UINT32 j = 0; j < sx; ++j)
			{
				for (UINT32 k = 0; k < sy; ++k)
				{
					// Draw an 8x8 tile
					for (UINT32 y1 = 0; y1 < 8; ++y1)
					{
						UINT16 *src = &m_cha_ram[(code * 2 * 8) + y1 * 2];
						UINT32 byteaddr = ((y + k * 8 + y1) * 512 + (j * 8 + x)) * 2;

						UINT8 *pixdata = reinterpret_cast<UINT8 *>(m_g_ram.target()) + BYTE_XOR_BE(1);

						UINT8 p2 = *src & 0xff;
						UINT8 p1 = *src++ >> 8;
						UINT8 p4 = *src;
						UINT8 p3 = *src++ >> 8;

						// Draw 8 pixels
						for (UINT32 x1 = 0; x1 < 8; ++x1)
						{
							UINT16 pix = (BIT(p4, x1) << 3) | (BIT(p3, x1) << 2) | (BIT(p2, x1) << 1) | BIT(p1, x1);

							if ((attr & 0x8000) == 0)
							{
								// Transparency mode
								if (pix)
								{
									pixdata[byteaddr & gram_mask] = basepix | pix;
								}
							}
							else
							{
								pixdata[byteaddr & gram_mask] = basepix | pix;
							}

							byteaddr += 2;
							++pixcnt;
						}
					}
					++code;
				}
			}
		}
		else
		{
			// Set pixel or palette data
			for (UINT32 y1 = 0; y1 < sy * 8; ++y1)
			{
				UINT32 byteaddr = (((y + y1) * 512) + x) * 2;

				if ((attr & 0x4000) == 0)
				{
					// Clear pixel data
					UINT8 *pixdata = reinterpret_cast<UINT8 *>(m_g_ram.target()) + BYTE_XOR_BE(1);

					for (UINT32 x1 = 0; x1 < sx * 8; ++x1)
					{
						pixdata[byteaddr & gram_mask] = colour;
						byteaddr += 2;
						++pixcnt;
					}
				}
				else
				{
					// Clear palette data
					UINT8 *paldata = reinterpret_cast<UINT8 *>(m_g_ram.target()) + BYTE_XOR_BE(0);

					for (UINT32 x1 = 0; x1 < sx * 8; ++x1)
					{
						paldata[byteaddr & gram_mask] = colour;
						byteaddr += 2;
						++pixcnt;
					}
				}
			}
		}
	}

	return pixcnt;
}


void mlanding_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_DMA_COMPLETE:
			m_dma_busy = 0;
			break;

		default:
			assert_always(FALSE, "Unknown id in mlanding_state::device_timer");
	}
}



/*************************************
 *
 *  I/O
 *
 *************************************/

READ16_MEMBER(mlanding_state::input_r)
{
	/*
	    FEDCBA98 76543210
	    ........ xxxxxxxx   DSWA
	    .xxxxxxx ........   DSWB
	    x....... ........   DMA busy
	*/

	UINT8 dswa = ioport("DSWA")->read();
	UINT8 dswb = ioport("DSWB")->read() & 0x7f;
	return m_dma_busy << 15 | dswb << 8 | dswa;
}


WRITE16_MEMBER(mlanding_state::output_w)
{
	/*
	    76543210
	    x.......    Start lamp?
	    .x......    /Mecha CPU reset
	    ..x.....    ? (Briefly transitions from 1 to 0 at $5040, after pressing start)
	    ...x....    /Sub CPU reset
	    ....x...    Coin counter B
	    .....x..    Coin counter A
	    ......x.    /Coin lockout B
	    .......x    /Coin lockout A
	*/
	m_subcpu->set_input_line(INPUT_LINE_RESET, data & 0x10 ? CLEAR_LINE : ASSERT_LINE);
	m_mechacpu->set_input_line(INPUT_LINE_RESET, data & 0x40 ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Analog controls
 *
 *************************************/

READ16_MEMBER(mlanding_state::analog1_msb_r)
{
	return (ioport("THROTTLE")->read() >> 4) & 0xff;
}


READ16_MEMBER(mlanding_state::analog2_msb_r)
{
	return (ioport("STICK_X")->read() >> 4) & 0xff;
}


READ16_MEMBER(mlanding_state::analog3_msb_r)
{
	return (ioport("STICK_Y")->read() >> 4) & 0xff;
}


READ16_MEMBER(mlanding_state::analog1_lsb_r)
{
	/*
	    76543210
	    ....xxxx    Counter 1 bits 3-0
	    ...x....    Handle left
	    ..x.....    Slot down
	    .x......    Slot up
	*/
	UINT16 throttle = ioport("THROTTLE")->read();
	UINT16 x = ioport("STICK_X")->read();

	UINT8 res = 0x70 | (throttle & 0x0f);

	if (throttle & 0x800)
		res ^= 0x20;
	else if (throttle > 0)
		res ^= 0x40;

	if (!(x & 0x800) && x > 0)
		res ^= 0x10;

	return res;
}


READ16_MEMBER(mlanding_state::analog2_lsb_r)
{
	/*
	    76543210
	    ....xxxx    Counter 2 bits 3-0
	*/
	return ioport("STICK_X")->read() & 0x0f;
}


READ16_MEMBER(mlanding_state::analog3_lsb_r)
{
	/*
	    76543210
	    ....xxxx    Counter 3 bits 3-0
	    ...x....    Handle up
	    ..x.....    Handle right
	    .x......    Handle down
	*/
	UINT16 x = ioport("STICK_X")->read();
	UINT16 y = ioport("STICK_Y")->read();

	UINT8 res = 0x70 | (y & 0x0f);

	if (y & 0x800)
		res ^= 0x40;
	else if (y > 0)
		res ^= 0x10;

	if (x & 0x800)
		res ^= 0x20;

	return res;
}


/*************************************
 *
 *  DSP control
 *
 *************************************/

READ16_MEMBER(mlanding_state::dsp_hold_signal_r)
{
	return m_dsp_hold_signal;
}


WRITE16_MEMBER(mlanding_state::dsp_control_w)
{
	/*
	    1 after zeroing 'dot' RAM
	    3 after uploading DSP program
	*/
	m_dsp->set_input_line(INPUT_LINE_RESET, data & 0x2 ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Sound
 *
 *************************************/

WRITE8_MEMBER(mlanding_state::sound_bankswitch_w)
{
	// Unused?
}


void mlanding_state::msm5205_update(int chip)
{
	if (m_msm_reset[chip])
		return;

	const UINT8 *rom = chip ? m_msm2_rom : m_msm1_rom;
	UINT8 data = rom[m_msm_pos[chip]];
	msm5205_device *msm = chip ? m_msm2 : m_msm1;

	msm->data_w((m_msm_nibble[chip] ? data : data >> 4) & 0xf);

	if (m_msm_nibble[chip])
		++m_msm_pos[chip];

	m_msm_nibble[chip] ^= 1;
}


WRITE_LINE_MEMBER(mlanding_state::msm5205_1_vck)
{
	msm5205_update(0);
}


WRITE_LINE_MEMBER(mlanding_state::z80ctc_to0)
{
	if (m_msm2_vck2 && !state)
	{
		// CTC output is divided by 2
		if (m_msm2_vck)
		{
			m_msm2->vclk_w(1);
		}
		else
		{
			// Update on falling edge of /VCK
			msm5205_update(1);

			// Handle looping
			if (m_msm_pos[1] == 0x2000)
			{
				m_msm_pos[1] = 0;
				m_msm2->reset_w(1);
				m_msm2->vclk_w(0);
				m_msm2->reset_w(0);
			}
			else
			{
				m_msm2->vclk_w(0);
			}
		}

		m_msm2_vck ^= 1;
	}
	m_msm2_vck2 = state;
}


WRITE8_MEMBER(mlanding_state::msm5205_1_start_w)
{
	m_msm_reset[0] = 0;
	m_msm1->reset_w(0);
}


WRITE8_MEMBER(mlanding_state::msm5205_1_stop_w)
{
	m_msm_reset[0] = 1;
	m_msm_nibble[0] = 0;
	m_msm_pos[0] &= ~0xff;
	m_msm1->reset_w(1);
}


WRITE8_MEMBER(mlanding_state::msm5205_1_addr_lo_w)
{
	m_msm_pos[0] &= ~0x0ff00;
	m_msm_pos[0] |= data << 8;
}


WRITE8_MEMBER(mlanding_state::msm5205_1_addr_hi_w)
{
	m_msm_pos[0] &= ~0x70000;
	m_msm_pos[0] |= (data & 7) << 16;
}


WRITE8_MEMBER(mlanding_state::msm5205_2_start_w)
{
	m_msm_reset[1] = 0;
	m_msm2->reset_w(0);
}


WRITE8_MEMBER(mlanding_state::msm5205_2_stop_w)
{
	m_msm_reset[1] = 1;
	m_msm_nibble[1] = 0;
	m_msm2->reset_w(1);
}



/*************************************
 *
 *  Mecha drive (motorized cabinet)
 *
 *************************************/

READ16_MEMBER(mlanding_state::power_ram_r)
{
	return m_power_ram[offset];
}


WRITE16_MEMBER(mlanding_state::power_ram_w)
{
	if (ACCESSING_BITS_0_7)
		m_power_ram[offset] = data & 0xff;
}


READ8_MEMBER(mlanding_state::motor_r)
{
	/*
	    9001: RIGHT MOTOR: 1F=UP, 00=STOP, 2F=DOWN
	    9003: LEFT MOTOR:  1F=UP, 00=STOP, 2F=DOWN

	    9800: xxxx .... - Counter R 3-0
	    9801: .... xxxx - Counter R 7-4
	       ...x .... - SW R
	    9802: xxxx .... - Counter L 3-0
	    9803: .... xxxx - Counter L 7-4
	    9804: .... .... -
	    9805: ...x .... - SW L
	*/

	return 0x10;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, mlanding_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM
	AM_RANGE(0x100000, 0x17ffff) AM_RAM AM_SHARE("g_ram")
	AM_RANGE(0x180000, 0x1bffff) AM_RAM AM_SHARE("cha_ram")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAMBANK("dma_ram")
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM AM_SHARE("sub_com_ram")
	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(dma_start_w)
	AM_RANGE(0x1d0002, 0x1d0003) AM_WRITE(dma_stop_w)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x240004, 0x240005) AM_READNOP // Watchdog
	AM_RANGE(0x240006, 0x240007) AM_READ(input_r)
	AM_RANGE(0x280000, 0x280fff) AM_READWRITE(power_ram_r, power_ram_w)
	AM_RANGE(0x290000, 0x290001) AM_READ_PORT("IN1")
	AM_RANGE(0x290002, 0x290003) AM_READ_PORT("IN0")
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(output_w)
	AM_RANGE(0x2b0000, 0x2b0001) AM_READ(analog1_msb_r)
	AM_RANGE(0x2b0002, 0x2b0003) AM_READ(analog1_lsb_r)
	AM_RANGE(0x2b0004, 0x2b0005) AM_READ(analog2_msb_r)
	AM_RANGE(0x2b0006, 0x2b0007) AM_READ(analog2_lsb_r)
	AM_RANGE(0x2c0000, 0x2c0001) AM_READ(analog3_msb_r)
	AM_RANGE(0x2c0002, 0x2c0003) AM_READ(analog3_lsb_r)
	AM_RANGE(0x2d0000, 0x2d0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x2d0002, 0x2d0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
ADDRESS_MAP_END



/*************************************
 *
 *  Sub CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 16, mlanding_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x050000, 0x0503ff) AM_RAM AM_SHARE("dsp_prog")
	AM_RANGE(0x060000, 0x060001) AM_WRITE(dsp_control_w)
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAMBANK("dma_ram")
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM AM_SHARE("sub_com_ram")
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("dot_ram")
ADDRESS_MAP_END



/*************************************
 *
 *  DSP memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( dsp_map_prog, AS_PROGRAM, 16, mlanding_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("dsp_prog")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_map_data, AS_DATA, 16, mlanding_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("dot_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_map_io, AS_IO, 16, mlanding_state )
	AM_RANGE(TMS32025_HOLD, TMS32025_HOLD) AM_READ(dsp_hold_signal_r)
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITENOP
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map_prog, AS_PROGRAM, 8, mlanding_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(msm5205_2_start_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(msm5205_2_stop_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(msm5205_1_start_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(msm5205_1_stop_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(msm5205_1_addr_lo_w)
	AM_RANGE(0xf200, 0xf200) AM_WRITE(msm5205_1_addr_hi_w)
	AM_RANGE(0xf400, 0xf400) AM_WRITENOP
	AM_RANGE(0xf600, 0xf600) AM_WRITENOP // MSM5205 2 volume?
	AM_RANGE(0xf800, 0xf800) AM_WRITENOP
	AM_RANGE(0xfa00, 0xfa00) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_map_io, AS_IO, 8, mlanding_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
ADDRESS_MAP_END



/*************************************
 *
 *  Mecha CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mecha_map_prog, AS_PROGRAM, 8, mlanding_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("power_ram")
	AM_RANGE(0x9000, 0x9001) AM_WRITENOP
	AM_RANGE(0x9000, 0x9003) AM_WRITENOP
	AM_RANGE(0x9800, 0x9805) AM_READ(motor_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mlanding )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Standard ) )
	PORT_DIPSETTING(    0x00, "Deluxe" ) // with Mecha driver
	PORT_DIPNAME( 0x02, 0x02, "Coin Mode" ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x02, "Mode A (Japan)" ) /* Mode A is TAITO_COINAGE_JAPAN_OLD */
	PORT_DIPSETTING(    0x00, "Mode B (World)" ) /* Mode B is TAITO_COINAGE_WORLD */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWA:3" ) PORT_NAME("Test Mode 1")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Test Mode 2") PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SWB:7") // probably not meant to be used on German version?
	PORT_DIPSETTING(    0x40, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Door") PORT_TOGGLE
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Coin A Enable" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Coin B Enable" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("THROTTLE")
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_Z ) PORT_MINMAX(0x00800, 0x07ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("STICK_X")
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0x00800, 0x07ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("STICK_Y")
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0x00800, 0x07ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( mlanding, mlanding_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) // Appears to be 68000P8 in PCB photo
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mlanding_state, irq6_line_hold)

	MCFG_CPU_ADD("subcpu", M68000, 8000000) // Appears to be 68000P8 in PCB photo
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(audio_map_prog)
	MCFG_CPU_IO_MAP(audio_map_io)

	MCFG_CPU_ADD("mechacpu", Z80, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(mecha_map_prog)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mlanding_state, irq0_line_hold)

	MCFG_CPU_ADD("dsp", TMS32025, 32000000) // ?
	MCFG_CPU_PROGRAM_MAP(dsp_map_prog)
	MCFG_CPU_DATA_MAP(dsp_map_data)
	MCFG_CPU_IO_MAP(dsp_map_io)

	MCFG_DEVICE_ADD("ctc", Z80CTC, 4000000)
	MCFG_Z80CTC_ZC0_CB(WRITELINE(mlanding_state, z80ctc_to0))

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)

	// Estimated
	MCFG_SCREEN_RAW_PARAMS(16000000, 640, 0, 512, 462, 0, 400)
	MCFG_SCREEN_UPDATE_DRIVER(mlanding_state, screen_update_mlanding)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 32768)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(mlanding_state, sound_bankswitch_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_SOUND_ADD("msm1", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(mlanding_state, msm5205_1_vck)) // VCK function
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      // 8 kHz, 4-bit
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("msm2", MSM5205, 384000)
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)      // Slave mode, 4-bit
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mlanding )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ml_b0929.epr", 0x00000, 0x10000, CRC(ab3f38f3) SHA1(4357112ca11a8e7bfe08ba99ac3bddac046c230a))
	ROM_LOAD16_BYTE( "ml_b0928.epr", 0x00001, 0x10000, CRC(21e7a8f6) SHA1(860d3861d4375866cd27d426d546ddb2894a6629) )
	ROM_LOAD16_BYTE( "ml_b0927.epr", 0x20000, 0x10000, CRC(b02f1805) SHA1(b8050f955c7070dc9b962db329b5b0ee8b2acb70) )
	ROM_LOAD16_BYTE( "ml_b0926.epr", 0x20001, 0x10000, CRC(d57ff428) SHA1(8ff1ab666b06fb873f1ba9b25edf4cd49b9861a1) )
	ROM_LOAD16_BYTE( "ml_b0925.epr", 0x40000, 0x10000, CRC(ff59f049) SHA1(aba490a28aba03728415f34d321fd599c31a5fde) )
	ROM_LOAD16_BYTE( "ml_b0924.epr", 0x40001, 0x10000, CRC(9bc3e1b0) SHA1(6d86804327df11a513a0f06dceb57b83b34ac007) )

	ROM_REGION( 0x20000, "subcpu", 0 )
	ROM_LOAD16_BYTE( "ml_b0923.epr", 0x00000, 0x10000, CRC(81b2c871) SHA1(a085bc528c63834079469db6ae263a5b9b984a7c) )
	ROM_LOAD16_BYTE( "ml_b0922.epr", 0x00001, 0x10000, CRC(36923b42) SHA1(c31d7c45a563cfc4533379f69f32889c79562534) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ml_b0935.epr", 0x00000, 0x08000, CRC(b85915c5) SHA1(656e97035ae304f84e90758d0dd6f0616c40f1db) )

	ROM_REGION( 0x10000, "mechacpu", 0 )
	ROM_LOAD( "ml_b0937.epr", 0x00000, 0x08000, CRC(4bdf15ed) SHA1(b960208e63cede116925e064279a6cf107aef81c) )

	ROM_REGION( 0x80000, "adpcm1", 0 )
	ROM_LOAD( "ml_b0934.epr", 0x00000, 0x10000, CRC(0899666f) SHA1(032e3ddd4caa48f82592570616e16c084de91f3e) )
	ROM_LOAD( "ml_b0933.epr", 0x10000, 0x10000, CRC(f5cac954) SHA1(71abdc545e0196ad4d357af22dd6312d10a1323f) )
	ROM_LOAD( "ml_b0932.epr", 0x20000, 0x10000, CRC(4721dc59) SHA1(faad75d577344e9ba495059040a2cf0647567426) )
	ROM_LOAD( "ml_b0931.epr", 0x30000, 0x10000, CRC(9c4a82bf) SHA1(daeac620c636013a36595ce9f37e84e807f88977) )
	ROM_LOAD( "ml_b0930.epr", 0x40000, 0x10000, CRC(214a30e2) SHA1(3dcc3a89ed52e4dbf232d2a92a3e64975b46c2dd) )

	ROM_REGION( 0x2000, "adpcm2", 0 )
	ROM_LOAD( "ml_b0936.epr", 0x00000, 0x02000, CRC(51fd3a77) SHA1(1fcbadf1877e25848a1d1017322751560a4823c0) )
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, mlanding, 0, mlanding, mlanding, driver_device, 0, ROT0, "Taito America Corporation", "Midnight Landing (Germany)", MACHINE_SUPPORTS_SAVE )
