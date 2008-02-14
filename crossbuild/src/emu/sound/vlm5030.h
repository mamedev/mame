#ifndef VLM5030_h
#define VLM5030_h

struct VLM5030interface
{
	int memory_region;  /* memory region of speech rom    */
	int memory_size;    /* memory size of speech rom (0=memory region length) */
};

/* set speech rom address */
void VLM5030_set_rom(void *speech_rom);

/* get BSY pin level */
int VLM5030_BSY(void);
/* latch contoll data */
WRITE8_HANDLER( VLM5030_data_w );
/* set RST pin level : reset / set table address A8-A15 */
void VLM5030_RST (int pin );
/* set VCU pin level : ?? unknown */
void VLM5030_VCU(int pin );
/* set ST pin level  : set table address A0-A7 / start speech */
void VLM5030_ST(int pin );

#endif

