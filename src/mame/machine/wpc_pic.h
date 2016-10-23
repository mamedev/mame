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
	wpc_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_pic_device();

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

extern const device_type WPC_PIC;

#endif
