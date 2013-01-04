/*****************************************************************************
 *
 * includes/pce.h
 *
 * NEC PC Engine/TurboGrafx16
 *
 ****************************************************************************/

#ifndef PCE_H_
#define PCE_H_

#include "cdrom.h"
#include "cpu/h6280/h6280.h"
#include "sound/msm5205.h"
#include "machine/nvram.h"
#include "video/huc6260.h"

#define C6280_TAG			"c6280"

#define	MAIN_CLOCK		21477270
#define PCE_CD_CLOCK	9216000

#define PCE_HEADER_SIZE		512

#define TG_16_JOY_SIG		0x00
#define PCE_JOY_SIG			0x40
#define NO_CD_SIG			0x80
#define CD_SIG				0x00
/* these might be used to indicate something, but they always seem to return 1 */
#define CONST_SIG			0x30

/* the largest possible cartridge image (street fighter 2 - 2.5MB) */
#define PCE_ROM_MAXSIZE		0x280000

struct pce_cd_t
{
	UINT8	regs[16];
	UINT8	*bram;
	UINT8	*adpcm_ram;
	int		bram_locked;
	int		adpcm_read_ptr;
	UINT8	adpcm_read_buf;
	int		adpcm_write_ptr;
	UINT8	adpcm_write_buf;
	int		adpcm_length;
	int		adpcm_clock_divider;
	UINT32  msm_start_addr;
	UINT32	msm_end_addr;
	UINT32	msm_half_addr;
	UINT8	msm_nibble;
	UINT8	msm_idle;
	UINT8	msm_repeat;

	/* SCSI signals */
	int		scsi_BSY;	/* Busy. Bus in use */
	int		scsi_SEL;	/* Select. Initiator has won arbitration and has selected a target */
	int		scsi_CD;	/* Control/Data. Target is sending control (data) information */
	int		scsi_IO;	/* Input/Output. Target is sending (receiving) information */
	int		scsi_MSG;	/* Message. Target is sending or receiving a message */
	int		scsi_REQ;	/* Request. Target is requesting a data transfer */
	int		scsi_ACK;	/* Acknowledge. Initiator acknowledges that it is ready for a data transfer */
	int		scsi_ATN;	/* Attention. Initiator has a message ready for the target */
	int		scsi_RST;	/* Reset. Initiator forces all targets and any other initiators to do a warm reset */
	int		scsi_last_RST;	/* To catch setting of RST signal */
	int		cd_motor_on;
	int		selected;
	UINT8	*command_buffer;
	int		command_buffer_index;
	int		status_sent;
	int		message_after_status;
	int		message_sent;
	UINT8	*data_buffer;
	int		data_buffer_size;
	int		data_buffer_index;
	int		data_transferred;

	/* Arcade Card specific */
	UINT8	*acard_ram;
	UINT8	acard_latch;
	UINT8	acard_ctrl[4];
	UINT32	acard_base_addr[4];
	UINT16	acard_addr_offset[4];
	UINT16	acard_addr_inc[4];
	UINT32	acard_shift;
	UINT8	acard_shift_reg;

	UINT32	current_frame;
	UINT32	end_frame;
	UINT32	last_frame;
	UINT8	cdda_status;
	UINT8	cdda_play_mode;
	UINT8	*subcode_buffer;
	UINT8	end_mark;
	cdrom_file	*cd;
	const cdrom_toc*	toc;
	emu_timer	*data_timer;
	emu_timer	*adpcm_dma_timer;

	emu_timer	*cdda_fadeout_timer;
	emu_timer	*cdda_fadein_timer;
	double	cdda_volume;
	emu_timer	*adpcm_fadeout_timer;
	emu_timer	*adpcm_fadein_timer;
	double	adpcm_volume;
};


class pce_state : public driver_device
{
public:
	pce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu"),
		m_cd_ram(*this, "cd_ram"),
		m_user_ram(*this, "user_ram"),
		m_huc6260(*this, "huc6260")
	{ }

    required_device<h6280_device> m_maincpu;
	required_shared_ptr<UINT8> m_cd_ram;
	required_shared_ptr<UINT8> m_user_ram;
	optional_device<huc6260_device> m_huc6260;
	UINT8 m_io_port_options;
	UINT8 m_sys3_card;
	UINT8 m_acard;
	pce_cd_t m_cd;
	UINT8 *m_cartridge_ram;
	int m_joystick_port_select;
	int m_joystick_data_select;
	UINT8 m_joy_6b_packet[5];
	DECLARE_WRITE8_MEMBER(pce_sf2_banking_w);
	DECLARE_WRITE8_MEMBER(pce_cartridge_ram_w);
	DECLARE_WRITE8_MEMBER(mess_pce_joystick_w);
	DECLARE_READ8_MEMBER(mess_pce_joystick_r);
	DECLARE_WRITE8_MEMBER(pce_cd_bram_w);
	DECLARE_WRITE8_MEMBER(pce_cd_intf_w);
	DECLARE_READ8_MEMBER(pce_cd_intf_r);
	DECLARE_READ8_MEMBER(pce_cd_acard_r);
	DECLARE_WRITE8_MEMBER(pce_cd_acard_w);
	DECLARE_READ8_MEMBER(pce_cd_acard_wram_r);
	DECLARE_WRITE8_MEMBER(pce_cd_acard_wram_w);
	DECLARE_DRIVER_INIT(sgx);
	DECLARE_DRIVER_INIT(tg16);
	DECLARE_DRIVER_INIT(mess_pce);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_MACHINE_START(pce);
	DECLARE_MACHINE_RESET(mess_pce);
	TIMER_CALLBACK_MEMBER(pce_cd_data_timer_callback);
	TIMER_CALLBACK_MEMBER(pce_cd_cdda_fadeout_callback);
	TIMER_CALLBACK_MEMBER(pce_cd_cdda_fadein_callback);
	TIMER_CALLBACK_MEMBER(pce_cd_adpcm_fadeout_callback);
	TIMER_CALLBACK_MEMBER(pce_cd_adpcm_fadein_callback);
	TIMER_CALLBACK_MEMBER(pce_cd_clear_ack);
	TIMER_CALLBACK_MEMBER(pce_cd_adpcm_dma_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
};


/*----------- defined in machine/pce.c -----------*/
DEVICE_IMAGE_LOAD(pce_cart);
extern const msm5205_interface pce_cd_msm5205_interface;

#endif /* PCE_H_ */
