/*Slk_PEC
Programmer : Santosh L kakad  (SVNIT surat M-TECH power electronics P15EL001)
Email ID - > kakadsantosh24@gmail.com	  
Project : Sin Pulse Width modulation
OUTPUT Pin are PE 8,9,10,11,12,12
Dade TIME is 1us
TIMER 3 is used to generate a angle for referance signal with sampling rate of REF_FRE *360(Sample/degree)
TIM1->CNT serves as a carrier wave. max value of this carrier wave is 
TIM1-> ARR or 	TIM_TimeBaseStructure.TIM_Period
*/


#include<stm32f4xx.h>
#include"math.h"
#include"arm_math.h"

#define REF_FRE 50            // change here for change in sinwave frequncy 
#define CAR_FRE 10000					// chang here for chane in carrier frequency 
#define Har_Mag 0.1666f				// magnitude of 3rd harmonics injected (1/3) is prefferd
#define MI 0.8f

GPIO_InitTypeDef  GPIO_InitStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
NVIC_InitTypeDef NVIC_InitStructure;
DAC_InitTypeDef  DAC_InitStructure;
uint16_t ARR_val; 
uint16_t sig_flag=0;
float theta;
float theta1=0,theta2=120,theta3=240,Va,Vb,Vc,sig1,sig2,sig3;
void GPIO_Config(void);
void TIM_Config(void);
void DAC_Config(void);
void bound_check(float *var);
int main(void)
{		
	GPIO_Config();
	TIM_Config();
	DAC_Config();
	while(1)
	{
		if(sig_flag==1)
		{
			sig_flag=0;
			theta1++;
			bound_check(&theta1);
			theta2++;
			bound_check(&theta2);
			theta3++;
			bound_check(&theta3);

			//Va =MI*sinf(theta1*PI/180)/2; 			// FOR SIN PWM uncommnet three lines
			//Vb =MI*sinf(theta3*PI/180)/2;
			//Vc =MI*sinf(theta2*PI/180)/2;
			sig1 =(ARR_val+(ARR_val*MI*sinf(theta1*PI/180)+ Har_Mag*ARR_val*sinf(theta1*PI/60)))/2;   // for sin PWM with 3rd harmonics
			sig2 =(ARR_val+(ARR_val*MI*sinf(theta2*PI/180)+ Har_Mag*ARR_val*sinf(theta1*PI/60)))/2;
			sig3 =(ARR_val+(ARR_val*MI*sinf(theta3*PI/180)+ Har_Mag*ARR_val*sinf(theta1*PI/60)))/2;
			TIM1->CCR1 = (uint16_t)sig1;
			TIM1->CCR2 = (uint16_t)sig2;
			TIM1->CCR3 = (uint16_t)sig3;
			DAC->DHR12R1=(uint16_t)(TIM1->CNT)/3;
			DAC->DHR12R2=(uint16_t)sig1/8;
		}
	}
}
void GPIO_Config(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN ;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}
void DAC_Config()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None ;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None ;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_2, ENABLE);
	
	
}
void TIM_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period =(uint16_t)(SystemCoreClock/2*CAR_FRE) - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM1,ENABLE);
	
	ARR_val =(uint16_t)(SystemCoreClock/(2*REF_FRE*360)) - 1;  //18KHz
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseStructure.TIM_Period =ARR_val;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3,ENABLE);
	
	TIM_ITConfig(TIM3, TIM_IT_Update , ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse =(uint16_t) 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity =TIM_OCNPolarity_High;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	TIM1->BDTR = 0xff99;
}
void TIM3_IRQHandler(void)
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		sig_flag=1;
	}
	
void bound_check(float *var)
{
	if(*var ==360)
	{
		*var = 0;
	}
}
