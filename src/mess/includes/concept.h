/*****************************************************************************
 *
 * includes/concept.h
 *
 * Corvus Concept driver
 *
 * Raphael Nabet, 2003
 *
 ****************************************************************************/

#ifndef CONCEPT_H_
#define CONCEPT_H_

#include "machine/6522via.h"
#include "machine/wd17xx.h"

/* keyboard interface */
enum
{
	KeyQueueSize = 32,
	MaxKeyMessageLen = 1
};

typedef struct
{
	read8_space_func reg_read;
	write8_space_func reg_write;
	read8_space_func rom_read;
	write8_space_func rom_write;
} expansion_slot_t;


class concept_state : public driver_device
{
public:
	concept_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this,"videoram") { }

	required_shared_ptr<UINT16> m_videoram;
	UINT8 m_pending_interrupts;
	char m_clock_enable;
	char m_clock_address;
	UINT8 m_KeyQueue[KeyQueueSize];
	int m_KeyQueueHead;
	int m_KeyQueueLen;
	UINT32 m_KeyStateSave[3];
	UINT8 m_fdc_local_status;
	UINT8 m_fdc_local_command;
	expansion_slot_t m_expansion_slots[4];
	DECLARE_READ16_MEMBER(concept_io_r);
	DECLARE_WRITE16_MEMBER(concept_io_w);
	DECLARE_WRITE8_MEMBER(concept_fdc_reg_w);
	DECLARE_READ8_MEMBER(concept_hdc_reg_r);
	DECLARE_WRITE8_MEMBER(concept_hdc_reg_w);
};


/*----------- defined in machine/concept.c -----------*/

extern const via6522_interface concept_via6522_intf;
extern const wd17xx_interface concept_wd17xx_interface;

MACHINE_START(concept);
VIDEO_START(concept);
SCREEN_UPDATE_IND16(concept);
INTERRUPT_GEN( concept_interrupt );


#endif /* CONCEPT_H_ */
