/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

extern uint8_t direc;//variable que indica si debe decrecer o crecer el el valor mostrado

extern uint8_t ms; //variable externa ms
extern uint8_t sec; //variable externa sec
extern uint8_t min; //variable externa min
extern uint8_t hour; //variable externa hour
extern uint8_t barra; //variable externa barra

float timeRef=0;//variable para configurar el progra

uint8_t Regms = 0; //variable que registra el valor programado de ms
uint8_t Regsec = 0;//variable que registra el valor programado de sec
uint8_t Regmin = 0;//variable que registra el valor programado de min
uint8_t Reghour = 0;//variable que registra el valor programado de hour


uint8_t bt4=0; //indica el estado del boton 4

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
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

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  IniciaComu();//inicia comunicacion
  EnviarByte(0x8A);//se envia comando para configurar el brillo
  ParaComu();//para comunicacion
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t time=0; //indicador de veces, esto nos sirve para detectar las veces que se presiono el boton 1
  uint8_t B1ant= 0; //Estado anterior del pulsador 1
  uint8_t estadoCron=0; //Me indica si ya se inició o finalizó la cuenta
  uint8_t almacenado=0;//indica si guarda el record
  uint8_t leds=1;//estado de los leds
  Home();//muestra en el display "HOME"
  while (1)
  {
	  Leds(leds);//ejecuta la funcion que desplaza un led de der a izq
	  uint8_t bt = LeerBotones();  // Lee el estado actual de los botones
	  uint8_t B1act = (bt >> 1) & 1; //Estado actual del pulsador 1

	  //accion al pulsar el boton 2 y encontrarnos en el estado 0 del cronometro
	  	  if(((bt >> 2) & 1) & (estadoCron==0)){  //accion al pulsar el boton 2 (START)
	  		  direc=0;//se indica que la cuenta debe ser ascendente
	  		  leds=0;//el estado de leds vuelve a cero
	  		  HAL_TIM_Base_Stop_IT(&htim6);//se detiene el cronometro
	            Reiniciar();//reiniciamos los valores del contador ms,sec,min y hour a cero
	            Cuenta();//Da la cuenta regresiva 3,2,1...
	            barra=2;//Se indica el estado de la barra como 2
	  		  HAL_TIM_Base_Start_IT(&htim6);//se iniciar las interrupciones y esto inicia el contador
	  		  estadoCron=1; //estado del cronometro en 1
	  	  }

		  //accion al pulsar los botones 3 y 1 al mismo tiempo, estando en el estado 0 del cronometro
		  if(((bt >> 3) & 1) & ((bt >> 1) & 1) & (estadoCron==0) ){
			  BorrarRecord();//se borra el record almacenado
			  almacenado=0;//se indique que no hay record almacenado
			  Deleted();//Se muestra en el "DELETED" en el display
			  HAL_Delay(1000);//retraso de 1 segundo para que se visualice "DELETED"
		  }

		  //accion al pulsar el boton 3 y el estado del cronometro en 0
		  if(((bt >> 3) & 1) & (estadoCron==0)){  //menu record
			  if(almacenado){
				  MostrarRecord();//muestra el record si esta almacenado
			  }else{
				  MostrarRecordN();//muestra que no hay record almacenado "NoDATA"
			  }
		  }

		  //accion al pulsar el boton 4 y el estado del cronometro en 0
		  if(((bt >> 4) & 1) & (estadoCron==0)){ //modo programable
			  bt4=0;//estado de bt4 en cero
			  estadoCron=2;//Se indica el estado del cronometro a 2
			  leds=0;//leds en off
			  HAL_TIM_Base_Stop_IT(&htim6);//se detiene el cronometro
			  Reiniciar();//reiniciamos los valores de ms,sec,min y hour
			  Progra();//mostramos en el Display "PROGRA"
			  HAL_Delay(1000);//retraso de 1 segundo para que se visualice "PROGRA"
			  ActualizaDisplay();//Actualizamos el display
		  }

	//accion al pulsar el boton 7 (HOME)
		  if(((bt >> 7) & 1)){ //Home
			  HAL_TIM_Base_Stop_IT(&htim6);//Se detiene el cronometro
			  Reiniciar();//reiniciamos los valores del contador ms,sec,min y hour a cero
			  estadoCron=0;//estado del cronometro vuelve a 0
			  leds=1;//estado de los leds en 1
			  Home();//Se muestra "HOME" en el display
		  }

	  //accion al pulsar el boton 1 y encontrarnos en el estado 1 del cronometro  (PAUSE & CONTINUE)
	  if ( (B1act != B1ant) & (estadoCron==1)){
	          // Si hay un cambio de estado en el pulsador
	          if (B1act == 1) {//si se presiona el pulsador1
	              if (time == 0) {//si es la primera vez que se pulsa
	                  HAL_TIM_Base_Stop_IT(&htim6);//detiene el cronometro parando la ejecucion de interrupciones
	                  time=1;//indica que el pulsador ya fue presionado una vez
	              } else if (time == 1) {//si es la segunda vez que se pulsa
	                  HAL_TIM_Base_Start_IT(&htim6);//continua la cuenta reiniciando la ejecucion interrupciones
	                  time = 0;//el estado de veces pulsado vuelve a cero
	              }
	          }
	          B1ant = B1act;
	      }

	  //accion al pulsar el boton 3 estando en el estado 1 del cronometro
	  if(((bt >> 3) & 1) & (estadoCron==1)){  //con record
		  Record(); //analiza si es record y lo almacena
		  almacenado=1; //indique que ya hay record
		  Probed();//muestra en el display "PROBED"
		  HAL_Delay(2000);//retraso de 2 segundos para que se visualice "PROBED"
	  }

	  //accion al pulsar el boton 4 y el estado del cronometro en 1
	  if(((bt >> 4) & 1) & (estadoCron==1)){  //menu record
		  direc=1;//cronometro en forma descendente
	  }

	  //accion al pulsar el boton 5 y el estado del cronometro en 1
	  if(((bt >> 5) & 1) & (estadoCron==1)){  //menu record
		  direc=0;//cronometro en forma ascendente
	  }

	  //accion al pulsar el boton 3 y el estado del cronometro en 2
	  if(((bt >> 3) & 1) & (estadoCron==2)){ //temporizador config
		  if(bt4 == 1){//si el estado de bt4 está en 1
			  ms++;//aumeta ms en una unidad
		  }
		  if(bt4 == 2){//si el estado de bt4 está en 2
			  sec++;//aumeta ms en una unidad
		  }
		  if(bt4 == 3){//si el estado de bt4 está en 3
			  min++;//aumeta ms en una unidad
		  }
		  if(bt4 == 0){//si el estado de bt4 está en 0
			  hour++;//aumeta ms en una unidad
		  }
		  if(bt4<0){//si el estado de bt4 es menor que 0
			  bt4=0;//se reinicia el estado de bt4
		  }
		  ActualizaDisplay();//se actualiza el display
		  HAL_Delay(500);//retraso de medio segundo para permitir pulsar
	  }

	  //accion al pulsar el boton 4 y el estado del cronometro en 2
	  if(((bt >> 4) & 1) & (estadoCron==2)){ //temporizador config
		  bt4++;//aumenta en una unidad el estado de bt4
		  if(bt4>3){//si el estado de bt4 es mayor a 3
			  bt4=0;//se reinicia el estado de bt4
		  }
		  HAL_Delay(500);//retraso de medio segundo para permitir pulsar
	  }

	  //accion al pulsar el boton 5 y el estado del cronometro en 2
	  if(((bt >> 5) & 1) & (estadoCron==2)){ //startprogra
		  Regms = ms;//establece el valor programado de ms
		  Regsec = sec;//establece el valor programado de sec
		  Regmin = min;//establece el valor programado de min
		  Reghour = hour;//establece el valor programado de hour
		  Reiniciar();//reiniciamos los valores de ms,sec,min y hour a cero
		  barra=1;//indica la configuracion de la barra de leds en ON
		  timeRef=(Reghour * 3600) + (Regmin * 60) + Regsec + (Regms / 100.0);//tiempo de referencia
		  Cuenta();//muestra la cuenta 3,2,1....
		  HAL_TIM_Base_Start_IT(&htim6);//inicia el cronometro
	  }

	  //finalizacion del temporizador programado
	  if((Regms <= ms) & (Regsec <= sec) & (Regmin <= min) & (Reghour <= hour) & (estadoCron==2)){ //barra de progreso config
		  HAL_TIM_Base_Stop_IT(&htim6);//cuando se llega a los valores "Reg", se para el cronometro
		  barra=0;//indica la configuracion de la barra de leds en OFF
	  }

	  HAL_Delay(60);//retraso para evitar rebotes al pulsar
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 99;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 LD2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
