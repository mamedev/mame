// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Seals Electronics 8K SC RAM board

    PolyMorphic Systems used this S-100 memory board in their first
    series of computers before introducing a 16K dynamic RAM of their
    own design.

    The 8K SC originally used AMD 91L02APC 1024x1 static RAMs with a
    500 ns access time. Replacing these with faster RAMs was certainly
    possible (the 8K SC-Z kit was supplied with the 250 ns 2102LHPC),
    and essential for use with high-speed CPUs since the tightly-
    packed board had absolutely no room for a wait state circuit.

    The 8K SC could be configured for battery backup by providing +2
    to +4 V at 600 mA on pin 14. (Pin 14 also could provide standby
    power to static RAM boards by Ithaca Audio, but this use was
    incompatible with some other S-100 systems and not sanctioned by
    the IEEE-696 standard). Power for this prominently advertised yet
    entirely optional feature was supplied by a Battery Back-Up Card
    (BBUC) which Seals sold separately. However, 64 static RAM chips
    made the 8K SC rather power-hungry even when operating under
    normal power (which it drew through four 7805 voltage regulators).

**********************************************************************/

#include "emu.h"
#include "seals8k.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_8k_sc_device

class s100_8k_sc_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	s100_8k_sc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// delegated construction
	s100_8k_sc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-specific overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// S-100 memory access handlers
	virtual u8 s100_smemr_r(offs_t offset) override;
	virtual void s100_mwrt_w(offs_t offset, u8 data) override;

	// internal state
	std::unique_ptr<u8[]> m_ram;

private:
	// helpers
	bool board_selected(offs_t offset) const;

	// object finder
	required_ioport m_dsw;
};

// ======================> s100_8k_sc_bb_device

class s100_8k_sc_bb_device : public s100_8k_sc_device, public device_nvram_interface
{
public:
	// construction/destruction
	s100_8k_sc_bb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_nvram_interface overrides
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;
};

DEFINE_DEVICE_TYPE_PRIVATE(S100_8K_SC, device_s100_card_interface, s100_8k_sc_device, "s100_8k_sc", "Seals 8K SC Memory Board")
DEFINE_DEVICE_TYPE_PRIVATE(S100_8K_SC_BB, device_s100_card_interface, s100_8k_sc_bb_device, "s100_8k_sc_bb", "Seals 8K SC Memory Board with Battery Backup")
template class device_finder<device_s100_card_interface, false>;
template class device_finder<device_s100_card_interface, true>;


//**************************************************************************
//  CONFIGURATION SETTINGS
//**************************************************************************

static INPUT_PORTS_START( s100_8k_sc )
	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfb, "Address Range") PORT_DIPLOCATION("IC67:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(0xfe, "0 to 8K")
	PORT_DIPSETTING(0xfd, "8K to 16K")
	PORT_DIPSETTING(0xfb, "16K to 24K")
	PORT_DIPSETTING(0xf7, "24K to 32K")
	PORT_DIPSETTING(0xef, "32K to 40K")
	PORT_DIPSETTING(0xdf, "40K to 48K")
	PORT_DIPSETTING(0xbf, "48K to 56K")
	PORT_DIPSETTING(0x7f, "56K to 64K")
INPUT_PORTS_END


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  s100_8k_sc_device - constructor
//-------------------------------------------------

s100_8k_sc_device::s100_8k_sc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_s100_card_interface(mconfig, *this),
	m_dsw(*this, "DSW")
{
}


s100_8k_sc_device::s100_8k_sc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	s100_8k_sc_device(mconfig, S100_8K_SC, tag, owner, clock)
{
}


//-------------------------------------------------
//  s100_8k_sc_bb_device - constructor
//-------------------------------------------------

s100_8k_sc_bb_device::s100_8k_sc_bb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	s100_8k_sc_device(mconfig, S100_8K_SC_BB, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_input_ports - input port construction
//-------------------------------------------------

ioport_constructor s100_8k_sc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(s100_8k_sc);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_8k_sc_device::device_start()
{
	m_ram = make_unique_clear<u8[]>(0x2000);
	save_pointer(NAME(m_ram), 0x2000);
}


//-------------------------------------------------
//  nvram_read - read NVRAM from the file
//-------------------------------------------------

bool s100_8k_sc_bb_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_ram.get(), 0x2000);
	return !err && (actual == 0x2000);
}


//------------------------------------------------
//  nvram_write - write NVRAM to the file
//-------------------------------------------------

bool s100_8k_sc_bb_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_ram.get(), 0x2000);
	return !err;
}


//------------------------------------------------
//  nvram_default - default NVRAM setting
//-------------------------------------------------

void s100_8k_sc_bb_device::nvram_default()
{
}


//-------------------------------------------------
//  board_selected - return true if the address
//  is within the board's configured range
//-------------------------------------------------

bool s100_8k_sc_device::board_selected(offs_t offset) const
{
	// A13-A15 decoded by 74138 at IC66
	return (m_dsw->read() | 1 << ((offset >> 13) & 7)) == 0xff;
}


//-------------------------------------------------
//  s100_smemr_r - memory read
//-------------------------------------------------

u8 s100_8k_sc_device::s100_smemr_r(offs_t offset)
{
	if (board_selected(offset))
		return m_ram[offset & 0x1fff];
	else
		return 0xff;
}


//-------------------------------------------------
//  s100_mwrt_w - memory write
//-------------------------------------------------

void s100_8k_sc_device::s100_mwrt_w(offs_t offset, u8 data)
{
	if (board_selected(offset))
		m_ram[offset & 0x1fff] = data;
}
