/*
    Hitachi HD63450 DMA Controller
*/

#include "emu.h"

struct hd63450_intf
{
	const char *cpu_tag;
	attotime clock[4];
	attotime burst_clock[4];
	void (*dma_end)(running_machine &machine,int channel,int irq);  // called when the DMA transfer ends
	void (*dma_error)(running_machine &machine,int channel, int irq);  // called when a DMA transfer error occurs
	int (*dma_read[4])(running_machine &machine,int addr);  // special read / write handlers for each channel
	void (*dma_write[4])(running_machine &machine,int addr,int data);
};

int hd63450_read(device_t* device, int offset, UINT16 mem_mask);
void hd63450_write(device_t* device,int offset, int data, UINT16 mem_mask);
void hd63450_single_transfer(device_t* device, int x);
void hd63450_set_timer(device_t* device, int channel, attotime tm);

int hd63450_get_vector(device_t* device, int channel);
int hd63450_get_error_vector(device_t* device, int channel);

class hd63450_device : public device_t
{
public:
	hd63450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~hd63450_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type HD63450;


#define MCFG_HD63450_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, HD63450, 0) \
	MCFG_DEVICE_CONFIG(_config)

READ16_DEVICE_HANDLER( hd63450_r );
WRITE16_DEVICE_HANDLER( hd63450_w );
