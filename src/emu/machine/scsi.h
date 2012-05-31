/***************************************************************************

 scsi.h - Header which defines the interface between SCSI device handlers
          and SCSI interfaces.

***************************************************************************/

#ifndef _SCSI_H_
#define _SCSI_H_

typedef struct scsiconfigitem
{
	int scsiID;
	const char *tag;
} SCSIConfigItem;

#define SCSI_MAX_DEVICES	(16)

typedef struct scsiconfigtable
{
	int devs_present;
	const SCSIConfigItem devices[SCSI_MAX_DEVICES];
} SCSIConfigTable;

// SCSI IDs
enum
{
	SCSI_ID_0 = 0,
	SCSI_ID_1,
	SCSI_ID_2,
	SCSI_ID_3,
	SCSI_ID_4,
	SCSI_ID_5,
	SCSI_ID_6,
	SCSI_ID_7
};

#endif
