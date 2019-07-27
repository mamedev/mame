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
        0x800000 - 0x9fffff     Zilog 8530 SCC (Serial Control Chip) Read
        0xa00000 - 0xbfffff     Zilog 8530 SCC (Serial Control Chip) Write
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
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "machine/6522via.h"
#include "machine/z80scc.h"
#include "machine/ncr5380.h"
#include "machine/applefdc.h"
#include "machine/swim.h"
#include "machine/sonydriv.h"
#include "machine/mackbd.h"
#include "machine/macrtc.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "formats/ap_dsk35.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/scsicd.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#define MAC_SCREEN_NAME "screen"
#define MAC_539X_1_TAG "539x_1"
#define MAC_539X_2_TAG "539x_2"
#define MACKBD_TAG "mackbd"
#define DAC_TAG "macdac"
#define SCC_TAG "scc"

#define C7M (15.6672_MHz_XTAL / 2)
#define C3_7M (15.6672_MHz_XTAL / 4).value()

// uncomment to run i8021 keyboard in original Mac/512(e)/Plus
//#define MAC_USE_EMULATED_KBD (1)

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
		m_mackbd(*this, MACKBD_TAG),
		m_rtc(*this,"rtc"),
		m_mouse0(*this, "MOUSE0"),
		m_mouse1(*this, "MOUSE1"),
		m_mouse2(*this, "MOUSE2"),
		m_key_port(*this, "KEY%u", 0),
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
	optional_device<mackbd_device> m_mackbd;
	optional_device<rtc3430042_device> m_rtc;

	required_ioport m_mouse0, m_mouse1, m_mouse2;
	optional_ioport_array<7> m_key_port;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	mac128model_t m_model;

	uint32_t m_overlay;

	int m_irq_count, m_ca1_data, m_ca2_data;

#ifndef MAC_USE_EMULATED_KBD
	/* used to store the reply to most keyboard commands */
	int m_keyboard_reply;

	/* Keyboard communication in progress? */
	int m_kbd_comm;
	int m_kbd_receive;
	/* timer which is used to time out inquiry */
	emu_timer *m_inquiry_timeout;

	int m_kbd_shift_reg;
	int m_kbd_shift_count;

	/* keycode buffer (used for keypad/arrow key transition) */
	int m_keycode_buf[2];
	int m_keycode_buf_index;

	int m_cb2_in;
#endif

	/* keyboard matrix to detect transition - macadb needs to stop relying on this */
	int m_key_matrix[7];

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

	DECLARE_READ16_MEMBER ( ram_r );
	DECLARE_WRITE16_MEMBER ( ram_w );
	DECLARE_READ16_MEMBER ( ram_600000_r );
	DECLARE_WRITE16_MEMBER ( ram_600000_w );
	DECLARE_READ16_MEMBER ( mac_via_r );
	DECLARE_WRITE16_MEMBER ( mac_via_w );
	DECLARE_READ16_MEMBER ( mac_autovector_r );
	DECLARE_WRITE16_MEMBER ( mac_autovector_w );
	DECLARE_READ16_MEMBER ( mac_iwm_r );
	DECLARE_WRITE16_MEMBER ( mac_iwm_w );
	DECLARE_READ16_MEMBER ( macplus_scsi_r );
	DECLARE_WRITE16_MEMBER ( macplus_scsi_w );
	DECLARE_WRITE_LINE_MEMBER(mac_scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(set_scc_interrupt);

	TIMER_CALLBACK_MEMBER(mac_scanline);
	TIMER_CALLBACK_MEMBER(mac_hblank);
	DECLARE_VIDEO_START(mac);
	uint32_t screen_update_mac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
#ifndef MAC_USE_EMULATED_KBD
	TIMER_CALLBACK_MEMBER(kbd_clock);
	TIMER_CALLBACK_MEMBER(inquiry_timeout_func);
#else
	DECLARE_WRITE_LINE_MEMBER(mac_kbd_clk_in);
#endif
	DECLARE_WRITE_LINE_MEMBER(mac_via_out_cb2);
	DECLARE_READ8_MEMBER(mac_via_in_a);
	DECLARE_READ8_MEMBER(mac_via_in_b);
	DECLARE_WRITE8_MEMBER(mac_via_out_a);
	DECLARE_WRITE8_MEMBER(mac_via_out_b);
	void mac128_state_load();
	DECLARE_WRITE_LINE_MEMBER(mac_via_irq);
	int scan_keyboard();
	void keyboard_init();
	void kbd_shift_out(int data);
	void keyboard_receive(int val);
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
#ifndef MAC_USE_EMULATED_KBD
	m_keyboard_reply = 0;
	m_kbd_comm = 0;
	m_kbd_receive = 0;
	m_kbd_shift_reg = 0;
	m_kbd_shift_count = 0;
#endif
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

READ16_MEMBER(mac128_state::ram_r)
{
	if (m_overlay)
	{
		return m_rom_ptr[offset];
	}

	return m_ram_ptr[offset & m_ram_mask];
}

WRITE16_MEMBER(mac128_state::ram_w)
{
	if (!m_overlay)
	{
		COMBINE_DATA(&m_ram_ptr[offset & m_ram_mask]);
	}
}

READ16_MEMBER(mac128_state::ram_600000_r)
{
	return m_ram_ptr[offset & m_ram_mask];
}

WRITE16_MEMBER(mac128_state::ram_600000_w)
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
#ifndef MAC_USE_EMULATED_KBD
	/* handle keyboard */
	if (m_kbd_comm == true && m_kbd_receive == false)
	{
		int keycode = scan_keyboard();

		if (keycode != 0x7B)
		{
			/* if key pressed, send the code */

			logerror("keyboard enquiry successful, keycode %X\n", keycode);

			m_inquiry_timeout->reset();
			kbd_shift_out(keycode);
		}
	}
#endif

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

READ16_MEMBER ( mac128_state::macplus_scsi_r )
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	if ((reg == 6) && (offset == 0x130))
	{
		reg = R5380_CURDATA_DTACK;
	}

	return m_ncr5380->ncr5380_read_reg(reg)<<8;
}

WRITE16_MEMBER ( mac128_state::macplus_scsi_w )
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

READ16_MEMBER ( mac128_state::mac_iwm_r )
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

WRITE16_MEMBER ( mac128_state::mac_iwm_w )
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

READ16_MEMBER ( mac128_state::mac_via_r )
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

WRITE16_MEMBER ( mac128_state::mac_via_w )
{
	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_w: offset=0x%02x data=0x%08x\n", offset, data);

	m_via->write(offset, (data >> 8) & 0xff);

	m_maincpu->adjust_icount(m_via_cycles);
}

WRITE16_MEMBER ( mac128_state::mac_autovector_w )
{
	if (LOG_GENERAL)
		logerror("mac_autovector_w: offset=0x%08x data=0x%04x\n", offset, data);

	/* This should throw an exception */

	/* Not yet implemented */
}

READ16_MEMBER ( mac128_state::mac_autovector_r )
{
	if (LOG_GENERAL)
		logerror("mac_autovector_r: offset=0x%08x\n", offset);

	/* This should throw an exception */

	/* Not yet implemented */
	return 0;
}

READ8_MEMBER(mac128_state::mac_via_in_a)
{
	return 0x80;
}

READ8_MEMBER(mac128_state::mac_via_in_b)
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

WRITE8_MEMBER(mac128_state::mac_via_out_a)
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

WRITE8_MEMBER(mac128_state::mac_via_out_b)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);

	m_snd_enable = ((data & 0x80) == 0) ? true : false;
	update_volume();
	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

/*
    R Nabet 000531 : added keyboard code
*/

/* *************************************************************************
 * non-ADB keyboard support
 *
 * The keyboard uses a i8021 (?) microcontroller.
 * It uses a bidirectional synchonous serial line, connected to the VIA (SR feature)
 *
 * Our emulation is more a hack than anything else - the keyboard controller is
 * not emulated, instead we interpret keyboard commands directly.  I made
 * many guesses, which may be wrong
 *
 * todo :
 * * find the correct model number for the Mac Plus keyboard ?
 * * emulate original Macintosh keyboards (2 layouts : US and international)
 *
 * references :
 * * IM III-29 through III-32 and III-39 through III-42
 * * IM IV-250
 * *************************************************************************/

/*
    scan_keyboard()

    scan the keyboard, and returns key transition code (or NULL ($7B) if none)
*/
#ifndef MAC_USE_EMULATED_KBD
int mac128_state::scan_keyboard()
{
	int i, j;
	int keybuf = 0;
	int keycode;

	if (m_keycode_buf_index)
	{
		return m_keycode_buf[--m_keycode_buf_index];
	}

	for (i=0; i<7; i++)
	{
		keybuf = m_key_port[i]->read();

		if (keybuf != m_key_matrix[i])
		{
			/* if state has changed, find first bit which has changed */
			if (LOG_KEYBOARD)
				logerror("keyboard state changed, %d %X\n", i, keybuf);

			for (j=0; j<16; j++)
			{
				if (((keybuf ^ m_key_matrix[i]) >> j) & 1)
				{
					/* update m_key_matrix */
					m_key_matrix[i] = (m_key_matrix[i] & ~ (1 << j)) | (keybuf & (1 << j));

					if (i < 4)
					{
						/* create key code */
						keycode = (i << 5) | (j << 1) | 0x01;
						if (! (keybuf & (1 << j)))
						{
							/* key up */
							keycode |= 0x80;
						}
						return keycode;
					}
					else if (i < 6)
					{
						/* create key code */
						keycode = ((i & 3) << 5) | (j << 1) | 0x01;

						if ((keycode == 0x05) || (keycode == 0x0d) || (keycode == 0x11) || (keycode == 0x1b))
						{
							/* these keys cause shift to be pressed (for compatibility with mac 128/512) */
							if (keybuf & (1 << j))
							{
								/* key down */
								if (! (m_key_matrix[3] & 0x0100))
								{
									/* shift key is really up */
									m_keycode_buf[0] = keycode;
									m_keycode_buf[1] = 0x79;
									m_keycode_buf_index = 2;
									return 0x71;    /* "presses" shift down */
								}
							}
							else
							{   /* key up */
								if (! (m_key_matrix[3] & 0x0100))
								{
									/* shift key is really up */
									m_keycode_buf[0] = keycode | 0x80;
									m_keycode_buf[1] = 0x79;
									m_keycode_buf_index = 2;
									return 0xF1;    /* "releases" shift */
								}
							}
						}

						if (! (keybuf & (1 << j)))
						{
							/* key up */
							keycode |= 0x80;
						}
						m_keycode_buf[0] = keycode;
						m_keycode_buf_index = 1;
						return 0x79;
					}
					else /* i == 6 */
					{
						/* create key code */
						keycode = (j << 1) | 0x01;
						if (! (keybuf & (1 << j)))
						{
							/* key up */
							keycode |= 0x80;
						}
						m_keycode_buf[0] = keycode;
						m_keycode_buf_index = 1;
						return 0x79;
					}
				}
			}
		}
	}

	return 0x7B;    /* return NULL */
}

/*
    power-up init
*/
void mac128_state::keyboard_init()
{
	int i;

	/* init flag */
	m_kbd_comm = FALSE;
	m_kbd_receive = FALSE;
	m_kbd_shift_reg=0;
	m_kbd_shift_count=0;

	/* clear key matrix */
	for (i=0; i<7; i++)
	{
		m_key_matrix[i] = 0;
	}

	/* purge transmission buffer */
	m_keycode_buf_index = 0;
}
#endif

/******************* Keyboard <-> VIA communication ***********************/

#ifdef MAC_USE_EMULATED_KBD

WRITE_LINE_MEMBER(mac128_state::mac_kbd_clk_in)
{
	printf("CLK: %d\n", state^1);
	m_via->write_cb1(state ? 0 : 1);
}

WRITE_LINE_MEMBER(mac128_state::mac_via_out_cb2)
{
	printf("Sending %d to kbd (PC=%x)\n", state, m_maincpu->pc());
	m_mackbd->data_w(state ? ASSERT_LINE : CLEAR_LINE);
}

#else   // keyboard HLE

TIMER_CALLBACK_MEMBER(mac128_state::kbd_clock)
{
	int i;

	if (m_kbd_comm == TRUE)
	{
		for (i=0; i<9; i++)
		{
			/* Put data on CB2 if we are sending*/
			if (m_kbd_receive == FALSE)
			{
				m_via->write_cb2(m_kbd_shift_reg&0x80?1:0);
				if (i > 0)
				{
					m_kbd_shift_reg <<= 1;
				}
			}

			m_via->write_cb1(0);
			m_via->write_cb1(1);

			if (m_kbd_receive == TRUE)
			{
				if (i < 8)
				{
					m_kbd_shift_reg <<= 1;
					m_kbd_shift_reg |= (m_cb2_in & 1);
				}
			}
		}
		if (m_kbd_receive == TRUE)
		{
			m_kbd_receive = FALSE;
			/* Process the command received from mac */
			//printf("Mac sent %02x\n", m_kbd_shift_reg & 0xff);
			keyboard_receive(m_kbd_shift_reg & 0xff);
		}
		else
		{
			/* Communication is over */
			m_kbd_comm = FALSE;
		}
	}
}

void mac128_state::kbd_shift_out(int data)
{
	if (m_kbd_comm == TRUE)
	{
		//printf("%02x to Mac\n", data);
		m_kbd_shift_reg = data;
		machine().scheduler().timer_set(attotime::from_msec(1), timer_expired_delegate(FUNC(mac128_state::kbd_clock),this));
	}
}

WRITE_LINE_MEMBER(mac128_state::mac_via_out_cb2)
{
	//printf("CB2 = %d, kbd_comm = %d\n", state, m_kbd_comm);
	if (m_kbd_comm == FALSE && state == 0)
	{
		/* Mac pulls CB2 down to initiate communication */
		m_kbd_comm = TRUE;
		m_kbd_receive = TRUE;
		machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(mac128_state::kbd_clock),this));
	}
	if (m_kbd_comm == TRUE && m_kbd_receive == TRUE)
	{
		/* Shift in what mac is sending */
		m_cb2_in = state;
	}
}

/*
    called when inquiry times out (1/4s)
*/
TIMER_CALLBACK_MEMBER(mac128_state::inquiry_timeout_func)
{
	if (LOG_KEYBOARD)
		logerror("keyboard enquiry timeout\n");
	kbd_shift_out(0x7B); /* always send NULL */
}

/*
    called when a command is received from the mac
*/
void mac128_state::keyboard_receive(int val)
{
	//printf("Mac sent %02x\n", val);
	switch (val)
	{
	case 0x10:
		/* inquiry - returns key transition code, or NULL ($7B) if time out (1/4s) */
		if (LOG_KEYBOARD)
			logerror("keyboard command : inquiry\n");

		m_inquiry_timeout->adjust(
			attotime(0, DOUBLE_TO_ATTOSECONDS(0.25)), 0);
		break;

	case 0x14:
		/* instant - returns key transition code, or NULL ($7B) */
		if (LOG_KEYBOARD)
			logerror("keyboard command : instant\n");

		kbd_shift_out(scan_keyboard());
		break;

	case 0x16:
		/* model number - resets keyboard, return model number */
		if (LOG_KEYBOARD)
			logerror("keyboard command : model number\n");

		{   /* reset */
			int i;

			/* clear key matrix */
			for (i=0; i<7; i++)
			{
				m_key_matrix[i] = 0;
			}

			/* purge transmission buffer */
			m_keycode_buf_index = 0;
		}

		/* format : 1 if another device (-> keypad ?) connected | next device (-> keypad ?) number 1-8
		                    | keyboard model number 1-8 | 1  */
		/* keyboards :
		    3 : mac 512k, US and international layout ? Mac plus ???
		    other values : Apple II keyboards ?
		*/
		/* keypads :
		    ??? : standard keypad (always available on Mac Plus) ???
		*/
		kbd_shift_out(0x17);   /* probably wrong */
		break;

	case 0x36:
		/* test - resets keyboard, return ACK ($7D) or NAK ($77) */
		if (LOG_KEYBOARD)
			logerror("keyboard command : test\n");

		kbd_shift_out(0x7D);   /* ACK */
		break;

	default:
		if (LOG_KEYBOARD)
			logerror("unknown keyboard command 0x%X\n", val);

		kbd_shift_out(0);
		break;
	}
}
#endif

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

	/* setup keyboard */
#ifndef MAC_USE_EMULATED_KBD
	keyboard_init();
	m_inquiry_timeout = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac128_state::inquiry_timeout_func),this));
#else
	/* clear key matrix for macadb */
	for (int i=0; i<7; i++)
	{
		m_key_matrix[i] = 0;
	}
#endif

	/* save state stuff */
	machine().save().register_postload(save_prepost_delegate(FUNC(mac128_state::mac128_state_load), this));
}

void mac128_state::mac128_state_load()
{
}

VIDEO_START_MEMBER(mac128_state,mac)
{
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

void mac128_state::mac512ke(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, C7M);        /* 7.8336 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mac128_state::mac512ke_map);
	config.m_minimum_quantum = attotime::from_hz(60);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(15.6672_MHz_XTAL, MAC_H_TOTAL, 0, MAC_H_VIS, MAC_V_TOTAL, 0, MAC_V_VIS);
	m_screen->set_screen_update(FUNC(mac128_state::screen_update_mac));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	MCFG_VIDEO_START_OVERRIDE(mac128_state,mac)

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_PWM(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // 2 x ls161
	voltage_regulator_device &vreg(VOLTAGE_REGULATOR(config, "vref"));
	vreg.add_route(0, DAC_TAG, 1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, DAC_TAG, -1.0, DAC_VREF_NEG_INPUT);

	/* devices */
	RTC3430042(config, m_rtc, 32.768_kHz_XTAL);
	IWM(config, m_iwm, 0).set_config(&mac_iwm_interface);
	sonydriv_floppy_image_device::legacy_2_drives_add(config, &mac_floppy_interface);

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(C3_7M, 0, C3_7M, 0);
	m_scc->out_int_callback().set(FUNC(mac128_state::set_scc_interrupt));

	VIA6522(config, m_via, 1000000);
	m_via->readpa_handler().set(FUNC(mac128_state::mac_via_in_a));
	m_via->readpb_handler().set(FUNC(mac128_state::mac_via_in_b));
	m_via->writepa_handler().set(FUNC(mac128_state::mac_via_out_a));
	m_via->writepb_handler().set(FUNC(mac128_state::mac_via_out_b));
	m_via->cb2_handler().set(FUNC(mac128_state::mac_via_out_cb2));
	m_via->irq_handler().set(FUNC(mac128_state::mac_via_irq));

#ifdef MAC_USE_EMULATED_KBD
	mackbd_device &mackbd(MACKBD(config, MACKBD_TAG, 0));
	mackbd.dataout_handler().set(m_via, FUNC(via6522_device::write_cb2));
	mackbd.clkout_handler().set(FUNC(mac128_state::mac_kbd_clk_in));
#else
	MACKBD(config, MACKBD_TAG, 0);
#endif

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("512K");

	// software list
	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
	SOFTWARE_LIST(config, "hdd_list").set_type("mac_hdd", SOFTWARE_LIST_ORIGINAL_SYSTEM);
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

	/* R Nabet 000531 : pseudo-input ports with keyboard layout */
	/* we only define US layout for keyboard - international layout is different! */
	/* note : 16 bits at most per port! */

	/* main keyboard pad */

	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)    /* extra key on ISO : */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("KEY3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)    /* keyboard Enter : */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)    /* escape: */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)    /* ??? */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Command") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Option") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)    /* Control: */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)    /* keypad pseudo-keycode */
	PORT_BIT(0xE000, IP_ACTIVE_HIGH, IPT_UNUSED)    /* ??? */

	/* keypad */
	PORT_START("KEY4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)          PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x0038, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(/*KEYCODE_NUMLOCK*/KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(/*CODE_OTHER*/KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT(0x0E00, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0xE000, IP_ACTIVE_HIGH, IPT_UNUSED)

	/* Arrow keys */
	PORT_START("KEY6")
	PORT_BIT(0x0003, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Arrow") PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x0038, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Arrow") PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down Arrow") PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x1E00, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up Arrow") PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0xC000, IP_ACTIVE_HIGH, IPT_UNUSED)
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
