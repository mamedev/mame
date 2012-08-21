/*
    Hitachi HD63450 DMA Controller
*/

#include "emu.h"

typedef struct _hd63450_interface hd63450_intf;
struct _hd63450_interface
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

DECLARE_LEGACY_DEVICE(HD63450, hd63450);

#define MCFG_HD63450_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, HD63450, 0) \
	MCFG_DEVICE_CONFIG(_config)

READ16_DEVICE_HANDLER( hd63450_r );
WRITE16_DEVICE_HANDLER( hd63450_w );
