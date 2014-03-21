/*
    Hitachi HD63450 DMA Controller
*/

#include "emu.h"


struct hd63450_regs
{  // offsets in bytes
	unsigned char csr;  // [00] Channel status register (R/W)
	unsigned char cer;  // [01] Channel error register (R)
	unsigned char dcr;  // [04] Device control register (R/W)
	unsigned char ocr;  // [05] Operation control register (R/W)
	unsigned char scr;  // [06] Sequence control register (R/W)
	unsigned char ccr;  // [07] Channel control register (R/W)
	unsigned short mtc;  // [0a,0b]  Memory Transfer Counter (R/W)
	unsigned long mar;  // [0c-0f]  Memory Address Register (R/W)
	unsigned long dar;  // [14-17]  Device Address Register (R/W)
	unsigned short btc;  // [1a,1b]  Base Transfer Counter (R/W)
	unsigned long bar;  // [1c-1f]  Base Address Register (R/W)
	unsigned char niv;  // [25]  Normal Interrupt Vector (R/W)
	unsigned char eiv;  // [27]  Error Interrupt Vector (R/W)
	unsigned char mfc;  // [29]  Memory Function Code (R/W)
	unsigned char cpr;  // [2d]  Channel Priority Register (R/W)
	unsigned char dfc;  // [31]  Device Function Code (R/W)
	unsigned char bfc;  // [39]  Base Function Code (R/W)
	unsigned char gcr;  // [3f]  General Control Register (R/W)
};

struct hd63450_interface
{
	const char *m_cpu_tag;
	attotime m_our_clock[4];
	attotime m_burst_clock[4];
	void (*dma_end)(running_machine &machine,int channel,int irq);  // called when the DMA transfer ends
	void (*dma_error)(running_machine &machine,int channel, int irq);  // called when a DMA transfer error occurs
	int (*dma_read[4])(running_machine &machine,int addr);  // special read / write handlers for each channel
	void (*dma_write[4])(running_machine &machine,int addr,int data);
};

class hd63450_device : public device_t,
								public hd63450_interface
{
public:
	hd63450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~hd63450_device() {}

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );
	
	void single_transfer(int x);
	void set_timer(int channel, attotime tm);
	int get_vector(int channel);
	int get_error_vector(int channel);
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	// internal state
	hd63450_regs m_reg[4];
	emu_timer* m_timer[4];  // for timing data reading/writing each channel
	int m_in_progress[4];  // if a channel is in use
	int m_transfer_size[4];
	int m_halted[4];  // non-zero if a channel has been halted, and can be continued later.
	cpu_device *m_cpu;

	TIMER_CALLBACK_MEMBER(dma_transfer_timer);
	void dma_transfer_abort(int channel);
	void dma_transfer_halt(int channel);
	void dma_transfer_continue(int channel);
	void dma_transfer_start(int channel, int dir);
};

extern const device_type HD63450;


#define MCFG_HD63450_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, HD63450, 0) \
	MCFG_DEVICE_CONFIG(_config)
