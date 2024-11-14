// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

Hilger Analytical AB6089 Mk. 1 (LSI Octopus)

2013-07-26 Skeleton driver.

http://computers.mcbx.netne.net/8bit/hilger/index.htm

Below is an extract from the page:

The computer has 2 CPUs: Z80 and 8088. Most circuits are dated 1985-1986, display circuitry is made by Signetics.
Mainboard was manufactured by LSI Computers Ltd. under part numbers: 15000SS100 and 15000P4100. All steel parts
of casing are grounded by wires. It's graphics card works in pass-through mode: It takes picture from mainboard's
TTL output and adds image to it, then it puts it to monitor. Its ROM is prepared for hard disk and some type of
network, yet no HDD controller nor network interfaces are present inside - it seems that they were added as
expansion cards.

UPDATE: It's re-branded LSI Octopus computer, a very well-expandable machine which was designed to "grow with a
company". First stage was a computer which could be used even with TV set. As requirements increased, Octopus
could be equipped with hard disk controller, network adapter, multi-terminal serial port card to act as a terminal
server or even CPU cards to run concurrent systems. There were even tape backup devices for it. Octopus could run
CP/M, MP/M (concurrent - multitasking-like OS, even with terminals), or even MS-DOS - CP/M or MP/M could be used
with Z80 or 8080. There was also LSI ELSIE system, a concurrent DOS. Last British LSI machines were 386 computers
which could be used as servers for Octopus computers.

Manufacturer    Hilger Analytical / LSI Computers Ltd.

Origin  UK
Year of unit    1986?
Year of introduction    1985
End of production   ?
CPU     Z80, 8088
Speed   8MHz (8088) or 6MHz (Z80)
RAM     128kB or 256kB, expandable to 768kB
ROM     16kB (Basic)
Colors:     ??
Sound:  Speaker. Beeps :)
OS:     CP/M 80 or 86
MP/M 80 o 86
Concurrent CP/M
LSI ELSIE
MS-DOS
Text display: SCN2674B CRTC, SCB2675C for attributes
Graphics: ?? (option board, ROM is dumped)

Media:  Two internal 5.25" floppy disk drives, DS DD, 96tpi.
Probably hard disk

Power supply:
Built-in switching power supply.

I/O:    Serial port
2 parallel ports

Video TTL Output
Composite video output

Possible upgrades:  Many

Software accessibility:
Dedicated: Impossible.
CP/M - Good
DOS - Good.

It won't take XT nor AT keyboard, but pinout is quite similar. UPDATE: I saw a few photos of keyboard.
It's another Z80 computer! It has an EPROM, simple memory and CPU.

After powering on, it should perform POST writing:

TESTING...
    Main Processor
    PROM
    DMA Controllers
    RAM
    Interrupts
    Floppy Discs
    Hard Disc Controller   (optionally - if installed)

Waiting for hard Disc... (Optionally - if installed)

Firmware versions:

SYSTEM         18B (or other)
GRAPHICS      4    (if graphic card installed)

And probably it should boot or display:

Insert System Disk.

Or maybe:

Nowhere to boot from.

Load options:
    Floppy
    Pro Network
    Winchester
Enter selection:

This information was gained by studying boot ROM of the machine.

It's a very rare computer. It has 2 processors, Z80 and 8088, so it can run both MS-DOS and CP/M.

****************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "octo_kbd.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "video/scn2674.h"

#include "bus/centronics/ctronics.h"
#include "bus/centronics/comxpl80.h"
#include "bus/centronics/epson_ex800.h"
#include "bus/centronics/epson_lx800.h"
#include "bus/centronics/epson_lx810l.h"
#include "bus/centronics/printer.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class octopus_state : public driver_device
{
public:
	octopus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_crtc(*this, "crtc"),
		m_vram(*this, "vram", 0x10000, ENDIANNESS_LITTLE),
		m_fontram(*this, "fram"),
		m_dma1(*this, "dma1"),
		m_dma2(*this, "dma2"),
		m_pic1(*this, "pic_master"),
		m_pic2(*this, "pic_slave"),
		m_rtc(*this, "rtc"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_kb_uart(*this, "keyboard"),
		m_pit(*this, "pit"),
		m_ppi(*this, "ppi"),
		m_speaker(*this, "speaker"),
		m_serial(*this, "serial"),
		m_parallel(*this, "parallel"),
		m_z80_bankdev(*this, "z80_bank"),
		m_ram(*this, "ram"),
		m_dswa(*this, "DSWA"),
		m_current_dma(-1),
		m_speaker_active(false),
		m_beep_active(false),
		m_z80_active(false)
	{ }

	void octopus(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	SCN2674_DRAW_CHARACTER_MEMBER(display_pixels);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t get_slave_ack(offs_t offset);
	[[maybe_unused]] void fdc_drq(int state);
	uint8_t bank_sel_r(offs_t offset);
	void bank_sel_w(offs_t offset, uint8_t data);
	uint8_t dma_read(offs_t offset);
	void dma_write(offs_t offset, uint8_t data);
	void dma_hrq_changed(int state);
	uint8_t system_r(offs_t offset);
	void system_w(offs_t offset, uint8_t data);
	uint8_t cntl_r();
	void cntl_w(uint8_t data);
	uint8_t gpo_r();
	void gpo_w(uint8_t data);
	uint8_t vidcontrol_r();
	void vidcontrol_w(uint8_t data);
	uint8_t z80_io_r();
	void z80_io_w(uint8_t data);
	IRQ_CALLBACK_MEMBER(x86_irq_cb);
	uint8_t rtc_r();
	void rtc_w(uint8_t data);
	uint8_t z80_vector_r(offs_t offset);
	void z80_vector_w(offs_t offset, uint8_t data);
	uint8_t parallel_r(offs_t offset);
	void parallel_w(offs_t offset, uint8_t data);
	uint8_t video_latch_r(offs_t offset);
	void video_latch_w(offs_t offset, uint8_t data);

	void spk_w(int state);
	void spk_freq_w(int state);
	void beep_w(int state);
	void serial_clock_w(int state);
	void parallel_busy_w(int state) { m_printer_busy = state; }
	void parallel_slctout_w(int state) { m_printer_slctout = state; }

	void dack0_w(int state) { m_dma1->hack_w(state ? 0 : 1); }  // for all unused DMA channel?
	void dack1_w(int state) { if(!state) m_current_dma = 1; else if(m_current_dma == 1) m_current_dma = -1; }  // HD
	void dack2_w(int state) { if(!state) m_current_dma = 2; else if(m_current_dma == 2) m_current_dma = -1; }  // RAM refresh
	void dack3_w(int state) { m_dma1->hack_w(state ? 0 : 1); }
	void dack4_w(int state) { m_dma1->hack_w(state ? 0 : 1); }
	void dack5_w(int state) { if(!state) m_current_dma = 5; else if(m_current_dma == 5) m_current_dma = -1; }  // Floppy
	void dack6_w(int state) { m_dma1->hack_w(state ? 0 : 1); }
	void dack7_w(int state) { m_dma1->hack_w(state ? 0 : 1); }

	void octopus_io(address_map &map) ATTR_COLD;
	void octopus_mem(address_map &map) ATTR_COLD;
	void octopus_sub_io(address_map &map) ATTR_COLD;
	void octopus_sub_mem(address_map &map) ATTR_COLD;
	void octopus_vram(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(beep_off);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<scn2674_device> m_crtc;
	memory_share_creator<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_fontram;
	required_device<am9517a_device> m_dma1;
	required_device<am9517a_device> m_dma2;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<mc146818_device> m_rtc;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<i8251_device> m_kb_uart;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<speaker_sound_device> m_speaker;
	required_device<z80sio_device> m_serial;
	required_device<centronics_device> m_parallel;
	required_device<address_map_bank_device> m_z80_bankdev;
	required_device<ram_device> m_ram;
	required_ioport m_dswa;

	uint8_t m_hd_bank;  // HD bank select
	uint8_t m_fd_bank;  // Floppy bank select
	uint8_t m_z80_bank; // Z80 bank / RAM refresh
	int8_t m_current_dma;  // current DMA channel (-1 for none)
	uint8_t m_current_drive;
	uint8_t m_cntl;  // RTC / FDC control (PPI port B)
	uint8_t m_gpo;  // General purpose outputs (PPI port C)
	uint8_t m_vidctrl;
	bool m_speaker_active;
	bool m_beep_active;
	bool m_speaker_level;
	bool m_z80_active;
	bool m_rtc_address;
	bool m_rtc_data;
	uint8_t m_prev_cntl;
	uint8_t m_rs232_vector;
	uint8_t m_rs422_vector;
	bool m_printer_busy;
	bool m_printer_slctout;
	uint8_t m_char_latch_r;
	uint8_t m_attr_latch_r;
	uint8_t m_char_latch_w;
	uint8_t m_attr_latch_w;

	emu_timer* m_timer_beep;
};


void octopus_state::octopus_mem(address_map &map)
{
	map(0x00000, 0xcffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	//map(0xd0000, 0xdffff).ram().share("vram");
	map(0xe0000, 0xe3fff).noprw();
	map(0xe4000, 0xe5fff).ram().share("fram");
	map(0xe6000, 0xe7fff).rom().region("chargen", 0);
	map(0xe8000, 0xfbfff).noprw();
	map(0xfc000, 0xfffff).rom().region("user1", 0);
}

void octopus_state::octopus_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x0f).rw(m_dma1, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x10, 0x1f).rw(m_dma2, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x20, 0x20).portr("DSWA");
	map(0x21, 0x2f).rw(FUNC(octopus_state::system_r), FUNC(octopus_state::system_w));
	map(0x31, 0x33).rw(FUNC(octopus_state::bank_sel_r), FUNC(octopus_state::bank_sel_w));
	map(0x50, 0x51).rw(m_kb_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	// 0x70-73: HD controller
	map(0x80, 0x83).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xa0, 0xa0).rw(m_serial, FUNC(z80sio_device::da_r), FUNC(z80sio_device::da_w));
	map(0xa1, 0xa1).rw(m_serial, FUNC(z80sio_device::ca_r), FUNC(z80sio_device::ca_w));
	map(0xa2, 0xa2).rw(m_serial, FUNC(z80sio_device::db_r), FUNC(z80sio_device::db_w));
	map(0xa3, 0xa3).rw(m_serial, FUNC(z80sio_device::cb_r), FUNC(z80sio_device::cb_w));
	map(0xb0, 0xb1).rw(m_pic1, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xb4, 0xb5).rw(m_pic2, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xc0, 0xc7).rw(m_crtc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0xc8, 0xc8).rw(FUNC(octopus_state::vidcontrol_r), FUNC(octopus_state::vidcontrol_w));
	map(0xc9, 0xca).rw(FUNC(octopus_state::video_latch_r), FUNC(octopus_state::video_latch_w));
	// 0xcf: mode control
	map(0xd0, 0xd3).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0xe0, 0xe4).rw(FUNC(octopus_state::z80_vector_r), FUNC(octopus_state::z80_vector_w));
	map(0xf0, 0xf1).rw(FUNC(octopus_state::parallel_r), FUNC(octopus_state::parallel_w));
	map(0xf8, 0xff).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}


void octopus_state::octopus_sub_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_z80_bankdev, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}

void octopus_state::octopus_sub_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(octopus_state::z80_io_r), FUNC(octopus_state::z80_io_w));
}

void octopus_state::octopus_vram(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(octopus_state::vram_r), FUNC(octopus_state::vram_w));
}

/* Input ports */
static INPUT_PORTS_START( octopus )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x02, "Number of floppy drives" ) PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING( 0x00, "None" )
	PORT_DIPSETTING( 0x01, "1 Floppy" )
	PORT_DIPSETTING( 0x02, "2 Floppies" )
	PORT_DIPSETTING( 0x03, "Not used" )
	PORT_DIPNAME( 0x04, 0x00, "Quad drives" ) PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING( 0x00, "Disabled" )
	PORT_DIPSETTING( 0x04, "Enabled" )
	PORT_DIPNAME( 0x38, 0x00, "Winchester drive type" ) PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING( 0x00, "None" )
	PORT_DIPSETTING( 0x08, "RO201" )
	PORT_DIPSETTING( 0x10, "RO202" )
	PORT_DIPSETTING( 0x18, "Reserved" )
	PORT_DIPSETTING( 0x20, "RO204" )
	PORT_DIPSETTING( 0x28, "Reserved" )
	PORT_DIPSETTING( 0x30, "RO208" )
	PORT_DIPSETTING( 0x38, "Reserved" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Colour monitor connected" ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(octopus_state::beep_off)
{
	m_beep_active = false;
}

void octopus_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = m_char_latch_w;
	m_vram[offset+0x1000] = m_attr_latch_w;
}

uint8_t octopus_state::vram_r(offs_t offset)
{
	m_char_latch_r = m_vram[offset];
	m_attr_latch_r = m_vram[offset+0x1000];
	return m_vram[offset];
}

void octopus_state::fdc_drq(int state)
{
	// TODO
}

uint8_t octopus_state::bank_sel_r(offs_t offset)
{
	switch(offset)
	{
	case 0:
		return m_hd_bank;
	case 1:
		return m_fd_bank;
	case 2:
		return m_z80_bank;
	}
	return 0xff;
}

void octopus_state::bank_sel_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:
		m_hd_bank = data;
		logerror("HD bank = %i\n",data);
		break;
	case 1:
		m_fd_bank = data;
		logerror("Floppy bank = %i\n",data);
		break;
	case 2:
		m_z80_bank = data;
		m_z80_bankdev->set_bank(m_z80_bank & 0x0f);
		logerror("Z80/RAM bank = %i\n",data);
		break;
	}
}

// System control
// 0x20: read: System type, write: Z80 NMI
// 0x21: read: bit5=SLCTOUT from parallel interface, bit6=option board parity fail, bit7=main board parity fail
//       write: parity fail reset
// ports 0x20 and 0x21 read out the DIP switch configuration (the firmware function to get system config simply does IN AX,20h)
// 0x28: write: Z80 enable
void octopus_state::system_w(offs_t offset, uint8_t data)
{
	logerror("SYS: System control offset %i data %02x\n",offset+1,data);
	switch(offset)
	{
	case 7:  // enable Z80, halt 8088
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_z80_active = true;
		break;
	}
}

uint8_t octopus_state::system_r(offs_t offset)
{
	uint8_t val = 0x00;
	switch(offset)
	{
	case 0:
		val = 0x1f;
		if(m_printer_slctout)
			val |= 0x20;
		return val;  // do bits 0-4 mean anything?  Language DIPs?
	}

	return 0xff;
}

// Any I/O cycle relinquishes control of the bus
uint8_t octopus_state::z80_io_r()
{
	z80_io_w(0);
	return 0x00;
}

void octopus_state::z80_io_w(uint8_t data)
{
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_z80_active = false;
}

// Z80 vector for RS232 and RS422
uint8_t octopus_state::z80_vector_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
			return m_rs232_vector;
		case 4:
			return m_rs422_vector;
		default:
			return 0xff;
	}
	return 0xff;
}

void octopus_state::z80_vector_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			m_rs232_vector = data;
			logerror("RS232 vector set to %02x\n",data);
			break;
		case 4:
			m_rs422_vector = data;
			logerror("RS422 vector set to %02x\n",data);
			break;
		default:
			logerror("Read invalid vector port 0x%02x\n",0xe0 + offset);
	}
}

// RTC data and I/O - PPI port A
// bits 0-3 of RTC/FDC control go to control lines of the MC146818
// The technical manual does not mention what is connected to each bit
// This is an educated guess, based on the BIOS code
// bit 0 = ? (Pulsed low after writing to an RTC register)
// bit 1 = PPI Port A strobe?
// bit 2 = Data strobe?
// bit 3 = Address strobe?
uint8_t octopus_state::rtc_r()
{
	uint8_t ret = 0xff;

	if(m_rtc_data)
		ret = m_rtc->data_r();

	return ret;
}

void octopus_state::rtc_w(uint8_t data)
{
	if(m_rtc_data)
		m_rtc->data_w(data);
	else if(m_rtc_address)
		m_rtc->address_w(data);
}

// RTC/FDC control - PPI port B
// bits0-3: RTC control lines
// bit4-5: write precomp.
// bit6-7: drive select
uint8_t octopus_state::cntl_r()
{
	return m_cntl;
}

void octopus_state::cntl_w(uint8_t data)
{
	m_cntl = data;

	if((m_cntl & 0x08) && !(m_prev_cntl & 0x08))
	{
		m_rtc_address = true;
		m_rtc_data = false;
	}
	if((data & 0x04) && !(m_prev_cntl & 0x04))
	{
		m_rtc_address = false;
		m_rtc_data = true;
	}
	m_ppi->pc4_w(data & 0x02);
	m_prev_cntl = m_cntl;
	m_current_drive = (data & 0xc0) >> 6;
	switch(m_current_drive)
	{
	case 1:
		m_fdc->set_floppy(m_floppy[0]->get_device());
		m_floppy[0]->get_device()->mon_w(0);
		break;
	case 2:
		m_fdc->set_floppy(m_floppy[1]->get_device());
		m_floppy[1]->get_device()->mon_w(0);
		break;
	}
	logerror("Selected floppy drive %i (%02x)\n",m_current_drive,data);
}

// General Purpose Outputs - PPI port C
// bit 2 - floppy side select
// bit 1 - parallel data I/O (0 = output)
// bit 0 - parallel control I/O (0 = output)
uint8_t octopus_state::gpo_r()
{
	return m_gpo;
}

void octopus_state::gpo_w(uint8_t data)
{
	m_gpo = data;
	switch(m_current_drive)
	{
	case 1:
		m_floppy[0]->get_device()->ss_w((data & 0x04) >> 2);
		break;
	case 2:
		m_floppy[1]->get_device()->ss_w((data & 0x04) >> 2);
		break;
	default:
		logerror("Attempted to set side on unknown drive %i\n",m_current_drive);
	}
}

// Video control register
// bit 0 - video dot clock - 0=17.6MHz, 1=16MHz
// bit 2 - floppy DDEN line
// bit 3 - floppy FCLOCK line - 0=1MHz, 1=2MHz
// bits 4-5 - character width - 0=10 dots, 1=6 dots, 2=8 dots, 3=9 dots
// bit 6 - cursor mode (colour only) - 0=inverse cursor, 1=white cursor (normal)
// bit 7 - 1=monochrome mode, 0=colour mode
// Is bit 7 writable, or just mirrors DIP switch setting?  Tech manual is unclear.
uint8_t octopus_state::vidcontrol_r()
{
	return m_vidctrl;
}

void octopus_state::vidcontrol_w(uint8_t data)
{
	m_fdc->dden_w(BIT(data, 2));
	m_fdc->set_unscaled_clock(16_MHz_XTAL / (BIT(data, 3) ? 16 : 8));

	if (((m_vidctrl ^ data) & 0x31) != 0)
	{
		unsigned dots = 4 + ((data & 0x30) >> 3);
		if ((data & 0x30) == 0)
			dots = 10;
		else if ((data & 0x30) == 0x30)
			dots = 9;

		auto dotclk = BIT(data, 0) ? 16_MHz_XTAL : 17.6_MHz_XTAL;

		m_crtc->set_character_width(dots);
		m_crtc->set_unscaled_clock(dotclk / dots);
	}

	m_vidctrl = data;
}

// Sound hardware
// Sound level provided by i8253 timer 2
// Enabled by /DTR signal from i8251
// 100ms beep triggered by pulsing /CTS signal low on i8251
void octopus_state::spk_w(int state)
{
	m_speaker_active = !state;
	m_speaker->level_w(((m_speaker_active || m_beep_active) && m_speaker_level) ? 1 : 0);
}

void octopus_state::spk_freq_w(int state)
{
	m_speaker_level = state;
	m_speaker->level_w(((m_speaker_active || m_beep_active) && m_speaker_level) ? 1 : 0);
}

void octopus_state::beep_w(int state)
{
	if(!state)  // active low
	{
		m_beep_active = true;
		m_speaker->level_w(((m_speaker_active || m_beep_active) && m_speaker_level) ? 1 : 0);
		m_timer_beep->adjust(attotime::from_msec(100));
	}
}

void octopus_state::serial_clock_w(int state)
{
	m_serial->rxca_w(state);
	m_serial->txca_w(state);
}

// Parallel Centronics port
// 0xf0 : data
// 0xf1 : control
//      bit 2 = INIT?  On boot, bits 0 and 1 are set high, bit 2 is set low then high again, all other bits are set low
// can generate interrupts - tech manual suggests that Strobe, Init, Ack, and Busy can trigger an interrupt (IRQ14)
uint8_t octopus_state::parallel_r(offs_t offset)
{
	switch(offset)
	{
	case 0:
		return 0;
	case 1:
		return m_printer_busy ? 0x01 : 0x00;  // correct?  Tech manual doesn't explain which bit is which
	}
	return 0xff;
}

void octopus_state::parallel_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:  // data
		if(!(m_gpo & 0x02))  // parallel data direction
		{
			m_parallel->write_data0(BIT(data,0));
			m_parallel->write_data1(BIT(data,1));
			m_parallel->write_data2(BIT(data,2));
			m_parallel->write_data3(BIT(data,3));
			m_parallel->write_data4(BIT(data,4));
			m_parallel->write_data5(BIT(data,5));
			m_parallel->write_data6(BIT(data,6));
			m_parallel->write_data7(BIT(data,7));
		}
		break;
	case 1:  // control (bit order unknown?)
		if(!(m_gpo & 0x01))  // parallel control direction
		{
			m_parallel->write_init(BIT(data,2));
			m_pic2->ir6_w(!BIT(data,2));
		}
		break;
	}
}

uint8_t octopus_state::dma_read(offs_t offset)
{
	uint8_t byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_current_dma == -1)
		return 0;
	byte = prog_space.read_byte((m_fd_bank << 16) + offset);
	return byte;
}

void octopus_state::dma_write(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_current_dma == -1)
		return;
	prog_space.write_byte((m_fd_bank << 16) + offset, data);
}

void octopus_state::dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma2->hack_w(state);
}

// Any interrupt will also give bus control back to the 8088
IRQ_CALLBACK_MEMBER(octopus_state::x86_irq_cb)
{
	uint8_t vector;
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_z80_active = false;
	vector = m_pic1->inta_cb(device,irqline);
	if(vector == 0x61)  // if we have hit a serial comms IRQ, then also have the Z80SIO/2 acknowledge the interrupt
		vector = m_serial->m1_r();
	return vector;
}

void octopus_state::machine_start()
{
	m_timer_beep = timer_alloc(FUNC(octopus_state::beep_off), this);
	m_vidctrl = 0xff;

	// install RAM
	m_maincpu->space(AS_PROGRAM).install_ram(0x0000,m_ram->size()-1,m_ram->pointer());
	m_maincpu->space(AS_PROGRAM).nop_readwrite(m_ram->size(),0xcffff);
}

void octopus_state::machine_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);  // halt Z80 to start with
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_z80_active = false;
	m_current_dma = -1;
	m_current_drive = 0;
	m_rtc_address = true;
	m_rtc_data = false;
	m_kb_uart->write_dsr(1);  // DSR is used to determine if a keyboard is connected?  If DSR is high, then the CHAR_OUT BIOS function will not output to the screen.
}

void octopus_state::video_start()
{
}

uint8_t octopus_state::video_latch_r(offs_t offset)
{
	if(offset & 0x01)
		return m_attr_latch_r;
	else
		return m_char_latch_r;
}

void octopus_state::video_latch_w(offs_t offset, uint8_t data)
{
	if(offset & 0x01)
		m_attr_latch_w = data;
	else
		m_char_latch_w = data;
}

SCN2674_DRAW_CHARACTER_MEMBER(octopus_state::display_pixels)
{
	// Attributes:
	//  - common bits
	// b7 : blink
	// b3 : underline
	//  - Monochrome
	// b6 : GP1 (general purpose)
	// b5 : reverse video
	// b4 : GP2 (general purpose)
	// b2 : High intensity
	// b1 : Grey background
	// b0 : Blank (TODO)
	//  - Colour
	// b6,5,4 : background colour (RGB)
	// b2,1,0 : foreground colour (RGB)
	if(!lg)
	{
		uint8_t tile = m_vram[address & 0x0fff];
		uint8_t attr = m_vram[(address & 0x0fff) + 0x1000];
		uint8_t data = m_fontram[(tile * 16) + linecount];
		rgb_t fg,bg;
		if(m_dswa->read() & 0x80)  // monochrome or colour mode is selected by switch 8 of system DIP switches
		{
			// colour (is the background at half intensity?)
			bg.set_r((attr & 0x40) ? 0x7f : 0x00);
			bg.set_g((attr & 0x20) ? 0x7f : 0x00);
			bg.set_b((attr & 0x10) ? 0x7f : 0x00);
			fg.set_r((attr & 0x04) ? 0xff : 0x00);
			fg.set_g((attr & 0x02) ? 0xff : 0x00);
			fg.set_b((attr & 0x01) ? 0xff : 0x00);
		}
		else
		{
			// monochrome
			if(attr & 0x02)
				fg = 0xffffff;
			else
				fg = 0x7f7f7f;
			if(attr & 0x04)
				bg = 0x7f7f7f;
			else
				bg = 0x000000;

			if(attr & 0x20)  // reverse video
				data = ~data;
		}
		if(ul && (attr & 0x08))
			data = 0xff;
		if(blink && (attr & 0x80))
			data = 0x00;
		if(cursor && !blink)
		{
			bool inverse = true;

			if(!(m_dswa->read() & 0x80))  // not available in monochrome mode
				inverse = false;
			if(m_vidctrl & 0x40)  // not enabled
				inverse = false;
			if(inverse)
				data = ~data;
			else
				data = 0xff;
		}
		for (int z=0;z<8;z++)
			bitmap.pix(y,x + z) = BIT(data,z) ? fg : bg;
	}
}

uint8_t octopus_state::get_slave_ack(offs_t offset)
{
	if (offset==7)
		return m_pic2->acknowledge();

	return 0x00;
}

static void octopus_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void keyboard(device_slot_interface &device)
{
	device.option_add("octopus", OCTOPUS_KEYBOARD);
}

void octopus_centronics_devices(device_slot_interface &device)
{
	device.option_add("pl80", COMX_PL80);
	device.option_add("ex800", EPSON_EX800);
	device.option_add("lx800", EPSON_LX800);
	device.option_add("lx810l", EPSON_LX810L);
	device.option_add("ap2000", EPSON_AP2000);
	device.option_add("printer", CENTRONICS_PRINTER);
}

void octopus_state::octopus(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 24_MHz_XTAL / 3);  // 8MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &octopus_state::octopus_mem);
	m_maincpu->set_addrmap(AS_IO, &octopus_state::octopus_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(octopus_state::x86_irq_cb));

	Z80(config, m_subcpu, 24_MHz_XTAL / 4); // 6MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &octopus_state::octopus_sub_mem);
	m_subcpu->set_addrmap(AS_IO, &octopus_state::octopus_sub_io);

	AM9517A(config, m_dma1, 24_MHz_XTAL / 6);  // 4MHz
	m_dma1->out_hreq_callback().set(m_dma2, FUNC(am9517a_device::dreq0_w));
	m_dma1->in_memr_callback().set(FUNC(octopus_state::dma_read));
	m_dma1->out_memw_callback().set(FUNC(octopus_state::dma_write));
	//m_dma1->in_ior_callback<0>().set_nop();
	//m_dma1->in_ior_callback<1>().set_nop();  // HDC
	//m_dma1->in_ior_callback<2>().set_nop();  // RAM Refresh
	//m_dma1->in_ior_callback<3>().set_nop();
	//m_dma1->out_iow_callback<0>().set_nop();
	//m_dma1->out_iow_callback<1>().set_nop();  // HDC
	//m_dma1->out_iow_callback<2>().set_nop();  // RAM Refresh
	//m_dma1->out_iow_callback<3>().set_nop();
	m_dma1->out_dack_callback<0>().set(FUNC(octopus_state::dack0_w));
	m_dma1->out_dack_callback<1>().set(FUNC(octopus_state::dack1_w));
	m_dma1->out_dack_callback<2>().set(FUNC(octopus_state::dack2_w));
	m_dma1->out_dack_callback<3>().set(FUNC(octopus_state::dack3_w));

	AM9517A(config, m_dma2, 24_MHz_XTAL / 6);  // 4MHz
	m_dma2->out_hreq_callback().set(FUNC(octopus_state::dma_hrq_changed));
	m_dma2->in_memr_callback().set(FUNC(octopus_state::dma_read));
	m_dma2->out_memw_callback().set(FUNC(octopus_state::dma_write));
	//m_dma2->in_ior_callback<0>().set_nop();
	m_dma2->in_ior_callback<1>().set(m_fdc, FUNC(fd1793_device::data_r));  // FDC
	//m_dma2->in_ior_callback<2>().set_nop();
	//m_dma2->in_ior_callback<3>().set_nop();
	//m_dma2->out_iow_callback<0>().set_nop();
	m_dma2->out_iow_callback<1>().set(m_fdc, FUNC(fd1793_device::data_w));  // FDC
	//m_dma2->out_iow_callback<2>().set_nop();
	//m_dma2->out_iow_callback<3>().set_nop();
	m_dma2->out_dack_callback<0>().set(FUNC(octopus_state::dack4_w));
	m_dma2->out_dack_callback<1>().set(FUNC(octopus_state::dack5_w));
	m_dma2->out_dack_callback<2>().set(FUNC(octopus_state::dack6_w));
	m_dma2->out_dack_callback<3>().set(FUNC(octopus_state::dack7_w));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(octopus_state::get_slave_ack));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir7_w));
	m_pic2->in_sp_callback().set_constant(0);

	// RTC (MC146818 via i8255 PPI)
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(octopus_state::rtc_r));
	m_ppi->in_pb_callback().set(FUNC(octopus_state::cntl_r));
	m_ppi->in_pc_callback().set(FUNC(octopus_state::gpo_r));
	m_ppi->out_pa_callback().set(FUNC(octopus_state::rtc_w));
	m_ppi->out_pb_callback().set(FUNC(octopus_state::cntl_w));
	m_ppi->out_pc_callback().set(FUNC(octopus_state::gpo_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pic2, FUNC(pic8259_device::ir2_w));

	// Keyboard UART
	I8251(config, m_kb_uart, 0);
	m_kb_uart->rxrdy_handler().set("pic_slave", FUNC(pic8259_device::ir4_w));
	m_kb_uart->dtr_handler().set(FUNC(octopus_state::spk_w));
	m_kb_uart->rts_handler().set(FUNC(octopus_state::beep_w));
	rs232_port_device &keyboard_port(RS232_PORT(config, "keyboard_port", keyboard, "octopus"));
	keyboard_port.rxd_handler().set(m_kb_uart, FUNC(i8251_device::write_rxd));
	clock_device &keyboard_clock_rx(CLOCK(config, "keyboard_clock_rx", 9600 * 64));
	keyboard_clock_rx.signal_handler().set(m_kb_uart, FUNC(i8251_device::write_rxc));
	clock_device &keyboard_clock_tx(CLOCK(config, "keyboard_clock_tx", 1200 * 64));
	keyboard_clock_tx.signal_handler().set(m_kb_uart, FUNC(i8251_device::write_txc));

	FD1793(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(m_pic1, FUNC(pic8259_device::ir5_w));
	m_fdc->drq_wr_callback().set(m_dma2, FUNC(am9517a_device::dreq1_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], octopus_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], octopus_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	SOFTWARE_LIST(config, "fd_list").set_original("octopus");

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(4.9152_MHz_XTAL / 2);  // DART channel A
	m_pit->out_handler<0>().set(FUNC(octopus_state::serial_clock_w));  // being able to write both Rx and Tx clocks at one time would be nice
	m_pit->set_clk<1>(4.9152_MHz_XTAL / 2);  // DART channel B
	m_pit->out_handler<1>().set(m_serial, FUNC(z80sio_device::rxtxcb_w));
	m_pit->set_clk<2>(4.9152_MHz_XTAL / 2);  // speaker frequency
	m_pit->out_handler<2>().set(FUNC(octopus_state::spk_freq_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	Z80SIO(config, m_serial, 16_MHz_XTAL / 4); // clock rate not mentioned in tech manual
	m_serial->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir1_w));
	m_serial->out_txda_callback().set("serial_a", FUNC(rs232_port_device::write_txd));
	m_serial->out_txdb_callback().set("serial_b", FUNC(rs232_port_device::write_txd));
	m_serial->out_rtsa_callback().set("serial_a", FUNC(rs232_port_device::write_rts));
	m_serial->out_rtsb_callback().set("serial_b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial_a(RS232_PORT(config, "serial_a", default_rs232_devices, nullptr));
	serial_a.rxd_handler().set(m_serial, FUNC(z80sio_device::rxa_w));
	serial_a.cts_handler().set(m_serial, FUNC(z80sio_device::ctsa_w)).invert();
	//serial_a.ri_handler().set(m_serial, FUNC(z80sio_device::ria_w)).invert();
	rs232_port_device &serial_b(RS232_PORT(config, "serial_b", default_rs232_devices, nullptr));
	serial_b.rxd_handler().set(m_serial, FUNC(z80sio_device::rxb_w));
	serial_b.cts_handler().set(m_serial, FUNC(z80sio_device::ctsb_w)).invert();
	//serial_b.ri_handler().set(m_serial, FUNC(z80sio_device::rib_w)).invert();

	CENTRONICS(config, m_parallel, octopus_centronics_devices, "printer");
	m_parallel->busy_handler().set(FUNC(octopus_state::parallel_busy_w));
	m_parallel->select_handler().set(FUNC(octopus_state::parallel_slctout_w));
	// TODO: Winchester HD controller (Xebec/SASI compatible? uses TTL logic)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 918, 0, 729, 350, 0, 325);
	//screen.set_raw(17.6_MHz_XTAL, 1008, 0, 792, 348, 0, 319);
	screen.set_screen_update("crtc", FUNC(scn2674_device::screen_update));

	SCN2674(config, m_crtc, 16_MHz_XTAL / 9); // dot clock and character width are both selectable
	m_crtc->intr_callback().set("pic_slave", FUNC(pic8259_device::ir0_w));
	m_crtc->set_character_width(9);
	m_crtc->set_display_callback(FUNC(octopus_state::display_pixels));
	m_crtc->set_addrmap(0, &octopus_state::octopus_vram);
	m_crtc->set_screen("screen");

	ADDRESS_MAP_BANK(config, m_z80_bankdev).set_map(&octopus_state::octopus_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x10000);

	RAM(config, "ram").set_default_size("256K").set_extra_options("128K,512K,768K");
}

/* ROM definition */
ROM_START( octopus )
	ROM_REGION( 0x4000, "user1", 0 )
	ROM_LOAD( "octopus_main_prom", 0x0000, 0x4000, CRC(b5b4518d) SHA1(41b8729c4c9074914fd4ea181c8b6d4805ee2b93) )

	// This rom was on the graphics card (yes, it has slots)
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "octopus_gfx_card",  0x0000, 0x2000, CRC(b2386534) SHA1(5e3c4682afb4eb222e48a7203269a16d26911836) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                 FULLNAME       FLAGS
COMP( 1986, octopus, 0,      0,      octopus, octopus, octopus_state, empty_init, "Digital Microsystems", "LSI Octopus", MACHINE_NOT_WORKING)
