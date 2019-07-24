// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*
    ataridev.h

*/

#ifndef MAME_MACHINE_ATARIFDC_H
#define MAME_MACHINE_ATARIFDC_H

#include "imagedev/flopdrv.h"
#include "machine/6821pia.h"
#include "sound/pokey.h"

class atari_fdc_device : public device_t
{
public:
	atari_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( serin_r );
	DECLARE_WRITE8_MEMBER( serout_w );
	DECLARE_WRITE_LINE_MEMBER( pia_cb2_w );
	void atari_load_proc(device_image_interface &image, bool is_created);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void clr_serout(int expect_data);
	void add_serout(int expect_data);
	void clr_serin(int ser_delay);
	void add_serin(uint8_t data, int with_checksum);
	void a800_serial_command();
	void a800_serial_write();
	legacy_floppy_image_device *atari_floppy_get_device_child(int drive);

	struct atari_drive
	{
		std::unique_ptr<uint8_t[]> image;       /* alloc'd image */
		int type;           /* type of image (XFD, ATR, DSK) */
		int mode;           /* 0 read only, != 0 read/write */
		int density;        /* 0 SD, 1 MD, 2 DD */
		int header_skip;    /* number of bytes in format header */
		int tracks;         /* number of tracks (35,40,77,80) */
		int heads;          /* number of heads (1,2) */
		int spt;            /* sectors per track (18,26) */
		int seclen;         /* sector length (128,256) */
		int bseclen;        /* boot sector length (sectors 1..3) */
		int sectors;        /* total sectors, ie. tracks x heads x spt */
	};

	required_device<pokey_device> m_pokey;
	required_device<pia6821_device> m_pia;

	int  m_serout_count;
	int  m_serout_offs;
	uint8_t m_serout_buff[512];
	uint8_t m_serout_chksum;
//  int  m_serout_delay;

	int  m_serin_count;
	int  m_serin_offs;
	uint8_t m_serin_buff[512];
	uint8_t m_serin_chksum;
	int  m_serin_delay;

	atari_drive m_drv[4];
};

DECLARE_DEVICE_TYPE(ATARI_FDC, atari_fdc_device)

#endif // MAME_MACHINE_ATARIFDC_H
