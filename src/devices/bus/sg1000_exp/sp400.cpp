// license: BSD-3-Clause
// copyright-holders: Fabio Dalla Libera

/*
    Sega SP-400 Plotter

    6805 dumped using https://github.com/charlesmacd/HD6805_Reader

    Plotter uses ALPS Micrographic Printer DPG 1302 mechanism.

    IC list:

    * M5L8035LP (Mitsubishi clone of I8035)

    P1 is the deserialized output data
    P2.4 is the input serial data
    P2.5 is busy (output to the computer)
    P2.6 is the output strobe
    P2.7 is busy (input from the 6805)

    * HD6805V (Hitachi chip, pin compatible with MC6805P2, with 4K of ROM)

    port A connected to the panel buttons
        "By analyzing the disassembly I found out that three pins (PA5,6,7 of the 6805) are used to set
        different configurations. The resolution can be changed, probably to support different mechanics.
        The baud rate is 4800."
    port B connected to the motors
    port C connected to the pen and misc signals
    port D connected to M5L8035LP parallel data
*/

// this is currently incomplete: missing paper output and pen sensor, everything else is working

#include "emu.h"
#include "sp400.h"

#include "cpu/m6805/m68705.h"
#include "cpu/mcs48/mcs48.h"


namespace {

class sp400_printer_device : public device_t, public device_sk1100_printer_port_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	sp400_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void input_data(int state) override { m_data = state; }
	virtual int output_busy() override { return m_busy; }

private:
	void dser_map(address_map &map);
	void dser_p1_w(uint8_t data);
	uint8_t dser_p2_r();
	void dser_p2_w(uint8_t data);

	uint8_t mot_pa_r();
	void mot_pb_w(uint8_t data);
	uint8_t mot_pc_r();
	void mot_pc_w(uint8_t data);
	uint8_t mot_pd_r();
	void update_pen_state(uint8_t data);
	uint8_t pen_sensor() { return 0; }  // not currently hooked up

	required_device<i8035_device> m_dsercpu;     // "deserializer CPU"
	required_device<m6805_hmos_device> m_motcpu; // "motor CPU"

	uint8_t m_data, m_busy;
	uint8_t m_dserdataout, m_dserstrobe;
	uint8_t m_motbusy, m_motpenup, m_motpendown;
	uint8_t m_pendown;

	required_ioport m_buttons;
};

sp400_printer_device::sp400_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SP400_PRINTER, tag, owner, clock),
	device_sk1100_printer_port_interface(mconfig, *this),
	m_dsercpu(*this, "dsercpu"),
	m_motcpu(*this, "motcpu"),
	m_data(0),
	m_busy(1),
	m_dserdataout(0xff),
	m_dserstrobe(1),
	m_motbusy(0),
	m_motpenup(1),
	m_motpendown(1),
	m_pendown(0),
	m_buttons(*this, "BUTTONS")
{
}

void sp400_printer_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_busy));
	save_item(NAME(m_dserdataout));
	save_item(NAME(m_dserstrobe));
	save_item(NAME(m_motbusy));
	save_item(NAME(m_motpenup));
	save_item(NAME(m_motpendown));
	save_item(NAME(m_pendown));
}

void sp400_printer_device::dser_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void sp400_printer_device::dser_p1_w(uint8_t data)
{
	m_dserdataout = data;
}

void sp400_printer_device::dser_p2_w(uint8_t data)
{
	m_busy = BIT(data, 5);
	m_dserstrobe = BIT(data, 6);
	m_motcpu->set_input_line(M6805_IRQ_LINE, m_dserstrobe);
}

uint8_t sp400_printer_device::dser_p2_r()
{
	return
			(m_motbusy << 7) |
			(m_dserstrobe << 6) |
			(m_busy << 5) |
			(m_data << 4) |
			(1 << 3) |
			(1 << 2) |
			(1 << 1) |
			(1 << 0);
}

uint8_t sp400_printer_device::mot_pa_r()
{
	return
			(0 << 0) |
			(0 << 1) |
			(m_buttons->read() << 2) |  // bits 2..4
			(0 << 5) |
			(0 << 6) |
			(1 << 7);
}

void sp400_printer_device::mot_pb_w(uint8_t data)
{
	// cr stepper data in 0..3
	// pf stepper data in 4..7
}

uint8_t sp400_printer_device::mot_pc_r()
{
	return
			(m_motpenup   << 0) |
			(m_motpendown << 1) |
			(1 << 2) |
			(pen_sensor() << 3) |
			(m_motbusy    << 4) |
			(1 << 5) | (1 << 6) | (1 << 7); // unconfirmed
}

void sp400_printer_device::mot_pc_w(uint8_t data)
{
	m_motpenup   = BIT(data, 0);
	m_motpendown = BIT(data, 1);
	m_motbusy    = BIT(data, 4);
	update_pen_state(data);
}

uint8_t sp400_printer_device::mot_pd_r()
{
	return sp400_printer_device::m_dserdataout;
}

void sp400_printer_device::update_pen_state(uint8_t data)
{
	if (!BIT(data, 0)) m_pendown = 0;
	if (!BIT(data, 1)) m_pendown = 1;
}

void sp400_printer_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_dsercpu, 4_MHz_XTAL);
	m_dsercpu->p1_out_cb().set(FUNC(sp400_printer_device::dser_p1_w));
	m_dsercpu->p2_in_cb().set(FUNC(sp400_printer_device::dser_p2_r));
	m_dsercpu->p2_out_cb().set(FUNC(sp400_printer_device::dser_p2_w));
	m_dsercpu->set_addrmap(AS_PROGRAM, &sp400_printer_device::dser_map);

	M6805U3(config, m_motcpu, 4_MHz_XTAL);
	m_motcpu->porta_r().set(FUNC(sp400_printer_device::mot_pa_r));
	m_motcpu->portb_w().set(FUNC(sp400_printer_device::mot_pb_w));
	m_motcpu->portc_r().set(FUNC(sp400_printer_device::mot_pc_r));
	m_motcpu->portc_w().set(FUNC(sp400_printer_device::mot_pc_w));
	m_motcpu->portd_r().set(FUNC(sp400_printer_device::mot_pd_r));
}

ROM_START( sp400 )
	ROM_REGION( 0x1000, "dsercpu", 0 )
	ROM_LOAD( "sp400_8035.bin", 0x0000, 0x1000, CRC(0eb48272) SHA1(08a2727f1592f5d2ecb2e368126ad7bfc5d3c270) )

	ROM_REGION(0x1000, "motcpu", 0 )
	ROM_LOAD( "sp400_hd6805v1.bin", 0x0000, 0x1000, CRC(aa073745) SHA1(65016f3b022af30cc6b084af1e43b29168721a60) )
ROM_END

const tiny_rom_entry *sp400_printer_device::device_rom_region() const
{
	return ROM_NAME( sp400 );
}

INPUT_PORTS_START( sp400 )
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Color Select") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Line Feed")    PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pen Change")   PORT_CODE(KEYCODE_3_PAD)
INPUT_PORTS_END

ioport_constructor sp400_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sp400);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SP400_PRINTER, device_sk1100_printer_port_interface, sp400_printer_device, "sega_sp400", "Sega SP-400 Plotter")
