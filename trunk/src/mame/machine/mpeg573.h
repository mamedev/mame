#ifndef _MPEG573_H_
#define _MPEG573_H_

#include "sound/mas3507d.h"
#include "machine/ds2401.h"

#define MCFG_MPEG573_ADD(_tag, _clock, _output_cb ) \
	MCFG_DEVICE_ADD(_tag, MPEG573, _clock) \
	downcast<mpeg573_device *>(device)->set_output_cb(DEVCB2_##_output_cb);

#define MCFG_MPEG573_OUTPUT_CALLBACK( _output_cb )  \
	downcast<mpeg573_device *>(device)->set_output_cb(DEVCB2_##_output_cb);

class mpeg573_device : public device_t
{
public:
	mpeg573_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _write> void set_output_cb(_write _output_cb)
	{
		output_cb.set_callback(_output_cb);
	}

	required_device<mas3507d_device> mas3507d;
	required_device<ds2401_device> digital_id;

	DECLARE_ADDRESS_MAP(amap, 16);

	DECLARE_READ16_MEMBER(a00_r);
	DECLARE_READ16_MEMBER(a02_r);
	DECLARE_READ16_MEMBER(a04_r);
	DECLARE_READ16_MEMBER(a06_r);
	DECLARE_READ16_MEMBER(a0a_r);
	DECLARE_READ16_MEMBER(a80_r);
	DECLARE_WRITE16_MEMBER(mpeg_start_adr_high_w);
	DECLARE_WRITE16_MEMBER(mpeg_start_adr_low_w);
	DECLARE_WRITE16_MEMBER(mpeg_end_adr_high_w);
	DECLARE_WRITE16_MEMBER(mpeg_end_adr_low_w);
	DECLARE_WRITE16_MEMBER(mpeg_key_1_w);
	DECLARE_READ16_MEMBER(mas_i2c_r);
	DECLARE_WRITE16_MEMBER(mas_i2c_w);
	DECLARE_WRITE16_MEMBER(mpeg_ctrl_w);
	DECLARE_WRITE16_MEMBER(ram_write_adr_high_w);
	DECLARE_WRITE16_MEMBER(ram_write_adr_low_w);
	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);
	DECLARE_WRITE16_MEMBER(ram_read_adr_high_w);
	DECLARE_WRITE16_MEMBER(ram_read_adr_low_w);
	DECLARE_WRITE16_MEMBER(output_0_w);
	DECLARE_WRITE16_MEMBER(output_1_w);
	DECLARE_WRITE16_MEMBER(output_7_w);
	DECLARE_WRITE16_MEMBER(output_3_w);
	DECLARE_WRITE16_MEMBER(mpeg_key_2_w);
	DECLARE_WRITE16_MEMBER(mpeg_key_3_w);
	DECLARE_READ16_MEMBER(digital_id_r);
	DECLARE_WRITE16_MEMBER(digital_id_w);
	DECLARE_READ16_MEMBER(fpga_status_r);
	DECLARE_WRITE16_MEMBER(fpga_firmware_w);
	DECLARE_WRITE16_MEMBER(output_4_w);
	DECLARE_WRITE16_MEMBER(output_2_w);
	DECLARE_WRITE16_MEMBER(output_5_w);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb2_write8 output_cb;

	UINT16 *ram;
	UINT32 ram_adr;
	UINT8 output_data[8];

	void output(int offset, UINT16 data);
};

extern const device_type MPEG573;

#endif
