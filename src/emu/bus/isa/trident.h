/*
 * trident.h
 *
 */

#ifndef TRIDENT_H_
#define TRIDENT_H_

#include "video/pc_vga.h"

// ======================> trident_vga_device

class trident_vga_device :  public svga_device
{
public:
	// construction/destruction
	trident_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);
	virtual UINT16 offset();
protected:
	virtual void device_start();
	virtual void device_reset();
	struct
	{
		UINT8 sr0c;
		UINT8 sr0d_old;
		UINT8 sr0d_new;
		UINT8 sr0e_old;
		UINT8 sr0e_new;
		UINT8 sr0f;
		UINT8 gc0e;
		UINT8 gc0f;
		UINT8 cr1f;
		UINT8 dac;
		bool new_mode;
		bool port_3c3;
		UINT8 clock;
		UINT8 pixel_depth;
		UINT8 revision;
		bool dac_active;
		UINT8 dac_count;
	} tri;
private:
	UINT8 trident_seq_reg_read(UINT8 index);
	void trident_seq_reg_write(UINT8 index, UINT8 data);
	void trident_define_video_mode();
	UINT8 trident_crtc_reg_read(UINT8 index);
	void trident_crtc_reg_write(UINT8 index, UINT8 data);
	UINT8 trident_gc_reg_read(UINT8 index);
	void trident_gc_reg_write(UINT8 index, UINT8 data);
};


// device type definition
extern const device_type TRIDENT_VGA;

#endif /* TRIDENT_H_ */
