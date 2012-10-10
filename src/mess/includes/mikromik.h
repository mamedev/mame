#pragma once

#ifndef __MIKROMIKKO__
#define __MIKROMIKKO__


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/flopdrv.h"
#include "machine/am9517a.h"
#include "machine/i8212.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/upd7201.h"
#include "video/i8275.h"
#include "video/upd7220.h"
#include "sound/speaker.h"

#define SCREEN_TAG		"screen"
#define I8085A_TAG		"ic40"
#define I8212_TAG		"ic12"
#define I8237_TAG		"ic45"
#define I8253_TAG		"ic6"
#define UPD765_TAG		"ic15"
#define I8275_TAG		"ic59"
#define UPD7201_TAG		"ic11"
#define UPD7220_TAG		"ic101"

class mm1_state : public driver_device
{
public:
	mm1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, I8085A_TAG),
		  m_iop(*this, I8212_TAG),
		  m_dmac(*this, I8237_TAG),
		  m_pit(*this, I8253_TAG),
		  m_crtc(*this, I8275_TAG),
		  m_fdc(*this, UPD765_TAG),
		  m_mpsc(*this, UPD7201_TAG),
		  m_hgdc(*this, UPD7220_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_floppy0(*this, UPD765_TAG ":0:525qd"),
		  m_floppy1(*this, UPD765_TAG ":1:525qd"),
		  m_ram(*this, RAM_TAG),
		  m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8212_device> m_iop;
	required_device<am9517a_device> m_dmac;
	required_device<device_t> m_pit;
	required_device<device_t> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_device<upd7201_device> m_mpsc;
	required_device<upd7220_device> m_hgdc;
	required_device<device_t> m_speaker;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_video_ram;


	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( mmu_r );
	DECLARE_WRITE8_MEMBER( mmu_w );
	DECLARE_WRITE8_MEMBER( ls259_w );
	DECLARE_READ8_MEMBER( kb_r );
	DECLARE_WRITE_LINE_MEMBER( dma_hrq_changed );
	DECLARE_READ8_MEMBER( mpsc_dack_r );
	DECLARE_WRITE8_MEMBER( mpsc_dack_w );
	DECLARE_WRITE_LINE_MEMBER( tc_w );
	DECLARE_WRITE_LINE_MEMBER( dack3_w );
	DECLARE_WRITE_LINE_MEMBER( itxc_w );
	DECLARE_WRITE_LINE_MEMBER( irxc_w );
	DECLARE_WRITE_LINE_MEMBER( auxc_w );
	DECLARE_WRITE_LINE_MEMBER( drq2_w );
	DECLARE_WRITE_LINE_MEMBER( drq1_w );
	DECLARE_READ_LINE_MEMBER( dsra_r );
	DECLARE_PALETTE_INIT(mm1);
	DECLARE_READ8_MEMBER(fdc_dma_r);
	DECLARE_WRITE8_MEMBER(fdc_dma_w);

	void fdc_irq(bool state);
	void fdc_drq(bool state);

	void scan_keyboard();

	const UINT8 *m_mmu_rom;
	int m_a8;

	// keyboard state
	int m_sense;
	int m_drive;
	UINT8 m_keydata;
	const UINT8 *m_key_rom;

	// video state
	bitmap_ind16 m_bitmap;
	const UINT8 *m_char_rom;
	int m_llen;

	// serial state
	int m_intc;
	int m_rx21;
	int m_tx21;
	int m_rcl;

	// floppy state
	int m_recall;
	int m_dack3;
	int m_tc;
	UINT32 screen_update_mm1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(kbclk_tick);
};


//----------- defined in video/mikromik.c -----------

MACHINE_CONFIG_EXTERN( mm1m6_video );


#endif
