#pragma once

#ifndef ADAM_H_
#define ADAM_H_

#define Z80_TAG			"u1"
#define SN76489A_TAG	"u20"
#define TMS9928A_TAG	"tms9928a"
#define WD2793_TAG		"u11"
#define M6801_MAIN_TAG	"cpu1"
#define M6801_KB_TAG	"cpu2"
#define M6801_DDP_TAG	"cpu3"
#define M6801_PRN_TAG	"cpu4"
#define M6801_FDC_TAG	"cpu5"
#define M6801_SPI_TAG	"cpu6"
#define SCREEN_TAG		"screen"

class adam_state : public driver_device
{
public:
	adam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_netcpu(*this, M6801_MAIN_TAG),
		  m_fdc(*this, WD2793_TAG),
		  m_ram(*this, RAM_TAG),
		  m_ddp0(*this, CASSETTE_TAG),
		  m_ddp1(*this, CASSETTE2_TAG),
		  m_floppy0(*this, FLOPPY_0),
		  m_rxd(1),
		  m_reset(0),
		  m_dma(1),
		  m_bwr(1)
	{
		for (int i = 0; i < 6; i++)
			m_txd[i] = 1;
	}

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_netcpu;
	required_device<device_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_ddp0;
	required_device<cassette_image_device> m_ddp1;
	required_device<device_t> m_floppy0;

	virtual void machine_start();
	virtual void machine_reset();

	void bankswitch();
	void adamnet_txd_w(int device, int state);

	DECLARE_WRITE_LINE_MEMBER( os3_w );

	DECLARE_READ8_MEMBER( adamnet_r );
	DECLARE_WRITE8_MEMBER( adamnet_w );
	DECLARE_READ8_MEMBER( mioc_r );
	DECLARE_WRITE8_MEMBER( mioc_w );
	DECLARE_WRITE8_MEMBER( paddle_w );
	DECLARE_WRITE8_MEMBER( joystick_w );
	DECLARE_READ8_MEMBER( input1_r );
	DECLARE_READ8_MEMBER( input2_r );

	DECLARE_WRITE8_MEMBER( master6801_p1_w );
	DECLARE_READ8_MEMBER( master6801_p2_r );
	DECLARE_WRITE8_MEMBER( master6801_p2_w );
	DECLARE_READ8_MEMBER( master6801_p3_r );
	DECLARE_WRITE8_MEMBER( master6801_p3_w );
	DECLARE_WRITE8_MEMBER( master6801_p4_w );

	DECLARE_READ8_MEMBER( kb6801_p1_r );
	DECLARE_READ8_MEMBER( kb6801_p2_r );
	DECLARE_WRITE8_MEMBER( kb6801_p2_w );
	DECLARE_READ8_MEMBER( kb6801_p3_r );
	DECLARE_WRITE8_MEMBER( kb6801_p3_w );
	DECLARE_READ8_MEMBER( kb6801_p4_r );
	DECLARE_WRITE8_MEMBER( kb6801_p4_w );

	DECLARE_WRITE8_MEMBER( ddp6801_p1_w );
	DECLARE_READ8_MEMBER( ddp6801_p2_r );
	DECLARE_WRITE8_MEMBER( ddp6801_p2_w );
	DECLARE_READ8_MEMBER( ddp6801_p4_r );

	DECLARE_WRITE8_MEMBER( printer6801_p1_w );
	DECLARE_READ8_MEMBER( printer6801_p2_r );
	DECLARE_WRITE8_MEMBER( printer6801_p2_w );
	DECLARE_READ8_MEMBER( printer6801_p3_r );
	DECLARE_READ8_MEMBER( printer6801_p4_r );
	DECLARE_WRITE8_MEMBER( printer6801_p4_w );

	DECLARE_READ8_MEMBER( fdc6801_p1_r );
	DECLARE_WRITE8_MEMBER( fdc6801_p1_w );
	DECLARE_READ8_MEMBER( fdc6801_p2_r );
	DECLARE_WRITE8_MEMBER( fdc6801_p2_w );
	DECLARE_WRITE8_MEMBER( fdc6801_p4_w );

	// memory state
	UINT8 m_mioc;
	int m_game;

	// ADAMnet state
	UINT8 m_adamnet;
	int m_txd[6];
	int m_rxd;
	int m_reset;

	// DMA state
	UINT16 m_ba;
	int m_dma;
	int m_bwr;
	UINT8 m_data_in;
	UINT8 m_data_out;

	// keyboard state
	UINT16 m_key_y;

	// paddle state
	int m_joy_mode;
	UINT8 m_joy_status0;
	UINT8 m_joy_status1;

	// video state
	int m_vdp_nmi;

	// cassette state
	int m_wr0;
	int m_wr1;
	int m_track;
};

#endif
