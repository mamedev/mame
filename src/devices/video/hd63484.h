// license:BSD-3-Clause
// copyright-holders:Roberto Zandona'
/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#ifndef __HD63484_H__
#define __HD63484_H__


/* the on-chip FIFO is 16 bytes long, but we use a larger one to simplify */
/* decoding of long commands. Commands can be up to 64KB long... but Shanghai */
/* doesn't reach that length. */

#define FIFO_LENGTH 256
#define HD63484_RAM_SIZE 0x100000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class hd63484_device : public device_t
{
public:
	hd63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~hd63484_device() {}

	DECLARE_READ16_MEMBER( status_r );
	DECLARE_WRITE16_MEMBER( address_w );
	DECLARE_WRITE16_MEMBER( data_w );
	DECLARE_READ16_MEMBER( data_r );

	DECLARE_READ16_MEMBER( ram_r );
	DECLARE_READ16_MEMBER( regs_r );
	DECLARE_WRITE16_MEMBER( ram_w );
	DECLARE_WRITE16_MEMBER( regs_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT16 *   m_ram;
	UINT16 m_reg[256/2];

	int          m_fifo_counter;
	UINT16       m_fifo[FIFO_LENGTH];
	UINT16       m_readfifo;

	UINT16       m_pattern[16];
	int          m_org, m_org_dpd, m_rwp;
	UINT16       m_cl0, m_cl1, m_ccmp, m_edg, m_mask, m_ppy, m_pzcy, m_ppx, m_pzcx, m_psy, m_psx, m_pey, m_pzy, m_pex, m_pzx, m_xmin, m_ymin, m_xmax, m_ymax, m_rwp_dn;
	INT16        m_cpx, m_cpy;

	int          m_regno;
	int          m_skattva_hack;

	void doclr16( int opcode, UINT16 fill, int *dst, INT16 _ax, INT16 _ay );
	void docpy16( int opcode, int src, int *dst, INT16 _ax, INT16 _ay );
	int org_first_pixel( int _org_dpd );
	void dot( int x, int y, int opm, UINT16 color );
	int get_pixel( int x, int y );
	int get_pixel_ptn( int x, int y );
	void agcpy( int opcode, int src_x, int src_y, int dst_x, int dst_y, INT16 _ax, INT16 _ay );
	void ptn( int opcode, int src_x, int src_y, INT16 _ax, INT16 _ay );
	void line( INT16 sx, INT16 sy, INT16 ex, INT16 ey, INT16 col );
	void circle( INT16 sx, INT16 sy, UINT16 r, INT16 col );
	void paint( int sx, int sy, int col );

	void command_w(UINT16 cmd);
};

extern ATTR_DEPRECATED const device_type HD63484;

#endif /* __HD63484_H__ */
