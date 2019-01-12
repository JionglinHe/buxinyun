#ifndef     _GPS_H_
#define     _GPS_H_

typedef struct{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
}DATE_TIME;

typedef struct{
    int  latitude; //����
    int  longitude; //γ��
    int latitude_Degree;    //��
    int        latitude_Cent;   //��
    int     latitude_Second; //��
    int longitude_Degree;    //��
    int        longitude_Cent;  //��
    int     longitude_Second; //��
    int      speed; //�ٶ�
    int      direction; //����
    int      height; //���θ߶�
    int satellite;
    u8     NS;
    u8     EW;
    DATE_TIME D;
    char latitude_buf[10];
    char longitude_buf[10];
}GPS_INFO_1;



/////////////////////////////////////

#define 	GPS_DATA_SIZE			(512)
#define	DATA_OFFSET			(4)

typedef struct
{
	u16 m_dwWrId;
	u16 m_dwRdId;
}FIFO_BUF_STRUCT;
typedef struct
{
	u8 m_bRMCYear;					//
	u8 m_btRMCMonth;				//
	u8 m_btRMCDay;					//
	u8 m_btRMCUTCHour;			//�����UTCʱ��
	u8 m_btRMCMinute;				//
	u8 m_btRMCSecond;				//
}GPS_TIME;


typedef struct{
    u8 curSpeed;   //�ٶ�
    u32 curLattitude; //γ��
	u32 curLongitude; //����
    u8 curAngle;     //����   
    u8 satellite;  //ʹ�ú�������
    u8 alarmType;  //��ʾ����
    u8 limitSpeed;  //����ֵ  
    u16 distance;    //����ֵ
    u8 sate_en;
    u8 err;
    struct utime time;
}GPS_INFO;


extern GPS_INFO  gps_info;

void Decode_Uart_Data(GPS_INFO *g_info);
void DecodeUart(GPS_INFO *g_info);
void get_g_info(GPS_INFO * g_info);
u8 DataIn_Decode(u8 *pBuff, u16 nMaxCount,u16 *pLen);
void clear_gps();
void start_gps();

#endif
