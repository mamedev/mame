// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*******************************************************************************

    Tesla PMD 32 floppy disk unit

    An external disk unit with its own processor, for the PMD 85 family and the
    Consul 2717.  Hardware:

        MHB 8080A CPU, 2 KB ROM at 0x0000, 1 KB RAM at 0x1800
        8272A FDC         I/O 0x00-0x01
        8255 PIO          I/O 0x20-0x23 (port A in mode 2: link to the host)
        8257 DMA          I/O 0x40-0x48 (channel 0 serves the FDC)
        drive latch       I/O 0xe0
        two 5.25" double-sided double-density drives

    The host drives the unit over a bidirectional parallel link: its own 8255
    runs port A in mode 2, and each side strobes the byte the other has written
    into its input latch.  After power-on the unit repeatedly offers the
    presentation byte 0xaa and waits for the host to answer with the same.
    Commands are a letter, its parameters and an XOR checksum; the unit
    answers 0x33 (ACK) or 0x99 (NAK), a result code, and any data.

    Media are formatted 40 cylinders x 2 sides x 9 sectors x 512 bytes MFM.
    The control program presents them as 80 logical tracks of 36 sectors of
    128 bytes (cylinder = track / 2, head = track & 1, four logical sectors to
    each physical sector).  80-cylinder drives (525qd) hold 160 logical
    tracks.

    TODO:
    - The real link cable has not been traced; it is modelled as each side
      strobing in the other's byte and acknowledging the byte it reads.
    - CPU clock is unverified.
    - The 8272's READY wiring has not been traced; READY follows the selected
      drive holding a disk.

*******************************************************************************/

#include "emu.h"
#include "pmd32.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/upd765.h"

#include "formats/pmd32_dsk.h"


namespace {

void pmd32_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

class pmd32_device : public device_t, public device_pmd85_gpio_interface
{
public:
	pmd32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_pmd85_gpio_interface implementation
	virtual uint8_t pa_r() override;
	virtual void pa_w(uint8_t data) override;
	virtual void pc_w(uint8_t data) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<i8080_cpu_device> m_cpu;
	required_device<i8255_device> m_ppi;
	required_device<i8272a_device> m_fdc;
	required_device<i8257_device> m_dma;
	required_device_array<floppy_connector, 2> m_floppy;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	uint8_t m_host_data;
	uint8_t m_unit_data;
	uint8_t m_host_pc;
	uint8_t m_unit_pc;
	bool m_host_ibf;
	bool m_host_obf;
	bool m_unit_ibf;
	bool m_unit_obf;
	bool m_in_link;
	uint8_t m_drive;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	void ppi_pa_w(uint8_t data);
	void ppi_pc_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(host_pa_sync);
	TIMER_CALLBACK_MEMBER(unit_pa_sync);
	TIMER_CALLBACK_MEMBER(host_pc_sync);
	TIMER_CALLBACK_MEMBER(unit_pc_sync);
	void link_active();
	void update_link(bool ack_unit, bool ack_host);
	void drive_w(uint8_t data);
	void update_ready(floppy_image_device *removed);
	void floppy_load(floppy_image_device *floppy) { update_ready(nullptr); }
	void floppy_unload(floppy_image_device *floppy) { update_ready(floppy); }
	uint8_t dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, uint8_t data);
	void hrq_w(int state);
};


pmd32_device::pmd32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PMD32, tag, owner, clock)
	, device_pmd85_gpio_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_ppi(*this, "ppi")
	, m_fdc(*this, "fdc")
	, m_dma(*this, "dma")
	, m_floppy(*this, { "fdc:0", "fdc:2" })
	, m_host_data(0xff)
	, m_unit_data(0xff)
	, m_host_pc(0xff)
	, m_unit_pc(0xff)
	, m_host_ibf(false)
	, m_host_obf(true)
	, m_unit_ibf(false)
	, m_unit_obf(true)
	, m_in_link(false)
	, m_drive(0)
{
}


ROM_START(pmd32)
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD("pmd32.rom", 0x0000, 0x0800, CRC(5c28d71d) SHA1(28ef888a4de259a36095177e4d05425d05814d59))
ROM_END

const tiny_rom_entry *pmd32_device::device_rom_region() const
{
	return ROM_NAME(pmd32);
}


void pmd32_device::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("rom", 0);
	map(0x1800, 0x1bff).ram();
}

void pmd32_device::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).m(m_fdc, FUNC(i8272a_device::map));
	map(0x20, 0x23).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x48).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0xe0, 0xe0).w(FUNC(pmd32_device::drive_w));
}


void pmd32_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_PMD32_FORMAT);
	fr.add(FLOPPY_PMD32_C2717_FORMAT);
}

void pmd32_device::device_add_mconfig(machine_config &config)
{
	I8080(config, m_cpu, 2'048'000);
	m_cpu->set_addrmap(AS_PROGRAM, &pmd32_device::mem_map);
	m_cpu->set_addrmap(AS_IO, &pmd32_device::io_map);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set([this] () -> uint8_t { return m_host_data; });
	m_ppi->out_pa_callback().set(FUNC(pmd32_device::ppi_pa_w));
	m_ppi->out_pc_callback().set(FUNC(pmd32_device::ppi_pc_w));

	I8272A(config, m_fdc, 8'000'000);
	m_fdc->set_ready_line_connected(false);
	m_fdc->intrq_wr_callback().set_inputline(m_cpu, I8085_INTR_LINE);
	m_fdc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq0_w));

	I8257(config, m_dma, 2'048'000);
	m_dma->out_hrq_cb().set(FUNC(pmd32_device::hrq_w));
	m_dma->in_memr_cb().set(FUNC(pmd32_device::dma_mem_r));
	m_dma->out_memw_cb().set(FUNC(pmd32_device::dma_mem_w));
	m_dma->in_ior_cb<0>().set(m_fdc, FUNC(i8272a_device::dma_r));
	m_dma->out_iow_cb<0>().set(m_fdc, FUNC(i8272a_device::dma_w));
	m_dma->out_tc_cb().set(m_fdc, FUNC(i8272a_device::tc_line_w));

	// the control program addresses drive A as unit 0 and drive B as unit 2
	FLOPPY_CONNECTOR(config, m_floppy[0], pmd32_floppies, "525dd", pmd32_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], pmd32_floppies, "525dd", pmd32_device::floppy_formats).enable_sound(true);
}


void pmd32_device::device_start()
{
	save_item(NAME(m_host_data));
	save_item(NAME(m_unit_data));
	save_item(NAME(m_host_pc));
	save_item(NAME(m_unit_pc));
	save_item(NAME(m_host_ibf));
	save_item(NAME(m_host_obf));
	save_item(NAME(m_unit_ibf));
	save_item(NAME(m_unit_obf));
	save_item(NAME(m_drive));

	m_cpu->space(AS_PROGRAM).specific(m_program);
	update_ready(nullptr);

	for (auto &connector : m_floppy)
	{
		if (floppy_image_device *floppy = connector->get_device())
		{
			floppy->setup_load_cb(floppy_image_device::load_cb(&pmd32_device::floppy_load, this));
			floppy->setup_unload_cb(floppy_image_device::unload_cb(&pmd32_device::floppy_unload, this));
		}
	}
}


// A byte waiting in one side's output latch (/OBFA low) is strobed into the
// other side whenever its input latch is empty (IBFA low); reading it (IBFA
// falling) acknowledges it to the sender.  Strobing on the level rather than
// the edge matters: the unit offers its presentation byte long before the host
// switches its 8255 to mode 2.
//
// Each side's own accesses reach the other side through synchronize(), so the
// unit never sees a host byte before its control program has emptied its
// input latch.  Changes caused by the link's own strobes and acknowledges
// already happen at a synchronised point.  A port C sync applies that side's
// latest port C, not the value it was queued with: an acknowledge delivered
// in between must not be undone by an older /OBF.  Inside the link only rises
// are taken (a strobe raises IBF, an acknowledge raises /OBF); falls come from
// that side's own accesses, which arrive through their own syncs.

uint8_t pmd32_device::pa_r()
{
	return m_unit_data;
}

void pmd32_device::pa_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pmd32_device::host_pa_sync), this), data);
}

void pmd32_device::pc_w(uint8_t data)
{
	m_host_pc = data;
	if (m_in_link)
	{
		m_host_obf = m_host_obf || BIT(data, 7);
		m_host_ibf = m_host_ibf || BIT(data, 5);
	}
	else
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pmd32_device::host_pc_sync), this));
	}
}

void pmd32_device::ppi_pa_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pmd32_device::unit_pa_sync), this), data);
}

void pmd32_device::ppi_pc_w(uint8_t data)
{
	m_unit_pc = data;
	if (m_in_link)
	{
		m_unit_obf = m_unit_obf || BIT(data, 7);
		m_unit_ibf = m_unit_ibf || BIT(data, 5);
	}
	else
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(pmd32_device::unit_pc_sync), this));
	}
}

TIMER_CALLBACK_MEMBER(pmd32_device::host_pa_sync)
{
	m_host_data = param;
}

TIMER_CALLBACK_MEMBER(pmd32_device::unit_pa_sync)
{
	m_unit_data = param;
}

TIMER_CALLBACK_MEMBER(pmd32_device::host_pc_sync)
{
	bool const obf = BIT(m_host_pc, 7);
	bool const ibf = BIT(m_host_pc, 5);
	bool const ack_unit = m_host_ibf && !ibf;
	if ((obf != m_host_obf) || (ibf != m_host_ibf))
		link_active();
	m_host_obf = obf;
	m_host_ibf = ibf;
	update_link(ack_unit, false);
}

TIMER_CALLBACK_MEMBER(pmd32_device::unit_pc_sync)
{
	bool const obf = BIT(m_unit_pc, 7);
	bool const ibf = BIT(m_unit_pc, 5);
	bool const ack_host = m_unit_ibf && !ibf;
	if ((obf != m_unit_obf) || (ibf != m_unit_ibf))
		link_active();
	m_unit_obf = obf;
	m_unit_ibf = ibf;
	update_link(false, ack_host);
}

void pmd32_device::link_active()
{
	// every byte takes a handshake between the two CPUs: keep them in step
	// while the link is busy
	machine().scheduler().perfect_quantum(attotime::from_msec(10));
}

// ack_unit / ack_host: that side's byte has just been read by the other side
void pmd32_device::update_link(bool ack_unit, bool ack_host)
{
	if (m_in_link)
		return;
	m_in_link = true;

	if (ack_unit)
	{
		m_ppi->pc6_w(0);
		m_ppi->pc6_w(1);
	}
	if (ack_host)
	{
		acknowledge_host();
	}
	if (!m_unit_obf && !m_host_ibf)
	{
		strobe_host();
	}
	if (!m_host_obf && !m_unit_ibf)
	{
		m_ppi->pc4_w(0);
		m_ppi->pc4_w(1);
	}

	m_in_link = false;
}


void pmd32_device::drive_w(uint8_t data)
{
	// bit 7: select B, bit 6: select A, bit 5: motor B, bit 4: motor A
	m_drive = data;
	for (int i = 0; i < 2; i++)
	{
		if (floppy_image_device *floppy = m_floppy[i]->get_device())
			floppy->mon_w(!BIT(data, 4 + i));
	}
	update_ready(nullptr);
}

// The FDC's READY input is modelled as "the selected drive holds a disk" (the
// unit's wiring has not been traced).
void pmd32_device::update_ready(floppy_image_device *removed)
{
	bool ready = false;
	for (int i = 0; i < 2; i++)
	{
		floppy_image_device *const floppy = m_floppy[i]->get_device();
		if (BIT(m_drive, 6 + i) && floppy && (floppy != removed) && floppy->exists())
			ready = true;
	}
	m_fdc->ready_w(!ready);
}

uint8_t pmd32_device::dma_mem_r(offs_t offset)
{
	return m_program.read_byte(offset);
}

void pmd32_device::dma_mem_w(offs_t offset, uint8_t data)
{
	m_program.write_byte(offset, data);
}

void pmd32_device::hrq_w(int state)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, state);
	m_dma->hlda_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PMD32, device_pmd85_gpio_interface, pmd32_device, "pmd32", "Tesla PMD 32 floppy disk unit")
