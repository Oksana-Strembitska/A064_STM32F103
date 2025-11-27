/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : STM32F103_CAN_chair_A027_V3_COAG.c
  * @brief          : Program for chair board and pedal Board AEGA 5.139.027/3
											with coagulator
  ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "crc.h" // розрахунок контрольної суми
#include "io.h" // порти вводу / виводу
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
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

CAN_HandleTypeDef hcan;

/* USER CODE BEGIN PV */
void decodeDataChairBuffRx (uint8_t *dataChairBuffRx);	// функція для обробки змінної data_chair - формування відповіді data_chair_answer
void SLIP_AddCrcToFrame( uint8_t *frame, uint16_t *size); // функція для розрахунку контрольної суми
void decodeDataPedalONOFF (uint8_t *dataPedalONOFF); // фунуція для обробки стану педалі з А025

CAN_TxHeaderTypeDef TxHeader; // вихідний буфер CAN_Tx
CAN_RxHeaderTypeDef RxHeader; // вхідний буфер CAN_Rx

uint8_t len = 0; // лічильний для прийнятих байтів
uint8_t CAN_status_OK = 0; // прапорець, який сигналізує про наявність даних по CAN
uint8_t CANRxDataChairNOT_OK = 0; // прапорець, який сигналізує про наявність даних по CAN

uint8_t dataChairBuffRx [255] = {0}; // буфер data_chair - буфер, який містить інформацію про поточний стан кнопок, які керують кріслом, LIGHT ON, WATER F та WATER S
uint8_t dataChairAnswerBuffTx [255] = {0}; // буфер data_chair_answer - підтвердження отримання буфера data_chair
uint8_t dataPedalBuffTx [255] = {0}; // буфер data_pedal - буфер, який містить інформацію про стан функцій педалі ADC, Pedal ON/OFF, CHIP, SPRAY
uint8_t dataResetBuffRx [255] = {0};
uint8_t dataBuffRx [255] = {0};
uint8_t dataSPRAYBuffTx [255] = {0};
uint8_t dataPedalONOFF [255] = {0}; // буфер для прийому даних по стану педалі з А025

uint32_t TxMailBox = 0; // Mailbox для відправки даних по CAN
uint8_t canRxBuffer [255] = {0}; // буфер для прийому даних по CAN
uint8_t canTxBuffer [255] = {0}; // буфер для передачі даних по CAN

uint16_t adcResult = 0; // дані, які отримано з АЦП 12-бітного
uint8_t dataResult = 0; // дані, які приведено до АЦП 8-бітного

uint8_t button_push_pedal = 0; // стан кнопки Pedal_ON/OFF / Педаль ВКЛ./ВиКЛ.
uint8_t button_push_CHIP = 0; // стан кнопки CHIP / ПРОДУВКА
uint8_t butto_push_SPRAY = 0; // стан кнопки SPRAY / СПРЕЙ

uint8_t button_push_pedal_sent = 0; // змінна, що відповідає за стан кнопки Pedal_ON/OFF / Педаль ВКЛ./ВиКЛ.
uint8_t button_push_CHIP_sent = 0; //змінна, що відповідає за стан кнопки CHIP / ПРОДУВКА
uint8_t button_push_SPRAY_sent = 0; // змінна, що відповідає за стан кнопки SPRAY / СПРЕЙ

uint8_t stateButtSPRAY = 1; // стан кнопки SPRAY / СПРЕЙ

uint8_t sent_data = 0; // стан натискання кнопок педалі

uint8_t len_pedal = 0; // лічильник для надсилання даних про стан всіх кнопок педалі
uint8_t len_chip_spray = 0; // лічильник для надсилання даних тільки про стан кнопок CHIP та SPRAY

uint8_t flag_rx_a025 = 0; // прапорець по педалі - прийом або передача 
uint8_t pedal_on_off_a025 = 0; // стан натискання кнопки педалі на А025

uint32_t timeStatus = 0;
uint32_t tim_data_answer = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t data_chair_OK = 0; // прапорець, що сигналізує про те, що прийом / передача даних по CAN пройшли успішно
uint8_t data_chair_NOT_OK = 0; // прапорець, що сигналізує про перехід до стану, коли світлодіод мигає із фіксованою частотою - помилка по CAN

uint8_t stateChairDown = 0; // стан байта, що відповідає за стан кнопки CHAIR DOWN / КРІСЛО ВНиЗ
uint8_t stateChairUp = 0; // стан байта, що відповідає за стан кнопопки CHAIR UP / КРІСЛО ВВЕРХ
uint8_t stateBackrestUp = 0; // стан байта, що відповідає за стан кнопки BACKREST UP / СПиНКА ВВЕРХ
uint8_t stateBackrestDown = 0; // стан байта, що відповідає за стан кнопки BACKREST DOWN / СПиНКА ВНиЗ
uint8_t stateSpittoon = 0; // стан байта, що відповідає за стан кнопки SPITTOON / СПЛЬОВУВАННЯ
uint8_t stateA = 0; // стан байта, що відповідає за стан кнопки А
uint8_t stateB = 0; // стан байта, що відповідає за стан кнопки В
uint8_t stateLightOn; // стан байта, що відповідає за стан кнопки LIGHT ON / СВІТиЛЬНиК
uint8_t stateWaterF; // стан байта, що відповідає за стан кнопки WATER F / НАПОВНЕННЯ СКЛЯНКи
uint8_t stateWaterS; // стан байта, що відповідає за стан кнопки WATER S / ЗМиВ ПЛЮВАЛЬНи0ЦІ
uint8_t state_data_chair = 0; // прапорець, який використовується для переходу до формування буфера data_chair_answer

uint16_t dataChairRxCRC = 0; // отримана у data_chair контрольна сума CRC
uint16_t dataChairCalcCRC = 0; // розрахована для data_chair контрольна сума CRC

/*Стан натискання педалі отриманий з А025*/
void decodeDataPedalONOFF (uint8_t *dataPedalONOFF)
{
	if (dataPedalONOFF [0] != 0x00 && dataPedalONOFF [1] != 0x00 && dataPedalONOFF [7] != 0x00)
	{
		if (dataPedalONOFF [2] == 0x80)
		{
			pedal_on_off_a025 = 0;
		}			
		else if (dataPedalONOFF [2] == 0x81)
		{
			pedal_on_off_a025 = 1;
		}
	}
}

void decodeDataBuffRx (uint8_t *dataBuffRx) // робота з прийнятим повідомленням data_reset
{
if (dataBuffRx [0] != 0x00 && dataBuffRx [1] != 0x00 && dataBuffRx [15] != 0x00)
{ 
		if (dataBuffRx [4] == 0x22)
		{
			butto_push_SPRAY = 1;
			button_push_SPRAY_sent = 0x2B;
		}
		else if (dataBuffRx [4] == 0x00)
		{
			butto_push_SPRAY = 0;
			button_push_SPRAY_sent = 0x2D;
		}	
	}
}

/* Робота з повідомленням про поточний стан кнопок data*/
void decodeDataChairBuffRx (uint8_t *dataChairBuffRx) // робота з прийнятим повідомленням data_chair
{	
	/* Контрольна сума отримана у буфері data_chair*/
	dataChairRxCRC = dataChairBuffRx [13];
	dataChairRxCRC = dataChairRxCRC << 8;
	dataChairRxCRC |= dataChairBuffRx [14];

	/*Розрахунок для data_chair контрольної суми - розрахунок проводиться для прийнятого буфера*/
	uint16_t lengDataBuffRx = 13; // довжина буфера корисних даних для розрахунку контрольної суми СRC
	SLIP_AddCrcToFrame (dataChairBuffRx, &lengDataBuffRx); // функція для розрахунку контрольної суми CRC
	
	dataChairCalcCRC = dataChairBuffRx [13];
	dataChairCalcCRC = dataChairCalcCRC << 8;
	dataChairCalcCRC |= dataChairBuffRx [14];
		
	if (dataChairCalcCRC == dataChairRxCRC) // умова виконується якщо контрольна сума CRC отримана та розрахована - співпала
	{
		CAN_status_OK = 1; // засвічується світлодіод, який сигналізує, що є передача даних по CAN 
		data_chair_OK = 1; // засвічується світлодіод, який сигналізує про відправку буфера data_chair_answer та отримання буфера data_chair
		data_chair_NOT_OK = 0; // скидається прапорець, що сигналізує про помилку прийому / передачі по CAN
		/* Формування буфера data_chair_answer - ввідповідь на прийняте повідомлення data_chair*/
		dataChairAnswerBuffTx [0] = 0x16;
		dataChairAnswerBuffTx [1] = 0x06;
		dataChairAnswerBuffTx [2] = dataChairBuffRx [13]; // в посилці надсилати контрольну суму старший байт
		dataChairAnswerBuffTx [3] = dataChairBuffRx [14]; // в посилці надсилати контрольну суму молодший байт
		dataChairAnswerBuffTx [4] = 0xFF;

		stateChairDown = dataChairBuffRx [2]; // змінна, яка визначає стан кнопки CHAIR DOWN / КРІСЛО ВНиЗ
		stateChairUp = dataChairBuffRx [3]; // змінна, яка визначає стан кнопки CHAIR UP / КРІСЛО ВВЕРХ
		stateBackrestUp = dataChairBuffRx [4]; // змінна, яка визначає стан кнопки BACKREST UP / СПиНКА ВВЕРХ
		stateBackrestDown = dataChairBuffRx [5]; // змінна, яка визначає стан кнопки BACKREST DOWN / СПиНКА ВНиЗ
		stateSpittoon = dataChairBuffRx [6]; // змінна, яка визначає стан кнопки SPITTOON / СПЛЬОВУВАННЯ
		stateA = dataChairBuffRx [7]; // змінна, яка визначає стан кнопки A
		stateB = dataChairBuffRx [8]; // змінна, яка визначає стан кнопки В
		stateLightOn = dataChairBuffRx [9]; // змінна, яка визначає стан кнопки LIGHT ON / СВІТиЛЬНиК
		stateWaterF = dataChairBuffRx [10]; // змінна, яка визначає стан кнопки WATER F / НАПОВНЕННЯ СКЛЯНКи
		stateWaterS = dataChairBuffRx [11]; // змінна, яка визначає стан кнопки WATER S / ЗМиВ ПЛЮВАЛЬНиЦІ

		/*Формування стану виходів мікроконтролера для керування стоматологічним кріслом*/
		
		/*Стан кнопки CHAIR DOWN / КРІСЛО ВНиЗ*/
		if (stateChairDown == 0x10)
		{
			ChairDownSet (); // вихід мікроконтролера в 1
		}
		else if (stateChairDown == 0x11)
		{
			ChairDownReset (); // вихід мікроконтролера в 0
		}
		
		/*Стан кнопки CHAIR UP / КРІСЛО ВВЕРХ*/
		if (stateChairUp == 0x20)
		{
			ChairUpSet (); // вихід мікроконтролера в 1 
		}
		else if (stateChairUp == 0x21)
		{
			ChairUpReset (); // вихід мікроконтролера в 0	
		}		
		
		/*Стан кнопки BACKREST UP / СПиНКА ВВЕРХ*/
		if (stateBackrestUp == 0x30)
		{
			BackrestUpSet (); // вихід мікроконтролера в 1
		}
		else if (stateBackrestUp == 0x31)
		{
			BackrestUpReset (); // вихід мікроконтролера в 0
		}				
		
		/*Стан кнопки BACKREST DOWN / СПиНКА ВНиЗ*/
		if (stateBackrestDown == 0x40)
		{
			BackrestDownSet (); // вихід мікроконтролера в 1
		}
		else if (stateBackrestDown == 0x41)
		{
			BackrestDownReset (); // вихід мікроконтролера в 0	
		}	
		
		/*Стан кнопки SPITTOON / СПЛЬОВУВАННЯ*/
		if (stateSpittoon == 0x50)
		{
			SpittoonSet (); // вихід мікроконтролера в 1
		}
		else if (stateSpittoon == 0x51)
		{
			SpittoonReset (); // вихід мікроконтролера в 0	
		}
		
//		/*Стан кнопки A*/
//		if (stateA == 0x60)
//		{
//			ASet (); // вихід мікроконтролера в 1
//		}
//		else if (stateA == 0x61)
//		{
//			AReset (); // вихід мікроконтролера в 0
//		}
		
//		/*Стан кнопки B*/
//		if (stateB == 0x70)
//		{
//			BSet (); // вихід мікроконтролера в 1
//		}
//		else if (stateB == 0x71)
//		{
//			BReset (); // вихід мікроконтролера в 0	
//		}
		
		/* Формування стану виходів мікроконтролера для керування станом кнопок LIGHT ON, WATER F та WATER S для контролеру гідроблоку А120М*/	
		
		/*Стан кнопки LIGHT ON / СВІТиЛЬНиК*/
		if (stateLightOn == 0x81)
		{
			LightONSet (); // вихід мікроконтролера в 1
		}
		else if (stateLightOn == 0x84)
		{
			LightONReset (); // вихід мікроконтролера в 0
		}	
		
		/*Стан кнопки WATER F / НАПОВНЕННЯ СКЛЯНКи*/
		if (stateWaterF == 0x82)
		{
			WaterFSet (); // вихід мікроконтролера в 1
		}
		else if (stateWaterF == 0x85)
		{
			WaterFReset (); // вихід мікроконтролера в 0
		}	
		
		/*Стан кнопки WATER S / ЗМиВ ПЛЮВАЛЬНиЦІ*/
		if (stateWaterS == 0x83)
		{
			WaterSSet (); // вихід мікроконтролера в 1
		}
		else if (stateWaterS == 0x86)
		{
			WaterSReset (); // вихід мікроконтролера в 0
		}						
  }
  else if (dataChairCalcCRC != dataChairRxCRC || dataChairRxCRC == 0x00) // виконується якщо виникли проблеми на лінії - частина даних втрачена
	{
		dataChairAnswerBuffTx [0] = 0x16;
		dataChairAnswerBuffTx [1] = 0x06;
		dataChairAnswerBuffTx [2] = 0x00; // в посилці надсилати 0x00
		dataChairAnswerBuffTx [3] = 0x00; // в посилці надсилати 0x00
		dataChairAnswerBuffTx [4] = 0xFF;		
		data_chair_NOT_OK = 1; // перехід до стану, коли світлодіод мигає із фіксованою частотою
		CAN_status_OK = 0; // світлодіод, що сигналізує про передачу даних по CAN - мигає із фіксованою частотою
	}
}
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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
	uint32_t timCanRxDataChairNOTOK = 0; // таймер, який призначений для індикації помилки по CAN
	uint32_t tim_data_chair_ok = 0; // таймер, який використовується для відліку часу роботи із світлодіодом	
	uint32_t time_error_LED = 0; // таймер, який використовується для мигання свтлодіодом при помилці по CAN - в буфері data_chair_answer crc заповнено 0
	uint32_t tim_data_chair_answer = 0; // таймер, який використовується для відліку часу по передачі буфера data_chair_answer
	uint32_t tim_CAN_NOT_OK = 0; // таймер, який використовується для відліку часу на прийом першої посилки data_chair
	uint32_t tim_CAN_error = 0; //таймер для відліку 3с на початку роботи системи
	uint32_t timeRemove = 0; // відлік часу для обнулення вхідного буфера по CAN
	uint32_t time_button = 0;// таймер для опитування кнопок
	uint32_t time_pedal_on = 0; // таймер для перезаписування буфера про стан кнопки
 
 // Ініціалізація CAN
 HAL_CAN_Init (&hcan);
 
 	// Фільтр по прийому Rx для CAN
 CAN_FilterTypeDef canFilterConfig;
 canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
 canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
 canFilterConfig.FilterIdHigh = 0x0000;
 canFilterConfig.FilterIdLow = 0x0000;
 canFilterConfig.FilterMaskIdHigh = 0x0000<<5;
 canFilterConfig.FilterMaskIdLow = 0x0000;
 canFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
 canFilterConfig.FilterActivation = ENABLE;
 canFilterConfig.FilterBank = 1;
 HAL_CAN_ConfigFilter (&hcan, &canFilterConfig);
 HAL_CAN_Start (&hcan);
 
 // Rx вхідний буфер
 RxHeader.StdId = 0x00;
 RxHeader.RTR = CAN_RTR_DATA;
 RxHeader.IDE = CAN_ID_STD;
 RxHeader.DLC = 8;
 HAL_Delay(10);	
 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		// Створення віддаленого репозиторію 21.01.2025
			/* Прийом даних data_chair*/
		HAL_CAN_GetRxMessage (&hcan, CAN_RX_FIFO0, &RxHeader, canRxBuffer); 
	  if (RxHeader.StdId == 0x51) // прийом буфера data_chair - 1 частина посилки
	  {
	    for (len=0; len<8; len++)
			dataChairBuffRx [len] = canRxBuffer [len];  // перезапис даних із mailbox у буфер dataChairBuffRx (1 частина посилки), якщо це посилка з потрібним Id
	  }
	  else if (RxHeader.StdId == 0x52) // прийом буфера data_chair - 2 частина посилки
	  {
	    for (len=0; len<8; len++)
	    dataChairBuffRx [len+8] = canRxBuffer [len]; // перезапис даних із mailbox у буфер dataChairBuffRx (2 частина посилки), якщо це посилка з потрібним Id	
			state_data_chair = 1; // прапорець, який використовується для переходу до формування буфера data_chair_answer	
			decodeDataChairBuffRx (dataChairBuffRx);	// зверненння до функції обробки буфера data_chair	
    }
		if (RxHeader.StdId == 0x81) // прийом буфера data_pedal_on_off
		{
	    for (len=0; len<8; len++)
			dataPedalONOFF [len] = canRxBuffer [len];  // перезапис даних із mailbox у буфер data_pedal_on_off, якщо це посилка з потрібним Id
			flag_rx_a025 = 1;	
		}


if (HAL_GetTick () - tim_CAN_NOT_OK >= 100) // таймер, який відраховує 3с від ввімкнення живлення, якщо немає прийому даних data_chair
{	
	tim_CAN_NOT_OK = HAL_GetTick (); // для відліку часу використано системний таймер
	if (state_data_chair == 0) // посилка data_chair не отримана - помилка з'єднання
	{
		tim_CAN_error ++;  // початук відліку часу для оцінки роботи CAN
		if (tim_CAN_error >= 30) // відраховуємо 3с
		{
			data_chair_NOT_OK = 1; // помилка при прийомі даних
		}
	}
  if (state_data_chair == 1) // посилка data_chair отримана
  {
		tim_CAN_error = 0; // обнулення таймера, який відраховує 3с
	}
}	
		
if (HAL_GetTick () - tim_data_chair_ok >= 100) // обновлення стану світлодіодів відбувається кожні 100мс
{
	tim_data_chair_ok = HAL_GetTick(); // для відліку часу використано системний таймер
	timeRemove++;
	
	if (data_chair_OK == 1 && state_data_chair == 1) // якщо отримано буфер data_chair та/або відправлено змінну data_chair_answer
	{
		ErrorLEDReset (); // ввімкнення світлодіода Error - вимкнено - є передача даних по CAN 
	}
	
	if (data_chair_NOT_OK == 1) // втрата даних / помилка на лінії CAN - в буфері data_chair_answer crc заповнено 0
	{
		timCanRxDataChairNOTOK++;
		if (timCanRxDataChairNOTOK >= 30)
		{
		data_chair_OK = 0;
		time_error_LED ++; // запускаємо таймер, який починає відлік часу, який відведено на прийом / передачу даних
		if (time_error_LED  < 5) // частота мигання світлодіода - 500мс
		{
			ErrorLEDSet (); // ввімкнення світлодіода, що сигналізує про роботу CAN
  	}
		else if (time_error_LED < 10) // частота мигання світлодіода - 500мс
		{
			ErrorLEDReset (); // вимиканення світлодіода, що сигналізує про роботу CAN
		}
		else
		{
			time_error_LED = 0; // після одного циклу мигання обнулення таймера для відліку часу
		}
	 }
	}
	
	if (CAN_status_OK == 1)
	{
		StatusLEDSet (); // світлодіод Status LED ввімкнуто
	}
	else if (CAN_status_OK == 0)
	{
		timeStatus++;
		if (timeStatus == 5)
		{
			StatusLEDReset (); // світлодіод Status LED вимкнуто
		}
	}	
}
	

if (HAL_GetTick () - time_button >= 50)
{
	time_button = HAL_GetTick ();
	
	//Прийом даних з АЦП / ADC1
	HAL_ADC_Start (&hadc1); // старт АЦП
	HAL_ADC_PollForConversion (&hadc1, 100); // АЦП запускає перетворення даних кожні 100мс
	adcResult = HAL_ADC_GetValue (&hadc1); // дані, які отримано з АЦП 12-бітного
	dataResult = adcResult >> 4;  // дані, які приведено до АЦП 8-бітного
	HAL_ADC_Stop (&hadc1);	// зупинка АЦП
	
	
	if (flag_rx_a025 == 1) // обробка стану педалі по прийому даних з А025
	{
		if (dataPedalONOFF [2] == 0x80)
		{
			CoagReset ();
		}
		else if (dataPedalONOFF [2] == 0x81)
		{
			CoagSet ();
		}		
	}
	 
	else if (flag_rx_a025 == 0) // обробка стану педалі по обробці даних з CAN контролера крісла
	{
	//Стан кнопок для керування роботою багатофункційної педалі - Pedal_ON/OFF, CHIP та SPRAY
	if (readButtonPedalONOFF () == 1) // стан кнопки Pedal_ON/OFF / Педаль_ВКЛ./ВиКЛ.
	{
		button_push_pedal = 1;
	}
	else
	{
		button_push_pedal = 0;
	}
		
	if (button_push_pedal == 0)
		{
			button_push_pedal_sent = 0x73; // функція Pedal_ON/OFF / Педаль_ВКЛ./ВиКЛ. - вимкнута OFF
			CoagReset ();
		}
	else if (button_push_pedal == 1)
		{
		if (HAL_GetTick () - time_pedal_on >= 50)
		{
			time_pedal_on = HAL_GetTick ();
			button_push_pedal_sent = 0x71; // функція Pedal_ON/OFF / Педаль_ВКЛ./ВиКЛ. - ввімкнута ON
			CoagSet ();
		}
		}	
	}
	
	if (readButtonCHIP () == 1) // стан кнопки CHIP / ПРОДУВКА
	{
		button_push_CHIP = 1;
	}
	else
	{
		button_push_CHIP = 0;
	}
	if (button_push_CHIP == 0)
	{
		button_push_CHIP_sent = 0x74; // функція CHIP / ПРОДУВКА - вимкнута OFF
	}
	else if (button_push_CHIP == 1)
	{
		button_push_CHIP_sent = 0x72; // функція Pedal_ON/OFF / Педаль_ВКЛ./ВиКЛ. - ввімкнута ON
	}	
	

	if (readButtonSPRAY () == 1) // стан кнопки SPRAY / СПРЕЙ
	{
		if (stateButtSPRAY == 0)
		{
			if (butto_push_SPRAY < 1)
			{
				butto_push_SPRAY ++;
			}
			else if (butto_push_SPRAY > 0)
			{
				butto_push_SPRAY --;
			}
		}
		stateButtSPRAY = 1;	
	}
	else
	{
		stateButtSPRAY = 0;
	}

	if (butto_push_SPRAY == 0)
	{
		button_push_SPRAY_sent = 0x2D; // функція SPRAY / СПРЕЙ вимкнута OFF
	}
	else if (butto_push_SPRAY == 1)
	{
		button_push_SPRAY_sent = 0x2B; // функція SPRAY / СПРЕЙ ввімкнута ON	
	}
				
	if (button_push_pedal_sent == 0x71)
	{
		// Формування буфера data_pedal - стан педалі	
		dataPedalBuffTx [0] = 0x16;
		dataPedalBuffTx [1] = 0x07;
		dataPedalBuffTx [2] = 0x71;
		dataPedalBuffTx [3] = dataResult;
		dataPedalBuffTx [4] = button_push_CHIP_sent;
		dataPedalBuffTx [5] = 0x00;
		dataPedalBuffTx [6] = 0x00;
		dataPedalBuffTx [7] = 0xFF;	
		}
	else if (button_push_pedal_sent == 0x73)
	{
		// Формування буфера data_pedal - стан педалі	
		dataPedalBuffTx [0] = 0x16;
		dataPedalBuffTx [1] = 0x07;
		dataPedalBuffTx [2] = 0x73;
		dataPedalBuffTx [3] = 0x00;
		dataPedalBuffTx [4] = button_push_CHIP_sent;
		dataPedalBuffTx [5] = 0x00;
		dataPedalBuffTx [6] = 0x00;
		dataPedalBuffTx [7] = 0xFF;
	}
}

	if (HAL_GetTick () - tim_data_chair_answer >= 100) // передача даних кожні 100мс
	{
		tim_data_chair_answer = HAL_GetTick (); // для розрахунку часу використано системний таймер

	if (button_push_pedal_sent == 0x71 && flag_rx_a025 != 1)
	{		
			len_pedal = 3;
			for (len_pedal = 3; len_pedal > 0; len_pedal --)
			{
			// Передача буфера data_pedal				
			TxHeader.StdId = 0x71;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 8;
			TxHeader.ExtId = 0;

			canTxBuffer[0] = dataPedalBuffTx [0];
			canTxBuffer[1] = dataPedalBuffTx [1];
			canTxBuffer[2] = dataPedalBuffTx [2];
			canTxBuffer[3] = dataPedalBuffTx [3];
			canTxBuffer[4] = dataPedalBuffTx [4];
			canTxBuffer[5] = dataPedalBuffTx [5];
			canTxBuffer[6] = dataPedalBuffTx [6];
			canTxBuffer[7] = dataPedalBuffTx [7];	
			
			HAL_CAN_AddTxMessage (&hcan, &TxHeader, canTxBuffer, &TxMailBox);			
			HAL_Delay (10);				
		}
	}
	
	if (button_push_pedal_sent == 0x73 && flag_rx_a025 != 1)
	{		
			len_pedal = 3;
			for (len_pedal = 3; len_pedal > 0; len_pedal --)
			{
			// Передача буфера data_pedal				
			TxHeader.StdId = 0x71;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 8;
			TxHeader.ExtId = 0;

			canTxBuffer[0] = dataPedalBuffTx [0];
			canTxBuffer[1] = dataPedalBuffTx [1];
			canTxBuffer[2] = dataPedalBuffTx [2];
			canTxBuffer[3] = dataPedalBuffTx [3];
			canTxBuffer[4] = dataPedalBuffTx [4];
			canTxBuffer[5] = dataPedalBuffTx [5];
			canTxBuffer[6] = dataPedalBuffTx [6];
			canTxBuffer[7] = dataPedalBuffTx [7];	
			
			HAL_CAN_AddTxMessage (&hcan, &TxHeader, canTxBuffer, &TxMailBox);			
			HAL_Delay (10);				
		}
	}		
}							
	
	if (HAL_GetTick () - tim_data_answer >= 100) // передача даних кожні 100мс
	{
		tim_data_answer = HAL_GetTick (); // для розрахунку часу використано системний таймер
	
		
		if (state_data_chair == 1)
		{
			// Передача буфера data_chair_answer				
			TxHeader.StdId = 0x61;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 8;
			TxHeader.ExtId = 0;

			canTxBuffer[0] = dataChairAnswerBuffTx [0];
			canTxBuffer[1] = dataChairAnswerBuffTx [1];
			canTxBuffer[2] = dataChairAnswerBuffTx [2];
			canTxBuffer[3] = dataChairAnswerBuffTx [3];
			canTxBuffer[4] = dataChairAnswerBuffTx [4];
			canTxBuffer[5] = 0x00;
			canTxBuffer[6] = 0x00;
			canTxBuffer[7] = 0x00;	
			
			HAL_CAN_AddTxMessage (&hcan, &TxHeader, canTxBuffer, &TxMailBox);			
			HAL_Delay (10);				
		}
		
			TxHeader.StdId = 0x99;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 8;
			TxHeader.ExtId = 0;

			canTxBuffer[0] = 0x16;
			canTxBuffer[1] = 0x99;
			canTxBuffer[2] = button_push_SPRAY_sent;
			canTxBuffer[3] = 0x00;
			canTxBuffer[4] = 0x00;
			canTxBuffer[5] = 0x00;
			canTxBuffer[6] = 0x00;
			canTxBuffer[7] = 0xFF;	
			
			HAL_CAN_AddTxMessage (&hcan, &TxHeader, canTxBuffer, &TxMailBox);			
			HAL_Delay (10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 5;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CHAIR_DOWN_Pin|CHAIR_UP_Pin|BACKREST_UP_Pin|BACKREST_DOWN_Pin
                          |LIGHT_ON_Pin|WATER_S_Pin|WATER_F_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPITTOON_Pin|A_Pin|B_Pin|COAG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Open_Collector_1_Pin|Open_Collector_2_Pin|Code_LED_Pin|Status_LED_Pin
                          |Error_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Coag_init_Pin */
  GPIO_InitStruct.Pin = Coag_init_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Coag_init_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CHAIR_DOWN_Pin CHAIR_UP_Pin BACKREST_UP_Pin BACKREST_DOWN_Pin
                           LIGHT_ON_Pin WATER_S_Pin WATER_F_Pin */
  GPIO_InitStruct.Pin = CHAIR_DOWN_Pin|CHAIR_UP_Pin|BACKREST_UP_Pin|BACKREST_DOWN_Pin
                          |LIGHT_ON_Pin|WATER_S_Pin|WATER_F_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SPITTOON_Pin A_Pin B_Pin COAG_Pin */
  GPIO_InitStruct.Pin = SPITTOON_Pin|A_Pin|B_Pin|COAG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Pedal_ON_OFF_Pin */
  GPIO_InitStruct.Pin = Pedal_ON_OFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Pedal_ON_OFF_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CHIP_Pin SPRAY_Pin */
  GPIO_InitStruct.Pin = CHIP_Pin|SPRAY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Open_Collector_1_Pin Open_Collector_2_Pin Code_LED_Pin Status_LED_Pin
                           Error_LED_Pin */
  GPIO_InitStruct.Pin = Open_Collector_1_Pin|Open_Collector_2_Pin|Code_LED_Pin|Status_LED_Pin
                          |Error_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
