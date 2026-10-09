// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MSD SD-1/SD-2 Disk Drive emulation

**********************************************************************/

#include "emu.h"
#include "msdsd.h"

#include "formats/d64_dsk.h"
#include "formats/g64_dsk.h"
#include "machine/rescap.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MSD_SD1, msd_sd1_device, "msdsd1", "MSD SD-1 Disk Drive")
DEFINE_DEVICE_TYPE(MSD_SD2, msd_sd2_device, "msdsd2", "MSD SD-2 Dual Disk Drive")
DEFINE_DEVICE_TYPE(GPIB_MSD_SD1, msd_sd1_ieee488_device, "msdsd1_ieee488", "MSD SD-1 Disk Drive (IEEE-488)")
DEFINE_DEVICE_TYPE(GPIB_MSD_SD2, msd_sd2_ieee488_device, "msdsd2_ieee488", "MSD SD-2 Dual Disk Drive (IEEE-488)")


//-------------------------------------------------
//  ROM( msdsd1 )
//-------------------------------------------------

ROM_START( msdsd1 )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_LOAD( "sd-1-1.3-c000.bin", 0x0000, 0x2000, CRC(f399778d) SHA1(c0d939c354d84018038c60a231fc43fb9279d8a4) )
	ROM_LOAD( "sd-1-1.3-e000.bin", 0x2000, 0x2000, CRC(7ac80da4) SHA1(99dd15c6d97938eba73880b18986a037e90742ab) )
ROM_END


//-------------------------------------------------
//  ROM( msdsd2 )
//-------------------------------------------------

ROM_START( msdsd2 )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("msd")
	ROM_SYSTEM_BIOS( 0, "msd", "MSD DOS V2.3" )
	ROMX_LOAD( "sd-2-2.3-c000.u6", 0x0000, 0x2000, CRC(2207560e) SHA1(471e9b4a4ac09ceee9acc1774534510396f98b9a), ROM_BIOS(0) )
	ROMX_LOAD( "sd-2-2.3-e000.u5", 0x2000, 0x2000, CRC(4efd87a2) SHA1(4beec0b7ce2349add3b0a5bceee60826637df8d9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "cldmd", "Chip Level Designs Mass Duplicator" )
	ROMX_LOAD( "cld-ac.u6", 0x0000, 0x2000, CRC(0440d2d8) SHA1(0b8e36f98bb41052d98184d0f108857ef4024916), ROM_BIOS(1) )
	ROMX_LOAD( "cld-fc.u5", 0x2000, 0x2000, CRC(0bb7426f) SHA1(afa6c5c6bb175b05b0a39615b425fd6a542e2da8), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *msd_sd1_device::device_rom_region() const
{
	return ROM_NAME( msdsd1 );
}

const tiny_rom_entry *msd_sd2_device::device_rom_region() const
{
	return ROM_NAME( msdsd2 );
}

const tiny_rom_entry *msd_sd1_ieee488_device::device_rom_region() const
{
	return ROM_NAME( msdsd1 );
}

const tiny_rom_entry *msd_sd2_ieee488_device::device_rom_region() const
{
	return ROM_NAME( msdsd2 );
}


//-------------------------------------------------
//  ADDRESS_MAP( mem_map )
//-------------------------------------------------

void msd_sd_device_base::common_map(address_map &map)
{
	map(0x4000, 0x57ff).ram();
	map(0xa000, 0xa000).mirror(0x1fff).w(FUNC(msd_sd_device_base::latch_w));
	map(0xc000, 0xffff).rom().region("rom", 0);
}


void msd_sd1_device::mem_map(address_map &map)
{
	common_map(map);
	map(0x5800, 0x5fff).ram();
}

void msd_sd1_ieee488_device::mem_map(address_map &map)
{
	common_map(map);
	map(0x5800, 0x5fff).ram();
}


//-------------------------------------------------
//  latch_w - U25 output latch
//-------------------------------------------------

void msd_sd_device_base::latch_w(u8 data)
{
	m_latch = data;

	for (int i = 0; i < 3; i++)
		m_leds[i] = BIT(data, i);

	m_ga->ds_w((data >> 4) & 0x03);
}


//-------------------------------------------------
//  status_r - U22 status buffer
//-------------------------------------------------

u8 msd_sd_device_base::status_r()
{
	u8 data = ~(bus_address() - 8) & 0x03;
	int trk00 = 1;
	int wpt = 1;

	for (int i = 0; i < 2; i++)
	{
		floppy_image_device *floppy = m_floppy[i].found() ? m_floppy[i]->get_device() : nullptr;

		if (floppy && BIT(m_pd, i))
		{
			trk00 &= floppy->trk00_r();
			wpt &= !floppy->wpt_r();
		}
	}

	data |= wpt << 2;
	data |= trk00 << 3;

	return data | 0xf0;
}


//-------------------------------------------------
//  pa_r - port A pull inputs
//-------------------------------------------------

u8 msd_sd_device_base::pa_r()
{
	if (talker())
		return 0xff;

	if (!iec_sample_ready())
		return 0xff;

	return 0xcf | (iec_data_r() << 4) | (iec_clk_r() << 5);
}

void msd_sd_device_base::pa_w(u8 data)
{
	m_pa = data;
	update_bus();
}


//-------------------------------------------------
//  pb_r - port B inputs
//-------------------------------------------------

u8 msd_sd_device_base::pb_r()
{
	u8 data = 0xff;

	if (!BIT(m_latch, 3))
		data &= status_r();

	if (!BIT(m_pd, 2) && !talker())
		data &= ieee_dio_r();

	if (!BIT(m_pd, 7))
		data &= m_yb;

	return data;
}

void msd_sd_device_base::pb_w(u8 data)
{
	m_pb = data;
	m_ga->yb_w(data);
	update_bus();
}


//-------------------------------------------------
//  pc_r - port C inputs
//-------------------------------------------------

u8 msd_sd_device_base::pc_r()
{
	u8 data = 0xff;

	if (talker())
	{
		if (!iec_sample_ready())
			return 0xff;

		data &= ~0x3c;
		data |= ieee_nrfd_r() << 2;
		data |= ieee_ndac_r() << 3;
		data |= iec_clk_r() << 4;
		data |= iec_data_r() << 5;
	}
	else
	{
		data &= ~0x03;
		data |= ieee_dav_r();
		data |= ieee_eoi_r() << 1;
	}

	return data;
}

void msd_sd_device_base::pc_w(u8 data)
{
	m_pc = data;
	update_bus();
}


//-------------------------------------------------
//  pd_w - port D outputs
//-------------------------------------------------

void msd_sd_device_base::pd_w(u8 data)
{
	u8 const prev = m_pd;
	m_pd = data;

	bool const motor = (data & 0x03) != 0;
	int drive = m_drive;
	if (BIT(data, 0))
		drive = 0;
	else if (BIT(data, 1) && m_floppy[1].found())
		drive = 1;

	if (drive != m_drive)
	{
		m_ga->mtr_w(0);
		m_drive = drive;
		m_ga->set_floppy(selected_floppy());
	}

	for (int i = 0; i < 2; i++)
	{
		floppy_image_device *floppy = m_floppy[i].found() ? m_floppy[i]->get_device() : nullptr;

		if (!floppy)
			continue;

		floppy->mon_w(!motor);

		if (BIT(data, i) && BIT(data, 5) && !BIT(prev, 5))
		{
			floppy->dir_w(!BIT(data, 4));
			for (int step = 0; step < 2; step++)
			{
				floppy->stp_w(0);
				floppy->stp_w(1);
			}
		}
	}

	m_ga->mtr_w(motor);
	m_ga->oe_w(BIT(data, 6));

	update_bus();
}


//-------------------------------------------------
//  selected_floppy - drive connected to the
//  read/write electronics
//-------------------------------------------------

floppy_image_device *msd_sd_device_base::selected_floppy()
{
	return m_floppy[m_drive]->get_device();
}


//-------------------------------------------------
//  ga_sync_w - SYNC (U19/U22D)
//-------------------------------------------------

void msd_sd_device_base::ga_sync_w(int state)
{
	m_maincpu->pa_w<6>(!state);
}


//-------------------------------------------------
//  ga_byte_w - BYTESTB (U28A) and BYTERQ (U30B)
//-------------------------------------------------

void msd_sd_device_base::ga_byte_w(int state)
{
	m_maincpu->pa_w<0>(!state);

	if (state && !m_byte)
	{
		m_maincpu->pa_w<7>(1);
		m_byterq_timer->adjust(attotime::from_double(0.7 * RES_K(47) * CAP_P(390))); // U30B 74LS221: R12 * C32
	}

	m_byte = state;
}

TIMER_CALLBACK_MEMBER(msd_sd_device_base::byterq_off)
{
	m_maincpu->pa_w<7>(0);
}


//-------------------------------------------------
//  ga_yb_w - read shift register (U20)
//-------------------------------------------------

void msd_sd_device_base::ga_yb_w(u8 data)
{
	m_yb = data;
}


//-------------------------------------------------
//  update_bus - drive the IEC and IEEE-488 lines
//-------------------------------------------------

void msd_sd_device_base::update_bus()
{
	bool const talk = talker();
	int const atnack = BIT(m_pa, 1);
	int const ieee_ack = BIT(m_pa, 2) && !m_ieee_atn && !atnack;
	int const iec_ack = BIT(m_pa, 3) && !m_iec_atn && !atnack;

	int const clk = talk ? BIT(m_pa, 5) : BIT(m_pc, 4);
	int const data = (talk ? BIT(m_pa, 4) : BIT(m_pc, 5)) && !iec_ack;

	iec_w(clk, data);

	u8 const dio = (talk && !BIT(m_pd, 2)) ? m_pb : 0xff;
	int const dav = talk ? BIT(m_pc, 0) : 1;
	int const eoi = talk ? BIT(m_pc, 1) : 1;
	int const nrfd = (talk ? 1 : BIT(m_pc, 2)) && !ieee_ack;
	int const ndac = (talk ? 1 : BIT(m_pc, 3)) && !ieee_ack;

	ieee_w(dio, dav, eoi, nrfd, ndac);
}


//-------------------------------------------------
//  iec_atn_w - serial ATN changed
//-------------------------------------------------

void msd_sd_device_base::iec_atn_w(int state)
{
	m_iec_atn = state;
	m_maincpu->pa_w<3>(!state);
	update_bus();
}


//-------------------------------------------------
//  ieee_atn_w - IEEE ATN changed
//-------------------------------------------------

void msd_sd_device_base::ieee_atn_w(int state)
{
	m_ieee_atn = state;
	m_maincpu->pa_w<2>(!state);
	update_bus();
}


//-------------------------------------------------
//  bus_reset_w - serial RESET or IEEE IFC changed
//-------------------------------------------------

void msd_sd_device_base::bus_reset_w(int state)
{
	if (!state)
		reset();
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

static void msdsd_floppies(device_slot_interface &device)
{
	device.option_add("525ssqd", FLOPPY_525_SSQD);
}

void msd_sd_device_base::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D64_FORMAT);
	fr.add(FLOPPY_G64_FORMAT);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void msd_sd_device_base::common_config(machine_config &config)
{
	R6511(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->pa_in_cb().set(FUNC(msd_sd_device_base::pa_r));
	m_maincpu->pa_out_cb().set(FUNC(msd_sd_device_base::pa_w));
	m_maincpu->pb_in_cb().set(FUNC(msd_sd_device_base::pb_r));
	m_maincpu->pb_out_cb().set(FUNC(msd_sd_device_base::pb_w));
	m_maincpu->pc_in_cb().set(FUNC(msd_sd_device_base::pc_r));
	m_maincpu->pc_out_cb().set(FUNC(msd_sd_device_base::pc_w));
	m_maincpu->pd_out_cb().set(FUNC(msd_sd_device_base::pd_w));

	C64H156(config, m_ga, 16_MHz_XTAL);
	m_ga->sync_callback().set(FUNC(msd_sd_device_base::ga_sync_w));
	m_ga->byte_callback().set(FUNC(msd_sd_device_base::ga_byte_w));
	m_ga->yb_wr_cb().set(FUNC(msd_sd_device_base::ga_yb_w));
}

void msd_sd_device_base::add_floppy(machine_config &config, unsigned index)
{
	floppy_connector &floppy(FLOPPY_CONNECTOR(config, m_floppy[index], msdsd_floppies, "525ssqd", msd_sd_device_base::floppy_formats));
	floppy.enable_sound(true);
	floppy.set_fixed(true);
}

void msd_sd1_device::device_add_mconfig(machine_config &config)
{
	common_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &msd_sd1_device::mem_map);
	add_floppy(config, 0);
}

void msd_sd2_device::device_add_mconfig(machine_config &config)
{
	common_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &msd_sd2_device::common_map);
	add_floppy(config, 0);
	add_floppy(config, 1);
}

void msd_sd1_ieee488_device::device_add_mconfig(machine_config &config)
{
	common_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &msd_sd1_ieee488_device::mem_map);
	add_floppy(config, 0);
}

void msd_sd2_ieee488_device::device_add_mconfig(machine_config &config)
{
	common_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &msd_sd2_ieee488_device::common_map);
	add_floppy(config, 0);
	add_floppy(config, 1);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msd_sd_device_base - constructor
//-------------------------------------------------

msd_sd_device_base::msd_sd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, "u7")
	, m_ga(*this, "ga")
	, m_floppy(*this, "%u", 0U)
	, m_leds(*this, "led%u", 0U)
	, m_byterq_timer(nullptr)
	, m_pa(0xff)
	, m_pb(0xff)
	, m_pc(0xff)
	, m_pd(0xff)
	, m_latch(0xff)
	, m_yb(0xff)
	, m_byte(1)
	, m_drive(0)
	, m_iec_atn(1)
	, m_ieee_atn(1)
{
}

msd_sd_iec_device_base::msd_sd_iec_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: msd_sd_device_base(mconfig, type, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
{
}

msd_sd_ieee488_device_base::msd_sd_ieee488_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: msd_sd_device_base(mconfig, type, tag, owner, clock)
	, device_ieee488_interface(mconfig, *this)
{
}

msd_sd1_device::msd_sd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msd_sd_iec_device_base(mconfig, MSD_SD1, tag, owner, clock)
{
}

msd_sd2_device::msd_sd2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msd_sd_iec_device_base(mconfig, MSD_SD2, tag, owner, clock)
{
}

msd_sd1_ieee488_device::msd_sd1_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msd_sd_ieee488_device_base(mconfig, GPIB_MSD_SD1, tag, owner, clock)
{
}

msd_sd2_ieee488_device::msd_sd2_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msd_sd_ieee488_device_base(mconfig, GPIB_MSD_SD2, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msd_sd_device_base::device_start()
{
	m_byterq_timer = timer_alloc(FUNC(msd_sd_device_base::byterq_off), this);

	m_ga->set_floppy(selected_floppy());
	m_ga->soe_w(1);

	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_pc));
	save_item(NAME(m_pd));
	save_item(NAME(m_latch));
	save_item(NAME(m_yb));
	save_item(NAME(m_byte));
	save_item(NAME(m_drive));
	save_item(NAME(m_iec_atn));
	save_item(NAME(m_ieee_atn));
}


//-------------------------------------------------
//  device_post_load - restore the drive connected
//  to the read/write electronics
//-------------------------------------------------

void msd_sd_device_base::device_post_load()
{
	m_ga->set_floppy(selected_floppy());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msd_sd_device_base::device_reset()
{
	m_maincpu->pa_w<2>(!m_ieee_atn);
	m_maincpu->pa_w<3>(!m_iec_atn);
}


//-------------------------------------------------
//  unconnected bus defaults
//-------------------------------------------------

bool msd_sd_device_base::iec_sample_ready() { return true; }
int msd_sd_device_base::iec_clk_r() { return 1; }
int msd_sd_device_base::iec_data_r() { return 1; }
void msd_sd_device_base::iec_w(int clk, int data) { }
u8 msd_sd_device_base::ieee_dio_r() { return 0xff; }
int msd_sd_device_base::ieee_dav_r() { return 1; }
int msd_sd_device_base::ieee_eoi_r() { return 1; }
int msd_sd_device_base::ieee_nrfd_r() { return 1; }
int msd_sd_device_base::ieee_ndac_r() { return 1; }
void msd_sd_device_base::ieee_w(u8 dio, int dav, int eoi, int nrfd, int ndac) { }


//-------------------------------------------------
//  serial bus
//-------------------------------------------------

int msd_sd_iec_device_base::bus_address() { return m_slot->get_address(); }
bool msd_sd_iec_device_base::iec_sample_ready() { return m_bus->sample_ready(*m_maincpu); }
int msd_sd_iec_device_base::iec_clk_r() { return m_bus->clk_r(); }
int msd_sd_iec_device_base::iec_data_r() { return m_bus->data_r(); }

void msd_sd_iec_device_base::cbm_iec_atn(int state)
{
	iec_atn_w(state);
}

void msd_sd_iec_device_base::cbm_iec_reset(int state)
{
	bus_reset_w(state);
}

void msd_sd_iec_device_base::iec_w(int clk, int data)
{
	m_bus->clk_w(this, clk);
	m_bus->data_w(this, data);
}


//-------------------------------------------------
//  IEEE-488 bus
//-------------------------------------------------

int msd_sd_ieee488_device_base::bus_address() { return m_slot->get_address(); }
u8 msd_sd_ieee488_device_base::ieee_dio_r() { return m_bus->dio_r(); }
int msd_sd_ieee488_device_base::ieee_dav_r() { return m_bus->dav_r(); }
int msd_sd_ieee488_device_base::ieee_eoi_r() { return m_bus->eoi_r(); }
int msd_sd_ieee488_device_base::ieee_nrfd_r() { return m_bus->nrfd_r(); }
int msd_sd_ieee488_device_base::ieee_ndac_r() { return m_bus->ndac_r(); }

void msd_sd_ieee488_device_base::ieee488_atn(int state)
{
	ieee_atn_w(state);
}

void msd_sd_ieee488_device_base::ieee488_ifc(int state)
{
	bus_reset_w(state);
}

void msd_sd_ieee488_device_base::ieee_w(u8 dio, int dav, int eoi, int nrfd, int ndac)
{
	m_bus->dio_w(this, dio);
	m_bus->dav_w(this, dav);
	m_bus->eoi_w(this, eoi);
	m_bus->nrfd_w(this, nrfd);
	m_bus->ndac_w(this, ndac);
}
