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
Text display: SCN2674 CRTC, SCB2675 for attributes
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
#include "machine/octo_kbd.h"
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
#include "softlist.h"
#include "speaker.h"


class octopus_state : public driver_device
{
public:
	octopus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_crtc(*this, "crtc"),
		m_vram(*this, "vram"),
		m_fontram(*this, "fram"),
		m_dma1(*this, "dma1"),
		m_dma2(*this, "dma2"),
		m_pic1(*this, "pic_master"),
		m_pic2(*this, "pic_slave"),
		m_rtc(*this, "rtc"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
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

	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	SCN2674_DRAW_CHARACTER_MEMBER(display_pixels);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);
	DECLARE_READ8_MEMBER(bank_sel_r);
	DECLARE_WRITE8_MEMBER(bank_sel_w);
	DECLARE_READ8_MEMBER(dma_read);
	DECLARE_WRITE8_MEMBER(dma_write);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_READ8_MEMBER(system_r);
	DECLARE_WRITE8_MEMBER(system_w);
	DECLARE_READ8_MEMBER(cntl_r);
	DECLARE_WRITE8_MEMBER(cntl_w);
	DECLARE_READ8_MEMBER(gpo_r);
	DECLARE_WRITE8_MEMBER(gpo_w);
	DECLARE_READ8_MEMBER(vidcontrol_r);
	DECLARE_WRITE8_MEMBER(vidcontrol_w);
	DECLARE_READ8_MEMBER(z80_io_r);
	DECLARE_WRITE8_MEMBER(z80_io_w);
	IRQ_CALLBACK_MEMBER(x86_irq_cb);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ8_MEMBER(z80_vector_r);
	DECLARE_WRITE8_MEMBER(z80_vector_w);
	DECLARE_READ8_MEMBER(parallel_r);
	DECLARE_WRITE8_MEMBER(parallel_w);
	DECLARE_READ8_MEMBER(video_latch_r);
	DECLARE_WRITE8_MEMBER(video_latch_w);

	DECLARE_WRITE_LINE_MEMBER(spk_w);
	DECLARE_WRITE_LINE_MEMBER(spk_freq_w);
	DECLARE_WRITE_LINE_MEMBER(beep_w);
	DECLARE_WRITE_LINE_MEMBER(serial_clock_w);
	DECLARE_WRITE_LINE_MEMBER(parallel_busy_w) { m_printer_busy = state; }
	DECLARE_WRITE_LINE_MEMBER(parallel_slctout_w) { m_printer_slctout = state; }

	DECLARE_WRITE_LINE_MEMBER(dack0_w) { m_dma1->hack_w(state ? 0 : 1); }  // for all unused DMA channel?
	DECLARE_WRITE_LINE_MEMBER(dack1_w) { if(!state) m_current_dma = 1; else if(m_current_dma == 1) m_current_dma = -1; }  // HD
	DECLARE_WRITE_LINE_MEMBER(dack2_w) { if(!state) m_current_dma = 2; else if(m_current_dma == 2) m_current_dma = -1; }  // RAM refresh
	DECLARE_WRITE_LINE_MEMBER(dack3_w) { m_dma1->hack_w(state ? 0 : 1); }
	DECLARE_WRITE_LINE_MEMBER(dack4_w) { m_dma1->hack_w(state ? 0 : 1); }
	DECLARE_WRITE_LINE_MEMBER(dack5_w) { if(!state) m_current_dma = 5; else if(m_current_dma == 5) m_current_dma = -1; }  // Floppy
	DECLARE_WRITE_LINE_MEMBER(dack6_w) { m_dma1->hack_w(state ? 0 : 1); }
	DECLARE_WRITE_LINE_MEMBER(dack7_w) { m_dma1->hack_w(state ? 0 : 1); }

	enum
	{
		BEEP_TIMER = 100
	};

	void octopus(machine_config &config);
	void octopus_io(address_map &map);
	void octopus_mem(address_map &map);
	void octopus_sub_io(address_map &map);
	void octopus_sub_mem(address_map &map);
	void octopus_vram(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<scn2674_device> m_crtc;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_fontram;
	required_device<am9517a_device> m_dma1;
	required_device<am9517a_device> m_dma2;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<mc146818_device> m_rtc;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
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


ADDRESS_MAP_START(octopus_state::octopus_mem)
	AM_RANGE(0x00000, 0xcffff) AM_RAMBANK("main_ram_bank")
	AM_RANGE(0xd0000, 0xdffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xe0000, 0xe3fff) AM_NOP
	AM_RANGE(0xe4000, 0xe5fff) AM_RAM AM_SHARE("fram")
	AM_RANGE(0xe6000, 0xe7fff) AM_ROM AM_REGION("chargen",0)
	AM_RANGE(0xe8000, 0xfbfff) AM_NOP
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

ADDRESS_MAP_START(octopus_state::octopus_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x0f) AM_DEVREADWRITE("dma1", am9517a_device, read, write)
	AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE("dma2", am9517a_device, read, write)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSWA")
	AM_RANGE(0x21, 0x2f) AM_READWRITE(system_r, system_w)
	AM_RANGE(0x31, 0x33) AM_READWRITE(bank_sel_r, bank_sel_w)
	AM_RANGE(0x50, 0x50) AM_DEVREADWRITE("keyboard", i8251_device, data_r, data_w)
	AM_RANGE(0x51, 0x51) AM_DEVREADWRITE("keyboard", i8251_device, status_r, control_w)
	// 0x70-73: HD controller
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("serial", z80sio_device, da_r, da_w)
	AM_RANGE(0xa1, 0xa1) AM_DEVREADWRITE("serial", z80sio_device, ca_r, ca_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREADWRITE("serial", z80sio_device, db_r, db_w)
	AM_RANGE(0xa3, 0xa3) AM_DEVREADWRITE("serial", z80sio_device, cb_r, cb_w)
	AM_RANGE(0xb0, 0xb1) AM_DEVREADWRITE("pic_master", pic8259_device, read, write)
	AM_RANGE(0xb4, 0xb5) AM_DEVREADWRITE("pic_slave", pic8259_device, read, write)
	AM_RANGE(0xc0, 0xc7) AM_DEVREADWRITE("crtc", scn2674_device, read, write)
	AM_RANGE(0xc8, 0xc8) AM_READWRITE(vidcontrol_r, vidcontrol_w)
	AM_RANGE(0xc9, 0xca) AM_READWRITE(video_latch_r, video_latch_w)
	// 0xcf: mode control
	AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE("fdc", fd1793_device, read, write)
	AM_RANGE(0xe0, 0xe4) AM_READWRITE(z80_vector_r, z80_vector_w)
	AM_RANGE(0xf0, 0xf1) AM_READWRITE(parallel_r, parallel_w)
	AM_RANGE(0xf8, 0xff) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END


ADDRESS_MAP_START(octopus_state::octopus_sub_mem)
	AM_RANGE(0x0000, 0xffff) AM_DEVREADWRITE("z80_bank", address_map_bank_device, read8, write8)
ADDRESS_MAP_END

ADDRESS_MAP_START(octopus_state::octopus_sub_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(z80_io_r, z80_io_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(octopus_state::octopus_vram)
	AM_RANGE(0x0000,0xffff) AM_READWRITE(vram_r, vram_w)
ADDRESS_MAP_END

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

void octopus_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case BEEP_TIMER:  // switch off speaker
		m_beep_active = false;
		break;
	}
}

WRITE8_MEMBER(octopus_state::vram_w)
{
	m_vram[offset] = m_char_latch_w;
	m_vram[offset+0x1000] = m_attr_latch_w;
}

READ8_MEMBER(octopus_state::vram_r)
{
	m_char_latch_r = m_vram[offset];
	m_attr_latch_r = m_vram[offset+0x1000];
	return m_vram[offset];
}

WRITE_LINE_MEMBER(octopus_state::fdc_drq)
{
	// TODO
}

READ8_MEMBER(octopus_state::bank_sel_r)
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

WRITE8_MEMBER(octopus_state::bank_sel_w)
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
WRITE8_MEMBER(octopus_state::system_w)
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

READ8_MEMBER(octopus_state::system_r)
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
READ8_MEMBER(octopus_state::z80_io_r)
{
	z80_io_w(space,offset,0);
	return 0x00;
}

WRITE8_MEMBER(octopus_state::z80_io_w)
{
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_z80_active = false;
}

// Z80 vector for RS232 and RS422
READ8_MEMBER(octopus_state::z80_vector_r)
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

WRITE8_MEMBER(octopus_state::z80_vector_w)
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
READ8_MEMBER(octopus_state::rtc_r)
{
	uint8_t ret = 0xff;

	if(m_rtc_data)
		ret = m_rtc->read(space,1);
	else if(m_rtc_address)
		ret = m_rtc->read(space,0);

	return ret;
}

WRITE8_MEMBER(octopus_state::rtc_w)
{
	if(m_rtc_data)
		m_rtc->write(space,1,data);
	else if(m_rtc_address)
		m_rtc->write(space,0,data);
}

// RTC/FDC control - PPI port B
// bits0-3: RTC control lines
// bit4-5: write precomp.
// bit6-7: drive select
READ8_MEMBER(octopus_state::cntl_r)
{
	return m_cntl;
}

WRITE8_MEMBER(octopus_state::cntl_w)
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
		m_fdc->set_floppy(m_floppy0->get_device());
		m_floppy0->get_device()->mon_w(0);
		break;
	case 2:
		m_fdc->set_floppy(m_floppy1->get_device());
		m_floppy1->get_device()->mon_w(0);
		break;
	}
	logerror("Selected floppy drive %i (%02x)\n",m_current_drive,data);
}

// General Purpose Outputs - PPI port C
// bit 2 - floppy side select
// bit 1 - parallel data I/O (0 = output)
// bit 0 - parallel control I/O (0 = output)
READ8_MEMBER(octopus_state::gpo_r)
{
	return m_gpo;
}

WRITE8_MEMBER(octopus_state::gpo_w)
{
	m_gpo = data;
	switch(m_current_drive)
	{
	case 1:
		m_floppy0->get_device()->ss_w((data & 0x04) >> 2);
		break;
	case 2:
		m_floppy1->get_device()->ss_w((data & 0x04) >> 2);
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
READ8_MEMBER(octopus_state::vidcontrol_r)
{
	return m_vidctrl;
}

WRITE8_MEMBER(octopus_state::vidcontrol_w)
{
	m_vidctrl = data;
	m_fdc->dden_w(data & 0x04);
	m_fdc->set_unscaled_clock((data & 0x08) ? XTAL(16'000'000) / 16 : XTAL(16'000'000) / 8);
}

// Sound hardware
// Sound level provided by i8253 timer 2
// Enabled by /DTR signal from i8251
// 100ms beep triggered by pulsing /CTS signal low on i8251
WRITE_LINE_MEMBER(octopus_state::spk_w)
{
	m_speaker_active = !state;
	m_speaker->level_w(((m_speaker_active || m_beep_active) && m_speaker_level) ? 1 : 0);
}

WRITE_LINE_MEMBER(octopus_state::spk_freq_w)
{
	m_speaker_level = state;
	m_speaker->level_w(((m_speaker_active || m_beep_active) && m_speaker_level) ? 1 : 0);
}

WRITE_LINE_MEMBER(octopus_state::beep_w)
{
	if(!state)  // active low
	{
		m_beep_active = true;
		m_speaker->level_w(((m_speaker_active || m_beep_active) && m_speaker_level) ? 1 : 0);
		m_timer_beep->adjust(attotime::from_msec(100));
	}
}

WRITE_LINE_MEMBER(octopus_state::serial_clock_w)
{
	m_serial->rxca_w(state);
	m_serial->txca_w(state);
}

// Parallel Centronics port
// 0xf0 : data
// 0xf1 : control
//      bit 2 = INIT?  On boot, bits 0 and 1 are set high, bit 2 is set low then high again, all other bits are set low
// can generate interrupts - tech manual suggests that Strobe, Init, Ack, and Busy can trigger an interrupt (IRQ14)
READ8_MEMBER(octopus_state::parallel_r)
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

WRITE8_MEMBER(octopus_state::parallel_w)
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

READ8_MEMBER(octopus_state::dma_read)
{
	uint8_t byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_current_dma == -1)
		return 0;
	byte = prog_space.read_byte((m_fd_bank << 16) + offset);
	return byte;
}

WRITE8_MEMBER(octopus_state::dma_write)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_current_dma == -1)
		return;
	prog_space.write_byte((m_fd_bank << 16) + offset, data);
}

WRITE_LINE_MEMBER( octopus_state::dma_hrq_changed )
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
	m_timer_beep = timer_alloc(BEEP_TIMER);

	// install RAM
	m_maincpu->space(AS_PROGRAM).install_readwrite_bank(0x0000,m_ram->size()-1,"main_ram_bank");
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
	membank("main_ram_bank")->set_base(m_ram->pointer());
	m_kb_uart->write_dsr(1);  // DSR is used to determine if a keyboard is connected?  If DSR is high, then the CHAR_OUT BIOS function will not output to the screen.
}

void octopus_state::video_start()
{
	m_vram.allocate(0x10000);
}

READ8_MEMBER(octopus_state::video_latch_r)
{
	if(offset & 0x01)
		return m_attr_latch_r;
	else
		return m_char_latch_r;
}

WRITE8_MEMBER(octopus_state::video_latch_w)
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
			bitmap.pix32(y,x + z) = BIT(data,z) ? fg : bg;
	}
}

READ8_MEMBER( octopus_state::get_slave_ack )
{
	if (offset==7)
		return m_pic2->acknowledge();

	return 0x00;
}

static SLOT_INTERFACE_START( octopus_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(keyboard)
	SLOT_INTERFACE("octopus", OCTOPUS_KEYBOARD)
SLOT_INTERFACE_END

SLOT_INTERFACE_START(octopus_centronics_devices)
	SLOT_INTERFACE("pl80", COMX_PL80)
	SLOT_INTERFACE("ex800", EPSON_EX800)
	SLOT_INTERFACE("lx800", EPSON_LX800)
	SLOT_INTERFACE("lx810l", EPSON_LX810L)
	SLOT_INTERFACE("ap2000", EPSON_AP2000)
	SLOT_INTERFACE("printer", CENTRONICS_PRINTER)
SLOT_INTERFACE_END

MACHINE_CONFIG_START(octopus_state::octopus)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8088, XTAL(24'000'000) / 3)  // 8MHz
	MCFG_CPU_PROGRAM_MAP(octopus_mem)
	MCFG_CPU_IO_MAP(octopus_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(octopus_state, x86_irq_cb)

	MCFG_CPU_ADD("subcpu",Z80, XTAL(24'000'000) / 4) // 6MHz
	MCFG_CPU_PROGRAM_MAP(octopus_sub_mem)
	MCFG_CPU_IO_MAP(octopus_sub_io)

	MCFG_DEVICE_ADD("dma1", AM9517A, XTAL(24'000'000) / 6)  // 4MHz
	MCFG_I8237_OUT_HREQ_CB(DEVWRITELINE("dma2", am9517a_device, dreq0_w))
	MCFG_I8237_IN_MEMR_CB(READ8(octopus_state,dma_read))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(octopus_state,dma_write))
	//MCFG_I8237_IN_IOR_0_CB(NOOP)
	//MCFG_I8237_IN_IOR_1_CB(NOOP)  // HDC
	//MCFG_I8237_IN_IOR_2_CB(NOOP)  // RAM Refresh
	//MCFG_I8237_IN_IOR_3_CB(NOOP)
	//MCFG_I8237_OUT_IOW_0_CB(NOOP)
	//MCFG_I8237_OUT_IOW_1_CB(NOOP)  // HDC
	//MCFG_I8237_OUT_IOW_2_CB(NOOP)  // RAM Refresh
	//MCFG_I8237_OUT_IOW_3_CB(NOOP)
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(octopus_state, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(octopus_state, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(octopus_state, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(octopus_state, dack3_w))
	MCFG_DEVICE_ADD("dma2", AM9517A, XTAL(24'000'000) / 6)  // 4MHz
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(octopus_state, dma_hrq_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(octopus_state,dma_read))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(octopus_state,dma_write))
	//MCFG_I8237_IN_IOR_0_CB(NOOP)
	MCFG_I8237_IN_IOR_1_CB(DEVREAD8("fdc",fd1793_device,data_r))  // FDC
	//MCFG_I8237_IN_IOR_2_CB(NOOP)
	//MCFG_I8237_IN_IOR_3_CB(NOOP)
	//MCFG_I8237_OUT_IOW_0_CB(NOOP)
	MCFG_I8237_OUT_IOW_1_CB(DEVWRITE8("fdc",fd1793_device,data_w))  // FDC
	//MCFG_I8237_OUT_IOW_2_CB(NOOP)
	//MCFG_I8237_OUT_IOW_3_CB(NOOP)
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(octopus_state, dack4_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(octopus_state, dack5_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(octopus_state, dack6_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(octopus_state, dack7_w))

	MCFG_DEVICE_ADD("pic_master", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", 0))
	MCFG_PIC8259_IN_SP_CB(VCC)
	MCFG_PIC8259_CASCADE_ACK_CB(READ8(octopus_state, get_slave_ack))

	MCFG_DEVICE_ADD("pic_slave", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(DEVWRITELINE("pic_master", pic8259_device, ir7_w))
	MCFG_PIC8259_IN_SP_CB(GND)

	// RTC (MC146818 via i8255 PPI)
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(octopus_state,rtc_r))
	MCFG_I8255_IN_PORTB_CB(READ8(octopus_state,cntl_r))
	MCFG_I8255_IN_PORTC_CB(READ8(octopus_state,gpo_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(octopus_state,rtc_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(octopus_state,cntl_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(octopus_state,gpo_w))
	MCFG_MC146818_ADD("rtc", XTAL(32'768))
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic_slave",pic8259_device, ir2_w)) MCFG_DEVCB_INVERT

	// Keyboard UART
	MCFG_DEVICE_ADD("keyboard", I8251, 0)
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic_slave",pic8259_device, ir4_w))
	MCFG_I8251_DTR_HANDLER(WRITELINE(octopus_state,spk_w))
	MCFG_I8251_RTS_HANDLER(WRITELINE(octopus_state,beep_w))
	MCFG_RS232_PORT_ADD("keyboard_port", keyboard, "octopus")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("keyboard", i8251_device, write_rxd))
	MCFG_DEVICE_ADD("keyboard_clock_rx", CLOCK, 9600 * 64)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("keyboard",i8251_device,write_rxc))
	MCFG_DEVICE_ADD("keyboard_clock_tx", CLOCK, 1200 * 64)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("keyboard",i8251_device,write_txc))

	MCFG_FD1793_ADD("fdc",XTAL(16'000'000) / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic_master",pic8259_device, ir5_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("dma2",am9517a_device, dreq1_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", octopus_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", octopus_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("fd_list","octopus")

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(2457500)  // DART channel A
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(octopus_state,serial_clock_w))  // being able to write both Rx and Tx clocks at one time would be nice
	MCFG_PIT8253_CLK1(2457500)  // DART channel B
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("serial",z80sio_device,rxtxcb_w))
	MCFG_PIT8253_CLK2(2457500)  // speaker frequency
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(octopus_state,spk_freq_w))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("serial", Z80SIO, XTAL(16'000'000) / 4) // clock rate not mentioned in tech manual
	MCFG_Z80SIO_OUT_INT_CB(DEVWRITELINE("pic_master",pic8259_device, ir1_w))
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("serial_a",rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE("serial_b",rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("serial_a",rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_RTSB_CB(DEVWRITELINE("serial_b",rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("serial_a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("serial",z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("serial",z80sio_device, ctsa_w)) MCFG_DEVCB_INVERT
	//MCFG_RS232_RI_HANDLER(DEVWRITELINE("serial",z80sio_device, ria_w)) MCFG_DEVCB_INVERT
	MCFG_RS232_PORT_ADD("serial_b", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("serial",z80sio_device, rxb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("serial",z80sio_device, ctsb_w)) MCFG_DEVCB_INVERT
	//MCFG_RS232_RI_HANDLER(DEVWRITELINE("serial",z80sio_device, rib_w)) MCFG_DEVCB_INVERT

	MCFG_CENTRONICS_ADD("parallel", octopus_centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(octopus_state, parallel_busy_w))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(octopus_state, parallel_slctout_w))
	// TODO: Winchester HD controller (Xebec/SASI compatible? uses TTL logic)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(720, 360)
	MCFG_SCREEN_VISIBLE_AREA(0, 720-1, 0, 360-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc",scn2674_device, screen_update)
//  MCFG_SCREEN_PALETTE("palette")
//  MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("crtc", SCN2674, 0)  // character clock can be selectable, either 16MHz or 17.6MHz
	MCFG_SCN2674_INTR_CALLBACK(DEVWRITELINE("pic_slave", pic8259_device, ir0_w))
	MCFG_SCN2674_TEXT_CHARACTER_WIDTH(8)
	MCFG_SCN2674_GFX_CHARACTER_WIDTH(8)
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(octopus_state, display_pixels)
	MCFG_DEVICE_ADDRESS_MAP(0, octopus_vram)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("z80_bank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(octopus_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("128K,512K,768K")

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( octopus )
	ROM_REGION( 0x4000, "user1", 0 )
	ROM_LOAD( "octopus_main_prom", 0x0000, 0x4000, CRC(b5b4518d) SHA1(41b8729c4c9074914fd4ea181c8b6d4805ee2b93) )

	// This rom was on the graphics card (yes, it has slots)
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "octopus_gfx_card",  0x0000, 0x2000, CRC(b2386534) SHA1(5e3c4682afb4eb222e48a7203269a16d26911836) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    STATE          INIT  COMPANY                 FULLNAME       FLAGS
COMP( 1986, octopus,  0,      0,       octopus,   octopus, octopus_state, 0,    "Digital Microsystems", "LSI Octopus", MACHINE_NOT_WORKING)
