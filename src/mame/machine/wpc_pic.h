// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Pic-based protection simulation

#ifndef WPC_PIC_H
#define WPC_PIC_H

#define MCFG_WPC_PIC_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, WPC_PIC, 0 )

class wpc_pic_device : public device_t
{
public:
	wpc_pic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~wpc_pic_device();

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_serial(const char *serial);

protected:
	required_ioport_array<8> swarray;

	UINT8 mem[16], chk[3], curcmd, scrambler, count, chk_count, cmpchk[3];
	const char *serial;

	virtual void device_start() override;
	virtual void device_reset() override;

	void serial_to_pic();
	void check_game_id();
};

extern const device_type WPC_PIC;

#endif
