#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<malloc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<math.h>


#define BUF_OTHER_ITME_SIZE 12
#define CODE_STATUS_BUF_SIZE 12
#define CODE_VALUE_BUF_SIZE 13
#define CODE_ENTRY_BUF_SIZE 100

#define CODE_DATA_SIZE 1
#define CODE_UPDATE_START_SIZE 4
#define CODE_VERSION_SIZE 2
#define NO_CONFIG_DATA_SIZE 2
#define CONFIG_DATA_SIZE 6
#define CODE_UPDATE_DATA_SIZE 80

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
int update_start = 0;
int update_end = 0;

char config_info[5] ={0} ;

// frame_num = rand();

char get_check_code(char *data, signed short length)
{
    char check_code = 0;
    char *ptemp = data;
    while(length)
    {
        check_code += *ptemp;
        ptemp++;
        length--;
    }
    
    return check_code;
}


void create_buf(char* buf, short frame_no, char order, short length, char* data_buf)
{
	int i;
	char head, tail, check_code;
	char* check_buf ;
	head = DATA_HEAD ;
	tail = DATA_TAIL ;
	
	check_buf = buf ;
	for(i=0; i<3; i++){
			memcpy(buf, &head, sizeof(char));
			 buf+=sizeof(char);
			
		}
		memcpy(buf, &frame_no, sizeof(short));
		buf+=sizeof(short);
		memcpy(buf, &order, sizeof(char));
		buf+=sizeof(char);
		// printf("buf = %x %x %x %x %x %x \n",p[0],p[1],p[2],p[3],p[4],p[5]);
		memcpy(buf, &length, sizeof(short));
		buf+=sizeof(short);
		// printf("buf = %x %x %x %x %x %x %x \n",p[0],p[1],p[2],p[3],p[4],p[5],p[6]);
		memcpy(buf, data_buf, length);
		buf+=length;
		// printf("buf = %x %x %x %x %x %x %x %s\n",p[0],p[1],p[2],p[3],p[4],p[5],p[6],buf);
		
		check_code = get_check_code(check_buf+3, 2+1+2+length);
		memcpy(buf,&check_code, sizeof(char));
		buf+=sizeof(char);
		printf("check_code  = %x \n",check_code);
		
		for(i=0; i<3; i++){
			//buf = code_value_buf ;
			memcpy(buf, &tail, sizeof(char));
			buf+=sizeof(char);
			// printf("buf = %x %x %x %x %x %x %x  %x %x %x %x\n",p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[14],p[15],p[1]);
		}
		// printf("buf = %s \n",buf);
		//printf("buf is %s",buf);
		//return p ;

}

//ture 1 , false -1 , no config 0, config 2

int analyze_buf(char* buf, short length)
{
	char* tmp_buf;
	char frame[2], len ,order, tmp[13];
	char* data_buf=NULL ;
	char head[3], tail[3], check_code, check;
	int i;
	short frame_no;
	
	// buf = tmp;
	
	printf("analyze buf = %p  ltheng= %d\n",buf,length);
	for(i=0;i<length;i++)
		{
			printf("%x \t",buf[i]);
		}
	printf("\n");
	tmp_buf = buf ;
	
	check_code = get_check_code(buf+3, length-7);
		// for(i=0; i<3; i++){
			memcpy(head,tmp_buf, 3*sizeof(char));
			tmp_buf+=3*sizeof(char);
			for(i=0; i<3; i++){
				printf("head[%d] = %x \n",i,*(head+i));
			}
		memcpy(&frame_no, tmp_buf, sizeof(short));
		tmp_buf+=sizeof(short);
		// frame_no = ((short)frame[1]<<8)+(short)frame[0] ;
		printf("frame_no = %x \n",frame_no);
		
		memcpy(&order, tmp_buf, sizeof(char));
		tmp_buf+=sizeof(char);
		printf("order = %x \n",order);
		
		memcpy(&len, tmp_buf, sizeof(short));
		tmp_buf+=sizeof(short);
		printf("len = %x \n",len);
		
		data_buf=(char*)malloc(len*sizeof(char));
		if(data_buf==NULL){
			printf("malloc failed \n");
			return -1;
		}
		memcpy(data_buf, tmp_buf, len);
		printf("data buf  = %x \n",*data_buf);
		tmp_buf+=len;
		
		memcpy(&check,tmp_buf, sizeof(char));
		printf("check  = %x \n",check);
		tmp_buf+=sizeof(char);
		
		// for(i=0; i<3; i++){
			memcpy(tail, tmp_buf, 3*sizeof(char));
			tmp_buf+=3*sizeof(char);
		for(i=0; i<3; i++){	
			printf("tail[%d] = %x \n",i,*(tail+i));
		}
		
		
		if(frame_no!=frame_num){
			printf("frame_no error \n");
			free(data_buf);
			return -1;
		}
		for(i=0; i<3; i++){
			if(head[i]!=DATA_HEAD){
				printf("head error \n");
				free(data_buf);
				return -1;
			}
		}
		
		for(i=0; i<3; i++){
			if(tail[i]!=DATA_TAIL){
				printf("tail error \n");
				free(data_buf);
				return -1;
			}
		}
		
		
		if(order!=ACK_CMD){
			printf("code order error \n");
			free(data_buf);
			return -1;
		
		}
		
		if(check_code!=check){
			printf("check error \n");
			free(data_buf);
			return -1;
		}
		
		if(!(len==CODE_DATA_SIZE || len==CONFIG_DATA_SIZE)){
		
			printf("code data len error \n");
			free(data_buf);
			return -1 ;
			// printf("len ture\n");
		}
		
		if(len==CODE_DATA_SIZE){
			printf("code data size = 1 \n");
			if(*data_buf==ACK){
				printf("rece ACK \n");
				frame_num++;
				printf("frame_num = %x \n",frame_num);
				free(data_buf);
				return 1;
			}else if(*data_buf==NACK){
				printf("rece NACK \n");
				free(data_buf);
				return -1;
			}else{
				printf("rece other NACK \n");	
				free(data_buf);
				return -1;
			}
		}else if(len==CONFIG_DATA_SIZE){
				printf("code data size = 6 \n");
				if(*data_buf==CONFIG_CHECK){
					printf("config check = 3 \n");
					// data_buf+=1;
					for(i=0; i<CONFIG_DATA_SIZE-1; i++){
						config_info[i]=data_buf[i+1];
						printf("config info[%d]=%x \n",i,config_info[i]);
					}
					// config_info=data_buf;
					// printf("config info[%d]=%x",i,config_info[i]);
					frame_num++;
					printf("frame_num = %x \n",frame_num);
					free(data_buf);
					return 2;
				}else{
					printf("config check !!= 3\n");
					frame_num++;
					printf("frame_num = %x",frame_num);
					printf("rece no config \n");
					free(data_buf);
					return 0;
				}
				
		}
		
		free(data_buf);		
		return -1;
	

}

char* joint_buf(short frame_no, char order, short length, char* data_buf ,int buf_len)
{
	char code_value_buf[CODE_VALUE_BUF_SIZE], code_entry_buf[CODE_ENTRY_BUF_SIZE], *buf;
	char *p;
	int i ;
	 
	switch(order){
	
	case 0x01 :{
		p = malloc( buf_len*sizeof(char));
		buf = p;
		memset(buf, 0, buf_len);
		printf("buf = %s \n",buf);
		create_buf(buf, frame_no, order, length, data_buf);
		return p;
		
		
	} break ;
	
	case 0x02 : {
		p = malloc( buf_len*sizeof(char));
		buf = p;
		memset(buf, 0, buf_len);
		printf("buf = %s \n",buf);
		
		create_buf(buf, frame_no, order, length, data_buf);
		return p;
		
		
	} break ;
	
	 case 0x03 : {
		p = malloc( buf_len*sizeof(char));
		buf = p;
		memset(buf, 0, buf_len);
		printf("buf = %s \n",buf);
		create_buf(buf, frame_no, order, length, data_buf);
		
		return p;
		
	 
	 }break;
	
	// case 0xee : break;
		
	default : break;
	
	
	}
	
	return NULL;
	
	 
}




int serial_send_data(char* buf, int length)
{
	
	int ret,fd,i;
	// fd =open("/dev/ttyACM0",O_RDWR|O_NOCTTY);
	fd =open("/dev/ttyACM0",O_RDWR|O_NOCTTY);
	if(fd<0){
		perror("open /dev/ttyACM0");
	}
	
	printf("serial_send_data\n");
	for(i=0;i<length;i++)
		{
			printf("%x \t",buf[i]);
		}
	printf("\n");
	// char buf[]={"SSello, serial port send test\n"};
	//char buf[]={0xA5,0xA5,0xA5,0x00,0x00,0x01,0x00,0x05,0x01,0x02,0x03,0x04,0x05,0x13,0xB6,0xB6,0xB6};
	struct termios option;
	//int length = sizeof(buf);
	printf("ready for sending data.....\n");
	printf("buf length = %d \n",length);
	
	tcgetattr(fd,&option);
	// option.c_cflag &= ~PARENB;
	// option.c_cflag &= ~CRTSCTS;
	cfmakeraw(&option);
	
	cfsetispeed(&option,B38400);
	cfsetospeed(&option,B38400);
	
	tcsetattr(fd,TCSANOW,&option);
	
	ret=write(fd,buf,length);
	if(ret<0){
		perror("write....");
		close(fd);
		return -1;
	}
	printf("success ret = %d \n", ret);
	close(fd);	
	return 0;
}


char* serial_rece_data(int len)
{
	int fd, retv, i;
	char  hd[ACK_SIZE],*rbuf; 
	struct termios option;
	
	fd_set descriptors;
	struct timeval time_to_wait;

	

	time_to_wait.tv_sec = 0;
	time_to_wait.tv_usec = 1500;
	
	fd = open("/dev/ttyACM0",O_RDWR|O_NOCTTY|O_NONBLOCK);
	if(fd<0){
			printf("can't open /dev/ttyACM0 \n");
			return NULL;
	}
	
	FD_ZERO ( &descriptors );
	FD_SET ( fd, &descriptors );
	
	rbuf=(char*)malloc(len*sizeof(char)+1);
	memset(rbuf,0,len*sizeof(char)+1);
	tcgetattr(fd,&option);
	cfmakeraw(&option);

	cfsetispeed(&option,BAUDRATE); /*38400bps*/
	cfsetospeed(&option,BAUDRATE);

	tcsetattr(fd,TCSANOW,&option);
	// rbuf=hd;
	printf("ready for receiving data...\n");
	// retv=read(fd,rbuf,len); 
	retv=select( fd + 1, &descriptors, NULL, NULL, &time_to_wait);
	
	if ( retv < 0 ) {
		/* Error */
		printf("select error \n");
		return NULL;
	}

	else if ( ! retv ) {
		/* Timeout */
		printf("Time Out \n");
		return NULL;
	}

	else if ( FD_ISSET ( fd, &descriptors ) ) {
		/* Process the inotify events */
			printf("****************have read data************************\n");
		
			retv=read(fd,rbuf,len);
			if(retv<0){
				printf("read error \n");
				close(fd);
				return NULL;
			}
		
		}
		printf("The data received is:\n"); 
	// printf("%x \n",rbuf);
		for(i=0;i<len;i++)
			{
				printf("%x \t",rbuf[i]);
			}
		printf("\n");
		close(fd);
		return rbuf;

		
}



int send_code(char *data, char order, short data_size, int buf_size)
{
	printf("send func \n");
	int i;
	char* buf, *rece_buf;
	char value;
	// value=code;
	printf("data_size =%x  buf_size= %x ",data_size, buf_size);
	buf = joint_buf(frame_num, order, data_size, data, buf_size);
	for(i=0;i<buf_size;i++)
		{
			printf("%x \t",*(buf+i));
		}
	printf("\n");
	while(1){
		int ret = serial_send_data(buf, buf_size);
		if(ret<0){
				printf("send error \n");
				return -1;
			}
		usleep(500);
	
		rece_buf=serial_rece_data(ACK_SIZE);
		if(rece_buf==NULL){
			printf("select read \n");
			continue ;
		}
		// if(rece_buf==NULL){
			// printf("wait 500ms \n");
		
			// usleep(1000);
			// rece_buf=serial_rece_data(ACK_SIZE);
		
			// if(rece_buf==NULL){
				// printf("rece error \n");
				// continue ;
				// }
				
		// }
		printf("zzzzz rece buf = %p \n",rece_buf);
		ret = analyze_buf(rece_buf, ACK_SIZE);
		if(ret==-1){
			printf("zz rece NACK \n");
			// return -1;
			continue ;
		}else if(ret == 1){
			// break ;
			free(buf);
			free(rece_buf);
			return 1 ;
		}
	}
	free(buf);
	free(rece_buf);	
	return 1;
	
}


char* convert_string(char *buf,int* len)
 {
	int code = 0;
	// char str[] = "now # is the time for all # good men to come to the # aid of their country";
	int i=0;
	char delims[] = "#";
	char *split_buf[3] ;
	char *result = NULL;
	result = strtok( buf, delims );
	while( result != NULL ) {
       printf( "result is \"%s\"\n", result );
	   split_buf[i++]=result;
	   
	   code=atoi(result);
	   printf( "code is %d \n",code);
       result = strtok( NULL, delims );
   } 
   
   for(i=0;i<3;i++){
		printf( "split_buf [%d] %s\n", i, split_buf[i]);
   }
   
   result = strcat(split_buf[1],split_buf[2]);
   printf( "join  result = %s\n", result);
   
   *len = strlen(result);
   
   return result;
} 



int update_code_table(char* path){

	char config_buf[MAX_LINE]; /* 缓冲区 */
	char *data_buf;
	char start_buf[4]={0x00,0x00,0x00,0x00};
	char end_buf[4]={0xff,0xff,0xff,0xff};
	FILE *fp;
	int len, length, ret;

	ret = send_code( start_buf ,UPDATE_VALUE_CMD, sizeof(start_buf), BUF_OTHER_ITME_SIZE+sizeof(start_buf));
	if(ret<0){
		printf("send code err \n");
	}
	
	usleep(500);
	if((fp = fopen(path, "r")) == NULL){ /* 打开文件 */
		perror("fail to read");
		return -1 ;
		}
	
	while(fgets(config_buf, MAX_LINE, fp) != NULL){ /* 每次读入一行 */
	len = strlen(config_buf);
	/* 输出所读到的字符画串，并将字符个数输出 */
	config_buf[len - 1] = '\0'; /* 去掉换行符，这样其他的字符串函数就可以处理了 */

	data_buf = convert_string(config_buf, &length);
	printf( "data buf result = %s\n", data_buf);
	
	if(strstr(data_buf, STB_VERSION)!=NULL){
		printf("stb version info \n");
		ret = send_code( data_buf ,UPDATE_VALUE_CMD, length, BUF_OTHER_ITME_SIZE+length); //buf size 14
		if(ret<0){
			printf("send code err \n");
		}
	}else{
		printf("stb data info \n");
		ret = send_code( data_buf ,UPDATE_VALUE_CMD, length, BUF_OTHER_ITME_SIZE+length);
		if(ret<0){
			printf("send code err \n");
		}
	
	}

	printf("%s %d\n", config_buf, len - 1); /* 使用printf函数输出 */
	}

	ret = send_code( end_buf ,UPDATE_VALUE_CMD, sizeof(end_buf), BUF_OTHER_ITME_SIZE+sizeof(end_buf));
	if(ret<0){
		printf("send code err \n");
		return -1;
	}
	return 1;
}


int send_code_value(char code)
{
	printf("sen code value \n");
	int ret = send_code(&code, SEND_VALUE_CMD, sizeof(char), BUF_OTHER_ITME_SIZE+sizeof(char));
	if(ret<0){
		printf("send code err \n");
		return -1;
	}
	
	return 1;
}


int send_config_info()
{
	char* buf, *rece_buf;
	char order = CONFIG_CMD;
	char data = CONFIG_CHECK;
	short data_size = CODE_DATA_SIZE;
	int buf_size = BUF_OTHER_ITME_SIZE+data_size;
	int i;
	printf("send config info\n");
	printf("data_size =%x  buf_size= %x ",data_size, buf_size);
	buf = joint_buf(frame_num, order, data_size, &data, buf_size);
	for(i=0;i<buf_size;i++)
		{
			printf("%x \t",*(buf+i));
		}
	printf("\n");
	while(1){
		int ret = serial_send_data(buf, buf_size);
		if(ret<0){
				printf("send error \n");
				return -1;
			}
		usleep(500);
	
		rece_buf=serial_rece_data(CONFIG_ACK_SZIE);
		if(rece_buf==NULL){
			printf("select read \n");
			continue ;
		}
		
		printf("zzzzz rece buf = %p \n",rece_buf);
		ret = analyze_buf(rece_buf, CONFIG_ACK_SZIE);
		if(ret==0){
			printf("no config info \n");
			return 0;
		}else if(ret==2){
			printf("config info \n");
			return 1;
			
		}else if(ret==-1){
		frame_num++;
		continue;
		}
		

		
	}
}



void main()
{
	printf("aaaa\n");
	int ret ;
	// char code=0x88;
	// ret = send_code_value(code);
	
	// if(ret==1){
		// printf("send code success \n");
	// }
	
	ret = update_code_table("/data/test.txt");
	if(ret==1){
		printf("send code success \n");
	}
	
	// ret = send_config_info();
	// if(ret==1){
		// printf("rece config success \n");
	// }
	// char* test_buf="aaaaaa";
	// char* buf;
	// char* buffer;
	// char a = 0xb3;
	// buf = malloc(3*sizeof(char));
	// printf("buf = %x %x %x\n",buf[0],buf[1],buf[2]);
	// memset(buf, 0, 3*sizeof(char));
	// printf("buf = %x %x %x \n",buf[0],buf[1],buf[2]);
	// memcpy(buf,&a,sizeof(char));
	// memcpy(buf+1,&a,sizeof(char));
	// memcpy(buf+2,&a,sizeof(char));
	// buffer = malloc(sizeof(char));
	// memset(buffer, 0 , 6);
	// buf = joint_buf(0,0x01,6,test_buf);
	// analyze_buf(buf,6);
	// memcpy(buffer, buf+8, 6);
	
	// printf("zzz== %s \n",buffer);
}
