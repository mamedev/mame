// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MSD SD-1/SD-2 Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MSDSD_H
#define MAME_BUS_CBMIEC_MSDSD_H

#pragma once

#include "cbmiec.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m6502/r6511.h"
#include "imagedev/floppy.h"
#include "machine/64h156.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> msd_sd_device_base

class msd_sd_device_base : public device_t
{
protected:
	// construction/destruction
	msd_sd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	virtual int bus_address() = 0;
	virtual bool iec_sample_ready();
	virtual int iec_clk_r();
	virtual int iec_data_r();
	virtual void iec_w(int clk, int data);
	virtual u8 ieee_dio_r();
	virtual int ieee_dav_r();
	virtual int ieee_eoi_r();
	virtual int ieee_nrfd_r();
	virtual int ieee_ndac_r();
	virtual void ieee_w(u8 dio, int dav, int eoi, int nrfd, int ndac);

	void iec_atn_w(int state);
	void ieee_atn_w(int state);
	void bus_reset_w(int state);
	void update_bus();

	void common_map(address_map &map) ATTR_COLD;
	void common_config(machine_config &config) ATTR_COLD;
	void add_floppy(machine_config &config, unsigned index) ATTR_COLD;

	required_device<r6511_device> m_maincpu;

private:
	static void floppy_formats(format_registration &fr);

	u8 pa_r();
	void pa_w(u8 data);
	u8 pb_r();
	void pb_w(u8 data);
	u8 pc_r();
	void pc_w(u8 data);
	void pd_w(u8 data);
	void latch_w(u8 data);

	u8 status_r();
	floppy_image_device *selected_floppy();
	void ga_sync_w(int state);
	void ga_byte_w(int state);
	void ga_yb_w(u8 data);
	TIMER_CALLBACK_MEMBER(byterq_off);

	bool talker() const { return BIT(m_pd, 3); }

	required_device<c64h156_device> m_ga;
	optional_device_array<floppy_connector, 2> m_floppy;
	output_finder<3> m_leds;
	emu_timer *m_byterq_timer;

	u8 m_pa;
	u8 m_pb;
	u8 m_pc;
	u8 m_pd;
	u8 m_latch;
	u8 m_yb;
	u8 m_byte;
	u8 m_drive;
	u8 m_iec_atn;
	u8 m_ieee_atn;
};


// ======================> msd_sd_iec_device_base

class msd_sd_iec_device_base : public msd_sd_device_base, public device_cbm_iec_interface
{
protected:
	msd_sd_iec_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_cbm_iec_interface implementation
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_reset(int state) override;

	virtual int bus_address() override;
	virtual bool iec_sample_ready() override;
	virtual int iec_clk_r() override;
	virtual int iec_data_r() override;
	virtual void iec_w(int clk, int data) override;
};


// ======================> msd_sd_ieee488_device_base

class msd_sd_ieee488_device_base : public msd_sd_device_base, public device_ieee488_interface
{
protected:
	msd_sd_ieee488_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_ieee488_interface implementation
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ifc(int state) override;

	virtual int bus_address() override;
	virtual u8 ieee_dio_r() override;
	virtual int ieee_dav_r() override;
	virtual int ieee_eoi_r() override;
	virtual int ieee_nrfd_r() override;
	virtual int ieee_ndac_r() override;
	virtual void ieee_w(u8 dio, int dav, int eoi, int nrfd, int ndac) override;
};


// ======================> msd_sd1_device

class msd_sd1_device : public msd_sd_iec_device_base
{
public:
	msd_sd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};


// ======================> msd_sd2_device

class msd_sd2_device : public msd_sd_iec_device_base
{
public:
	msd_sd2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type declarations
DECLARE_DEVICE_TYPE(MSD_SD1, msd_sd1_device)
DECLARE_DEVICE_TYPE(MSD_SD2, msd_sd2_device)


// ======================> msd_sd1_ieee488_device

class msd_sd1_ieee488_device : public msd_sd_ieee488_device_base
{
public:
	msd_sd1_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static auto parent_rom_device_type() { return &MSD_SD1; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};


// ======================> msd_sd2_ieee488_device

class msd_sd2_ieee488_device : public msd_sd_ieee488_device_base
{
public:
	msd_sd2_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static auto parent_rom_device_type() { return &MSD_SD2; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(GPIB_MSD_SD1, msd_sd1_ieee488_device)
DECLARE_DEVICE_TYPE(GPIB_MSD_SD2, msd_sd2_ieee488_device)


#endif // MAME_BUS_CBMIEC_MSDSD_H
