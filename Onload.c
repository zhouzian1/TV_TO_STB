#define LOG_TAG "OnLoad"
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <jni.h>
#include <string.h>
//#include<cutils/log.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <math.h>
#include <asm/types.h>
#include <sys/poll.h>
#include <sys/socket.h>  
#include <linux/netlink.h>

#include "list.h"
#include "copy.h"


#define MAX_LINE 1024
#define MAX_BUF_SIZE 100
#define CODE_VALUE_BUF_SIZE 14
#define CODE_ENTRY_BUF_SIZE 30

#define BAUDRATE B38400
#define DATA_HEAD 0xa5
#define DATA_TAIL 0xb6

#define PID_VID "248a/883e/100"
#define ACTION_ADD "ACTION=add"
#define ACTION_REMOVE "ACTION=remove"
 



static int Opendev=1;
static JavaVM* sVm;
static int fd_key = -1;
struct input_event event;
struct code_data_node code_list ;
struct pollfd fds[1] ;


void jni_test();
void open_device_keycode();
int get_device_keycode();
int init_code_table(char* path);
int match_stb_value(int code);
//int serial_send_data(char* buf, int len);
//int serial_rece_data(int len);




//jni test
void jni_test()
{
	LOGI("jni test zzz");
}


//open IR device
void open_device_keycode()
{
	printf("open\n");
	fd_key=open("/dev/input/event1",O_RDWR);
	if(fd_key<0)
		//__android_log_print(ANDROID_LOG_INFO, "Codemapping", "open device failed");	
		LOGE("open device failed \n");
	else
		LOGD("open device sccuess \n");

        fds[0].fd = fd_key ;
        fds[0].events = POLLIN ;
		
}



//get key_code value form IR device
int get_device_keycode()
{
        int retv;
        LOGD("get device key-code");
        if(Opendev==1){
              open_device_keycode();
              Opendev=0;
        }


        while(1){
                retv = poll(fds, 1, -1);
                if(retv == -1){
                      LOGE("poll err");
                      return -1 ;
                }else if(!retv){
                        LOGE ("seconds elapsed.");
                        return -1 ;
                }
                else if(fds[0].revents & POLLIN){


			LOGD("****************have read code************************\n");

			int count = read(fd_key, &event, sizeof(struct input_event));
			if(EV_KEY == event.type && 1 == event.value){
			//printf("zzztime : %ld, %d \t", ev_temp.time.tv_sec, ev_temp.time.tv_usec);
			LOGD("zzgetcode code: %d, value: %d \n",event.code,event.value);
			//__android_log_print(ANDROID_LOG_INFO, "Codemapping", "getcode code: %d, value: %d",event.code,event.value);
			close(fd_key);
			return event.code;
			}


                }
        }

		close(fd_key);	
		return -1;
}


//initialise TV convert to  STB table
int init_code_table(char* path)
{
	LOGI("init_code_table");
	
	INIT_LIST_HEAD(&(code_list.list));
	LOGI("start.....za.... .....init.....................\n");
	struct code_data_node *tmp ;
	struct list_head *pos,*q ;
	unsigned int i;
	char delims[] = "#";
    char *result = NULL;
	
	char buf[MAX_LINE]; /* ������ */
	FILE *fp;
	int len;
	
	LOGI("config file path %s",path);
	if((fp = fopen(path, "r")) == NULL){ /* ���ļ� */
		LOGE("fail to load config file");
		return -1;
	}


	while(fgets(buf, MAX_LINE, fp) != NULL){ /* ÿ�ζ���һ�� */
		len = strlen(buf);
		/* �����������ַ��������ַ������� */
		buf[len - 1] = '\0'; /* ȥ�����з�����������ַ���Ϳ��Դ����� */
		
		tmp = (struct code_data_node*)malloc(sizeof(struct code_data_node));
		memset(tmp,0,sizeof(struct code_data_node));
		
		result = strtok( buf, delims );
		while( result != NULL ) {
			LOGI( "result is \"%s\"\n", result );
			LOGI("convert................\n");
			if(tmp->tv_code_value==0)
				tmp->tv_code_value = atol(result);
				// LOGD("%d",atoi(result));
				
			else if(tmp->stb_code_value==0)
				tmp->stb_code_value = atol(result);	
				// LOGD("%d",atoi(result));
			result = strtok( NULL, delims );
		} 

		LOGI("tv_code_value = %d, stb_code_value= %d\n", tmp->tv_code_value, tmp->stb_code_value);
		LOGI("\n");
		list_add(&(tmp->list), &(code_list.list));//ͷ�巨����ڵ�
		//printf("%s %d\n", buf, len - 1); /* ʹ��printf������� */
	}
	
	
	LOGI("list for each.........\n");
	list_for_each(pos, &(code_list.list)) //����ڵ�
	{

		tmp=list_entry(pos, struct code_data_node, list);//ȡ����ǰlist�ڵ��ָ��code_data_node�ĵ�ַ

		LOGI("tv_code_value = %d, stb_code_value= %d\n", tmp->tv_code_value, tmp->stb_code_value);//��ӡ��kool_list�Ľṹ���Ա����

	}

	LOGI("\n");
	return 1;
}

//match STB value
int match_stb_value(int code)
{
	struct code_data_node *tmp ;
	struct list_head *pos;
	list_for_each(pos, &(code_list.list)) //����ڵ�
		{

			tmp=list_entry(pos, struct code_data_node, list);//ȡ����ǰlist�ڵ��ָ��code_data_node�ĵ�ַ

			if(code==tmp->tv_code_value){
				LOGD("tv_code_value = %d, stb_code_value= %d\n", tmp->tv_code_value, tmp->stb_code_value);//��ӡ��kool_list�Ľṹ���Ա����
				return tmp->stb_code_value ; 
			}
		}
	return -1;
}

//serial send data
//int serial_send_data(char* buf, int len)
//{
//	int ret , fd;
//	fd =open("/dev/ttyACM0",O_RDWR|O_NOCTTY);
//	if(fd<0){
//		LOGE("open /dev/ttyACM0");
//		return -1;
//	}
//
//	//char buf[]={"SSello, serial port send test\n"};
//	struct termios option;
//	// int length = sizeof(buf);
//	int length = len ;
//	LOGD("ready for sending data.....length=%d",length);
//
//	tcgetattr(fd,&option);
//	cfmakeraw(&option);
//
//	cfsetispeed(&option,BAUDRATE);
//	cfsetospeed(&option,BAUDRATE);
//
//	tcsetattr(fd,TCSANOW,&option);
//
//	ret=write(fd,buf,length);
//	if(ret<0){
//		LOGE("write....");
//		close(fd);
//		return -1;
//	}
//	close(fd);
//	return len;
//
//}

//
//serial receive data
//int serial_rece_data(int len)
//{
//	int fd, retv;
//	char  hd[MAX_BUF_SIZE],*rbuf;
//	struct termios option;
//
//	fd = open("/dev/ttyACM0",O_RDWR|O_NOCTTY);
//	if(fd<0){
//			LOGE("can't open /dev/ttyACM0");
//			return -1;
//	}
//
//	tcgetattr(fd,&option);
//	cfmakeraw(&option);
//
//	cfsetispeed(&option,BAUDRATE); /*38400bps*/
//	cfsetospeed(&option,BAUDRATE);
//
//	tcsetattr(fd,TCSANOW,&option);
//	rbuf=hd;
//	LOGD("ready for receiving data...\n");
//	retv=read(fd,rbuf,len);
//	if(retv==-1){
//		LOGE("read error");
//		return -1;
//	}
//	LOGD("The data received is:\n");
//	LOGD("%s",rbuf);
//
//	close(fd);
//	return 1;
//
//}






int check_config_file(char* path)
{
	FILE *fp;
	char ch;
	if((fp=fopen(path,"r"))==NULL)
	{
		LOGE("open config file error!");
		return -1;
	}
	ch=fgetc(fp);
	if(ch==EOF){
	LOGD("file is empty");
	return -1 ;
	}
		else{
		LOGD("file is not empty");
		return 1 ;
	}
}


int monitor_netlink_uevent()
    {
        int sockfd;
        struct sockaddr_nl sa;
        int len;
        char buf[4096];
        struct iovec iov;
        struct msghdr msg;
        int i;
		// char *des = "1a1d/40/100";
		// char *string_pid = "248a/883e/100";
		char *string_pid = "951/1642/100";
		char *string_action_add="ACTION=add";
		char *string_action_remove="ACTION=remove";
		char *ptr;
		char *ptr_action_add;
		char *ptr_action_remove;

        memset(&sa,0,sizeof(sa));
        sa.nl_family=AF_NETLINK;
        sa.nl_groups=NETLINK_KOBJECT_UEVENT;
        sa.nl_pid = 0;//getpid(); both is ok
        memset(&msg,0,sizeof(msg));
        iov.iov_base=(void *)buf;
        iov.iov_len=sizeof(buf);
        msg.msg_name=(void *)&sa;
        msg.msg_namelen=sizeof(sa);
        msg.msg_iov=&iov;
        msg.msg_iovlen=1;

        sockfd=socket(AF_NETLINK,SOCK_RAW,NETLINK_KOBJECT_UEVENT);
        if(sockfd==-1)
            LOGE("socket creating failed:%s",strerror(errno));
        if(bind(sockfd,(struct sockaddr *)&sa,sizeof(sa))==-1)
            LOGE("bind error:%s",strerror(errno));

		LOGD("zzzzzzzzzzzzzz  usb ");
			len=recvmsg(sockfd,&msg,0);
			if(len<0)
				LOGE("receive error");
			else if(len<32||len>sizeof(buf))
				LOGE("invalid message");
			for(i=0;i<len;i++)
				if(*(buf+i)=='\0')
					buf[i]='\n';
			LOGD("received %d bytes %s",len,buf);
			ptr=strstr(buf, string_pid);
			ptr_action_add=strstr(buf, string_action_add);
			ptr_action_remove=strstr(buf, string_action_remove);
			if(ptr!=NULL&&ptr_action_add!=NULL){
				LOGD("zzzzzzzzzzzzzza add device");
				return 1;
			}
			else if(ptr!=NULL&&ptr_action_remove!=NULL){
				LOGD("qqqqqqqqqqqqqqa remove device");
				return 0;
			}
			
		return -1;
			
    }
/////////////////////////////////////////////////native methods////////////////////////////////////////////////
void native_jnitest(JNIEnv* env, jobject obj)
{
	LOGD("native_jnitest");
	jni_test();
	
}

int native_get_code(JNIEnv* env, jobject obj)
{
	int code;
	LOGD("native_get_code");
	return code = get_device_keycode();
}


int native_init(JNIEnv* env, jobject obj, jstring path)
{
	LOGD("native_init");
	jboolean isCopy;
	char* config_path = (*env)->GetStringUTFChars(env, path, &isCopy);
	int ret = init_code_table(config_path);
	(*env)->ReleaseStringUTFChars(env, path, config_path);
	return ret ;
}

int native_match_value(JNIEnv* env, jobject obj, jint code)
{
	LOGD("native_match");
	code = match_stb_value(code);
	return code;
}

//int native_send_data(JNIEnv* env, jobject obj, jstring buf)
//{
//	LOGD("native_send_data");
//	jboolean isCopy;
//	char* data = (*env)->GetStringUTFChars(env, buf, &isCopy);
//	LOGE("%s",data);
//	int len = (*env)->GetStringUTFLength(env, buf);
//	LOGE("%d",len);
//	int ret = serial_send_data(data, len);
//	(*env)->ReleaseStringUTFChars(env, buf, data);
//	return ret ;
//}

//int native_rece_data(JNIEnv* env, jobject obj, jint len)
//{
//	LOGD("native_rece_data");
//	int ret = serial_rece_data(len);
//	if(ret<0){
//		LOGE("read data error");
//		return -1;
//	}
//	return ret;
//
//}


int native_check_config(JNIEnv* env, jobject obj, jstring path)
{
	LOGD("native check config");
	
	jboolean isCopy;
	char* config_path = (*env)->GetStringUTFChars(env, path, &isCopy);
	LOGE("%s",config_path);
	int ret = check_config_file(config_path);
	(*env)->ReleaseStringUTFChars(env, path, config_path);
	return ret ;
}

int native_usb_monitor(JNIEnv* env, jobject obj)
{
	LOGD("native usb monitor");
	int ret = monitor_netlink_uevent();
	return ret;
	
}

int native_update_table(JNIEnv* env, jobject obj, jstring path)
{
        LOGD("native update table");

        jboolean isCopy;
        char* config_path = (*env)->GetStringUTFChars(env, path, &isCopy);
        int ret = update_code_table(config_path);
        (*env)->ReleaseStringUTFChars(env, path, config_path);
        return ret;
}

int native_send_code(JNIEnv* env, jobject obj, jchar code)
{
        LOGD("native send code");
        int ret = send_code_value(code);
        return ret ;
}

int native_send_config(JNIEnv* env, jobject obj)
{
      LOGD("native send configuration information");
      int ret =  send_config_info();
      return ret ;
}
////////////////////////////////////////////JNI COMMON START//////////////////////////////////////////////////

//static JavaVM* sVm;

/*
 * methods list
 *
 * typedef struct {
 * const char* name;
 * const char* signature;	 (...)V/Z...
 * void* fnPtr;		} JNINativeMethod;
 *
 *
 *��    	Java         	C
 ****************************************
 *V     	void         	void
 *Z      	jboolean     	boolean
 *I      	jint         	int
 *J      	jlong        	long
 *D      	jdouble      	double
 *F      	jfloat       	float
 *B      	jbyte        	byte
 *C      	jchar        	char
 *S      	jshort       	short
 ****************************************
 *[I     	jintArray       int[]
 *[F     	jfloatArray     float[]
 *[B     	jbyteArray      byte[]
 *[C     	jcharArray      char[]
 *[S     	jshortArray     short[]
 *[D     	jdoubleArray    double[]
 *[J     	jlongArray      long[]
 *[Z     	jbooleanArray   boolean[]
 ********************class*******************
 *class java/lang/String; String jstring
 */
static JNINativeMethod methods[] =
		{
				{"jni_test", "()V", (void*)native_jnitest },
				{"get_code_value", "()I",(void*)native_get_code },
				{"init_code_table", "(Ljava/lang/String;)I", (void*)native_init },
				{"match_stb_value", "(I)I", (void*)native_match_value},
				// {"match_serial_device", "(I)I", (void*)native_match_device},
//				{"serial_send_data", "(Ljava/lang/String;)I", (void*)native_send_data},
//				{"serial_rece_data", "(I)I", (void*)native_rece_data},
				{"check_config_file", "(Ljava/lang/String;)I", (void*)native_check_config},
				{"usb_monitor", "()I", (void*)native_usb_monitor},
				{"update_code_table;", "(Ljava/lang/String;)I", (void*)native_update_table},
				{"send_code_value", "(C)I", (void*)native_send_code},
				{"send_config_info", "()I", (void*)native_send_config},
		};

//className for register


char* className = "com/skyworth/device/Codemapping";

/*
 * register jni, the mothed called by OnLoad
 */
int register_JNI(JNIEnv *env)
{
	LOGD("register_JNI");

	jclass clazz;
	clazz = (*env)->FindClass(env, className);

	int numMethods = sizeof(methods) / sizeof(methods[0]);

	if (clazz == NULL)
	{
		LOGD("clazz == NULL");
		return -1;
	}

	if ((*env)->RegisterNatives(env, clazz, methods, numMethods) < 0)
	{
		LOGD("env->RegisterNatives() < 0");
		return -1;
	}

	LOGD("register_JNI ok");
	return 0;
}
;

/*
 *for throw exception
 */
int jniThrowException(JNIEnv* env, const char* className, const char* msg)
{
	jclass exceptionClass = (*env)->FindClass(env, className);
	if (exceptionClass == NULL)
		return -1;

	if ((*env)->ThrowNew(env, exceptionClass, msg) != JNI_OK)
	{
		return -1;
	}

	return 0;
}

//JNI_ONLOAD , the method will be called when the library on load
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	LOGD("JNI_OnLoad");
	JNIEnv* env = NULL;
	jint result = JNI_ERR;
	sVm = vm;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOGD("vm->GetEnv() != JNI_OK");
		return result;
	}

	if (register_JNI(env) != JNI_OK)
	{
		LOGD("register_JNI() != JNI_OK");
		return result;
	}

	result = JNI_VERSION_1_4;

	LOGD("OnLoad ok");
	return result;
}

////////////////////////////////////////////JNI COMMON END//////////////////////////////////////////////////
