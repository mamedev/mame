/***************************************************************************

CuboCD32 definitions

***************************************************************************/

#ifndef __CUBOCD32_H__
#define __CUBOCD32_H__

#include "includes/amiga.h"
#include "machine/microtch.h"
#include "sound/cdda.h"
#include "machine/i2cmem.h"

class cd32_state : public amiga_state
{
public:
	cd32_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag),
			m_microtouch(*this, "microtouch"),
			m_p1_port(*this, "P1"),
			m_p2_port(*this, "P2"),
			m_cdda(*this, "cdda")
	{
	}

	required_device<microtouch_device> m_microtouch;
	optional_ioport m_p1_port;
	optional_ioport m_p2_port;
	required_device<cdda_device> m_cdda;

	DECLARE_WRITE8_MEMBER(microtouch_tx);
	UINT16 m_potgo_value;
	int m_cd32_shifter[2];
	void (*m_input_hack)(running_machine &machine);
	int m_oldstate[2];
	DECLARE_CUSTOM_INPUT_MEMBER(cubo_input);
	DECLARE_CUSTOM_INPUT_MEMBER(cd32_sel_mirror_input);

	DECLARE_WRITE32_MEMBER(aga_overlay_w);
	DECLARE_WRITE8_MEMBER(cd32_cia_0_porta_w);
	DECLARE_READ8_MEMBER(cd32_cia_0_portb_r);
	DECLARE_WRITE8_MEMBER(cd32_cia_0_portb_w);
	DECLARE_DRIVER_INIT(cd32);
	DECLARE_DRIVER_INIT(mgprem11);
	DECLARE_DRIVER_INIT(odeontw2);
	DECLARE_DRIVER_INIT(cndypuzl);
	DECLARE_DRIVER_INIT(haremchl);
	DECLARE_DRIVER_INIT(mgnumber);
	DECLARE_DRIVER_INIT(lsrquiz2);
	DECLARE_DRIVER_INIT(lasstixx);
	DECLARE_DRIVER_INIT(lsrquiz);
};

/*----------- defined in machine/cd32.c -----------*/

class akiko_device : public device_t
{
public:
	akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~akiko_device() {}

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

private:
	// internal state
	address_space *m_space;

	/* chunky to planar converter */
	UINT32  m_c2p_input_buffer[8];
	UINT32  m_c2p_output_buffer[8];
	UINT32  m_c2p_input_index;
	UINT32  m_c2p_output_index;

	/* i2c bus */
	int     m_i2c_scl_out;
	int     m_i2c_scl_dir;
	int     m_i2c_sda_out;
	int     m_i2c_sda_dir;

	/* cdrom */
	UINT32  m_cdrom_status[2];
	UINT32  m_cdrom_address[2];
	UINT32  m_cdrom_track_index;
	UINT32  m_cdrom_lba_start;
	UINT32  m_cdrom_lba_end;
	UINT32  m_cdrom_lba_cur;
	UINT16  m_cdrom_readmask;
	UINT16  m_cdrom_readreqmask;
	UINT32  m_cdrom_dmacontrol;
	UINT32  m_cdrom_numtracks;
	UINT8   m_cdrom_speed;
	UINT8   m_cdrom_cmd_start;
	UINT8   m_cdrom_cmd_end;
	UINT8   m_cdrom_cmd_resp;
	cdda_device *m_cdda;
	cdrom_file *m_cdrom;
	UINT8 * m_cdrom_toc;
	emu_timer *m_dma_timer;
	emu_timer *m_frame_timer;
	i2cmem_device *m_i2cmem;

	int m_cdrom_is_device;

	void nvram_write(UINT32 data);
	UINT32 nvram_read();

	void c2p_write(UINT32 data);
	UINT32 c2p_read();

	void cdda_stop();
	void cdda_play(UINT32 lba, UINT32 num_blocks);
	void cdda_pause(int pause);
	UINT8 cdda_getstatus(UINT32 *lba);
	void set_cd_status(UINT32 status);

	TIMER_CALLBACK_MEMBER(frame_proc);
	TIMER_CALLBACK_MEMBER(dma_proc);

	void start_dma();
	void setup_response( int len, UINT8 *r1 );

	TIMER_CALLBACK_MEMBER( cd_delayed_cmd );
	void update_cdrom();
};

extern const device_type AKIKO;


#endif /* __CUBOCD32_H__ */
