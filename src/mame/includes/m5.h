// license:BSD-3-Clause
// copyright-holders:Curt Coder, Ales Dlabac
#ifndef MAME_INCLUDES_M5_H
#define MAME_INCLUDES_M5_H

#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/wd_fdc.h" //brno mod
#include "machine/z80ctc.h"

#include "bus/centronics/ctronics.h"
#include "bus/m5/slot.h"


#define Z80_TAG         "ic17"
#define Z80CTC_TAG      "ic19"
#define SN76489AN_TAG   "ic15"
#define TMS9918A_TAG    "ic10"
#define TMS9929A_TAG    "ic10"
#define I8255A_TAG      "i8255a"
#define Z80_FD5_TAG     "z80fd5"
#define UPD765_TAG      "upd765"
#define CENTRONICS_TAG  "centronics"
#define SCREEN_TAG      "screen"
//brno mod
#define WD2797_TAG      "5f"
#define RAMDISK         "ramdisk"


class m5_state : public driver_device
{
public:
	m5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_ctc(*this, Z80CTC_TAG)
		, m_fd5cpu(*this, Z80_FD5_TAG)
		, m_ppi(*this, I8255A_TAG)
		, m_fdc(*this, UPD765_TAG)
		, m_floppy0(*this, UPD765_TAG ":0:525dd")
		, m_cassette(*this, "cassette")
		, m_cart1(*this, "cartslot1")
		, m_cart2(*this, "cartslot2")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_ram(*this, RAM_TAG)
		, m_reset(*this, "RESET")
		, m_DIPS(*this, "DIPS")
	{ }

	void m5(machine_config &config);
	void pal(machine_config &config);
	void ntsc(machine_config &config);

	void init_pal();
	void init_ntsc();

	DECLARE_WRITE_LINE_MEMBER(sordm5_video_interrupt_callback);

protected:
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	//I've changed following devices to optional since we have to remove them in BRNO mod (I don't know better solution)
	optional_device<cpu_device> m_fd5cpu;
	optional_device<i8255_device> m_ppi;
	optional_device<upd765a_device> m_fdc;
	optional_device<floppy_image_device> m_floppy0;
	required_device<cassette_image_device> m_cassette;
	optional_device<m5_cart_slot_device> m_cart1;
	optional_device<m5_cart_slot_device> m_cart2;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_ioport m_reset;
	optional_ioport m_DIPS;
	m5_cart_slot_device *m_cart_ram, *m_cart;

	DECLARE_READ8_MEMBER( sts_r );
	DECLARE_WRITE8_MEMBER( com_w );

	virtual void machine_start() override;
	virtual void machine_reset() override;
private:
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pa_w );
	DECLARE_WRITE8_MEMBER( ppi_pb_w );
	DECLARE_READ8_MEMBER( ppi_pc_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

	DECLARE_READ8_MEMBER( fd5_data_r );
	DECLARE_WRITE8_MEMBER( fd5_data_w );
	DECLARE_READ8_MEMBER( fd5_com_r );
	DECLARE_WRITE8_MEMBER( fd5_com_w );
	DECLARE_WRITE8_MEMBER( fd5_ctrl_w );
	DECLARE_WRITE8_MEMBER( fd5_tc_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

	// memory
	DECLARE_READ8_MEMBER( mem64KBI_r );
	DECLARE_WRITE8_MEMBER( mem64KBI_w );
	DECLARE_WRITE8_MEMBER( mem64KBF_w );
	DECLARE_WRITE8_MEMBER( mem64KRX_w );

	void fd5_io(address_map &map);
	void fd5_mem(address_map &map);
	void m5_io(address_map &map);
	void m5_mem(address_map &map);

	// video state
//  const TMS9928a_interface *m_vdp_intf;

	int m_centronics_busy;

	uint8_t m_ram_mode;
	uint8_t m_ram_type;
	memory_region *m_cart_rom;

	// floppy state for fd5
	uint8_t m_fd5_data;
	uint8_t m_fd5_com;
	int m_intra;
	int m_ibfa;
	int m_obfa;
};


class brno_state : public m5_state
{
public:
	brno_state(const machine_config &mconfig, device_type type, const char *tag)
		: m5_state(mconfig, type, tag)
		, m_fdc(*this, WD2797_TAG)
		, m_floppy0(*this, WD2797_TAG":0")
		, m_floppy1(*this, WD2797_TAG":1")
		//,  m_ramdisk(*this, RAMDISK)
	{ }

	void brno(machine_config &config);

	void init_brno();

private:
	DECLARE_READ8_MEMBER(mmu_r);
	DECLARE_WRITE8_MEMBER(mmu_w);
	DECLARE_READ8_MEMBER(ramsel_r);
	DECLARE_WRITE8_MEMBER(ramsel_w);
	DECLARE_READ8_MEMBER(romsel_r);
	DECLARE_WRITE8_MEMBER(romsel_w);

	DECLARE_READ8_MEMBER(fd_r);
	DECLARE_WRITE8_MEMBER(fd_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);


	//  DECLARE_WRITE_LINE_MEMBER( wd2797_intrq_w );
	//  DECLARE_WRITE_LINE_MEMBER( wd2797_drq_w );
	//  DECLARE_WRITE_LINE_MEMBER( wd2797_index_callback);

		//required_device<ram_device> m_ramdisk;
	DECLARE_SNAPSHOT_LOAD_MEMBER(brno);
	//  DECLARE_DEVICE_IMAGE_LOAD_MEMBER(m5_cart);

	void brno_io(address_map &map);
	void m5_mem_brno(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	uint8_t m_rambank; // bank #
	uint8_t m_ramcpu; //where Ramdisk bank is mapped
	bool m_romen;
	bool m_ramen;

	uint8_t m_rammap[16]; // memory map
};

#endif // MAME_INCLUDES_M5_H
