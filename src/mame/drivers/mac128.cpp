// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/****************************************************************************

    drivers/mac128.cpp
    Original-style Macintosh family emulation

    The cutoff here is Macs with 128k-style video and audio and no ADB

    Nate Woods, Raphael Nabet, R. Belmont

        0x000000 - 0x3fffff     RAM/ROM (switches based on overlay)
        0x400000 - 0x4fffff     ROM
        0x580000 - 0x5fffff     5380 NCR/Symbios SCSI peripherals chip (Mac Plus only)
        0x600000 - 0x6fffff     RAM
        0x800000 - 0x9fffff     Zilog 8530 SCC (Serial Communications Controller) Read
        0xa00000 - 0xbfffff     Zilog 8530 SCC (Serial Communications Controller) Write
        0xc00000 - 0xdfffff     IWM (Integrated Woz Machine; floppy)
        0xe80000 - 0xefffff     Rockwell 6522 VIA
        0xf00000 - 0xffffef     ??? (the ROM appears to be accessing here)
        0xfffff0 - 0xffffff     Auto Vector

    Interrupts:
        M68K:
            Level 1 from VIA
            Level 2 from SCC
            Level 4 : Interrupt switch (not implemented)

        VIA:
            CA1 from VBLANK
            CA2 from 1 Hz clock (RTC)
            CB1 from Keyboard Clock
            CB2 from Keyboard Data
            SR  from Keyboard Data Ready

        SCC:
            PB_EXT (DCDB)  from mouse Y circuitry
            PA_EXT (DCDA)  from mouse X circuitry

SCC Init:

Control B:
09   select reg 9
40   reset channel B
04   select WR4
4c   2 stop bits, x16 clock
02   select WR2
00   int vector = 0
03   select WR3
c0   8 data bits, Rx disabled
0f   select WR15, external interrupt status/control
08   enable 08
00   select WR0
10   reset external status/control interrupt
00   select wR0
10   reset external again
01   select WR1
01   enable external interrupts

Control A:
09   select reg 9
80   reset channel A
04   select WR4
4c   2 stop bits, x16 clock
03   select WR3
c0   8 data bits, Rx disabled
0f   select WR15, external interrupt status/control
08   enable 08
00   select WR0
10   reset external status/control interrupt
00   select WR0
10   reset again
01   select WR1
01   enable external interrupts
09   select WR9
0a   enable MIE / NV


****************************************************************************/

#include "emu.h"

#include "machine/mackbd.h"
#include "machine/macrtc.h"

#include "bus/macpds/hyperdrive.h"
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "machine/6522via.h"
#include "machine/applefdc.h"
#include "machine/ncr5380.h"
#include "machine/ram.h"
#include "machine/sonydriv.h"
#include "machine/swim.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "formats/ap_dsk35.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


#define MAC_SCREEN_NAME "screen"
#define MAC_539X_1_TAG "539x_1"
#define MAC_539X_2_TAG "539x_2"
#define DAC_TAG "macdac"
#define SCC_TAG "scc"

#define C7M (15.6672_MHz_XTAL / 2)
#define C3_7M (15.6672_MHz_XTAL / 4).value()

/* tells which model is being emulated (set by macxxx_init) */
enum mac128model_t
{
	MODEL_MAC_128K512K, // 68000 machines
	MODEL_MAC_512KE,
	MODEL_MAC_PLUS
};

// video parameters
#define MAC_H_VIS   (512)
#define MAC_V_VIS   (342)
#define MAC_H_TOTAL (704)       // (512+192)
#define MAC_V_TOTAL (370)       // (342+28)

// sound buffer locations
#define MAC_MAIN_SND_BUF_OFFSET (0x0300>>1)
#define MAC_ALT_SND_BUF_OFFSET  (0x5F00>>1)

#define LOG_KEYBOARD    0
#define LOG_GENERAL     0
#define LOG_MAC_IWM     0
#define LOG_VIA         0
#define LOG_MEMORY      0

class mac128_state : public driver_device
{
public:
	mac128_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via6522_0"),
		m_ram(*this, RAM_TAG),
		m_ncr5380(*this, "ncr5380"),
		m_iwm(*this, "fdc"),
		m_mackbd(*this, "mackbd"),
		m_rtc(*this,"rtc"),
		m_mouse0(*this, "MOUSE0"),
		m_mouse1(*this, "MOUSE1"),
		m_mouse2(*this, "MOUSE2"),
		m_screen(*this, "screen"),
		m_dac(*this, DAC_TAG),
		m_scc(*this, SCC_TAG)
	{
	}

	void mac512ke(machine_config &config);
	void mac128k(machine_config &config);
	void macplus(machine_config &config);

	void init_mac128k512k();
	void init_mac512ke();
	void init_macplus();

private:
	required_device<m68000_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<ram_device> m_ram;
	optional_device<ncr5380_device> m_ncr5380;
	required_device<applefdc_base_device> m_iwm;
	required_device<mackbd_device> m_mackbd;
	optional_device<rtc3430042_device> m_rtc;

	required_ioport m_mouse0, m_mouse1, m_mouse2;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	mac128model_t m_model;

	uint32_t m_overlay;

	int m_irq_count, m_ca1_data, m_ca2_data;

	int m_mouse_bit_x;
	int m_mouse_bit_y;
	int last_mx, last_my;
	int count_x, count_y;
	int m_last_was_x;
	int m_screen_buffer;
	emu_timer *m_scan_timer;
	emu_timer *m_hblank_timer;

	// interrupts
	int m_scc_interrupt, m_via_interrupt, m_scsi_interrupt, m_last_taken_interrupt;

	void scc_mouse_irq( int x, int y );
	void set_via_interrupt(int value);
	void field_interrupts();
	void vblank_irq();
	void mouse_callback();

	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ram_600000_r(offs_t offset);
	void ram_600000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~ 0);
	uint16_t mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, uint16_t data);
	uint16_t mac_autovector_r(offs_t offset);
	void mac_autovector_w(offs_t offset, uint16_t data);
	uint16_t mac_iwm_r(offs_t offset, uint16_t mem_mask = ~0);
	void mac_iwm_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t macplus_scsi_r(offs_t offset, uint16_t mem_mask = ~0);
	void macplus_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(mac_scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(set_scc_interrupt);

	TIMER_CALLBACK_MEMBER(mac_scanline);
	TIMER_CALLBACK_MEMBER(mac_hblank);
	uint32_t screen_update_mac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t mac_via_in_a();
	uint8_t mac_via_in_b();
	void mac_via_out_a(uint8_t data);
	void mac_via_out_b(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(mac_via_irq);
	void mac_driver_init(mac128model_t model);
	void update_volume();

	void mac512ke_map(address_map &map);
	void macplus_map(address_map &map);
private:
	// wait states for accessing the VIA
	int m_via_cycles;
	bool m_snd_enable;
	bool m_main_buffer;
	int m_snd_vol;
	u16 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_mask, m_ram_size;

	required_device<screen_device> m_screen;
	required_device<dac_8bit_pwm_device> m_dac;
	required_device<z80scc_device> m_scc;
};

void mac128_state::machine_start()
{
	m_ram_ptr = (u16*)m_ram->pointer();
	m_ram_size = m_ram->size()>>1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u16*)memregion("bootrom")->base();

	m_scan_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac128_state::mac_scanline), this));
	m_hblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac128_state::mac_hblank), this));
}

void mac128_state::machine_reset()
{
	m_via_cycles = -10;
	m_last_taken_interrupt = -1;
	m_overlay = 1;
	m_screen_buffer = 1;
	m_mouse_bit_x = m_mouse_bit_y = 0;
	m_last_taken_interrupt = 0;
	m_snd_enable = false;
	m_main_buffer = true;
	m_snd_vol = 3;
	m_irq_count = 0;
	m_ca1_data = 0;
	m_ca2_data = 0;

	const int next_vpos = m_screen->vpos() + 1;
	m_scan_timer->adjust(m_screen->time_until_pos(next_vpos), next_vpos);
	if (m_screen->vblank())
		m_via->write_pb6(0);
}

uint16_t mac128_state::ram_r(offs_t offset)
{
	if (m_overlay)
	{
		return m_rom_ptr[offset];
	}

	return m_ram_ptr[offset & m_ram_mask];
}

void mac128_state::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!m_overlay)
	{
		COMBINE_DATA(&m_ram_ptr[offset & m_ram_mask]);
	}
}

uint16_t mac128_state::ram_600000_r(offs_t offset)
{
	return m_ram_ptr[offset & m_ram_mask];
}

void mac128_state::ram_600000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram_ptr[offset & m_ram_mask]);
}

void mac128_state::field_interrupts()
{
	int take_interrupt = -1;

	if ((m_scc_interrupt) || (m_scsi_interrupt))
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

//  printf("field_interrupts: take %d\n", take_interrupt);

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

WRITE_LINE_MEMBER(mac128_state::set_scc_interrupt)
{
//  printf("SCC IRQ: %d\n", state);
	m_scc_interrupt = state;
	field_interrupts();
}

void mac128_state::set_via_interrupt(int value)
{
	m_via_interrupt = value;
	this->field_interrupts();
}

void mac128_state::vblank_irq()
{
	m_ca1_data ^= 1;
	m_via->write_ca1(m_ca1_data);

	if (++m_irq_count == 60)
	{
		m_irq_count = 0;

		m_ca2_data ^= 1;
		/* signal 1 Hz irq on CA2 input on the VIA */
		m_via->write_ca2(m_ca2_data);
	}
}

void mac128_state::update_volume()
{
	if (!m_snd_enable)
	{
		// ls161 clear input
		m_dac->set_output_gain(ALL_OUTPUTS, 0);
	}
	else
	{
		// sound -> r13 (470k)
		// sound -> r12 (470k) -> 4016 (pa0 != 0)
		// sound -> r17 (150k) -> 4016 (pa1 != 0)
		// sound -> r16 (68k)  -> 4016 (pa2 != 0)
		m_dac->set_output_gain(ALL_OUTPUTS, 8.0 / (m_snd_vol + 1));
	}
}

TIMER_CALLBACK_MEMBER(mac128_state::mac_scanline)
{
	int scanline = param;
	uint16_t *mac_snd_buf_ptr;

	if (scanline == MAC_V_VIS)
	{
		vblank_irq();
	}

	/* video beam in display (! VBLANK && ! HBLANK basically) */
	if (scanline < MAC_V_VIS)
	{
		m_via->write_pb6(1);
		m_hblank_timer->adjust(m_screen->time_until_pos(scanline, MAC_H_VIS));
	}

	if (!(scanline % 10))
	{
		mouse_callback();
	}

	if (m_main_buffer)
	{
		mac_snd_buf_ptr = (uint16_t *)(m_ram_ptr + m_ram_size - MAC_MAIN_SND_BUF_OFFSET);
	}
	else
	{
		mac_snd_buf_ptr = (uint16_t *)(m_ram_ptr + m_ram_size - MAC_ALT_SND_BUF_OFFSET);
	}

	m_dac->write(mac_snd_buf_ptr[scanline] >> 8);
	m_scan_timer->adjust(m_screen->time_until_pos(scanline+1), (scanline+1) % m_screen->height());
}

TIMER_CALLBACK_MEMBER(mac128_state::mac_hblank)
{
	m_via->write_pb6(0);
}

WRITE_LINE_MEMBER(mac128_state::mac_scsi_irq)
{
}

uint16_t mac128_state::macplus_scsi_r(offs_t offset, uint16_t mem_mask)
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	if ((reg == 6) && (offset == 0x130))
	{
		reg = R5380_CURDATA_DTACK;
	}

	return m_ncr5380->ncr5380_read_reg(reg)<<8;
}

void mac128_state::macplus_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_w: data %x offset %x mask %x\n", data, offset, mem_mask);

	if ((reg == 0) && (offset == 0x100))
	{
		reg = R5380_OUTDATA_DTACK;
	}

	m_ncr5380->ncr5380_write_reg(reg, data);
}

void mac128_state::scc_mouse_irq(int x, int y)
{
	static int lasty = 0;
	static int lastx = 0;

	// DCD lines are active low in hardware but active high to software
	if (x && y)
	{
		if (m_last_was_x)
		{
			if(x == 2)
			{
				if(lastx)
				{
					m_scc->dcda_w(1);
					m_mouse_bit_x = 0;
				}
				else
				{
					m_scc->dcda_w(0);
					m_mouse_bit_x = 1;
				}
			}
			else
			{
				if(lastx)
				{
					m_scc->dcda_w(1);
					m_mouse_bit_x = 1;
				}
				else
				{
					m_scc->dcda_w(0);
					m_mouse_bit_x = 0;
				}
			}
			lastx = !lastx;
		}
		else
		{
			if(y == 2)
			{
				if(lasty)
				{
					m_scc->dcdb_w(1);
					m_mouse_bit_y = 0;
				}
				else
				{
					m_scc->dcdb_w(0);
					m_mouse_bit_y = 1;
				}
			}
			else
			{
				if(lasty)
				{
					m_scc->dcdb_w(1);
					m_mouse_bit_y = 1;
				}
				else
				{
					m_scc->dcdb_w(0);
					m_mouse_bit_y = 0;
				}
			}
			lasty = !lasty;
		}

		m_last_was_x ^= 1;
	}
	else
	{
		if (x)
		{
			if(x == 2)
			{
				if(lastx)
				{
					m_scc->dcda_w(1);
					m_mouse_bit_x = 0;
				}
				else
				{
					m_scc->dcda_w(0);
					m_mouse_bit_x = 1;
				}
			}
			else
			{
				if(lastx)
				{
					m_scc->dcda_w(1);
					m_mouse_bit_x = 1;
				}
				else
				{
					m_scc->dcda_w(0);
					m_mouse_bit_x = 0;
				}
			}
			lastx = !lastx;
		}
		else
		{
			if(y == 2)
			{
				if(lasty)
				{
					m_scc->dcdb_w(1);
					m_mouse_bit_y = 0;
				}
				else
				{
					m_scc->dcdb_w(0);
					m_mouse_bit_y = 1;
				}
			}
			else
			{
				if(lasty)
				{
					m_scc->dcdb_w(1);
					m_mouse_bit_y = 1;
				}
				else
				{
					m_scc->dcdb_w(0);
					m_mouse_bit_y = 0;
				}
			}
			lasty = !lasty;
		}
	}
}

uint16_t mac128_state::mac_iwm_r(offs_t offset, uint16_t mem_mask)
{
	/* The first time this is called is in a floppy test, which goes from
	 * $400104 to $400126.  After that, all access to the floppy goes through
	 * the disk driver in the MacOS
	 *
	 * I just thought this would be on interest to someone trying to further
	 * this driver along
	 */

	uint16_t result = 0;

	result = m_iwm->read(offset >> 8);

	if (LOG_MAC_IWM)
		printf("mac_iwm_r: offset=0x%08x mem_mask %04x = %02x (PC %x)\n", offset, mem_mask, result, m_maincpu->pc());

	return (result << 8) | result;
}

void mac128_state::mac_iwm_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (LOG_MAC_IWM)
		printf("mac_iwm_w: offset=0x%08x data=0x%04x mask %04x (PC=%x)\n", offset, data, mem_mask, m_maincpu->pc());

	if (ACCESSING_BITS_0_7)
		m_iwm->write((offset >> 8), data & 0xff);
	else
		m_iwm->write((offset >> 8), data>>8);
}

WRITE_LINE_MEMBER(mac128_state::mac_via_irq)
{
	/* interrupt the 68k (level 1) */
	set_via_interrupt(state);
}

uint16_t mac128_state::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_r: offset=0x%02x\n", offset);
	data = m_via->read(offset);

	m_maincpu->adjust_icount(m_via_cycles);

	return (data & 0xff) | (data << 8);
}

void mac128_state::mac_via_w(offs_t offset, uint16_t data)
{
	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_w: offset=0x%02x data=0x%08x\n", offset, data);

	m_via->write(offset, (data >> 8) & 0xff);

	m_maincpu->adjust_icount(m_via_cycles);
}

void mac128_state::mac_autovector_w(offs_t offset, uint16_t data)
{
	if (LOG_GENERAL)
		logerror("mac_autovector_w: offset=0x%08x data=0x%04x\n", offset, data);

	/* This should throw an exception */

	/* Not yet implemented */
}

uint16_t mac128_state::mac_autovector_r(offs_t offset)
{
	if (LOG_GENERAL)
		logerror("mac_autovector_r: offset=0x%08x\n", offset);

	/* This should throw an exception */

	/* Not yet implemented */
	return 0;
}

uint8_t mac128_state::mac_via_in_a()
{
	return 0x80;
}

uint8_t mac128_state::mac_via_in_b()
{
	int val = 0x40;

	if (m_mouse_bit_y)  /* Mouse Y2 */
		val |= 0x20;
	if (m_mouse_bit_x)  /* Mouse X2 */
		val |= 0x10;
	if ((m_mouse0->read() & 0x01) == 0)
		val |= 0x08;

	val |= m_rtc->data_r();

//  printf("%s VIA1 IN_B = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

void mac128_state::mac_via_out_a(uint8_t data)
{
//  printf("%s VIA1 OUT A: %02x (PC %x)\n", machine().describe_context().c_str(), data);

	//set_scc_waitrequest((data & 0x80) >> 7);
	m_screen_buffer = (data & 0x40) >> 6;
	sony_set_sel_line(m_iwm, (data & 0x20) >> 5);

	m_main_buffer = ((data & 0x08) == 0x08) ? true : false;
	m_snd_vol = data & 0x07;
	update_volume();

	/* Early Mac models had VIA A4 control overlaying.  In the Mac SE (and
	 * possibly later models), overlay was set on reset, but cleared on the
	 * first access to the ROM. */

	if (((data & 0x10) >> 4) != m_overlay)
	{
		m_overlay = (data & 0x10) >> 4;
	}
}

void mac128_state::mac_via_out_b(uint8_t data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);

	m_snd_enable = ((data & 0x80) == 0) ? true : false;
	update_volume();
	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

/* *************************************************************************
 * Mouse
 * *************************************************************************/

void mac128_state::mouse_callback()
{
	int     new_mx, new_my;
	int     x_needs_update = 0, y_needs_update = 0;

	new_mx = m_mouse1->read();
	new_my = m_mouse2->read();

	/* see if it moved in the x coord */
	if (new_mx != last_mx)
	{
		int     diff = new_mx - last_mx;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		count_x += diff;

		last_mx = new_mx;
	}
	/* see if it moved in the y coord */
	if (new_my != last_my)
	{
		int     diff = new_my - last_my;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		count_y += diff;

		last_my = new_my;
	}

	/* update any remaining count and then return */
	if (count_x)
	{
		if (count_x < 0)
		{
			count_x++;
			m_mouse_bit_x = 0;
			x_needs_update = 2;
		}
		else
		{
			count_x--;
			m_mouse_bit_x = 1;
			x_needs_update = 1;
		}
	}
	else if (count_y)
	{
		if (count_y < 0)
		{
			count_y++;
			m_mouse_bit_y = 1;
			y_needs_update = 1;
		}
		else
		{
			count_y--;
			m_mouse_bit_y = 0;
			y_needs_update = 2;
		}
	}

	if (x_needs_update || y_needs_update)
	{
		/* assert Port B External Interrupt on the SCC */
		scc_mouse_irq(x_needs_update, y_needs_update );
	}
}

void mac128_state::mac_driver_init(mac128model_t model)
{
	m_scsi_interrupt = 0;
	m_model = model;
#if 0
	/* set up RAM mirror at 0x600000-0x6fffff (0x7fffff ???) */
	mac_install_memory(0x600000, 0x6fffff, m_ram->size(), m_ram->pointer(), FALSE, "bank2");

	/* set up ROM at 0x400000-0x4fffff (-0x5fffff for mac 128k/512k/512ke) */
	mac_install_memory(0x400000, (model >= MODEL_MAC_PLUS) ? 0x4fffff : 0x5fffff,
		memregion("bootrom")->bytes(), memregion("bootrom")->base(), TRUE, "bank3");
#endif

	memset(m_ram->pointer(), 0, m_ram->size());
}

#define MAC_MAIN_SCREEN_BUF_OFFSET  (0x5900>>1)
#define MAC_ALT_SCREEN_BUF_OFFSET   (0xD900>>1)

uint32_t mac128_state::screen_update_mac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t video_base;
	const uint16_t *video_ram;
	uint16_t word;
	uint16_t *line;
	int y, x, b;

	video_base = m_ram_size - (m_screen_buffer ? MAC_MAIN_SCREEN_BUF_OFFSET : MAC_ALT_SCREEN_BUF_OFFSET);
	video_ram = (const uint16_t *) (m_ram_ptr + video_base);

	for (y = 0; y < MAC_V_VIS; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < MAC_H_VIS; x += 16)
		{
			word = *(video_ram++);
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

#define MAC_DRIVER_INIT(label, model)   \
void mac128_state::init_##label()     \
{   \
	mac_driver_init(model); \
}

MAC_DRIVER_INIT(mac128k512k, MODEL_MAC_128K512K)
MAC_DRIVER_INIT(mac512ke, MODEL_MAC_512KE)
MAC_DRIVER_INIT(macplus, MODEL_MAC_PLUS)

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mac128_state::mac512ke_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(FUNC(mac128_state::ram_r), FUNC(mac128_state::ram_w));
	map(0x400000, 0x4fffff).rom().region("bootrom", 0).mirror(0x100000);
	map(0x600000, 0x6fffff).rw(FUNC(mac128_state::ram_600000_r), FUNC(mac128_state::ram_600000_w));
	map(0x800000, 0x9fffff).r(m_scc, FUNC(z80scc_device::dc_ab_r)).umask16(0xff00);
	map(0xa00000, 0xbfffff).w(m_scc, FUNC(z80scc_device::dc_ab_w)).umask16(0x00ff);
	map(0xc00000, 0xdfffff).rw(FUNC(mac128_state::mac_iwm_r), FUNC(mac128_state::mac_iwm_w));
	map(0xe80000, 0xefffff).rw(FUNC(mac128_state::mac_via_r), FUNC(mac128_state::mac_via_w));
	map(0xfffff0, 0xffffff).rw(FUNC(mac128_state::mac_autovector_r), FUNC(mac128_state::mac_autovector_w));
}

void mac128_state::macplus_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(FUNC(mac128_state::ram_r), FUNC(mac128_state::ram_w));
	map(0x400000, 0x4fffff).rom().region("bootrom", 0);
	map(0x580000, 0x5fffff).rw(FUNC(mac128_state::macplus_scsi_r), FUNC(mac128_state::macplus_scsi_w));
	map(0x800000, 0x9fffff).r(m_scc, FUNC(z80scc_device::dc_ab_r)).umask16(0xff00);
	map(0xa00000, 0xbfffff).w(m_scc, FUNC(z80scc_device::dc_ab_w)).umask16(0x00ff);
	map(0xc00000, 0xdfffff).rw(FUNC(mac128_state::mac_iwm_r), FUNC(mac128_state::mac_iwm_w));
	map(0xe80000, 0xefffff).rw(FUNC(mac128_state::mac_via_r), FUNC(mac128_state::mac_via_w));
	map(0xfffff0, 0xffffff).rw(FUNC(mac128_state::mac_autovector_r), FUNC(mac128_state::mac_autovector_w));
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static const applefdc_interface mac_iwm_interface =
{
	sony_set_lines,
	sony_set_enable_lines,

	sony_read_data,
	sony_write_data,
	sony_read_status
};

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const floppy_interface mac_floppy_interface =
{
	FLOPPY_STANDARD_3_5_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_mac),
	"floppy_3_5"
};

static void mac_pds_cards(device_slot_interface &device)
{
	device.option_add("hyperdrive", PDS_HYPERDRIVE);  // GCC HyperDrive ST-506 interface
}

void mac128_state::mac512ke(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, C7M);        /* 7.8336 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mac128_state::mac512ke_map);
	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(15.6672_MHz_XTAL, MAC_H_TOTAL, 0, MAC_H_VIS, MAC_V_TOTAL, 0, MAC_V_VIS);
	m_screen->set_screen_update(FUNC(mac128_state::screen_update_mac));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_PWM(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // 2 x ls161
	voltage_regulator_device &vreg(VOLTAGE_REGULATOR(config, "vref"));
	vreg.add_route(0, DAC_TAG, 1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, DAC_TAG, -1.0, DAC_VREF_NEG_INPUT);

	/* devices */
	RTC3430042(config, m_rtc, 32.768_kHz_XTAL);
	LEGACY_IWM(config, m_iwm, 0).set_config(&mac_iwm_interface);
	sonydriv_floppy_image_device::legacy_2_drives_add(config, &mac_floppy_interface);

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(C3_7M, 0, C3_7M, 0);
	m_scc->out_int_callback().set(FUNC(mac128_state::set_scc_interrupt));

	VIA6522(config, m_via, 1000000);
	m_via->readpa_handler().set(FUNC(mac128_state::mac_via_in_a));
	m_via->readpb_handler().set(FUNC(mac128_state::mac_via_in_b));
	m_via->writepa_handler().set(FUNC(mac128_state::mac_via_out_a));
	m_via->writepb_handler().set(FUNC(mac128_state::mac_via_out_b));
	m_via->cb2_handler().set(m_mackbd, FUNC(mackbd_device::datain_w));
	m_via->irq_handler().set(FUNC(mac128_state::mac_via_irq));

	MACKBD(config, m_mackbd, 0);
	m_mackbd->dataout_handler().set(m_via, FUNC(via6522_device::write_cb2));
	m_mackbd->clkout_handler().set(m_via, FUNC(via6522_device::write_cb1)).invert();

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("512K");

	MACPDS(config, "macpds", "maincpu");
	MACPDS_SLOT(config, "pds", "macpds", mac_pds_cards, nullptr);

	// software list
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
}

void mac128_state::mac128k(machine_config &config)
{
	mac512ke(config);
	m_ram->set_default_size("128K");
}


void mac128_state::macplus(machine_config &config)
{
	mac512ke(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac128_state::macplus_map);

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));
	scsibus.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));

	NCR5380(config, m_ncr5380, C7M);
	m_ncr5380->set_scsi_port("scsi");
	m_ncr5380->irq_callback().set(FUNC(mac128_state::mac_scsi_irq));

	/* internal ram */
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("1M,2M,2560K,4M");
}

static INPUT_PORTS_START( macplus )
	PORT_START("MOUSE0") /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("MOUSE1") /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE2") /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************

  Machine driver(s)

***************************************************************************/

/*
ROM_START( mactw )
    ROM_REGION16_BE(0x100000, "bootrom", 0)
    ROM_LOAD( "rom4.3t_07-04-83.bin", 0x0000, 0x10000, CRC(d2c42f18) SHA1(f868c09ca70383a69751c37a5a3110a9597462a4) )
ROM_END
*/

ROM_START( mac128k )
	ROM_REGION16_BE(0x100000, "bootrom", 0)
	// Apple used at least 3 manufacturers for these ROMs, but they're always Apple part numbers 342-0220-A and 342-0221-A
	ROMX_LOAD("342-0220-a.u6d",  0x00000, 0x08000, CRC(198210ad) SHA1(2590ff4af5ac0361babdf0dc5da18e2eecad454a), ROM_SKIP(1) )
	ROMX_LOAD("342-0221-a.u8d",  0x00001, 0x08000, CRC(fd2665c2) SHA1(8507932a854bd28196a17785c8b1851cb53eaf64), ROM_SKIP(1) )
	/* Labels seen in the wild:
	VTi:
	"<VTi logo along side> // 416 VH 2605 // 23256-1020 // 342-0220-A // (C)APPLE 83 // KOREA-AE"
	"<VTi logo along side> // 416 VH 2826 // 23256-1023 // 342-0221-A // (C)APPLE 83 // KOREA-AE"
	Synertek:
	"<Synertek 'S' logo> 8416 G // C19728 // 342-0220-A // (C)APPLE 83"
	"<Synertek 'S' logo> 8410 G // C19729 // 342-0221-A // (C)APPLE 83"
	Hitachi:
	[can't find reference for rom-hi]
	"<Hitachi 'target' logo> 8413 // 3256 016 JAPAN // (C)APPLE 83 // 342-0221-A"

	References:
	http://www.vintagecomputer.net/apple/Macintosh/Macintosh_motherboard.jpg
	https://upload.wikimedia.org/wikipedia/commons/3/34/Macintosh-motherboard.jpg
	https://68kmla.org/forums/uploads/monthly_01_2016/post-2105-0-31195100-1452296677.jpg
	https://68kmla.org/forums/uploads/monthly_12_2014/post-2597-0-46269000-1419299800.jpg
	http://cdn.cultofmac.com/wp-content/uploads/2014/01/12A-128k-Motherboard.jpg
	*/
ROM_END

ROM_START( mac512k )
	ROM_REGION16_BE(0x100000, "bootrom", 0)
	ROMX_LOAD("342-0220-b.u6d",  0x00000, 0x08000, CRC(0dce9a3f) SHA1(101ca6570f5a273e400d1a8bc63e15ee0e94153e), ROM_SKIP(1) ) // "<VTi logo along side> 512 VH 6434 // 23256-1104 // 342-0220-B // (C) APPLE 84 // KOREA-A"
	ROMX_LOAD("342-0221-b.u8d",  0x00001, 0x08000, CRC(d51f376e) SHA1(575586109e876cffa4a4d472cb38771aa21b70cb), ROM_SKIP(1) ) // "<VTi logo along side> 512 VH 6709 // 23256-1105 // 342-0221-B // (C) APPLE 84 // KOREA-A"
	// reference: http://i.ebayimg.com/images/g/Uj8AAOSwvzRXy2tW/s-l1600.jpg
ROM_END

ROM_START( unitron )
	ROM_REGION16_BE(0x100000, "bootrom", 0)
	ROM_LOAD16_WORD( "unitron_512.rom", 0x00000, 0x10000, CRC(1eabd37f) SHA1(a3d3696c08feac6805effb7ee07b68c2bf1a8dd7) )
ROM_END

ROM_START( utrn1024 )
	ROM_REGION16_BE(0x100000, "bootrom", 0)
	// CRCs match the original "Lonely Hearts" version 1 Mac Plus ROM: 4d1eeee1
	ROMX_LOAD( "342-0341-a.u6d", 0x000000, 0x010000, CRC(5095fe39) SHA1(be780580033d914b5035d60b5ebbd66bd1d28a9b), ROM_SKIP(1) ) // not correct label
	ROMX_LOAD( "342-0342-a.u8d", 0x000001, 0x010000, CRC(fb766270) SHA1(679f529fbfc05f9cc98924c53457d2996dfcb1a7), ROM_SKIP(1) ) // not correct label
ROM_END

/*
 SCC init macplus.rom
 * Channel B and A init - reset command and vector differs
 * 09 <- 40/80 Master Interrup Control: channel B/A reset
 * 04 <- 4c Clocks: x16 mode, 2 stop bits, no parity
 * 02 <- 00 Interrupt vector (just for chan B)
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 0f <- 08 External/Status Control: DCD ints enabled
 * 00 <- 10 Reset External/status interrupts
 * 00 <- 10 Reset External/status interrupts
 * 01 <- 01 Enable External Interrupts
 * Above init first for channel B and then for channel A
 * 09 <- 0a Master Interrupt Control: No vector and Interrupts enabled!
 *
 SCC re-init of Channel B booting MacOS 7.0.0 (on Mac plus)
 * 09 <- 40 Master Interrup Control: channel B reset
 * 04 <- 20 x1 clock, Sync Modes Enable, SDLC Mode (01111110 Flag)
 * 0a <- e0 CRC preset to '1's, FM0 encoding scheme
 * 06 <- 00 Receiver SDLC ADR0-ADR7 bits
 * 07 <- 7e Receiver SDLC Flag character (0x7e as expected)
 * 0c <- 06 Low baudrate divider
 * 0d <- 00 Hi baudrate divider
 * 0e <- c0 Set FM Mode Command
 * 03 <- dd Rx 8 bit, Enter Hunt Mode, Rx CRC Enable, Enter SDLC Address Search Mode, Rx enable
 * 02 <- 00 Interrupt vector
 * 0f <- 08 External/Status Control: DCD interrupts enabled
 * 01 <- 09 Enable External Interrupts + Rx Int On First Character or Special Condition
 * 09 <- 0a Master Interrupt Control: No vector and Interrupts enabled!
 * 0b <- 70 Rx Clock is DPLL Output, Tx Clock is BRG output + TTL Clock on RTxC
 * 0e <- 21 Enter Search Mode Command + BRG enable + RTxC as BRG clock
 * 05 <- 60 Tx 8 bit, Tx disable, SDLC CRC Polynomial selected, Tx CRC disabled
 * 06 <- 01 Receiver SDLC ADR0-ADR7 bits updated
 * 0f <- 88 External/Status Control: Abort/Break and DCD interrupts enabled
*/

ROM_START( mac512ke ) // 512ke has been observed with any of the v3, v2 or v1 macplus romsets installed, and v1 romsets are more common here than in the plus, since the 512ke lacks scsi, which is the cause of the major bug fixed between v1 and v2, hence 512ke is unaffected and was a good way for apple to use up the buggy roms rather than destroying them.
	ROM_REGION16_BE(0x100000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "v3", "Loud Harmonicas")
	ROMX_LOAD( "342-0341-c.u6d", 0x000000, 0x010000, CRC(f69697e6) SHA1(41317614ac71eb94941e9952f6ea37407e21ffff), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "342-0342-b.u8d", 0x000001, 0x010000, CRC(49f25913) SHA1(72f658c02bae265e8845899582575fb7c784ee87), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_FILL(0x20000, 0x2, 0xff)    // ROM checks for same contents at 20000 and 40000 to determine if SCSI is present
	ROM_FILL(0x40000, 0x2, 0xaa)
	ROM_SYSTEM_BIOS(1, "v2", "Lonely Heifers")
	ROMX_LOAD( "342-0341-b.u6d", 0x000000, 0x010000, CRC(65341487) SHA1(bf43fa4f5a3dcbbac20f1fe1deedee0895454379), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "342-0342-a.u8d", 0x000001, 0x010000, CRC(fb766270) SHA1(679f529fbfc05f9cc98924c53457d2996dfcb1a7), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_FILL(0x20000, 0x2, 0xff)
	ROM_FILL(0x40000, 0x2, 0xaa)
	ROM_SYSTEM_BIOS(2, "v1", "Lonely Hearts")
	ROMX_LOAD( "342-0341-a.u6d", 0x000000, 0x010000, CRC(5095fe39) SHA1(be780580033d914b5035d60b5ebbd66bd1d28a9b), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "342-0342-a.u8d", 0x000001, 0x010000, CRC(fb766270) SHA1(679f529fbfc05f9cc98924c53457d2996dfcb1a7), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_FILL(0x20000, 0x2, 0xff)
	ROM_FILL(0x40000, 0x2, 0xaa)
	/* from Technical note HW11 (https://www.fenestrated.net/mirrors/Apple%20Technotes%20(As%20of%202002)/hw/hw_11.html)
	1st version (Lonely Hearts, checksum 4D 1E EE E1)
	Bug in the SCSI driver; won't boot if external drive is turned off. We only produced about
	one and a half months worth of these.

	2nd version (Lonely Heifers, checksum 4D 1E EA E1):
	Fixed boot bug. This version is the vast majority of beige Macintosh Pluses.

	3rd version (Loud Harmonicas, checksum 4D 1F 81 72):
	Fixed bug for drives that return Unit Attention on power up or reset. Basically took the
	SCSI bus Reset command out of the boot sequence loop, so it will only reset once
	during boot sequence.
	*/
	/* Labels seen in the wild:
	v3/4d1f8172:
	    'ROM-HI' @ U6D:
	        "VLSI // 740 SA 1262 // 23512-1054 // 342-0341-C // (C)APPLE '83-'86 // KOREA A"
	        "342-0341-C // (C)APPLE 85,86 // (M)AMI 8849MBL // PHILLIPINES"
	    'ROM-LO' @ U8D:
	        "VLSI // 740 SA 1342 // 23512-1055 // 342-0342-B // (C)APPLE '83-'86 // KOREA A"
	        "<VLSI logo>VLSI // 8905AV 0 AS759 // 23512-1055 // 342-0342-B // (C)APPLE '85-'86"
	v2/4d1eeae1:
	    'ROM-HI' @ U6D:
	        "VTI // 624 V0 8636 // 23512-1010 // 342-0341-B // (C)APPLE '85 // MEXICO R"
	    'ROM-LO' @ U8D:
	        "VTI // 622 V0 B637 // 23512-1007 // 342-0342-A // (C)APPLE '83-'85 // KOREA A"
	v1/4d1eeee1:
	    'ROM-HI' @ U6D:
	        GUESSED, since this ROM is very rare: "VTI // 62? V0 86?? // 23512-1008 // 342-0341-A // (C)APPLE '83-'85 // KOREA A"
	    'ROM-LO' @ U8D is same as v2/4d1eeae1 'ROM-LO' @ U8D
	*/
ROM_END

ROM_START( macplus ) // same notes as above apply here as well
	ROM_REGION16_BE(0x100000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "v3", "Loud Harmonicas")
	ROMX_LOAD( "342-0341-c.u6d", 0x000000, 0x010000, CRC(f69697e6) SHA1(41317614ac71eb94941e9952f6ea37407e21ffff), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "342-0342-b.u8d", 0x000001, 0x010000, CRC(49f25913) SHA1(72f658c02bae265e8845899582575fb7c784ee87), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_FILL(0x20000, 0x2, 0xff)    // ROM checks for same contents at 20000 and 40000 to determine if SCSI is present
	ROM_FILL(0x40000, 0x2, 0xaa)
	ROM_SYSTEM_BIOS(1, "v2", "Lonely Heifers")
	ROMX_LOAD( "342-0341-b.u6d", 0x000000, 0x010000, CRC(65341487) SHA1(bf43fa4f5a3dcbbac20f1fe1deedee0895454379), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "342-0342-a.u8d", 0x000001, 0x010000, CRC(fb766270) SHA1(679f529fbfc05f9cc98924c53457d2996dfcb1a7), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_FILL(0x20000, 0x2, 0xff)
	ROM_FILL(0x40000, 0x2, 0xaa)
	ROM_SYSTEM_BIOS(2, "v1", "Lonely Hearts")
	ROMX_LOAD( "342-0341-a.u6d", 0x000000, 0x010000, CRC(5095fe39) SHA1(be780580033d914b5035d60b5ebbd66bd1d28a9b), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "342-0342-a.u8d", 0x000001, 0x010000, CRC(fb766270) SHA1(679f529fbfc05f9cc98924c53457d2996dfcb1a7), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_FILL(0x20000, 0x2, 0xff)
	ROM_FILL(0x40000, 0x2, 0xaa)
	ROM_SYSTEM_BIOS(3, "romdisk", "mac68k.info self-boot (1/1/2015)")
	ROMX_LOAD( "modplus-harp2.bin", 0x000000, 0x028000, CRC(ba56078d) SHA1(debdf328ac73e1662d274a044d8750224f47edef), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "romdisk2", "bigmessofwires.com ROMinator (2/25/2015)")
	ROMX_LOAD( "rominator-20150225-lo.bin", 0x000001, 0x080000, CRC(62cf2a0b) SHA1(f78ebb0919dd9e094bef7952b853b70e66d05e01), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "rominator-20150225-hi.bin", 0x000000, 0x080000, CRC(a28ba8ec) SHA1(9ddcf500727955c60db0ff24b5ca2458f53fd89a), ROM_SKIP(1) | ROM_BIOS(4) )
ROM_END

/*    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS         INIT              COMPANY              FULLNAME */
//COMP( 1983, mactw,    0,       0,      mac128k,  macplus, mac128_state, init_mac128k512k, "Apple Computer",    "Macintosh (4.3T Prototype)",  MACHINE_NOT_WORKING )
COMP( 1984, mac128k,  0,       0,      mac128k,  macplus, mac128_state, init_mac128k512k, "Apple Computer",    "Macintosh 128k",  MACHINE_NOT_WORKING )
COMP( 1984, mac512k,  mac128k, 0,      mac512ke, macplus, mac128_state, init_mac128k512k, "Apple Computer",    "Macintosh 512k",  MACHINE_NOT_WORKING )
COMP( 1986, mac512ke, macplus, 0,      mac512ke, macplus, mac128_state, init_mac512ke,    "Apple Computer",    "Macintosh 512ke", MACHINE_NOT_WORKING )
COMP( 1985, unitron,  macplus, 0,      mac512ke, macplus, mac128_state, init_mac512ke,    "bootleg (Unitron)", "Mac 512",  MACHINE_NOT_WORKING )
COMP( 1986, macplus,  0,       0,      macplus,  macplus, mac128_state, init_macplus,     "Apple Computer",    "Macintosh Plus",  MACHINE_NOT_WORKING )
COMP( 1985, utrn1024, macplus, 0,      macplus,  macplus, mac128_state, init_macplus,     "bootleg (Unitron)", "Unitron 1024",  MACHINE_NOT_WORKING )
