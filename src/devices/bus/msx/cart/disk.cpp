// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * MSX Floopy drive interface add-on cartridges
 *
 * Currently supported:
 * - AVT DPF-550 + DD 5.25" Drive - FD1770 or FD179x?
 * - Digital Design Drive DDX + DD 5.25" Drive - WD2793
 * - Microsol CDX-2 + DD 5.25" Drive - WD2793?
 * - Mitsubishi ML-30DC + ML-30FD DSDD 3.5" Drive - WD2793
 * - National FS-CF351 + FS-FD351 DSDD 3.5" Drive - MB8877A
 * - Panasonic FS-FD1 + DSDD 3.5" Drive - WD2793?
 * - Panasonic FS-FD1A with DSDD 3.5" Drive - TC8566F
 *                     - Rom label reads: "FDC BIOS V1.0 / COPYRIGHT MEI / 1987 DASFD1AA1"
 * - Philips NMS-1200 - Floppy interface - WD2793?
 * - Philips VY-0010 (Interface cartridge + 1 3.5" SS floppy drive)
 * - Sanyo MFD-001 + DS 5.25" Drive - MB8877?
 * - Sharp Epcom HB-3600 + HB-6000 DD 5.25" Drive - WD2793
 * - Sony HBD-20W + DSDD 3.5" Drive - WD2793
 * - Sony HBD-50 + SSDD 3.5" Drive - WD2793
 *   - Missing version SNYJX130
 * - Sony HBD-F1 + DSDD 3.5" Drive - WD2793
 * - Talent DPF-550 + SS 5.25" Drive - WD1770
 * - Toshiba HX-F101PE + SS 3.5" Drive - WD2793
 * - Yamaha FD-03 + SSDD 3.5" Floppy drive - FD1793 (according to service manual)
 * - Yamaha FD-051 + FD-05 DSDD 3.5" Drive - WD2793
 *   - Loading seems slower than other units
 *
 * Not supported yet:
 * - Canon VF-100 - DSDD 3.5" Floppy drive + interface + 1 floppy disk containing MSX-DOS
 * - Talent DPF-555 - WD1772 - DSDD 5.25" Floppy drive (360KB) plus interface (manufactured by Daewoo)
 *                  - Rom label markings: MSX DISK / DPF 555D
 *
 * Drive only:
 * - Philps VY-0011 - 3.5" SSDD Floppy drive
 * - Talent DPF-560 - DSDD 5.25" Floppy drive
 *
 * To be investigated:
 * - Daewoo CPF-350C - DD 3.5" Floppy drive
 * - Daewoo CPF-360C - DD 3.5" Floppy drive
 * - Daewoo MPF-550 - DSDD 5.25" Floppy drive + interface
 * = Daewoo MPF-560 - DSDD 5.25" Floppy drive
 * - DMX Interface para drive - Interface + 1 floppy disk containg MSX-DOS 1.0
 * - Fenner FD-300 - DSDD 3.5" Floppy drive
 * - Fenner FD-400 - Floppy interface for FD-300
 * - Hitachi MPF-310CH - DSDD Floppy drive
 * - hitachi MPC-310CH - Interface for MPF-310CH
 * - JVC HC-F303 - Floppy drive
 * - Philips NMS-9111 - 3.5" Floppy drive
 * - Philips NMS-9113 - 3.5" Floppy drive
 * - Sakir AFD-01 - SSDD 3.5" Floppy drive
 * - Sanyo MFD-002 - 360KB 5.25" Floppy drive (2nd drive for MFD-001?)
 * - Sanyo MFD-25FD - DSDD 3.5" Floppy drive
 * - Sanyo MFD-35 - SSDD 3.5" Floppy drive + interface
 * - Sony HBD-100 - SSDD 3.5" Floppy drive
 * - Sony HBD-30X/30W - DSDD 3.5" drive
 * - Sony HBX-30 (interface only, meant for 30W) - WD2793
 * - Spectravideo SVI-213 - MB8877A - Floppy interface for SVI-707
 * - Spectravideo SVI-707 - MB8877A - 5.25" SSDD? drive (320KB) - There seem to be 2 ROMs on the PCB, apparently one is for MSX and one is for CP/M operation?
 *                        - See https://plus.google.com/photos/115644813183575095400/albums/5223347091895442113?banner=pwa
 * - Spectravideo SVI-717 - Interface for 2 drives?
 * - Spectravideo SVI-787 - SSDD 3.5" Floppy drive
 * - Spectravideo SVI-801 - Interface
 * - Toshiba HX-F100 - Interface + SSDD 3.5" Floppy drive
 * - Toshiba HX-F101 - Interface + SSDD or DSDD 3.5" Floppy drive
 * - Yamaha FD-01 - Interface + SSDD 3.5" Floppy drive
 * - Other models:
 *   - ACVS 3.5" Floppy drive interface
 *   - Tradeco floppy interface
 *   - Angeisa 3.5" Floppy drive
 *   - Angeisa 5.25" 360KB Floppy drive
 *   - Angeisa 5.25" 720KB Floppy drive
 *   - Angeisa floppy drive interface
 *   - Datagame floppy drive interface
 *   - Digital Design DSDD 3.5" Floppy drive
 *   - Digital Design 5.25" 360KB Floppy drive
 *   - Digital Design 5.25" 720KB Floppy drive
 *   - Digital Design floppy drive interface
 *   - DMX 3.5" Floppy drive
 *   - DMX floppy drive interface
 *   - Liftron 3.5" Floppy drive
 *   - Liftron floppy drive interface
 *   - Microsol DRX-180 5.25" Floppy drive FS
 *   - Microsol DRX-360 5.25" Floppy drive FD
 *   - Microsol DRX-720 5.25" Floppy drive 80 track (720KB)
 *   - Microsol CDX-1 floppy interface
 *   - Racidata 3.5" Floppy drive
 *   - Racidata 5.25" Floppy drive
 *   - Racidata floppy interface
 *   - Sileman Triton-s 3.5" FS Floppy drive
 *   - Sileman Triton-d 3.5" FD Floppy drive
 *   - Talent TPF-723 5.25" Floppy drive
 *   - Talent TPF-725 5.25" Flpppy drive
 *   - Technohead Leopard 3.5" Floppy drive
 *   - Technohead Leopard 5.25" Floppy drive
 *   - Technohead floppy interface
 * - More??
 *
 * Several model references found in Vitropedia (ISBN 9781409212774)
 *
 ****************************************************************************/

#include "emu.h"
#include "disk.h"
#include "slotoptions.h"

#include "bus/msx/slot/cartridge.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/upd765.h"

#include "softlist_dev.h"

#include "formats/dmk_dsk.h"
#include "formats/msx_dsk.h"


DECLARE_DEVICE_TYPE(MSX_CART_AVDPF550, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_CDX2,     msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_DDX3,     msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_FD03,     msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_FD051,    msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_FSCF351,  msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_FSFD1,    msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_FSFD1A,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_HB3600,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_HBD20W,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_HBD50,    msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_HBDF1,    msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_HXF101PE, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_MFD001,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_ML30DC,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_NMS1200,  msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_TADPF550, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_VY0010,   msx_cart_interface)


void msx_cart_disk_register_options(device_slot_interface &device)
{
	using namespace bus::msx::cart;
	device.option_add_internal(slotoptions::DISK_AVDPF550, MSX_CART_AVDPF550);
	device.option_add_internal(slotoptions::DISK_CDX2,     MSX_CART_CDX2);
	device.option_add_internal(slotoptions::DISK_DDX3,     MSX_CART_DDX3);
	device.option_add_internal(slotoptions::DISK_FD03,     MSX_CART_FD03);
	device.option_add_internal(slotoptions::DISK_FD051,    MSX_CART_FD051);
	device.option_add_internal(slotoptions::DISK_FSCF351,  MSX_CART_FSCF351);
	device.option_add_internal(slotoptions::DISK_FSFD1,    MSX_CART_FSFD1);
	device.option_add_internal(slotoptions::DISK_FSFD1A,   MSX_CART_FSFD1A);
	device.option_add_internal(slotoptions::DISK_HB3600,   MSX_CART_HB3600);
	device.option_add_internal(slotoptions::DISK_HBD20W,   MSX_CART_HBD20W);
	device.option_add_internal(slotoptions::DISK_HBD50,    MSX_CART_HBD50);
	device.option_add_internal(slotoptions::DISK_HBDF1,    MSX_CART_HBDF1);
	device.option_add_internal(slotoptions::DISK_HXF101PE, MSX_CART_HXF101PE);
	device.option_add_internal(slotoptions::DISK_MFD001,   MSX_CART_MFD001);
	device.option_add_internal(slotoptions::DISK_ML30DC,   MSX_CART_ML30DC);
	device.option_add_internal(slotoptions::DISK_NMS1200,  MSX_CART_NMS1200);
	device.option_add_internal(slotoptions::DISK_TADPF550, MSX_CART_TADPF550);
	device.option_add_internal(slotoptions::DISK_VY0010,   MSX_CART_VY0010);
}


namespace {

void msx_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}



class msx_cart_disk_device : public device_t, public msx_cart_interface
{
public:
	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	msx_cart_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int fdc_regs_start_page, int fdc_regs_end_page)
		: device_t(mconfig, type, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy(nullptr)
		, m_fdc_regs_start_page(fdc_regs_start_page)
		, m_fdc_regs_end_page(fdc_regs_end_page)
	{ }

	static void floppy_formats(format_registration &fr);
	void softlist_35(machine_config &config);
	void softlist_525(machine_config &config);
	template <bool Is35, bool IsDS> void add_floppy_mconfig(machine_config &config);

	static constexpr int PAGE0 = 0;
	static constexpr int PAGE1 = 1;
	static constexpr int PAGE2 = 2;
	static constexpr int PAGE3 = 3;
	static constexpr bool F35 = true;
	static constexpr bool F525 = false;
	static constexpr bool DS = true;
	static constexpr bool SS = false;
	static constexpr bool FORCE_READY = true;

	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	int m_fdc_regs_start_page;
	int m_fdc_regs_end_page;
};

void msx_cart_disk_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MSX_FORMAT);
	fr.add(FLOPPY_DMK_FORMAT);
}

std::error_condition msx_cart_disk_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_disk_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x4000)
	{
		message = "msx_cart_disk_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	return std::error_condition();
}

void msx_cart_disk_device::softlist_35(machine_config &config)
{
	// Attach software lists
	// We do not know in what kind of machine the user has inserted the floppy interface
	// so we list all msx floppy software lists.
	//
	SOFTWARE_LIST(config, "msx2_flop_list").set_original("msx2_flop");
	SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
}

void msx_cart_disk_device::softlist_525(machine_config &config)
{
	// Attach software lists
	SOFTWARE_LIST(config, "msx1_flop_525_list").set_compatible("msx1_flop_525");
}

template <bool Is35, bool IsDS>
void msx_cart_disk_device::add_floppy_mconfig(machine_config &config)
{
	FLOPPY_CONNECTOR(config, "fdc:0", msx_floppies, Is35 ? (IsDS ? "35dd" : "35ssdd") : (IsDS ? "525dd" : "525ssdd"), msx_cart_disk_device::floppy_formats).enable_sound(true);
	if (Is35)
		softlist_35(config);
	else
		softlist_525(config);
}




class disk_tc8566_device : public msx_cart_disk_device
{
protected:
	disk_tc8566_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: msx_cart_disk_device(mconfig, type, tag, owner, clock, PAGE1, PAGE2)
		, m_fdc(*this, "fdc")
	{ }

	required_device<tc8566af_device> m_fdc;
};



class disk_wd_device : public msx_cart_disk_device
{
protected:
	disk_wd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int fdc_regs_start_page = PAGE1, int fdc_regs_end_page = PAGE2)
		: msx_cart_disk_device(mconfig, type, tag, owner, clock, fdc_regs_start_page, fdc_regs_end_page)
		, m_fdc(*this, "fdc")
	{ }

	required_device<wd_fdc_device_base> m_fdc;

	virtual void device_reset() override
	{
		m_fdc->dden_w(false);
	}

	template <bool Is35, bool IsDS, typename FDCType> void add_mconfig(machine_config &config, FDCType &&type)
	{
		add_mconfig<Is35, IsDS, false>(config, type);
	}

	template <bool Is35, bool IsDS, bool ForceReady, typename FDCType> void add_mconfig(machine_config &config, FDCType &&type)
	{
		std::forward<FDCType>(type)(config, m_fdc, 4_MHz_XTAL / 4);
		if (ForceReady)
			m_fdc->set_force_ready(true);
		add_floppy_mconfig<Is35, IsDS>(config);
	}
};



class disk_type1_device : public disk_wd_device
{
public:
	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	disk_type1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int fdc_regs_start_page = PAGE1, int fdc_regs_end_page = PAGE2)
		: disk_wd_device(mconfig, type, tag, owner, clock, fdc_regs_start_page, fdc_regs_end_page)
		, m_led(*this, "led0")
		, m_side_control(0)
		, m_control(0)
	{ }

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	u8 side_control_r();
	u8 control_r();
	u8 status_r();
	void set_side_control(u8 data);
	void set_control(u8 data);

	output_finder<> m_led;
	u8 m_side_control;
	u8 m_control;
};

void disk_type1_device::device_start()
{
	m_led.resolve();

	save_item(NAME(m_side_control));
	save_item(NAME(m_control));
}

std::error_condition disk_type1_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_wd_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	for (int i = m_fdc_regs_start_page; i <= m_fdc_regs_end_page; i++)
	{
		const offs_t base = 0x4000 * i;
		page(i)->install_read_handler(base + 0x3ff8, base + 0x3ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
		page(i)->install_read_handler(base + 0x3ff9, base + 0x3ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
		page(i)->install_read_handler(base + 0x3ffa, base + 0x3ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
		page(i)->install_read_handler(base + 0x3ffb, base + 0x3ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
		page(i)->install_read_handler(base + 0x3ffc, base + 0x3ffc, emu::rw_delegate(*this, FUNC(disk_type1_device::side_control_r)));
		page(i)->install_read_handler(base + 0x3ffd, base + 0x3ffd, emu::rw_delegate(*this, FUNC(disk_type1_device::control_r)));
		page(i)->install_read_handler(base + 0x3fff, base + 0x3fff, emu::rw_delegate(*this, FUNC(disk_type1_device::status_r)));
		page(i)->install_write_handler(base + 0x3ff8, base + 0x3ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
		page(i)->install_write_handler(base + 0x3ff9, base + 0x3ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
		page(i)->install_write_handler(base + 0x3ffa, base + 0x3ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
		page(i)->install_write_handler(base + 0x3ffb, base + 0x3ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
		page(i)->install_write_handler(base + 0x3ffc, base + 0x3ffc, emu::rw_delegate(*this, FUNC(disk_type1_device::set_side_control)));
		page(i)->install_write_handler(base + 0x3ffd, base + 0x3ffd, emu::rw_delegate(*this, FUNC(disk_type1_device::set_control)));
	}

	return std::error_condition();
}

void disk_type1_device::device_post_load()
{
	set_control(m_control);
}

void disk_type1_device::set_control(u8 data)
{
	u8 old_m_control = m_control;

	m_control = data;

	switch (m_control & 0x03)
	{
		case 0:
		case 2:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
			break;

		case 1:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
			break;

		default:
			m_floppy = nullptr;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w((m_control & 0x80) ? 0 : 1);
		m_floppy->ss_w(m_side_control & 0x01);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led = BIT(~m_control, 6);
	}
}

void disk_type1_device::set_side_control(u8 data)
{
	m_side_control = data;

	if (m_floppy)
	{
		m_floppy->ss_w(m_side_control & 0x01);
	}
}

u8 disk_type1_device::side_control_r()
{
	return 0xfe | (m_side_control & 0x01);
}

u8 disk_type1_device::control_r()
{
	return (m_control & 0x83) | 0x78;
}

u8 disk_type1_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
}



class fd051_device : public disk_type1_device
{
public:
	fd051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_FD051, tag, owner, clock, PAGE0, PAGE3)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS, FORCE_READY>(config, WD2793);
	}
};



class fsfd1_device : public disk_type1_device
{
public:
	fsfd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_FSFD1, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS>(config, WD2793);
	}
};



class hb3600_device : public disk_type1_device
{
public:
	hb3600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_HB3600, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F525, DS, FORCE_READY>(config, WD2793);
	}
};



class hbd20w_device : public disk_type1_device
{
public:
	hbd20w_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_HBD20W, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS>(config, WD2793);
	}
};



class hbd50_device : public disk_type1_device
{
public:
	hbd50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_HBD50, tag, owner, clock, PAGE0, PAGE3)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, SS>(config, WD2793);
	}
};



class hbdf1_device : public disk_type1_device
{
public:
	hbdf1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_HBDF1, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS>(config, WD2793);
	}
};



class ml30dc_device : public disk_type1_device
{
public:
	ml30dc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_ML30DC, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS, FORCE_READY>(config, WD2793);
		// Optional second drive in the same enclosure
		//FLOPPY_CONNECTOR(config, "fdc:1", msx_floppies, "35dd", msx_cart_disk_device::floppy_formats);
	}
};



class nms1200_device : public disk_type1_device
{
public:
	nms1200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_NMS1200, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS, FORCE_READY>(config, WD2793);
	}
};



class vy0010_device : public disk_type1_device
{
public:
	vy0010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type1_device(mconfig, MSX_CART_VY0010, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, SS, FORCE_READY>(config, WD2793);
	}
};



class disk_type2_device : public disk_wd_device
{
public:
	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	disk_type2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int fdc_regs_start_page = PAGE1, int fdc_regs_end_page = PAGE2)
		: disk_wd_device(mconfig, type, tag, owner, clock, fdc_regs_start_page, fdc_regs_end_page)
		, m_led(*this, "led0")
		, m_control(0)
	{ }

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	void set_control(u8 data);
	u8 status_r();

	output_finder<> m_led;
	u8 m_control;
};

void disk_type2_device::device_start()
{
	m_led.resolve();

	save_item(NAME(m_control));
}

void disk_type2_device::device_post_load()
{
	set_control(m_control);
}

std::error_condition disk_type2_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_wd_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	for (int i = m_fdc_regs_start_page; i <= m_fdc_regs_end_page; i++)
	{
		const offs_t base = 0x4000 * i;
		page(i)->install_read_handler(base + 0x3fb8, base + 0x3fb8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
		page(i)->install_read_handler(base + 0x3fb9, base + 0x3fb9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
		page(i)->install_read_handler(base + 0x3fba, base + 0x3fba, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
		page(i)->install_read_handler(base + 0x3fbb, base + 0x3fbb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
		page(i)->install_read_handler(base + 0x3fbc, base + 0x3fbc, emu::rw_delegate(*this, FUNC(disk_type2_device::status_r)));
		page(i)->install_write_handler(base + 0x3fb8, base + 0x3fb8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
		page(i)->install_write_handler(base + 0x3fb9, base + 0x3fb9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
		page(i)->install_write_handler(base + 0x3fba, base + 0x3fba, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
		page(i)->install_write_handler(base + 0x3fbb, base + 0x3fbb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
		page(i)->install_write_handler(base + 0x3fbc, base + 0x3fbc, emu::rw_delegate(*this, FUNC(disk_type2_device::set_control)));
	}

	return std::error_condition();
}

void disk_type2_device::set_control(u8 data)
{
	uint8_t old_m_control = m_control;

	m_control = data;

	switch (m_control & 3)
	{
		case 1:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
			break;

		case 2:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
			break;

		default:
			m_floppy = nullptr;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w((m_control & 0x08) ? 0 : 1);
		m_floppy->ss_w((m_control & 0x04) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led = BIT(~m_control, 6);
	}
}

u8 disk_type2_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}



class fscf351_device : public disk_type2_device
{
public:
	fscf351_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type2_device(mconfig, MSX_CART_FSCF351, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F35, DS, FORCE_READY>(config, MB8877);
	}
};



class tadpf550_device : public disk_type2_device
{
public:
	tadpf550_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type2_device(mconfig, MSX_CART_TADPF550, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		WD1770(config, m_fdc, 8_MHz_XTAL);
		add_floppy_mconfig<F525, SS>(config);
	}
};



class disk_type5_device : public disk_wd_device
{
public:
	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	disk_type5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: disk_wd_device(mconfig, type, tag, owner, clock)
		, m_control(0)
	{ }

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	void control_w(u8 control);
	virtual u8 status_r();

	u8 m_control;
};

void disk_type5_device::device_start()
{
	save_item(NAME(m_control));
}

std::error_condition disk_type5_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_wd_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());

	// Install IO read/write handlers
	io_space().install_write_handler(0xd0, 0xd0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	io_space().install_write_handler(0xd1, 0xd1, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	io_space().install_write_handler(0xd2, 0xd2, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	io_space().install_write_handler(0xd3, 0xd3, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	io_space().install_write_handler(0xd4, 0xd4, emu::rw_delegate(*this, FUNC(disk_type5_device::control_w)));
	io_space().install_read_handler(0xd0, 0xd0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	io_space().install_read_handler(0xd1, 0xd1, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	io_space().install_read_handler(0xd2, 0xd2, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	io_space().install_read_handler(0xd3, 0xd3, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	io_space().install_read_handler(0xd4, 0xd4, emu::rw_delegate(*this, FUNC(disk_type5_device::status_r)));

	return std::error_condition();
}

void disk_type5_device::device_post_load()
{
	control_w(m_control);
}

void disk_type5_device::control_w(u8 control)
{
	m_control = control;

	switch (m_control & 0x0f)
	{
	case 0x01:
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
		break;

	case 0x02:
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
		break;

	default:
		m_floppy = nullptr;
		break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_control, 5) ? 0 : 1);
		m_floppy->ss_w(BIT(m_control, 4) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);
}

u8 disk_type5_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}


class cdx2_device : public disk_type5_device
{
public:
	cdx2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type5_device(mconfig, MSX_CART_CDX2, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F525, DS>(config, WD2793);
	}
};


class ddx3_device : public disk_type5_device
{
public:
	ddx3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_type5_device(mconfig, MSX_CART_DDX3, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		add_mconfig<F525, DS, FORCE_READY>(config, WD2793);
	}
};



class fsfd1a_device : public disk_tc8566_device
{
public:
	fsfd1a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_tc8566_device(mconfig, MSX_CART_FSFD1A, tag, owner, clock)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override
	{
		TC8566AF(config, m_fdc, 16'000'000);
		add_floppy_mconfig<F35, DS>(config);
	}
};

std::error_condition fsfd1a_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_tc8566_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	for (int i = m_fdc_regs_start_page; i <= m_fdc_regs_end_page; i++)
	{
		const offs_t base = 0x4000 * i;
		page(i)->install_read_handler(base + 0x3ffa, base + 0x3ffa, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::msr_r)));
		page(i)->install_read_handler(base + 0x3ffb, base + 0x3ffb, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_r)));
		page(i)->install_write_handler(base + 0x3ff8, base + 0x3ff8, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::dor_w)));
		page(i)->install_write_handler(base + 0x3ff9, base + 0x3ff9, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::cr1_w)));
		page(i)->install_write_handler(base + 0x3ffb, base + 0x3ffb, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_w)));
	}

	return std::error_condition();
}



class fd03_device : public disk_wd_device
{
public:
	fd03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_wd_device(mconfig, MSX_CART_FD03, tag, owner, clock)
		, m_led(*this, "led0")
		, m_control(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	u8 status_r();
	u8 dskchg_r();
	void set_control(u8 data);
	void dskchg_w(u8 data);

	output_finder<> m_led;
	u8 m_control;
};

void fd03_device::device_add_mconfig(machine_config &config)
{
	add_mconfig<F35, SS>(config, FD1793);
}

void fd03_device::device_start()
{
	m_led.resolve();

	save_item(NAME(m_control));
}

std::error_condition fd03_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "fd03_device:: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x4000 && cart_rom_region()->bytes() != 0x8000)
	{
		message = "fd03_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	if (cart_rom_region()->bytes() >= 0x8000)
		page(2)->install_rom(0x8000,0xbfff, cart_rom_region()->base() + 0x4000);

	for (int i = m_fdc_regs_start_page; i <= m_fdc_regs_end_page; i++)
	{
		const offs_t base = 0x4000 * i;
		page(i)->install_read_handler(base + 0x3fc0, base + 0x3fc0, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
		page(i)->install_read_handler(base + 0x3fc1, base + 0x3fc1, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
		page(i)->install_read_handler(base + 0x3fc2, base + 0x3fc2, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
		page(i)->install_read_handler(base + 0x3fc3, base + 0x3fc3, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
		page(i)->install_read_handler(base + 0x3fe0, base + 0x3fef, emu::rw_delegate(*this, FUNC(fd03_device::status_r)));
		page(i)->install_read_handler(base + 0x3ff0, base + 0x3fff, emu::rw_delegate(*this, FUNC(fd03_device::dskchg_r)));
		page(i)->install_write_handler(base + 0x3fc0, base + 0x3fc0, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
		page(i)->install_write_handler(base + 0x3fc1, base + 0x3fc1, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
		page(i)->install_write_handler(base + 0x3fc2, base + 0x3fc2, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
		page(i)->install_write_handler(base + 0x3fc3, base + 0x3fc3, 0, 0x001c, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
		page(i)->install_write_handler(base + 0x3fe0, base + 0x3fef, emu::rw_delegate(*this, FUNC(fd03_device::set_control)));
		page(i)->install_write_handler(base + 0x3ff0, base + 0x3fff, emu::rw_delegate(*this, FUNC(fd03_device::dskchg_w)));
	}

	return std::error_condition();
}

void fd03_device::device_post_load()
{
	set_control(m_control);
}

void fd03_device::set_control(u8 data)
{
	u8 old_m_control = m_control;

	m_control = data;

	switch (m_control & 0x03)
	{
		case 1:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
			break;

		case 2:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
			break;

		default:
			m_floppy = nullptr;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_control, 2) ? 0 : 1);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led = BIT(~m_control, 6);
	}
}

u8 fd03_device::status_r()
{
	// bit 0 - drive 0 ready / has media
	// bit 1 - drive 1 ready / has media
	// bit 2 - diskchange drive 0
	// bit 3 - diskchange drive 1
	u8 result = 0x30;
	if (!m_floppy0)
		result |= 0x01;
	if (!m_floppy1)
		result |= 0x02;
	if (m_floppy0 && m_floppy0->get_device()->dskchg_r())
		result |= 0x04;
	if (m_floppy1 && m_floppy1->get_device()->dskchg_r())
		result |= 0x08;

	return result | (m_fdc->intrq_r() ? 0x80 : 0) | (m_fdc->drq_r() ? 0x40 : 0);
}

u8 fd03_device::dskchg_r()
{
	if (m_floppy1)
		m_floppy1->get_device()->dskchg_w(0);

	return 0xff;
}

void fd03_device::dskchg_w(u8 data)
{
	if (m_floppy0)
		m_floppy0->get_device()->dskchg_w(0);
}



class hxf101pe_device : public disk_wd_device
{
public:
	hxf101pe_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_wd_device(mconfig, MSX_CART_HXF101PE, tag, owner, clock, PAGE1, PAGE1)
		, m_side_motor(0)
		, m_drive_select0(0)
		, m_drive_select1(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	void set_side_motor();
	void select_drive();
	u8 side_motor_r();
	u8 select0_r();
	u8 select1_r();
	u8 status_r();
	void side_motor_w(u8 data);
	void select0_w(u8 data);
	void select1_w(u8 data);

	u8 m_side_motor;
	u8 m_drive_select0;
	u8 m_drive_select1;
};

void hxf101pe_device::device_add_mconfig(machine_config &config)
{
	add_mconfig<F35, SS>(config, WD2793);
}

void hxf101pe_device::device_start()
{
	save_item(NAME(m_side_motor));
	save_item(NAME(m_drive_select0));
	save_item(NAME(m_drive_select1));
}

std::error_condition hxf101pe_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_wd_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(1)->install_read_handler(0x7ff0, 0x7ff0, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff1, 0x7ff1, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ff2, 0x7ff2, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ff3, 0x7ff3, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ff4, 0x7ff4, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::side_motor_r)));
	page(1)->install_read_handler(0x7ff5, 0x7ff5, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::select0_r)));
	page(1)->install_read_handler(0x7ff6, 0x7ff6, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::select1_r)));
	page(1)->install_read_handler(0x7ff7, 0x7ff7, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::status_r)));
	page(1)->install_write_handler(0x7ff0, 0x7ff0, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff1, 0x7ff1, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ff2, 0x7ff2, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ff3, 0x7ff3, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ff4, 0x7ff4, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::side_motor_w)));
	page(1)->install_write_handler(0x7ff5, 0x7ff5, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::select0_w)));
	page(1)->install_write_handler(0x7ff6, 0x7ff6, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(hxf101pe_device::select1_w)));

	return std::error_condition();
}

void hxf101pe_device::device_post_load()
{
	select_drive();
}

void hxf101pe_device::select_drive()
{
	if (m_drive_select1)
	{
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
		if (!m_floppy)
			m_drive_select1 = 0;
	}

	if (m_drive_select0)
	{
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
		if (!m_floppy)
			m_drive_select0 = 0;
	}

	m_fdc->set_floppy(m_floppy);

	set_side_motor();
}

void hxf101pe_device::set_side_motor()
{
	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_side_motor, 1) ? 0 : 1);
		m_floppy->ss_w(BIT(m_side_motor, 0));
	}
}

u8 hxf101pe_device::side_motor_r()
{
	// bit 0 = side control
	// bit 1 = motor control
	return 0xfc | m_side_motor;
}

u8 hxf101pe_device::select0_r()
{
	// This reads back a 1 in bit 0 if drive0 is present and selected
	return 0xfe | m_drive_select0;
}

u8 hxf101pe_device::select1_r()
{
	// This reads back a 1 in bit 0 if drive1 is present and selected
	return 0xfe | m_drive_select1;
}

u8 hxf101pe_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
}

void hxf101pe_device::side_motor_w(u8 data)
{
	// Side and motor control
	// bit 0 = side select
	// bit 1 = motor on/off
	m_side_motor = data;
	set_side_motor();
}

void hxf101pe_device::select0_w(u8 data)
{
	// bit 0 - select drive 0
	m_drive_select0 = data;
	select_drive();
}

void hxf101pe_device::select1_w(u8 data)
{
	// bit 1 - select drive 1
	m_drive_select1 = data;
	select_drive();
}



class mfd001_device : public disk_wd_device
{
public:
	mfd001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_wd_device(mconfig, MSX_CART_MFD001, tag, owner, clock)
		, m_control(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	void set_side_motor();
	void select_drive();
	u8 status_r();
	void control_w(u8 data);

	u8 m_control;
};

void mfd001_device::device_add_mconfig(machine_config &config)
{
	add_mconfig<F525, DS>(config, MB8877);
}

std::error_condition mfd001_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_wd_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());

	for (int i = m_fdc_regs_start_page; i <= m_fdc_regs_end_page; i++)
	{
		const offs_t base = 0x4000 * i;
		page(i)->install_read_handler(base + 0x3ff8, base + 0x3ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
		page(i)->install_read_handler(base + 0x3ff9, base + 0x3ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
		page(i)->install_read_handler(base + 0x3ffa, base + 0x3ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
		page(i)->install_read_handler(base + 0x3ffb, base + 0x3ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
		page(i)->install_read_handler(base + 0x3ffc, base + 0x3ffc, emu::rw_delegate(*this, FUNC(mfd001_device::status_r)));
		page(i)->install_write_handler(base + 0x3ff8, base + 0x3ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
		page(i)->install_write_handler(base + 0x3ff9, base + 0x3ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
		page(i)->install_write_handler(base + 0x3ffa, base + 0x3ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
		page(i)->install_write_handler(base + 0x3ffb, base + 0x3ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
		page(i)->install_write_handler(base + 0x3ffc, base + 0x3ffc, emu::rw_delegate(*this, FUNC(mfd001_device::control_w)));
	}

	return std::error_condition();
}

void mfd001_device::device_start()
{
	save_item(NAME(m_control));
}

void mfd001_device::device_post_load()
{
	select_drive();
}

void mfd001_device::select_drive()
{
	if (BIT(m_control, 0))
	{
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
	}

	if (BIT(m_control, 1))
	{
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
	}

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_floppy->ss_w(BIT(m_control, 2));
	}
}

u8 mfd001_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0x80 : 0) | (m_fdc->drq_r() ? 0x40 : 0);
}

void mfd001_device::control_w(u8 data)
{
	// Drive, side and motor control
	// bit 0 = select drive 0
	// bit 1 = select drive 1
	// bit 2 = side select
	// bit 3 = motor on/off
	m_control = data;
	select_drive();
}


class avdpf550_device : public disk_wd_device
{
public:
	avdpf550_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: disk_wd_device(mconfig, MSX_CART_AVDPF550, tag, owner, clock)
		, m_control(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	void control_w(u8 control);
	virtual u8 status_r();

	u8 m_control;
};

void avdpf550_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);
	add_floppy_mconfig<F525, SS>(config);
}

void avdpf550_device::device_start()
{
	save_item(NAME(m_control));
}

std::error_condition avdpf550_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = disk_wd_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());

	// Install IO read/write handlers
	io_space().install_write_handler(0xd0, 0xd0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	io_space().install_write_handler(0xd1, 0xd1, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	io_space().install_write_handler(0xd2, 0xd2, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	io_space().install_write_handler(0xd3, 0xd3, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	io_space().install_write_handler(0xd5, 0xd5, emu::rw_delegate(*this, FUNC(avdpf550_device::control_w)));
	io_space().install_read_handler(0xd0, 0xd0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	io_space().install_read_handler(0xd1, 0xd1, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	io_space().install_read_handler(0xd2, 0xd2, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	io_space().install_read_handler(0xd3, 0xd3, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	io_space().install_read_handler(0xd4, 0xd4, emu::rw_delegate(*this, FUNC(avdpf550_device::status_r)));

	return std::error_condition();
}

void avdpf550_device::device_post_load()
{
	control_w(m_control);
}

void avdpf550_device::control_w(u8 control)
{
	m_control = control;

	switch (m_control & 0x03)
	{
	case 0x01:
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
		break;

	case 0x02:
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
		break;

	default:
		m_floppy = nullptr;
		break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_floppy->ss_w(BIT(m_control, 2) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);
}

u8 avdpf550_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_AVDPF550, msx_cart_interface, avdpf550_device, "msx_cart_avdpf550", "MSX Cartridge - AVT DPF-550")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_CDX2,     msx_cart_interface, cdx2_device,     "msx_cart_cdx2",     "MSX Cartridge - CDX-2")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_DDX3,     msx_cart_interface, ddx3_device,     "msx_cart_ddx3",     "MSX Cartridge - DDX3.0")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FD03,     msx_cart_interface, fd03_device,     "msx_cart_fd03",     "MSX Cartridge - FD-03")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FD051,    msx_cart_interface, fd051_device,    "msx_cart_fd051",    "MSX Cartridge - FD-051")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FSFD1,    msx_cart_interface, fsfd1_device,    "msx_cart_fsfd1",    "MSX Cartridge - FS-FD1")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FSFD1A,   msx_cart_interface, fsfd1a_device,   "msx_cart_fsfd1a",   "MSX Cartridge - FS-FD1A")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FSCF351,  msx_cart_interface, fscf351_device,  "msx_cart_fscf351",  "MSX Cartridge - FS-CF351")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_HB3600,   msx_cart_interface, hb3600_device,   "msx_cart_hb3600",   "MSX Cartridge - HB-3600")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_HBD20W,   msx_cart_interface, hbd20w_device,   "msx_cart_hbd20w",   "MSX Cartridge - HBD-20W")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_HBD50,    msx_cart_interface, hbd50_device,    "msx_cart_hbd50",    "MSX Cartridge - HBD-50")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_HBDF1,    msx_cart_interface, hbdf1_device,    "msx_cart_hbdf1",    "MSX Cartridge - HBD-F1")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_HXF101PE, msx_cart_interface, hxf101pe_device, "msx_cart_hxf101pe", "MSX Cartridge - HX-F101PE")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MFD001,   msx_cart_interface, mfd001_device,   "msx_cart_mfd001",   "MSX Cartridge - MFD-001")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_ML30DC,   msx_cart_interface, ml30dc_device,   "msx_cart_ml30dc",   "MSX Cartridge - ML-30DC")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_NMS1200,  msx_cart_interface, nms1200_device,  "msx_cart_nms1200",  "MSX Cartridge - NMS-1200")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_TADPF550, msx_cart_interface, tadpf550_device, "msx_cart_tadpf550", "MSX Cartridge - Talent DPF-550")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_VY0010,   msx_cart_interface, vy0010_device,   "msx_cart_vy0010",   "MSX Cartridge - VY0010")
