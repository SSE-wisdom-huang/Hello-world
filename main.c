/*
*********************************************************
*工 程 名：adc
*功能描述：adc查询模式
*作    者：Yexin.Song
*时    间：2020.05.10
*版    本：V1.0.2

更新内容：      2700K    3000K    3500K     4000K     5000K
           WW   100%     85%      59%       36%       0.1%
           CW   0%       15%      41%       64%       99.9%





*********************************************************
*/
#include"rb5xpxx5a.h"

typedef unsigned char u8;
typedef unsigned int  u16;


u16   Adc1_Data_Num;
u16   Adc_Volata_Value;
u16   voltage1 = 1300;
u16   voltage2 = 2100;
u16   voltage3 = 2900;
u16   voltage4 = 3700;


/*
**********************************************************
*函 数 名：mclk_config
*功能描述：主时钟配置
*输    入：无
*输    出：无
**********************************************************
*/
void mclk_config(void)
{
    SPKEY = 0x36;
    CKDS = 2;     //8分频
}

/*
**********************************************************
*函 数 名：io_config
*功能描述：io端口配置
*输    入：无
*输    出：无
**********************************************************
*/
void io_config(void)
{
    P0DPL = 0xAA;	//推挽输出
    P1DPL = 0x2A;	//00 10 10 10  P1.3 口悬空输入; P1.2 口推挽输出;  P1.1 口推挽输出;  P1.0 口推挽输出;
    P1DSS = 0x03; //P1.1和P1.0 拉/灌电流选择强驱动档。
}



/*
**********************************************************
*函 数 名：PPG_CONFIG
*功能描述：PPG配置
*输    入：无
*输    出：无
**********************************************************
*/
void PPG_CONFIG(void)
{
   
    IOMUXC2 = 0x0C;   //设置PWM10输出到P1.1端口
    IOMUXC1 = 0x02;    //设置PWM11输出到P1.0端口
    PPGPSC = 0x0F;   //内部高频时钟分频计数32MHZ/PPGPSC  k
    PPGOMD = 0x0C;  //允许PWM11，PWM10
  
    PPGPH = 0x03;
    PPGPL = 0xE7; //  //设置PWM周期  PPGCLK/(PPGH*256+PPGL)  2K

 //   C1DH = 0x01;
 //  C1DL = 0xF3;         //设置PWM1占空比
    PPGCON = 0;          //向上计数，边沿对齐
    C1CON = 0; //12位互补输出
    

    
    SPKEY = 0x36;   //work
    PPGOPS = 0x00;  //PWM11正常输出 PWM10正常输出
  //  PPGCON = 0x80;  //是能PPG
}


/*
**********************************************************
*函 数 名：adc_config
*功能描述：adc模块配置
*输    入：无
*输    出：无
**********************************************************
*/
void adc_config(void)
{
    P1AMEN = 8;  //开启ADC3通道，相对应的通道需设置为浮空输入
    ADCFG = 0x11;//Vref=VDD,4分频，右对齐
    ADCON = 0x38;//AD通道来自ADC3，AD模块使能
}

/*
**********************************************************************************************
*函 数 名：samp_acquire
*功能描述：ADC数据采集，转换为10bit
*输    入：无
*输    出：无
**********************************************************************************************
*/
u16 samp_acquire(void)
{
    u16 vol=0;
    while(EOC == 1){}//判断是否有未完成的转换
    ADCHS0 = 1;
    ADCHS1 = 1;
    ADCHS2 = 0;
    ADCHS3 = 0;
	
    vol = ADIF;      //清标志(读1清0)
    ADIF = 0;

    ADGO  = 1;       //启动转换
    while(ADIF == 0) //等待转换完成标志
    {}
	
	//读取转换值
    vol = ADRH;
		vol <<= 8;
		vol |= ADRL;
    return vol;
}

void main(void)
{

    Adc1_Data_Num = 0;
    Adc_Volata_Value = 0;


    EA = 0;
    mclk_config();
    io_config();
    PPG_CONFIG();
    adc_config();
    while(1)
    {

      Adc1_Data_Num = samp_acquire();
      Adc_Volata_Value = (u16) (((float)((Adc1_Data_Num/4096.0)*5))*1000); //adc的电压扩大1000倍
      
      if(Adc_Volata_Value < voltage1)
      {
            C1DH = 0x03;
            C1DL = 0xDD;         //设置PWM1占空比 99%
            PPGCON = 0x80;       //是能PPG
         
      }
      
      else if(Adc_Volata_Value < voltage2)
      {
            C1DH = 0x02;
            C1DL = 0x7F;         //设置PWM1占空比 85%  15%  64% --- 36%
            PPGCON = 0x80;       //是能PPG
            
      }
      
      else if(Adc_Volata_Value < voltage3)
      { 
            C1DH = 0x01;
            C1DL = 0x99;         //设置PWM1占空比 41%  --- 59%  
            PPGCON = 0x80;       //是能PPG
      }
      
      else if(Adc_Volata_Value < voltage4)
      {
            C1DH = 0x00;
            C1DL = 0x95;         //设置PWM1占空比 36%  --- 64%   15% -- 85%
            PPGCON = 0x80;       //是能PPG
        
      }
      
      else 
      {
            C1DH = 0x00;
            C1DL = 0x09;         //设置PWM1占空比 1%
            PPGCON = 0x80;       //是能PPG
          
      }
      
    }
}
