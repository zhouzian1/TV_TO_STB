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
#include <android/log.h>
#include"copy.h"

char config_info[5]={0};

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

		memcpy(buf, &length, sizeof(short));
		buf+=sizeof(short);

		memcpy(buf, data_buf, length);
		buf+=length;
		

		check_code = get_check_code(check_buf+3, 2+1+2+length);
		memcpy(buf,&check_code, sizeof(char));
		buf+=sizeof(char);
		LOGD("check_code  = %x \n",check_code);
		
		for(i=0; i<3; i++){

			memcpy(buf, &tail, sizeof(char));
			buf+=sizeof(char);

		}
		// LOGD("buf = %s \n",buf);
		//LOGD("buf is %s",buf);
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
	
	LOGD("analyze buf = %p  ltheng= %d\n",buf,length);
	for(i=0;i<length;i++)
		{
			LOGD("%x \t",buf[i]);
		}
	LOGD("\n");
	tmp_buf = buf ;
	
	check_code = get_check_code(buf+3, length-7);
		// for(i=0; i<3; i++){
			memcpy(head,tmp_buf, 3*sizeof(char));
			tmp_buf+=3*sizeof(char);
			for(i=0; i<3; i++){
				LOGD("head[%d] = %x \n",i,*(head+i));
			}
		memcpy(&frame_no, tmp_buf, sizeof(short));
		tmp_buf+=sizeof(short);
		// frame_no = ((short)frame[1]<<8)+(short)frame[0] ;
		LOGD("frame_no = %x \n",frame_no);
		
		memcpy(&order, tmp_buf, sizeof(char));
		tmp_buf+=sizeof(char);
		LOGD("order = %x \n",order);
		
		memcpy(&len, tmp_buf, sizeof(short));
		tmp_buf+=sizeof(short);
		LOGD("len = %x \n",len);
		
		data_buf=(char*)malloc(len*sizeof(char));
		if(data_buf==NULL){
			LOGD("malloc failed \n");
			return -1;
		}
		memcpy(data_buf, tmp_buf, len);
		LOGD("data buf  = %x \n",*data_buf);
		tmp_buf+=len;
		
		memcpy(&check,tmp_buf, sizeof(char));
		LOGD("check  = %x \n",check);
		tmp_buf+=sizeof(char);
		
		// for(i=0; i<3; i++){
			memcpy(tail, tmp_buf, 3*sizeof(char));
			tmp_buf+=3*sizeof(char);
		for(i=0; i<3; i++){	
			LOGD("tail[%d] = %x \n",i,*(tail+i));
		}
		
		
		if(frame_no!=frame_num){
			LOGD("frame_no error \n");
			free(data_buf);
			return -1;
		}
		for(i=0; i<3; i++){
			if(head[i]!=DATA_HEAD){
				LOGD("head error \n");
				free(data_buf);
				return -1;
			}
		}
		
		for(i=0; i<3; i++){
			if(tail[i]!=DATA_TAIL){
				LOGD("tail error \n");
				free(data_buf);
				return -1;
			}
		}
		
		
		if(order!=ACK_CMD){
			LOGD("code order error \n");
			free(data_buf);
			return -1;
		
		}
		
		if(check_code!=check){
			LOGD("check error \n");
			free(data_buf);
			return -1;
		}
		
		if(!(len==CODE_DATA_SIZE || len==CONFIG_DATA_SIZE)){
		
			LOGD("code data len error \n");
			free(data_buf);
			return -1 ;
			// LOGD("len ture\n");
		}
		
		if(len==CODE_DATA_SIZE){
			LOGD("code data size = 1 \n");
			if(*data_buf==ACK){
				LOGD("rece ACK \n");
				frame_num++;
				LOGD("frame_num = %x \n",frame_num);
				free(data_buf);
				return 1;
			}else if(*data_buf==NACK){
				LOGD("rece NACK \n");
				free(data_buf);
				return -1;
			}else{
				LOGD("rece other NACK \n");
				free(data_buf);
				return -1;
			}
		}else if(len==CONFIG_DATA_SIZE){
				LOGD("code data size = 6 \n");
				if(*data_buf==CONFIG_CHECK){
					LOGD("config check = 3 \n");
					// data_buf+=1;
					for(i=0; i<CONFIG_DATA_SIZE-1; i++){
						config_info[i]=data_buf[i+1];
						LOGD("config info[%d]=%x \n",i,config_info[i]);
					}
					// config_info=data_buf;
					// LOGD("config info[%d]=%x",i,config_info[i]);
					frame_num++;
					LOGD("frame_num = %x \n",frame_num);
					free(data_buf);
					return 2;
				}else{
					LOGD("config check !!= 3\n");
					frame_num++;
					LOGD("frame_num = %x",frame_num);
					LOGD("rece no config \n");
					free(data_buf);
					return 0;
				}
				
		}
		
		free(data_buf);		
		return -1;
	

}

char* joint_buf(short frame_no, char order, short length, char* data_buf ,int buf_len)
{
	char *buf,*ptr;
	int i ;
	 
	switch(order){
	
	case 0x01 :{
		ptr = malloc( buf_len*sizeof(char));
		buf = ptr;
		memset(buf, 0, buf_len);
		LOGD("buf = %s \n",buf);
		create_buf(buf, frame_no, order, length, data_buf);
		return ptr;
		
		
	} break ;
	
	case 0x02 : {
		ptr = malloc( buf_len*sizeof(char));
		buf = ptr;
		memset(buf, 0, buf_len);
		LOGD("buf = %s \n",buf);
		
		create_buf(buf, frame_no, order, length, data_buf);
		return ptr;
		
		
	} break ;
	
	 case 0x03 : {
		ptr = malloc( buf_len*sizeof(char));
		buf = ptr;
		memset(buf, 0, buf_len);
		LOGD("buf = %s \n",buf);
		create_buf(buf, frame_no, order, length, data_buf);
		
		return ptr;
		
	 
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
	
	LOGD("serial_send_data\n");
	// for(i=0;i<length;i++)
		// {
			// LOGD("%x \t",buf[i]);
		// }
	// LOGD("\n");
	struct termios option;
	LOGD("ready for sending data.....\n");
	LOGD("buf length = %d \n",length);
	
	tcgetattr(fd,&option);
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
	LOGD("success ret = %d \n", ret);
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
			LOGD("can't open /dev/ttyACM0 \n");
			return NULL;
	}
	
	FD_ZERO( &descriptors);

	FD_SET( fd, &descriptors );
	
	rbuf=(char*)malloc(len*sizeof(char)+1);
	memset(rbuf,0,len*sizeof(char)+1);
	tcgetattr(fd,&option);
	cfmakeraw(&option);

	cfsetispeed(&option,BAUDRATE); /*38400bps*/
	cfsetospeed(&option,BAUDRATE);

	tcsetattr(fd,TCSANOW,&option);
	// rbuf=hd;
	LOGD("ready for receiving data...\n");
	// retv=read(fd,rbuf,len); 
	retv=select( fd + 1, &descriptors, NULL, NULL, &time_to_wait);
	
	if ( retv < 0 ) {
		/* Error */
		LOGD("select error \n");
		return NULL;
	}

	else if ( ! retv ) {
		/* Timeout */
		LOGD("Time Out \n");
		return NULL;
	}

	else if ( FD_ISSET ( fd, &descriptors ) ) {
		/* Process the inotify events */
			LOGD("****************have read data************************\n");
		
			retv=read(fd,rbuf,len);
			if(retv<0){
				LOGD("read error \n");
				close(fd);
				return NULL;
			}
		
		}
		LOGD("The data received is:\n");
	// LOGD("%x \n",rbuf);
		for(i=0;i<len;i++)
			{
				LOGD("%x \t",rbuf[i]);
			}
		LOGD("\n");
		close(fd);
		return rbuf;

		
}



int send_code(char *data, char order, short data_size, int buf_size)
{
	LOGD("send func \n");
	int i;
	char* buf, *rece_buf;
	char value;
	// value=code;
	LOGD("data_size =%x  buf_size= %x ",data_size, buf_size);
	buf = joint_buf(frame_num, order, data_size, data, buf_size);
	// for(i=0;i<buf_size;i++)
		// {
			// LOGD("%x \t",*(buf+i));
		// }
	// LOGD("\n");
	while(1){
		int ret = serial_send_data(buf, buf_size);
		if(ret<0){
				LOGD("send error \n");
				return -1;
			}
		usleep(500);
	
		rece_buf=serial_rece_data(ACK_SIZE);
		if(rece_buf==NULL){
			LOGD("select read \n");
			continue ;
		}
		LOGD("zzzzz rece buf = %p \n",rece_buf);
		ret = analyze_buf(rece_buf, ACK_SIZE);
		if(ret==-1){
			LOGD("zz rece NACK \n");
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
	int i=0;
	char delims[] = "#";
	char *split_buf[3] ;
	char *result = NULL;
	result = strtok( buf, delims );
	while( result != NULL ) {
       // LOGD( "result is \"%s\"\n", result );
	   split_buf[i++]=result;
	   
	   code=atoi(result);
	   // LOGD( "code is %d \n",code);
       result = strtok( NULL, delims );
   } 
   
   for(i=0;i<3;i++){
		LOGD( "split_buf [%d] %s\n", i, split_buf[i]);
   }
   
   result = strcat(split_buf[1],split_buf[2]);
   LOGD( "join  result = %s\n", result);
   
   *len = strlen(result);
   
   return result;
} 



int update_code_table(char* path)
{

	char config_buf[MAX_LINE]; /* 缓冲区 */
	char *data_buf;
	char start_buf[4]={0x00,0x00,0x00,0x00};
	char end_buf[4]={0xff,0xff,0xff,0xff};
	FILE *fp;
	int len, length, ret;

	ret = send_code( start_buf ,UPDATE_VALUE_CMD, sizeof(start_buf), BUF_OTHER_ITMES_SIZE+sizeof(start_buf));
	if(ret<0){
		LOGD("send code err \n");
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
	LOGD( "data buf result = %s\n", data_buf);
	
	if(strstr(data_buf, STB_VERSION)!=NULL){
		LOGD("stb version info \n");
		ret = send_code( data_buf ,UPDATE_VALUE_CMD, length, BUF_OTHER_ITMES_SIZE+length); //buf size 14
		if(ret<0){
			LOGD("send code err \n");
		}
	}else{
		LOGD("stb data info \n");
		ret = send_code( data_buf ,UPDATE_VALUE_CMD, length, BUF_OTHER_ITMES_SIZE+length);
		if(ret<0){
			LOGD("send code err \n");
		}
	
	}

	LOGD("%s %d\n", config_buf, len - 1); /* 使用LOGD函数输出 */
	}

	ret = send_code( end_buf ,UPDATE_VALUE_CMD, sizeof(end_buf), BUF_OTHER_ITMES_SIZE+sizeof(end_buf));
	if(ret<0){
		LOGD("send code err \n");
		return -1;
	}
	return 1;
}


int send_code_value(char code)
{
	LOGD("sen code value \n");
	int ret = send_code(&code, SEND_VALUE_CMD, sizeof(char), BUF_OTHER_ITMES_SIZE+sizeof(char));
	if(ret<0){
		LOGD("send code err \n");
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
	int buf_size = BUF_OTHER_ITMES_SIZE+data_size;
	int i;
	LOGD("send config info\n");
	LOGD("data_size =%x  buf_size= %x ",data_size, buf_size);
	buf = joint_buf(frame_num, order, data_size, &data, buf_size);
	for(i=0;i<buf_size;i++)
		{
			LOGD("%x \t",*(buf+i));
		}
	LOGD("\n");
	while(1){
		int ret = serial_send_data(buf, buf_size);
		if(ret<0){
				LOGD("send error \n");
				return -1;
			}
		usleep(500);
	
		rece_buf=serial_rece_data(CONFIG_ACK_SZIE);
		if(rece_buf==NULL){
			LOGD("select read \n");
			continue ;
		}
		
		LOGD("zzzzz rece buf = %p \n",rece_buf);
		ret = analyze_buf(rece_buf, CONFIG_ACK_SZIE);
		if(ret==0){
			LOGD("no config info \n");
			return 0;
		}else if(ret==2){
			LOGD("config info \n");
			return 1;
			
		}else if(ret==-1){
		frame_num++;
		continue;
		}
		

	      }
	return -1;
}



void main()
{
	LOGD("aaaa\n");
	int ret ;
	// char code=0x88;
	// ret = send_code_value(code);
	
	// if(ret==1){
		// LOGD("send code success \n");
	// }
	
	ret = update_code_table("/data/test.txt");
	if(ret==1){
		LOGD("send code success \n");
	}
	
	// ret = send_config_info();
	// if(ret==1){
		// LOGD("rece config success \n");
	// }
	// char* test_buf="aaaaaa";
	// char* buf;
	// char* buffer;
	// char a = 0xb3;
	// buf = malloc(3*sizeof(char));
	// LOGD("buf = %x %x %x\n",buf[0],buf[1],buf[2]);
	// memset(buf, 0, 3*sizeof(char));
	// LOGD("buf = %x %x %x \n",buf[0],buf[1],buf[2]);
	// memcpy(buf,&a,sizeof(char));
	// memcpy(buf+1,&a,sizeof(char));
	// memcpy(buf+2,&a,sizeof(char));
	// buffer = malloc(sizeof(char));
	// memset(buffer, 0 , 6);
	// buf = joint_buf(0,0x01,6,test_buf);
	// analyze_buf(buf,6);
	// memcpy(buffer, buf+8, 6);
	
	// LOGD("zzz== %s \n",buffer);
}
