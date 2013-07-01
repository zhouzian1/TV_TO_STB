/*
 * copy.h
 *
 *  Created on: Jun 29, 2013
 *      Author: zhouzian
 */

#ifndef TV_TO_STB_COM_H_
#define TV_TO_STB_COM_H_

#define BUF_OTHER_ITMES_SIZE 12
//#define CODE_STATUS_BUF_SIZE 12
//#define CODE_VALUE_BUF_SIZE 13
//#define CODE_ENTRY_BUF_SIZE 100

#define CODE_DATA_SIZE 1
//#define CODE_UPDATE_START_SIZE 4
//#define CODE_VERSION_SIZE 2
//#define NO_CONFIG_DATA_SIZE 2
#define CONFIG_DATA_SIZE 6
//#define CODE_UPDATE_DATA_SIZE 80

#define ACK_SIZE 13
#define NACK_SIZE 13
#define CONFIG_ACK_SZIE 18
#define NO_CONFIG_ACK_SIZE 18

#define BAUDRATE B38400
#define DATA_HEAD 0xa5
#define DATA_TAIL 0xb6
#define CONFIG_CHECK 0x03

#define SEND_VALUE_CMD 0x01
#define UPDATE_VALUE_CMD 0x02
#define CONFIG_CMD 0x03
#define ACK_CMD 0xee

#define ACK 0x01
#define NACK 0xff
#define MAX_LINE 1024

#define STB_VERSION "version"
static short frame_num = 1 ;


extern char config_info[5];


#define LOGI(...)      __android_log_print(ANDROID_LOG_INFO, __FUNCTION__, __VA_ARGS__)
#define LOGD(...)      __android_log_print(ANDROID_LOG_DEBUG, __FUNCTION__, __VA_ARGS__)
#define LOGE(...)      __android_log_print(ANDROID_LOG_ERROR, __FUNCTION__, __VA_ARGS__)

/*
 * Function
 */

char get_check_code(char *data, signed short length);           //calculate check code

char* joint_buf(short frame_no, char order, short length, char* data_buf ,int buf_len);         //joint frame buffer
void create_buf(char* buf, short frame_no, char order, short length, char* data_buf);           //create frame buffer
int analyze_buf(char* buf, short length);               //split frame buffer


int serial_send_data(char* buf, int length);            //send frame buffer by serial
char* serial_rece_data(int len);                        //receive frame buffer by serial
int send_code(char *data, char order, short data_size, int buf_size);           //send code value and update code table interface
char* convert_string(char *buf,int* len);               //convert string form config.txt


int update_code_table(char* path);              //update code table to STB
int send_code_value(char code);                 //send STB code
int send_config_info();                         //send version configuration info


#endif /* COPY_H_ */
