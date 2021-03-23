/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.76
        Device            :  PIC16F1503
        Driver Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
 */
/*
 *@author Samon Sun 
 *@telphone +12535339798(message me call number before contact)
 *@time 03，23，2021
 *@sepc: fireware for AC hammer MH5,KMH5 etc 220v  
*/
#include "mcc_generated_files/mcc.h"
//#include <xc.h>
//#include "digitalfilter.h"
 
//#define DEBUG 10085
#define NTC_MAX 500
#define TEST_MAX 800
#define TEST_MIN 600
#define OFF 0
#define ON 1
#define TRUE 0
#define FALSE 1
#define TRIGER_TIME 25  
#define NTC_OVER_HEART_THREHOLD 200   //  
#define MACRO  1
#define SAMPLERATING  250  
uint16_t v_adj=0, v_ntc, v_test=1024;
uint8_t trigger = 0;
uint16_t adjtime = 0;
 
uint16_t v_adj_filter[4];
uint16_t i,tmr0cnt=0;
uint16_t cnt_adc=99;
uint16_t v_adj_slow=7000;
uint8_t count;
uint8_t count_sta=0;
uint8_t ra2sta;
uint8_t overheatcnt=0;
uint8_t carbonbrushoutcnt=0;
static uint8_t switch_count=0;
static uint16_t t2_last_tick=0;
static uint16_t t1_last_tick=0;
static uint16_t t0_tick=0;
uint8_t start_switchpushflag=FALSE; 
static uint8_t switchflag=OFF;
static uint8_t init_flag=1;
static uint16_t cmp1cnt=0; 
static uint8_t flag_rpmcapture=0;
static uint16_t cnt_rpmcapture=0;
static uint16_t cnt=0;
static uint8_t flag_L1=1;
static uint8_t flag_L2=0;
static uint8_t flag_blink=0;
void status_machine_func(void);   
static uint16_t v_adj_convert(uint16_t button_adj);
uint16_t rpm_celiang(void);   
void tmr1gate_func_init ( void );
void led_blink(void) ;     
void tmr0reloadcnt(void);   
void tmr1cnt_func_init(void);   
 
static uint16_t reloading_val  =6800;  
 

typedef enum tmr1fuctionswitch
{
  gate_trg      = 1,
  counter       = 2,
}
Tmr1fuctionswitch_t;
Tmr1fuctionswitch_t Tmr1fuctionswitch;

typedef enum tmr0fuctionswitch
{
  trg          = 1,
  dlycnt       = 2,
  dlycnt_processing=3,
}
Tmr0fuctionswitch_t;
Tmr0fuctionswitch_t Tmr0fuctionswitch=dlycnt_processing;

typedef enum Motor_RpmType
{
  Lower_than_400hz               = 0,
  Between_400hz_and_1000hz       = 1,
  Between_1000hz_and_1500hz      = 2,
  More_than_1500hz               = 3,
  More_than_1800hz               = 5,  
}
Motor_RpmType_t;
Motor_RpmType_t Motor_Rpm_Mode;

typedef enum Motor_run_status_machine
{
  Init_status            = 0,
  low_speed_status       = 1,
  mid_speed_status       = 2,
  hi_speed_status       = 3,
  mid_speed_adj         = 4,
  hi_speed_adj          = 5,
  motor_block_status    = 6,
  motor_fault_status    = 7,
  motor_stop_status    = 8,
  motor_capture_status    = 10,
  motor_startup    = 10,
  speedauto_status       = 12,
}
Motor_run_status_t;
Motor_run_status_t Motor_run_status=Init_status;

 

void INT_T2() 
{    
 #ifndef DEBUG
    t0_tick++;
    if(t0_tick>1)
    {
        led_blink();
        t0_tick=0;
    }
 #endif
 

    if(init_flag ==0 && start_switchpushflag==FALSE && trigger ==0 )
    {
        tmr1gate_func_init ();
        cmp1cnt=rpm_celiang();
    }
    if(cmp1cnt !=0)
    { 
                      cnt_rpmcapture=cmp1cnt ; 
                       cmp1cnt=0;   
                                #ifdef DEBUG
                               //LED2_LAT=1;
                               // LED1_LAT=0;
                                #endif                             
    }    
    else
    {
                                #ifdef DEBUG
                               //LED2_LAT=0;
                               // LED1_LAT=1;
                                #endif   
    } 
cnt_adc++;
if(cnt_adc>SAMPLERATING)//
{
    cnt_adc=0;
    if(v_adj==0)
    {
       v_adj = ADC_GetConversion(ADJ); //旋钮RC3
    }
    else 
    {
        //滤波函数
        for(uint8_t i=0;i<4;i++)
        {
            v_adj_filter[i]=ADC_GetConversion(ADJ);
        }
        if(v_adj_filter[0]-v_adj >50 || v_adj-v_adj_filter[0]  >50)
        {
            v_adj=v_adj_filter[0];
        }
    }

    v_ntc = ADC_GetConversion(NTC); //NTC RC0
//    v_fb0 = ADC_GetConversion(FB0);
  //  v_fb1 = ADC_GetConversion(FB1);
    v_test = ADC_GetConversion(TEST); 
}
/*
if(v_adj>950)
{
                                #ifdef DEBUG
                                LED2_LAT=0;
                                //LED1_LAT=0;
                                #endif
}
else
{
                                #ifdef DEBUG
                                LED2_LAT=1;
                                //LED1_LAT=1;
                                #endif
}
*/

    //这里简化了一下判断机制，RC1 高峰值如果大于4v，就亮灯，如果低值低于3v就灭灯，这里将来如果不对再改逻辑
 // if(init_flag ==0 && start_switchpushflag==FALSE && trigger==0 )
//if(trigger==0 )
if(init_flag ==0 && start_switchpushflag==FALSE )
{
    //这里判断RA2是否在以50hz频率翻转
    if (ra2sta == RA2_PORT)
     {
       if (count_sta < 250)
        {
            count_sta++;
        }
    } 
    else 
        {
                count_sta = 0;
                ra2sta = RA2_PORT;
        }
    //如果RA2没有50hz频率翻转， 缓期重置
        if (count_sta >= 40) 
        {//50HZ翻转，参数取决于进入函数时间，1ms进入t0终端的话就是 10,因为上
            v_adj_slow = 6800;     
            cmp1cnt=0;
            cnt_rpmcapture=0;
            Motor_run_status=Init_status;
//            cnt_power=0;
      //   LED1_LAT=0;
        } 
    else 
    {
     /************* 非恒速代码BOF*****/
  /*   
//reloading_val=6887- v_adj*5 ;
reloading_val=7168- v_adj*7 ;
//reloading_val=8192- v_adj*8 ;
         if (v_adj_slow > reloading_val) 
         {
                                #ifdef DEBUG
                               // LED2_LAT=0;
                               // LED1_LAT=1;
                                #endif

            if (count < MACRO)  
                count++;
            else 
            {
                v_adj_slow --;
                count = 0;
            }
         }
         else  v_adj_slow = reloading_val;
#ifdef DEBUG
         //if(cnt_rpmcapture>2700 && cnt_rpmcapture< 2800 )
         if(cnt_rpmcapture<2727)
         {
                              
                           
                                LED1_LAT=1;
                               // LED2_LAT=1;
                           
         }
         else
         {
                         
                                LED1_LAT=0;
                                
         }
#endif     
    }
    */
 /************* 非恒速代码EOF*****/
/************* 恒速代码BOF****/
    if(v_adj<=950)
    {
         // reloading_val=(uint16_t) (7135- v_adj*33/10) ;//a-950b=4000    a-100b=6800   745b=2800 b=3.3 a=6030
          reloading_val=(uint16_t) (5270- v_adj*4/3) ;//a-950b=4000    a-20=5250   950b=1270  b=4/3  a=5270   delay=5250=357*40.04 rpm=一档 
    }

    //300 大于5500
    else
    {
        reloading_val=4000;
                                #ifdef DEBUG
                               // LED2_LAT=0;
                               // LED1_LAT=0;
                                #endif

    }
 
         if (v_adj_slow > reloading_val) 
         {
            if (count < MACRO)  
                count++;
            else 
            {
                v_adj_slow=v_adj_slow-2;
                count = 0;
            }
         } 
        
        else 
        {
            if(v_adj>950 && cnt_rpmcapture>2200 && cnt_rpmcapture< 4000 )   //是否是自动挡判断
            {
              

                     if(cnt_rpmcapture >2727)  //
                     {
                        if (count < MACRO)  
                            count++;
                        else { 
                                 v_adj_slow --;

                                    count = 0;
                             }
                     }
                     
                     else 
                     {
                         v_adj_slow++;                
                     }

  
            }
            else
            {

                    v_adj_slow=reloading_val;

            }

        }
    }
    /************ 恒速代码EOF******/
#ifdef DEBUG
         //if(cnt_rpmcapture>2700 && cnt_rpmcapture< 2800 )
         if(cnt_rpmcapture<2727)
         {
                              
                           
                                LED1_LAT=1;
                               // LED2_LAT=1;
                           
         }
         else
         {
                         
                                LED1_LAT=0;
                                
         }
#endif   
}
// code 判断是否开机一直按键处理
    t2_last_tick++;
    if(t2_last_tick>1001)
    {
            t2_last_tick=0;
            
        if(v_adj>950)flag_L2=0;
        else 
                {
                    if(flag_L2)flag_L2=0;
                    else flag_L2=1;
                }
        if (v_test > TEST_MAX)
        {
            carbonbrushoutcnt=0;
 
                                #ifdef DEBUG
                               //LED2_LAT = 0;//LED2是绿灯，为0时候亮
                               // LED1_LAT=0;
                                #endif
            flag_L1=1;
 
        
        }

        else if (v_test < TEST_MIN)
        {
            // LED1_LAT = 0;
     
            carbonbrushoutcnt++;
            if(carbonbrushoutcnt>2)
            {
               // init_flag=2;
                    if(flag_L1)flag_L1=0;
                    else flag_L1=1;
            }
			else 
            {
               
                init_flag=0;

            }

        }  
        
        if(v_ntc < NTC_OVER_HEART_THREHOLD )
        {
        //  LED1_LAT = 0;
          flag_L1=0;
          overheatcnt++;
          if(overheatcnt>=3)init_flag=2;
          else 
          {
              init_flag=0;
          }
        // Motor_run_status = motor_fault_status;
        }
        if(v_ntc > NTC_OVER_HEART_THREHOLD)
        {
            init_flag=0;
           // flag_L1=1;
        } 
    }

    
    static uint8_t last_switch_count=0;


    if (init_flag==2)
    {
    t1_last_tick++;
    if(t1_last_tick>1001)
    {
            t1_last_tick=0;
                if(v_adj>950)flag_L2=0;
        else 
                {
                    if(flag_L2)flag_L2=0;
                    else flag_L2=1;
                }
    }
    
        if(last_switch_count !=switch_count)
           {
                if(t2_last_tick>1000)
 
                {
                        t2_last_tick=0;
                        start_switchpushflag=TRUE;
                        switch_count=0;
                      //  LED1_LAT = 0;
                      flag_L1=0;                   
                        last_switch_count =switch_count;

                }

            }
        else 
        {
         //   switch_count=1;
                        cnt++;
                        if(cnt>500)
                        {
                                    init_flag=0;
                                    start_switchpushflag=FALSE;
                                   // LED1_LAT=1;
                                   flag_L1=1;

                        }
                        else 
                        {
                            // LED1_LAT = 0;//松开开机键要500ms以上，否则红灯亮
                            flag_L1=0;
                        }
              
        }
   }

}

void INT_RA2()
{
 

    count_sta=0;
    cnt=0;

      if(PIR1bits.TMR1IF && trigger == 0 )
        {
           PIR1bits.TMR1GIF=0; 
           PIR1bits.TMR1IF=0; 
            TMR1_WriteTimer(0x00);
         //  T1GCONbits.T1GGO_nDONE=1; 
        } 
  //  status_machine_func();
   if(init_flag==1)//开机前初始化时候init_flag==1
   {
        switch_count++;//switch_count从0开始+1
        if(switch_count>3)//当switch_count加3次后经过约60ms时间，如果按住开机键则进入此循环
        {
  
                switch_count++;
                start_switchpushflag=TRUE;
                if (switch_count>50)
                {
                    switch_count=3;
                }

        } 

   } 
     if (start_switchpushflag==TRUE)//等待到进入wihle 1循环时候把init_flag设置为2
   {
       init_flag=2;
       switch_count=0;
       //init_flag==2 证明进入了开机一直按开关不松动步骤
   }
   if(init_flag==2)
   {
       trigger = 0;
       switch_count++;

   }
   else if(trigger==0 && init_flag ==0 && start_switchpushflag==FALSE )
   //else if(trigger==0 )
     {
         trigger = 1;
        T1GCON = 0x42;
        T1CON = 0x25;   
        TMR1_WriteTimer(0xffff-v_adj_slow); 
 
         TMR1_StartTimer();
         PIE1bits.TMR1IE=1;
                                #ifdef DEBUG
                              LED2_LAT=0;
                               // LED1_LAT=0;
                                #endif 
     }

 
}
 

/*
                         Main application
 */
void main(void) {
    // initialize the device
    SYSTEM_Initialize();
 
   TMR0_SetInterruptHandler(tmr0reloadcnt); 
   TMR1_SetInterruptHandler(tmr1cnt_func_init); 
    TMR2_SetInterruptHandler(INT_T2);
    IOCAF2_SetInterruptHandler(INT_RA2);
 
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    MOTOR_LAT = 0;
    Tmr0fuctionswitch=dlycnt_processing;
    __delay_ms(200);
    init_flag=0; 
    while (1) {
 
        if (trigger == 1 && start_switchpushflag==FALSE && init_flag ==0) 
        {

 

            
         //   if(v_adj_slow>7500)v_adj_slow=7500;
  if(v_adj_slow<1000)v_adj_slow=1000;
 


            
        }
        // Add your application code
    }
}
 
void tmr1gate_func_init ( void )
{
 
    T1GCON = 0xF2;
 
    T1CON = 0x05; //including TMR1_StartTimer();  
 
    
        if(PIR1bits.TMR1IF )
        {
           PIR1bits.TMR1GIF=0; 
           PIR1bits.TMR1IF=0; 
           TMR1_WriteTimer(0x00);
        } 
}

void tmr1cnt_func_init ( void )
{            
 
            PIR1bits.TMR1IF=0;
            PIE1bits.TMR1IE=0;
            INTCONbits.TMR0IE=1; 
                                #ifdef DEBUG
                               //LED2_LAT=1;
                                //LED1_LAT=0;
                                #endif         
}
//void tmr0reloadcnt( uint16_t reload, Tmr0fuctionswitch_t delayortrigger )
void tmr0reloadcnt()
 
{

    if(Tmr0fuctionswitch==dlycnt_processing && tmr0cnt==0 )
    {
                
                    tmr0cnt=12;
                    Tmr0fuctionswitch=trg;
                  //  INTCONbits.TMR0IF = 0;
                                #ifdef DEBUG
                            //   LED2_LAT=0;
                            //    LED1_LAT=0;
                                #endif             
    }
    if(Tmr0fuctionswitch==trg)
    {
        if(tmr0cnt%2)
        {
                MOTOR_LAT=1;
        }
        else
        {
                MOTOR_LAT=0;
        }
        
      //  MOTOR_LAT=1-tmr0cnt%2;
        TMR0 = 0xce;
     // timer0ReloadVal=0xce;
        if(tmr0cnt>0)
        {          
            tmr0cnt--;
          //  INTCONbits.TMR0IF = 0;
        }
        else 
        {
            Tmr0fuctionswitch=dlycnt_processing;   
            tmr0cnt=0;
            TMR0= 0xff;
            MOTOR_LAT=0;
           // INTCONbits.TMR0IF=0;
            INTCONbits.TMR0IE=0;
            //PIE1bits.TMR2IE=1;
            trigger=0;
            TMR1_WriteTimer(0x00);
        }

    }
}

uint16_t rpm_celiang(void)
{
    uint16_t a=0;
        if(TMR1_ReadTimer()>50000)
        {
            TMR1_Reload();
        }
 
        if(!T1GCONbits.T1GGO_nDONE  )
        {
            
                        __delay_us(1);
                        TMR1_StopTimer();  

                        a=TMR1_ReadTimer();
                     //   __delay_us(1);
                        TMR1_WriteTimer(0x00);
                        __delay_us(1);
                if(a<2400 || a>4000 )  
                {           
                    a=0;  
                }  

                PIR1bits.TMR1GIF=0;
                __delay_us(1);
                T1GCONbits.T1GGO_nDONE=1; 
                TMR1_StartTimer();

    
                return a ;
               
        }

        else 
        
        {
            return 0 ;
        }

}


void led_blink(void)
{

if(flag_L1==0 && flag_L2==0)
{
            if(flag_blink)
            {
                LED1_LAT=1;
                LED2_LAT=0;
                flag_blink=0;
            }
            else
            {
                LED1_LAT=0;
                LED2_LAT=1;
                flag_blink=1;
            } 
}
else
{
        if(flag_L1==0)
        {
            if(flag_blink)
            {
                LED1_LAT=1;
                flag_blink=0;
            }
            else
            {
                LED1_LAT=0;
                flag_blink=1;
            }
            LED2_LAT=1;
        }
        if(flag_L2==0)
        {
            if(flag_blink)
            {
                LED2_LAT=1;
                flag_blink=0;
            }
            else
            {
                LED2_LAT=0;
                flag_blink=1;
            }
            LED1_LAT=1;
        }
}

            if(flag_L1)
            {
                LED1_LAT=flag_L1;
            }
            if(flag_L2)
            {
                LED2_LAT=flag_L2;
            }
 
}


void status_machine_func(void)
{


    if( (Motor_run_status ==motor_startup ) && start_switchpushflag==FALSE && init_flag ==0 && Motor_Rpm_Mode==Between_400hz_and_1000hz )
    {
        Motor_run_status=low_speed_status;
                                #ifdef DEBUG
                              //  LED2_LAT=1;
                              //  LED1_LAT=1;
                                #endif  
    }
    if((Motor_run_status == low_speed_status ) && start_switchpushflag==FALSE && init_flag ==0 && Motor_Rpm_Mode==Between_1000hz_and_1500hz )
    {
        Motor_run_status=mid_speed_status;
                                #ifdef DEBUG
                              //  LED2_LAT=1;
                              //  LED1_LAT=0;
                                #endif  
    }
    if((Motor_run_status == mid_speed_status) && start_switchpushflag==FALSE && init_flag ==0 && Motor_Rpm_Mode==More_than_1500hz )
    {
        Motor_run_status=hi_speed_status;
                                #ifdef DEBUG
                              //  LED2_LAT=0;
                              //  LED1_LAT=0;
                                #endif  
    }
  if(Motor_run_status==hi_speed_status  && v_adj>900 )
    {

                              
    }
    if( ( Motor_run_status==hi_speed_status  ) && Motor_Rpm_Mode==Lower_than_400hz )
    {

    }

}
/*
void motor_start_up_check(uint16_t counter)
{
    //维持10-100ms 高强度触发，等马达转动起来后启动延时缓启程序
    
    if(Motor_run_status ==Init_status  && counter)
    {
        v_adj_slow=800;
        counter--;
    }
    if (counter<=0)
    {
        Motor_run_status ==motor_startup;
    }
    
}
*/
/*
uint16_t v_adj_convert(uint16_t button_adj)
{
    uint16_t tmp;
 
    if(button_adj<=230)
    {
        tmp=230;
    }
    else
    {
        if(button_adj<=950)
        {
           tmp= (uint16_t) (button_adj*19/72+169);// 230a+b=230,950a+b=420, a=0.264,b=169.31
        }
        else
        {
            tmp=425;
        }
    }
    return tmp;
    
}*/
/**
 End of File
 */