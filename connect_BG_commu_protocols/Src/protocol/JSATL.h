/*
 * JSATL.h
 *
 *  Created on: Jun 12, 2019
 *      Author: tony
 */

#ifndef JSATL_H_
#define JSATL_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>
#include "../net_connection/net_connection.h"

/* frame format:
 *  | 标识位 | 消息头	| 消息体 | 校验码 | 标识位 |
 *
 *  标识位: 0x7e
 *
 *  消息头:
 *  starting byte  |   entry         |  data type |
 *  0              |  msg ID         |  WORD      |
 *  2              |  msg property   |  WORD      |
 *  4              |  phe number     |  BCD[6]    |
 *  10             |  msg SN         |  WORD      |
 *  12             |  msg subpackage |  ----      |
 *
 *  msg property (word):
 *  15	14 | 13	| 12 11	10 |9	8	7	6	5	4	3	2	1	0|
 *   保留   |分包	|数据加密方式|	      消息体长度                       |
 *
 *  msg subpackage:
 *  starting byte  |       entry             |  data type |   explanation |
 *  0              |total numbers of packages|  WORD      | amount of sub packages |
 *  2              |index of current pkg     |  WORD      | beginning from 1       |
 *
 *  校验码: xor byte by byte from head of frame until end of message body
 *
 */

#define  ID_BYTE  0x7e
#define  HEAD_ID_INX  0

#define  MSG_HEAD_START_INX  1
#define  MSG_ID_INX  (MSG_HEAD_START_INX+0)
#define  MSG_PROPERTY_INX (MSG_HEAD_START_INX+2)
#define  PHONE_NUMBER_INX (MSG_HEAD_START_INX+4)
#define  MSG_SN_INX (MSG_HEAD_START_INX+10)

#define  MSG_PACKAGE_INX (MSG_HEAD_START_INX+12)
#define  MSG_TOTAL_PACKAGES_INX  MSG_PACKAGE_INX
#define  SEQUENCE_OF_PACKAGE_INX  (MSG_PACKAGE_INX+2)

#define  MSG_BODY_BEG_INX_NO_SUBPKG    13
#define  MSG_BODY_BEG_INX_WITH_SUBPKG  17

#define  MAX_MSG_BODY_LEN_OF_SUBPACKAGE  0x3FF


#define  BG_GENERAL_REPLY  0x8001
#define  BG_REGISTER_REPLY 0x8100
#define  CMD_QUERY_INFO  0x8900
#define  REP_QUERY_INFO  0x0900
#define  CMD_PARAM_SET  0x8103
#define  CMD_ALL_PARAM_QUERY  0x8104
#define  CMD_SPECIFY_PARAM_QUERY  0x8106
#define  CMD_TAKE_PHOTO  0x8801
#define  WARNNING_ATTACHED_CALL  0x9208
#define  FINISH_UPLOAD_FILE_REPLY  0x9212


#define  TERMINAL_GENERAL_REPLY  0x0001
#define  TERMINAL_HEART_BEAT  0x0002
#define  TERMINAL_REGISTER  0x0100
#define  TERMINAL_UNREGISTER  0x0003
#define  TERMINAL_AUTHEN  0x0102
#define  TERMINAL_BSC_INFO_REPLY  0x0900
#define  TERMINAL_QUERY_PARAM_REPLY  0x0104
#define  TERMINAL_TAKE_PHOTO_REPLY  0x0805
#define  TERMINAL_SEND_WARN_ATTACH_INFO  0x1210
#define  TERMINAL_SEND_WARN_ATTACH_DATA  0x1211
#define  TERMINAL_FINISH_UPLOAD_FILE  0x1212


#define  DEV_COUNT  4
#define  DEV_SYS_INFO_BUF_LEN  512

typedef enum {
	LOGOUT = 0,
	LOGIN = 1,
	AUTHEN = 2,
	NORMAL = 3,
} JSATL_State;

typedef enum {
	ADAS_ID = 0x64,
	DSM_ID =  0x65,
	TPMS_ID = 0x66,
	BSD_ID =  0x67
} Dev_ID;


typedef struct {
	unsigned char working_state;
	unsigned int  warnning_state;
} Dev_State;


typedef struct {
	unsigned char company_name_len;
	unsigned char pdt_model_len;
	unsigned char hdware_ven_len;
	unsigned char sfware_ven_len;
	unsigned char dev_ID_len;
	unsigned char custm_code_len;
	unsigned char *sys_info_buf;
} Dev_Sys_Info;


typedef struct {
	unsigned char dev_ID;
	Dev_State dev_state;
	Dev_Sys_Info dev_sys_info;
} Dev_Bsc_Info;


#ifndef  ADAS_PARAMS_ID
#define  ADAS_PARAMS_ID 0xF364
#define  ADAS_SPEED_THRESHOLD_INX  0
#define  ADAS_WARN_VOLUME_INX  1
#define  ADAS_PHOTO_LOGIC_INX  2
#define  ADAS_TIMING_PHOTO_INTERVAL_INX  3
#define  ADAS_DISTANCE_PHOTO_INTERVAL_INX  5
#define  ADAS_PHOTO_COUNT_INX  7
#define  ADAS_PHOTO_DISTANCE_INX  8
#define  ADAS_PHOTO_RESOLUTION_INX  9
#define  ADAS_VIDEO_RESOLUTION_INX  10
#define  ADAS_WARN_ENABLE_INX  11
#define  ADAS_EVENT_ENABLE_INX  15
#define  ADAS_RESERVE_INX  19
#define  ADAS_BLOCK_WARN_DISTANCE_INX  20
#define  ADAS_BLOCK_WARN_SPEED_INX  21
#define  ADAS_BLOCK_WARN_VIDEO_SEC_INX  22
#define  ADAS_BLOCK_WARN_PHOTO_COUNT_INX  23
#define  ADAS_BLOCK_WARN_PHOTO_INTERVAL_INX  24

#define  ADAS_LANES_CHANGE_JUDGE_SEC_INX  25
#define  ADAS_LANES_CHANGE_JUDGE_NUM_INX  26
#define  ADAS_LANES_CHANGE_JUDGE_SPEED_INX  27
#define  ADAS_LANES_CHANGE_JUDGE_VIDEO_SEC_INX  28
#define  ADAS_LANES_CHANGE_JUDGE_PHOTO_COUNT_INX  29
#define  ADAS_LANES_CHANGE_JUDGE_PHOTO_INTERVAL_INX  30

#define  ADAS_LDW_WARN_SPEED_INX  31
#define  ADAS_LDW_WARN_VIDEO_SEC_INX  32
#define  ADAS_LDW_WARN_PHOTO_COUNT_INX  33
#define  ADAS_LDW_WARN_PHOTO_INTERVAL  34

#define  ADAS_FCW_WARN_TIME_THRESHOLD_INX  35
#define  ADAS_FCW_WARN_SPEED_THRESHOLD_INX  36
#define  ADAS_FCW_WARN_VIDEO_SEC_INX  37
#define  ADAS_FCW_WARN_PHOTO_COUNT_INX  38
#define  ADAS_FCW_WARN_PHOTO_INTERVAL_INX  39

#define  ADAS_PCW_WARN_TIME_THRESHOLD_INX  40
#define  ADAS_PCW_WARN_SPEED_THRESHOLD_INX  41
#define  ADAS_PCW_WARN_VIDEO_SEC_INX  42
#define  ADAS_PCW_WARN_PHOTO_COUNT_INX  43
#define  ADAS_PCW_WARN_PHOTO_INTERVAL_INX  44

#define  ADAS_HMWS_WARN_DISTANCE_THRESHOLD_INX  45
#define  ADAS_HMWS_WARN_SPEED_THRESHOLD_INX  46
#define  ADAS_HMWS_WARN_VIDEO_SEC_INX  47
#define  ADAS_HMWS_WARN_PHOTO_COUNT_INX  48
#define  ADAS_HMWS_WARN_PHOTO_INTERVAL_INX  49

#define  ADAS_ROAD_MARK_PHOTO_COUNT_INX  50
#define  ADAS_ROAD_MARK_PHOTO_INTERVAL_INX  51
#define  ADAS_RESERVED_ARRAY_INX  52
#define  ADAS_SET_PARAM_ARRAY_LEN  56
#endif


#ifndef  DSM_PARAMS_ID
#define  DSM_PARAMS_ID  0xF365
#define  DSM_WARN_SPEED_THRESHOLD_INX  0
#define  DSM_WARN_VOLUME_INX  1
#define  DSM_WARN_PHOTO_LOGIC_INX  2
#define  DSM_WARN_TIMING_PHOTO_INTERVAL_INX  3
#define  DSM_WARN_DISTANCE_PHOTO_INTERVAL_INX  5
#define  DSM_WARN_PHOTO_COUNT_INX  7
#define  DSM_WARN_PHOTO_INTERVAL_INX  8
#define  DSM_WARN_PHOTO_RESOLUTION_INX  9
#define  DSM_WARN_VIDEO_RESOLUTION_INX  10
#define  DSM_WARN_ENABLE_INX  11
#define  DSM_EVENT_ENABLE_INX  15

#define  DSM_SMK_WARN_INTERVAL_INX  19
#define  DSM_PHN_WARN_INTERVAL_INX  21
#define  DSM_RESERVED_ARRAY_1_INX  23

#define  DSM_FD_WARN_GRADE_SPEED_THRESHOLD_INX  26
#define  DSM_FD_WARN_VIDEO_SEC_INX  27
#define  DSM_FD_WARN_PHOTO_COUNT_INX  28
#define  DSM_FD_WARN_PHOTO_INTERVAL_INX  29

#define  DSM_PHN_WARN_GRADE_SPEED_THRESHOLD_INX  30
#define  DSM_PHN_WARN_VIDEO_SEC_INX  31
#define  DSM_PHN_WARN_PHOTO_COUNT_INX  32
#define  DSM_PHN_WARN_PHOTO_INTERVAL_INX  33

#define  DSM_SMK_WARN_GRADE_SPEED_THRESHOLD_INX  34
#define  DSM_SMK_WARN_VIDEO_SEC_INX  35
#define  DSM_SMK_WARN_PHOTO_COUNT_INX  36
#define  DSM_SMK_WARN_PHOTO_INTERVAL_INX  37

#define  DSM_DST_WARN_GRADE_SPEED_THRESHOLD_INX  38
#define  DSM_DST_WARN_VIDEO_SEC_INX  39
#define  DSM_DST_WARN_PHOTO_COUNT_INX  40
#define  DSM_DST_WARN_PHOTO_INTERVAL_INX  41

#define  DSM_ABNML_WARN_GRADE_SPEED_THRESHOLD_INX  42
#define  DSM_ABNML_WARN_VIDEO_SEC_INX  43
#define  DSM_ABNML_WARN_PHOTO_COUNT_INX  44
#define  DSM_ABNML_WARN_PHOTO_INTERVAL_INX  45

#define  DSM_DRIVER_ID_RECOG_TRIGER_INX  46
#define  DSM_RESERVED_ARRAY_2_INX  47
#define  DSM_SET_PARAM_ARRAY_LEN  49
#endif


#ifndef  TPMS_PARAMS_ID
#define  TPMS_PARAMS_ID  0xF366

#define  TPMS_TIRE_MODEL_INX  0
#define  TPMS_TIRE_PRESS_UNIT_INX  12
#define  TPMS_TIRE_NOML_PRESS_VAL_INX  14
#define  TPMS_PRESS_UNBLANCE_THRESHOLD_INX  16
#define  TPMS_SLOW_LEAK_THRESHOLD_INX  18
#define  TPMS_LOW_PRESS_THRESHOLD_INX  20
#define  TPMS_HIG_PRESS_THRESHOLD_INX  22
#define  TPMS_HIG_TEMP_THRESHOLD_INX  24
#define  TPMS_VOLTAGE_THRESHOLD_INX  26
#define  TPMS_REPORT_PERIOD_INX  28
#define  TPMS_RESVER_ARRAY       30
#define  TPMS_SET_PARAM_ARRAY_LEN  36
#endif

#ifndef  BSD_PARAMS_ID
#define  BSD_PARAMS_ID  0xF367
#define  BSD_TAIL_WARN_TIME_THRESHOLD_INX  0
#define  BSD_SIDE_TAIL_WARN_TIME_THRESHOLD_INX  1
#define  BSD_SET_PARAM_ARRAY_LEN  2
#endif

typedef struct {
	unsigned int dev_ID;
	unsigned char params_len;
	unsigned char *params_info_buf;
} Dev_Params_Info;


typedef struct {
	unsigned char add_info_ID;
	unsigned char add_info_len;
	unsigned char *add_info_buf;
} Dev_Warn_Info;


#ifndef  BASIC_LOCATION_INFO
#define  BASIC_LOCATION_INFO
#define  BLI_WARN_FLAG_INX  0
#define  BLI_STATE_INX  4
#define  BLI_LATITUDE_INX  8
#define  BLI_LONGITUDE_INX  12
#define  BLI_ALTITUDE_INX  16
#define  BLI_SPEED_INX  18
#define  BLI_DIRECTION_INX  20
#define  BLI_DATE_TIME_INX  22
#define  BLI_ARRAY_LNE  28
#endif

#define  ADAS_ADD_INFO_LEN  47
#define  DSM_ADD_INFO_LEN  47
#define  TPMS_ADD_INFO_LEN  49
#define  BSD_ADD_INFO_LEN  41

#define  WARN_ADD_INFO_ID_INX  28
#define  WARN_ADD_INFO_LEN_INX  29

#define  ADAS_ADD_INFO_WARN_ID_INX  30
#define  ADAS_ADD_INFO_FLAG_STATE_INX  34
#define  ADAS_ADD_INFO_WARN_TYPE_INX  35
#define  ADAS_ADD_INFO_WARN_LEVEL_INX  36
#define  ADAS_ADD_INFO_FRONT_CAR_SPEED_INX  37
#define  ADAS_ADD_INFO_FRONT_CAR_DISTANCE_INX  38
#define  ADAS_ADD_INFO_DEPARTURE_TYPE_INX  39
#define  ADAS_ADD_INFO_SPEED_INX  42
#define  ADAS_ADD_INFO_ALTITUDE_INX  43
#define  ADAS_ADD_INFO_LATITUDE_INX  45
#define  ADAS_ADD_INFO_LONGITUDE_INX  49
#define  ADAS_ADD_INFO_DATE_TIME_INX  53
#define  ADAS_ADD_INFO_VEHICLE_STATE_INX  59
#define  ADAS_ADD_INFO_WARN_ID_NUM_INX  61
#define  ADAS_ADD_INFO_TOTAL_LEN_INX  77

#define  DSM_ADD_INFO_WARN_ID_INX  30
#define  DSM_ADD_INFO_SPEED_INX  42
#define  DSM_ADD_INFO_ALTITUDE_INX  43
#define  DSM_ADD_INFO_LATITUDE_INX  45
#define  DSM_ADD_INFO_LONGITUDE_INX  49
#define  DSM_ADD_INFO_DATE_TIME_INX  53
#define  DSM_ADD_INFO_VEHICLE_STATE_INX  59
#define  DSM_ADD_INFO_WARN_ID_NUM_INX  61
#define  DSM_ADD_INFO_TOTAL_LEN_INX  77

#define  TPMS_ADD_INFO_WARN_ID_INX  30
#define  TPMS_ADD_INFO_SPEED_INX  35
#define  TPMS_ADD_INFO_ALTITUDE_INX  36
#define  TPMS_ADD_INFO_LATITUDE_INX  38
#define  TPMS_ADD_INFO_LONGITUDE_INX  42
#define  TPMS_ADD_INFO_DATE_TIME_INX  46
#define  TPMS_ADD_INFO_VEHICLE_STATE_INX  52
#define  TPMS_ADD_INFO_WARN_ID_NUM_INX  54
#define  TPMS_ADD_INFO_WARN_LIST_NUM_INX  69
#define  TPMS_ADD_INFO_WARN_POS_INX  70
#define  TPMS_ADD_INFO_WARN_TYPE_INX  72
#define  TPMS_ADD_INFO_PRESSURE_INX  74
#define  TPMS_ADD_INFO_TEMP_INX  76
#define  TPMS_ADD_INFO_BATTERY_POWER_INX  78
#define  TPMS_ADD_INFO_TOTAL_LEN_INX  80

#define  BSD_ADD_INFO_WARN_ID_INX  30
#define  BSD_ADD_INFO_SPEED_INX  36
#define  BSD_ADD_INFO_ALTITUDE_INX  37
#define  BSD_ADD_INFO_LATITUDE_INX  39
#define  BSD_ADD_INFO_LONGITUDE_INX  43
#define  BSD_ADD_INFO_DATE_TIME_INX  47
#define  BSD_ADD_INFO_VEHICLE_STATE_INX  53
#define  BSD_ADD_INFO_WARN_ID_NUM_INX  55
#define  BSD_ADD_INFO_TOTAL_LEN  71


#define  VEHICLE_STATE_DATA_BLOCKS_INX  0
#define  VEHICLE_STATE_DATA_CUR_BLOCK_INX  4
#define  VEHICLE_STATE_DATA_WARN_FLAG_INX  8
#define  VEHICLE_STATE_DATA_CAR_STATE_INX  12
#define  VEHICLE_STATE_DATA_LATITUDE_INX  16
#define  VEHICLE_STATE_DATA_LONGITUDE_INX  20
#define  VEHICLE_STATE_DATA_ALTITUDE_INX  24
#define  VEHICLE_STATE_DATA_SPEED_INX  26
#define  VEHICLE_STATE_DATA_DIRECTION_INx  28
#define  VEHICLE_STATE_DATA_DATE_TIME_INX  30
#define  VEHICLE_STATE_DATA_X_AXIS_ACC_INX  36
#define  VEHICLE_STATE_DATA_Y_AXIS_ACC_INX  38
#define  VEHICLE_STATE_DATA_Z_AXIS_ACC_INX  40
#define  VEHICLE_STATE_DATA_X_AXIS_ANGL_VEL_INX  42
#define  VEHICLE_STATE_DATA_Y_AXIS_ANGL_VEL_INX  44
#define  VEHICLE_STATE_DATA_Z_AXIS_ANGL_VEL_INX  46
#define  VEHICLE_STATE_DATA_PULSE_VEL_INX  48
#define  VEHICLE_STATE_DATA_OBD_VEL_INX  50
#define  VEHICLE_STATE_DATA_GEAR_STATE_INX  52
#define  VEHICLE_STATE_DATA_ACC_PANEL_DUTY_INX  53
#define  VEHICLE_STATE_DATA_BRAKE_PANEL_DUTY_INX  54
#define  VEHICLE_STATE_DATA_BRAKE_STATE_INX  55
#define  VEHICLE_STATE_DATA_ENGINE_RPM_INX  56
#define  VEHICLE_STATE_DATA_WHEEL_POS_INX  58
#define  VEHICLE_STATE_DATA_TURN_LIGHT_STATE_INX  60
#define  VEHICLE_STATE_DATA_REVERSE_INX  61
#define  VEHICLE_STATE_DATA_CHECK_INX  63
#define  VEHICLE_STATE_DATA_ARRAY_LEN  64


typedef enum {
	ORIG_STATE = 0,
	INIT_STATE = 1,
	SEDNDING_STATE = 2,
	SENT_OK = 3,
	SENT_INCOMPLETE = 4,
} File_Sent_State;

typedef struct {
	unsigned char file_type; //00: picture, 01: audio, 02: video, 03: text, 04: others
	File_Sent_State file_sent_state;  //00: empty, 01:full, 02: sending, 03: sent
	char *file_name;
	char *local_file_name;
	unsigned int  file_data_len;
} Attach_File;


typedef enum {
	CONNECT_SERVER = 1,
	UPLOAD_ATTACH_INFO = 2,
	UPLOAD_FILE_INFO = 3,
	UPLOAD_FILE_DATA = 4,
	UPLOAD_FINISH_INFO = 5,
	DISCONNECT_SERVER = 6,
	WAIT_SERVER_REPLY = 7,
} Attach_Upload_Seq;


typedef struct {
	unsigned char dev_ID;
	unsigned char terminal_ID[7];
	unsigned char warn_tag[16];
	unsigned char warn_SN[32];
	unsigned char files_count;
	unsigned char upload_files_count;
	Attach_Upload_Seq file_upload_seq;
	//Dev_Warn_Info *dev_warn_info;
	Attach_File *attach_file_array;
} Dev_Warn_Msg;



Dev_Bsc_Info *dev_bsc_info_list;
Dev_Params_Info *dev_params_info_list;
Dev_Warn_Info *dev_warn_info_list;
Dev_Warn_Msg *dev_warn_msg;

Net_Connection *archive_server_connection;


typedef struct {
	unsigned char phone_num[6];
	unsigned char register_list[64];
	char *authen_str;
	Dev_Bsc_Info *bsc_info_list;
	Dev_Params_Info *params_info_list;
	Dev_Warn_Info *bsc_warn_info_list;
	Dev_Warn_Msg  *warn_file_list;
} JSATL_Dev;

JSATL_Dev *jsatl_dev;

void parse_msg_from_BG_task(void *connection);
void authen_and_warn_process_task(void *connection);
int init_protocol_JSATL(Dev_Bsc_Info **dev_bsc_info_list);
int init_protocol_params(Dev_Params_Info **dev_params_info_list);
int init_warn_info(Dev_Warn_Info **dev_warn_info_list);
int init_warn_info_msg(Dev_Warn_Msg **dev_warn_msg);











#endif /* JSATL_H_ */
