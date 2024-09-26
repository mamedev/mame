// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pcxporter.cpp

    Implementation of the Applied Engineering PC Transporter card
    Preliminary version by R. Belmont, additional reverse-engineering by Peter Ferrie
    PC-XT portion adapted from genpc.cpp by Wilbert Pol and Miodrag Milanovic

    The PC Transporter is basically a PC-XT on an Apple II card.
    Features include:
    - V30 CPU @ 4.77 MHz
    - 768K of RAM, which defines the V30 address space from 0x00000 to 0xBFFFF
      and is fully read/writable by the Apple's CPU.
    - Usual XT hardware, mostly inside custom ASICs.  There's a discrete
      NEC uPD71054 (i8254-compatible PIT) though.
    - CGA-compatible video, output to a separate CGA monitor or NTSC-compliant analog
      RGB monitor (e.g. the IIgs RGB monitor).
    - XT-compatible keyboard interface.
    - PC-style floppy controller: supports 360K 5.25" disks and 720K 3.5" disks
    - HDD controller which is redirected to a file on the Apple's filesystem

    The V30 BIOS is downloaded by the Apple; the Apple also writes text to the CGA screen prior to
    the V30's being launched.

    The board was developed by The Engineering Department, a company made up mostly of early Apple
    engineers including Apple /// designer Dr. Wendall Sander and ProDOS creator Dick Huston.

    Software and user documentation at:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/CPU/AE%20PC%20Transporter/

    Notes:
        Registers live at CFxx; access CFFF to clear C800 reservation,
        then read Cn00 to map C800-CFFF first.

        PC RAM from 0xA0000-0xAFFFF is where the V30 BIOS is downloaded,
        plus used for general storage by the system.  This is mirrored at
        Fxxxx on the V30 so that it can boot.
        RAM from 0xB8000-0xBFFFF is the CGA framebuffer as usual.

        C800-CFFE: RAM / registers, locations as follows
        C828-C82A: bi-directional mailslots used to allow the PC to make ProDOS MLI calls,
                   likely for the HDD emulation (which uses a file on a ProDOS volume
                   as the PC).
                   $C828 = hi 8 bits of ptr to ProDOS call info, $C829 = middle 8 bits, $C82A = lower 8 bits
                   If bit 7 of $C828 is set, then the 6502 will take action.
        C832: current CGA mode index, used by 6502 @ $6869 to setup 6845, or 6845 reg index
        C833: 6845 data to write in the case where C832 is the reg index rather than a mode offset
        C860-C864: PC ports 60h-64h, used for keyboard comms
        CAC1: year for PC real-time clock
        CAC2: month for PC real-time clock
        CAC3: day for PC real-time clock
        CAC4: hour for PC real-time clock
        CAC5: minute for PC real-time clock
        CF00: PC memory pointer (bits 0-7)
        CF01: PC memory pointer (bits 8-15)
        CF02: PC memory pointer (bits 16-23)
        CF03: read/write PC memory at the pointer and increment the pointer
        CF04: read/write PC memory at the pointer and *don't* increment the pointer
        CF2C: CGA 6845 register select (port 3D0/3D2/3D4/3D6)
        CF2D: CGA 6845 data read/write (port 3D1/3D3/3D5/3D7)
        CF2E: CGA mode select (port 3D8)
        CF2F: CGA color select (port 3D9)
        CF30: control/flags: bit 4 = 1 to release reset on V30, 5 = 1 to release halt on V30
                             bit 7: read for card IRQ status, write 1 to clear/disable? card IRQ
        CF31: control/flags: bit 4 = 1 to assert reset on V30, 5 = 1 to assert halt on V30
               if bit 3 is set on an IRQ, the 6502 will force color 80x25 CGA text mode.
               bit 7: write 1 to enable card IRQ

    TODO:
        - Code at $70b0-$70c5 waits for the V30 to answer FPU presence.
        - What's going on at CF0E/CF0F?  One value set for normal operation, another during
          ProDOS calls.  Probably safe to ignore.
        - The manual indicates there is no ROM; special drivers installed into ProDOS 8
          provide the RAMdisk and A2-accessing-PC-drives functionality.

*********************************************************************/

#include "emu.h"
#include "pc_xporter.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/nec/nec.h"
#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"

#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_pcxporter_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_pcxporter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	[[maybe_unused]] uint16_t pc_bios_r(offs_t offset); // TODO: hook up to something?

protected:
	a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

private:
	required_device<v30_device> m_v30;
	required_device<pic8259_device>  m_pic8259;
	required_device<am9517a_device>  m_dma8237;
	required_device<pit8253_device>  m_pit8253;
	required_device<speaker_sound_device>  m_speaker;
	required_device<isa8_device>  m_isabus;
	optional_device<pc_kbdc_device>  m_pc_kbdc;

	uint8_t   m_u73_q2;
	uint8_t   m_out1;
	int m_dma_channel;
	uint8_t m_dma_offset[4];
	uint8_t m_pc_spkrdata;
	uint8_t m_pit_out2;
	bool m_cur_eop;

	uint8_t m_nmi_enabled;

	uint8_t m_ram[768*1024];
	uint8_t m_c800_ram[0x400];
	uint8_t m_regs[0x400];
	uint32_t m_offset;
	address_space *m_pcmem_space, *m_pcio_space;
	bool m_reset_during_halt;

	uint8_t m_6845_reg;

	// interface to the keyboard
	void keyboard_clock_w(int state);
	void keyboard_data_w(int state);

	void pc_pit8253_out1_changed(int state);
	void pc_pit8253_out2_changed(int state);

	void pc_dma_hrq_changed(int state);
	void pc_dma8237_out_eop(int state);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma8237_1_dack_r();
	uint8_t pc_dma8237_2_dack_r();
	uint8_t pc_dma8237_3_dack_r();
	void pc_dma8237_1_dack_w(uint8_t data);
	void pc_dma8237_2_dack_w(uint8_t data);
	void pc_dma8237_3_dack_w(uint8_t data);
	void pc_dma8237_0_dack_w(uint8_t data);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);

	uint8_t kbd_6502_r(offs_t offset);
	void kbd_6502_w(offs_t offset, uint8_t data);

	[[maybe_unused]] void pc_speaker_set_spkrdata(int state); // TODO: hook up to something?

	void pc_page_w(offs_t offset, uint8_t data);
	void nmi_enable_w(uint8_t data);
	[[maybe_unused]] void iochck_w(int state); // TODO: hook up to something?

	void pc_select_dma_channel(int channel, bool state);

	void pc_io(address_map &map) ATTR_COLD;
	void pc_map(address_map &map) ATTR_COLD;
};

void a2bus_pcxporter_device::pc_map(address_map &map)
{
	map.unmap_value_high();
}

void a2bus_pcxporter_device::pc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).rw("dma8237", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x002f).rw("pic8259", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x004f).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x0065).rw(FUNC(a2bus_pcxporter_device::kbd_6502_r), FUNC(a2bus_pcxporter_device::kbd_6502_w));
	map(0x0080, 0x008f).w(FUNC(a2bus_pcxporter_device::pc_page_w));
	map(0x00a0, 0x00a1).w(FUNC(a2bus_pcxporter_device::nmi_enable_w));
}

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_pcxporter_device::device_add_mconfig(machine_config &config)
{
	V30(config, m_v30, A2BUS_7M_CLOCK);    // 7.16 MHz as per manual
	m_v30->set_addrmap(AS_PROGRAM, &a2bus_pcxporter_device::pc_map);
	m_v30->set_addrmap(AS_IO, &a2bus_pcxporter_device::pc_io);
	m_v30->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));
	m_v30->set_disable();

	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>(A2BUS_7M_CLOCK / 6.0); // heartbeat IRQ
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->set_clk<1>(A2BUS_7M_CLOCK / 6.0); // DRAM refresh
	m_pit8253->out_handler<1>().set(FUNC(a2bus_pcxporter_device::pc_pit8253_out1_changed));
	m_pit8253->set_clk<2>(A2BUS_7M_CLOCK / 6.0); // PIO port C pin 4, and speaker polling enough
	m_pit8253->out_handler<2>().set(FUNC(a2bus_pcxporter_device::pc_pit8253_out2_changed));

	PCXPORT_DMAC(config, m_dma8237, A2BUS_7M_CLOCK / 2);
	m_dma8237->out_hreq_callback().set(FUNC(a2bus_pcxporter_device::pc_dma_hrq_changed));
	m_dma8237->out_eop_callback().set(FUNC(a2bus_pcxporter_device::pc_dma8237_out_eop));
	m_dma8237->in_memr_callback().set(FUNC(a2bus_pcxporter_device::pc_dma_read_byte));
	m_dma8237->out_memw_callback().set(FUNC(a2bus_pcxporter_device::pc_dma_write_byte));
	m_dma8237->in_ior_callback<1>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_1_dack_r));
	m_dma8237->in_ior_callback<2>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_2_dack_r));
	m_dma8237->in_ior_callback<3>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_3_dack_r));
	m_dma8237->out_iow_callback<0>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_0_dack_w));
	m_dma8237->out_iow_callback<1>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_1_dack_w));
	m_dma8237->out_iow_callback<2>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_2_dack_w));
	m_dma8237->out_iow_callback<3>().set(FUNC(a2bus_pcxporter_device::pc_dma8237_3_dack_w));
	m_dma8237->out_dack_callback<0>().set(FUNC(a2bus_pcxporter_device::pc_dack0_w));
	m_dma8237->out_dack_callback<1>().set(FUNC(a2bus_pcxporter_device::pc_dack1_w));
	m_dma8237->out_dack_callback<2>().set(FUNC(a2bus_pcxporter_device::pc_dack2_w));
	m_dma8237->out_dack_callback<3>().set(FUNC(a2bus_pcxporter_device::pc_dack3_w));

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set_inputline(m_v30, 0);

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace(m_v30, AS_PROGRAM);
	m_isabus->set_iospace(m_v30, AS_IO);
	m_isabus->irq2_callback().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_isabus->irq3_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set(m_dma8237, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237, FUNC(am9517a_device::dreq3_w));

	PC_KBDC(config, m_pc_kbdc, pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270);
	m_pc_kbdc->out_clock_cb().set(FUNC(a2bus_pcxporter_device::keyboard_clock_w));
	m_pc_kbdc->out_data_cb().set(FUNC(a2bus_pcxporter_device::keyboard_data_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	ISA8_SLOT(config, "isa1", 0, m_isabus, pc_isa8_cards, "cga", true); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, m_isabus, pc_isa8_cards, "fdc_xt", true);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_pcxporter_device::a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_v30(*this, "v30"),
	m_pic8259(*this, "pic8259"),
	m_dma8237(*this, "dma8237"),
	m_pit8253(*this, "pit8253"),
	m_speaker(*this, "speaker"),
	m_isabus(*this, "isa"),
	m_pc_kbdc(*this, "kbd")
{
}

a2bus_pcxporter_device::a2bus_pcxporter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_pcxporter_device(mconfig, A2BUS_PCXPORTER, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_pcxporter_device::device_start()
{
	memset(m_ram, 0, 768*1024);
	memset(m_regs, 0, 0x400);
	m_offset = 0;
	m_reset_during_halt = false;

	save_item(NAME(m_ram));
	save_item(NAME(m_regs));
	save_item(NAME(m_offset));

	m_v30->space(AS_PROGRAM).install_ram(0, 0xaffff, m_ram);
	m_v30->space(AS_PROGRAM).install_rom(0xf0000, 0xfffff, &m_ram[0xa0000]);

	m_pcmem_space = &m_v30->space(AS_PROGRAM);
	m_pcio_space = &m_v30->space(AS_IO);
}

void a2bus_pcxporter_device::device_reset()
{
	m_v30->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_reset_during_halt = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_pcxporter_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_pcxporter_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_pcxporter_device::read_cnxx(uint8_t offset)
{
	// read only to trigger C800?
	return 0xff;
}

void a2bus_pcxporter_device::write_cnxx(uint8_t offset, uint8_t data)
{
	logerror("Write %02x to cn%02x (%s)\n", data, offset, machine().describe_context());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_pcxporter_device::read_c800(uint16_t offset)
{
//  logerror("Read C800 at %x\n", offset + 0xc800);

	if (offset < 0x400)
	{
		return m_c800_ram[offset];
	}
	else
	{
		uint8_t rv;

		switch (offset)
		{
			case 0x700:
				return m_offset & 0xff;

			case 0x701:
				return (m_offset >> 8) & 0xff;

			case 0x702:
				return (m_offset >> 16) & 0xff;

			case 0x703: // read with increment
				rv = m_ram[m_offset];
				// don't increment if the debugger's reading
				if (!machine().side_effects_disabled())
				{
					m_offset++;
				}
				return rv;

			case 0x704: // read w/o increment
				rv = m_ram[m_offset];
				return rv;

			default:
				//logerror("Read $C800 at %x\n", offset + 0xc800);
				break;
		}

		return m_regs[offset & 0x3ff];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_pcxporter_device::write_c800(uint16_t offset, uint8_t data)
{
	if (offset < 0x400)
	{
		m_c800_ram[offset] = data;
	}
	else
	{
		switch (offset)
		{
			case 0x700:
				m_offset &= ~0xff;
				m_offset |= data;
//              logerror("offset now %x (%s)\n", m_offset, machine().describe_context());
				break;

			case 0x701:
				m_offset &= ~0xff00;
				m_offset |= (data<<8);
//              logerror("offset now %x (%s)\n", m_offset, machine().describe_context());
				break;

			case 0x702:
				m_offset &= ~0xff0000;
				m_offset |= (data<<16);
//              logerror("offset now %x (%s)\n", m_offset, machine().describe_context());
				break;

			case 0x703: // write w/increment
				m_ram[m_offset] = data;
				if (m_offset >= 0xb0000 && m_offset <= 0xb3fff) m_pcmem_space->write_byte(m_offset+0x8000, data);
				else if (m_offset >= 0xb4000 && m_offset <= 0xb7fff) m_pcmem_space->write_byte(m_offset+0x4000, data);
				else if (m_offset >= 0xb8000 && m_offset <= 0xbbfff) m_pcmem_space->write_byte(m_offset, data);
				else if (m_offset >= 0xbc000 && m_offset <= 0xbffff) m_pcmem_space->write_byte(m_offset-0x4000, data);
				m_offset++;
				break;

			case 0x704: // write w/o increment
				m_ram[m_offset] = data;
				if (m_offset >= 0xb0000 && m_offset <= 0xb3fff) m_pcmem_space->write_byte(m_offset+0x8000, data);
				else if (m_offset >= 0xb4000 && m_offset <= 0xb7fff) m_pcmem_space->write_byte(m_offset+0x4000, data);
				else if (m_offset >= 0xb8000 && m_offset <= 0xbbfff) m_pcmem_space->write_byte(m_offset, data);
				else if (m_offset >= 0xbc000 && m_offset <= 0xbffff) m_pcmem_space->write_byte(m_offset-0x4000, data);
				break;

			case 0x72c: // CGA 6845 register select
				m_pcio_space->write_byte(0x3d6, data);
				m_6845_reg = data;
				break;

			case 0x72d: // CGA 6845 data read/write
				// HACK: adjust the 40 column mode the 6502 sets to
				// be more within specs.
				switch (m_6845_reg)
				{
					case 0:
						if (data == 0x3e)
						{
							data -= 6;
						}
						break;

					case 2:
						if (data == 0x29)
						{
							data += 4;
						}
						break;

					case 3:
						if (data == 0x5)
						{
							data *= 2;
						}
						break;
				}

				m_pcio_space->write_byte(0x3d7, data);
				break;

			case 0x72e: // CGA mode select
				m_pcio_space->write_byte(0x3d8, data);
				break;

			case 0x72f: // CGA color select
				m_pcio_space->write_byte(0x3d9, data);
				break;

			case 0x730: // control 1
				if (data & 0x10) { m_v30->set_input_line(INPUT_LINE_RESET, CLEAR_LINE); m_reset_during_halt = true; }
				if (data & 0x20)
				{
					if (m_reset_during_halt)
					{
						m_v30->reset();
						m_reset_during_halt = false;
					}

					m_v30->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
					m_v30->resume(SUSPEND_REASON_HALT | SUSPEND_REASON_DISABLE);
				}
				break;

			case 0x731: // control 2
				if (data & 0x10) m_v30->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				if (data & 0x20) m_v30->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				break;

			default:
//              logerror("%02x to C800 at %x\n", data, offset + 0xc800);
				m_regs[offset & 0x3ff] = data;
				break;
		}
	}
}

uint16_t a2bus_pcxporter_device::pc_bios_r(offs_t offset)
{
	return m_ram[offset+0xa0000] | (m_ram[offset+0xa0001]<<8);
}

uint8_t a2bus_pcxporter_device::kbd_6502_r(offs_t offset)
{
	return m_c800_ram[offset+0x60];
}

void a2bus_pcxporter_device::kbd_6502_w(offs_t offset, uint8_t data)
{
	m_c800_ram[offset+0x60] = data;
}

/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

void a2bus_pcxporter_device::pc_page_w(offs_t offset, uint8_t data)
{
	switch(offset % 4)
	{
	case 1:
		m_dma_offset[2] = data;
		break;
	case 2:
		m_dma_offset[3] = data;
		break;
	case 3:
		m_dma_offset[0] = m_dma_offset[1] = data;
		break;
	}
}


void a2bus_pcxporter_device::pc_dma_hrq_changed(int state)
{
	m_v30->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237->hack_w(state);
}


uint8_t a2bus_pcxporter_device::pc_dma_read_byte(offs_t offset)
{
	if(m_dma_channel == -1)
		return 0xff;
	address_space &spaceio = m_v30->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;
	return spaceio.read_byte( page_offset + offset);
}


void a2bus_pcxporter_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	if(m_dma_channel == -1)
		return;
	address_space &spaceio = m_v30->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;

	spaceio.write_byte( page_offset + offset, data);
}


uint8_t a2bus_pcxporter_device::pc_dma8237_1_dack_r()
{
	return m_isabus->dack_r(1);
}

uint8_t a2bus_pcxporter_device::pc_dma8237_2_dack_r()
{
	return m_isabus->dack_r(2);
}


uint8_t a2bus_pcxporter_device::pc_dma8237_3_dack_r()
{
	return m_isabus->dack_r(3);
}


void a2bus_pcxporter_device::pc_dma8237_1_dack_w(uint8_t data)
{
	m_isabus->dack_w(1,data);
}

void a2bus_pcxporter_device::pc_dma8237_2_dack_w(uint8_t data)
{
	m_isabus->dack_w(2,data);
}


void a2bus_pcxporter_device::pc_dma8237_3_dack_w(uint8_t data)
{
	m_isabus->dack_w(3,data);
}


void a2bus_pcxporter_device::pc_dma8237_0_dack_w(uint8_t data)
{
	m_u73_q2 = 0;
	m_dma8237->dreq0_w( m_u73_q2 );
}


void a2bus_pcxporter_device::pc_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1 && m_cur_eop)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void a2bus_pcxporter_device::pc_select_dma_channel(int channel, bool state)
{
	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			m_isabus->eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			m_isabus->eop_w(channel, CLEAR_LINE );
	}
}

void a2bus_pcxporter_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void a2bus_pcxporter_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void a2bus_pcxporter_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void a2bus_pcxporter_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

void a2bus_pcxporter_device::pc_speaker_set_spkrdata(int state)
{
	m_pc_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}


/*************************************************************
 *
 * pit8253 configuration
 *
 *************************************************************/

void a2bus_pcxporter_device::pc_pit8253_out1_changed(int state)
{
	/* Trigger DMA channel #0 */
	if ( m_out1 == 0 && state == 1 && m_u73_q2 == 0 )
	{
		m_u73_q2 = 1;
		m_dma8237->dreq0_w( m_u73_q2 );
	}
	m_out1 = state;
}


void a2bus_pcxporter_device::pc_pit8253_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}


void a2bus_pcxporter_device::keyboard_clock_w(int state)
{
}


void a2bus_pcxporter_device::keyboard_data_w(int state)
{
}

/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

void a2bus_pcxporter_device::nmi_enable_w(uint8_t data)
{
	m_nmi_enabled = BIT(data,7);
	if (!m_nmi_enabled)
		m_v30->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void a2bus_pcxporter_device::iochck_w(int state)
{
	if (m_nmi_enabled && !state)
		m_v30->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_PCXPORTER, device_a2bus_card_interface, a2bus_pcxporter_device, "a2pcxport", "Applied Engineering PC Transporter")
