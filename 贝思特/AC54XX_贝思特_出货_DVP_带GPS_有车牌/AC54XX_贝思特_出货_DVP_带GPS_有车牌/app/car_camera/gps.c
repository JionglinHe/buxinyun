#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
//#include "video_dec.h"
#include "asm/clock.h"
#include "os/os_api.h"
#include "asm/uart.h"
#include "generic/gpio.h"
#include "spinlock.h"
#include "init.h"
#include "action.h"
#include "style.h"
#include "gps.h"


struct uartx_debug_reg {
    volatile u32 *ut_con;
    volatile u8  *ut_buf;
    volatile u32 *ut_txdma_addr;
    volatile u32 *ut_txdma_cnt;
};
GPS_INFO  gps_info;
static struct uartx_debug_reg debug = {NULL, NULL};

static DEFINE_SPINLOCK(lock);
static u8 uart_dma_buf[1024 * 2];
static u8 tx_busy = 1;
static u8 dma_enable = 0;
static u16 data_len = 0;
static u16 w_pos = 0;

volatile u8 data_busy = 0;
int soildog_msg_count = 0;
static int uart_timer = 0;
volatile u8 updata = 0;         //�����ݸ���
volatile u8 updata_get = 0;         //�����ݸ���
volatile u8 last_updata = 0;    //�����ݸ���

/****************************************************/
u8 GPS_DataBuf[GPS_DATA_SIZE];
u8 Decode_DataBuf[GPS_DATA_SIZE]; //after verify
FIFO_BUF_STRUCT GpsSave; //GPS Data
GPS_TIME CurTime;

/****************************************************/

___interrupt
void uartx_irq_handler()
{
    irq_handler_enter(UART2_INT);

    *debug.ut_con |= BIT(13);

    spin_lock(&lock);
    tx_busy = 0;
    spin_unlock(&lock);
  //  uart_dma_tx();

    irq_handler_exit(UART2_INT);
}

int communication_putByte(char a)
{

    UT2_BUF = a;
    while((UT2_CON & BIT(15)) == 0);

    UT2_CON |= BIT(13);
    return a;
}

#if 0
void get_g_info(GPS_INFO * g_info)
{
    while(data_busy)
    {
        asm("nop");
    }
    memcpy(g_info, &gps_info, sizeof(gps_info));
}
#else
void get_g_info(GPS_INFO * g_info)
{
   DecodeUart(g_info);
   //memcpy(g_info, &gps_info, sizeof(gps_info));
}

#endif
char  HexToASCII(unsigned char  data_hex)
{
 char  ASCII_Data;

 ASCII_Data=data_hex & 0x0F;
 if(ASCII_Data<10)
  ASCII_Data=ASCII_Data+0x30; //��0--9��
 else
  ASCII_Data=ASCII_Data+0x37;       //��A--F��
 return ASCII_Data;
}

#if 0
___interrupt
void uarrx_irq_handler()
{
    char RX_BUF = 0;
     char temp;

    if(UT2_CON & BIT(15)) //tpend
    {
        //�����������
        UT2_CON |= BIT(13);
    }
    else if(UT2_CON & BIT(14)) //rpend
    {
        RX_BUF = UT2_BUF;//get one byte

        GPS_DataBuf[GpsSave.m_dwWrId++]=RX_BUF;
		if(GpsSave.m_dwWrId >= GPS_DATA_SIZE)
		{
			GpsSave.m_dwWrId = 0;
		}
        if(RX_BUF == '|' || RX_BUF == '~')
            putchar(RX_BUF);
        //�����������
        UT2_CON |=BIT(12) ;
        //DecodeUart(&gps_info);

    }
    else if(UT2_CON & BIT(11))  //otpend
    {
        //�������ݳ�ʱ
        UT2_CON |= BIT(10);
        UT2_CON |= BIT(7) ; //��UT0_HRXCNT
    }


    irq_handler_exit(UART2_INT);
}
#endif

void clear_gps()
{
    GpsSave.m_dwRdId = 0;
	GpsSave.m_dwWrId = 0;
    memset(GPS_DataBuf, 0, GPS_DATA_SIZE);
    memset(Decode_DataBuf, 0, GPS_DATA_SIZE);
    //UT2_CON &= ~BIT(3);//���ڽ����ж�����
    UT2_CON &= ~BIT(0);//����ģ��ʹ��
}

void start_gps()
{
    GpsSave.m_dwRdId = 0;
	GpsSave.m_dwWrId = 0;
    memset(GPS_DataBuf, 0, GPS_DATA_SIZE);
    memset(Decode_DataBuf, 0, GPS_DATA_SIZE);
    //UT2_CON |= BIT(3);//���ڽ����ж�����
    UT2_CON |= BIT(0);//����ģ��ʹ��
}

___interrupt
void uarrx_irq_handler()
{
    char RX_BUF = 0;
     char temp;
     u8 bRet;

    if(UT2_CON & BIT(15)) //tpend
    {
        //�����������
        UT2_CON |= BIT(13);
    }
    else if(UT2_CON & BIT(14)) //rpend
    {
        RX_BUF = UT2_BUF;//get one byte
        #if 1
        GPS_DataBuf[GpsSave.m_dwWrId++]=RX_BUF; 
		if(GpsSave.m_dwWrId >= GPS_DATA_SIZE)
		{
			GpsSave.m_dwWrId = 0;
		}
		//if(RX_BUF == '|')
		//	putchar('a');
		//putchar(RX_BUF);
        #endif
        #if 0
        if(RX_BUF == '|')
        {
            GpsSave.m_dwWrId = 0;
            memset(GPS_DataBuf, 0, sizeof(GPS_DataBuf));
        }
        //putchar(RX_BUF);
        GPS_DataBuf[GpsSave.m_dwWrId++]=RX_BUF;
        if(RX_BUF == '~')
        {
            bRet = DataIn_Decode(GPS_DataBuf,GpsSave.m_dwWrId,&temp);
			if(bRet==TRUE)
            {
                memcpy(Decode_DataBuf,GPS_DataBuf,GpsSave.m_dwWrId);
                Decode_Uart_Data(&gps_info);
            }
        }
        #endif
        //�����������
        UT2_CON |=BIT(12) ;
        

    }
    else if(UT2_CON & BIT(11))  //otpend
    {
        //�������ݳ�ʱ
        UT2_CON |= BIT(10);
        UT2_CON |= BIT(7) ; //��UT0_HRXCNT
    }

    irq_handler_exit(UART2_INT);
}

//��һ�㣬ֱ�ӽ���һ������
//len Ϊ�������ݰ��ĳ���
u8 CalCRC(u8 *ptr, u16 len)
{
	u8 crc;
	u16 i;
	u16 calLen;

	//�������� 5���ֽ�
	if(len < 5)
	{
		return FALSE;
	}

	crc = 0;
	calLen = len-2;
	for(i=1;i<calLen;i++)
	{
		crc += *(ptr+i);
	}
	crc ^= 0x8A;

	if(crc == *(ptr+len -2))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

u8 DataIn_Decode(u8 *pBuff, u16 nMaxCount,u16 *pLen)
{
	u8 ret;
	u16 tryLen;

	if((*pBuff) == '|')
	{
		tryLen = ((u16)*(pBuff+1))&0xFF;
		tryLen |= (((u16)*(pBuff+2))<<8)&0xFF00;
		tryLen = tryLen+2;

		if(tryLen <= nMaxCount)
		{
			if(*(pBuff+tryLen-1) == '~')
			{

				ret = CalCRC(pBuff, tryLen);
				if(ret)
				{
					*pLen = tryLen;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}




/* ʮ��������ת��Ϊʮ������ */  
u8 hexToDec(u8 source)  
{  
    u8 sum;
	sum = ((source>>4)*16) + (source&0x0f); 
    return sum;   
}   
u16 hex16ToDec16(u16 source)  
{  
    u16 temp;
	temp += (source>>12&0xf)*4096;
	//printf("%d\n", temp);
	temp += (source>>8&0xf)*256;
	//printf("%d\n", temp);
	temp += (source>>4&0xf)*16;
	//printf("%d\n", temp);
	temp += source&0xf;
	//printf("%d\n", temp);
    return temp;   
 
} 
u32 hex32ToDec32(u32 source)  
{  
    u32 temp;
	temp = 	0;
	temp += (source>>20&0xf)*1048576;
	//printf("%d\n", temp);
	temp += (source>>16&0xf)*65536;
	//printf("%d\n", temp);
	temp += (source>>12&0xf)*4096;
	//printf("%d\n", temp);
	temp += (source>>8&0xf)*256;
	//printf("%d\n", temp);
	temp += (source>>4&0xf)*16;
	//printf("%d\n", temp);
	temp += source&0xf;
	//printf("%d\n", temp);
    return temp;   
} 
void GPS_Command_Analysis(GPS_INFO *g_info)
{
	u16 i;
	u8 alarmType;
	u8 limitSpeed;
	u16 distance;
	u8 curSpeed;
	u8 curAngle;
	u32 curLattitude;
	u32 curLongitude;
    

	i = DATA_OFFSET;

    //putchar(Decode_DataBuf[i]);
	if(Decode_DataBuf[i]=='A') //GPS��λ����������ʹ��
	{
		//putchar('b');
		//��һ�μ�⵽Aʱ������ֱ������DVR��ʱ��
		g_info->sate_en = 1;  //�ж�λ
		i++;
		if(Decode_DataBuf[i]>0)
		{
			//���ֵ����ۣ�ֱ����Ҫ��ʾICON������ǵ�һ����0-->X,
			//������Ҫ������ǰ��XXX����XXX
			alarmType = Decode_DataBuf[i];   //��ʾ����
            g_info->alarmType = hexToDec(alarmType);
			i++;
			limitSpeed = Decode_DataBuf[i];   //����ֵ
			g_info->limitSpeed = hexToDec(limitSpeed);
			i++;
		}
		else
		{
		    i += 2;
		    g_info->alarmType = 0;
            g_info->limitSpeed = 0;
			//�������X-->0,˵����������ʧ����Ҫ���ICON��ʾ
		}

        //printf("alarmType = %d\n", g_info->alarmType);
        //printf("%d\n", g_info->limitSpeed);

        //�������룬һ�㶼�ǵݼ��ģ��벻Ҫ���ݵ�������Ϊ0���ж�
		//�Ƿ�ͨ�������ۣ���Ҫ����alarmTypeΪ0���ж�
		memcpy((u8*)&distance,&Decode_DataBuf[i],2);
        g_info->distance = hex16ToDec16(distance);
        //printf("%d\n", g_info->distance);
		i += 2;
		//��ǰ���٣����ᳬ��220
		curSpeed = Decode_DataBuf[i];
        //printf("curSpeed = %x\n", curSpeed);
        g_info->curSpeed = hexToDec(curSpeed);
        //printf("%d\n", g_info->curSpeed);
		i++;
        

		//��ǰ���ԽǶȣ�����˳ʱ�뷽������Ϊ1������Ϊ2...
		curAngle = Decode_DataBuf[i];
        g_info->curAngle = hexToDec(curAngle);
		i++;
        //printf("%d\n", g_info->curAngle);
		//��ǰγ�ȣ��Ŵ���10000��������2212345��ʵ������22.12345��
		memcpy((u8*)&curLattitude,&Decode_DataBuf[i],3);
        //printf("c=0x%x=0x%x=0x%x\n", Decode_DataBuf[i],Decode_DataBuf[i+1],Decode_DataBuf[i+3]);
		i += 3;
        g_info->curLattitude = hex32ToDec32(curLattitude);
        
        //printf("curLattitude = %d\n", curLattitude);
		//��ǰ���ȣ��Ŵ���10000��������11934567��ʵ������119.34567��
		memcpy((u8*)&curLongitude,&Decode_DataBuf[i],3);
        //printf("l=0x%x=0x%x=0x%x\n", Decode_DataBuf[i],Decode_DataBuf[i+1],Decode_DataBuf[i+3]);
		i += 3;
        g_info->curLongitude = hex32ToDec32(curLongitude);
        //printf("curLongitude = %d\n", curLongitude);
		//����Ĳ���˵��
		//��һ���ر�ע�⣬�����ʱ���Ǳ�׼��UTCʱ��
		//��Ҫת���ɱ���ʱ�䣬���Ǿ��Բ�����ʹ��UTC+8��
		//��Сʱ���м򵥵ļ�8������Ҫʹ��time.h�����
		//����������86400(8*60*60)���ٻ����Ϊ��׼��ʱ��
		//���磬��ǰUTCʱ����2015.12.31 23:58:59����ȷ��
		//����ʱ��Ӧ����2016.01.01 07:58:59
		i += 2; //��ȷ��λ

		g_info->time.year = hexToDec(Decode_DataBuf[i]);
        
        i++;
        g_info->time.month= hexToDec(Decode_DataBuf[i]);
		i++;
        g_info->time.day= hexToDec(Decode_DataBuf[i]);
        i++;
        g_info->time.hour= hexToDec(Decode_DataBuf[i]);
        i++;
        g_info->time.min= hexToDec(Decode_DataBuf[i]);
        i++;
        g_info->time.sec= hexToDec(Decode_DataBuf[i]);

        i += 2;
        g_info->satellite = hexToDec(Decode_DataBuf[i]);  //ʹ����������
	}
    else
    {
       // putchar('s');
        g_info->sate_en = 0;  //û�ж�λ
    }
}
void Version_Command_Analysis(void)
{
    u16 i;
    u16 ver, fir;
    i = 4;
    //memcpy((u8*)&ver,&Decode_DataBuf[i],2);
    //i++;
    //fir = Decode_DataBuf[i];

    //printf("ver = %d  fir = %d\n", ver, fir);
}
void Error_Command_Analysis(GPS_INFO *g_info)
{
    u16 i;
    g_info->err = Decode_DataBuf[4];    
}

void Update_Command_Analysis(void)
{
}

void Decode_Uart_Data(GPS_INFO *g_info)
{
	//printf("==%x==\n",Decode_DataBuf[3]);
	switch(Decode_DataBuf[3])
	{
       	case 0x81:
    		// GPS Ԥ����Ϣ
    		GPS_Command_Analysis(g_info);
		//printf("===ok===\n");
    		break;

    	case 0x82:
    		//�汾������Ϣ
    		Version_Command_Analysis();
    		break;

    	case 0x83:
    		//�쳣��ʾ��Ϣ
    		Error_Command_Analysis(g_info);
    		break;

    	case 0x84:
    		//��������������ʾ��Ϣ
    		Update_Command_Analysis();
    		break;

    	default:
    		break;
	}
}
void DecodeUart(GPS_INFO *g_info)
{
	u8 bRet;
	u16 curHandLen;
	u16 findLen;
	u16 i;
	static u8 cnt;

    //putchar('b');
    cnt++;
	if(GpsSave.m_dwRdId < GpsSave.m_dwWrId)
	{
	    //putchar('a');
	    
		i = 0;
		curHandLen = GpsSave.m_dwWrId - GpsSave.m_dwRdId; //current left length
		if(curHandLen > (GPS_DATA_SIZE>>1)) //delay long time or make mistake,discard
		{
			curHandLen = 0; //error
			GpsSave.m_dwRdId = 0;
			GpsSave.m_dwWrId = 0;
		}
		if(cnt > 10)  //3�붼û�н������
		{
			printf("decode err\n");
			curHandLen = 0; //error
			GpsSave.m_dwRdId = 0;
			GpsSave.m_dwWrId = 0;
			cnt = 0;
		}
		
		while(curHandLen)
		{
			bRet = DataIn_Decode(&GPS_DataBuf[i+GpsSave.m_dwRdId],curHandLen,&findLen);
			if(bRet==TRUE)
			{
			    //putchar('b');
			    //data_busy = 1;
				//copy data
				memcpy(Decode_DataBuf,&GPS_DataBuf[i+GpsSave.m_dwRdId],findLen);
				Decode_Uart_Data(g_info);
				i += findLen;  //find data,need decode
				GpsSave.m_dwRdId += findLen;
				curHandLen -= findLen;
				cnt = 0;
                //data_busy = 0;
			}
			else
			{
				i++;
				curHandLen--;
			}
		}
		//simple handle
		if(GpsSave.m_dwRdId == GpsSave.m_dwWrId)
		{
			GpsSave.m_dwRdId = 0;
			GpsSave.m_dwWrId = 0;
		}
	}
}


int muart_init(const struct uart_platform_data *data)
{
    int uart_clk = clk_get("uart");

    gpio_direction_output(data->tx_pin, 0);

    switch (data->irq) {
    case UART2_INT:
        IOMC3 |= BIT(11) ;//ռ����Ӧ��IO
        if (data->rx_pin == IO_PORTC_04) {

        } else if (data->tx_pin == IO_PORTH_10) {
            IOMC3 |= BIT(12);
	    	IOMC3 &= ~(BIT(13)) ;
            //gpio_direction_output(IO_PORTH_10,0);
            gpio_direction_input(IO_PORTH_09);
        } else if (data->tx_pin == IO_PORTD_00) {
            IOMC3 |= BIT(13);

        } else if (data->tx_pin == IO_PORTH_00) {
            IOMC3 |= (BIT(12) | BIT(13));

        }
        UT2_BAUD = (uart_clk / data->baudrate) / 4 - 1;
        UT2_CON =  BIT(0);

        UT2_CON &= ~BIT(1);          //8 bits
        UT2_CON &= ~BIT(6);          //��ͨģʽ������DMAģʽ
        UT2_CON |= BIT(13)|BIT(12)|BIT(10); //��TXPND,RXPND,OTPND
        UT2_CON |= BIT(3);//���ڽ����ж�����
        UT2_CON |= BIT(0);//����ģ��ʹ��
        break;

    default:
        return -EINVAL;
    }

    request_irq(data->irq, 2, uarrx_irq_handler, 0);

    return 0;
}



void mputbyte(char a)
{
    if (debug.ut_con == NULL) {
        return;
    }

    spin_lock(&lock);

    if (dma_enable) {

        if (a == '\n') {
            uart_dma_buf[w_pos++] = '\r';
            if (w_pos == sizeof(uart_dma_buf)) {
                w_pos = 0;
            }
            data_len++;
            /*cbuf_write(&uart_cbuf, &b, 1);*/
        }
        uart_dma_buf[w_pos++] = a;
        if (w_pos == sizeof(uart_dma_buf)) {
            w_pos = 0;
        }
        data_len++;

        spin_unlock(&lock);


    } else {
        spin_unlock(&lock);

        if (a == '\n') {
            if (tx_busy) {
                while ((*debug.ut_con & BIT(15)) == 0);
            }
            *debug.ut_buf = '\r';
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            tx_busy = 1;
        }
        if (tx_busy) {
            while ((*debug.ut_con & BIT(15)) == 0);
        }
        *debug.ut_buf = a;
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        tx_busy = 1;

    }
}

int mgetbyte(char *c)
{
    if (debug.ut_con == NULL) {
        return 0;
    }
    if ((*debug.ut_con & BIT(14))) {
        *c = *debug.ut_buf;
        *debug.ut_con |= BIT(12);
        return 1;
    }

    return 0;
}


extern void video_rec_post_msg(const char *msg, ...);
extern void video_home_post_msg(const char *msg, ...);


static void get_gps_info_hander(void *arg)
{
	
	static u8 state = 0, cnt = 0;
    if(get_dog_status())
    { 
        cnt++;
        if(cnt > 10)
        {
            cnt = 10;
        }
    }
    else
    {
        if(cnt > 0)
            cnt--;       
    }
    if(cnt == 10 && state == 0)
    {
        state = 1;
        //printf("showdog\n");
        video_rec_post_msg("showdog"); 
    }
    if(cnt == 0 && state == 1)
    {
        state = 0;
        cnt = 0;
        //printf("hidedog\n");
        video_rec_post_msg("hidedog");
        
    }
    if(state == 1)
    {
    	
        if(ui_get_current_window_id() == ID_WINDOW_VIDEO_REC)
        {
        	DecodeUart(&gps_info);
            video_rec_post_msg("doginfo");
        }
        else if(ui_get_current_window_id() == ID_WINDOW_MAIN_PAGE)
        {
        	DecodeUart(&gps_info);
            video_home_post_msg("doginfo");  
        }
    }
}

static int gps_decode(void)
{
    dog_det_init();
    sys_timer_add(NULL, get_gps_info_hander, 300);
    return 0;
}
late_initcall(gps_decode);

#if 0
static void uart_dma_mode()
{
    if (debug.ut_txdma_addr) {
        dma_enable = 1;
        tx_busy = 0;
        *debug.ut_con |= BIT(13);
        *debug.ut_con |= BIT(2);
    }
}
late_initcall(uart_dma_mode);
#endif
