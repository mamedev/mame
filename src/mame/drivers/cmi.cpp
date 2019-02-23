// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI Series

    driver by Phil Bennett

    Systems supported:

        * CMI IIx

    To do:

    * MASTER 'TIM' test fails
    * LGTST  'TIM' test is out of tolerance without 6840 hack.

    Information from:

             CMI SYSTEM SERVICE MANUAL
        FAIRLIGHT INSTRUMENTS, FEBRUARY 1985
                  Revision 2.1

    This document is available on archive.org at the following URL:

    https://archive.org/details/fairlight_CMI-IIx_SERVICE_MANUAL

    Summary:

        The Fairlight CMI system conists typically of:
            - One velocity-sensitive unweighted keyboard, with a numeric
              keypad and several control surfaces
            - (Optionally) one additional keyboard, not velocity-sensitive
            - One alphanumeric keyboard for manual control
            - A 15-inch green-screen monitor and light pen for more direct
              control
            - A box consisting of:
              * An audio board including balanced line drivers for eight
                channels and mixed output
              * A 500-watt power supply
              * A 21-slot backplane
              * Two 8-inch double-density floppy disk drives. The format
                used is soft-sectored, 128 bytes per sector (single density),
                or 256 bytes per sector (double density), using FM
                recording.
              * And the following cards:
                Slot  1: Master Card CMI-02
                Slot  2: General Interface Card CMI-08/28 (Optional)
                Slots 3-11: 8 Channel Controller Cards & 1 Voice Master Module Card, order unknown
                Slot 12: 64K System RAM Q-096
                Slot 13: 256K System RAM Q-256
                Slot 14: 256K System RAM Q-256
                Slot 15: 4-Port ACIA Module Q-014 (Optional)
                Slot 16: Processor Control Module Q-133
                Slot 17: Central Processor Module Q-209
                Slot 18: Lightpen/Graphics Interface Q-219
                Slot 19: Floppy Disk Controller QFC-9
                Slot 20: Hard Disk Controller Q-077 (Optional)

        The Master Keyboard
        -------------------

        The master keyboard has the following features:
            - A serial connector for communicating with the CMI mainframe
            - A connector for a slave keyboard
            - A connector for the alphanumeric keyboard
            - Connectors for pedal controls
            - Three slider-type analog controls
            - Two switch controls (one momentary, one toggle on/off)
            - Two lamp indicators for the switches with software-defined
              control
            - A 12-character LED alphanumeric display
            - A 16-switch keypad

        All communications with all peripherals and controls on the master
        keyboard is handled via the master keyboard's controller, and as
        such there is one single serial link to the "CMI mainframe" box
        itself.

        Q209 Dual 6809 Central Processor Card
        -------------------------------------

        The CPU card has two 6809 processors, with robust inter-CPU
        communications capabilities including:
            - Uninterruptible instructions
            - CPU-specific ID register and memory map registers
            - Interprocessor interrupts
            - Automatic memory map-switching register

        The CPUs are multiplexed onto the address and data buses
        in an interleaved manner such that there is no contention
        on simultaneous memory accesses.

        All system timing is derived from a 40MHz clock crystal, which
        is divided into two opposite-phase 20MHz squre waves.

        Other data entry from service manual to be completed later - RH 12 Aug 2016

****************************************************************************/

#include "emu.h"

#include "audio/cmi01a.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/7474.h"
#include "machine/clock.h"
#include "machine/i8214.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/msm5832.h"
#include "machine/wd_fdc.h"

#include "video/dl1416.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define Q209_CPU_CLOCK      40.21_MHz_XTAL / 40 // divider not verified (very complex circuit)

#define M6809_CLOCK             8000000 // wrong
#define MASTER_OSCILLATOR       34.291712_MHz_XTAL

#define CPU_1                   0
#define CPU_2                   1

#define MAPPING_A               1
#define MAPPING_B               0

#define NUM_Q256_CARDS          1   // Max of 2
#define NUM_CHANNEL_CARDS       8

#define PAGE_SIZE               2048
#define PAGE_COUNT              (65536 / PAGE_SIZE)
#define PAGE_MASK               (PAGE_SIZE - 1)
#define PAGE_SHIFT              5

#define PIXEL_CLOCK             10.38_MHz_XTAL
#define HTOTAL                  672
#define HBLANK_END              0
#define HBLANK_START            512
#define VTOTAL                  304
#define VBLANK_END              0
#define VBLANK_START            256

#define HBLANK_FREQ     (PIXEL_CLOCK / HTOTAL)
#define VBLANK_FREQ     (HBLANK_FREQ / VTOTAL)

#define MAPSEL_P2_B             0x00
#define MAPSEL_P2_A             0x03
#define MAPSEL_P2_A_DMA1        0x04
#define MAPSEL_P2_A_DMA2        0x05
#define MAPSEL_P2_A_DMA3        0x06
#define MAPSEL_P2_A_DMA4        0x07
#define MAPSEL_P1_B             0x08
#define MAPSEL_P1_A             0x0b
#define MAPSEL_P1_A_DMA1        0x0c
#define MAPSEL_P1_A_DMA2        0x0d
#define MAPSEL_P1_A_DMA3        0x0e
#define MAPSEL_P1_A_DMA4        0x0f

#define IRQ_ACINT_LEVEL         (0 ^ 7)
#define IRQ_MIDINT_LEVEL        (0 ^ 7)
#define IRQ_TIMINT_LEVEL        (1 ^ 7)
#define IRQ_INTP1_LEVEL         (2 ^ 7)
#define IRQ_IPI1_LEVEL          (3 ^ 7)
#define IRQ_SMIDINT_LEVEL       (3 ^ 7)
#define IRQ_AIC_LEVEL           (4 ^ 7)

#define IRQ_CHINT8_LEVEL        (15 ^ 7)
#define IRQ_CHINT7_LEVEL        (14 ^ 7)
#define IRQ_CHINT6_LEVEL        (13 ^ 7)
#define IRQ_CHINT5_LEVEL        (12 ^ 7)
#define IRQ_CHINT4_LEVEL        (11 ^ 7)
#define IRQ_CHINT3_LEVEL        (10 ^ 7)
#define IRQ_CHINT2_LEVEL        (9 ^ 7)
#define IRQ_CHINT1_LEVEL        (8 ^ 7)

static const int ch_int_levels[8] =
{
	12, 8, 13, 9, 14, 10, 15, 11 //IRQ_CHINT8_LEVEL, IRQ_CHINT7_LEVEL, IRQ_CHINT6_LEVEL, IRQ_CHINT5_LEVEL, IRQ_CHINT4_LEVEL, IRQ_CHINT3_LEVEL, IRQ_CHINT2_LEVEL, IRQ_CHINT1_LEVEL
};

#define IRQ_PERRINT_LEVEL       (0 ^ 7)
#define IRQ_RTCINT_LEVEL        (0 ^ 7)
#define IRQ_RINT_LEVEL          (1 ^ 7)
#define IRQ_INTP2_LEVEL         (2 ^ 7)
#define IRQ_IPI2_LEVEL          (3 ^ 7)
#define IRQ_TOUCHINT_LEVEL      (4 ^ 7)
#define IRQ_PENINT_LEVEL        (5 ^ 7)
#define IRQ_ADINT_LEVEL         (6 ^ 7)
#define IRQ_DISKINT_LEVEL       (7 ^ 7)

#define FDC_CONTROL_INTEN       (1 << 2)

#define FDC_STATUS_READY        (1 << 3)
#define FDC_STATUS_TWO_SIDED    (1 << 4)
#define FDC_STATUS_DISK_CHANGE  (1 << 5)
#define FDC_STATUS_INTERRUPT    (1 << 6)
#define FDC_STATUS_DRIVER_LOAD  (1 << 7)


class cmi_state : public driver_device
{
public:
	cmi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu1(*this, "maincpu1")
		, m_maincpu2(*this, "maincpu2")
		, m_muskeyscpu(*this, "muskeys")
		, m_alphakeyscpu(*this, "alphakeys")
		, m_midicpu(*this, "smptemidi")
		, m_cmi07cpu(*this, "cmi07cpu")
		, m_msm5832(*this, "msm5832")
		, m_i8214(*this, "i8214_%u", 1U)
		, m_q133_pia(*this, "q133_pia_%u", 1U)
		, m_q133_ptm(*this, "q133_ptm")
		, m_q133_acia(*this, "q133_acia_%u", 0U)
		, m_q133_region(*this, "q133")
		, m_q219_pia(*this, "q219_pia")
		, m_q219_ptm(*this, "q219_ptm")
		, m_cmi02_pia(*this, "cmi02_pia_%u", 1U)
		, m_cmi02_ptm(*this, "cmi02_ptm")
		, m_ank_pia(*this, "ank_pia")
		, m_acia_mkbd_kbd(*this, "acia_mkbd_kbd")
		, m_acia_mkbd_cmi(*this, "acia_mkbd_cmi")
		, m_cmi07_ptm(*this, "cmi07_ptm")
		, m_qfc9_region(*this, "qfc9")
		, m_floppy0(*this, "wd1791:0")
		, m_floppy1(*this, "wd1791:1")
		, m_wd1791(*this, "wd1791")
		, m_channels(*this, "cmi01a_%u", 0)
		, m_cmi10_pia_u20(*this, "cmi10_pia_u20")
		, m_cmi10_pia_u21(*this, "cmi10_pia_u21")
		, m_dp1(*this, "dp1")
		, m_dp2(*this, "dp2")
		, m_dp3(*this, "dp3")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_ankrow_ports(*this, "ROW%u", 0)
		, m_lp_x_port(*this, "LP_X")
		, m_lp_y_port(*this, "LP_Y")
		, m_lp_touch_port(*this, "LP_TOUCH")
		, m_keypad_a_port(*this, "KEYPAD_A")
		, m_keypad_b_port(*this, "KEYPAD_B")
		, m_key_mux_ports{ { *this, "KEY_%u_0", 0 }, { *this, "KEY_%u_1", 0 }, { *this, "KEY_%u_2", 0 }, { *this, "KEY_%u_3", 0 } }
		, m_digit(*this, "digit%u", 0U)
		, m_cmi07_ram(*this, "cmi07_ram")
	{
	}

	void set_interrupt(int cpunum, int level, int state);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_MAP_SWITCH = 0;
	static const device_timer_id TIMER_HBLANK = 1;
	static const device_timer_id TIMER_JAM_TIMEOUT = 2;
	static const device_timer_id TIMER_CMI10_SCND = 3;

	void init_cmi2x();

	// CPU card
	DECLARE_WRITE_LINE_MEMBER( q133_acia_irq );
	DECLARE_WRITE8_MEMBER( i8214_cpu1_w );
	DECLARE_WRITE8_MEMBER( i8214_cpu2_w );
	DECLARE_WRITE_LINE_MEMBER( i8214_1_int_w );
	DECLARE_WRITE_LINE_MEMBER( i8214_2_int_w );
	DECLARE_WRITE_LINE_MEMBER( i8214_3_int_w );
	DECLARE_WRITE_LINE_MEMBER( i8214_3_enlg );
	DECLARE_READ8_MEMBER( shared_ram_r );
	DECLARE_WRITE8_MEMBER( shared_ram_w );

	DECLARE_READ8_MEMBER( q133_1_porta_r );
	DECLARE_WRITE8_MEMBER( q133_1_porta_w );
	DECLARE_WRITE8_MEMBER( q133_1_portb_w );

	DECLARE_WRITE_LINE_MEMBER( cmi_iix_vblank );
	IRQ_CALLBACK_MEMBER( cpu1_interrupt_callback );
	IRQ_CALLBACK_MEMBER( cpu2_interrupt_callback );

	// Video-related
	DECLARE_READ8_MEMBER( video_r );
	DECLARE_READ8_MEMBER( lightpen_r );
	DECLARE_READ8_MEMBER( pia_q219_b_r );
	DECLARE_WRITE8_MEMBER( video_w );
	DECLARE_WRITE8_MEMBER( vscroll_w );
	DECLARE_WRITE8_MEMBER( video_attr_w );
	DECLARE_READ8_MEMBER( vram_r );
	DECLARE_WRITE8_MEMBER( vram_w );
	DECLARE_WRITE_LINE_MEMBER( pia_q219_irqa );
	DECLARE_WRITE_LINE_MEMBER( pia_q219_irqb );
	DECLARE_WRITE_LINE_MEMBER( ptm_q219_irq );
	uint32_t screen_update_cmi2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// Memory mapping
	template<int cpunum> DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( map_ram_w );
	template<int cpunum> DECLARE_READ8_MEMBER( vector_r );
	template<int cpunum> DECLARE_READ8_MEMBER( map_r );
	template<int cpunum> DECLARE_WRITE8_MEMBER( map_w );
	DECLARE_READ8_MEMBER( atomic_r );
	DECLARE_WRITE8_MEMBER( cpufunc_w );
	DECLARE_READ8_MEMBER( parity_r );
	DECLARE_WRITE8_MEMBER( mapsel_w );
	template<int cpunum> DECLARE_READ8_MEMBER( irq_ram_r );
	template<int cpunum> DECLARE_WRITE8_MEMBER( irq_ram_w );

	// MIDI/SMPTE
	DECLARE_WRITE16_MEMBER( midi_dma_w );
	DECLARE_READ16_MEMBER( midi_dma_r );

	// Floppy
	DECLARE_WRITE8_MEMBER( fdc_w );
	DECLARE_READ8_MEMBER( fdc_r );
	DECLARE_WRITE_LINE_MEMBER( wd1791_irq );
	DECLARE_WRITE_LINE_MEMBER( wd1791_drq );

	// Master card
	DECLARE_READ8_MEMBER( cmi02_r );
	DECLARE_WRITE8_MEMBER( cmi02_w );
	DECLARE_WRITE8_MEMBER( master_tune_w );
	DECLARE_WRITE_LINE_MEMBER( cmi02_ptm_irq );
	DECLARE_WRITE_LINE_MEMBER( cmi02_ptm_o2 );

	// Alphanumeric keyboard
	DECLARE_READ8_MEMBER( ank_col_r );
	DECLARE_READ_LINE_MEMBER( ank_rts_r );

	// ???
	DECLARE_READ8_MEMBER( cmi07_r );
	DECLARE_WRITE8_MEMBER( cmi07_w );

	// Music keyboard/alphanumeric display/keypad
	DECLARE_WRITE8_MEMBER( cmi10_u20_a_w );
	DECLARE_WRITE8_MEMBER( cmi10_u20_b_w );
	DECLARE_READ_LINE_MEMBER( cmi10_u20_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( cmi10_u20_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( cmi10_u21_cb2_w );
	DECLARE_READ8_MEMBER( cmi10_u21_a_r );
	template <unsigned N> DECLARE_WRITE16_MEMBER( cmi_iix_update_dp );

	DECLARE_WRITE_LINE_MEMBER( msm5832_irq );
	DECLARE_WRITE_LINE_MEMBER( mkbd_kbd_acia_int );
	DECLARE_WRITE_LINE_MEMBER( mkbd_cmi_acia_int );
	DECLARE_WRITE_LINE_MEMBER( cmi07_irq );
	DECLARE_WRITE_LINE_MEMBER( mkbd_acia_clock );

	template<int Channel> DECLARE_WRITE_LINE_MEMBER( channel_irq );

	void cmi2x(machine_config &config);
	void alphakeys_map(address_map &map);
	void cmi07cpu_map(address_map &map);
	void maincpu1_map(address_map &map);
	void maincpu2_map(address_map &map);
	void midicpu_map(address_map &map);
	void muskeys_map(address_map &map);

protected:
	required_device<mc6809e_device> m_maincpu1;
	required_device<mc6809e_device> m_maincpu2;
	required_device<m6802_cpu_device> m_muskeyscpu;
	required_device<m6802_cpu_device> m_alphakeyscpu;
	required_device<m68000_device> m_midicpu;
	required_device<mc6809e_device> m_cmi07cpu;

	required_device<msm5832_device> m_msm5832;
	required_device_array<i8214_device, 3> m_i8214;
	required_device_array<pia6821_device, 2> m_q133_pia;
	required_device<ptm6840_device> m_q133_ptm;
	required_device_array<mos6551_device, 4> m_q133_acia;
	required_memory_region m_q133_region;

	required_device<pia6821_device> m_q219_pia;
	required_device<ptm6840_device> m_q219_ptm;

	required_device_array<pia6821_device, 2> m_cmi02_pia;
	required_device<ptm6840_device> m_cmi02_ptm;

	required_device<pia6821_device> m_ank_pia;
	required_device<acia6850_device> m_acia_mkbd_kbd;
	required_device<acia6850_device> m_acia_mkbd_cmi;

	required_device<ptm6840_device> m_cmi07_ptm;

	required_memory_region m_qfc9_region;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<fd1791_device> m_wd1791;

	required_device_array<cmi01a_device, 8> m_channels;

	required_device<pia6821_device> m_cmi10_pia_u20;
	required_device<pia6821_device> m_cmi10_pia_u21;
	required_device<dl1416_device> m_dp1;
	required_device<dl1416_device> m_dp2;
	required_device<dl1416_device> m_dp3;

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport_array<8> m_ankrow_ports;

	required_ioport m_lp_x_port;
	required_ioport m_lp_y_port;
	required_ioport m_lp_touch_port;

	required_ioport m_keypad_a_port;
	required_ioport m_keypad_b_port;

	required_ioport_array<3> m_key_mux_ports[4];

	output_finder<12> m_digit;

	required_shared_ptr<uint8_t> m_cmi07_ram;

	address_space *m_cpu1space;
	address_space *m_cpu2space;

private:

	emu_timer *m_map_switch_timer;
	emu_timer *m_hblank_timer;
	emu_timer *m_cmi10_scnd_timer;
	emu_timer *m_jam_timeout_timer;

	uint8_t m_video_data;

	// Memory
	bool map_is_active(int cpunum, int map, uint8_t *map_info);
	void update_address_space(int cpunum, uint8_t mapinfo);
	void install_video_ram(int cpunum);
	void install_peripherals(int cpunum);

	// Video
	void hblank();
	void update_video_pos(int y, int x, int byte_size);
	void video_write(int offset);

	// Floppy
	void dma_fdc_rom();
	void write_fdc_ctrl(uint8_t data);
	void fdc_dma_transfer();

	// Q133 CPU Card
	uint8_t *m_q133_rom;

	uint8_t   m_hp_int;
	std::unique_ptr<uint8_t[]>    m_shared_ram;
	std::unique_ptr<uint8_t[]>    m_scratch_ram[2];

	/* Memory management */
	uint8_t   m_map_sel[16];
	std::unique_ptr<uint8_t[]>    m_map_ram[2];
	std::unique_ptr<uint8_t[]>    m_q256_ram[2];
	uint8_t   m_map_ram_latch;
	int     m_cpu_active_space[2]; // TODO: Make one register
	int     m_cpu_map_switch[2];
	uint8_t   m_irq_address[2][2];
	int     m_m6809_bs_hack_cnt;
	int     m_m6809_bs_hack_cpu;

	/* Q219 lightpen/graphics card */
	std::unique_ptr<uint8_t[]>    m_video_ram;
	uint16_t  m_x_pos;
	uint8_t   m_y_pos;
	uint16_t  m_lp_x;
	uint8_t   m_lp_y;
	uint8_t   m_q219_b_touch;

	/* QFC9 floppy disk controller card */
	uint8_t * m_qfc9_region_ptr;
	int     m_fdc_drq;
	uint8_t   m_fdc_addr;
	uint8_t   m_fdc_ctrl;
	uint8_t   m_fdc_status;
	PAIR    m_fdc_dma_addr;
	PAIR    m_fdc_dma_cnt;

	/* CMI-07 */
	uint8_t   m_cmi07_ctrl;

	/* CMI-10 */
	uint8_t   m_scnd;

	/* Musical keyboard */
	uint8_t   m_msm5832_addr;
	int     m_mkbd_kbd_acia_irq;
	int     m_mkbd_cmi_acia_irq;

	// Master card (CMI-02)
	int     m_cmi02_ptm_irq;
};

/**************************************
 *
 *  Video hardware
 *
 *************************************/

uint32_t cmi_state::screen_update_cmi2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	uint8_t y_scroll = m_q219_pia->a_output();
	uint8_t invert = (!BIT(m_q219_pia->b_output(), 3)) & 1;

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		uint8_t *src = &m_video_ram[(512/8) * ((y + y_scroll) & 0xff)];
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
		{
			uint8_t data = *src++;

			/* Store 8 pixels */
			for (int i = 0; i < 8; ++i)
				*dest++ = pen[BIT(data, 7 - i) ^ invert];
		}
	}

	/* Get lightpen position */
	//if (LPCEN && NOT_TOUCHING)
	if (m_lp_touch_port->read() && BIT(m_q219_pia->b_output(), 1))
	{
		/* Invert target pixel */
		bitmap.pix32(m_lp_y_port->read(), m_lp_x_port->read()) ^= 0x00ffffff;
	}



	return 0;
}

void cmi_state::hblank()
{
	int v = m_screen->vpos();

	if (!m_screen->vblank())
	{
		int _touch = m_lp_touch_port->read();
		int _tfh = !BIT(m_q219_pia->b_output(), 2);

		if (v == m_lp_y_port->read())
		{
			if (_touch)
				m_q219_b_touch = 0;
			else
				m_q219_b_touch = 1 << 5;

			m_q219_pia->ca1_w(!_touch ? 1 : 0);

			if (!_touch || !_tfh)
			{
				/* Latch the counters */
				m_lp_x = m_lp_x_port->read();
				m_lp_y = m_lp_y_port->read();

				/* LPSTB */
				m_q219_pia->cb1_w(1);
			}
		}
	}
	/* Adjust for next scanline */
	if (++v >= VTOTAL)
		v = 0;

	m_hblank_timer->adjust(m_screen->time_until_pos(v, HBLANK_START));

}

void cmi_state::update_video_pos(int y, int x, int byte_size)
{
	uint8_t *video_addr = &m_video_ram[m_y_pos * (512 / 8) + (m_x_pos / 8)];

	if (byte_size)
	{
		*video_addr = m_video_data;
	}
	else
	{
		int bit_mask = 1 << ((7 ^ m_x_pos) & 7);

		*video_addr &= ~bit_mask;
		*video_addr |= m_video_data & bit_mask;
	}

	if (y > 0)
		m_y_pos = (m_y_pos + 1) & 0xff;
	else if (y < 0)
		m_y_pos = (m_y_pos - 1) & 0xff;

	if (x > 0)
		m_x_pos = (m_x_pos + 1) & 0x1ff;
	else if (x < 0)
		m_x_pos = (m_x_pos - 1) & 0x1ff;
}

void cmi_state::video_write(int offset)
{
	switch (offset)
	{
		case 0x0: update_video_pos( 0,  0, 0); break;
		case 0x1: update_video_pos( 0,  1, 0); break;
		case 0x2: update_video_pos( 0, -1, 0); break;
		case 0x3: update_video_pos( 0,  0, 1); break;
		case 0x4: update_video_pos( 1,  0, 0); break;
		case 0x5: update_video_pos( 1,  1, 0); break;
		case 0x6: update_video_pos( 1, -1, 0); break;
		case 0x7: update_video_pos( 1,  0, 1); break;
		case 0x8: update_video_pos(-1,  0, 0); break;
		case 0x9: update_video_pos(-1,  1, 0); break;
		case 0xa: update_video_pos(-1, -1, 0); break;
		case 0xb: update_video_pos(-1,  0, 1); break;
//      default: printf("Video Write %x %x\n", offset, m_video_data);
	}
}

READ8_MEMBER( cmi_state::video_r )
{
	if (machine().side_effects_disabled())
		return m_video_data;

	m_video_data = m_video_ram[m_y_pos * (512 / 8) + (m_x_pos / 8)];

	video_write(offset);
	return m_video_data;
}

READ8_MEMBER( cmi_state::lightpen_r )
{
	if (offset & 2)
		return m_lp_y;
	else
		return m_lp_x >> 1;
}

READ8_MEMBER( cmi_state::pia_q219_b_r )
{
	return ((m_lp_x << 7) & 0x80) | m_q219_b_touch;
}


WRITE8_MEMBER( cmi_state::video_w )
{
	m_video_data = data;
	video_write(offset);
}

WRITE8_MEMBER( cmi_state::vscroll_w )
{
	// TODO: Partial updates. Also, this should be done through a PIA
}

WRITE8_MEMBER( cmi_state::video_attr_w )
{
	// TODO
}

WRITE8_MEMBER( cmi_state::vram_w )
{
	m_video_ram[offset] = data;
}

READ8_MEMBER( cmi_state::vram_r )
{
	if (machine().side_effects_disabled())
		return m_video_ram[offset];

	/* Latch the current video position */
	m_y_pos = (offset >> 6) & 0xff;
	m_x_pos = (offset & 0x3f) << 3;

	return m_video_ram[offset];
}



/* Memory handling */

template<int cpunum> READ8_MEMBER( cmi_state::rom_r )
{
	uint16_t base = (cpunum ? 0x1000 : 0x2000);
	return *(((uint8_t *)m_q133_region->base()) + base + offset);
}

WRITE8_MEMBER( cmi_state::map_ram_w )
{
	if ((offset & 1) == 0)
	{
		m_map_ram_latch = data;
	}
	else
	{
		for (int i = 0; i < NUM_Q256_CARDS; ++i)
		{
			uint8_t map_info;
			int map = (offset >> 6);
			int page_enable = ((m_map_ram_latch & 0x80) && (i == (m_map_ram_latch & 7))) ? 0x80 : 0;

			m_map_ram[i][offset >> 1] = page_enable | (data & 0x7f);

			/* Determine if this map is in use by either CPU */
			if (map_is_active(CPU_1, map, &map_info))
				update_address_space(0, map_info);

			if (map_is_active(CPU_2, map, &map_info))
				update_address_space(1, map_info);
		}
	}
}

template<int cpunum> READ8_MEMBER( cmi_state::vector_r )
{
	return m_q133_rom[(cpunum ? 0xbfe : 0xffe) + offset];
}

template<int cpunum> READ8_MEMBER( cmi_state::map_r )
{
	uint8_t data = (m_cpu_active_space[1] << 2) | (m_cpu_active_space[0] << 1) | cpunum;
	return data;
}

template<int cpunum> WRITE8_MEMBER( cmi_state::map_w )
{
	m_map_switch_timer->adjust(attotime::from_ticks(data & 0xf, M6809_CLOCK), cpunum);
}

template<int cpunum> READ8_MEMBER( cmi_state::irq_ram_r )
{
	if (machine().side_effects_disabled())
		return m_scratch_ram[cpunum][0xf8 + offset];

	if (m_m6809_bs_hack_cnt > 0 && m_m6809_bs_hack_cpu == cpunum)
	{
		m_m6809_bs_hack_cnt--;
		return m_irq_address[cpunum][offset];
	}
	return m_scratch_ram[cpunum][0xf8 + offset];
}

template<int cpunum> WRITE8_MEMBER( cmi_state::irq_ram_w )
{
	m_scratch_ram[cpunum][0xf8 + offset] = data;
}

void cmi_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_MAP_SWITCH:
		{
			m_cpu_active_space[param] = m_cpu_map_switch[param];
			uint8_t map_info = (m_cpu_map_switch[param] == MAPPING_A) ?
							 m_map_sel[param ? MAPSEL_P2_A : MAPSEL_P1_A] :
							 m_map_sel[param ? MAPSEL_P2_B : MAPSEL_P1_B];
			update_address_space(param, map_info);
			m_map_switch_timer->adjust(attotime::never);
			break;
		}

		case TIMER_HBLANK:
			hblank();
			break;

		case TIMER_JAM_TIMEOUT:
			m_maincpu2->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_jam_timeout_timer->adjust(attotime::never);
			break;

		case TIMER_CMI10_SCND:
			m_cmi10_pia_u20->ca1_w(m_scnd);
			m_scnd ^= 1;
			m_cmi10_pia_u21->ca1_w(m_scnd);
			break;
	}
}

READ8_MEMBER( cmi_state::atomic_r )
{
	// TODO
	//printf("atomic access\n");
	return 0;
}

WRITE8_MEMBER( cmi_state::cpufunc_w )
{
	int cpunum = data & 1;
	int idx = data & 6;
	int bit = (data & 8) >> 3;

	switch (idx)
	{
		case 0: set_interrupt(cpunum, IRQ_IPI2_LEVEL, bit ? ASSERT_LINE : CLEAR_LINE);
				break;
		case 2: // TODO: Hardware trace
				printf("TODO: Hardware trace %02x\n", data);
				break;
		case 4: m_cpu_map_switch[cpunum] = bit;
				break;
		case 6: if (cpunum == CPU_1)
					m_maincpu1->set_input_line(M6809_FIRQ_LINE, bit ? ASSERT_LINE : CLEAR_LINE);
				else
					m_maincpu2->set_input_line(M6809_FIRQ_LINE, bit ? ASSERT_LINE : CLEAR_LINE);
				break;
	}
}

READ8_MEMBER( cmi_state::parity_r )
{
	//printf("parity_r %04x\n", offset);
	// TODO
	return 0xff;
}

WRITE8_MEMBER( cmi_state::mapsel_w )
{
	data ^= 0x1f;
	m_map_sel[offset] = data;

	if ((offset == MAPSEL_P1_A) && (m_cpu_active_space[0] == MAPPING_A))
		update_address_space(0, data);
	else if ((offset == MAPSEL_P1_B) && (m_cpu_active_space[0] == MAPPING_B))
		update_address_space(0, data);

	if ((offset == MAPSEL_P2_A) && (m_cpu_active_space[1] == MAPPING_A))
		update_address_space(1, data);
	else if ((offset == MAPSEL_P2_B) && (m_cpu_active_space[1] == MAPPING_B))
		update_address_space(1, data);
}



WRITE16_MEMBER( cmi_state::midi_dma_w )
{
	address_space *cmi_space = ((offset & 0x8000) ? m_cpu2space : m_cpu1space);
	offset &= 0x7fff;

	if (ACCESSING_BITS_0_7)
		cmi_space->write_byte(offset * 2 + 1, data);
	if (ACCESSING_BITS_8_15)
		cmi_space->write_byte(offset * 2, data >> 8);
}

READ16_MEMBER( cmi_state::midi_dma_r )
{
	address_space *cmi_space = ((offset & 0x8000) ? m_cpu2space : m_cpu1space);
	offset &= 0x7fff;
	return cmi_space->read_word(offset * 2);
}

/* The maps are dynamically populated */
void cmi_state::maincpu1_map(address_map &map)
{
	map(0xfffe, 0xffff).r(FUNC(cmi_state::vector_r<0>));
}

void cmi_state::maincpu2_map(address_map &map)
{
	map(0xfffe, 0xffff).r(FUNC(cmi_state::vector_r<1>));
}

void cmi_state::muskeys_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();
	map(0x0080, 0x0083).rw(m_cmi10_pia_u21, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).rw(m_cmi10_pia_u20, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x00a0, 0x00a1).rw(m_acia_mkbd_kbd, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x00b0, 0x00b1).rw(m_acia_mkbd_cmi, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x4000, 0x47ff).ram();
	map(0xb000, 0xb400).rom();
	map(0xf000, 0xffff).rom();
}

void cmi_state::alphakeys_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();
	map(0x4000, 0x7fff).portr("ANK_OPTIONS");
	map(0x8000, 0xbfff).rw(m_ank_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xc3ff).rom().mirror(0x3c00);
}

void cmi_state::midicpu_map(address_map &map)
{
	map(0x000000, 0x003fff).rom();
	map(0x040000, 0x05ffff).rw(FUNC(cmi_state::midi_dma_r), FUNC(cmi_state::midi_dma_w));
//  AM_RANGE(0x060000, 0x06001f) TIMERS
//  AM_RANGE(0x060050, 0x06005f) ACIA
//  AM_RANGE(0x060070, 0x06007f) SMPTE
	map(0x080000, 0x083fff).ram();
}

void cmi_state::cmi07cpu_map(address_map &map)
{
	map(0x0000, 0x3fff).noprw(); // TODO
	map(0x4000, 0x4fff).noprw(); // TODO
	map(0x8000, 0x8fff).rw(m_cmi07_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xc000, 0xffff).ram().share("cmi07_ram");
}

/* Input ports */
static INPUT_PORTS_START( cmi2x )
	PORT_START("LP_X")
	PORT_BIT( 0xffff, HBLANK_START/2, IPT_LIGHTGUN_X) PORT_NAME ("Lightpen X") PORT_MINMAX(0, HBLANK_START - 1) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START("LP_Y")
	PORT_BIT( 0xffff, VBLANK_START/2, IPT_LIGHTGUN_Y) PORT_NAME ("Lightpen Y") PORT_MINMAX(0, VBLANK_START - 1) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START("LP_TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME ( "Lightpen Touch" ) PORT_CODE( MOUSECODE_BUTTON1 )

	/* Alphanumeric keyboard */
	PORT_START("ANK_OPTIONS")
	PORT_DIPNAME( 0x07, 0x00, "Speed (baud)" )
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600"  )
	PORT_DIPSETTING(    0x05, "300"  )
	PORT_DIPSETTING(    0x06, "150"  )
	PORT_DIPSETTING(    0x07, "110"  )

	PORT_DIPNAME( 0x30, 0x20, "Parity" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPSETTING(    0x10, "None, bit 7 is 0" )
	PORT_DIPSETTING(    0x20, "Odd" )
	PORT_DIPSETTING(    0x30, "None, bit 7 is 1" )

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                PORT_CHAR('8')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)            PORT_CHAR('-')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)         PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)            PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                PORT_CHAR('9')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)            PORT_NAME("Right")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)               PORT_NAME("Up")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)        PORT_CHAR('-')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                PORT_CHAR('0')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)              PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Set")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Add")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)            PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)            PORT_CHAR(',')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)           PORT_NAME("LShift")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)             PORT_NAME("Down")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Clear")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("WTF")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                PORT_CHAR('L')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)           PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Home")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)        PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                PORT_CHAR('K')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)            PORT_NAME("Return (a)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)            PORT_NAME("Return (b)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                PORT_CHAR('I')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                     PORT_NAME("Sub")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)            PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)           PORT_NAME("RShift")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)             PORT_CHAR('.')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)             PORT_NAME("Left")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                PORT_CHAR('O')

	/* Keypad */
	PORT_START("KEYPAD_A")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEYPAD_B")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)

	/* Master musical keyboard */
	PORT_START("KEY_0_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0 #")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G0 #")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A1 #")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C1")

	PORT_START("KEY_0_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C1 #")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D1 #")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1 #")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G1 #")

	PORT_START("KEY_0_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A2 #")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C2 #")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D2 #")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E2")

	PORT_START("KEY_0_3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY_1_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2 #")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G2 #")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A3 #")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3")

	PORT_START("KEY_1_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3 #")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D3 #")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G3 #")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A4")

	PORT_START("KEY_1_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A4 #")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B4 #")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C4 #")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4 #")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E4")

	PORT_START("KEY_1_3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY_2_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4 #")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G4 #")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A5 #")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C5")

	PORT_START("KEY_2_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C5 #")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D5 #")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5 #")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G5 #")

	PORT_START("KEY_2_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A6 #")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C6 #")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D6 #")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E6")

	PORT_START("KEY_2_3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6")
INPUT_PORTS_END

bool cmi_state::map_is_active(int cpunum, int map, uint8_t *map_info)
{
	if (m_cpu_active_space[cpunum] == MAPPING_A)
	{
		*map_info = m_map_sel[cpunum ? MAPSEL_P2_A : MAPSEL_P1_A];

		if ((*map_info & 0x1f) == map)
			return 1;
	}
	else
	{
		*map_info = m_map_sel[cpunum ? MAPSEL_P2_B : MAPSEL_P1_B];

		if ((*map_info & 0x1f) == map)
			return 1;
	}

	return 0;
}

void cmi_state::update_address_space(int cpunum, uint8_t mapinfo)
{
	int map = mapinfo & 0x1f;
	bool vram_en = !BIT(mapinfo, 5);
	bool periph_en = !BIT(mapinfo, 7);
	int i;

	address_space *space = (cpunum == 0 ? m_cpu1space : m_cpu2space);

	space->unmap_readwrite(0x0000, 0xffff);

	/* Step through the map RAM assignments */
	for (int page = 0; page < PAGE_COUNT; ++page)
	{
		int address = page * PAGE_SIZE;
		uint8_t page_info = 0;

		/* Scan through the cards */
		for (i = 0; i < NUM_Q256_CARDS; ++i)
		{
			page_info = m_map_ram[i][(map << PAGE_SHIFT) + page];

			/* Page is enabled in this bank */
			if (page_info & 0x80)
				break;
		}

		if (BIT(m_cmi07_ctrl, 6))
		{
			if ((cpunum == 0) || !BIT(m_cmi07_ctrl, 7))
			{
				if (m_cmi07_ctrl & 0x30)
				if ((address & 0xc000) == ((m_cmi07_ctrl & 0x30) << 10))
				{
					space->install_ram(address, address + PAGE_SIZE - 1, &m_cmi07_ram[(page * PAGE_SIZE) & 0x3fff]);
					continue;
				}
			}
		}

		/* No banks had this page enabled - skip */
		if ((page_info & 0x80) == 0)
			continue;

		/* If Video RAM is enabled, don't install RAM here */
		if (vram_en && address >= 0x8000 && address <= 0xbfff)
			continue;

		/* If peripherals are enabled, don't install RAM here */
		if (periph_en && address >= 0xf000 && address <= 0xffff) // TODO
			continue;

		/* Now map the RAM page */
		space->install_ram(address, address + PAGE_SIZE - 1, &m_q256_ram[i][(page_info & 0x7f) * PAGE_SIZE]);
	}

	if (vram_en)
		install_video_ram(cpunum);

	if (periph_en)
		install_peripherals(cpunum);
}

WRITE8_MEMBER( cmi_state::cmi07_w )
{
	m_cmi07_ctrl = data;

	m_cmi07cpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
	m_cmi07cpu->set_input_line(M6809_FIRQ_LINE,  BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
	m_cmi07cpu->set_input_line(INPUT_LINE_NMI,   BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE);
	m_cmi07cpu->set_input_line(INPUT_LINE_HALT,  BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	/* We need to update the address spaces */
	uint8_t map_info = (m_cpu_active_space[0] == MAPPING_A) ? m_map_sel[MAPSEL_P1_A] : m_map_sel[MAPSEL_P1_B];
	update_address_space(0, map_info);

	map_info = (m_cpu_active_space[1] == MAPPING_A) ? m_map_sel[MAPSEL_P2_A] : m_map_sel[MAPSEL_P2_B];
	update_address_space(1, map_info);
}

READ8_MEMBER( cmi_state::cmi07_r )
{
	//printf("CMI 07 R: %x\n", offset);
	return 0xff;
}

WRITE_LINE_MEMBER( cmi_state::q133_acia_irq )
{
	set_interrupt(CPU_1, IRQ_ACINT_LEVEL, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER( cmi_state::ank_col_r )
{
	int row = m_ank_pia->b_output() ^ 0xff;

	switch (row)
	{
		case 0x01: return m_ankrow_ports[0]->read();
		case 0x02: return m_ankrow_ports[1]->read();
		case 0x04: return m_ankrow_ports[2]->read();
		case 0x08: return m_ankrow_ports[3]->read();
		case 0x10: return m_ankrow_ports[4]->read();
		case 0x20: return m_ankrow_ports[5]->read();
		case 0x40: return m_ankrow_ports[6]->read();
		case 0x80: return m_ankrow_ports[7]->read();
		default:   return 0xff;
	}
}


/**************************************
 *
 *  Floppy disk interface
 *
 *************************************/

void cmi_state::dma_fdc_rom()
{
	/* DMA channel 1 is used*/
	uint8_t map_info = m_map_sel[MAPSEL_P2_A_DMA1];
	int map = map_info & 0x1f;
	int addr = m_fdc_dma_addr.w.l & ~PAGE_MASK;
	int page = addr / PAGE_SIZE;
	uint8_t p_info = 0;

	/* Active low */
	m_fdc_status &= ~FDC_STATUS_DRIVER_LOAD;

	int i;
	for (i = 0; i < NUM_Q256_CARDS; ++i)
	{
		p_info = m_map_ram[i][(map << PAGE_SHIFT) | page];

		if (p_info & 0x80)
			break;
	}

	if ((p_info & 0x80) == 0)
	{
		printf("Trying to DMA FDC driver to a non-enabled page!\n");
		return;
	}

	/* TODO: This should be stuck in a deferred write */
	int cnt = std::min(m_fdc_dma_cnt.w.l ^ 0xffff, 2048);
	memcpy(&m_q256_ram[i][(p_info & 0x7f) * PAGE_SIZE], m_qfc9_region_ptr, cnt);
	m_fdc_status |= FDC_STATUS_DRIVER_LOAD;

	/* TODO: Is this correct? */
	m_fdc_dma_addr.w.l += 0x800;
	m_fdc_dma_cnt.w.l = 0;
}

void cmi_state::write_fdc_ctrl(uint8_t data)
{
	int drive = data & 1;
	int side = BIT(data, 5) ? 1 : 0;

	switch (drive)
	{
	case 0: m_wd1791->set_floppy(m_floppy0->get_device()); break;
	case 1: m_wd1791->set_floppy(m_floppy1->get_device()); break;
	}

	if (m_floppy0->get_device()) m_floppy0->get_device()->ss_w(side);
	if (m_floppy1->get_device()) m_floppy1->get_device()->ss_w(side);

	m_wd1791->dden_w(BIT(data, 7) ? true : false);

	m_fdc_ctrl = data;
}

WRITE8_MEMBER( cmi_state::fdc_w )
{
	if (offset == 0)
	{
		switch (m_fdc_addr)
		{
			case 0x0: write_fdc_ctrl(data);         break;
			case 0x2: m_fdc_dma_addr.b.l = data;    break;
			case 0x4: m_fdc_dma_addr.b.h  = data;   break;
			case 0x6: m_fdc_dma_cnt.b.l = data;     break;
			case 0x8: m_fdc_dma_cnt.b.h = data;     break;
			case 0xa: dma_fdc_rom();                break;
			case 0xc: m_wd1791->cmd_w(data ^ 0xff);        break;
			case 0xd: m_wd1791->track_w(data ^ 0xff);      break;
			case 0xe: m_wd1791->sector_w(data ^ 0xff);     break;
			case 0xf: m_wd1791->data_w(data ^ 0xff);       break;
			default: printf("fdc_w: Invalid access (%x with %x)", m_fdc_addr, data);
		}
	}
	else
		m_fdc_addr = data;
}

READ8_MEMBER( cmi_state::fdc_r )
{
	if (machine().side_effects_disabled())
		return 0;

	if (offset == 0)
	{
		switch (m_fdc_addr)
		{
			case 0xc: return m_wd1791->status_r() ^ 0xff;
			case 0xd: return m_wd1791->track_r() ^ 0xff;
			case 0xe: return m_wd1791->sector_r() ^ 0xff;
			case 0xf: return m_wd1791->data_r() ^ 0xff;
			default:  return 0;
		}
	}
	else
		return m_fdc_status;
}

void cmi_state::fdc_dma_transfer()
{
	/* DMA channel 1 is used*/
	uint8_t map_info = m_map_sel[MAPSEL_P2_A_DMA1];
	int map = map_info & 0x1f;

	int cpu_page = (m_fdc_dma_addr.w.l & ~PAGE_MASK) / PAGE_SIZE;
	int phys_page = 0;

	int i;
	for (i = 0; i < NUM_Q256_CARDS; ++i)
	{
		phys_page = m_map_ram[i][(map << PAGE_SHIFT) | cpu_page];

		if (phys_page & 0x80)
			break;
	}

	//phys_page &= 0x7f;

	/* Transfer from disk to RAM */
	if (!BIT(m_fdc_ctrl, 4))
	{
		/* Read a byte at a time */
		uint8_t data = m_wd1791->data_r() ^ 0xff;

		if (m_fdc_dma_cnt.w.l == 0xffff)
			return;

		if (m_cmi07_ctrl & 0x30)
			if (BIT(m_cmi07_ctrl, 6) && !BIT(m_cmi07_ctrl, 7))
			{
				if ((m_fdc_dma_addr.w.l & 0xc000) == ((m_cmi07_ctrl & 0x30) << 10))
					m_cmi07_ram[m_fdc_dma_addr.w.l & 0x3fff] = data;
			}

		if (phys_page & 0x80)
			m_q256_ram[i][((phys_page & 0x7f) * PAGE_SIZE) + (m_fdc_dma_addr.w.l & PAGE_MASK)] = data;

		if (!BIT(m_fdc_ctrl, 3))
			m_fdc_dma_addr.w.l++;
	}

	// Transfer from RAM to disk
	else
	{
		if (m_fdc_dma_cnt.w.l == 0xffff)
			return;

		/* Write a byte at a time */
		uint8_t data = 0;

		/* TODO: This should be stuck in a deferred write */
		if (phys_page & 0x80)
			data = m_q256_ram[i][((phys_page & 0x7f) * PAGE_SIZE) + (m_fdc_dma_addr.w.l & PAGE_MASK)];

		m_wd1791->data_w(data ^ 0xff);

		if (!BIT(m_fdc_ctrl, 3))
			m_fdc_dma_addr.w.l++;
	}

	m_fdc_dma_cnt.w.l++;
}

WRITE_LINE_MEMBER( cmi_state::wd1791_irq )
{
	if (state)
	{
		m_fdc_status |= FDC_STATUS_INTERRUPT;

		if (m_fdc_ctrl & FDC_CONTROL_INTEN)
			set_interrupt(CPU_2, IRQ_DISKINT_LEVEL, ASSERT_LINE);

	}
	else
	{
		m_fdc_status &= ~FDC_STATUS_INTERRUPT;
		set_interrupt(CPU_2, IRQ_DISKINT_LEVEL, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER( cmi_state::wd1791_drq )
{
	m_fdc_drq = state;
	if (state)
		fdc_dma_transfer();
}



/**************************************
 *
 *  Master card and channel cards
 *
 *************************************/

/*
0 - 1f
20 = PIA
21 = PIA
22 = PIA
23 = PIA

24 = ADC
25 = ADC
26 = HALT CPU 2
27 = UNHALT CPU 2
28 - 2B = PIA
*/

WRITE8_MEMBER( cmi_state::master_tune_w )
{
//  double mfreq = (double)data * ((double)MASTER_OSCILLATOR / 2.0) / 256.0;
}

WRITE_LINE_MEMBER( cmi_state::cmi02_ptm_irq )
{
	//printf("cmi02_ptm_irq: %d\n", state);
	m_cmi02_ptm_irq = state;
	set_interrupt(CPU_1, IRQ_TIMINT_LEVEL, m_cmi02_ptm_irq ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( cmi_state::cmi02_ptm_o2 )
{
	m_cmi02_ptm->set_c1(state);
	m_cmi02_ptm->set_c3(state);
}

READ8_MEMBER( cmi_state::cmi02_r )
{
	if (machine().side_effects_disabled())
		return 0;

	if (offset <= 0x1f)
	{
		int ch_mask = m_cmi02_pia[0]->a_output();

		for (int i = 0; i < 8; ++i)
		{
			if (ch_mask & (1 << i))
			{
				return m_channels[i]->read(offset & 0x1f);
			}
		}

		return 0xff;
	}
	else
	{
		switch (offset)
		{
			case 0x20: case 0x21: case 0x22: case 0x23:
				return m_cmi02_pia[0]->read(offset & 3);

			case 0x26:
				m_maincpu2->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				/* LS123 one-shot with 10n and 150k */
				m_jam_timeout_timer->adjust(attotime::from_usec(675));
				return 0xff;

			case 0x27:
				m_maincpu2->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				return 0xff;

			case 0x28: case 0x29: case 0x2a: case 0x2b:
				return m_cmi02_pia[1]->read(offset & 3);

			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				return m_cmi02_ptm->read(offset & 7);

			default:
				logerror("CMI02 R: %x\n", offset);
				return 0;
		}
	}
}

WRITE8_MEMBER( cmi_state::cmi02_w )
{
	if (offset <= 0x1f)
	{
		int ch_mask = m_cmi02_pia[0]->a_output();

		for (int i = 0; i < 8; ++i)
		{
			if (ch_mask & (1 << i))
				m_channels[i]->write(offset & 0x1f, data);
		}
	}
	else
	{
		switch (offset)
		{
			case 0x20: case 0x21: case 0x22: case 0x23:
				m_cmi02_pia[0]->write(offset & 3, data);
				break;

			case 0x28: case 0x29: case 0x2a: case 0x2b:
				m_cmi02_pia[1]->write(offset & 3, data);
				break;

			case 0x30:
				m_maincpu1->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
				data ^= 0xff;
				m_i8214[2]->sgs_w((data >> 3) & 1);
				m_i8214[2]->b_w(data & 0x7);
				break;

			case 0x31: case 0x32:
				set_interrupt(0, IRQ_INTP1_LEVEL, (offset & 2) ? CLEAR_LINE : ASSERT_LINE);
				break;

			case 0x33: case 0x34:
				set_interrupt(1, IRQ_INTP2_LEVEL, (offset & 4) ? CLEAR_LINE : ASSERT_LINE);
				break;

			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				m_cmi02_ptm->write(offset & 7, data);
				break;

			default:
				logerror("CMI02 W: %x %x\n", offset, data);
		}
	}
}

template<int Channel>
WRITE_LINE_MEMBER(cmi_state::channel_irq)
{
	set_interrupt(CPU_1, ch_int_levels[Channel], state);
}

void cmi_state::install_video_ram(int cpunum)
{
	address_space *space = (cpunum == CPU_1 ? m_cpu1space : m_cpu2space);

	space->install_readwrite_handler(0x8000, 0xbfff, read8_delegate(FUNC(cmi_state::vram_r),this), write8_delegate(FUNC(cmi_state::vram_w),this));
}

WRITE8_MEMBER( cmi_state::i8214_cpu1_w )
{
	//printf("i8214_cpu1_w: %02x\n", data);
	m_maincpu1->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	data ^= 0xff;
	m_i8214[0]->sgs_w((data >> 3) & 1);
	m_i8214[0]->b_w(data & 0x7);
}


WRITE8_MEMBER( cmi_state::i8214_cpu2_w )
{
	//printf("i8214_cpu2_w: %02x\n", data);
	m_maincpu2->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	data ^= 0xff;
	m_i8214[1]->sgs_w((data >> 3) & 1);
	m_i8214[1]->b_w(data & 0x7);
}

// TODO: replace with AM_SHARE
READ8_MEMBER( cmi_state::shared_ram_r )
{
	return m_shared_ram[offset];
}

WRITE8_MEMBER( cmi_state::shared_ram_w )
{
	m_shared_ram[offset] = data;
}

READ_LINE_MEMBER( cmi_state::ank_rts_r )
{
//  printf("ANK RTS?\n");
	return 0;
}

void cmi_state::install_peripherals(int cpunum)
{
	address_space *space = (cpunum == CPU_1 ? m_cpu1space : m_cpu2space);

	space->install_readwrite_handler(0xe000, 0xe03f, read8_delegate(FUNC(cmi_state::cmi02_r),this), write8_delegate(FUNC(cmi_state::cmi02_w),this));

	if (cpunum)
		space->install_readwrite_handler(0xf000, 0xf7ff, read8_delegate(FUNC(cmi_state::rom_r<1>),this), write8_delegate(FUNC(cmi_state::map_ram_w),this));
	else
		space->install_readwrite_handler(0xf000, 0xf7ff, read8_delegate(FUNC(cmi_state::rom_r<0>),this), write8_delegate(FUNC(cmi_state::map_ram_w),this));

	space->install_rom(0xf800, 0xfbff, m_q133_rom + (cpunum == CPU_2 ? 0x1800 : 0x2800));

	space->install_readwrite_handler(0xfc40, 0xfc4f, read8_delegate(FUNC(cmi_state::parity_r),this), write8_delegate(FUNC(cmi_state::mapsel_w),this));
	space->nop_readwrite(0xfc5a, 0xfc5b); // Q077 HDD controller - not installed
	space->install_readwrite_handler(0xfc5e, 0xfc5e, read8_delegate(FUNC(cmi_state::atomic_r),this), write8_delegate(FUNC(cmi_state::cpufunc_w),this));
	if (cpunum)
		space->install_readwrite_handler(0xfc5f, 0xfc5f, read8_delegate(FUNC(cmi_state::map_r<1>),this), write8_delegate(FUNC(cmi_state::map_w<1>),this));
	else
		space->install_readwrite_handler(0xfc5f, 0xfc5f, read8_delegate(FUNC(cmi_state::map_r<0>),this), write8_delegate(FUNC(cmi_state::map_w<0>),this));

	space->install_readwrite_handler(0xfc80, 0xfc83, read8sm_delegate(FUNC(mos6551_device::read),m_q133_acia[0].target()), write8sm_delegate(FUNC(mos6551_device::write),m_q133_acia[0].target()));
	space->install_readwrite_handler(0xfc84, 0xfc87, read8sm_delegate(FUNC(mos6551_device::read),m_q133_acia[1].target()), write8sm_delegate(FUNC(mos6551_device::write),m_q133_acia[1].target()));
	space->install_readwrite_handler(0xfc88, 0xfc8b, read8sm_delegate(FUNC(mos6551_device::read),m_q133_acia[2].target()), write8sm_delegate(FUNC(mos6551_device::write),m_q133_acia[2].target()));
	space->install_readwrite_handler(0xfc8c, 0xfc8f, read8sm_delegate(FUNC(mos6551_device::read),m_q133_acia[3].target()), write8sm_delegate(FUNC(mos6551_device::write),m_q133_acia[3].target()));
	space->install_readwrite_handler(0xfc90, 0xfc97, read8sm_delegate(FUNC(ptm6840_device::read),m_q133_ptm.target()), write8sm_delegate(FUNC(ptm6840_device::write),m_q133_ptm.target()));

	space->install_readwrite_handler(0xfcbc, 0xfcbc, read8_delegate(FUNC(cmi_state::cmi07_r),this), write8_delegate(FUNC(cmi_state::cmi07_w),this));

	space->install_read_handler(0xfcc0, 0xfcc3, read8_delegate(FUNC(cmi_state::lightpen_r),this));
	space->install_readwrite_handler(0xfcc4, 0xfcc7, read8sm_delegate(FUNC(pia6821_device::read),m_q219_pia.target()), write8sm_delegate(FUNC(pia6821_device::write),m_q219_pia.target()));
	space->install_readwrite_handler(0xfcc8, 0xfccf, read8sm_delegate(FUNC(ptm6840_device::read),m_q219_ptm.target()), write8sm_delegate(FUNC(ptm6840_device::write),m_q219_ptm.target()));
	space->install_readwrite_handler(0xfcd0, 0xfcdc, read8_delegate(FUNC(cmi_state::video_r),this), write8_delegate(FUNC(cmi_state::video_w),this));
	space->install_readwrite_handler(0xfce0, 0xfce1, read8_delegate(FUNC(cmi_state::fdc_r),this), write8_delegate(FUNC(cmi_state::fdc_w),this));
	space->nop_readwrite(0xfce2, 0xfcef); // Monitor ROM will attempt to detect floppy disk controller cards in this entire range
	space->install_readwrite_handler(0xfcf0, 0xfcf7, read8sm_delegate(FUNC(pia6821_device::read),m_q133_pia[0].target()), write8sm_delegate(FUNC(pia6821_device::write),m_q133_pia[0].target()));
	space->install_readwrite_handler(0xfcf8, 0xfcff, read8sm_delegate(FUNC(pia6821_device::read),m_q133_pia[1].target()), write8sm_delegate(FUNC(pia6821_device::write),m_q133_pia[1].target()));

	space->install_write_handler(0xfcfc, 0xfcfc, write8_delegate(FUNC(cmi_state::i8214_cpu1_w),this));
	space->install_write_handler(0xfcfd, 0xfcfd, write8_delegate(FUNC(cmi_state::i8214_cpu2_w),this));

	space->install_readwrite_handler(0xfd00, 0xfeff, read8_delegate(FUNC(cmi_state::shared_ram_r),this), write8_delegate(FUNC(cmi_state::shared_ram_w),this));

	space->install_ram(0xff00, 0xfff7, &m_scratch_ram[cpunum][0]);
	space->install_ram(0xfffa, 0xfffd, &m_scratch_ram[cpunum][0xfa]);

	if (cpunum) {
		space->install_readwrite_handler(0xfff8, 0xfff9, read8_delegate(FUNC(cmi_state::irq_ram_r<1>),this), write8_delegate(FUNC(cmi_state::irq_ram_w<1>),this));
		space->install_read_handler(0xfffe, 0xffff, read8_delegate(FUNC(cmi_state::vector_r<1>),this));
	} else {
		space->install_readwrite_handler(0xfff8, 0xfff9, read8_delegate(FUNC(cmi_state::irq_ram_r<0>),this), write8_delegate(FUNC(cmi_state::irq_ram_w<0>),this));
		space->install_read_handler(0xfffe, 0xffff, read8_delegate(FUNC(cmi_state::vector_r<0>),this));
	}
}


/*************************************
 *
 *  Interrupt Handling
 *
 *************************************/

WRITE_LINE_MEMBER( cmi_state::ptm_q219_irq )
{
	set_interrupt(CPU_2, IRQ_RINT_LEVEL, state);
}

IRQ_CALLBACK_MEMBER( cmi_state::cpu1_interrupt_callback )
{
	/* Switch to mapping A */
	m_cpu_active_space[CPU_1] = MAPPING_A;
	update_address_space(CPU_1, m_map_sel[MAPSEL_P1_A]);

	if (irqline == INPUT_LINE_IRQ0)
	{
		int vector = (m_hp_int ? 0xffe0 : 0xffd0);
		int level = (m_hp_int ? m_i8214[2]->a_r() : m_i8214[0]->a_r()) ^ 7;
		m_irq_address[CPU_1][0] = m_cpu1space->read_byte(vector + level*2);
		m_irq_address[CPU_1][1] = m_cpu1space->read_byte(vector + level*2 + 1);

		m_m6809_bs_hack_cnt = 2;
		m_m6809_bs_hack_cpu = CPU_1;

		//printf("cpu1 interrupt, will be pushing address %02x%02x\n", m_irq_address[CPU_1][0], m_irq_address[CPU_1][1]);
	}

	return 0;
}

IRQ_CALLBACK_MEMBER( cmi_state::cpu2_interrupt_callback )
{
	/* Switch to mapping A */
	m_cpu_active_space[CPU_2] = MAPPING_A;
	update_address_space(CPU_2, m_map_sel[MAPSEL_P2_A]);

	if (irqline == INPUT_LINE_IRQ0)
	{
		int level = m_i8214[1]->a_r() ^ 0x7;
		m_irq_address[CPU_2][0] = m_cpu2space->read_byte(0xffe0 + level*2);
		m_irq_address[CPU_2][1] = m_cpu2space->read_byte(0xffe0 + level*2 + 1);

		m_m6809_bs_hack_cnt = 2;
		m_m6809_bs_hack_cpu = CPU_2;

		//printf("cpu1 interrupt, will be pushing address %02x%02x\n", m_irq_address[CPU_2][0], m_irq_address[CPU_2][1]);
	}
	return 0;
}

void cmi_state::set_interrupt(int cpunum, int level, int state)
{
	//printf("CPU%d Int: %x State: %x\n", cpunum + 1, level, state);

	i8214_device *i8214 = ((cpunum == CPU_2) ? m_i8214[1] : (level < 8 ? m_i8214[2] : m_i8214[0]));
	i8214->r_w(level & 7, state ? 0 : 1);
}

WRITE_LINE_MEMBER( cmi_state::i8214_1_int_w )
{
	//printf("i8214_1_int_w: %d\n", state);
	if (state)
		m_maincpu1->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

WRITE_LINE_MEMBER( cmi_state::i8214_2_int_w )
{
	//printf("i8214_2_int_w: %d\n", state);
	if (state)
		m_maincpu2->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

WRITE_LINE_MEMBER( cmi_state::i8214_3_int_w )
{
	//printf("i8214_3_int_w: %d\n", state);
	m_hp_int = state;
	if (state)
		m_maincpu1->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}


WRITE_LINE_MEMBER( cmi_state::i8214_3_enlg )
{
	// Not needed?
//  m_hp_int = state;
}

WRITE_LINE_MEMBER( cmi_state::pia_q219_irqa )
{
	set_interrupt(CPU_2, IRQ_TOUCHINT_LEVEL, state);
}

WRITE_LINE_MEMBER( cmi_state::pia_q219_irqb )
{
	set_interrupt(CPU_2, IRQ_PENINT_LEVEL, state);
}


/* E9 - E11 - MSM5832RS */
/*
    A0-A3  = MSM5832 A0-A3
    A4+CA1 = MSM5832 D0
    A5+CB1 = MSM5832 D1
    A6     = MSM5832 D2
    A7     = MSM5832 D3
    B0     = HOLD
    B1     = READ
    B2     = WRITE
    CB2    = OPTO
    B3     = ----
    B4-B7  = PC0-3

    IRQA/B = /RTINT?
*/

READ8_MEMBER( cmi_state::q133_1_porta_r )
{
	if (BIT(m_q133_pia[0]->b_output(), 1))
	{
		return m_msm5832->data_r(space, m_msm5832_addr) << 4;
	}
	return 0xff;
}

WRITE8_MEMBER( cmi_state::q133_1_porta_w )
{
	m_msm5832_addr = data & 0xf;
	m_msm5832->address_w(data & 0x0f);
}

WRITE8_MEMBER( cmi_state::q133_1_portb_w )
{
	m_msm5832->hold_w(BIT(data, 0));
	m_msm5832->read_w(BIT(data, 1));
	m_msm5832->write_w(BIT(data, 2));
}

/*
    PA0-7 = BKA0-7 (display)

    PB0 = DA1
    PB1 = DA0
    PB2 = CS2
    PB3 = CU2
    PB4 = CS1
    PB5 = CU1
    PB6 = CS0
    PB7 = CU0

    CB1 = /KPAD
    CB2 = /DWS
*/

WRITE8_MEMBER( cmi_state::cmi10_u20_a_w )
{
	// low 7 bits connected to alphanumeric display data lines
	m_dp1->data_w(data & 0x7f);
	m_dp2->data_w(data & 0x7f);
	m_dp3->data_w(data & 0x7f);

	/*
	int bk = data;
	int bit = 0;

	if (BIT(bk, 3))
	    bit = BIT(input_port_read(device->machine, "KEYPAD_A"), bk & 7);
	else if (!BIT(bk, 4))
	    bit = BIT(input_port_read(device->machine, "KEYPAD_B"), bk & 7);

	pia6821_cb1_w(m_cmi10_pia_u20, 0, !bit);
	*/
}

WRITE8_MEMBER( cmi_state::cmi10_u20_b_w )
{
	// connected to alphanumeric display control lines
	uint8_t const addr = bitswap<2>(data, 0, 1);

	m_dp1->ce_w(BIT(data, 6));
	m_dp1->cu_w(BIT(data, 7));
	m_dp1->addr_w(addr);

	m_dp2->ce_w(BIT(data, 4));
	m_dp2->cu_w(BIT(data, 5));
	m_dp2->addr_w(addr);

	m_dp3->ce_w(BIT(data, 2));
	m_dp3->cu_w(BIT(data, 3));
	m_dp3->addr_w(addr);
}

READ_LINE_MEMBER( cmi_state::cmi10_u20_cb1_r )
{
	int bk = m_cmi10_pia_u20->a_output();
	int bit = 0;

	if (BIT(bk, 3))
		bit = BIT(m_keypad_a_port->read(), bk & 7);
	else if (!BIT(bk, 4))
		bit = BIT(m_keypad_b_port->read(), bk & 7);

	return !bit;
}

WRITE_LINE_MEMBER( cmi_state::cmi10_u20_cb2_w )
{
	// connected to alphanumeric display write strobe
	m_dp1->wr_w(state);
	m_dp2->wr_w(state);
	m_dp3->wr_w(state);
}

template <unsigned N> WRITE16_MEMBER( cmi_state::cmi_iix_update_dp )
{
	m_digit[(N << 2) | ((offset ^ 3) & 3)] = data;
}

/* Begin Conversion */
WRITE_LINE_MEMBER( cmi_state::cmi10_u21_cb2_w )
{
	// if 0
//  state = state;
}


READ8_MEMBER( cmi_state::cmi10_u21_a_r )
{
#if 0
//  int thld = m_cmi10_pia_u21->ca2_output();
	int sel = m_cmi10_pia_u20->a_output();
	int key = sel & 7;
	int mux = (sel >> 3) & 3;
	uint8_t data = 0x38; // slave keyboard not used


	for (int module = 0; module < 3; ++module)
	{
//      char keyname[16];
		uint8_t keyval;
		int state = 1;

		if (mux == 0 && key == 3)
		{
			//keyval = input_port_read(device->machine, "ANALOG");

			/* Unpressed */
			if (keyval <= 0)
				state = 1;
			/* In flight */

	#if 0
			else if (keyval <= 80)
			{
				if (thld == 1)
					state = 0;
				else
					state = 1;
			}
			/* Fully depressed */
	#endif
			else
				state = 0;

		}

		data |= state << module;
	}

	return data;
#else
	int sel = m_cmi10_pia_u20->a_output();
	int key = sel & 7;
	int mux = (sel >> 3) & 3;
	uint8_t data = 0xf8; // slave keyboard not used

	for (int module = 0; module < 3; ++module)
	{
		uint8_t keyval = m_key_mux_ports[mux][module]->read();
		data |= BIT(keyval, key) << module;
	}

	return data;
#endif
}

 /*************************************
 *
 *  6850 ACIAs
 *
 *************************************/

//static int kbd_to_cmi;
//static int cmi_to_kbd;

WRITE_LINE_MEMBER( cmi_state::mkbd_acia_clock )
{
	m_acia_mkbd_kbd->write_rxc(state);
	m_acia_mkbd_kbd->write_txc(state);
	m_acia_mkbd_cmi->write_rxc(state);
	m_acia_mkbd_cmi->write_txc(state);
	for (auto &acia : m_q133_acia)
		acia->write_rxc(state);
}

WRITE_LINE_MEMBER( cmi_state::msm5832_irq )
{
#if 0
	set_interrupt(CPU_2, IRQ_RTCINT_LEVEL, state ? ASSERT_LINE : CLEAR_LINE);
#endif
}

WRITE_LINE_MEMBER( cmi_state::mkbd_kbd_acia_int )
{
	m_mkbd_kbd_acia_irq = state;

	if (m_mkbd_kbd_acia_irq)
	{
		m_muskeyscpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
	else if (!m_mkbd_cmi_acia_irq)
	{
		m_muskeyscpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER( cmi_state::mkbd_cmi_acia_int )
{
	m_mkbd_cmi_acia_irq = state;

	if (m_mkbd_cmi_acia_irq)
		m_muskeyscpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else if (!m_mkbd_kbd_acia_irq)
		m_muskeyscpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE_LINE_MEMBER( cmi_state::cmi07_irq )
{
	m_cmi07cpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

void cmi_state::machine_reset()
{
	m_cpu1space = &m_maincpu1->space(AS_PROGRAM);
	m_cpu2space = &m_maincpu2->space(AS_PROGRAM);

	m_qfc9_region_ptr = (uint8_t *)m_qfc9_region->base();

	/* Set 8214 interrupt lines */
	m_i8214[0]->etlg_w(1);
	m_i8214[0]->inte_w(1);
	m_i8214[1]->etlg_w(1);
	m_i8214[1]->inte_w(1);
	m_i8214[2]->etlg_w(1);
	m_i8214[2]->inte_w(1);

	m_hblank_timer->adjust(m_screen->time_until_pos(0, HBLANK_START));

	m_scnd = 0;

	for (int cpunum = 0; cpunum < 2; ++cpunum)
	{
		address_space *space = (cpunum == CPU_1 ? m_cpu1space : m_cpu2space);

		space->unmap_readwrite(0x0000, 0xffff);

		/* Select A (system) spaces */
		m_cpu_active_space[cpunum] = MAPPING_A;

		install_peripherals(cpunum);

		m_irq_address[cpunum][0] = space->read_byte(0xfff8);
		m_irq_address[cpunum][1] = space->read_byte(0xfff9);
	}

	// TODO - we need to detect empty disk drive!!
	m_fdc_status |= FDC_STATUS_READY;

	/* CMI-07 */
	m_cmi07_ctrl = 0;
	m_cmi07cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_cmi10_scnd_timer->adjust(attotime::from_hz(4000000 / 4 / 2048 / 2), 0, attotime::from_hz(4000000 / 4 / 2048 / 2));
	m_scnd = 0;

	m_cmi02_ptm_irq = 0;
	m_m6809_bs_hack_cnt = 0;
	m_m6809_bs_hack_cpu = 0;
}

void cmi_state::machine_start()
{
	m_digit.resolve();

	m_q133_rom = (uint8_t *)m_q133_region->base();

	// allocate timers for the built-in two channel timer
	m_map_switch_timer = timer_alloc(TIMER_MAP_SWITCH);
	m_hblank_timer = timer_alloc(TIMER_HBLANK);
	m_cmi10_scnd_timer = timer_alloc(TIMER_CMI10_SCND);
	m_jam_timeout_timer = timer_alloc(TIMER_JAM_TIMEOUT);

	m_map_switch_timer->adjust(attotime::never);
	m_hblank_timer->adjust(attotime::never);
	m_cmi10_scnd_timer->adjust(attotime::never);
	m_jam_timeout_timer->adjust(attotime::never);

	/* Allocate 1kB memory mapping RAM */
	m_map_ram[0] = std::make_unique<uint8_t[]>(0x400);
	m_map_ram[1] = std::make_unique<uint8_t[]>(0x400);

	/* Allocate 256kB for each Q256 RAM card */
	m_q256_ram[0] = std::make_unique<uint8_t[]>(0x40000);
	m_q256_ram[1] = std::make_unique<uint8_t[]>(0x40000);

	/* Allocate 16kB video RAM */
	m_video_ram = std::make_unique<uint8_t[]>(0x4000);

	/* Allocate 512B shared RAM */
	m_shared_ram = std::make_unique<uint8_t[]>(0x200);

	/* Allocate 256B scratch RAM per CPU */
	m_scratch_ram[0] = std::make_unique<uint8_t[]>(0x100);
	m_scratch_ram[1] = std::make_unique<uint8_t[]>(0x100);

	m_msm5832->cs_w(1);
}

WRITE_LINE_MEMBER( cmi_state::cmi_iix_vblank )
{
	if (state)
	{
		/* VSYNC */
		m_q219_pia->cb2_w(1);
		m_q219_pia->cb2_w(0);

		/* LPSTB */
		m_q219_pia->cb1_w(0);
	}
}

static void cmi2x_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
	device.option_add("8dssd", FLOPPY_8_DSSD);
}

MACHINE_CONFIG_START(cmi_state::cmi2x)
	MCFG_DEVICE_ADD("maincpu1", MC6809E, Q209_CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(maincpu1_map)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(cmi_state, cpu1_interrupt_callback)
	config.m_perfect_cpu_quantum = subtag("maincpu1");

	MCFG_DEVICE_ADD("maincpu2", MC6809E, Q209_CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(maincpu2_map)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(cmi_state, cpu2_interrupt_callback)
	config.m_perfect_cpu_quantum = subtag("maincpu2");

	MCFG_DEVICE_ADD("muskeys", M6802, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(muskeys_map)

	MCFG_DEVICE_ADD("alphakeys", M6802, 3.84_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(alphakeys_map)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(cmi_state, irq0_line_hold, 3.84_MHz_XTAL / 400) // TODO: PIA controls this

	MCFG_DEVICE_ADD("smptemidi", M68000, 20_MHz_XTAL / 2)
	MCFG_DEVICE_PROGRAM_MAP(midicpu_map)

	MCFG_DEVICE_ADD("cmi07cpu", MC6809E, Q209_CPU_CLOCK) // ?
	MCFG_DEVICE_PROGRAM_MAP(cmi07cpu_map)

	/* alpha-numeric display */
	DL1416T(config, m_dp1, u32(0));
	m_dp1->update().set(FUNC(cmi_state::cmi_iix_update_dp<0>));
	DL1416T(config, m_dp2, u32(0));
	m_dp2->update().set(FUNC(cmi_state::cmi_iix_update_dp<1>));
	DL1416T(config, m_dp3, u32(0));
	m_dp3->update().set(FUNC(cmi_state::cmi_iix_update_dp<2>));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBLANK_END, HBLANK_START, VTOTAL, VBLANK_END, VBLANK_START);
	m_screen->set_screen_update(FUNC(cmi_state::screen_update_cmi2x));
	m_screen->screen_vblank().set(FUNC(cmi_state::cmi_iix_vblank));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MSM5832(config, m_msm5832, 32.768_kHz_XTAL);

	I8214(config, m_i8214[0], 1000000); // cmi_8214_intf_1
	m_i8214[0]->int_wr_callback().set(FUNC(cmi_state::i8214_1_int_w));
	I8214(config, m_i8214[1], 1000000); // cmi_8214_intf_2
	m_i8214[1]->int_wr_callback().set(FUNC(cmi_state::i8214_2_int_w));
	I8214(config, m_i8214[2], 1000000); // cmi_8214_intf_3
	m_i8214[2]->int_wr_callback().set(FUNC(cmi_state::i8214_3_int_w));
	m_i8214[2]->enlg_wr_callback().set(FUNC(cmi_state::i8214_3_enlg));

	PIA6821(config, m_q133_pia[0], 0); // pia_q133_1_config
	m_q133_pia[0]->readpa_handler().set(FUNC(cmi_state::q133_1_porta_r));
	m_q133_pia[0]->writepa_handler().set(FUNC(cmi_state::q133_1_porta_w));
	m_q133_pia[0]->writepb_handler().set(FUNC(cmi_state::q133_1_portb_w));

	PIA6821(config, m_q133_pia[1], 0); // pia_q133_2_config

	PTM6840(config, m_q133_ptm, 2000000); // ptm_q133_config
	m_q133_ptm->set_external_clocks(1024, 1, 111); // Third is todo

	PIA6821(config, m_q219_pia, 0); // pia_q219_config
	m_q219_pia->readpb_handler().set(FUNC(cmi_state::pia_q219_b_r));
	m_q219_pia->writepa_handler().set(FUNC(cmi_state::vscroll_w));
	m_q219_pia->writepb_handler().set(FUNC(cmi_state::video_attr_w));
	m_q219_pia->irqa_handler().set(FUNC(cmi_state::pia_q219_irqa));
	m_q219_pia->irqb_handler().set(FUNC(cmi_state::pia_q219_irqb));

	PTM6840(config, m_q219_ptm, 2000000); // ptm_q219_config
	m_q219_ptm->set_external_clocks(HBLANK_FREQ.dvalue(), VBLANK_FREQ.dvalue(), 1'000'000); // TODO: does the third thing come from a crystal?
	m_q219_ptm->irq_callback().set(FUNC(cmi_state::ptm_q219_irq));

	PIA6821(config, m_cmi02_pia[0], 0); // pia_cmi02_1_config
	m_cmi02_pia[0]->writepb_handler().set(FUNC(cmi_state::master_tune_w));

	PIA6821(config, m_cmi02_pia[1], 0); // pia_cmi02_2_config

	PTM6840(config, m_cmi02_ptm, 2000000); // ptm_cmi02_config TODO
	m_cmi02_ptm->o2_callback().set(FUNC(cmi_state::cmi02_ptm_o2));
	m_cmi02_ptm->irq_callback().set(FUNC(cmi_state::cmi02_ptm_irq));

	clock_device &mkbd_acia_clock(CLOCK(config, "mkbd_acia_clock", 1.8432_MHz_XTAL / 12));
	mkbd_acia_clock.signal_handler().set(FUNC(cmi_state::mkbd_acia_clock));

	for (auto &acia : m_q133_acia)
		MOS6551(config, acia, 1.8432_MHz_XTAL).set_xtal(1.8432_MHz_XTAL);
	m_q133_acia[0]->irq_handler().set("q133_acia_irq", FUNC(input_merger_device::in_w<0>));
	m_q133_acia[1]->irq_handler().set("q133_acia_irq", FUNC(input_merger_device::in_w<1>));
	m_q133_acia[2]->irq_handler().set("q133_acia_irq", FUNC(input_merger_device::in_w<2>));
	m_q133_acia[3]->irq_handler().set("q133_acia_irq", FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, "q133_acia_irq").output_handler().set(FUNC(cmi_state::q133_acia_irq));

	ACIA6850(config, m_acia_mkbd_kbd, 1.8432_MHz_XTAL / 12); // acia_mkbd_kbd
	ACIA6850(config, m_acia_mkbd_cmi, 1.8432_MHz_XTAL / 12); // acia_mkbd_cmi
	PIA6821(config, m_ank_pia, 0); // pia_ank_config

	m_q133_acia[0]->txd_handler().set(m_acia_mkbd_cmi, FUNC(acia6850_device::write_rxd));
	m_q133_acia[0]->rts_handler().set(m_acia_mkbd_cmi, FUNC(acia6850_device::write_cts));

	m_acia_mkbd_cmi->txd_handler().set("q133_acia_0", FUNC(mos6551_device::write_rxd));
	m_acia_mkbd_cmi->rts_handler().set("q133_acia_0", FUNC(mos6551_device::write_cts));
	m_acia_mkbd_cmi->irq_handler().set(FUNC(cmi_state::mkbd_cmi_acia_int));

	m_acia_mkbd_kbd->txd_handler().set("ank_pia", FUNC(pia6821_device::cb2_w));
	m_acia_mkbd_kbd->rts_handler().set("ank_pia", FUNC(pia6821_device::ca2_w));
	m_acia_mkbd_kbd->irq_handler().set(FUNC(cmi_state::mkbd_kbd_acia_int));

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_alphakeyscpu, M6802_IRQ_LINE);

	m_ank_pia->readpa_handler().set(FUNC(cmi_state::ank_col_r));
	m_ank_pia->readcb1_handler().set(FUNC(cmi_state::ank_rts_r));
	m_ank_pia->ca2_handler().set("acia_mkbd_kbd", FUNC(acia6850_device::write_cts));
	m_ank_pia->cb2_handler().set("acia_mkbd_kbd", FUNC(acia6850_device::write_rxd));
	m_ank_pia->irqa_handler().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_ank_pia->irqb_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	clock_device &ank_pia_clock(CLOCK(config, "ank_pia_clock", 9600));
	ank_pia_clock.signal_handler().set(m_ank_pia, FUNC(pia6821_device::ca1_w));

	PTM6840(config, m_cmi07_ptm, 2000000); // ptm_cmi07_config TODO
	m_cmi07_ptm->irq_callback().set(FUNC(cmi_state::cmi07_irq));

	FD1791(config, m_wd1791, 16_MHz_XTAL / 8); // wd1791_interface
	m_wd1791->intrq_wr_callback().set(FUNC(cmi_state::wd1791_irq));
	m_wd1791->drq_wr_callback().set(FUNC(cmi_state::wd1791_drq));
	FLOPPY_CONNECTOR(config, "wd1791:0", cmi2x_floppies, "8dsdd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1791:1", cmi2x_floppies, "8dsdd", floppy_image_device::default_floppy_formats);

	/* Musical keyboard */
	PIA6821(config, m_cmi10_pia_u20, 0);
	m_cmi10_pia_u20->readcb1_handler().set(FUNC(cmi_state::cmi10_u20_cb1_r));
	m_cmi10_pia_u20->writepa_handler().set(FUNC(cmi_state::cmi10_u20_a_w));
	m_cmi10_pia_u20->writepb_handler().set(FUNC(cmi_state::cmi10_u20_b_w));
	m_cmi10_pia_u20->cb2_handler().set(FUNC(cmi_state::cmi10_u20_cb2_w));

	PIA6821(config, m_cmi10_pia_u21, 0);
	m_cmi10_pia_u21->readpa_handler().set(FUNC(cmi_state::cmi10_u21_a_r));
	m_cmi10_pia_u21->cb2_handler().set(FUNC(cmi_state::cmi10_u21_cb2_w));

	SPEAKER(config, "mono").front_center();

	// Channel cards
	cmi01a_device &cmi01a_0(CMI01A_CHANNEL_CARD(config, "cmi01a_0", 0));
	cmi01a_0.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_0.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_1(CMI01A_CHANNEL_CARD(config, "cmi01a_1", 0));
	cmi01a_1.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_1.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_2(CMI01A_CHANNEL_CARD(config, "cmi01a_2", 0));
	cmi01a_2.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_2.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_3(CMI01A_CHANNEL_CARD(config, "cmi01a_3", 0));
	cmi01a_3.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_3.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_4(CMI01A_CHANNEL_CARD(config, "cmi01a_4", 0));
	cmi01a_4.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_4.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_5(CMI01A_CHANNEL_CARD(config, "cmi01a_5", 0));
	cmi01a_5.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_5.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_6(CMI01A_CHANNEL_CARD(config, "cmi01a_6", 0));
	cmi01a_6.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_6.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
	cmi01a_device &cmi01a_7(CMI01A_CHANNEL_CARD(config, "cmi01a_7", 0));
	cmi01a_7.add_route(ALL_OUTPUTS, "mono", 0.25);
	cmi01a_7.irq_callback().set(FUNC(cmi_state::channel_irq<0>));
MACHINE_CONFIG_END

ROM_START( cmi2x )
	/* Q133 Processor control card */
	ROM_REGION( 0x3000, "q133", 0 )
	ROM_LOAD( "q9f0mrk1.bin", 0x000, 0x800, CRC(16f195cc) SHA1(fcc4be370ba60ae5a4145c36cdbdc97a7be91f8f) )
	ROM_LOAD( "f8lmrk5.bin",  0x800, 0x800, CRC(cfc7967f) SHA1(0695cc757cf6fab35414dc068dd2a3e50084685c) )

	/* For CPU1 */
	ROM_COPY( "q133", 0x000, 0x1000, 0x800 )
	ROM_COPY( "q133", 0x800, 0x1800, 0x400 )

	/* For CPU2 */
	ROM_COPY( "q133", 0x000, 0x2000, 0x800 )
	ROM_COPY( "q133", 0xc00, 0x2800, 0x400 )

	/* General Interface (SMPTE/MIDI) CPU */
	ROM_REGION( 0x4000, "smptemidi", 0 )
	ROM_LOAD16_BYTE( "mon1110e.bin", 0x0000, 0x2000, CRC(476f7d5f) SHA1(9af21e0072eaa58cae42947c20dca05d35dfadd0) )
	ROM_LOAD16_BYTE( "mon1110o.bin", 0x0001, 0x2000, CRC(150c8ebe) SHA1(bbd371bebac29628f60537832d0587e83323ad01) )

	/* QFC9 Floppy disk controller driver */
	ROM_REGION( 0x800, "qfc9", 0 )
	ROM_LOAD( "dqfc911.bin", 0x00, 0x800, CRC(5bc38db2) SHA1(bd840e19e51a336e669c40b9e18cdaf6b3c62a8a) )

	/* Musical keyboard CPU */
	// Both of these dumps have been trimmed to size from within a roughly 2x-bigger file.
	// The actual size is known based on the format apparently used by the dumping device, shared with the prom
	// dumps and cmikeys4.bin dump.
	ROM_REGION( 0x10000, "muskeys", 0 )
	ROM_LOAD( "velkeysd.bin", 0xb000, 0x0400, CRC(9b636781) SHA1(be29a72a1d6d313dafe0b63951b5e3e18ddb9a21) )
	ROM_LOAD( "kbdioa.bin",   0xfc00, 0x0400, CRC(a5cbe218) SHA1(bc6784aaa5697c28eab126e20500139b8d0c1f50) )

	/* Alphanumeric keyboard CPU */
	// This dump has been trimmed to size from within a roughly 2x-bigger file. The actual size is known based
	// on the format apparently used by the dumping device, shared with the prom dumps and music keys dump.
	ROM_REGION( 0x10000, "alphakeys", 0 )
	ROM_LOAD( "cmikeys4.bin", 0xc000, 0x400, CRC(b214fbe9) SHA1(8c404f58ba3e5a50aa42f761e966c74374e96cc9) )

	// All of these PROM dumps have been trimmed to size from within a roughly 2x-bigger file.
	// The actual sizes are known from the schematics and the starting address of the actual PROM data was obvious
	// based on repeated data in some of the 256x4 PROMs, but it would be nice to get redumps, in the extremely
	// unlikely event that someone finds a CMI IIx for sale.
	ROM_REGION( 0x420, "proms", 0 )
	ROM_LOAD( "brom.bin",   0x000, 0x100, CRC(3f730d15) SHA1(095df6eee95b9ad6418b910fb5d2ae46913750f9) ) // Unknown use, lightgun/graphics card
	ROM_LOAD( "srom.bin",   0x100, 0x100, CRC(a1b4b71b) SHA1(6ea96480af2f1e43967f209218a74fc17972ce0e) ) // Used to generate signal timing for lightpen
	ROM_LOAD( "mrom.bin",   0x200, 0x100, CRC(dc26642c) SHA1(49b207ff80d1b055c3b855dc954129846c49bfe3) ) // Unknown use, master card
	ROM_LOAD( "timrom.bin", 0x300, 0x100, CRC(a426e4a2) SHA1(6b7ea128c730f5afd1042820ccd55bbda683afd8) ) // Unknown use, master card
	ROM_LOAD( "wrom.bin",   0x400, 0x020, CRC(68a9e17f) SHA1(c3364a37a8d19a1882d7910add1c1df9b63ee32c) ) // Unknown use, lightgun/graphics card
ROM_END

/* TODO: Machine start? */
void cmi_state::init_cmi2x()
{
}

CONS( 1983, cmi2x, 0, 0, cmi2x, cmi2x, cmi_state, init_cmi2x, "Fairlight", "CMI IIx", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
