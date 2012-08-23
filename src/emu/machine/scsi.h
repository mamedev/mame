/***************************************************************************

 scsi.h - Header which defines the interface between SCSI device handlers
          and SCSI interfaces.

***************************************************************************/

#ifndef _SCSI_H_
#define _SCSI_H_

typedef struct scsiconfigitem
{
	const char *tag;
} SCSIConfigItem;

#define SCSI_MAX_DEVICES	(16)

typedef struct scsiconfigtable
{
	int devs_present;
	const SCSIConfigItem devices[SCSI_MAX_DEVICES];
} SCSIConfigTable;

#endif
