// Creative Labs Ensonic AudioPCI97 ES1373

#ifndef ES1373_H
#define ES1373_H

#include "machine/pci.h"

#define MCFG_ES1373_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ES1373, 0x12741371, 0x04, 0x040100, 0x12741371)

/* Ensonic ES1373 registers 0x00-0x3f */
#define ES_INT_CS_CTRL         	(0x00/4)
#define ES_INT_CS_STATUS       	(0x04/4)
#define ES_UART_DATA         	  (0x08/4)
#define ES_UART_STATUS       	  (0x09/4)
#define ES_UART_CTRL         	  (0x09/4)
#define ES_UART_RSVD         	  (0x0A/4)
#define ES_MEM_PAGE         	  (0x0C/4)
#define ES_SRC_IF         	    (0x10/4)
#define ES_CODEC         	      (0x14/4)
#define ES_LEGACY        	      (0x18/4)
#define ES_CHAN_CTRL    	      (0x1C/4)
#define ES_SERIAL_CTRL    	    (0x20/4)
#define ES_DAC1_CNT    	        (0x24/4)
#define ES_DAC2_CNT    	        (0x28/4)
#define ES_ADC_CNT    	        (0x2C/4)
#define ES_ADC_CNT    	        (0x2C/4)
#define ES_HOST_IF0    	        (0x30/4)
#define ES_HOST_IF1    	        (0x34/4)
#define ES_HOST_IF2    	        (0x38/4)
#define ES_HOST_IF3    	        (0x3C/4)

struct frame_reg {
	UINT32 pci_addr;
	UINT16 curr_count;
	UINT16 buff_size;
	frame_reg() : pci_addr(0), curr_count(0), buff_size(0) {}
};

class es1373_device : public pci_device {
public:
	es1373_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER (reg_r);
	DECLARE_WRITE32_MEMBER(reg_w);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(map, 32);
	UINT16 m_ac97_regs[0x80];
	UINT32 m_es_regs[0x10];
	UINT16 m_src_ram[0x80];
	frame_reg m_dac1_fr;
	frame_reg m_dac2_fr;
	frame_reg m_adc_fr;
};

extern const device_type ES1373;

#endif
