// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Micronics Technology WIN89

  A parallel port and hard drive interface in a left slot, with its own
  system ROM. The user moved the system ROM from the CPU board to the card;
  the card's ROM is selected at power on, and the software can switch to
  the other one.

  The hard drive interface is an "ISA" slot (most pins are hard wired) for
  an Everex EV-390 hard drive controller.

  I/O ports
    0x10 - 0x17  8255 printer port (A2 is not decoded, so 0x14 - 0x17 mirror 0x10 - 0x13)
    0x20 - 0x27  hard drive controller
      0x20  data
      0x21  status/command
      0x22  configuration
      0x24  controller reset
      0x25  system ROM select (write only), bit 0: 0 = card ROM, 1 = original ROM

  The manual says a write to 0x14 - 0x17 resets the 8255, but the schematic
  has its RESET pin tied to ground.

  The 8255 drives a Centronics printer through a 34 pin header, with no
  buffers between them
    PA0 - PA7  data
    PC0        data strobe
    PC1        input prime
    PC4        paper empty (the manual lists it as not used)
    PC5        busy, the only status the supplied BIOS code reads
    PC6        not connected to the printer
    PC7        fault

  TODO
    - hard drive controller

****************************************************************************/

#include "emu.h"

#include "mt_win89.h"

#include "bus/centronics/ctronics.h"
#include "machine/buffer.h"
#include "machine/i8255.h"
#include "machine/output_latch.h"


namespace {

class mt_win89_device : public device_t, public device_h89bus_left_card_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

	mt_win89_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// device_h89bus_left_card_interface implementation
	virtual u8 *sys_rom() override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void rom_select_w(u8 data);

	required_region_ptr<u8> m_sys_rom;
	required_device<i8255_device> m_par;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;

	bool m_original_rom;
};


mt_win89_device::mt_win89_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H89BUS_MT_WIN89, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_sys_rom(*this, "sys_rom"),
	m_par(*this, "parallel_port"),
	m_centronics(*this, "centronics"),
	m_cent_data_out(*this, "cent_data_out"),
	m_cent_status_in(*this, "cent_status_in")
{
}

u8 *mt_win89_device::sys_rom()
{
	return &m_sys_rom[0];
}

void mt_win89_device::rom_select_w(u8 data)
{
	m_original_rom = BIT(data, 0);

	select_card_sys_rom(!m_original_rom);
}

void mt_win89_device::device_start()
{
	// the left slots have no reset line, so only power on selects the card's ROM
	m_original_rom = false;

	save_item(NAME(m_original_rom));

	h89bus().install_io_device(0x10, 0x17,
			read8sm_delegate(*m_par, FUNC(i8255_device::read)),
			write8sm_delegate(*m_par, FUNC(i8255_device::write)));
	h89bus().install_io_device(0x25, 0x25,
			write8smo_delegate(*this, FUNC(mt_win89_device::rom_select_w)));
}

void mt_win89_device::device_reset()
{
	// the memory map is built after the cards start
	select_card_sys_rom(!m_original_rom);
}

void mt_win89_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_par);
	m_par->out_pa_callback().set(m_cent_data_out, FUNC(output_latch_device::write));
	m_par->out_pc_callback().set(m_centronics, FUNC(centronics_device::write_strobe)).bit(0);
	m_par->out_pc_callback().append(m_centronics, FUNC(centronics_device::write_init)).bit(1);
	m_par->in_pc_callback().set(m_cent_status_in, FUNC(input_buffer_device::read));

	OUTPUT_LATCH(config, m_cent_data_out);
	INPUT_BUFFER(config, m_cent_status_in);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(*m_cent_data_out);
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit4));
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit5));
	m_centronics->fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));
}

ROM_START(mt_win89)
	ROM_REGION(0x1000, "sys_rom", 0)
	// board has no IC numbers
	ROM_LOAD("2732_win89_2_3.bin", 0x0000, 0x1000, CRC(c54c0b3d) SHA1(f354d494794b0281b9cc074ce46362d3cd065350))
ROM_END

const tiny_rom_entry *mt_win89_device::device_rom_region() const
{
	return ROM_NAME(mt_win89);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MT_WIN89, device_h89bus_left_card_interface, mt_win89_device, "h89_mt_win89", "Micronics Technology WIN89")
