/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ESP8266_Chelo.h"
#include "ModBUS_Chelo.h"
#include "STR_Chelo.h"
#include "ETH_W5100.h"
#include "string.h"
#include "stdio.h"
#include "RYLR896.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define TOK 1
#define FIND 0
#define SERVIDOR 1
#define CLIENTE 0
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */


struct LoRa lr;
struct MBUS mb_eth;			// Instancia Ethernet
struct MBUS mb_wf;		// Instancia Wi-Fi
struct W5100_SPI ETH; // Instancia de la comunicación Ethernet

uint8_t		decimal[17],
			spi_Data[64],
			spi_no_debug=0,
			EN_UART1_TMR=0,
			EN_UART2_TMR=0,
			EN_USART1_TMR=0,
			FLAG_TIMEOUT=0,
			FLAG_UART1=0,
			FLAG_UART2=0,
			SPI_READ_EN=0,
			UART1_DBG_EN=0,
			UART2_DBG_EN=0,
			error=0;

uint16_t	S0_get_size = 0,
			tx_mem_pointer=0,
			rx_mem_pointer=0,
			send_size=0;

uint32_t 	REG[254],		//Registros para ver ModBUS
	     	ms_ticks=0,
			min_ticks=0;



char		UART_RX_vect[512],
			UART2_RX_vect[512],
			UART_RX_vect_hld[512],
			CMP_VECT[]="\0",
			UART_RX_byte[2],
			UART2_RX_byte[2];

int 		UART_RX_items=0,
			UART2_RX_items=0,
			MB_TOUT_ticks=0,
			uart1pass=0,
			USART1_ticks=0,
			FLAG_USART1=0,
			chr_pos=0,
			items_rx=0,
			UART_RX_pos=0;
			UART2_RX_pos=0,
			ETH_DBG_EN=0;

enum {
		TEPELCO,
		TEST_1,
		TEST_2
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
	//uint8_t ESP8266_HW_Init(UART_HandleTypeDef *);
	void ESP8266_HW_Reset(void);
	void Actualizar_RXdata(int );

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//----------------------- ETHERNET W5100 Environment-------------------------//
		ETH_DBG_EN=0;
	//	GATEWAY ADDRESS
		ETH.GAR[0]=192;
		ETH.GAR[1]=168;
		ETH.GAR[2]=0;
		ETH.GAR[3]=1;
	//	SUBNET MASK
		ETH.SUBR[0]=255;
		ETH.SUBR[1]=255;
		ETH.SUBR[2]=255;
		ETH.SUBR[3]=0;
	//	MAC ADDRESS
		ETH.SHAR[0]=0x00;
		ETH.SHAR[1]=0x08;
		ETH.SHAR[2]=0xDC;
		ETH.SHAR[3]=0x00;
		ETH.SHAR[4]=0x00;
		ETH.SHAR[5]=0x01;

	//	IP ADDRESS
		ETH.SIPR[0]=192;
		ETH.SIPR[1]=168;
		ETH.SIPR[2]=0;
		ETH.SIPR[3]=6,//ETH.SIPR[3]=34,
	//  Socket RX memory
		ETH.RMSR=0x55;
	//  Socket TX memory


		ETH.TMSR=0x55;
	//  S0 Port Number
		ETH.S0_PORT[0]=0x01;
		ETH.S0_PORT[1]=0xF6;
	//	S0 Client IP ADDRESS
		ETH.S0_DIPR[0]=192;
		ETH.S0_DIPR[1]=168;
		ETH.S0_DIPR[2]=0;
		ETH.S0_DIPR[3]=3;
	//	S0 Client IP ADDRESS
		ETH.S0_DPORT[0]=0x01;
		ETH.S0_DPORT[1]=0xF6;

		ETH.gS0_RX_BASE = 0x6000;
		ETH.gS0_RX_MASK = 0x07FF;
		ETH.gS1_RX_BASE = 0x6800;
		ETH.gS1_RX_MASK = 0x07FF;
		ETH.gS2_RX_BASE = 0x7000;
		ETH.gS2_RX_MASK = 0x07FF;
		ETH.gS3_RX_BASE = 0x7800;
		ETH.gS3_RX_MASK = 0x07FF;
		ETH.gS0_TX_BASE = 0x4000;
		ETH.gS0_TX_MASK = 0x07FF;
		ETH.gS1_TX_BASE = 0x4800;
		ETH.gS1_TX_MASK = 0x07FF;
		ETH.gS2_TX_BASE = 0x5000;
		ETH.gS2_TX_MASK = 0x07FF;
		ETH.gS3_TX_BASE = 0x5800;
		ETH.gS3_TX_MASK = 0x07FF;

		ETH.S0_ENserver = 0;			//Actúa como servidor S0_ENserver=1 o cliente S0_ENserver=0

		//----------------------- ETHERNET W5100 Environment-------------------------//
	  //----------------------- LoRa ------------------------//

	  //----------------------- LoRa ------------------------//

	  //----------------------- WIFI ------------------------//

		spi_no_debug=1;
		ETH.NSS_PORT=GPIOA;
		ETH.NSS_PIN=GPIO_PIN_4;
		ETH.SPI= &hspi1;

		// ----------- FIN - Seteo de módulo Ethernet W5100 ----------- //

		 //----------------------- WIFI ------------------------//
			decimal[0]=1;
			decimal[1]=1;
			decimal[2]=1;
			decimal[3]=0;
			decimal[4]=1;
			decimal[5]=1;
			decimal[6]=1;
			decimal[7]=1;
			decimal[8]=1;
			decimal[9]=1;
			decimal[10]=1;
			decimal[11]=1;
			decimal[12]=1;
			decimal[13]=1;
			decimal[14]=1;
			decimal[15]=1;
			decimal[16]=1;
	 //----------------------- WIFI ------------------------//

	 //---------------------- ModBUS -----------------------//

		ModBUS_Config(&mb_eth);		//ETHERNET como cliente TCP envía  ModBUS
		mb_eth._mode = CLIENTE;
		ModBUS_Config(&mb_wf);	//WIFI como servidor TCP, recibe comadno ModBUS
		mb_wf._mode = CLIENTE;
		ModBUS_F03_Assign(&mb_wf,3,0xAA55);


	 //---------------------- ModBUS -----------------------//
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  SysTick_Config(SystemCoreClock/1000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
  ITM0_Write("\r\n INICIO OK\r\n",strlen("\r\n INICIO OK\r\n"));
     HAL_UART_Receive_IT(&huart1,(uint8_t *)UART_RX_byte,1);
     HAL_UART_Receive_IT(&huart2,(uint8_t *)UART2_RX_byte,1);
     if (ETH_DBG_EN == 1)ITM0_Write("\r\n SET-UP W5100 \r\n",strlen("\r\n SET-UP W5100 \r\n"));

   	 ETH.operacion=SPI_WRITE;
   	 ETH.TX[1]= 0;
   	 ETH.TX[2]= 1;
   	 ETH.TX[3]= 192;

   	 eth_init(&ETH);

   	 eth_socket_init(&ETH,0);

   SPI_READ_EN=1;
   ETH.operacion=SPI_READ;
   ETH.TX[1]= 0;
   ETH.TX[2]= 1;
   ETH.TX[3]= 0;
     HAL_Delay(1000);
     LoRa_set_sndTIMER(&lr,5000); //Inicio el ciclo de envíos

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


// AGREGAR TIMER EN MS TICKS PARA HABILITAR ESTADO Y CUENTA TODOS EN EL STRUCT
	  		if((FLAG_UART2 == 1)||(lr.tmr_dly_ON==1))  //Evento de dato recibido LoRA debo verificar que es
	  		{
	  			if(FLAG_UART2==1)
	  				{
	  				FLAG_UART2=0;
	  				LoRa_decode(&lr);
	  				}

	  			if(lr.tmr_dly_ON==1)
	  				{
	  					lr.tmr_dly_ON=0;
	  					LoRa_set_sndTIMER(&lr,5000);  //Vuelvo a enviar cada 5 seg
						lr.dest_address[0]='\0';
						lr.txbuff[0]='\0';


		  				lr.estado=_SENT;

		  				//Define address to send
		  				strncat(lr.dest_address,"1",1);
		  				//generate data to send
		  				char data[6];
		  				int n=0;
		  				while(n<16)//while(n<11)
		  					{
			  					data[0]='\0';
			  					//itoa(ModBUS_F03_Read(&mb_eth,n),data,10);
			  					FTOA(ModBUS_F03_Read(&mb_eth,n),data,decimal[n]);
								strncat(lr.txbuff,data,strlen(data));
								strncat(lr.txbuff,";",1);
								n++;
		  					}
		  				lr.txitems=strlen(lr.txbuff);
			  			error=LoRa_Send(&lr,&huart2);
	  				}

	  		}

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 100;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim2, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 100;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 150;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim3, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PCB_LED_GPIO_Port, PCB_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PCB_LED_Pin */
  GPIO_InitStruct.Pin = PCB_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PCB_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY_BTN_Pin WiFi_EN_Pin */
  GPIO_InitStruct.Pin = KEY_BTN_Pin|WiFi_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 SPI1_NSS_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_1|SPI1_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


int ITM0_Write( char *ptr, int len)
{
 int DataIdx;

  for(DataIdx=0; DataIdx<len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}

void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

	ms_ticks++;	//100 ms

	if(mb_eth._w_answer) MB_TOUT_ticks++;
	if ( mb_eth._w_answer && (mb_eth._timeout < MB_TOUT_ticks))
		{
			mb_eth._w_answer=0;
			MB_TOUT_ticks=0;
		}

// ENVIO DATOS LoRa ---------------------------------------------------------------//

	if(lr.tmr_dly_en==1)
	{
		lr.tmr_dlyCNT++;
		if(lr.tmr_dlyCNT > lr.tmr_dly)
		{
			lr.tmr_dly_ON=1;
			lr.tmr_dly_en=0;
		}
	}
// ENVIO DATOS LoRa ---------------------------------------------------------------//


/**********************[ INICIO - EHTERNET WDG ] **********************/

	if(ETH.S0_status == 0)
	{
		ETH.ETH_WDG++;
		if (ETH.ETH_WDG>=64000)
		{
			ETH.ETH_WDG=64000;
		}
	}

/**********************[ FIN 	- EHTERNET WDG ] **********************/

if (ms_ticks==100)//(ms_ticks==250)//(ms_ticks==50)
  {
	  ms_ticks=0;
	  min_ticks++;
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  if(spi_no_debug)
	  	  {
	  if(SPI_READ_EN)
	  {
	     ETH.S0_status=eth_rd_SOCKET_STAT(&ETH,0);

		  switch(ETH.S0_status)	//Check Socket status
	     {
			 case SOCK_CLOSED :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_CLOSED \r\n",strlen("\r\nS0_SOCK_CLOSED \r\n"));
					eth_wr_SOCKET_CMD(&ETH, 0 ,OPEN );
					 // Si no tengo intento de ARP por 5 segundos vuelvo a inicializar
					 if(ETH.ETH_WDG>=5000)
					 {
						 eth_init(&ETH);

						 eth_socket_init(&ETH,0);
					 }

				 }
			 break;
			 case  SOCK_INIT :
				 {
					 if(ETH.S0_ENserver == 1)
					 {
						 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_INIT \r\n",strlen("\r\nS0_SOCK_INIT \r\n"));
							eth_wr_SOCKET_CMD(&ETH, 0, LISTEN );
							ETH.ETH_WDG=0;
					 }
					 else
					 {
						 	eth_wr_SOCKET_CMD(&ETH,0, CONNECT);																				//only for server
						 	if (ETH_DBG_EN)ITM0_Write("\r\nETH-W5100-CONNECT\r\n",strlen("\r\nETH-W5100-CONNECT\r\n"));
						 	ETH.ETH_WDG=0;
					 }

				 }
			 break;
			 case SOCK_LISTEN :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_LISTEN \r\n",strlen("\r\nS0_SOCK_LISTEN \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_SYNSENT :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_SYNSENT \r\n",strlen("\r\nS0_SOCK_SYNSENT \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_SYNRECV :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_SYNRECV \r\n",strlen("\r\nS0_SOCK_SYNRECV \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_ESTABLISHED :
				 {
					 if (ETH_DBG_EN==1) ITM0_Write("\r\nS0_SOCK_ESTABLISHED \r\n",strlen("\r\nS0_SOCK_ESTABLISHED \r\n"));
					 ETH.ETH_WDG=0;

					if (ETH.S0_ENserver == 1)  // Si el puerto Ethernet actúa como server (Recibe datos conexión mas pedido mbus
					{

							S0_get_size = SPI_ETH_REG(&ETH, S0_RX_SZ_ADDR_BASEHH,S0_RX_SZ_ADDR_BASEHL ,SPI_READ, spi_Data,2);
							if(S0_get_size != 0x00)
							{
								eth_rd_SOCKET_DATA(&ETH,0,&rx_mem_pointer,S0_get_size); // read socket data
								SPI_ETH_WR_REG_16(&ETH,S0_RX_RD0,rx_mem_pointer );		// write rx memory pointer
								eth_wr_SOCKET_CMD(&ETH,0,RECV);							// write command to execute
								while(eth_rd_SOCKET_CMD(&ETH,0))						// wait until end of command execution
								{}

								CopiaVector(mb_eth._MBUS_RCVD, ETH.data, S0_get_size, 0, 0 );
								mb_eth._n_MBUS_RCVD=S0_get_size;

								if(S0_get_size > 0)	{ ETH.S0_data_available=1;}					//Flag data received

								if(ModBUS_Check(mb_eth._MBUS_RCVD, mb_eth._n_MBUS_RCVD))		//Ckecks ModBUS type data
								{
									ModBUS(&mb_eth);										//ModBUS protocol execution
									CopiaVector(ETH.data, mb_eth._MBUS_2SND, mb_eth._n_MBUS_2SND, 0, 0);
								}
								else
								{
									if (ETH_DBG_EN) ITM0_Write("\r\n NO MBUS \r\n",strlen("\r\n\r\n NO MBUS \r\n\r\n"));
								}

								send_size=mb_eth._n_MBUS_2SND;  //ModBUS data qty

								eth_wr_SOCKET_DATA(&ETH,0, &tx_mem_pointer, send_size);	// write socket data
								SPI_ETH_WR_REG_16(&ETH,0x424,tx_mem_pointer);			// write tx memory pointer
								eth_wr_SOCKET_CMD(&ETH,0,SEND);							// write command to execute
								while(eth_rd_SOCKET_CMD(&ETH,0))						// wait until end of command execution
								{}

							}
					}
					else	// Puerto ethernet labura como esclavo, se conecta al server para pedir datos
					{

						if (mb_eth._w_answer==0)
						{
							//Si ya envié vuelvo a enviar

							ETH.data[0]=0x00;
							ETH.data[1]=0x00;
							ETH.data[2]=0x00;
							ETH.data[3]=0x00;
							ETH.data[4]=0x00;
							ETH.data[5]=0x06;
							ETH.data[6]=0x01;
							ETH.data[7]=0x03;
							ETH.data[8]=0x00;
							ETH.data[9]=0x00;
							ETH.data[10]=0x00;
							ETH.data[11]=0x0A;
							send_size=12;

							ModBUS_F03_Request(&mb_eth,0,15);
							CopiaVector(ETH.data, mb_eth._MBUS_2SND, 12, 0, 0 );

							eth_wr_SOCKET_DATA(&ETH,0, &tx_mem_pointer, send_size);	// write socket data
							SPI_ETH_WR_REG_16(&ETH,0x424,tx_mem_pointer);			// write tx memory pointer
							eth_wr_SOCKET_CMD(&ETH,0,SEND);							// write command to execute
							while(eth_rd_SOCKET_CMD(&ETH,0))						// wait until end of command execution
							{}
							mb_eth._w_answer=1;	// Waiting answer flag
							MB_TOUT_ticks=0;	// restart counting
							if (ETH_DBG_EN==1) ITM0_Write("\r\n SENT MBUS REQ \r\n",strlen("\r\n\r\n SENT MBUS REQ \r\n\r\n"));
						}
						else
						{
						S0_get_size = SPI_ETH_REG(&ETH, S0_RX_SZ_ADDR_BASEHH,S0_RX_SZ_ADDR_BASEHL ,SPI_READ, spi_Data,2);
							if(S0_get_size != 0x00)
							{
								eth_rd_SOCKET_DATA(&ETH,0,&rx_mem_pointer,S0_get_size); // read socket data
								SPI_ETH_WR_REG_16(&ETH,S0_RX_RD0,rx_mem_pointer );		// write rx memory pointer
								eth_wr_SOCKET_CMD(&ETH,0,RECV);							// write command to execute
								while(eth_rd_SOCKET_CMD(&ETH,0))						// wait until end of command execution
								{}

								CopiaVector(mb_eth._MBUS_RCVD, ETH.data, S0_get_size, 0, 0 );
								mb_eth._n_MBUS_RCVD=S0_get_size;

								if(S0_get_size > 0)	{ ETH.S0_data_available=1;}

								if(ModBUS_Check(mb_eth._MBUS_RCVD, mb_eth._n_MBUS_RCVD))		//Ckecks ModBUS type data
									{
										mb_eth._w_answer=0;  									//Si el mensaje recibido ya es modbus digo que ya recibi
										MB_TOUT_ticks=0;
										ModBUS(&mb_eth);										//ModBUS protocol execution
										CopiaVector(ETH.swap, mb_eth._MBUS_RCVD, mb_eth._n_MBUS_RCVD, 0, 0);
										CopiaVector(mb_wf._Holding_Registers, mb_eth._Holding_Registers, 64, 0, 0);
										if (ETH_DBG_EN==1) ITM0_Write("\r\n RCVD MBUS REQ \r\n",strlen("\r\n\r\n RCVD MBUS REQ \r\n\r\n"));
									}
									else
										{
										if (ETH_DBG_EN) ITM0_Write("\r\n NO MBUS \r\n",strlen("\r\n\r\n NO MBUS \r\n\r\n"));
										}
							}
						}
					}
				 }
			 break;
			 case SOCK_FIN_WAIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_FIN_WAIT \r\n",strlen("\r\nS0_SOCK_FIN_WAIT \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_CLOSING :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_CLOSING \r\n",strlen("\r\nS0_SOCK_CLOSING \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case  SOCK_TIME_WAIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_TIME_WAIT \r\n",strlen("\r\nS0_SOCK_TIME_WAIT \r\n"));
					eth_wr_SOCKET_CMD(&ETH,0, DISCON );
					while( SPI_ETH_REG(&ETH, S0_CR_ADDR_BASEH,S0_CR_ADDR_BASEL ,SPI_READ, spi_Data,1))
					{}
					ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_CLOSE_WAIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_CLOSE_WAIT \r\n",strlen("\r\nS0_SOCK_CLOSE_WAIT \r\n"));
					eth_wr_SOCKET_CMD(&ETH,0,DISCON );
					while( SPI_ETH_REG(&ETH, S0_CR_ADDR_BASEH,S0_CR_ADDR_BASEL ,SPI_READ, spi_Data,1))
					{}
					ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_LAST_ACK :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_LAST_ACK \r\n",strlen("\r\nS0_SOCK_LAST_ACK \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_UDP :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_UDP \r\n",strlen("\r\nS0_SOCK_UDP \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case  SOCK_IPRAW :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_IPRAW \r\n",strlen("\r\nS0_SOCK_IPRAW \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case  SOCK_MACRAW :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_MACRAW \r\n",strlen("\r\nS0_SOCK_MACRAW \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_PPOE :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_PPOE \r\n",strlen("\r\nS0_SOCK_PPOE \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;

			 default:
				 {

				 }
	     }
	  }
	  }else
	  	  {

		  SPI_ETH(&ETH);
	  	  }
	  if(min_ticks==2)//if(min_ticks==10)
		  {
		  	  min_ticks=0;  /* SETEO CADA 2 min*/
		  }
  }

	if(EN_USART1_TMR==1) USART1_ticks++;

	if(USART1_ticks>=2)//if(USART1_ticks>=10)
	{
		USART1_ticks=0;
		FLAG_USART1=1;
		EN_USART1_TMR=0;
		items_rx=uart1pass;
		uart1pass=0;
	}

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *ERRUART)

{
	if(ERRUART->Instance==USART1)
	{
		 volatile int aore=0;
		 volatile int bore=0;

		//Al leer los registros de esta forma SR y luego DR se resetean los errores de Framing Noise y Overrun FE NE ORE
		 aore=ERRUART->Instance->SR;
		 bore=ERRUART->Instance->DR;
		 HAL_UART_DeInit(ERRUART);
		 MX_USART1_UART_Init();
		 HAL_UART_Receive_IT(ERRUART,(uint8_t *)UART_RX_byte,1);
	}
	if(ERRUART->Instance==USART2)
	{
		 volatile int aore=0;
		 volatile int bore=0;
		//Al leer los registros de esta forma SR y luego DR se resetean los errores de Framing Noise y Overrun FE NE ORE
			aore=ERRUART->Instance->SR;
			bore=ERRUART->Instance->DR;


		//HAL_UART_Transmit_IT(&huart5,"U4",strlen("U4"));
		 HAL_UART_DeInit(ERRUART);
		 MX_USART2_UART_Init();
		 HAL_UART_Receive_IT(ERRUART,(uint8_t *)UART_RX_byte,1);
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *INTSERIE)
{

// WiFi	USART 1 TIMER2
	if(INTSERIE->Instance==USART1)
		 {
			UART_RX_vect[UART_RX_pos]=UART_RX_byte[0];
			UART_RX_pos++;
			if(UART_RX_pos>=512) UART_RX_pos=512;
			HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);//HAL_TIM_Base_Start_IT(&htim7);	//Habilito el timer
			TIM2->CNT=1;
			EN_UART1_TMR=1;	//Habilito Timeout de software
			HAL_UART_Receive_IT(INTSERIE,(uint8_t *)UART_RX_byte,1);
		 }
// LoRa USART2 TIMER3
	if(INTSERIE->Instance==USART2)
		 {
			UART2_RX_vect[UART2_RX_pos]=UART2_RX_byte[0];
			UART2_RX_pos++;
			if(UART2_RX_pos>=512) UART2_RX_pos=512;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 1);
			HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);//HAL_TIM_Base_Start_IT(&htim7);	//Habilito el timer
			TIM3->CNT=1;
			EN_UART2_TMR=1;	//Habilito Timeout de software
			HAL_UART_Receive_IT(INTSERIE,(uint8_t *)UART2_RX_byte,1);
		 }
 }

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *TIMER)
{
// WiFi	USART 1 TIMER2
		//void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim2)
		if(TIMER->Instance==TIM2)
			{
				 HAL_TIM_OC_Stop_IT(TIMER, TIM_CHANNEL_1); //Paro el timer
				 FLAG_UART1=1;
				 EN_UART1_TMR=0;
				 UART_RX_items=UART_RX_pos;
				 UART_RX_pos=0;
				 UART_RX_vect[512]='\0'; //Finalizo el vector a la fuerza ya que recibo hasta 124
				 CopiaVector(UART_RX_vect_hld,UART_RX_vect,UART_RX_items,1,CMP_VECT);
				 HAL_UART_Receive_IT(&huart1,(uint8_t *)UART_RX_byte,1); //Habilito le recepcón de puerto serie al terminar
				 if (UART1_DBG_EN==1)
				 {
					 ITM0_Write((uint8_t *)UART_RX_vect_hld,UART_RX_items);
				 }
		}
// LoRa USART2 TIMER3
		//void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim2)
		if(TIMER->Instance==TIM3)
			{
				 HAL_TIM_OC_Stop_IT(TIMER, TIM_CHANNEL_1); //Paro el timer
				 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
				 FLAG_UART2=1;
				 EN_UART2_TMR=0;
				 UART2_RX_items=UART2_RX_pos;
				 UART2_RX_pos=0;
				 UART2_RX_vect[512]='\0'; //Finalizo el vector a la fuerza ya que recibo hasta 124
				 CopiaVector(lr.rxbuff,UART2_RX_vect,UART2_RX_items,1,CMP_VECT);
				 lr.rxitems=UART2_RX_items;
				 HAL_UART_Receive_IT(&huart2,(uint8_t *)UART2_RX_byte,1); //Habilito le recepcón de puerto serie al terminar
				 if (UART2_DBG_EN==1)
				 {
					 ITM0_Write("\r\nData LoRa recibida = ",strlen("\r\nData LoRa recibida = "));
					 ITM0_Write((uint8_t *)UART2_RX_vect,UART2_RX_items);
					 ITM0_Write("\r\n",strlen("\r\n"));
				 }
		}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
