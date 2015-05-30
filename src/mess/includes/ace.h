// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/*****************************************************************************
 *
 * includes/ace.h
 *
 ****************************************************************************/

#pragma once

#ifndef ACE_H_
#define ACE_H_

#define Z80_TAG         "z0"
#define AY8910_TAG      "ay8910"
#define I8255_TAG       "i8255"
#define SP0256AL2_TAG   "ic1"
#define Z80PIO_TAG      "z80pio"
#define CENTRONICS_TAG  "centronics"
#define SCREEN_TAG      "screen"

class ace_state : public driver_device
{
public:
	ace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ppi(*this, I8255_TAG),
			m_speaker(*this, "speaker"),
			m_cassette(*this, "cassette"),
			m_centronics(*this, CENTRONICS_TAG),
			m_ram(*this, RAM_TAG),
			m_sp0256(*this, SP0256AL2_TAG),
			m_video_ram(*this, "video_ram"),
			m_char_ram(*this, "char_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<sp0256_device> m_sp0256;

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pa_w );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pb_w );
	DECLARE_READ8_MEMBER( ppi_pc_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_READ8_MEMBER( ppi_control_r );
	DECLARE_WRITE8_MEMBER( ppi_control_w );
	DECLARE_READ8_MEMBER( pio_pa_r );
	DECLARE_WRITE8_MEMBER( pio_pa_w );

	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_char_ram;
	UINT32 screen_update_ace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(set_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(clear_irq);
	DECLARE_READ8_MEMBER(pio_ad_r);
	DECLARE_READ8_MEMBER(pio_bd_r);
	DECLARE_READ8_MEMBER(pio_ac_r);
	DECLARE_READ8_MEMBER(pio_bc_r);
	DECLARE_WRITE8_MEMBER(pio_ad_w);
	DECLARE_WRITE8_MEMBER(pio_bd_w);
	DECLARE_WRITE8_MEMBER(pio_ac_w);
	DECLARE_WRITE8_MEMBER(pio_bc_w);
	DECLARE_READ8_MEMBER(sby_r);
	DECLARE_WRITE8_MEMBER(ald_w);
	DECLARE_SNAPSHOT_LOAD_MEMBER( ace );
};

#endif /* ACE_H_ */
