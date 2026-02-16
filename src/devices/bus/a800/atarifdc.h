// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller

#ifndef MAME_BUS_A800_ATARIFDC_H
#define MAME_BUS_A800_ATARIFDC_H

#include "a8sio.h"

#include "imagedev/flopdrv.h"
#include "diserial.h"

class atari_fdc_device : public device_t, public device_serial_interface, public device_a8sio_card_interface
{
public:
	atari_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atari_load_proc(device_image_interface &image, bool is_created);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	// device_a8sio_card_interface implementation
	virtual void data_out_w(int state) override;
	virtual void command_w(int state) override;

private:
	TIMER_CALLBACK_MEMBER(serin_ready);

	void clr_serout(int expect_data);
	void add_serout(int expect_data);
	void clr_serin(int ser_delay);
	void add_serin(uint8_t data, int with_checksum);
	void a800_serial_command();
	void a800_serial_write();

	struct atari_drive
	{
		std::unique_ptr<uint8_t[]> image;   // alloc'd image
		int type = 0;                       // type of image (XFD, ATR, DSK)
		int mode = 0;                       // 0 read only, != 0 read/write
		int density = 0;                    // 0 SD, 1 MD, 2 DD
		int header_skip = 0;                // number of bytes in format header
		int tracks = 0;                     // number of tracks (35,40,77,80)
		int heads = 0;                      // number of heads (1,2)
		int spt = 0;                        // sectors per track (18,26)
		int seclen = 0;                     // sector length (128,256)
		int bseclen = 0;                    // boot sector length (sectors 1..3)
		int sectors = 0;                    // total sectors, ie. tracks x heads x spt
	};

	required_device_array<legacy_floppy_image_device, 4> m_floppy;

	int  m_serout_count;
	int  m_serout_offs;
	uint8_t m_serout_buff[512];
	uint8_t m_serout_chksum;

	int  m_serin_count;
	int  m_serin_offs;
	uint8_t m_serin_buff[512];
	uint8_t m_serin_chksum;
	emu_timer *m_serin_timer;

	bool m_command;

	atari_drive m_drv[4];
};

DECLARE_DEVICE_TYPE(ATARI_FDC, atari_fdc_device)

#endif // MAME_BUS_A800_ATARIFDC_H
