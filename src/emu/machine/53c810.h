#ifndef LSI53C810_H
#define LSI53C810_H

#include "machine/scsihle.h"

struct LSI53C810interface
{
	void (*irq_callback)(running_machine &machine, int); /* IRQ callback */
	void (*dma_callback)(running_machine &machine, UINT32, UINT32, int, int);	/* DMA callback */
	UINT32 (*fetch)(running_machine &machine, UINT32 dsp);
};

#define MCFG_LSI53C810_ADD( _tag, _config ) \
	MCFG_DEVICE_ADD( _tag, LSI53C810, 0 ) \
	MCFG_DEVICE_CONFIG(_config)

class lsi53c810_device : public device_t,
					   public LSI53C810interface
{
public:
	// construction/destruction
	lsi53c810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void lsi53c810_read_data(int bytes, UINT8 *pData);
	void lsi53c810_write_data(int bytes, UINT8 *pData);

	UINT8 lsi53c810_reg_r( int offset );
	void lsi53c810_reg_w(int offset, UINT8 data);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	typedef delegate<void (void)> opcode_handler_delegate;
	opcode_handler_delegate dma_opcode[256];

	UINT32 FETCH();
	void dmaop_invalid();
	void dmaop_move_memory();
	void dmaop_interrupt();
	void dmaop_block_move();
	void dmaop_select();
	void dmaop_wait_disconnect();
	void dmaop_wait_reselect();
	void dmaop_set();
	void dmaop_clear();
	void dmaop_move_from_sfbr();
	void dmaop_move_to_sfbr();
	void dmaop_read_modify_write();
	int scripts_compute_branch();
	UINT32 scripts_get_jump_dest();
	void dmaop_jump();
	void dmaop_call();
	void dmaop_return();
	void dmaop_store();
	void dmaop_load();
	void dma_exec();
	void add_opcode(UINT8 op, UINT8 mask, opcode_handler_delegate handler);
	void lsi53c810_init();
	UINT32 lsi53c810_dasm_fetch(UINT32 pc);
	unsigned lsi53c810_dasm(char *buf, UINT32 pc);

	scsihle_device *devices[8];	/* SCSI IDs 0-7 */
	UINT8 last_id;

	UINT8 scntl0;
	UINT8 scntl1;
	UINT8 scntl2;
	UINT8 scntl3;
	UINT8 scid;
	UINT8 sxfer;
	UINT8 socl;
	UINT8 istat;
	UINT8 dstat;
	UINT8 sstat0;
	UINT8 sstat1;
	UINT8 sstat2;
	UINT8 dien;
	UINT8 dcntl;
	UINT8 dmode;
	UINT32 temp;
	UINT32 dsa;
	UINT32 dsp;
	UINT32 dsps;
	UINT32 dcmd;
	UINT8 sien0;
	UINT8 sien1;
	UINT8 stime0;
	UINT8 respid;
	UINT8 stest1;
	UINT8 scratch_a[4];
	UINT8 scratch_b[4];
	int dma_icount;
	int halted;
	int carry;
};

// device type definition
extern const device_type LSI53C810;

#endif
