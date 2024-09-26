// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Ampro Little Board/PC.

    This is unusual among PC/XT-compatible machines in that many standard
    peripheral functions, including the interrupt and refresh controllers,
    are integrated into the V40 CPU itself, with some software assistance
    to compensate for DMAC incompatibilities. Two Vadem SDIP64 ASICs and a
    standard FDC and UART provide most other PC-like hardware features. The
    BIOS also supports the onboard SCSI controller.

****************************************************************************/

#include "emu.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/nscsi/devices.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/nec/v5x.h"
#include "imagedev/floppy.h"
#include "machine/ins8250.h"
#include "machine/ncr5380.h"
#include "machine/upd765.h"
#include "sound/spkrdev.h"
#include "softlist_dev.h"
#include "speaker.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

class lbpc_state : public driver_device
{
public:
	lbpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_expbus(*this, "expbus")
		, m_fdc(*this, "fdc")
		, m_scsic(*this, "scsi:7:ncr")
		, m_kbd(*this, "kbd")
		, m_speaker(*this, "speaker")
		, m_dma_channel(0xff)
		, m_eop_active(false)
		, m_port61(0xff)
		, m_speaker_data(false)
		, m_kbd_clock(true)
		, m_kbd_data(true)
		, m_kbd_irq(false)
		, m_kbd_input(0xff)
	{
	}

	void lbpc(machine_config &config);

	int hsi_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 exp_dack1_r();
	void exp_dack1_w(u8 data);
	void iochck_w(int state);
	template <int Line> void dmaak_w(int state);
	void eop_w(int state);

	void keyboard_shift_in();
	void kbd_clock_w(int state);
	void kbd_data_w(int state);
	u8 keyboard_r();
	u8 port61_r();
	void port61_w(u8 data);
	u8 port62_r();
	void tout2_w(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<v40_device> m_maincpu;
	required_device<isa8_device> m_expbus;
	required_device<wd37c65c_device> m_fdc;
	required_device<ncr53c80_device> m_scsic;
	required_device<pc_kbdc_device> m_kbd;
	required_device<speaker_sound_device> m_speaker;

	u8 m_dma_channel;
	bool m_eop_active;
	u8 m_port61;
	bool m_speaker_data;
	bool m_kbd_clock;
	bool m_kbd_data;
	bool m_kbd_irq;
	u8 m_kbd_input;
};


void lbpc_state::machine_start()
{
	save_item(NAME(m_dma_channel));
	save_item(NAME(m_eop_active));
	save_item(NAME(m_port61));
	save_item(NAME(m_speaker_data));
	save_item(NAME(m_kbd_clock));
	save_item(NAME(m_kbd_irq));
	save_item(NAME(m_kbd_input));
}

void lbpc_state::machine_reset()
{
	port61_w(0);
}


u8 lbpc_state::exp_dack1_r()
{
	return m_expbus->dack_r(0);
}

void lbpc_state::exp_dack1_w(u8 data)
{
	m_expbus->dack_w(0, data);
}

void lbpc_state::iochck_w(int state)
{
	// TODO
}

template <int Line>
void lbpc_state::dmaak_w(int state)
{
	m_expbus->dack_line_w(Line + 1, state);
	if (!state)
	{
		m_dma_channel = Line;
		if (m_eop_active)
		{
			m_expbus->eop_w(Line + 1, ASSERT_LINE);
			if (Line == 1)
				m_fdc->tc_w(1);
			if (Line == 2)
				m_scsic->eop_w(1);
		}
	}
	else if (m_dma_channel == Line)
	{
		m_dma_channel = 0xff;
		if (m_eop_active)
		{
			m_expbus->eop_w(Line + 1, CLEAR_LINE);
			if (Line == 1)
				m_fdc->tc_w(0);
			if (Line == 2)
				m_scsic->eop_w(0);
		}
	}
}

void lbpc_state::eop_w(int state)
{
	m_eop_active = state == ASSERT_LINE;
	if (m_dma_channel != 0xff)
	{
		m_expbus->eop_w(m_dma_channel + 1, state);
		if (m_dma_channel == 1)
			m_fdc->tc_w(m_eop_active);
		if (m_dma_channel == 2)
			m_scsic->eop_w(m_eop_active);
	}
}

void lbpc_state::keyboard_shift_in()
{
	if (BIT(m_port61, 7) || m_kbd_irq)
		return;

	if (BIT(m_kbd_input, 0))
	{
		m_kbd_irq = true;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, 1);
	}

	m_kbd_input >>= 1;
	if (m_kbd_data)
	{
		m_kbd_input |= 0x80;
		LOG("%s: Shifting in 1 bit (%02X)\n", machine().describe_context(), m_kbd_input);
	}
	else
		LOG("%s: Shifting in 0 bit (%02X)\n", machine().describe_context(), m_kbd_input);
}

void lbpc_state::kbd_clock_w(int state)
{
	if (m_kbd_clock && !state)
		keyboard_shift_in();
	m_kbd_clock = state;
}

void lbpc_state::kbd_data_w(int state)
{
	m_kbd_data = state;
}

u8 lbpc_state::keyboard_r()
{
	return m_kbd_input;
}

u8 lbpc_state::port61_r()
{
	return m_port61;
}

void lbpc_state::port61_w(u8 data)
{
	if (BIT(m_port61, 1) && !BIT(data, 1))
		m_speaker->level_w(0);
	else if (!BIT(m_port61, 1) && BIT(data, 1))
		m_speaker->level_w(m_speaker_data);
	m_maincpu->tctl2_w(BIT(data, 0));

	if (BIT(m_port61, 3) != BIT(data, 3))
	{
		LOG("%s: PB3 changed to %d\n", machine().describe_context(), BIT(data, 3));
		m_kbd->data_write_from_mb(BIT(data, 3));
	}
	if (BIT(m_port61, 6) != BIT(data, 6))
	{
		LOG("%s: PB6 changed to %d\n", machine().describe_context(), BIT(data, 6));
		m_kbd->clock_write_from_mb(BIT(data, 6));
	}
	if (BIT(m_port61, 7) != BIT(data, 7))
	{
		LOG("%s: PB7 changed to %d\n", machine().describe_context(), BIT(data, 7));
		if (BIT(data, 7))
		{
			m_kbd_input = 0;
			if (m_kbd_irq)
			{
				m_kbd_irq = false;
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, 0);
			}
		}
	}
	if (BIT(m_port61, 2) != BIT(data, 2))
		LOG("%s: PB2 changed to %d\n", machine().describe_context(), BIT(data, 2));

	m_port61 = data;
}

u8 lbpc_state::port62_r()
{
	return 0;
}

void lbpc_state::tout2_w(int state)
{
	m_speaker_data = state;
	if (BIT(m_port61, 1))
		m_speaker->level_w(state);
}

int lbpc_state::hsi_r()
{
	// TODO
	return 0;
}

void lbpc_state::mem_map(address_map &map)
{
	map(0x00000, 0x9ffff).ram(); // 256K, 512K or 768K DRAM
	// 0xE0000–0xEFFFF: empty socket
	// 0xF0000-0xF7FFF: empty socket
	map(0xf8000, 0xfffff).rom().region("bios", 0);
}

void lbpc_state::io_map(address_map &map)
{
	map(0x0060, 0x0060).r(FUNC(lbpc_state::keyboard_r));
	map(0x0061, 0x0061).rw(FUNC(lbpc_state::port61_r), FUNC(lbpc_state::port61_w));
	map(0x0062, 0x0062).r(FUNC(lbpc_state::port62_r));
	map(0x0330, 0x0337).rw(m_scsic, FUNC(ncr53c80_device::read), FUNC(ncr53c80_device::write));
	map(0x0338, 0x0338).portr("JUMPERS");
	map(0x0378, 0x037f).unmaprw(); // parallel printer port (ASIC1)
	map(0x03f2, 0x03f2).w(m_fdc, FUNC(wd37c65c_device::dor_w));
	map(0x03f4, 0x03f5).m(m_fdc, FUNC(wd37c65c_device::map));
	map(0x03f7, 0x03f7).w(m_fdc, FUNC(wd37c65c_device::ccr_w));
	map(0x03f8, 0x03ff).rw("com", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
}


static INPUT_PORTS_START(lbpc)
	PORT_START("JUMPERS")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_READ_LINE_MEMBER(lbpc_state, hsi_r)
	PORT_DIPNAME(0x28, 0x28, "Drive A Type") PORT_DIPLOCATION("W26+W28:1,2")
	PORT_DIPSETTING(0x28, "360K, 5-1/4\"")
	PORT_DIPSETTING(0x08, "1.2M, 5-1/4\"")
	PORT_DIPSETTING(0x20, "720K, 3-1/2\"")
	PORT_DIPSETTING(0x00, "1.44M, 3-1/2\"")
	PORT_DIPNAME(0x50, 0x50, "Drive B Type") PORT_DIPLOCATION("W27+W29:1,2")
	PORT_DIPSETTING(0x50, "360K, 5-1/4\"")
	PORT_DIPSETTING(0x10, "1.2M, 5-1/4\"")
	PORT_DIPSETTING(0x40, "720K, 3-1/2\"")
	PORT_DIPSETTING(0x00, "1.44M, 3-1/2\"")
	PORT_DIPNAME(0x07, 0x07, "SCSI Initiator ID") PORT_DIPLOCATION("W25-W23:3,2,1")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
INPUT_PORTS_END


static void lbpc_floppies(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

void lbpc_state::lbpc(machine_config &config)
{
	V40(config, m_maincpu, 14.318181_MHz_XTAL); // 7.16 MHz operating frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &lbpc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lbpc_state::io_map);
	m_maincpu->set_tclk(14.318181_MHz_XTAL / 12); // generated by ASIC1
	m_maincpu->tout2_cb().set(FUNC(lbpc_state::tout2_w));
	m_maincpu->out_hreq_cb().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_maincpu->out_hreq_cb().append(m_maincpu, FUNC(v40_device::hack_w));
	m_maincpu->out_eop_cb().set(FUNC(lbpc_state::eop_w));
	m_maincpu->in_memr_cb().set([this] (offs_t offset) { return m_maincpu->space(AS_PROGRAM).read_byte(offset); });
	m_maincpu->out_memw_cb().set([this] (offs_t offset, u8 data) { m_maincpu->space(AS_PROGRAM).write_byte(offset, data); });
	m_maincpu->out_dack_cb<0>().set(FUNC(lbpc_state::dmaak_w<0>));
	m_maincpu->in_ior_cb<0>().set(FUNC(lbpc_state::exp_dack1_r));
	m_maincpu->out_iow_cb<0>().set(FUNC(lbpc_state::exp_dack1_w));
	m_maincpu->out_dack_cb<1>().set(FUNC(lbpc_state::dmaak_w<1>));
	m_maincpu->in_ior_cb<1>().set(m_fdc, FUNC(wd37c65c_device::dma_r));
	m_maincpu->out_iow_cb<1>().set(m_fdc, FUNC(wd37c65c_device::dma_w));
	m_maincpu->out_dack_cb<2>().set(FUNC(lbpc_state::dmaak_w<2>));
	m_maincpu->in_ior_cb<2>().set(m_scsic, FUNC(ncr53c80_device::dma_r));
	m_maincpu->out_iow_cb<2>().set(m_scsic, FUNC(ncr53c80_device::dma_w));

	PC_KBDC(config, m_kbd, pc_xt_keyboards, STR_KBD_IBM_PC_XT_83);
	m_kbd->out_clock_cb().set(FUNC(lbpc_state::kbd_clock_w));
	m_kbd->out_data_cb().set(FUNC(lbpc_state::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);

	ins8250_device &com(INS8250(config, "com", 1.8432_MHz_XTAL)); // NS8250AV
	com.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	com.out_rts_callback().set("serial", FUNC(rs232_port_device::write_rts)); // J3 pin 4
	com.out_tx_callback().set("serial", FUNC(rs232_port_device::write_txd)); // J3 pin 5
	com.out_dtr_callback().set("serial", FUNC(rs232_port_device::write_dtr)); // J3 pin 7

	wd37c65c_device &fdc(WD37C65C(config, m_fdc, 16_MHz_XTAL, 9.6_MHz_XTAL)); // WD37C65BJM
	// 9.6 MHz XTAL is optional and not supported by the BIOS, but can still be installed
	fdc.intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	fdc.drq_wr_callback().set(m_maincpu, FUNC(v40_device::dreq_w<1>));

	FLOPPY_CONNECTOR(config, "fdc:0", lbpc_floppies, "525sd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", lbpc_floppies, nullptr, floppy_image_device::default_pc_floppy_formats);
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr", NCR53C80).machine_config([this] (device_t *device) {
		downcast<ncr53c80_device &>(*device).irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ5);
		downcast<ncr53c80_device &>(*device).drq_handler().set(m_maincpu, FUNC(v40_device::dreq_w<2>));
	});

	rs232_port_device &serial(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	serial.dcd_handler().set("com", FUNC(ins8250_device::dcd_w)); // J3 pin 1
	serial.dsr_handler().set("com", FUNC(ins8250_device::dsr_w)); // J3 pin 2
	serial.rxd_handler().set("com", FUNC(ins8250_device::rx_w)); // J3 pin 3
	serial.cts_handler().set("com", FUNC(ins8250_device::cts_w)); // J3 pin 6
	serial.ri_handler().set("com", FUNC(ins8250_device::ri_w)); // J3 pin 8

	ISA8(config, m_expbus, 14.318181_MHz_XTAL / 2);
	m_expbus->set_memspace(m_maincpu, AS_PROGRAM);
	m_expbus->set_iospace(m_maincpu, AS_IO);
	m_expbus->drq1_callback().set(m_maincpu, FUNC(v40_device::dreq_w<0>));
	m_expbus->irq2_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	m_expbus->irq3_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	m_expbus->iochck_callback().set(FUNC(lbpc_state::iochck_w));

	ISA8_SLOT(config, "exp", 0, m_expbus, pc_isa8_cards, "ega", false);
}


ROM_START(lbpc)
	ROM_REGION(0x8000, "bios", 0)
	// "Firmware Version 1.0H  03/08/89"
	ROM_LOAD("lbpc-bio.rom", 0x0000, 0x8000, CRC(47bddf8b) SHA1(8a04fe34502f9f3bfe1e233762bbd5bbdd1c455d))
ROM_END

} // anonymous namespace


COMP(1989, lbpc, 0, 0, lbpc, lbpc, lbpc_state, empty_init, "Ampro Computers", "Little Board/PC", MACHINE_NOT_WORKING)
