// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_dk.h: include file for 990_dk.c
*/

#ifndef __990_DK__
#define __990_DK__

extern const device_type FD800;

#define MAX_FLOPPIES 4

class fd800_legacy_device : public device_t
{
public:
	fd800_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( cru_r );
	DECLARE_WRITE8_MEMBER( cru_w );
	template<class _Object> static devcb_base &static_set_int_callback(device_t &device, _Object object)
	{
		return downcast<fd800_legacy_device &>(device).m_int_line.set_callback(object);
	}

private:
	void device_start(void) override;
	void device_reset(void) override;
	void set_interrupt_line();

	int     read_id(int unit, int head, int *cylinder_id, int *sector_id);
	int     find_sector(int unit, int head, int sector, int *data_id);
	int     do_seek(int unit, int cylinder, int head);
	int     do_restore(int unit);
	void    do_read(void);
	void    do_write(void);
	void    do_cmd(void);

	UINT16 m_recv_buf;
	UINT16 m_stat_reg;
	UINT16 m_xmit_buf;
	UINT16 m_cmd_reg;

	int m_interrupt_f_f;
	devcb_write_line m_int_line;

	enum buf_mode_t {
		bm_off, bm_read, bm_write
	};

	UINT8 m_buf[128];
	int m_buf_pos;
	buf_mode_t m_buf_mode;
	int m_unit;
//  int m_head;
	int m_sector;
	/*int m_non_seq_mode;*/
//  int m_ddam;

	struct
	{
	//  legacy_floppy_image_device *img;
		int phys_cylinder;
		int log_cylinder[2];
		int seclen;
	} m_drv[MAX_FLOPPIES];
};

// LEGACY_FLOPPY_OPTIONS_EXTERN(fd800);

#define MCFG_FD800_INT_HANDLER( _intcallb ) \
	devcb = &fd800_legacy_device::static_set_int_callback( *device, DEVCB_##_intcallb );

#endif
