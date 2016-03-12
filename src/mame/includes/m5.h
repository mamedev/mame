// license:BSD-3-Clause
// copyright-holders:Curt Coder, Ales Dlabac
#ifndef __M5__
#define __M5__


#include "machine/z80ctc.h"
#include "imagedev/snapquik.h"


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
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_fd5cpu(*this, Z80_FD5_TAG),
			m_ppi(*this, I8255A_TAG),
			m_fdc(*this, UPD765_TAG),
			m_floppy0(*this, UPD765_TAG ":0:525dd"),
			m_cassette(*this, "cassette"),
			m_cart1(*this, "cartslot1"),
			m_cart2(*this, "cartslot2"),
			m_centronics(*this, CENTRONICS_TAG),
			m_ram(*this, RAM_TAG),
			m_reset(*this, "RESET"),
			m_DIPS(*this, "DIPS")
	{ }

	required_device<cpu_device> m_maincpu;
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

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( sts_r );
	DECLARE_WRITE8_MEMBER( com_w );
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

	// video state
//  const TMS9928a_interface *m_vdp_intf;

	int m_centronics_busy;
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

	DECLARE_DRIVER_INIT(pal);
	DECLARE_DRIVER_INIT(ntsc);
	DECLARE_WRITE_LINE_MEMBER(sordm5_video_interrupt_callback);

	// memory
	DECLARE_READ8_MEMBER( mem64KBI_r );
	DECLARE_WRITE8_MEMBER( mem64KBI_w );
	DECLARE_WRITE8_MEMBER( mem64KBF_w );
	DECLARE_WRITE8_MEMBER( mem64KRX_w );
	UINT8 m_ram_mode;
	UINT8 m_ram_type;
	memory_region *m_cart_rom;
	m5_cart_slot_device *m_cart_ram, *m_cart;

	// floppy state for fd5
	UINT8 m_fd5_data;
	UINT8 m_fd5_com;
	int m_intra;
	int m_ibfa;
	int m_obfa;

};


class brno_state : public m5_state
{
public:
	brno_state(const machine_config &mconfig, device_type type, const char *tag)
		: m5_state(mconfig, type, tag),

		m_fdc(*this, WD2797_TAG),
		m_floppy0(*this, WD2797_TAG":0"),
		m_floppy1(*this, WD2797_TAG":1")
		//  m_ramdisk(*this, RAMDISK)
	{ }


	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;



	DECLARE_READ8_MEMBER( mmu_r );
	DECLARE_WRITE8_MEMBER( mmu_w );
	DECLARE_READ8_MEMBER( ramsel_r );
	DECLARE_WRITE8_MEMBER(ramsel_w );
	DECLARE_READ8_MEMBER( romsel_r );
	DECLARE_WRITE8_MEMBER(romsel_w );

	DECLARE_READ8_MEMBER( fd_r );
	DECLARE_WRITE8_MEMBER( fd_w );
	DECLARE_FLOPPY_FORMATS( floppy_formats );


//  DECLARE_WRITE_LINE_MEMBER( wd2797_intrq_w );
//  DECLARE_WRITE_LINE_MEMBER( wd2797_drq_w );
//  DECLARE_WRITE_LINE_MEMBER( wd2797_index_callback);

	//required_device<ram_device> m_ramdisk;
	DECLARE_DRIVER_INIT(brno);
	DECLARE_SNAPSHOT_LOAD_MEMBER( brno );
//  DECLARE_DEVICE_IMAGE_LOAD_MEMBER(m5_cart);


	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT8 m_rambank; // bank #
	UINT8 m_ramcpu; //where Ramdisk bank is mapped
	bool m_romen;
	bool m_ramen;


	UINT8 m_rammap[16]; // memory map
};

#endif
