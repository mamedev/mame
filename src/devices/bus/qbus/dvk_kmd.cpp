// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DVK KMD floppy controller (decimal 3.057.136, device driver MY.SYS)

    https://emuverse.ru/downloads/computers/DVK/docs/KMD/KMD_MY_RU1_.djvu

    22-bit bus capable, uses DMA.  CSR 172140, vector 170.

    Firmware 091 supports only ISO track format (without index mark) and
    double-sided disks.

    Firmware 092 also supports PC track format (with index mark) and
    single-sided disks, fixes bugs and adds 4 new commands.

    Bootstrap for systems without ROM 279:

    172140/000040 37 <newline>
    172142/xxxxxx 0  "^"
    172140/000000 40 "G"

    Commands:

    0	read
    1	write
    2	read dd
    3	write dd
    4	read track
    5	read id
    6	format iso
    7	seek
    8	set
    9	read error state
    A*
    B*	format & write
    C*	format ibm
    D*	block move
    E*	run user code
    F	boot

***************************************************************************/

#include "emu.h"
#include "dvk_kmd.h"

#include "cpu/t11/t11.h"
#include "machine/1801vp128.h"
#include "machine/pdp11.h"
#include "machine/z80daisy.h"

#include "formats/bk0010_dsk.h"

#define LOG_DBG     (1U <<  1)

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGDBG(format, ...)   LOGMASKED(LOG_DBG, "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


namespace {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> dvk_kmd_device

class dvk_kmd_device : public device_t,
					public device_qbus_card_interface,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	dvk_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	virtual void init_w() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

	void kmd_mem(address_map &map) ATTR_COLD;

	required_device<k1801vm1_device> m_maincpu;
	required_device<k1801vp128_device> m_fdc;

private:
	static constexpr uint16_t KMDCSR_GO      = 0001;
	static constexpr uint16_t KMDCSR_DONE    = 0040;
	static constexpr uint16_t KMDCSR_TR      = 0200;
	static constexpr uint16_t KMDCSR_RD      = CSR_ERR | KMDCSR_TR | CSR_IE | KMDCSR_DONE;
	static constexpr uint16_t KMDCSR_WR      = CSR_IE | 077437;
	static constexpr uint16_t KMDCSR_L_WR    = CSR_ERR | KMDCSR_TR | KMDCSR_DONE;

	bool m_installed;
	line_state m_rxrdy;

	uint16_t m_cr;
	uint16_t m_go;
	uint16_t m_dr;

	static void floppy_formats(format_registration &fr);

	uint16_t local_read(offs_t offset);
	void local_write(offs_t offset, uint16_t data);

	uint16_t dma_read(offs_t offset);
	void dma_write(offs_t offset, uint16_t data);
};


ROM_START(dvk_kmd)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_DEFAULT_BIOS("092")
	ROM_SYSTEM_BIOS(0, "091", "mask 091")
	ROMX_LOAD("091.dat", 0x0000, 0x2000, CRC(3bd0effc) SHA1(4a3e4567dc46cb306d37fbe71839bc46e232bac8), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "092", "mask 092")
	ROMX_LOAD("092.dat", 0x0000, 0x2000, CRC(eb4e1de1) SHA1(6bc020054934a5562e763a18f915b8d612ccc70c), ROM_BIOS(1))
ROM_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dvk_kmd_device - constructor
//-------------------------------------------------

dvk_kmd_device::dvk_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DVK_KMD, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_fdc(*this, "fdc")
	, m_installed(false)
{
}

void dvk_kmd_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BK0010_FORMAT);
}

static void kmd_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void dvk_kmd_device::kmd_mem(address_map &map)
{
	// FIXME: add trap
	map(0000000, 0017777).rom().region("maincpu", 0);
	map(0020000, 0023777).ram();
	map(0040000, 0077777).rw(FUNC(dvk_kmd_device::dma_read), FUNC(dvk_kmd_device::dma_write));
	map(0177100, 0177103).rw(FUNC(dvk_kmd_device::local_read), FUNC(dvk_kmd_device::local_write));
	map(0177130, 0177133).rw(m_fdc, FUNC(k1801vp128_device::read), FUNC(k1801vp128_device::write));
	map(0177716, 0177717).lr16(NAME([] (offs_t offset) { return 010000 | 1; })).nopw();
	map(0177760, 0177761).noprw(); // RAM chip base address register
}

const tiny_rom_entry *dvk_kmd_device::device_rom_region() const
{
	return ROM_NAME(dvk_kmd);
}

void dvk_kmd_device::device_add_mconfig(machine_config &config)
{
	K1801VM1(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dvk_kmd_device::kmd_mem);

	K1801VP128(config, m_fdc, XTAL(4'000'000));
	m_fdc->ds_in_callback().set(
			[] (uint16_t data)
			{
				switch (data & 15)
				{
					case 1: return 0;
					case 2: return 1;
					case 4: return 2;
					case 8: return 3;
					default: return -1;
				}
			});
	FLOPPY_CONNECTOR(config, "fdc:0", kmd_floppies, "525qd", dvk_kmd_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", kmd_floppies, "525qd", dvk_kmd_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", kmd_floppies, "525qd", dvk_kmd_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", kmd_floppies, "525qd", dvk_kmd_device::floppy_formats);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dvk_kmd_device::device_start()
{
	// save state
	save_item(NAME(m_installed));
	save_item(NAME(m_cr));
	save_item(NAME(m_dr));

	m_installed = false;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dvk_kmd_device::device_reset()
{
	if (!m_installed)
	{
		m_bus->install_device(0172140, 0172143,
				read16sm_delegate(*this, FUNC(dvk_kmd_device::read)),
				write16sm_delegate(*this, FUNC(dvk_kmd_device::write)));
		m_installed = true;
	}
	m_cr = m_dr = m_go = 0;
	m_rxrdy = CLEAR_LINE;
}

void dvk_kmd_device::init_w()
{
	m_maincpu->pulse_input_line(t11_device::CP2_LINE, m_maincpu->minimum_quantum_time());
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t dvk_kmd_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_cr & KMDCSR_RD;
		break;

	case 1:
		data = m_dr;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void dvk_kmd_device::write(offs_t offset, uint16_t data)
{
	LOGDBG("host W %06o <- %06o [cr %06o go %06o]\n", 0172140 + (offset << 1), data, m_cr, m_go);

	switch (offset)
	{
	case 0:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_rxrdy);
		}
		m_cr = ((m_cr & ~KMDCSR_WR) | (data & KMDCSR_WR));
		if (data & KMDCSR_GO)
		{
			m_go = m_cr;
			m_cr &= ~KMDCSR_DONE;
		}
		break;

	case 1:
		m_dr = data;
		m_cr &= ~KMDCSR_TR;
		clear_virq(m_bus->birq4_w, m_cr, CSR_IE, m_rxrdy);
		break;
	}
}


int dvk_kmd_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int dvk_kmd_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = 0170;
	}

	return vec;
}


//**************************************************************************
//  local bus
//**************************************************************************

uint16_t dvk_kmd_device::local_read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_cr;
		break;

	case 1:
		data = m_dr;
		break;
	}

	return data;
}

void dvk_kmd_device::local_write(offs_t offset, uint16_t data)
{
	LOGDBG("locl W %06o <- %06o [cr %06o go %06o]\n", 0177100 + (offset << 1), data, m_cr, m_go);

	switch (offset)
	{
	case 0:
		if ((data & KMDCSR_DONE) && !(m_cr & KMDCSR_DONE))
		{
			if (m_go & CSR_IE) LOGDBG("raising irq %s\n", "");
			raise_virq(m_bus->birq4_w, m_cr, CSR_IE, m_rxrdy);
		}
		else if (!(data & KMDCSR_DONE) && (m_cr & KMDCSR_DONE))
		{
			if (m_go & CSR_IE) LOGDBG("clearing irq %s\n", "");
			clear_virq(m_bus->birq4_w, m_cr, CSR_IE, m_rxrdy);
		}
		m_cr = ((m_cr & ~KMDCSR_L_WR) | (data & KMDCSR_L_WR));
		break;

	case 1:
		m_dr = data;
		break;
	}
}

// FIXME add DMA protocol (CPU halt etc.); HALT mode howto?
uint16_t dvk_kmd_device::dma_read(offs_t offset)
{
	LOGDBG("dma  R %06o=%06o\n", offset, (offset << 1) | ((m_dr & 3) << 14));

	return m_bus->read((offset << 1) | ((m_dr & 3) << 14));
}

void dvk_kmd_device::dma_write(offs_t offset, uint16_t data)
{
	LOGDBG("dma  W %06o=%06o <- %06o\n", offset, (offset << 1) | ((m_dr & 3) << 14), data);

	m_bus->write((offset << 1) | ((m_dr & 3) << 14), data);
}

} // anonymous namespace

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DVK_KMD, device_qbus_card_interface, dvk_kmd_device, "dvk_kmd", "DVK KMD floppy controller")
