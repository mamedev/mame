// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NSCSI_BUS_H
#define MAME_MACHINE_NSCSI_BUS_H

#pragma once


class nscsi_device;
class nscsi_slot_card_interface;

class nscsi_bus_device : public device_t
{
public:
	nscsi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void ctrl_w(int refid, uint32_t lines, uint32_t mask);
	void data_w(int refid, uint32_t lines);
	void ctrl_wait(int refid, uint32_t lines, uint32_t mask);

	uint32_t ctrl_r() const;
	uint32_t data_r() const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

private:
	struct dev_t {
		nscsi_device *dev;
		uint32_t ctrl, wait_ctrl;
		uint32_t data;
	};

	dev_t dev[16];
	int devcnt;

	uint32_t data, ctrl;

	void regen_data();
	void regen_ctrl(int refid);
};

class nscsi_connector: public device_t,
					   public device_single_card_slot_interface<nscsi_slot_card_interface>
{
public:
	template <typename T>
	nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: nscsi_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~nscsi_connector();

	nscsi_device *get_device();

protected:
	virtual void device_start() override ATTR_COLD;
};

class nscsi_slot_card_interface : public device_interface
{
	friend class nscsi_connector;

public:
	nscsi_slot_card_interface(const machine_config &mconfig, device_t &device, const char *nscsi_tag);

private:
	required_device<nscsi_device> m_nscsi;
};

class nscsi_device : public device_t
{
public:
	// Here because the biggest users are the devices, not the bus
	enum {
		S_INP = 0x0001,
		S_CTL = 0x0002,
		S_MSG = 0x0004,
		S_BSY = 0x0008,
		S_SEL = 0x0010,
		S_REQ = 0x0020,
		S_ACK = 0x0040,
		S_ATN = 0x0080,
		S_RST = 0x0100,
		S_ALL = 0x01ff,

		S_PHASE_DATA_OUT = 0,
		S_PHASE_DATA_IN  = S_INP,
		S_PHASE_COMMAND  = S_CTL,
		S_PHASE_STATUS   = S_CTL|S_INP,
		S_PHASE_MSG_OUT  = S_MSG|S_CTL,
		S_PHASE_MSG_IN   = S_MSG|S_CTL|S_INP,
		S_PHASE_MASK     = S_MSG|S_CTL|S_INP
	};

	// SCSI Messages
	// Here because some controllers interpret messages
	enum {
		SM_COMMAND_COMPLETE              = 0x00,
		SM_EXTENDED_MESSAGE              = 0x01,
		SM_SAVE_DATA_POINTER             = 0x02,
		SM_RESTORE_POINTERS              = 0x03,
		SM_DISCONNECT                    = 0x04,
		SM_INITIATOR_DETECTED_ERROR      = 0x05,
		SM_ABORT                         = 0x06,
		SM_MESSAGE_REJECT                = 0x07,
		SM_NO_OPERATION                  = 0x08,
		SM_MESSAGE_PARITY_ERROR          = 0x09,
		SM_LINKED_COMMAND_COMPLETE       = 0x0a,
		SM_LINKED_COMMAND_COMPLETE_WITH_FLAG = 0x0b,
		SM_BUS_DEVICE_RESET              = 0x0c,
		SM_ABORT_TAG                     = 0x0d,
		SM_CLEAR_QUEUE                   = 0x0e,
		SM_INITIATE_RECOVERY             = 0x0f,
		SM_RELEASE_RECOVERY              = 0x10,
		SM_TERMINATE_IO_PROCESS          = 0x11,
		SM_SIMPLE_QUEUE_TAG              = 0x20,
		SM_HEAD_OF_QUEUE_TAG             = 0x21,
		SM_ORDERED_QUEUE_TAG             = 0x22,
		SM_IGNORE_WIDE_RESIDUE           = 0x23
	};

	void connect_to_bus(nscsi_bus_device *bus, int refid, int default_scsi_id);
	virtual void scsi_ctrl_changed();

protected:
	nscsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	int scsi_id;
	int scsi_refid;
	nscsi_bus_device *scsi_bus;
};

class nscsi_full_device : public nscsi_device, public nscsi_slot_card_interface
{
public:
	virtual void scsi_ctrl_changed() override;

protected:
	// SCSI status returns
	enum {
		SS_GOOD                          = 0x00,
		SS_CHECK_CONDITION               = 0x02,
		SS_CONDITION_MET                 = 0x04,
		SS_BUSY                          = 0x08,
		SS_INT_GOOD                      = 0x10,
		SS_INT_CONDITION_MET             = 0x14,
		SS_RESV_CONFLICT                 = 0x18,
		SS_TERMINATED                    = 0x22,
		SS_QUEUE_FULL                    = 0x28
	};

	// SCSI sense keys
	enum {
		SK_NO_SENSE                      = 0x00,
		SK_RECOVERED_ERROR               = 0x01,
		SK_NOT_READY                     = 0x02,
		SK_MEDIUM_ERROR                  = 0x03,
		SK_HARDWARE_ERROR                = 0x04,
		SK_ILLEGAL_REQUEST               = 0x05,
		SK_UNIT_ATTENTION                = 0x06,
		SK_DATA_PROTECT                  = 0x07,
		SK_BLANK_CHECK                   = 0x08,
		SK_VENDOR_SPECIFIC               = 0x09,
		SK_COPY_ABORTED                  = 0x0a,
		SK_ABORTED_COMMAND               = 0x0b,
		SK_EQUAL                         = 0x0c,
		SK_VOLUME_OVERFLOW               = 0x0d,
		SK_MISCOMPARE                    = 0x0e,
		SK_COMPLETED                     = 0x0f
	};

	// SCSI SPACE type codes; SCSI-2 table 189
	enum {
		STC_BLOCKS                          = 0x00,
		STC_FILEMARKS                       = 0x01,
		STC_SEQUENTIAL_FILEMARKS            = 0x02,
		STC_END_OF_DATA                     = 0x03,
		STC_SETMARKS                        = 0x04,
		STC_SEQUENTIAL_SETMARKS             = 0x05
	};

	// SCSI MODE SENSE page controls; SCSI-2 table 55
	enum {
		SPC_CURRENT_VALUES                  = 0x00,
		SPC_CHANGEABLE_VALUES               = 0x01,
		SPC_DEFAULT_VALUES                  = 0x02,
		SPC_SAVED_VALUES                    = 0x03
	};

	// SCSI MODE SELECT/SENSE page codes; SCSI-2 tables 95, 155, 199, 216, 267, 296, 324, 350, 363; extra SCSI-3 codes from "The SCSI Bench Reference"
	enum {
		SPC_VENDOR_SPECIFIC                             = 0x00,
		SPC_READ_WRITE_ERROR_RECOVERY_PAGE              = 0x01,
		SPC_READ_ERROR_RECOVERY_PAGE                    = 0x01,
		SPC_DISCONNECT_RECONNECT_PAGE                   = 0x02,
		SPC_FORMAT_DEVICE_PAGE                          = 0x03,
		SPC_PARALLEL_PRINTER_INTERFACE_PAGE             = 0x03,
		SPC_MEASUREMENT_UNITS_PAGE                      = 0x03,
		SPC_RIGID_DISK_GEOMETRY_PAGE                    = 0x04,
		SPC_SERIAL_PRINTER_INTERFACE_PAGE               = 0x04,
		SPC_FLEXIBLE_DISK_PAGE                          = 0x05,
		SPC_PRINTER_OPTIONS_PAGE                        = 0x05,
		SPC_OPTICAL_MEMORY_PAGE                         = 0x06,
		SPC_VERIFY_ERROR_RECOVERY_PAGE                  = 0x07,
		SPC_CACHING_PAGE                                = 0x08,
		SPC_PERIPHERAL_DEVICE_PAGE                      = 0x09,
		SPC_CONTROL_MODE_PAGE                           = 0x0a,
		SPC_MEDIUM_TYPES_SUPPORTED_PAGE                 = 0x0b,
		SPC_NOTCH_AND_PARTITION_PAGE                    = 0x0c,
		SPC_DIRECT_ACCESS_POWER_CONDITION_PAGE          = 0x0d, // direct access device only
		SPC_CD_ROM_PAGE                                 = 0x0d,
		SPC_CD_ROM_AUDIO_CONTROL_PAGE                   = 0x0e,
		SPC_DATA_COMPRESSION_PAGE                       = 0x0f,
		SPC_XOR_CONTROL_MODE_PAGE                       = 0x10,
		SPC_DEVICE_CONFIGURATION_PAGE                   = 0x10,
		SPC_MEDIUM_PARTITION_PAGE_1                     = 0x11,
		SPC_MEDIUM_PARTITION_PAGE_2                     = 0x12,
		SPC_MEDIUM_PARTITION_PAGE_3                     = 0x13,
		SPC_MEDIUM_PARTITION_PAGE_4                     = 0x14,
		SPC_POWER_CONDITION_PAGE                        = 0x1a, // all other device types
		SPC_INFORMATIONAL_EXCEPTIONS_CONTROL_PAGE       = 0x1c,
		SPC_ELEMENT_ADDRESS_ASSIGNMENT_PAGE             = 0x1d,
		SPC_TRANSPORT_GEOMETRY_PARAMETERS_PAGE          = 0x1e,
		SPC_DEVICE_CAPABILITIES_PAGE                    = 0x1f,
		SPC_RETURN_ALL_MODE_PAGES                       = 0x3f
	};

	// SCSI additional sense codes and additional sense code qualifiers, packaged together as additional qualified sense key codes; SCSI-2 table 71; extra SCSI-3 codes from "The SCSI Bench Reference"
	enum {
		SKC_NO_ADDITIONAL_SENSE_INFORMATION                         = 0x0000,
		SKC_FILEMARK_DETECTED                                       = 0x0001,
		SKC_END_OF_PARTITION_MEDIUM_DETECTED                        = 0x0002,
		SKC_SETMARK_DETECTED                                        = 0x0003,
		SKC_BEGINNING_OF_PARTITION_MEDIUM_DETECTED                  = 0x0004,
		SKC_END_OF_DATA_DETECTED                                    = 0x0005,
		SKC_IO_PROCESS_TERMINATED                                   = 0x0006,
		SKC_AUDIO_PLAY_OPERATION_IN_PROGRESS                        = 0x0011,
		SKC_AUDIO_PLAY_OPERATION_PAUSED                             = 0x0012,
		SKC_AUDIO_PLAY_OPERATION_SUCCESSFULLY_COMPLETED             = 0x0013,
		SKC_AUDIO_PLAY_OPERATION_STOPPED_DUE_TO_ERROR               = 0x0014,
		SKC_NO_CURRENT_AUDIO_STATUS_TO_RETURN                       = 0x0015,
		SKC_OPERATION_IN_PROGRESS                                   = 0x0016,
		SKC_CLEANING_REQUESTED                                      = 0x0017,
		SKC_NO_INDEX_SECTOR_SIGNAL                                  = 0x0100,
		SKC_NO_SEEK_COMPLETE                                        = 0x0200,
		SKC_PERIPHERAL_DEVICE_WRITE_FAULT                           = 0x0300,
		SKC_NO_WRITE_CURRENT                                        = 0x0301,
		SKC_EXCESSIVE_WRITE_ERRORS                                  = 0x0302,
		SKC_LOGICAL_UNIT_NOT_READY_CAUSE_NOT_REPORTABLE             = 0x0400,
		SKC_LOGICAL_UNIT_IS_IN_PROCESS_OF_BECOMING_READY            = 0x0401,
		SKC_LOGICAL_UNIT_NOT_READY_INITIALIZING_COMMAND_REQUIRED    = 0x0402,
		SKC_LOGICAL_UNIT_NOT_READY_MANUAL_INTERVENTION_REQUIRED     = 0x0403,
		SKC_LOGICAL_UNIT_NOT_READY_FORMAT_IN_PROGRESS               = 0x0404,
		SKC_LOGICAL_UNIT_NOT_READY_OPERATION_IN_PROGRESS            = 0x0407,
		SKC_LOGICAL_UNIT_DOES_NOT_RESPOND_TO_SELECTION              = 0x0500,
		SKC_NO_REFERENCE_POSITION_FOUND                             = 0x0600,
		SKC_MULTIPLE_PERIPHERAL_DEVICES_SELECTED                    = 0x0700,
		SKC_LOGICAL_UNIT_COMMUNICATION_FAILURE                      = 0x0800,
		SKC_LOGICAL_UNIT_COMMUNICATION_TIME_OUT                     = 0x0801,
		SKC_LOGICAL_UNIT_COMMUNICATION_PARITY_ERROR                 = 0x0802,
		SKC_TRACK_FOLLOWING_ERROR                                   = 0x0900,
		SKC_TRACKING_SERVO_FAILURE                                  = 0x0901,
		SKC_FOCUS_SERVO_FAILURE                                     = 0x0902,
		SKC_SPINDLE_SERVO_FAILURE                                   = 0x0903,
		SKC_HEAD_SELECT_FAULT                                       = 0x0904,
		SKC_ERROR_LOG_OVERFLOW                                      = 0x0a00,
		SKC_WARNING                                                 = 0x0b00,
		SKC_WARNING_SPECIFIC_TEMPERATURE_EXCEEDED                   = 0x0b01,
		SKC_WRITE_ERROR                                             = 0x0c00,
		SKC_WRITE_ERROR_RECOVERED_WITH_AUTO_REALLOCATION            = 0x0c01,
		SKC_WRITE_ERROR_AUTO_REALLOCATION_FAILED                    = 0x0c02,
		SKC_WRITE_ERROR_RECOMMEND_REASSIGNMENT                      = 0x0c03,
		SKC_COMPRESSION_CHECK_MISCOMPARE_ERROR                      = 0x0c04,
		SKC_DATA_EXPANSION_OCCURRED_DURING_COMPRESSION              = 0x0c05,
		SKC_BLOCK_NOT_COMPRESSABLE                                  = 0x0c06,
		SKC_ID_CRC_OR_ECC_ERROR                                     = 0x1000,
		SKC_UNRECOVERED_READ_ERROR                                  = 0x1100,
		SKC_READ_RETRIES_EXHAUSTED                                  = 0x1101,
		SKC_ERROR_TOO_LONG_TO_CORRECT                               = 0x1102,
		SKC_MULTIPLE_READ_ERRORS                                    = 0x1103,
		SKC_UNRECOVERED_READ_ERROR_AUTO_REALLOCATE_FAILED           = 0x1104,
		SKC_L_EC_UNCORRECTABLE_ERROR                                = 0x1105,
		SKC_CIRC_UNRECOVERED_ERROR                                  = 0x1106,
		SKC_DATA_RESYNCHRONIZATION_ERROR                            = 0x1107,
		SKC_INCOMPLETE_BLOCK_READ                                   = 0x1108,
		SKC_NO_GAP_FOUND                                            = 0x1109,
		SKC_MISCORRECTED_ERROR                                      = 0x110a,
		SKC_UNRECOVERED_READ_ERROR_RECOMMEND_REASSIGNMENT           = 0x110b,
		SKC_UNRECOVERED_READ_ERROR_RECOMMEND_REWRITE                = 0x110c,
		SKC_DECOMPRESSION_CRC_ERROR                                 = 0x110d,
		SKC_CANNOT_DECOMPRESS_USING_DECLARED_ALGORITHM              = 0x100e,
		SKC_ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD                     = 0x1200,
		SKC_ADDRESS_MARK_NOT_FOUND_FOR_DATA_FIELD                   = 0x1300,
		SKC_RECORDED_ENTITY_NOT_FOUND                               = 0x1400,
		SKC_RECORD_NOT_FOUND                                        = 0x1401,
		SKC_FILEMARK_OR_SETMARK_NOT_FOUND                           = 0x1402,
		SKC_END_OF_DATA_NOT_FOUND                                   = 0x1403,
		SKC_BLOCK_SEQUENCE_ERROR                                    = 0x1404,
		SKC_RECORD_NOT_FOUND_RECOMMEND_REASSIGNMENT                 = 0x1405,
		SKC_RECORD_NOT_FOUND_DATA_AUTO_REALLOCATED                  = 0x1406,
		SKC_RANDOM_POSITIONING_ERROR                                = 0x1500,
		SKC_MECHANICAL_POSITIONING_ERROR                            = 0x1501,
		SKC_POSITIONING_ERROR_DETECTED_BY_READ_OF_MEDIUM            = 0x1502,
		SKC_DATA_SYNCHRONIZATION_MARK_ERROR                         = 0x1600,
		SKC_DATA_SYNCHRONIZATION_ERROR_DATA_REWRITTEN               = 0x1601,
		SKC_DATA_SYNCHRONIZATION_ERROR_RECOMMEND_REWRITE            = 0x1602,
		SKC_DATA_SYNCHRONIZATION_ERROR_DATA_AUTO_REALLOCATED        = 0x1603,
		SKC_DATA_SYNCHRONIZATION_ERROR_RECOMMEND_REASSIGNMENT       = 0x1604,
		SKC_RECOVERED_DATA_WITH_NO_ERROR_CORRECTION_APPLIED         = 0x1700,
		SKC_RECOVERED_DATA_WITH_RETRIES                             = 0x1701,
		SKC_RECOVERED_DATA_WITH_POSITIVE_HEAD_OFFSET                = 0x1702,
		SKC_RECOVERED_DATA_WITH_NEGATIVE_HEAD_OFFSET                = 0x1703,
		SKC_RECOVERED_DATA_WITH_RETRIES_AND_OR_CIRC_APPLIED         = 0x1704,
		SKC_RECOVERED_DATA_USING_PREVIOUS_SECTOR_ID                 = 0x1705,
		SKC_RECOVERED_DATA_WITHOUT_ECC_DATA_AUTO_REALLOCATED        = 0x1706,
		SKC_RECOVERED_DATA_WITHOUT_ECC_RECOMMEND_REASSIGNMENT       = 0x1707,
		SKC_RECOVERED_DATA_WITHOUT_ECC_RECOMMEND_REWRITE            = 0x1708,
		SKC_RECOVERED_DATA_WITHOUT_ECC_DATA_REWRITTEN               = 0x1809,
		SKC_RECOVERED_DATA_WITH_ERROR_CORRECTION_APPLIED            = 0x1800,
		SKC_RECOVERED_DATA_WITH_ERROR_CORRECTION_RETRIES_APPLIED    = 0x1801,
		SKC_RECOVERED_DATA_DATA_AUTO_REALLOCATED                    = 0x1802,
		SKC_RECOVERED_DATA_WITH_CIRC                                = 0x1803,
		SKC_RECOVERED_DATA_WITH_L_EC                                = 0x1804,
		SKC_RECOVERED_DATA_RECOMMEND_REASSIGNMENT                   = 0x1805,
		SKC_RECOVERED_DATA_RECOMMEND_REWRITE                        = 0x1806,
		SKC_RECOVERED_DATA_WITH_ECC_DATA_REWRITTEN                  = 0x1807,
		SKC_DEFECT_LIST_ERROR                                       = 0x1900,
		SKC_DEFECT_LIST_NOT_AVAILABLE                               = 0x1901,
		SKC_DEFECT_LIST_ERROR_IN_PRIMARY_LIST                       = 0x1902,
		SKC_DEFECT_LIST_ERROR_IN_GROWN_LIST                         = 0x1903,
		SKC_PARAMETER_LIST_LENGTH_ERROR                             = 0x1a00,
		SKC_SYNCHRONOUS_DATA_TRANSFER_ERROR                         = 0x1b00,
		SKC_DEFECT_LIST_NOT_FOUND                                   = 0x1c00,
		SKC_PRIMARY_DEFECT_LIST_NOT_FOUND                           = 0x1c01,
		SKC_GROWN_DEFECT_LIST_NOT_FOUND                             = 0x1c02,
		SKC_MISCOMPARE_DURING_VERIFY_OPERATION                      = 0x1d00,
		SKC_RECOVERED_ID_WITH_ECC_CORRECTION                        = 0x1e00,
		SKC_PARTIAL_DEFECT_LIST_TRANSFER                            = 0x1f00,
		SKC_INVALID_COMMAND_OPERATION_CODE                          = 0x2000,
		SKC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE                      = 0x2100,
		SKC_INVALID_ELEMENT_ADDRESS                                 = 0x2101,
		SKC_ILLEGAL_FUNCTION                                        = 0x2200, // "should use 0x2000, 0x2400, or 0x2600"
		SKC_INVALID_FIELD_IN_CDB                                    = 0x2400,
		SKC_LOGICAL_UNIT_NOT_SUPPORTED                              = 0x2500,
		SKC_INVALID_FIELD_IN_PARAMETER_LIST                         = 0x2600,
		SKC_PARAMETER_NOT_SUPPORTED                                 = 0x2601,
		SKC_PARAMETER_VALUE_INVALID                                 = 0x2602,
		SKC_THRESHOLD_PARAMETERS_NOT_SUPPORTED                      = 0x2603,
		SKC_INVALID_RELEASE_OF_ACTIVE_PERSISTENT_RESERVATION        = 0x2604,
		SKC_WRITE_PROTECTED                                         = 0x2700,
		SKC_NOT_READY_TO_READY_TRANSITION_MEDIUM_MAY_HAVE_CHANGED   = 0x2800,
		SKC_IMPORT_OR_EXPORT_ELEMENT_ACCESSED                       = 0x2801,
		SKC_POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED             = 0x2900,
		SKC_POWER_ON_OCCURRED                                       = 0x2901,
		SKC_SCSI_BUS_RESET_OCCURRED                                 = 0x2902,
		SKC_SCSI_BUS_DEVICE_RESET_FUNCTION_OCCURRED                 = 0x2903,
		SKC_PARAMETERS_CHANGED                                      = 0x2a00,
		SKC_MODE_PARAMETERS_CHANGED                                 = 0x2a01,
		SKC_LOG_PARAMETERS_CHANGED                                  = 0x2a02,
		SKC_RESERVATIONS_PREEMPTED                                  = 0x2a03,
		SKC_COPY_CANNOT_EXECUTE_SINCE_HOST_CANNOT_DISCONNECT        = 0x2b00,
		SKC_COMMAND_SEQUENCE_ERROR                                  = 0x2c00,
		SKC_TOO_MANY_WINDOWS_SPECIFIED                              = 0x2c01,
		SKC_INVALID_COMBINATION_OF_WINDOWS_SPECIFIED                = 0x2c02,
		SKC_OVERWRITE_ERROR_ON_UPDATE_IN_PLACE                      = 0x2d00,
		SKC_COMMANDS_CLEARED_BY_ANOTHER_INITIATOR                   = 0x2f00,
		SKC_INCOMPATIBLE_MEDIUM_INSTALLED                           = 0x3000,
		SKC_CANNOT_READ_MEDIUM_UNKNOWN_FORMAT                       = 0x3001,
		SKC_CANNOT_READ_MEDIUM_INCOMPATIBLE_FORMAT                  = 0x3002,
		SKC_CLEANING_CARTRIDGE_INSTALLED                            = 0x3003,
		SKC_CANNOT_WRITE_MEDIUM_UNKNOWN_FORMAT                      = 0x3004,
		SKC_CANNOT_WRITE_MEDIUM_INCOMPATIBLE_FORMAT                 = 0x3005,
		SKC_CANNOT_FORMAT_MEDIUM_INCOMPATIBLE_MEDIUM                = 0x3006,
		SKC_CLEANING_FAILURE                                        = 0x3007,
		SKC_MEDIUM_FORMAT_CORRUPTED                                 = 0x3100,
		SKC_FORMAT_COMMAND_FAILED                                   = 0x3101,
		SKC_NO_DEFECT_SPARE_LOCATION_AVAILABLE                      = 0x3200,
		SKC_DEFECT_LIST_UPDATE_FAILURE                              = 0x3201,
		SKC_TAPE_LENGTH_ERROR                                       = 0x3300,
		SKC_RIBBON_INK_OR_TONER_FAILURE                             = 0x3600,
		SKC_ROUNDED_PARAMETER                                       = 0x3700,
		SKC_SAVING_PARAMETERS_NOT_SUPPORTED                         = 0x3900,
		SKC_MEDIUM_NOT_PRESENT                                      = 0x3a00,
		SKC_SEQUENTIAL_POSITIONING_ERROR                            = 0x3b00,
		SKC_TAPE_POSITION_ERROR_AT_BEGINNING_OF_MEDIUM              = 0x3b01,
		SKC_TAPE_POSITION_ERROR_AT_END_OF_MEDIUM                    = 0x3b02,
		SKC_TAPE_OR_ELECTRONIC_VERTICAL_FORMS_UNIT_NOT_READY        = 0x3b03,
		SKC_SLEW_FAILURE                                            = 0x3b04,
		SKC_PAPER_JAM                                               = 0x3b05,
		SKC_FAILED_TO_SENSE_TOP_OF_FORM                             = 0x3b06,
		SKC_FAILED_TO_SENSE_BOTTOM_OF_FORM                          = 0x3b07,
		SKC_REPOSITION_ERROR                                        = 0x3b08,
		SKC_READ_PAST_END_OF_MEDIUM                                 = 0x3b09,
		SKC_READ_PAST_BEGINNING_OF_MEDIUM                           = 0x3b0a,
		SKC_POSITION_PAST_END_OF_MEDIUM                             = 0x3b0b,
		SKC_POSITION_PAST_BEGINNING_OF_MEDIUM                       = 0x3b0c,
		SKC_MEDIUM_DESTINATION_ELEMENT_FULL                         = 0x3b0d,
		SKC_MEDIUM_SOURCE_ELEMENT_EMPTY                             = 0x3b0e,
		SKC_MEDIUM_MAGAZINE_NOT_ACCESSIBLE                          = 0x3b11,
		SKC_MEDIUM_MAGAZINE_REMOVED                                 = 0x3b12,
		SKC_MEDIUM_MAGAZINE_INSERTED                                = 0x3b13,
		SKC_MEDIUM_MAGAZINE_LOCKED                                  = 0x3b14,
		SKC_MEDIUM_MAGAZINE_UNLOCKED                                = 0x3b15,
		SKC_INVALID_BITS_IN_IDENTIFY_MESSAGE                        = 0x3d00,
		SKC_LOGICAL_UNIT_HAS_NOT_SELF_CONFIGURED_YET                = 0x3e00,
		SKC_TARGET_OPERATING_CONDITIONS_HAVE_CHANGED                = 0x3f00,
		SKC_MICROCODE_HAS_BEEN_CHANGED                              = 0x3f01,
		SKC_CHANGED_OPERATING_DEFINITION                            = 0x3f02,
		SKC_INQUIRY_DATA_HAS_CHANGED                                = 0x3f03,
		SKC_RAM_FAILURE                                             = 0x4000, // "should use 0x40nn"
		SKC_DIAGNOSTIC_FAILURE_ON_COMPONENT_NN                      = 0x4000, // LSB is nn (0x80-0xff)
		SKC_DATA_PATH_FAILURE                                       = 0x4100, // "should use 0x40nn"
		SKC_POWER_ON_OR_SELF_TEST_FAILURE                           = 0x4200, // "should use 0x40nn"
		SKC_MESSAGE_ERROR                                           = 0x4300,
		SKC_INTERNAL_TARGET_FAILURE                                 = 0x4400,
		SKC_SELECT_OR_RESELECT_FAILURE                              = 0x4500,
		SKC_UNSUCCESSFUL_SOFT_RESET                                 = 0x4600,
		SKC_SCSI_PARITY_ERROR                                       = 0x4700,
		SKC_INITIATOR_DETECTED_ERROR_MESSAGE_RECEIVED               = 0x4800,
		SKC_INVALID_MESSAGE_ERROR                                   = 0x4900,
		SKC_COMMAND_PHASE_ERROR                                     = 0x4a00,
		SKC_DATA_PHASE_ERROR                                        = 0x4b00,
		SKC_LOGICAL_UNIT_FAILED_SELF_CONFIGURATION                  = 0x4c00,
		SKC_TAGGED_OVERLAPPED_COMMANDS_NN                           = 0x4d00, // "queue tag"; LSB is nn
		SKC_OVERLAPPED_COMMANDS_ATTEMPTED                           = 0x4e00,
		SKC_WRITE_APPEND_ERROR                                      = 0x5000,
		SKC_WRITE_APPEND_POSITION_ERROR                             = 0x5001,
		SKC_POSITION_ERROR_RELATED_TO_TIMING                        = 0x5002,
		SKC_ERASE_FAILURE                                           = 0x5100,
		SKC_CARTRIDGE_FAULT                                         = 0x5200,
		SKC_MEDIA_LOAD_OR_EJECT_FAILED                              = 0x5300,
		SKC_UNLOAD_TAPE_FAILURE                                     = 0x5301,
		SKC_MEDIUM_REMOVAL_PREVENTED                                = 0x5302,
		SKC_SCSI_TO_HOST_SYSTEM_INTERFACE_FAILURE                   = 0x5400,
		SKC_SYSTEM_RESOURCE_FAILURE                                 = 0x5500,
		SKC_SYSTEM_BUFFER_FULL                                      = 0x5501,
		SKC_UNABLE_TO_RECOVER_TABLE_OF_CONTENTS                     = 0x5700,
		SKC_GENERATION_DOES_NOT_EXIST                               = 0x5800,
		SKC_UPDATED_BLOCK_READ                                      = 0x5900,
		SKC_OPERATOR_REQUEST_OR_STATE_CHANGE_INPUT                  = 0x5a00, // "unspecified"
		SKC_OPERATOR_MEDIUM_REMOVAL_REQUEST                         = 0x5a01,
		SKC_OPERATOR_SELECTED_WRITE_PROTECT                         = 0x5a02,
		SKC_OPERATOR_SELECTED_WRITE_PERMIT                          = 0x5a03,
		SKC_LOG_EXCEPTION                                           = 0x5b00,
		SKC_THRESHOLD_CONDITION_MET                                 = 0x5b01,
		SKC_LOG_COUNTER_AT_MAXIMUM                                  = 0x5b02,
		SKC_LOG_LIST_CODES_EXHAUSTED                                = 0x5b03,
		SKC_RPL_STATUS_CHANGE                                       = 0x5c00,
		SKC_SPINDLES_SYNCHRONIZED                                   = 0x5c01,
		SKC_SPINDLES_NOT_SYNCHRONIZED                               = 0x5c02,
		SKC_FAILURE_PREDICTION_THRESHOLD_EXCEEDED                   = 0x5d00,
		SKC_FAILURE_PREDICTION_THRESHOLD_EXCEEDED_FALSE             = 0x5dff, // what?!
		SKC_LOW_POWER_CONDITION_ON                                  = 0x5e00,
		SKC_IDLE_CONDITION_ACTIVATED_BY_TIMER                       = 0x5e01,
		SKC_STANDBY_CONDITION_ACTIVATED_BY_TIMER                    = 0x5e02,
		SKC_IDLE_CONDITION_ACTIVATED_BY_COMMAND                     = 0x5e03,
		SKC_STANDBY_CONDITION_ACTIVATED_BY_COMMAND                  = 0x5e04,
		SKC_LAMP_FAILURE                                            = 0x6000,
		SKC_VIDEO_ACQUISITION_ERROR                                 = 0x6100,
		SKC_UNABLE_TO_ACQUIRE_VIDEO                                 = 0x6101,
		SKC_OUT_OF_FOCUS                                            = 0x6102,
		SKC_SCAN_HEAD_POSITIONING_ERROR                             = 0x6200,
		SKC_END_OF_USER_AREA_ENCOUNTERED_ON_THIS_TRACK              = 0x6300,
		SKC_ILLEGAL_MODE_FOR_THIS_TRACK                             = 0x6400,
		SKC_VOLTAGE_FAULT                                           = 0x6500,
		SKC_DECOMPRESSION_EXCEPTION_SHORT_ALGORITHM_ID_OF_NN        = 0x7000, // LSB is nn
		SKC_DECOMPRESSION_EXCEPTION_LONG_ALGORITHM_ID               = 0x7100
	};

	// SCSI addtional sense code qualifiers
	enum {
		SK_ASC_INVALID_FIELD_IN_CDB       = 0x24,
		SK_ASC_LOGICAL_UNIT_NOT_SUPPORTED = 0x25,
		SK_ASC_MEDIUM_NOT_PRESENT         = 0x3a
	};

	// SCSI commands
	static const char *const command_names[256];
	enum {
		SC_TEST_UNIT_READY               = 0x00,
		SC_REWIND                        = 0x01,
		SC_REZERO_UNIT                   = 0x01,
		SC_REQUEST_BLOCK_ADDRESS         = 0x02,
		SC_REQUEST_SENSE                 = 0x03,
		SC_FORMAT                        = 0x04,
		SC_FORMAT_UNIT                   = 0x04,
		SC_READ_BLOCK_LIMITS             = 0x05,
		SC_INITIALIZE_ELEMENT_STATUS     = 0x07,
		SC_REASSIGN_BLOCKS               = 0x07,
		SC_GET_MESSAGE_6                 = 0x08,
		SC_READ_6                        = 0x08,
		SC_RECEIVE                       = 0x08,
		SC_PRINT                         = 0x0a,
		SC_SEND_MESSAGE_6                = 0x0a,
		SC_SEND_6                        = 0x0a,
		SC_WRITE_6                       = 0x0a,
		SC_SEEK_6                        = 0x0b,
		SC_SLEW_AND_PRINT                = 0x0b,
		SC_SEEK_BLOCK                    = 0x0c,
		SC_READ_REVERSE                  = 0x0f,
		SC_SYNCHRONIZE_BUFFER            = 0x10,
		SC_WRITE_FILEMARKS               = 0x10,
		SC_SPACE                         = 0x11,
		SC_INQUIRY                       = 0x12,
		SC_VERIFY_6                      = 0x13,
		SC_RECOVER_BUFFERED_DATA         = 0x14,
		SC_MODE_SELECT_6                 = 0x15,
		SC_RESERVE_6                     = 0x16,
		SC_RESERVE_UNIT                  = 0x16,
		SC_RELEASE_6                     = 0x17,
		SC_RELEASE_UNIT                  = 0x17,
		SC_COPY                          = 0x18,
		SC_ERASE                         = 0x19,
		SC_MODE_SENSE_6                  = 0x1a,
		SC_LOAD_UNLOAD                   = 0x1b,
		SC_SCAN                          = 0x1b,
		SC_STOP_PRINT                    = 0x1b,
		SC_START_STOP_UNIT               = 0x1b,
		SC_RECEIVE_DIAGNOSTIC_RESULTS    = 0x1c,
		SC_SEND_DIAGNOSTIC               = 0x1d,
		SC_PREVENT_ALLOW_MEDIUM_REMOVAL  = 0x1e,
		SC_READ_FORMAT_CAPACITIES        = 0x23,
		SC_SET_WINDOW                    = 0x24,
		SC_GET_WINDOW                    = 0x25,
		SC_READ_CAPACITY                 = 0x25,
		SC_READ_CD_RECORDED_CAPACITY     = 0x25,
		SC_GET_MESSAGE_10                = 0x28,
		SC_READ_10                       = 0x28,
		SC_READ_GENERATION               = 0x29,
		SC_SEND_MESSAGE_10               = 0x2a,
		SC_SEND_10                       = 0x2a,
		SC_WRITE_10                      = 0x2a,
		SC_LOCATE                        = 0x2b,
		SC_POSITION_TO_ELEMENT           = 0x2b,
		SC_SEEK_10                       = 0x2b,
		SC_ERASE_10                      = 0x2c,
		SC_READ_UPDATED_BLOCK            = 0x2d,
		SC_WRITE_AND_VERIFY_10           = 0x2e,
		SC_VERIFY_10                     = 0x2f,
		SC_SEARCH_DATA_HIGH_10           = 0x30,
		SC_OBJECT_POSITION               = 0x31,
		SC_SEARCH_DATA_EQUAL_10          = 0x31,
		SC_SEARCH_DATA_LOW_10            = 0x32,
		SC_SET_LIMITS_10                 = 0x33,
		SC_PREFETCH                      = 0x34,
		SC_READ_POSITION                 = 0x34,
		SC_SYNCHRONIZE_CACHE             = 0x35,
		SC_LOCK_UNLOCK_CACHE             = 0x36,
		SC_READ_DEFECT_DATA_10           = 0x37,
		SC_MEDIUM_SCAN                   = 0x38,
		SC_COMPARE                       = 0x39,
		SC_COPY_AND_VERIFY               = 0x3a,
		SC_WRITE_BUFFER                  = 0x3b,
		SC_READ_BUFFER                   = 0x3c,
		SC_UPDATE_BLOCK                  = 0x3d,
		SC_READ_LONG                     = 0x3e,
		SC_WRITE_LONG                    = 0x3f,
		SC_CHANGE_DEFINITION             = 0x40,
		SC_WRITE_SAME                    = 0x41,
		SC_READ_SUB_CHANNEL              = 0x42,
		SC_READ_TOC_PMA_ATIP             = 0x43,
		SC_READ_HEADER                   = 0x44,
		SC_PLAY_AUDIO_10                 = 0x45,
		SC_GET_CONFIGURATION             = 0x46,
		SC_PLAY_AUDIO_MSF                = 0x47,
		SC_PLAY_AUDIO_TRACK_INDEX        = 0x48,
		SC_PLAY_TRACK_RELATIVE_10        = 0x49,
		SC_GET_EVENT_STATUS_NOTIFICATION = 0x4a,
		SC_PAUSE_RESUME                  = 0x4b,
		SC_LOG_SELECT                    = 0x4c,
		SC_LOG_SENSE                     = 0x4d,
		SC_STOP_PLAY_SCAN                = 0x4e,
		SC_XDWRITE                       = 0x50,
		SC_XPWRITE                       = 0x51,
		SC_READ_DISC_INFORMATION         = 0x51,
		SC_READ_TRACK_INFORMATION        = 0x52,
		SC_XDREAD                        = 0x52,
		SC_RESERVE_TRACK                 = 0x53,
		SC_SEND_OPC_INFORMATION          = 0x54,
		SC_MODE_SELECT_10                = 0x55,
		SC_RESERVE_10                    = 0x56,
		SC_RELEASE_10                    = 0x57,
		SC_REPAIR_TRACK                  = 0x58,
		SC_READ_MASTER_CUE               = 0x59,
		SC_MODE_SENSE_10                 = 0x5a,
		SC_CLOSE_TRACK_SESSION           = 0x5b,
		SC_READ_BUFFER_CAPACITY          = 0x5c,
		SC_SEND_CUE_SHEET                = 0x5d,
		SC_PERSISTENT_RESERVE_IN         = 0x5e,
		SC_PERSISTENT_RESERVE_OUT        = 0x5f,
		SC_XDWRITE_EXTENDED              = 0x80,
		SC_REBUILD                       = 0x81,
		SC_REGENERATE                    = 0x82,
		SC_EXTENDED_COPY                 = 0x83,
		SC_RECEIVE_COPY_RESULTS          = 0x84,
		SC_REPORT_LUNS                   = 0xa0,
		SC_BLANK                         = 0xa1,
		SC_SEND_EVENT                    = 0xa2,
		SC_REPORT_DEVICE_IDENTIFIER      = 0xa3,
		SC_SEND_KEY                      = 0xa3,
		SC_REPORT_KEY                    = 0xa4,
		SC_SET_DEVICE_IDENTIFIER         = 0xa4,
		SC_PLAY_AUDIO_12                 = 0xa5,
		SC_LOAD_UNLOAD_MEDIUM            = 0xa6,
		SC_MOVE_MEDIUM_ATTACHED          = 0xa7,
		SC_SET_READ_AHEAD                = 0xa7,
		SC_READ_12                       = 0xa8,
		SC_PLAY_RELATIVE_12              = 0xa9,
		SC_WRITE_12                      = 0xaa,
		SC_ERASE_12                      = 0xac,
		SC_GET_PERFORMANCE               = 0xac,
		SC_READ_DVD_STRUCTURE            = 0xad,
		SC_WRITE_AND_VERIFY_12           = 0xae,
		SC_VERIFY_12                     = 0xaf,
		SC_SEARCH_DATA_HIGH_12           = 0xb0,
		SC_SEARCH_DATA_EQUAL_12          = 0xb1,
		SC_SEARCH_DATA_LOW_12            = 0xb2,
		SC_SET_LIMITS_12                 = 0xb3,
		SC_READ_ELEMENT_STATUS_ATTACHED  = 0xb4,
		SC_SET_STREAMING                 = 0xb6,
		SC_READ_DEFECT_DATA_12           = 0xb7,
		SC_READ_CD_MSF                   = 0xb9,
		SC_SCAN_MMC                      = 0xba,
		SC_SET_CD_SPEED                  = 0xbb,
		SC_PLAY_CD                       = 0xbc,
		SC_MECHANISM_STATUS              = 0xbd,
		SC_READ_CD                       = 0xbe,
		SC_SEND_DVD_STRUCTURE            = 0xbf
	};

	enum {
		SBUF_MAIN,
		SBUF_SENSE
	};

	nscsi_full_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void scsi_message();
	virtual void scsi_command();
	virtual bool scsi_command_done(uint8_t command, uint8_t length);

	TIMER_CALLBACK_MEMBER(update_tick);

	void handle_request_sense(const u8 lun);
	void scsi_unknown_command();
	void scsi_status_complete(uint8_t st);
	void scsi_data_in(int buf, int size);
	void scsi_data_out(int buf, int size);

	struct sense_data {
		sense_data()
		{
			invalid = false;
			deferred = false;
			filemark = false;
			eom = false;
			bad_len = false;
			sense_key = 0;
			info = 0;
			sense_key_code = 0;
		}

		bool invalid;
		bool deferred;
		bool filemark;
		bool eom;
		bool bad_len;
		u8 sense_key;
		s32 info;
		u16 sense_key_code;
	};
	void set_sense_data(const u8 sense_key, const u16 sense_key_code, const sense_data *data = nullptr);
	void sense(bool deferred, uint8_t key, uint8_t asc = 0, uint8_t ascq = 0);
	void report_condition(const u8 sense_key, const u16 sense_key_code, const sense_data *data = nullptr);
	void report_bad_lun(const u8 cmd, const u8 lun);
	void report_bad_cmd(const u8 cmd);
	void report_filemark(const s32 info = 0, const bool eom = false);
	void report_bom(const s32 info = 0);
	void report_ew(const s32 info = 0);
	void report_eod(const s32 info = 0, const bool eom = false);
	void report_eom(const bool write, const s32 info = 0, const bool invalid = false);
	void report_bad_len(const bool over, const s32 info = 0);
	void report_bad_cdb_field();
	void report_bad_pl_field();
	void report_bad_pl_len();
	void report_no_saving_params();
	void report_no_medium();
	void report_medium_changed();
	void report_read_only();
	void report_read_failure();
	void report_write_failure();
	void report_erase_failure();

	int get_lun(int def = 0);
	void bad_lun();

	virtual uint8_t scsi_get_data(int buf, int offset);
	virtual void scsi_put_data(int buf, int offset, uint8_t data);

	// Default delays:

	// Arbitration delay (2.4us)
	virtual attotime scsi_arbitation_delay();

	// Assertion period (90ns)
	virtual attotime scsi_assertion_period();

	// Bus clear delay (800ns)
	virtual attotime scsi_bus_clear_delay();

	// Bus free delay (800ns)
	virtual attotime scsi_bus_free_delay();

	// Bus set delay (1.8us)
	virtual attotime scsi_bus_set_delay();

	// Bus settle delay (400ns)
	virtual attotime scsi_bus_settle_delay();

	// Cable skew delay (10ns)
	virtual attotime scsi_cable_skew_delay();

	// Data release delay (400ns)
	virtual attotime scsi_data_release_delay();

	// Deskew delay (45ns)
	virtual attotime scsi_deskew_delay();

	// Disconnection delay (200us)
	virtual attotime scsi_disconnection_delay();

	// Hold time (45ns)
	virtual attotime scsi_hold_time();

	// Negation period (90ns)
	virtual attotime scsi_negation_period();

	// Reset hold time (25us)
	virtual attotime scsi_reset_hold_time();

	// Selection abort time (200us)
	virtual attotime scsi_selection_abort_time();

	// Selection timeout delay (250ms)
	virtual attotime scsi_selection_timeout_delay();

	// Fast assertion period (30ns)
	virtual attotime scsi_fast_assertion_period();

	// Fast cable skew delay (5ns)
	virtual attotime scsi_fast_cable_skew_delay();

	// Fast deskew delay (20ns)
	virtual attotime scsi_fast_deskew_delay();

	// Fast hold time (10ns)
	virtual attotime scsi_fast_hold_time();

	// Fast negation period (30ns)
	virtual attotime scsi_fast_negation_period();

	// Byte transfer rate (immediate)
	virtual attotime scsi_data_byte_period();

	// Command delay (immediate)
	virtual attotime scsi_data_command_delay();

	uint8_t scsi_cmdbuf[4096];
	uint8_t scsi_sense_buffer[18];
	int scsi_cmdsize;
	uint8_t scsi_identify;

private:
	enum {
		IDLE
	};

	enum {
		TARGET_SELECT_WAIT_BUS_SETTLE = 1,
		TARGET_SELECT_WAIT_SEL_0,

		TARGET_NEXT_CONTROL,
		TARGET_WAIT_MSG_BYTE,
		TARGET_WAIT_CMD_BYTE,
		TARGET_WAIT_DATA_IN_BYTE,
		TARGET_WAIT_DATA_OUT_BYTE
	};

	enum {
		RECV_BYTE_T_WAIT_ACK_0 = 1,
		RECV_BYTE_T_WAIT_ACK_1,
		SEND_BYTE_T_WAIT_ACK_0,
		SEND_BYTE_T_WAIT_ACK_1
	};

	enum {
		STATE_MASK = 0x00ff,
		SUB_SHIFT  = 8,
		SUB_MASK   = 0xff00
	};

	enum {
		BC_MSG_OR_COMMAND,
		BC_STATUS,
		BC_MESSAGE_1,
		BC_MESSAGE_2,
		BC_DATA_IN,
		BC_DATA_OUT,
		BC_BUS_FREE
	};

	struct control {
		int action;
		int param1, param2;
	};

	emu_timer *scsi_timer;

	int scsi_state, scsi_substate;
	int scsi_initiator_id;
	int data_buffer_id, data_buffer_size, data_buffer_pos;

	control buf_control[32];
	int buf_control_rpos;
	int buf_control_wpos;

	control *buf_control_push();
	control *buf_control_pop();

	void step(bool timeout);
	void target_recv_byte();
	void target_send_byte(uint8_t val);
	void target_send_buffer_byte();
};


DECLARE_DEVICE_TYPE(NSCSI_BUS,       nscsi_bus_device)
DECLARE_DEVICE_TYPE(NSCSI_CONNECTOR, nscsi_connector)

#endif // MAME_MACHINE_NSCSI_BUS_H
