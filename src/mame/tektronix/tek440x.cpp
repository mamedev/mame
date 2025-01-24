// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR
/***************************************************************************

    Tektronix 440x "AI Workstations"

    skeleton by R. Belmont

    Hardware overview:
        * 68010 (4404) or 68020 (4405) with custom MMU
        * Intelligent floppy subsystem with 6502 driving a uPD765 controller
        * NS32081 FPU
        * 6551 debug console AICA
        * SN76496 PSG for sound
        * MC146818 RTC
        * MC68681 DUART / timer (3.6864 MHz clock) (serial channel A = keyboard, channel B = RS-232 port)
        * AM9513 timer (source of timer IRQ)
        * NCR5385 SCSI controller
				* 8255 Centronics printer interface
				
        Video is a 640x480 1bpp window on a 1024x1024 VRAM area; smooth panning around that area
        is possible as is flat-out changing the scanout address.

    IRQ levels:
        7 = Debug (NMI)
        6 = VBL
        5 = UART
        4 = Spare (exp slots)
        3 = SCSI
        2 = DMA
        1 = Timer	(and Printer)
        0 = Unused

    MMU info:
        Map control register (location 0x780000): bit 5 = Wenable, bit 4 = VMenable, bits3-0 process ID

        Map entries:
            bit 15 = dirty
            bit 14 = write enable
            bit 13-11 = process ID
            bits 10-0 = address bits 22-12 in the final address

***************************************************************************/

#include "emu.h"

#include "tek410x_kbd.h"
#include "tek_msu_fdc.h"

#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68010.h"
#include "machine/am9513.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "machine/mos6551.h"    // debug tty
#include "machine/ncr5385.h"
#include "machine/ns32081.h"
#include "machine/i8255.h"
#include "machine/nscsi_bus.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"

// mapcntl bits
#define MAP_VM_ENABLE 4
#define MAP_SYS_WR_ENABLE 5
#define MAP_BLOCK_ACCESS 6
#define MAP_CPU_WR 7

#define OFF8_TO_OFF16(A)	((A)>>1)
#define OFF16_TO_OFF8(A)	((A)<<1)

#define MAXRAM 0x200000	// +1MB
//#define MAXRAM 0x400000	// +3MB (which was never a Thing)

namespace {

class tek440x_state : public driver_device
{
public:
	tek440x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vm(*this, "vm"),
		m_duart(*this, "duart"),
		m_keyboard(*this, "keyboard"),
		m_snsnd(*this, "snsnd"),
		m_timer(*this, "timer"),
		m_rtc(*this, "rtc"),
		m_scsi(*this, "scsi:7:ncr5385"),
		m_vint(*this, "vint"),
		m_screen(*this, "screen"),
		m_acia(*this, "acia"),
		m_printer(*this, "printer"),
		m_prom(*this, "maincpu"),			// FIXME why is the bootrom called 'maincpu'?
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_map(*this, "map", 0x1000, ENDIANNESS_BIG),
		m_map_view(*this, "map"),
		m_mousex(*this, "mousex"),
		m_mousey(*this, "mousey"),
		m_mousebtn(*this, "mousebtn"),
		m_boot(false),
		m_map_control(0),
		m_kb_rdata(true),
		m_kb_tdata(true),
		m_kb_rclamp(false),
		m_kb_loop(false)
	{ }

	void tek4404(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 memory_r(offs_t offset, u16 mem_mask);
	void memory_w(offs_t offset, u16 data, u16 mem_mask);
	u16 map_r(offs_t offset);
	void map_w(offs_t offset, u16 data, u16 mem_mask);
	u8 mapcntl_r();
	void mapcntl_w(u8 data);
	void sound_w(u8 data);
	u8 diag_r();
	void diag_w(u8 data);
	u8 mouse_r(offs_t offset);
	void mouse_w(u8 data);
	void led_w(u8 data);
	u16 videoaddr_r(offs_t offset);
	void videoaddr_w(offs_t offset, u16 data);
	u8 videocntl_r();
	void videocntl_w(u8 data);

	// fake output of serial
	void write_txd(int state);

	// need to handle bit 8 reset
	void irq1_w(int state);
	u16 timer_r(offs_t offset);
	void timer_w(offs_t offset, u16 data);

	// need to handle loopback mode
	u8 duart_r(offs_t offset);
	void duart_w(offs_t offset, u8 data);

	
	void kb_rdata_w(int state);
	void kb_tdata_w(int state);
	void kb_rclamp_w(int state);

	void logical_map(address_map &map) ATTR_COLD;
	void physical_map(address_map &map) ATTR_COLD;

	required_device<m68010_device> m_maincpu;
	required_device<address_map_bank_device> m_vm;
	required_device<mc68681_device> m_duart;
	required_device<tek410x_keyboard_device> m_keyboard;
	required_device<sn76496_device> m_snsnd;
	required_device<am9513_device> m_timer;
	required_device<mc146818_device> m_rtc;
	required_device<ncr5385_device> m_scsi;
	required_device<input_merger_all_high_device> m_vint;
	required_device<screen_device> m_screen;
	required_device<mos6551_device> m_acia;
	required_device<i8255_device> m_printer;

	required_region_ptr<u16> m_prom;
	required_shared_ptr<u16> m_mainram;
	required_shared_ptr<u16> m_vram;
	memory_share_creator<u16> m_map;
	memory_view m_map_view;

	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mousebtn;
	
	int m_u244latch;
	
	bool m_boot;
	u8 m_map_control;
	bool m_kb_rdata;
	bool m_kb_tdata;
	bool m_kb_rclamp;
	bool m_kb_loop;
	u8 m_leds;
	u8 m_videoaddr[4];
	u8 m_videocntl;
	u8 m_diag;
	u8 m_mouse;
};

/*************************************
 *
 *  Machine start
 *
 *************************************/

void tek440x_state::machine_start()
{
	save_item(NAME(m_boot));
	save_item(NAME(m_map_control));
	save_item(NAME(m_kb_rdata));
	save_item(NAME(m_kb_tdata));
	save_item(NAME(m_kb_rclamp));
	save_item(NAME(m_kb_loop));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void tek440x_state::machine_reset()
{
	m_boot = true;
	diag_w(0);
	m_u244latch = 0;
	m_keyboard->kdo_w(1);
	mapcntl_w(0);
	m_vint->in_w<1>(0);
}


/*************************************
 *
 *  Video refresh
 *
 *************************************/

u32 tek440x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 480; y++)
	{
	
		//  FIXME: add in videopan
		u16 *const line = &bitmap.pix(y);
		u16 const *video_ram = &m_vram[y * 64];

		for (int x = 0; x < 640; x += 16)
		{
			u16 const word = *(video_ram++);
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = BIT(word, 15 - b);
			}
		}
	}

	return 0;
}



/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/
u16 tek440x_state::memory_r(offs_t offset, u16 mem_mask)
{
	if (m_boot)
		return m_prom[offset & 0x3fff];

	const offs_t offset0 = offset;
	if ((m_maincpu->get_fc() & 4) == 0)			// only in User mode
	if (BIT(m_map_control, MAP_VM_ENABLE))
	{
	
		// is !cpuWr
		m_map_control &= ~(1 << MAP_CPU_WR);

		// selftest expects fail if page.pid != map_control.pid
		if (BIT(m_map[offset >> 11], 11, 3) != (m_map_control & 7))
		{

			m_map_control |= (1 << MAP_BLOCK_ACCESS);
			LOG("memory_r: m_map_control(%02x)\n", m_map_control);

			LOG("memory_r: bus error: PID(%d) wrong %08x fc(%d)\n", BIT(m_map[offset >> 11], 11, 3), OFF16_TO_OFF8(offset), m_maincpu->get_fc());
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
			m_maincpu->set_buserror_details(offset0 << 1, 0, m_maincpu->get_fc());

			mem_mask = 0;
		}
		
		offset = BIT(offset, 0, 11) | BIT(m_map[offset >> 11], 0, 11) << 11;
	}

	// NB byte memory limit, offset is *word* offset
	if (offset < OFF8_TO_OFF16(0x600000) && offset >= OFF8_TO_OFF16(MAXRAM) && !machine().side_effects_disabled())
	{
		LOG("memory_r: bus error: %08x fc(%d)\n",  OFF16_TO_OFF8(offset), m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_maincpu->set_buserror_details(offset0 << 1, 1, m_maincpu->get_fc());
	}

	return m_vm->read16(offset, mem_mask);
}

void tek440x_state::memory_w(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t offset0 = offset;
	if ((m_maincpu->get_fc() & 4) == 0)
	if (BIT(m_map_control, MAP_VM_ENABLE))
	{
		//LOG("memory_w: m_map(0x%04x)\n", m_map[offset >> 11]);
	
		// is cpuWr
		m_map_control |= (1 << MAP_CPU_WR);
				
		// matching pid?
		if (BIT(m_map[offset >> 11], 11, 3) != (m_map_control & 7))
		{
			m_map_control &= ~(1 << MAP_BLOCK_ACCESS);
			LOG("memory_w: m_map_control(%02x)\n", m_map_control);

			LOG("memory_w: bus error: PID(%d) wrong %08x fc(%d)\n", BIT(m_map[offset >> 11], 11, 3), OFF16_TO_OFF8(offset), m_maincpu->get_fc());
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
			m_maincpu->set_buserror_details(offset0 << 1, 0, m_maincpu->get_fc());
			
			mem_mask = 0;
		}
		else
		{
			m_map_control |= (1 << MAP_BLOCK_ACCESS);
		}

		// write-enabled page?
		if (BIT(m_map[offset >> 11], 14) == 0)
		{
			m_map_control &= ~(1 << MAP_BLOCK_ACCESS);
			LOG("memory_w: m_map_control(%02x)\n", m_map_control);

			LOG("memory_w: bus error: READONLY %08x fc(%d)\n",  OFF16_TO_OFF8(offset), m_maincpu->get_fc());
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
			m_maincpu->set_buserror_details(offset0 << 1, 0, m_maincpu->get_fc());
			
			mem_mask = 0;
		}

		// mark page dirty (NB before we overwrite offset)
		if (mem_mask)
		{
			m_map[offset >> 11] |= 0x8000;
			LOG("memory_w: DIRTY m_map(0x%04x) m_map_control(%02x)\n", m_map[offset >> 11], m_map_control);
		}
		
		offset = BIT(offset, 0, 11) | BIT(m_map[offset >> 11], 0, 11) << 11;
	}

	// NB byte memory limit, offset is *word* offset
	if (offset < OFF8_TO_OFF16(0x600000) && offset >= OFF8_TO_OFF16(MAXRAM) && !machine().side_effects_disabled())
	{
		LOG("memory_w: bus error: %08x fc(%d)\n",  OFF16_TO_OFF8(offset), m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_maincpu->set_buserror_details(offset0 << 1, 0, m_maincpu->get_fc());
	}

	//LOG("memory_w: %08x <= %04x\n",  OFF16_TO_OFF8(offset), data);

	m_vm->write16(offset, data, mem_mask);
}

u16 tek440x_state::map_r(offs_t offset)
{
	LOG("map_r 0x%08x => %04x\n",offset>>11, m_map[offset >> 11] );

	// selftest does a read and expects it to fail iff !MAP_SYS_WR_ENABLE; its not WR enable, its enable..
	if (!BIT(m_map_control, MAP_SYS_WR_ENABLE))
	{
			LOG("map_r: bus error: PID(%d) %08x fc(%d)\n", BIT(m_map[offset >> 11], 11, 3), OFF16_TO_OFF8(offset), m_maincpu->get_fc());
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
			m_maincpu->set_buserror_details(offset, 0, m_maincpu->get_fc());
			return 0;
	}

	return m_map[offset >> 11];
}

void tek440x_state::map_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("map_w 0x%08x <= %04x\n",offset>>11, data);

	if (BIT(m_map_control, MAP_SYS_WR_ENABLE))
	{
		COMBINE_DATA(&m_map[offset >> 11]);
	}
}

u8 tek440x_state::mapcntl_r()
{
	// page 2.1-54 implies that this can only be read in user mode


	//LOG("mapcntl_r(%02x)\n", m_map_control);

	u8 ans = m_map_control;
	
	// mask out 'SysWrEn'
	if ((ans & 0xc0) != 0xc0)
		ans &= ~0x20;
	else
		ans |= 0x20;
		
	return ans;
}

void tek440x_state::mapcntl_w(u8 data)
{
#if 0
	LOG("mapcntl_w mmu_enable   %2d\n", BIT(data, MAP_VM_ENABLE));
	LOG("mapcntl_w write_enable %2d\n", BIT(data, MAP_SYS_WR_ENABLE));
	LOG("mapcntl_w pte PID    0x%02x\n", data & 15);
#endif

#if 0
	// think this is just wrong
	if (BIT(data, MAP_VM_ENABLE))
		m_map_view.select(0);
	else
		m_map_view.disable();
#endif

	// NB bit 6 & 7 is not used
	m_map_control = data & 0x3f;
	
}

u16 tek440x_state::videoaddr_r(offs_t offset)
{
	LOG("videoaddr_r %08x\n", offset);

	return m_videoaddr[offset];
}

void tek440x_state::videoaddr_w(offs_t offset, u16 data)
{
//	LOG("videoaddr_w %08x %04x\n", offset, data);
	m_videoaddr[offset] = data;
}

u8 tek440x_state::videocntl_r()
{
	int ans = m_videocntl;
	
	if (m_screen->vblank())
		ans |= 0x10;					// pretty sure this is VBL indicator; selftest looks for turning on and off within 2^15 cycles
	else
		ans |= 0x40 + 0x20;		// no idea what these are; selftest looks for turning on and off within 0x200000 cycles
	
	return ans;
}

void tek440x_state::videocntl_w(u8 data)
{
	m_videocntl = data;
#if 0
	LOG("m_videocntl %02x\n", data);
	LOG("m_videocntl VBenable   %2d\n", BIT(data, 6));
	LOG("m_videocntl ScreenOn   %2d\n", BIT(data, 5));
	LOG("m_videocntl ScreenInv  %2d\n", BIT(data, 4));
	LOG("m_videocntl ScreenPan  %2d\n", data & 15);
#endif
	m_vint->in_w<0>(BIT(data, 6));
}


void tek440x_state::sound_w(u8 data)
{
	if (m_boot)
		LOG("BOOT PROM disabled\n");
	
	m_snsnd->write(data);
	m_boot = false;
}

void tek440x_state::led_w(u8 data)
{

	m_leds = data;
	LOG("LED %c%c%c%c\n",m_leds & 8 ? '*' : '-',m_leds & 4 ? '*' : '-',m_leds & 2 ? '*' : '-',m_leds & 1 ? '*' : '-');
}

u8 tek440x_state::diag_r()
{
	return m_diag;
}

void tek440x_state::diag_w(u8 data)
{
	if (!m_kb_rclamp && m_kb_loop != BIT(data, 7))
		m_keyboard->kdo_w(!BIT(data, 7) || m_kb_tdata);

	m_kb_loop = BIT(data, 7);
	m_diag = data;
}

u8 tek440x_state::mouse_r(offs_t offset)
{
	u8 ans = 0;
	
	switch(offset)
	{
		case 0:
			ans = m_mousex->read();
			break;
		case 2:
			ans = m_mousey->read();
			break;
		case 4:
			ans = m_mousebtn->read();
			break;
		case 6:
			ans = m_mousebtn->read();
			break;

		default:
			break;
	}

	LOG("mouse_r %04x => %04x\n", offset, ans);
	
	return ans;
}

void tek440x_state::mouse_w(u8 data)
{
	m_mouse = data;
	LOG("mouse select(%x)\n", m_mouse);
}

void tek440x_state::write_txd(int state)
{
	LOG("mouse write_txd(%x)\n", state);
}

void tek440x_state::kb_rdata_w(int state)
{
	m_kb_rdata = state;
	if (!m_kb_rclamp)
		m_duart->rx_a_w(state);
}

void tek440x_state::kb_rclamp_w(int state)
{
	if (m_kb_rclamp != !state)
	{
		m_kb_rclamp = !state;

		// Clamp RXDA to 1 and KBRDATA to 0 when DUART asserts RxRDYA
		if (m_kb_tdata || !m_kb_loop)
			m_keyboard->kdo_w(state);
		m_duart->rx_a_w(state ? m_kb_rdata : 1);
	}
}

void tek440x_state::kb_tdata_w(int state)
{
	if (m_kb_tdata != state)
	{
		m_kb_tdata = state;

		m_duart->ip4_w(!state);
		if (m_kb_loop && m_kb_rdata && !m_kb_rclamp)
			m_keyboard->kdo_w(state);
	}
}

void tek440x_state::irq1_w(int state)
{
	LOG("irq_w %04x\n", state);
	
	m_u244latch = state;
	if (state == 1)
	{
		LOG("M68K_IRQ_1 assert\n");
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}
}

// to handle offset 0x1xx writes resetting TPInt...
u16 tek440x_state::timer_r(offs_t offset)
{
	LOG("timer_r %08x\n", offset);
	return m_timer->read16(offset);
}

void tek440x_state::timer_w(offs_t offset, u16 data)
{
	LOG("timer_w %08x %04x\n", OFF16_TO_OFF8(offset), data);
	m_timer->write16(offset, data);

	if (m_u244latch)
	{
		LOG("M68K_IRQ_1 clear\n");
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_u244latch = 0;
	}
}

u8 tek440x_state::duart_r(offs_t offset)
{
	return m_duart->read(offset);
}

void tek440x_state::duart_w(offs_t offset, u8 data)
{
	// Transmit Buffer?
	if (offset == 3)
	{
		if (m_diag & 0x80)
		{
			LOG("LOOPBACK mode\n");
			m_duart->write(0x0, 0x80);
		}
	}

	m_duart->write(offset, data);
}

void tek440x_state::logical_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(tek440x_state::memory_r), FUNC(tek440x_state::memory_w));
#if 0
	map(0x800000, 0xffffff).view(m_map_view);
	m_map_view[0](0x800000, 0xffffff).rw(FUNC(tek440x_state::map_r), FUNC(tek440x_state::map_w));
#else
	map(0x800000, 0xffffff).rw(FUNC(tek440x_state::map_r), FUNC(tek440x_state::map_w));
#endif
}

void tek440x_state::physical_map(address_map &map)
{
	map(0x000000, MAXRAM-1).ram().share("mainram");						// +1MB RAM option;
	map(0x600000, 0x61ffff).ram().share("vram");

	// 700000-71ffff spare 0
	// 720000-73ffff spare 1
	map(0x740000, 0x747fff).rom().mirror(0x8000).region("maincpu", 0).w(FUNC(tek440x_state::led_w));
	map(0x760000, 0x760fff).ram().mirror(0xf000); // debug RAM

	// 780000-79ffff processor board I/O
	map(0x780000, 0x780000).rw(FUNC(tek440x_state::mapcntl_r), FUNC(tek440x_state::mapcntl_w));
	// 782000-783fff: video address registers
	map(0x782000, 0x782001).rw(FUNC(tek440x_state::videoaddr_r),FUNC(tek440x_state::videoaddr_w));
	// 784000-785fff: video control registers
	map(0x784000, 0x784000).rw(FUNC(tek440x_state::videocntl_r),FUNC(tek440x_state::videocntl_w));
	// 786000-787fff: spare
	map(0x788000, 0x788000).w(FUNC(tek440x_state::sound_w));
	// 78a000-78bfff: NS32081 FPU
	map(0x78c000, 0x78c007).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write)).umask16(0xff00);
	// 78e000-78ffff: spare

	// 7a0000-7bffff peripheral board I/O
	// 7a0000-7affff: reserved
	map(0x7b0000, 0x7b0000).rw(FUNC(tek440x_state::diag_r),FUNC(tek440x_state::diag_w));
	// 7b1000-7b1fff: diagnostic registers
	// 7b2000-7b3fff: Centronics printer data
	map(0x7b2000, 0x7b3fff).rw(m_printer, FUNC(i8255_device::read), FUNC(i8255_device::write));
	
	map(0x7b4000, 0x7b401f).rw(FUNC(tek440x_state::duart_r), FUNC(tek440x_state::duart_w)).umask16(0xff00);
	// 7b6000-7b7fff: Mouse
	map(0x7b6000, 0x7b6fff).rw(FUNC(tek440x_state::mouse_r),FUNC(tek440x_state::mouse_w));

	map(0x7b8000, 0x7b8003).rw(m_timer, FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0x7b8100, 0x7b8103).rw(FUNC(tek440x_state::timer_r), FUNC(tek440x_state::timer_w));
	
	// 7ba000-7bbfff: MC146818 RTC
	map(0x7bc000, 0x7bc000).lw8(
		[this](u8 data)
		{
			m_scsi->set_own_id(data & 7);

			// TODO: bit 7 -> SCSI bus reset
			LOG("scsi bus reset %d\n", BIT(data, 7));
		}, "scsi_addr"); // 7bc000-7bdfff: SCSI bus address registers
	map(0x7be000, 0x7be01f).m(m_scsi, FUNC(ncr5385_device::map)).umask16(0xff00); //.mirror(0x1fe0) .cswidth(16);

	// 7c0000-7fffff EPROM application space
	map(0x7c0000, 0x7fffff).nopr();
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tek4404 )
	PORT_START("mousex")
	PORT_BIT( 0x00ff, 0, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)

	PORT_START("mousey")
	PORT_BIT( 0x00ff, 0, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)

	PORT_START("mousebtn")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tek_msu_fdc", TEK_MSU_FDC);
}

// interrupts
// 7 debug
// 6 vsync
// 5 uart
// 4 spare
// 3 scsi
// 2 dma (network?)
// 1 timer/printer

void tek440x_state::tek4404(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, 40_MHz_XTAL / 4); // MC68010L10
	m_maincpu->set_addrmap(AS_PROGRAM, &tek440x_state::logical_map);

	ADDRESS_MAP_BANK(config, m_vm);
	m_vm->set_addrmap(0, &tek440x_state::physical_map);
	m_vm->set_data_width(16);
	m_vm->set_addr_width(23);
	m_vm->set_endianness(ENDIANNESS_BIG);

	INPUT_MERGER_ALL_HIGH(config, m_vint);
	m_vint->output_handler().set_inputline(m_maincpu, M68K_IRQ_6);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(25.2_MHz_XTAL, 800, 0, 640, 525, 0, 480); // 31.5 kHz horizontal (guessed), 60 Hz vertical
	m_screen->set_screen_update(FUNC(tek440x_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(m_vint, FUNC(input_merger_all_high_device::in_w<1>));
	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	MOS6551(config, m_acia, 40_MHz_XTAL / 4 / 10);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->irq_handler().set_inputline(m_maincpu, M68K_IRQ_7);

	I8255A(config, m_printer);

	MC68681(config, m_duart, 14.7456_MHz_XTAL / 4);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_5); // auto-vectored
	m_duart->outport_cb().set(FUNC(tek440x_state::kb_rclamp_w)).bit(4);
	m_duart->outport_cb().append(m_keyboard, FUNC(tek410x_keyboard_device::reset_w)).bit(3);
	m_duart->a_tx_cb().set(m_keyboard, FUNC(tek410x_keyboard_device::kdi_w));

	TEK410X_KEYBOARD(config, m_keyboard);
	m_keyboard->tdata_callback().set(FUNC(tek440x_state::kb_tdata_w));
	m_keyboard->rdata_callback().set(FUNC(tek440x_state::kb_rdata_w));

	AM9513(config, m_timer, 40_MHz_XTAL / 4 / 10); // from CPU E output

	// see diagram page 2.2-6
	m_timer->out1_cb().set("irq1", FUNC(input_merger_device::in_w<0>));
	m_timer->out2_cb().set("irq1", FUNC(input_merger_device::in_w<1>));
	INPUT_MERGER_ALL_LOW(config, "irq1").output_handler().set(FUNC(tek440x_state::irq1_w));

	MC146818(config, m_rtc, 32.768_MHz_XTAL);

	NSCSI_BUS(config, "scsi");
	// hard disk is a Micropolis 1304 (https://www.micropolis.com/support/hard-drives/1304)
	// with a Xebec 1401 SASI adapter inside the Mass Storage Unit
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "tek_msu_fdc");
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5385", NCR5385).clock(40_MHz_XTAL / 4).machine_config(
		[this](device_t *device)
		{
			ncr5385_device &adapter = downcast<ncr5385_device &>(*device);

			adapter.irq().set_inputline(m_maincpu, M68K_IRQ_3);
		});

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	SPEAKER(config, "mono").front_center();

	SN76496(config, m_snsnd, 25.2_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.80);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tek4404 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "boot.bin", 0x000000, 0x008000, CRC(bceb9462) SHA1(01960a90eab482957469ad4e7e3dc74f33588779) )
	//ROM_LOAD16_BYTE( "tek_u158.bin", 0x000000, 0x004000, CRC(9939e660) SHA1(66b4309e93e4ff20c1295dc2ec2a8d6389b2578c) )
	//ROM_LOAD16_BYTE( "tek_u163.bin", 0x000001, 0x004000, CRC(a82dcbb1) SHA1(a7e4545e9ea57619faacc1556fa346b18f870084) )

	ROM_REGION( 0x2000, "scsimfm", 0 )
	ROM_LOAD( "scsi_mfm.bin", 0x000000, 0x002000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME                               FLAGS
COMP( 1984, tek4404, 0,      0,      tek4404, tek4404, tek440x_state, empty_init, "Tektronix", "4404 Artificial Intelligence System", MACHINE_NOT_WORKING )
