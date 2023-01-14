// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/***************************************************************************

Vector Graphic had two related disk controllers for the Vector 4. There was
the "dual-mode" ST506-interface HDD/5.25" FDD controller and a stripped-down
5.25" FDD-only controller. Both can handle four FDD. The dual-mode version
supports a HDD as drive 0, replacing a FDD when used.

The floppy and hard drive formatting is not IBM compatible. Instead they are
based on the Micropolis MFM hard-sectored format which starts and ends the
sector with 0x00 preamble and postable bytes and starts sector data with a
0xFF sync byte. The FDD has 16 hard sectors, but the HDD uses a normal
soft-sectored drive with a PLL on the controller to emulate 32 hard sectors.
No abnormal MFM clock bits are used.

https://www.bitsavers.org/pdf/vectorGraphic/hardware/7200-1200-02-1_Dual-Mode_Disk_Controller_Board_Engineering_Documentation_Feb81.pdf
https://archive.org/details/7200-0001-vector-4-technical-information-sep-82

TODO:
- use floppy subsystem
- HDD support
- ECC

****************************************************************************/

#include "emu.h"

#include "bus/s100/vectordualmode.h"

#include "logmacro.h"

vector_micropolis_image_device::vector_micropolis_image_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MICROPOLIS_IMAGE, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

void vector_micropolis_image_device::device_start()
{
}

image_init_result vector_micropolis_image_device::call_load()
{
	uint64_t length;
	image_core_file().length(length);
	if (length % 275 != 0) {// TODO
		m_err_message = "Unexpected file size. Expected a multiple of 275";
		return image_init_result::FAIL;
	}
	return image_init_result::PASS;
}

s100_vector_dualmode_device::s100_vector_dualmode_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S100_VECTOR_DUALMODE, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_floppy(*this, "floppy%u", 0U)
	, m_ram{0}
	, m_cmar(0)
	, m_drive(0)
	, m_head(0)
	, m_track(0)
	, m_sector(0)
	, m_read(false)
{
}

bool s100_vector_dualmode_device::hdd_selected()
{
	return m_drive == 0 && false;
}

uint8_t s100_vector_dualmode_device::s100_sinp_r(offs_t offset)
{
	// 7200-1200-02-1 page 16 (1-10)
	uint8_t data;
	if (offset == 0xc0) { // status (0) port
		bool write_protect; // FDD
		bool ready; // HDD
		bool track0;
		bool write_fault = false; // HDD
		bool seek_complete; // HDD
		bool loss_of_sync; // HDD
		if (hdd_selected()) {
			write_protect = false;
			ready = true;
			track0 = false;
			seek_complete = true;
			loss_of_sync = true;
		} else {
			write_protect = !m_floppy[m_drive]->is_open() || m_floppy[m_drive]->is_readonly();
			ready = false;
			track0 = m_floppy[m_drive]->is_open() && m_track == 0;
			seek_complete = false;
			loss_of_sync = false;
		}

		data = write_protect | (ready << 1) | (track0 << 2)
		    | (write_fault << 3) | (seek_complete << 4) | (loss_of_sync << 5)
			| 0xc0;
	} else if (offset == 0xc1) { // status (1) port
		bool floppy_disk_selected;
		bool controller_busy;
		bool motor_on; // FDD
		bool type_of_hard_disk = true;
		if (hdd_selected()) {
			floppy_disk_selected = false;
			controller_busy = false;
			motor_on = false;
		} else {
			floppy_disk_selected = true;
			controller_busy = false;
			motor_on = m_motor_on_timer->enabled();
		}
		data = floppy_disk_selected | (controller_busy << 1) | (motor_on << 2)
		    | (type_of_hard_disk << 3) | 0xf0;
	} else if (offset == 0xc2) { // data port
		data = m_ram[m_cmar++];
		m_cmar &= 0x1ff;
	} else if (offset == 0xc3) { // reset port
		m_cmar = 0;
		data = 0xff;
	} else {
		data = 0xff;
	}
	return data;
}

void s100_vector_dualmode_device::s100_sout_w(offs_t offset, uint8_t data)
{
	// 7200-1200-02-1 page 14 (1-8)
	if (offset == 0xc0) { // control (0) port
		m_drive = BIT(data, 0, 2);
		m_head = BIT(data, 2, 3);
		uint8_t step = BIT(data, 5);
		uint8_t step_in = BIT(data, 6);
		//uint8_t low_current = BIT(data, 7);
		if (step) {
			if (step_in && m_track != 80)
				m_track += 1;
			else if (!step_in && m_track != 0)
				m_track -= 1;
		}
		// WR0| triggers U60, a 74LS123 with 100uF cap and 100k res
		m_motor_on_timer->adjust(attotime::from_usec(2819600));
	} else if (offset == 0xc1) { // control (1) port
		m_sector = BIT(data, 0, 5);
		m_read = BIT(data, 5);
	} else if (offset == 0xc2) { // data port
		m_ram[m_cmar++] = data;
		m_cmar &= 0x1ff;
	} else if (offset == 0xc3) { // start port
		// Read and write use cmar, so if it is not 0 you get weird results.
		// It is always supposed to be reset before read/write
		if (m_cmar != 0 || !m_floppy[m_drive]->is_open() || m_track >= 77)
			return;
		uint64_t file_pos = 275*(m_sector+16*(m_track+77*m_head));
		util::core_file &file = m_floppy[m_drive]->image_core_file();
		if (m_read) {
			std::size_t toread = 275;
			std::size_t actual;
			std::error_condition err = file.read_at(file_pos, m_ram, toread, actual);
			if (err) {
				LOG_OUTPUT_FUNC("Error: dual-mode read error %s:%d %s\n", err.category().name(), err.value(), err.message());
			} else if (toread != actual) {
				LOG_OUTPUT_FUNC("Error: dual-mode read error: unexpected amount read %d", actual);
			}
			memset(m_ram+275, 0, 128); // Postamble
			m_cmar += 275+128; // CMAR is trashed

			m_ram[274] = 0; // Ignore ECC
		} else if (!m_floppy[m_drive]->is_readonly()) {
			// The hardware doesn't care what it is writing, except when ECC is
			// enabled and even then it just inserts the ECC into the
			// pre-defined position. When reading it cares little. It will sync
			// to the first one bit as the first bit of the first byte and needs
			// a preceding string of zeros for the PLL and MPM alignment. It
			// also inserts the ECC result into the pre-defined position when
			// enabled.
			//
			// Assume standard preamble of 40 bytes.
			m_cmar += 40;
			std::size_t towrite = 275;
			std::size_t actual;
			std::error_condition err = file.write_at(file_pos, &m_ram[m_cmar], towrite, actual);
			if (err) {
				LOG_OUTPUT_FUNC("Error: dual-mode write error %s:%d %s\n", err.category().name(), err.value(), err.message());
			} else if (towrite != actual) {
				LOG_OUTPUT_FUNC("Error: dual-mode write error: unexpected amount written %d", actual);
			}
			m_cmar += 275+128; // CMAR is trashed
		}
	}
}

void s100_vector_dualmode_device::device_start()
{
	m_motor_on_timer = timer_alloc();

	save_item(NAME(m_ram));
	save_item(NAME(m_cmar));
	save_item(NAME(m_drive));
	save_item(NAME(m_head));
	save_item(NAME(m_track));
	save_item(NAME(m_sector));
	save_item(NAME(m_read));
}

void s100_vector_dualmode_device::device_reset()
{
	// POC| resets
	// U9
	m_drive = 0;
	m_head = 0;
	// U18
	m_sector = 0;
	m_read = false;
	// U60
	m_motor_on_timer->enable(false);
}

void s100_vector_dualmode_device::device_add_mconfig(machine_config &config)
{
	MICROPOLIS_IMAGE(config, m_floppy[0], 0);
	MICROPOLIS_IMAGE(config, m_floppy[1], 0);
	MICROPOLIS_IMAGE(config, m_floppy[2], 0);
	MICROPOLIS_IMAGE(config, m_floppy[3], 0);
}

DEFINE_DEVICE_TYPE(MICROPOLIS_IMAGE, vector_micropolis_image_device, "micropolisimage", "Micropolis Image (VGI)")
DEFINE_DEVICE_TYPE(S100_VECTOR_DUALMODE, s100_vector_dualmode_device, "vectordualmode", "Vector Dual-Mode Disk Controller")
