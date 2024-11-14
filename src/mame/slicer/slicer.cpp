// license:BSD-3-Clause
// copyright-holders:Carl

// Slicer Computers Slicer 80186 SBC
// The bios makefile refers to a "exe3bin" utility, this can be substituted with FreeDOS exe2bin and the /l=0xf800 option
// which will fixup the relocations

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/rs232/rs232.h"
#include "bus/scsi/scsi.h"
#include "cpu/i86/i186.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/mc68681.h"
#include "machine/wd_fdc.h"


namespace {

class slicer_state : public driver_device
{
public:
	slicer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fdc(*this, "fdc"),
		m_floppies(*this, "fdc:%u", 0U),
		m_sasi(*this, "sasi")
	{
	}

	void slicer(machine_config &config);

private:
	void sio_out_w(uint8_t data);
	void drive_size_w(int state);
	template <unsigned Drive> void drive_sel_w(int state);

	void slicer_io(address_map &map) ATTR_COLD;
	void slicer_map(address_map &map) ATTR_COLD;

	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device<scsi_port_device> m_sasi;
};

void slicer_state::sio_out_w(uint8_t data)
{
	const int state = (data & 0x80) ? 0 : 1;

	for (auto &floppy : m_floppies)
	{
		if (floppy->get_device())
			floppy->get_device()->mon_w(state);
	}
}

template <unsigned Drive>
void slicer_state::drive_sel_w(int state)
{
	if (!state)
		return;

	m_fdc->set_floppy(m_floppies[Drive]->get_device());
}

void slicer_state::drive_size_w(int state)
{
	m_fdc->set_unscaled_clock (state ? 1'000'000 : 2'000'000);
}

void slicer_state::slicer_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // fixed 256k for now
	map(0xf8000, 0xfffff).rom().region("bios", 0);
}

void slicer_state::slicer_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write)).umask16(0x00ff); //PCS0
	map(0x0080, 0x00ff).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff); //PCS1
	map(0x0100, 0x010f).mirror(0x0070).w("drivelatch", FUNC(ls259_device::write_d0)).umask16(0x00ff); //PCS2
	// TODO: 0x180 sets ack
	map(0x0180, 0x0180).r("sasi_data_in", FUNC(input_buffer_device::read)).w("sasi_data_out", FUNC(output_latch_device::write)).umask16(0x00ff); //PCS3
	map(0x0181, 0x0181).r("sasi_ctrl_in", FUNC(input_buffer_device::read));
	map(0x0184, 0x0184).r("sasi_data_in", FUNC(input_buffer_device::read)).w("sasi_data_out", FUNC(output_latch_device::write)).umask16(0x00ff);
	map(0x0185, 0x0185).r("sasi_ctrl_in", FUNC(input_buffer_device::read));
}

static void slicer_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

void slicer_state::slicer(machine_config &config)
{
	i80186_cpu_device &maincpu(I80186(config, "maincpu", 16_MHz_XTAL)); // 8 MHz clock output
	maincpu.set_addrmap(AS_PROGRAM, &slicer_state::slicer_map);
	maincpu.set_addrmap(AS_IO, &slicer_state::slicer_io);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set("maincpu", FUNC(i80186_cpu_device::int0_w));
	duart.a_tx_cb().set("rs232_1", FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("rs232_2", FUNC(rs232_port_device::write_txd));
	duart.outport_cb().set(FUNC(slicer_state::sio_out_w));

	rs232_port_device &rs232_1(RS232_PORT(config, "rs232_1", default_rs232_devices, "terminal"));
	rs232_1.rxd_handler().set("duart", FUNC(scn2681_device::rx_a_w));
	rs232_port_device &rs232_2(RS232_PORT(config, "rs232_2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set("duart", FUNC(scn2681_device::rx_b_w));

	FD1797(config, m_fdc, 16_MHz_XTAL / 2 / 8);
	m_fdc->intrq_wr_callback().set("maincpu", FUNC(i80186_cpu_device::int1_w));
	m_fdc->drq_wr_callback().set("maincpu", FUNC(i80186_cpu_device::drq0_w));
	FLOPPY_CONNECTOR(config, m_floppies[0], slicer_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], slicer_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], slicer_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], slicer_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	ls259_device &drivelatch(LS259(config, "drivelatch")); // U29
	drivelatch.q_out_cb<0>().set(m_sasi, FUNC(scsi_port_device::write_sel));
	drivelatch.q_out_cb<1>().set(m_sasi, FUNC(scsi_port_device::write_rst));
	drivelatch.q_out_cb<2>().set(FUNC(slicer_state::drive_sel_w<3>));
	drivelatch.q_out_cb<3>().set(FUNC(slicer_state::drive_sel_w<2>));
	drivelatch.q_out_cb<4>().set(FUNC(slicer_state::drive_sel_w<1>));
	drivelatch.q_out_cb<5>().set(FUNC(slicer_state::drive_sel_w<0>));
	drivelatch.q_out_cb<6>().set(FUNC(slicer_state::drive_size_w));
	drivelatch.q_out_cb<7>().set(m_fdc, FUNC(fd1797_device::dden_w));

	SCSI_PORT(config, m_sasi, 0);
	m_sasi->set_data_input_buffer("sasi_data_in");
	m_sasi->bsy_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit3));
	m_sasi->msg_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit4));
	m_sasi->cd_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit5));
	m_sasi->req_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit6));
	m_sasi->io_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit7));

	output_latch_device &sasi_data_out(OUTPUT_LATCH(config, "sasi_data_out"));
	m_sasi->set_output_latch(sasi_data_out);
	INPUT_BUFFER(config, "sasi_data_in");
	INPUT_BUFFER(config, "sasi_ctrl_in");
}

ROM_START( slicer )
	ROM_REGION16_LE(0x8001, "bios", 0)
	// built from sources, reset.asm adds an extra byte
	ROM_LOAD("epbios.bin", 0x0000, 0x8001, CRC(96fe9dd4) SHA1(5fc43454fe7d51f2ae97aef822155dcd28eb7f23))

	ROM_REGION(0x10000, "user1", 0)
	//slicer_h.bin : main slicer board, high byte
	//slicer_l.bin : main slicer board, low byte
	//slvid_cg.bin : slicer video/keyboard expansion board, character generator
	//slvid_e.bin : slicer video/keyboard expansion board, even byte
	//slvid_o.bin : slicer video/keyboard expansion board, odd byte
	ROM_LOAD( "slicer_h.bin", 0x000000, 0x004000, CRC(1f9a79b7) SHA1(2070c6818d39fe7ec4370fc2304469793a126731) )
	ROM_LOAD( "slicer_l.bin", 0x000000, 0x004000, CRC(6feef94b) SHA1(174488591b727a4130166bcb2e83c0e74323d43b) )
	ROM_LOAD( "slvid_cg.bin", 0x000000, 0x001000, CRC(d4d9ac2f) SHA1(866c760320b224ba8670501ea905de32193acedc) )
	ROM_LOAD( "slvid_o.bin",  0x000000, 0x001000, CRC(c62dda77) SHA1(1d0b9abc53412b0725072d4c33c478fb5358ab5c) )
	ROM_LOAD( "slvid_e.bin",  0x000000, 0x001000, CRC(8694274f) SHA1(8373baaea8d689bf52699b587942a57f26baf740) )
ROM_END

} // anonymous namespace


COMP( 1983, slicer, 0, 0, slicer, 0, slicer_state, empty_init, "Slicer Computers", "Slicer", MACHINE_NO_SOUND )
