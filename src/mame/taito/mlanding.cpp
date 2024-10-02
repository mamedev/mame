// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, Philip Bennett
/***************************************************************************

    Midnight Landing

    driver by Tomasz Slanina, Phil Bennett & Angelo Salese
    Based on early work by David Haywood

    CPU Board quick layout:
    |----------------------------------------|
    |    68000CP8        PAL     DSW  DSW    |
    |    *   *                             J|--|
    |    22  23   5165              uPD4701 |--|
    |   6264 6264 5165     TMS32020 uPD4701 |--|
    |                                       |--|
    |                              PC050CM  |--|
    |A                          MB3731 x 3  |--|
    |    5165 5165              VR1 VR2 VR3 |--|
    | PAL                                   |--|
    | PAL               PAL                 |--|
    |                                        |
    | 16.000MHz   30   YM2151              R |
    |B    *   *   31    384KHz               |
    |     24  25  32   5205 5205             |
    | #   26  27  33                         |
    |     28  29  34                         |
    |                              36        |
    |                PAL           PC060HA   |
    |PAL             PAL     35    Z80 CTC   |
    |    68000CP8    PAL     Z80             |
    |----------------------------------------|

        A, B, R are flatcable connectors, and J is for Jamma
        All numbered roms are prefixed with B09
        XTALs are 16.0000MHz, 384KHz
        VR1, VR2 & VR3 are sound pots
        # denotes a MB3771 Power Supply Monitor
        * denotes unpopulated ROM sockets


    To do:
        * Determine correct video timing
        * Unknown sound writes (volume and body sonic control?)
        * Better document mecha drive CPU

****************************************************************************/

#include "emu.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "cpu/m68000/m68000.h"
#include "cpu/tms32025/tms32025.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "taitoio_yoke.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

/*************************************
 *
 *  Driver state
 *
 *************************************/

class mlanding_state : public driver_device
{
public:
	mlanding_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_dsp(*this, "dsp"),
		m_audiocpu(*this, "audiocpu"),
		m_mechacpu(*this, "mechacpu"),
		m_yoke(*this, "yokectrl"),
		m_msm(*this, "msm%u", 1U),
		m_ctc(*this, "ctc"),
		m_dma_bank(*this, "dma_ram"),
		m_msm_rom(*this, "adpcm%u", 1U),
		m_g_ram(*this, "g_ram"),
		m_cha_ram(*this, "cha_ram"),
		m_dot_ram(*this, "dot_ram"),
		m_power_ram(*this, "power_ram"),
		m_palette(*this, "palette"),
		m_io_dswa(*this, "DSWA"),
		m_io_dswb(*this, "DSWB"),
		m_io_limit(*this, "LIMIT%u", 0U)
	{ }

	void mlanding(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr u32 c_dma_bank_words = 0x2000;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_dsp;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mechacpu;
	required_device<taitoio_yoke_device> m_yoke;
	required_device_array<msm5205_device, 2> m_msm;
	required_device<z80ctc_device> m_ctc;

	required_memory_bank m_dma_bank;
	required_region_ptr_array<u8, 2> m_msm_rom;

	required_shared_ptr<u16> m_g_ram;
	required_shared_ptr<u16> m_cha_ram;
	required_shared_ptr<u16> m_dot_ram;
	required_shared_ptr<u8> m_power_ram;

	required_device<palette_device> m_palette;

	required_ioport m_io_dswa;
	required_ioport m_io_dswb;
	required_ioport_array<2> m_io_limit;

	std::unique_ptr<u16[]> m_dma_ram;
	u8 m_dma_cpu_bank = 0;
	u8 m_dma_busy = 0;
	u16 m_dsp_hold_signal = 0;
	emu_timer *m_dma_done_timer = nullptr;

	u32 m_msm_pos[2] = { };
	u8 m_msm_reset[2] = { };
	u8 m_msm_nibble[2] = { };
	u8 m_msm2_vck = 0;
	u8 m_msm2_vck2 = 0;

	TIMER_CALLBACK_MEMBER(dma_complete);
	void dma_start_w(u16 data = 0);
	void dma_stop_w(u16 data = 0);
	void output_w(u16 data);
	u16 input_r();
	u8 analog1_lsb_r();
	u8 analog2_lsb_r();
	u8 analog3_lsb_r();
	u8 analog1_msb_r();
	u8 analog2_msb_r();
	u8 analog3_msb_r();
	u8 power_ram_r(offs_t offset);
	void power_ram_w(offs_t offset, u8 data);

	void dsp_control_w(u16 data);
	u16 dsp_hold_signal_r();

	void sound_bankswitch_w(u8 data);
	void msm5205_1_start_w(u8 data = 0);
	void msm5205_1_stop_w(u8 data = 0);
	void msm5205_1_addr_lo_w(u8 data);
	void msm5205_1_addr_hi_w(u8 data);
	void msm5205_2_start_w(u8 data);
	void msm5205_2_stop_w(u8 data);
	void msm5205_1_vck(int state);
	void z80ctc_to0(int state);

	u8 motor_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 exec_dma();
	void msm5205_update(unsigned chip);

	void audio_map_io(address_map &map) ATTR_COLD;
	void audio_map_prog(address_map &map) ATTR_COLD;
	void dsp_map_data(address_map &map) ATTR_COLD;
	void dsp_map_prog(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void mecha_map_prog(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Initialization
 *
 *************************************/

void mlanding_state::machine_start()
{
	// Allocate two DMA RAM banks
	m_dma_ram = make_unique_clear<u16[]>(c_dma_bank_words * 2);
	m_dma_bank->configure_entries(0, 2, m_dma_ram.get(), c_dma_bank_words * 2);

	// Register state for saving
	save_pointer(NAME(m_dma_ram), c_dma_bank_words * 2);
	save_item(NAME(m_dma_cpu_bank));
	save_item(NAME(m_dma_busy));
	save_item(NAME(m_dsp_hold_signal));
	save_item(NAME(m_msm_pos));
	save_item(NAME(m_msm_reset));
	save_item(NAME(m_msm_nibble));
	save_item(NAME(m_msm2_vck));
	save_item(NAME(m_msm2_vck2));

	// Allocate DMA timers
	m_dma_done_timer = timer_alloc(FUNC(mlanding_state::dma_complete), this);
}


void mlanding_state::machine_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_mechacpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_dma_cpu_bank = 0;
	m_dma_bank->set_entry(m_dma_cpu_bank);

	m_dsp_hold_signal = 0;

	m_msm_pos[0] = 0;
	m_msm_pos[1] = 0;
	m_msm_reset[0] = 0;
	m_msm_reset[1] = 0;
	m_msm[0]->reset_w(1);
	m_msm[1]->reset_w(1);
	m_msm2_vck = 0;
	m_msm2_vck2 = 0;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

u32 mlanding_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pens = m_palette->pens();

	for (u32 y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		u16 const *src = &m_g_ram[(112 + y) * 512 + cliprect.min_x];
		u16 *dst = &bitmap.pix(y, cliprect.min_x);

		for (u32 x = cliprect.min_x; x <= cliprect.max_x; ++x)
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

void mlanding_state::dma_start_w(u16 data)
{
	m_dma_cpu_bank ^= 1;
	m_dma_bank->set_entry(m_dma_cpu_bank);

	const u32 pixels = exec_dma();

	if (pixels)
	{
		m_dma_busy = 1;

		// This is a rather crude estimate!
		m_dma_done_timer->adjust(attotime::from_hz(16000000) * pixels);
	}
}


void mlanding_state::dma_stop_w(u16 data)
{
	m_dma_busy = 0;
	m_dma_done_timer->adjust(attotime::never);
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
u32 mlanding_state::exec_dma()
{
	u32 pixcnt = 0;
	const u32 gram_mask = m_g_ram.bytes() - 1;
	const u16 *dma_ram = &m_dma_ram[(m_dma_cpu_bank ^ 1) * c_dma_bank_words];

	// Process the entries in DMA RAM
	for (u32 offs = 0; offs < c_dma_bank_words; offs += 4)
	{
		const u16 attr = dma_ram[offs];

		if (attr == 0)
			continue;

		u16 code = attr & 0x1fff;

		const u16 xword = dma_ram[offs + 1];
		const u16 yword = dma_ram[offs + 2];

		const u16 x = xword & 0x1ff;
		const u16 y = yword & 0x1ff;
		const u16 sx = ((xword >> 11) & 0x1f) + 1;
		const u16 sy = ((yword >> 11) & 0x1f) + 1;

		const u8 colour = dma_ram[offs + 3] & 0xff;

		if ((attr & 0x2000) == 0)
		{
			// Normal draw mode
			const u8 basepix = colour << 4;

			for (u32 j = 0; j < sx; ++j)
			{
				for (u32 k = 0; k < sy; ++k)
				{
					// Draw an 8x8 tile
					for (u32 y1 = 0; y1 < 8; ++y1)
					{
						const u16 *src = &m_cha_ram[(code * 2 * 8) + y1 * 2];
						u32 byteaddr = ((y + k * 8 + y1) * 512 + (j * 8 + x)) * 2;

						u8 *pixdata = reinterpret_cast<u8 *>(m_g_ram.target()) + BYTE_XOR_BE(1);

						const u8 p2 = *src & 0xff;
						const u8 p1 = *src++ >> 8;
						const u8 p4 = *src;
						const u8 p3 = *src++ >> 8;

						// Draw 8 pixels
						for (u32 x1 = 0; x1 < 8; ++x1)
						{
							const u16 pix = (BIT(p4, x1) << 3) | (BIT(p3, x1) << 2) | (BIT(p2, x1) << 1) | BIT(p1, x1);

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
			for (u32 y1 = 0; y1 < sy * 8; ++y1)
			{
				u32 byteaddr = (((y + y1) * 512) + x) * 2;

				if ((attr & 0x4000) == 0)
				{
					// Clear pixel data
					u8 *pixdata = reinterpret_cast<u8 *>(m_g_ram.target()) + BYTE_XOR_BE(1);

					for (u32 x1 = 0; x1 < sx * 8; ++x1)
					{
						pixdata[byteaddr & gram_mask] = colour;
						byteaddr += 2;
						++pixcnt;
					}
				}
				else
				{
					// Clear palette data
					u8 *paldata = reinterpret_cast<u8 *>(m_g_ram.target()) + BYTE_XOR_BE(0);

					for (u32 x1 = 0; x1 < sx * 8; ++x1)
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


TIMER_CALLBACK_MEMBER(mlanding_state::dma_complete)
{
	m_dma_busy = 0;
}



/*************************************
 *
 *  I/O
 *
 *************************************/

u16 mlanding_state::input_r()
{
	/*
	    FEDCBA98 76543210
	    ........ xxxxxxxx   DSWA
	    .xxxxxxx ........   DSWB
	    x....... ........   DMA busy
	*/

	const u8 dswa = m_io_dswa->read();
	const u8 dswb = m_io_dswb->read() & 0x7f;
	return m_dma_busy << 15 | dswb << 8 | dswa;
}


void mlanding_state::output_w(u16 data)
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
	machine().bookkeeping().coin_counter_w(0, data & 4);
	machine().bookkeeping().coin_counter_w(1, data & 8);
}



/*************************************
 *
 *  Analog controls
 *
 *************************************/

u8 mlanding_state::analog1_msb_r()
{
	return (m_yoke->throttle_r() >> 4) & 0xff;
}


u8 mlanding_state::analog2_msb_r()
{
	return (m_yoke->stickx_r() >> 4) & 0xff;
}


u8 mlanding_state::analog3_msb_r()
{
	return (m_yoke->sticky_r() >> 4) & 0xff;
}


u8 mlanding_state::analog1_lsb_r()
{
	/*
	    76543210
	    ....xxxx    Counter 1 bits 3-0
	    ...x....    Handle right
	    ..x.....    Slot up
	    .x......    Slot down
	*/

	const u8 res = (m_io_limit[0]->read() & 0x70) | (m_yoke->throttle_r() & 0xf);

	return res;
}


u8 mlanding_state::analog2_lsb_r()
{
	/*
	    76543210
	    ....xxxx    Counter 2 bits 3-0
	*/
	return m_yoke->stickx_r() & 0x0f;
}


u8 mlanding_state::analog3_lsb_r()
{
	/*
	    76543210
	    ....xxxx    Counter 3 bits 3-0
	    ...x....    Handle down
	    ..x.....    Handle left
	    .x......    Handle up
	*/
	const u8 res = (m_io_limit[1]->read() & 0x70) | (m_yoke->sticky_r() & 0xf);

	return res;
}


/*************************************
 *
 *  DSP control
 *
 *************************************/

u16 mlanding_state::dsp_hold_signal_r()
{
	return m_dsp_hold_signal;
}


void mlanding_state::dsp_control_w(u16 data)
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

void mlanding_state::sound_bankswitch_w(u8 data)
{
	// Unused?
}


void mlanding_state::msm5205_update(unsigned chip)
{
	if (m_msm_reset[chip])
		return;

	const u8 data = m_msm_rom[chip][m_msm_pos[chip]];

	m_msm[chip]->data_w((m_msm_nibble[chip] ? data : data >> 4) & 0xf);

	if (m_msm_nibble[chip])
		++m_msm_pos[chip];

	m_msm_nibble[chip] ^= 1;
}


void mlanding_state::msm5205_1_vck(int state)
{
	if (state)
		msm5205_update(0);
}


void mlanding_state::z80ctc_to0(int state)
{
	if (m_msm2_vck2 && !state)
	{
		// CTC output is divided by 2
		if (m_msm2_vck)
		{
			m_msm[1]->vclk_w(1);
		}
		else
		{
			// Update on falling edge of /VCK
			msm5205_update(1);

			// Handle looping
			if (m_msm_pos[1] == 0x2000)
			{
				m_msm_pos[1] = 0;
				m_msm[1]->reset_w(1);
				m_msm[1]->vclk_w(0);
				m_msm[1]->reset_w(0);
			}
			else
			{
				m_msm[1]->vclk_w(0);
			}
		}

		m_msm2_vck ^= 1;
	}
	m_msm2_vck2 = state;
}


void mlanding_state::msm5205_1_start_w(u8 data)
{
	m_msm_reset[0] = 0;
	m_msm[0]->reset_w(0);
}


void mlanding_state::msm5205_1_stop_w(u8 data)
{
	m_msm_reset[0] = 1;
	m_msm_nibble[0] = 0;
	m_msm_pos[0] &= ~0xff;
	m_msm[0]->reset_w(1);
}


void mlanding_state::msm5205_1_addr_lo_w(u8 data)
{
	m_msm_pos[0] &= ~0x0ff00;
	m_msm_pos[0] |= data << 8;
}


void mlanding_state::msm5205_1_addr_hi_w(u8 data)
{
	m_msm_pos[0] &= ~0x70000;
	m_msm_pos[0] |= (data & 7) << 16;
}


void mlanding_state::msm5205_2_start_w(u8 data)
{
	m_msm_reset[1] = 0;
	m_msm[1]->reset_w(0);
}


void mlanding_state::msm5205_2_stop_w(u8 data)
{
	m_msm_reset[1] = 1;
	m_msm_nibble[1] = 0;
	m_msm[1]->reset_w(1);
}



/*************************************
 *
 *  Mecha drive (motorized cabinet)
 *
 *************************************/

u8 mlanding_state::power_ram_r(offs_t offset)
{
	return m_power_ram[offset];
}


void mlanding_state::power_ram_w(offs_t offset, u8 data)
{
	m_power_ram[offset] = data;
}


u8 mlanding_state::motor_r()
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

void mlanding_state::main_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x080000, 0x08ffff).ram();
	map(0x100000, 0x17ffff).ram().share(m_g_ram);
	map(0x180000, 0x1bffff).ram().share(m_cha_ram);
	map(0x1c0000, 0x1c3fff).bankrw(m_dma_bank);
	map(0x1c4000, 0x1cffff).ram().share("sub_com_ram");
	map(0x1d0000, 0x1d0001).w(FUNC(mlanding_state::dma_start_w));
	map(0x1d0002, 0x1d0003).w(FUNC(mlanding_state::dma_stop_w));
	map(0x200000, 0x20ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x240004, 0x240005).nopr(); // Watchdog
	map(0x240006, 0x240007).r(FUNC(mlanding_state::input_r));
	map(0x280000, 0x280fff).rw(FUNC(mlanding_state::power_ram_r), FUNC(mlanding_state::power_ram_w)).umask16(0x00ff);
	map(0x290000, 0x290001).portr("IN1");
	map(0x290002, 0x290003).portr("IN0");
	map(0x2a0000, 0x2a0001).w(FUNC(mlanding_state::output_w));
	map(0x2b0001, 0x2b0001).r(FUNC(mlanding_state::analog1_msb_r));
	map(0x2b0003, 0x2b0003).r(FUNC(mlanding_state::analog1_lsb_r));
	map(0x2b0005, 0x2b0005).r(FUNC(mlanding_state::analog2_msb_r));
	map(0x2b0007, 0x2b0007).r(FUNC(mlanding_state::analog2_lsb_r));
	map(0x2c0001, 0x2c0001).r(FUNC(mlanding_state::analog3_msb_r));
	map(0x2c0003, 0x2c0003).r(FUNC(mlanding_state::analog3_lsb_r));
	map(0x2d0000, 0x2d0001).nopr();
	map(0x2d0001, 0x2d0001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x2d0003, 0x2d0003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
}



/*************************************
 *
 *  Sub CPU memory handlers
 *
 *************************************/

void mlanding_state::sub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x040000, 0x043fff).ram();
	map(0x050000, 0x0507ff).ram().share("dsp_prog");
	map(0x060000, 0x060001).w(FUNC(mlanding_state::dsp_control_w));
	map(0x1c0000, 0x1c3fff).bankrw(m_dma_bank);
	map(0x1c4000, 0x1cffff).ram().share("sub_com_ram");
	map(0x200000, 0x2007ff).ram();
	map(0x200800, 0x203fff).ram().share(m_dot_ram);
}



/*************************************
 *
 *  DSP memory handlers
 *
 *************************************/

void mlanding_state::dsp_map_prog(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("dsp_prog");
}

void mlanding_state::dsp_map_data(address_map &map)
{
	map(0x0400, 0x1fff).ram().share(m_dot_ram);
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

void mlanding_state::audio_map_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xb000, 0xb000).w(FUNC(mlanding_state::msm5205_2_start_w));
	map(0xc000, 0xc000).w(FUNC(mlanding_state::msm5205_2_stop_w));
	map(0xd000, 0xd000).w(FUNC(mlanding_state::msm5205_1_start_w));
	map(0xe000, 0xe000).w(FUNC(mlanding_state::msm5205_1_stop_w));
	map(0xf000, 0xf000).w(FUNC(mlanding_state::msm5205_1_addr_lo_w));
	map(0xf200, 0xf200).w(FUNC(mlanding_state::msm5205_1_addr_hi_w));
	map(0xf400, 0xf400).nopw();
	map(0xf600, 0xf600).nopw(); // MSM5205 2 volume?
	map(0xf800, 0xf800).nopw();
	map(0xfa00, 0xfa00).nopw();
}

void mlanding_state::audio_map_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}



/*************************************
 *
 *  Mecha CPU memory handlers
 *
 *************************************/

void mlanding_state::mecha_map_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8fff).ram().share(m_power_ram);
	map(0x9000, 0x9003).nopw();
	map(0x9800, 0x9805).r(FUNC(mlanding_state::motor_r));
}



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

	// despite what the service mode claims limits are really active low.
	PORT_START("LIMIT0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("yokectrl", taitoio_yoke_device, handle_right_r )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("yokectrl", taitoio_yoke_device, slot_up_r )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("yokectrl", taitoio_yoke_device, slot_down_r )

	PORT_START("LIMIT1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("yokectrl", taitoio_yoke_device, handle_down_r )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("yokectrl", taitoio_yoke_device, handle_left_r )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("yokectrl", taitoio_yoke_device, handle_up_r )
INPUT_PORTS_END

static INPUT_PORTS_START( mlandingj )
	PORT_INCLUDE(mlanding)

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Language ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mlanding_state::mlanding(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // TS68000CP8
	m_maincpu->set_addrmap(AS_PROGRAM, &mlanding_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(mlanding_state::irq6_line_hold));

	M68000(config, m_subcpu, 16_MHz_XTAL / 2); // TS68000CP8
	m_subcpu->set_addrmap(AS_PROGRAM, &mlanding_state::sub_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4); // Z08040004PSC
	m_audiocpu->set_addrmap(AS_PROGRAM, &mlanding_state::audio_map_prog);
	m_audiocpu->set_addrmap(AS_IO, &mlanding_state::audio_map_io);

	Z80(config, m_mechacpu, 4000000); // ?
	m_mechacpu->set_addrmap(AS_PROGRAM, &mlanding_state::mecha_map_prog);
	m_mechacpu->set_vblank_int("screen", FUNC(mlanding_state::irq0_line_hold));

	auto &dsp(TMS32020(config, m_dsp, 16_MHz_XTAL)); // TMS32020GBL
	dsp.set_addrmap(AS_PROGRAM, &mlanding_state::dsp_map_prog);
	dsp.set_addrmap(AS_DATA, &mlanding_state::dsp_map_data);
	dsp.hold_in_cb().set(FUNC(mlanding_state::dsp_hold_signal_r));
	dsp.hold_ack_out_cb().set_nop();

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->zc_callback<0>().set(FUNC(mlanding_state::z80ctc_to0));

	pc060ha_device& ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	config.set_maximum_quantum(attotime::from_hz(600));

	TAITOIO_YOKE(config, m_yoke, 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 640, 0, 512, 462, 0, 400); // Estimated
	screen.set_screen_update(FUNC(mlanding_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 32768);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.port_write_handler().set(FUNC(mlanding_state::sound_bankswitch_w));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	MSM5205(config, m_msm[0], 384_kHz_XTAL);
	m_msm[0]->vck_callback().set(FUNC(mlanding_state::msm5205_1_vck)); // VCK function
	m_msm[0]->set_prescaler_selector(msm5205_device::S48_4B); // 8 kHz, 4-bit
	m_msm[0]->add_route(ALL_OUTPUTS, "mono", 0.80);

	MSM5205(config, m_msm[1], 384_kHz_XTAL);
	m_msm[1]->set_prescaler_selector(msm5205_device::SEX_4B); // Slave mode, 4-bit
	m_msm[1]->add_route(ALL_OUTPUTS, "mono", 0.10);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mlanding )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b09_29.ic44", 0x00000, 0x10000, CRC(ab3f38f3) SHA1(4357112ca11a8e7bfe08ba99ac3bddac046c230a) ) // label needs verifying
	ROM_LOAD16_BYTE( "b09_28.ic27", 0x00001, 0x10000, CRC(21e7a8f6) SHA1(860d3861d4375866cd27d426d546ddb2894a6629) ) // label needs verifying
	ROM_LOAD16_BYTE( "b09_27.ic45", 0x20000, 0x10000, CRC(b02f1805) SHA1(b8050f955c7070dc9b962db329b5b0ee8b2acb70) )
	ROM_LOAD16_BYTE( "b09_26.ic28", 0x20001, 0x10000, CRC(d57ff428) SHA1(8ff1ab666b06fb873f1ba9b25edf4cd49b9861a1) )
	ROM_LOAD16_BYTE( "b09_25.ic46", 0x40000, 0x10000, CRC(ff59f049) SHA1(aba490a28aba03728415f34d321fd599c31a5fde) )
	ROM_LOAD16_BYTE( "b09_24.ic29", 0x40001, 0x10000, CRC(9bc3e1b0) SHA1(6d86804327df11a513a0f06dceb57b83b34ac007) )

	ROM_REGION( 0x20000, "subcpu", 0 )
	ROM_LOAD16_BYTE( "b09_23.ic56", 0x00000, 0x10000, CRC(81b2c871) SHA1(a085bc528c63834079469db6ae263a5b9b984a7c) )
	ROM_LOAD16_BYTE( "b09_22.ic39", 0x00001, 0x10000, CRC(36923b42) SHA1(c31d7c45a563cfc4533379f69f32889c79562534) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b09_35.ic80", 0x00000, 0x08000, CRC(b85915c5) SHA1(656e97035ae304f84e90758d0dd6f0616c40f1db) )

	ROM_REGION( 0x10000, "mechacpu", 0 )
	ROM_LOAD( "b09_37.epr", 0x00000, 0x08000, CRC(4bdf15ed) SHA1(b960208e63cede116925e064279a6cf107aef81c) )

	ROM_REGION( 0x80000, "adpcm1", 0 )
	ROM_LOAD( "b09_34.ic62", 0x00000, 0x10000, CRC(0899666f) SHA1(032e3ddd4caa48f82592570616e16c084de91f3e) )
	ROM_LOAD( "b09_33.ic63", 0x10000, 0x10000, CRC(f5cac954) SHA1(71abdc545e0196ad4d357af22dd6312d10a1323f) )
	ROM_LOAD( "b09_32.ic64", 0x20000, 0x10000, CRC(4721dc59) SHA1(faad75d577344e9ba495059040a2cf0647567426) )
	ROM_LOAD( "b09_31.ic65", 0x30000, 0x10000, CRC(9c4a82bf) SHA1(daeac620c636013a36595ce9f37e84e807f88977) )
	ROM_LOAD( "b09_30.ic66", 0x40000, 0x10000, CRC(214a30e2) SHA1(3dcc3a89ed52e4dbf232d2a92a3e64975b46c2dd) )

	ROM_REGION( 0x2000, "adpcm2", 0 )
	ROM_LOAD( "b09_36.ic111", 0x00000, 0x02000, CRC(51fd3a77) SHA1(1fcbadf1877e25848a1d1017322751560a4823c0) )
ROM_END

ROM_START( mlandingj )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b09_29-2.ic44", 0x00000, 0x10000, CRC(c78d55cd) SHA1(0126524310048a55f4c0c361a1673cc259e461ba) ) // 27512
	ROM_LOAD16_BYTE( "b09_28-3.ic27", 0x00001, 0x10000, CRC(7098c18d) SHA1(ee5b386fed9df5edfd19cb967fd01e9e7fd9ad08) ) // 27512
	ROM_LOAD16_BYTE( "b09_27.ic45",   0x20000, 0x10000, CRC(b02f1805) SHA1(b8050f955c7070dc9b962db329b5b0ee8b2acb70) )
	ROM_LOAD16_BYTE( "b09_26.ic28",   0x20001, 0x10000, CRC(d57ff428) SHA1(8ff1ab666b06fb873f1ba9b25edf4cd49b9861a1) )
	ROM_LOAD16_BYTE( "b09_25.ic46",   0x40000, 0x10000, CRC(ff59f049) SHA1(aba490a28aba03728415f34d321fd599c31a5fde) )
	ROM_LOAD16_BYTE( "b09_24.ic29",   0x40001, 0x10000, CRC(9bc3e1b0) SHA1(6d86804327df11a513a0f06dceb57b83b34ac007) )

	ROM_REGION( 0x20000, "subcpu", 0 )
	ROM_LOAD16_BYTE( "b09_23.ic56", 0x00000, 0x10000, CRC(81b2c871) SHA1(a085bc528c63834079469db6ae263a5b9b984a7c) )
	ROM_LOAD16_BYTE( "b09_22.ic39", 0x00001, 0x10000, CRC(36923b42) SHA1(c31d7c45a563cfc4533379f69f32889c79562534) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b09_35.ic80", 0x00000, 0x08000, CRC(b85915c5) SHA1(656e97035ae304f84e90758d0dd6f0616c40f1db) )

	ROM_REGION( 0x10000, "mechacpu", 0 )
	ROM_LOAD( "b09_37.epr", 0x00000, 0x08000, CRC(4bdf15ed) SHA1(b960208e63cede116925e064279a6cf107aef81c) )

	ROM_REGION( 0x80000, "adpcm1", 0 )
	ROM_LOAD( "b09_34.ic62", 0x00000, 0x10000, CRC(0899666f) SHA1(032e3ddd4caa48f82592570616e16c084de91f3e) )
	ROM_LOAD( "b09_33.ic63", 0x10000, 0x10000, CRC(f5cac954) SHA1(71abdc545e0196ad4d357af22dd6312d10a1323f) )
	ROM_LOAD( "b09_32.ic64", 0x20000, 0x10000, CRC(4721dc59) SHA1(faad75d577344e9ba495059040a2cf0647567426) )
	ROM_LOAD( "b09_31.ic65", 0x30000, 0x10000, CRC(9c4a82bf) SHA1(daeac620c636013a36595ce9f37e84e807f88977) )
	ROM_LOAD( "b09_30.ic66", 0x40000, 0x10000, CRC(214a30e2) SHA1(3dcc3a89ed52e4dbf232d2a92a3e64975b46c2dd) )

	ROM_REGION( 0x2000, "adpcm2", 0 )
	ROM_LOAD( "b09_36.ic111", 0x00000, 0x02000, CRC(51fd3a77) SHA1(1fcbadf1877e25848a1d1017322751560a4823c0) )
ROM_END

} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, mlanding,  0,        mlanding, mlanding,  mlanding_state, empty_init, ROT0, "Taito America Corporation", "Midnight Landing (Germany)",      MACHINE_SUPPORTS_SAVE ) // Japanese or German selectable via dip-switch. Copyright changes accordingly.
GAME( 1987, mlandingj, mlanding, mlanding, mlandingj, mlanding_state, empty_init, ROT0, "Taito Corporation",         "Midnight Landing (Japan, rev 3)", MACHINE_SUPPORTS_SAVE ) // Japanese or English selectable via dip-switch. Copyright changes accordingly.
