#ifndef SYS_TIME_H
#define SYS_TIME_H

#include "typedef.h"


struct sys_time {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

typedef struct DATE_TIME {
    u16 year1;
    u8 month1;
    u8 day1;
    u8 hour1;
    u8 minute1;
    u8 second1;
};
 typedef  struct data{
    double  latitude; //����
    double  longitude; //γ��
    int  latitude_Degree;    //��
    int         latitude_Cent;   //��
    int     latitude_Second; //��
    int  longitude_Degree;    //��
    int        longitude_Cent;  //��
    int     longitude_Second; //��
    float     speed; //�ٶ�
    float     direction; //����
    float     height; //���θ߶�
    int satellite;
   u8     NS;
    u8     EW;
	    u16 year1;
    u8 month1;
    u8 day1;
    u8 hour1;
    u8 minute1;
    u8 second1;
    struct DATE_TIME D;
}GPS_INFO;













#endif
