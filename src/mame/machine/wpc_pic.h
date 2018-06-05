// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Pic-based protection simulation

#ifndef MAME_MACHINE_WPC_PIC_H
#define MAME_MACHINE_WPC_PIC_H

#pragma once

class wpc_pic_device : public device_t
{
public:
	wpc_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_pic_device();

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_serial(const char *serial);

protected:
	required_ioport_array<8> swarray;

	uint8_t mem[16], chk[3], curcmd, scrambler, count, chk_count, cmpchk[3];
	const char *serial;

	virtual void device_start() override;
	virtual void device_reset() override;

	void serial_to_pic();
	void check_game_id();
};

DECLARE_DEVICE_TYPE(WPC_PIC, wpc_pic_device)

#endif // MAME_MACHINE_WPC_PIC_H
