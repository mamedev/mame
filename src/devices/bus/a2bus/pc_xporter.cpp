// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pc_xporter.cpp

    Implementation of the Applied Engineering PC Transporter card
    Emulation by R. Belmont, additional reverse-engineering by Peter Ferrie
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

        C800-CBFF on the A2 side are I/O ports 0h to 3FFh on the V30 side.

        C800-CFFE: RAM / registers, locations as follows
        C822-C827: mapped to V30 I/O ports 22h-27h
            22h - operation (2=read, 3=write, 4=verify)
            23h - dispatch/status
            24h - drive number (must be zero on existing h/w, for future expansion?)
            25h / 26h - offset into MSDOSVOL file on the ProDOS volume
            27h - number of sectors to read/write
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
        CF05: bank at $CC00
        CF06: page at $CC00
        CF0E: bank at $CE00
        CF0F: page at $CE00
        CF20-CF27: the TransDrive floppy controller, an Apple ISM (the MFM half of what became
               the SWIM), registers in ISM order:
               CF20 data, CF21 mark, CF22 error (read) / CRC (write), CF23 parameter RAM,
               CF24 phases, CF25 setup, CF26 mode write-zeroes,
               CF27 mode write-ones (write) / handshake (read)
               Mode bits: 0 clear FIFO, 1 drive 1 enable, 2 drive 2 enable, 3 Action,
               4 0=read/1=write, 5 side select, 6 reserved (always reads 1), 7 MOTORON.
               The chip is clocked at 16 MHz: the driver's cell-time parameters work out to
               exactly 4 us minimum cells and a 2 us half cell, i.e. 250 kbps MFM.
               On a TransDrive PHASE 0 is DIRECTION and PHASE 1 is STEP (outputs) and PHASE 2
               is TRACK 0 (input, low when there), and the handshake's SENSE bit is the drive's
               /WRITE PROTECT; an Apple 3.5 Drive on the same port is driven through those four
               lines the Apple way instead, with SENSE answering its status registers.
        CF28: Unknown device
        CF29: 16-bit read/write length?
        CF2A: 16-bit read/write ???
        CF2B: write only?
        CF2C: CGA 6845 register select (port 3D0/3D2/3D4/3D6)
        CF2D: CGA 6845 data read/write (port 3D1/3D3/3D5/3D7)
        CF2E: CGA mode select (port 3D8)
        CF2F: CGA color select (port 3D9)
        CF30/CF31: control register clear/set pair.  Writing a 1 to a bit of CF30 clears it,
               writing a 1 to a bit of CF31 sets it; reading CF30 gives the current value.
               (6502 code at $46D7-$46F7 and $6064 makes the pairing explicit.)
               bit 4 = V30 RESET, bit 5 = V30 HALT, bit 6 = V30 INT request,
               bit 7 = card IRQ to the 6502 enabled
               if bit 3 is set on an IRQ, the 6502 will force color 80x25 CGA text mode.
               CF31 reads back transfer status instead: bit 6: 0=PC did 8-bit access,
               1=PC did 16-bit access; bit 7: 1=PC read, 0=PC write
               The vector for a CF31 bit 6 interrupt is the byte the 6502 leaves at PC RAM
               offset 0xBFFFE (6502 code at $4A40-$4A61 points the $CE00 window there).
        CF32: control/flags: bit 7: read = 1 if PC data is pending, write 1 to bit 7 to tell PC transfer is complete
        CF33: r/w, seems related to CF30/CF31
        CF34/CF35: I/O address accessed on the PC side
        CF36: Apple modifier keys, for hosts with no $C025 of their own: bit 0 = control and
               bit 1 = shift (both active low), bit 2 = Open Apple, bit 3 = Solid Apple,
               bit 7 = caps lock (active low).  On a IIe the card is cabled to the keyboard
               for these; the 6502 driver picks this or $C025 at $3222.
        CF37: write, some sort of device select

    TODO:
        - Code at $70b0-$70c5 waits for the V30 to answer FPU presence.
        - The manual indicates there is no ROM; special drivers installed into ProDOS 8
          provide the RAMdisk and A2-accessing-PC-drives functionality.

*********************************************************************/

#include "emu.h"
#include "pc_xporter.h"

#include "bus/isa/cga.h"
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
#include "machine/swim1.h"
#include "sound/spkrdev.h"

#include "softlist_dev.h"
#include "speaker.h"

#define LOG_PORTS   (1U << 1)     // V30 I/O accesses trapped for the Apple II to service
#define LOG_IRQ     (1U << 2)     // mailbox handshake with the Apple II

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPORTS(...)   LOGMASKED(LOG_PORTS, __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ, __VA_ARGS__)

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

protected:
	a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;
	virtual bool take_c800() const override { return true; }

private:
	required_device<v30_device> m_v30;
	required_device<pic8259_device>  m_pic8259;
	required_device<am9517a_device>  m_dma8237;
	required_device<pit8253_device>  m_pit8253;
	required_device<speaker_sound_device>  m_speaker;
	required_device<isa8_device>  m_isabus;
	optional_device<pc_kbdc_device>  m_pc_kbdc;
	required_device<swim1_device> m_ism;
	required_device_array<floppy_connector, 2> m_transdrive;
	optional_ioport m_host_kbspecial;

	uint8_t m_u73_q2;
	uint8_t m_out1;
	int m_dma_channel;
	uint8_t m_dma_offset[4];
	uint8_t m_pc_spkrdata;
	uint8_t m_pit_out2;
	bool m_cur_eop;

	static constexpr uint32_t RAM_SIZE = 768 * 1024;
	// per-port routing nibbles, at the top of the card's RAM along with the interrupt vector
	static constexpr uint32_t PORT_ROUTING_TABLE = 0xbf400;

	// the Apple II side can aim the windows anywhere in a 24-bit space, but only the
	// card's 768K decodes
	uint8_t ram_r(uint32_t addr) const { return (addr < RAM_SIZE) ? m_ram[addr] : 0xff; }
	void ram_w(uint32_t addr, uint8_t data) { if (addr < RAM_SIZE) m_ram[addr] = data; }

	uint8_t m_ram[RAM_SIZE];
	uint8_t m_c800_ram[0x400];
	uint8_t m_regs[0x400];
	uint8_t m_cpu_status;
	bool m_irq_enable;
	uint32_t m_offset, m_cc00_offset, m_ce00_offset;
	address_space *m_pcmem_space, *m_pcio_space;
	bool m_reset_during_halt;

	uint8_t m_pc_data_pending, m_pc_xfer_complete;
	uint16_t m_pc_in_restart;
	bool m_apple_reading_io, m_apple_writing_io;
	bool m_pc_io_stalled;
	uint8_t m_control;

	// the TransDrive controller: see ism_phases_w for how the drives hang off it
	floppy_image_device *m_cur_floppy;
	uint8_t m_ism_phases;
	int m_ism_hdsel;

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

	void pc_select_dma_channel(int channel, bool state);

	void pc_io(address_map &map);
	void pc_map(address_map &map);

	u16 x86_port_r(offs_t offset, u16 mem_mask);
	void x86_port_w(offs_t offset, u16 data, u16 mem_mask);
	void stall_v30(offs_t port, bool is_read, bool is_16bit, bool supply = false);
	uint8_t port_routing(offs_t port) const;
	void update_irq();
	IRQ_CALLBACK_MEMBER(v30_int_ack);

	// the TransDrive floppy controller at $CF20-$CF27
	uint8_t ism_r(uint16_t offset);
	void ism_w(uint16_t offset, uint8_t data);
	void ism_phases_w(uint8_t phases);
	void ism_devsel_w(uint8_t devsel);
	void ism_hdsel_w(int state);
	static void transdrive_formats(format_registration &fr);
	static void transdrives(device_slot_interface &device);
};

void a2bus_pcxporter_device::pc_map(address_map &map)
{
	map.unmap_value_high();
}

void a2bus_pcxporter_device::pc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rw(FUNC(a2bus_pcxporter_device::x86_port_r), FUNC(a2bus_pcxporter_device::x86_port_w));
}

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// The CGA is on the card, not in a slot, and it is a variant: see isa8_cga_pcxport_device.
void pcxport_isa8_cards(device_slot_interface &device)
{
	device.option_add_internal("cga", ISA8_CGA_PCXPORT);
}

// The TransDrives are PC drives on the end of an Apple controller, so they take PC disks: the
// 5.25" unit is a 360K drive and the 3.5" one a 720K drive.  The two are wired identically and
// the driver makes no distinction beyond the geometry, so both go in either position.
void a2bus_pcxporter_device::transdrives(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void a2bus_pcxporter_device::transdrive_formats(format_registration &fr)
{
	fr.add_pc_formats();
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_pcxporter_device::device_add_mconfig(machine_config &config)
{
	V30(config, m_v30, DERIVED_CLOCK(1, 1)); // 7.16 MHz as per manual
	m_v30->set_addrmap(AS_PROGRAM, &a2bus_pcxporter_device::pc_map);
	m_v30->set_addrmap(AS_IO, &a2bus_pcxporter_device::pc_io);
	// the 6502 owns the interrupt controller, so it also supplies the vector
	m_v30->set_irq_acknowledge_callback(FUNC(a2bus_pcxporter_device::v30_int_ack));
	m_v30->set_disable();

	PIT8253(config, m_pit8253);
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->out_handler<1>().set(FUNC(a2bus_pcxporter_device::pc_pit8253_out1_changed));
	m_pit8253->out_handler<2>().set(FUNC(a2bus_pcxporter_device::pc_pit8253_out2_changed));

	PCXPORT_DMAC(config, m_dma8237, DERIVED_CLOCK(1, 2));
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

	// The 8259 is only here for the ISA bus to hang its IRQ lines off: the 6502 models the
	// real interrupt controller in software and pokes the V30's INT line through $CF30/$CF31.
	PIC8259(config, m_pic8259);

	ISA8(config, m_isabus);
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

	ISA8_SLOT(config, "isa1", 0, m_isabus, pcxport_isa8_cards, "cga", true); // FIXME: determine ISA bus clock

	// The TransDrive controller is an ISM (Integrated Sander Machine), or 1/2 of
	// the SWIM1 (mostly).  We use a SWIM1 and force it into ISM mode and it works fine.
	SWIM1(config, m_ism, 16_MHz_XTAL);
	m_ism->devsel_cb().set(FUNC(a2bus_pcxporter_device::ism_devsel_w));
	m_ism->phases_cb().set(FUNC(a2bus_pcxporter_device::ism_phases_w));
	m_ism->hdsel_cb().set(FUNC(a2bus_pcxporter_device::ism_hdsel_w));

	FLOPPY_CONNECTOR(config, m_transdrive[0], transdrives, "525dd", a2bus_pcxporter_device::transdrive_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_transdrive[1], transdrives, "35dd", a2bus_pcxporter_device::transdrive_formats).enable_sound(true);

	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
}

void a2bus_pcxporter_device::device_resolve_objects()
{
	// DERIVED_CLOCK doesn't work for this case, so do this here instead
	m_pit8253->set_clk<0>(clock() / 6.0); // heartbeat IRQ
	m_pit8253->set_clk<1>(clock() / 6.0); // DRAM refresh
	m_pit8253->set_clk<2>(clock() / 6.0); // PIO port C pin 4, and speaker polling enough
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
	m_pc_kbdc(*this, "kbd"),
	m_ism(*this, "ism"),
	m_transdrive(*this, "ism:%u", 0U),
	m_host_kbspecial(*this, ":keyb_special"),
	m_cpu_status(0),
	m_irq_enable(false),
	m_offset(0), m_cc00_offset(0), m_ce00_offset(0),
	m_pc_data_pending(0), m_pc_xfer_complete(0),
	m_pc_in_restart(0xffff),
	m_apple_reading_io(false),
	m_apple_writing_io(false),
	m_pc_io_stalled(false),
	m_control(0),
	m_cur_floppy(nullptr),
	m_ism_phases(0),
	m_ism_hdsel(0)
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
	memset(m_ram, 0, RAM_SIZE);
	memset(m_regs, 0, 0x400);
	m_offset = 0;
	m_reset_during_halt = false;

	save_item(NAME(m_ram));
	save_item(NAME(m_c800_ram));
	save_item(NAME(m_regs));
	save_item(NAME(m_offset));
	save_item(NAME(m_cc00_offset));
	save_item(NAME(m_ce00_offset));
	save_item(NAME(m_cpu_status));
	save_item(NAME(m_control));
	save_item(NAME(m_ism_phases));
	save_item(NAME(m_ism_hdsel));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_pc_data_pending));
	save_item(NAME(m_pc_xfer_complete));
	save_item(NAME(m_pc_in_restart));
	save_item(NAME(m_pc_io_stalled));
	save_item(NAME(m_reset_during_halt));

	m_v30->space(AS_PROGRAM).install_ram(0, 0xaffff, m_ram);
	m_v30->space(AS_PROGRAM).install_rom(0xf0000, 0xfffff, &m_ram[0xa0000]);

	m_pcmem_space = &m_v30->space(AS_PROGRAM);
	m_pcio_space = &m_v30->space(AS_IO);
}

void a2bus_pcxporter_device::device_reset()
{
	m_v30->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_reset_during_halt = false;
	m_pc_io_stalled = false;
	m_pc_in_restart = 0xffff;
	m_pc_data_pending = 0;
	m_pc_xfer_complete = 0;
	m_control = 0;
	m_irq_enable = false;
	m_cur_floppy = nullptr;
	m_ism_phases = 0;
	m_ism_hdsel = 0;
	lower_slot_irq();
}

void a2bus_pcxporter_device::device_reset_after_children()
{
	// The PCT software assumes ISM mode, but SWIM1 resets into IWM back-compatibility.
	// So do the "door knock" sequence to switch it into ISM mode.
	m_ism->write(0x0f, 0x40);
	m_ism->write(0x0f, 0x00);
	m_ism->write(0x0f, 0x40);
	m_ism->write(0x0f, 0x40);
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

// DMA helpers
void a2bus_pcxporter_device::pc_dma_hrq_changed(int state)
{
	m_v30->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	m_dma8237->hack_w(state);
}


uint8_t a2bus_pcxporter_device::pc_dma_read_byte(offs_t offset)
{
	if(m_dma_channel == -1)
	{
		return 0xff;
	}

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
	{
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
	}
}

void a2bus_pcxporter_device::pc_select_dma_channel(int channel, bool state)
{
	if(!state)
	{
		m_dma_channel = channel;
		if(m_cur_eop)
		{
			m_isabus->eop_w(channel, ASSERT_LINE );
		}
	}
	else if(m_dma_channel == channel)
	{
		m_dma_channel = -1;
		if(m_cur_eop)
		{
			m_isabus->eop_w(channel, CLEAR_LINE );
		}
	}
}

void a2bus_pcxporter_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void a2bus_pcxporter_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void a2bus_pcxporter_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void a2bus_pcxporter_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }

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

uint8_t a2bus_pcxporter_device::ism_r(uint16_t offset)
{
	switch (offset & 7)
	{
	case 4: // phases
		{
			// The phase lines are bidirectional and a TransDrive sends TRACK 0 back on PHASE 2,
			// which is not an external input on the SWIM1 version (the capability of interfacing
			// to PC-style drives was lost there).
			uint8_t const phases = m_ism_phases & ~0x04;
			return phases | ((!m_cur_floppy || m_cur_floppy->trk00_r()) ? 0x04 : 0x00);
		}

	case 7: // handshake
		{
			uint8_t const handshake = m_ism->read(7) & ~0x08;
			return handshake | ((m_cur_floppy && !m_cur_floppy->wpt_r()) ? 0x08 : 0x00);
		}
	}

	return m_ism->read(offset & 7);
}

void a2bus_pcxporter_device::ism_w(uint16_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 4: // phases
		// keep our own copy for the readback above; the ISM's phases_cb does the driving
		m_ism_phases = data;
		m_ism->write(4, data);
		break;

	case 6: // mode, write zeroes
		m_ism->write(6, data & ~0x40);
		break;

	case 7: // mode, write ones
		m_ism->write(7, data | 0x40);
		break;

	default:
		m_ism->write(offset & 7, data);
		break;
	}
}

void a2bus_pcxporter_device::ism_phases_w(uint8_t phases)
{
	// On a TransDrive the ISM's phase lines are the drive's control signals rather than an
	// Apple drive's phase windings: PHASE 0 is DIRECTION and PHASE 1 is STEP.
	if (!m_cur_floppy)
	{
		return;
	}

	m_cur_floppy->dir_w(BIT(phases, 0));
	m_cur_floppy->stp_w(BIT(phases, 1));
}

void a2bus_pcxporter_device::ism_devsel_w(uint8_t devsel)
{
	// The ISM gates its two drive-enable lines with MOTORON (mode bit 7), so a selected drive
	// is a spinning drive.
	m_cur_floppy = (devsel >= 1 && devsel <= 2) ? m_transdrive[devsel - 1]->get_device() : nullptr;

	for (int i = 0; i < 2; i++)
	{
		floppy_image_device *const floppy = m_transdrive[i]->get_device();
		if (floppy)
		{
			floppy->mon_w(floppy == m_cur_floppy ? 0 : 1);
		}
	}

	m_ism->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
	{
		m_cur_floppy->ss_w(m_ism_hdsel ? 0 : 1);
	}
}

void a2bus_pcxporter_device::ism_hdsel_w(int state)
{
	// HDSEL is inverted for PC-style drives
	if ((state != m_ism_hdsel) && m_cur_floppy)
	{
		m_cur_floppy->ss_w(state ? 0 : 1);
	}
	m_ism_hdsel = state;
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_pcxporter_device::read_c800(uint16_t offset)
{
	if (offset < 0x400)
	{
		if (!machine().side_effects_disabled())
		{
			m_apple_reading_io = true;
			const uint8_t val = m_pcio_space->read_byte(offset);
			m_apple_reading_io = false;
			return val;
		}
		else
		{
			return 0;
		}
	}
	else if (offset < 0x600) // $CC00-$CDFF (2 pages)
	{
		return ram_r(m_cc00_offset + offset - 0x400);
	}
	else if (offset < 0x700) // $CE00-$CEFF (single page)
	{
		return ram_r(m_ce00_offset + offset - 0x600);
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
			rv = ram_r(m_offset);
			// don't increment if the debugger's reading
			if (!machine().side_effects_disabled())
			{
				m_offset++;
			}
			return rv;

		case 0x704: // read w/o increment
			rv = ram_r(m_offset);
			return rv;

		case 0x705: // $CC00 bits 23-16
			return m_cc00_offset >> 16;

		case 0x706: // $CC00 bits 15-8
			return (m_cc00_offset >> 8) & 0xff;

		case 0x70e: // $CE00 bits 23-16
			return m_ce00_offset >> 16;

		case 0x70f: // $CE00 bits 15-8
			return (m_ce00_offset >> 8) & 0xff;

		case 0x720: case 0x721: case 0x722: case 0x723:
		case 0x724: case 0x725: case 0x726: case 0x727:
			return ism_r(offset);

		case 0x736:
		{
			// HACK: The IIe has nothing like the IIgs's $C025, so the card uses a
			// cable to get the keyboard modifier state.  Plumbing that out through a2bus
			// for a single card is not architecturally great, so hack it is.
			uint8_t const special = m_host_kbspecial.read_safe(0x01);
			uint8_t rv = 0;
			if (!BIT(special, 3))
			{
				rv |= 0x01; // control
			}
			if (!(special & 0x06))
			{
				rv |= 0x02; // either shift
			}
			if (BIT(special, 4))
			{
				rv |= 0x04; // open apple
			}
			if (BIT(special, 5))
			{
				rv |= 0x08; // solid apple
			}
			if (BIT(special, 0))
			{
				rv |= 0x80; // caps lock
			}
			return rv;
		}

		case 0x730:
			return (m_regs[offset & 0x3ff] & 0x7f) | m_pc_data_pending;

		case 0x731: // $CF31 V30 halt/reset status in bits 4 & 5
			return m_cpu_status | (m_regs[0x331] & 0xcf);

		case 0x732: // $CF32 Apple/V30 I/O coordination
			return (m_regs[offset & 0x3ff] & 0x7f) | m_pc_data_pending;

		default:
			// logerror("Read $C800 at %x\n", offset + 0xc800);
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
		m_apple_writing_io = true;
		m_pcio_space->write_byte(offset, data);
		m_apple_writing_io = false;
	}
	else if (offset < 0x600) // $CC00-$CDFF (2 pages)
	{
		ram_w(m_cc00_offset + offset - 0x400, data);
	}
	else if (offset < 0x700) // $CE00-$CEFF (single page)
	{
		ram_w(m_ce00_offset + offset - 0x600, data);
	}
	else
	{
		switch (offset)
		{
		case 0x700:
			m_offset &= ~0xff;
			m_offset |= data;
			break;

		case 0x701:
			m_offset &= ~0xff00;
			m_offset |= (data << 8);
			break;

		case 0x702:
			m_offset &= ~0xff0000;
			m_offset |= (data << 16);
			break;

		case 0x703: // write w/increment
			ram_w(m_offset, data);
			if (m_offset >= 0xb0000 && m_offset <= 0xb3fff)
			{
				m_pcmem_space->write_byte(m_offset + 0x8000, data);
			}
			else if (m_offset >= 0xb4000 && m_offset <= 0xb7fff)
			{
				m_pcmem_space->write_byte(m_offset + 0x4000, data);
			}
			else if (m_offset >= 0xb8000 && m_offset <= 0xbbfff)
			{
				m_pcmem_space->write_byte(m_offset, data);
			}
			else if (m_offset >= 0xbc000 && m_offset <= 0xbffff)
			{
				m_pcmem_space->write_byte(m_offset - 0x4000, data);
			}
			m_offset++;
			break;

		case 0x704: // write w/o increment
			ram_w(m_offset, data);
			if (m_offset >= 0xb0000 && m_offset <= 0xb3fff)
			{
				m_pcmem_space->write_byte(m_offset + 0x8000, data);
			}
			else if (m_offset >= 0xb4000 && m_offset <= 0xb7fff)
			{
				m_pcmem_space->write_byte(m_offset + 0x4000, data);
			}
			else if (m_offset >= 0xb8000 && m_offset <= 0xbbfff)
			{
				m_pcmem_space->write_byte(m_offset, data);
			}
			else if (m_offset >= 0xbc000 && m_offset <= 0xbffff)
			{
				m_pcmem_space->write_byte(m_offset - 0x4000, data);
			}
			break;

		case 0x705: // $CC00 bits 23-16
			m_cc00_offset &= 0xffff;
			m_cc00_offset |= (data << 16);
			break;

		case 0x706: // $CC00 bits 15-8
			m_cc00_offset &= 0xff00ff;
			m_cc00_offset |= (data << 8);
			break;

		case 0x70e: // $CE00 bits 23-16
			m_ce00_offset &= 0xffff;
			m_ce00_offset |= (data << 16);
			break;

		case 0x70f: // $CE00 bits 15-8
			m_ce00_offset &= 0xff00ff;
			m_ce00_offset |= (data << 8);
			break;

		case 0x720: case 0x721: case 0x722: case 0x723:
		case 0x724: case 0x725: case 0x726: case 0x727:
			ism_w(offset, data);
			break;

		case 0x72c: // CGA 6845 register select
			m_pcio_space->write_byte(0x3d6, data);
			break;

		case 0x72d: // CGA 6845 data read/write
			m_pcio_space->write_byte(0x3d7, data);
			break;

		case 0x72e: // CGA mode select
			m_pcio_space->write_byte(0x3d8, data);
			break;

		case 0x72f: // CGA color select
			m_pcio_space->write_byte(0x3d9, data);
			break;

		case 0x730: // control 1
			if (BIT(data, 4))
			{
				m_v30->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				m_reset_during_halt = true;
				m_cpu_status &= ~0x10;
			}

			if (BIT(data, 5))
			{
				if (m_reset_during_halt)
				{
					m_v30->reset();
					m_reset_during_halt = false;
				}

				m_v30->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_v30->resume(SUSPEND_REASON_HALT | SUSPEND_REASON_DISABLE);
				m_cpu_status &= ~0x20;
			}

			if (BIT(data, 6))
			{
				// release the V30's interrupt request
				m_control &= ~0x40;
				m_v30->set_input_line(0, CLEAR_LINE);
			}

			if (BIT(data, 7))
			{
				LOGIRQ("$CF30: card IRQ disabled (pending %d)\n", m_pc_data_pending ? 1 : 0);
				m_irq_enable = false;
				m_control &= ~0x80;
				update_irq();
			}
			break;

		case 0x731: // control 2
			if (BIT(data, 4))
			{
				m_v30->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}

			if (BIT(data, 5))
			{
				m_v30->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}

			if (BIT(data, 6))
			{
				// interrupt the V30; the vector is already waiting at the top of the card's RAM
				LOGIRQ("$CF31: interrupting the V30, vector %02x\n", ram_r(0xbfffe));
				m_control |= 0x40;
				m_v30->set_input_line(0, ASSERT_LINE);
			}

			if (BIT(data, 7))
			{
				// enable the card IRQ, and let a request the 6502 hasn't seen yet through
				m_control |= 0x80;
				m_irq_enable = true;
				update_irq();
			}

			m_cpu_status |= (data & 0x30);
			m_regs[0x331] &= ~0x0f;
			m_regs[0x331] |= (data & 0xf);
			break;

		case 0x732: // $CF32 Apple/V30 I/O coordination
			m_pc_xfer_complete = data & 0x80;
			if (BIT(data, 7))
			{
				// the 6502 has finished the transfer; let the V30 out of its wait state
				LOGIRQ("$CF32: transfer complete, releasing V30\n");
				m_pc_data_pending = 0;
				if (m_pc_io_stalled)
				{
					m_pc_io_stalled = false;
					m_v30->resume(SUSPEND_REASON_SPIN);
				}
				update_irq();
			}
			break;

		default:
			m_regs[offset & 0x3ff] = data;
			break;
		}
	}
}

/*-------------------------------------------------
    stall_v30 - the V30 has touched a port the 6502
    is responsible for.  The V30 core doesn't support
    MAME's stall-on-access mechanism so we perform
    an instruction retry similar to what 68020+ do
    for bus errors.
-------------------------------------------------*/

void a2bus_pcxporter_device::stall_v30(offs_t port, bool is_read, bool is_16bit, bool supply)
{
	m_pc_data_pending = 0x80;
	m_regs[0x334] = port & 0xff;
	m_regs[0x335] = (port >> 8) & 0xff;

	m_regs[0x331] &= ~0xc0;         // clear bit 7 (direction) and bit 6 (transfer size)
	if (is_read)
	{
		m_regs[0x331] |= 0x80;      // the V30 is reading, so the 6502 must supply data
	}
	if (is_16bit)
	{
		m_regs[0x331] |= 0x40;
	}

	if (is_read && supply)
	{
		// the 6502 is going to work the answer out, so wind the program counter back to the
		// start of the instruction and let it run again once the mailbox has been filled in
		// (NEC_PC is just IP, so the current code segment has to come back out of it)
		m_v30->set_state_int(NEC_PC, (m_v30->pcbase() - (m_v30->state_int(NEC_PS) << 4)) & 0xffff);
	}

	if (!m_pc_io_stalled)
	{
		m_pc_io_stalled = true;
		m_v30->suspend(SUSPEND_REASON_SPIN, true);
	}

	update_irq();
}

void a2bus_pcxporter_device::update_irq()
{
	if (m_pc_data_pending && m_irq_enable)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

IRQ_CALLBACK_MEMBER(a2bus_pcxporter_device::v30_int_ack)
{
	// the 6502 writes the vector at bfffe
	const uint8_t vector = ram_r(0xbfffe);
	LOGIRQ("V30 INTA -> vector %02x\n", vector);
	return vector;
}

/*-------------------------------------------------
    port_routing - not every I/O port needs the
    6502's attention.  We use the software's own table
    to determine which ones do.
-------------------------------------------------*/

uint8_t a2bus_pcxporter_device::port_routing(offs_t port) const
{
	const uint8_t entry = ram_r(PORT_ROUTING_TABLE + (port & ~1));
	return BIT(port, 0) ? (entry >> 4) : (entry & 0x0f);
}

u16 a2bus_pcxporter_device::x86_port_r(offs_t offset, u16 mem_mask)
{
	offset <<= 1;
	const uint16_t retval = (m_c800_ram[offset + 1] << 8) | (m_c800_ram[offset]);

	if (mem_mask == 0xff00)
	{
		offset++;
	}

	const uint8_t routing = m_apple_reading_io ? 0 : port_routing(offset);
	if (routing == 0)
	{
		// nothing to do: the V30 has the dual-ported RAM to itself
	}
	else if (!BIT(routing, 0))
	{
		// the mailbox already holds what the V30 came for; the 6502 runs afterwards, and
		// what it leaves behind is for the next read.
		LOGPORTS("V30 IN  %03x -> %04x (%s, latched), PC %05x\n", offset, retval, (mem_mask == 0xffff) ? "word" : "byte", m_v30->pcbase());
		stall_v30(offset, true, mem_mask == 0xffff, false);
	}
	else if (offset != m_pc_in_restart)
	{
		LOGPORTS("V30 IN  %03x (%s) -> wait for the 6502, PC %05x\n", offset, (mem_mask == 0xffff) ? "word" : "byte", m_v30->pcbase());
		m_pc_in_restart = offset;
		stall_v30(offset, true, mem_mask == 0xffff, true);
	}
	else
	{
		LOGPORTS("V30 IN  %03x -> %04x (serviced)\n", offset, retval);
		m_pc_in_restart = 0xffff;
	}

	return retval;
}

void a2bus_pcxporter_device::x86_port_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset <<= 1;

	if (ACCESSING_BITS_0_7)
	{
		m_c800_ram[offset] = data & 0xff;
	}
	if (ACCESSING_BITS_8_15)
	{
		m_c800_ram[offset + 1] = data >> 8;
	}

	if (mem_mask == 0xff00)
	{
		offset++; // send the correct code to the 6502
	}

	// only do this if it's the V30 writing, not a 6502 passthrough
	if (!m_apple_writing_io && (port_routing(offset) != 0))
	{
		LOGPORTS("V30 OUT %03x <- %04x (%s), PC %05x\n", offset, data, (mem_mask == 0xffff) ? "word" : "byte", m_v30->pcbase());
		stall_v30(offset, false, mem_mask == 0xffff);
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_PCXPORTER, device_a2bus_card_interface, a2bus_pcxporter_device, "a2pcxport", "Applied Engineering PC Transporter")
