// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion RS232/Parallel Module

**********************************************************************/

#include "emu.h"
#include "serpar.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "machine/output_latch.h"
#include "machine/psion_asic5.h"


namespace {

class psion_serial_parallel_device : public device_t, public device_psion_module_interface
{
public:
	psion_serial_parallel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, PSION_SERIAL_PARALLEL, tag, owner, clock)
		, device_psion_module_interface(mconfig, *this)
		, m_asic5(*this, "asic5")
		, m_cent_ctrl_out(*this, "cent_ctrl_out")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { };
	virtual void device_reset() override ATTR_COLD { m_cent_ctrl_out->write(0xff); };

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t data_r() override { return m_asic5->data_r(); }
	virtual void data_w(uint16_t data) override { m_asic5->data_w(data); }

private:
	required_device<psion_asic5_device> m_asic5;
	required_device<output_latch_device> m_cent_ctrl_out;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psion_serial_parallel_device::device_add_mconfig(machine_config &config)
{
	PSION_ASIC5(config, m_asic5, 1'536'000).set_mode(psion_asic5_device::PERIPHERAL_MODE); // TODO: clock derived from host
	m_asic5->set_info_byte(0x03); // RS232 + Parallel
	m_asic5->readpa_handler().set("cent_status_in", FUNC(input_buffer_device::read));
	m_asic5->writepb_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_asic5->writepc_handler().set("cent_ctrl_out", FUNC(output_latch_device::write));
	m_asic5->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_asic5->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_asic5->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_asic5->int_handler().set(DEVICE_SELF_OWNER, FUNC(psion_module_slot_device::intr_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_asic5, FUNC(psion_asic5_device::write_rxd));
	rs232.dcd_handler().set(m_asic5, FUNC(psion_asic5_device::write_dcd));
	rs232.dsr_handler().set(m_asic5, FUNC(psion_asic5_device::write_dsr));
	rs232.cts_handler().set(m_asic5, FUNC(psion_asic5_device::write_cts));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	centronics.ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));
	centronics.fault_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit6));
	centronics.perror_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);

	output_latch_device &cent_ctrl_out(OUTPUT_LATCH(config, "cent_ctrl_out"));
	cent_ctrl_out.bit_handler<4>().set("centronics", FUNC(centronics_device::write_strobe));
	cent_ctrl_out.bit_handler<5>().set("centronics", FUNC(centronics_device::write_init));
	cent_ctrl_out.bit_handler<6>().set("centronics", FUNC(centronics_device::write_autofd));
	cent_ctrl_out.bit_handler<7>().set("centronics", FUNC(centronics_device::write_select_in));

	INPUT_BUFFER(config, "cent_status_in");
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PSION_SERIAL_PARALLEL, device_psion_module_interface, psion_serial_parallel_device, "psion_serpar", "Psion RS232/Parallel Module")
