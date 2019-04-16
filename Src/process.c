#include "process.h"
#include "at_ublox_c.h"
#include <string.h>

uint16_t xout = 0;
uint16_t yout = 0;
uint16_t zout = 0;
uint16_t latitude = 0;
uint16_t longitude = 0;
uint16_t altitude = 0;
uint8_t adcState;

uint8_t blinkMessage[500];
uint16_t n = 0;

extern RTC_HandleTypeDef hrtc;
extern ADC_HandleTypeDef hadc;
extern UART_HandleTypeDef huart2;

HAL_StatusTypeDef writeEEPROMByte(uint32_t address, uint8_t data)
 {
    HAL_StatusTypeDef  status;
    address = address + 0x08080000;
    HAL_FLASHEx_DATAEEPROM_Unlock();  //Unprotect the EEPROM to allow writing
    status = HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_BYTE, address, data);
    HAL_FLASHEx_DATAEEPROM_Lock();  // Reprotect the EEPROM

    return status;
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC))
    {
        uint16_t temp = HAL_ADC_GetValue(hadc);

        if (adcState == MEASURE_X)
        {
            xout = temp;
        }
        else if (adcState == MEASURE_Y)
        {
			yout = temp;
        }
        else if (adcState == MEASURE_Z)
        {
			zout = temp;
        }
    }
}


void measureAccelerometer()
{
 	ADC_ChannelConfTypeDef sConfig;

	// Measure x value of the accelerometer
	adcState = MEASURE_X;
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
	HAL_ADC_Start_IT(&hadc);
	HAL_Delay(10);

	// Measure y value of the accelerometer
	adcState = MEASURE_Y;
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
	HAL_ADC_Start_IT(&hadc);
	HAL_Delay(10);

	// Measure z value of the accelerometer
	adcState = MEASURE_Z;
	sConfig.Channel = ADC_CHANNEL_6;
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
	HAL_ADC_Start_IT(&hadc);
	HAL_Delay(10);
}

void measureGPSLocation()
{
	char pResp[100] = "";
	char _temp[50];

	uint8_t _term_number = 0, _term_offset = 0;
	// 69 is bitmask for combining three GNSS types:
	// 1 is GPS, 4 is Galileo, 64 is GLONASS => 1|4|64 = 69
	GNSSSetPower(1, 0, AT_UGPS_ON_PARA, pResp);

	for (int i = 0; i < 100; i ++)
	{
		switch (pResp[i])
		{
		case ',':
			break;
		case '\r':
		case '\n':
		case '*':
			if (_term_offset < strlen(pResp))
			{
				strncpy(_temp, &(pResp[i]), 13);
				latitude = parse_degrees(_temp);
				strncpy(_temp, &(pResp[i + 13]), 13);
				longitude = parse_degrees(_temp);
				strncpy(_temp, &(pResp[i + 26]), 13);
				altitude = parse_degrees(_temp);
				return;
			}
		case '$':
			_term_number = _term_offset = 0;
			return;

		}
		_term_offset ++;
	}
}

RTC_AlarmTypeDef sAlarm;
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

void BootSIM7000()
{
	// Boot Radio
    n = sprintf((char*)&blinkMessage[0], "=>Booting BG96...\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_Delay(5000);

	// If the status pin is high, turn on the green led
	if (HAL_GPIO_ReadPin(RAD_UC_STATUS_GPIO_Port, RAD_UC_STATUS_Pin) == GPIO_PIN_SET)
	{
    	HAL_GPIO_WritePin(UC_BLUE_LED_GPIO_Port, UC_BLUE_LED_Pin, GPIO_PIN_SET);
		n = sprintf((char*)&blinkMessage[0], "=>SIM7000 is booted.\n");
		CDC_Transmit_FS(blinkMessage, n);
	}

	// Reset the SIM7000
	//HAL_GPIO_WritePin(UC_RAD_RSTN_GPIO_Port, UC_RAD_RSTN_Pin, GPIO_PIN_SET);
	//HAL_Delay(300);
	//HAL_GPIO_WritePin(UC_RAD_RSTN_GPIO_Port, UC_RAD_RSTN_Pin, GPIO_PIN_RESET);
}

void ShutdownSIM7000()
{
    n = sprintf((char*)&blinkMessage[0], "=>Powering Down SIM7000...\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(500);
	n = sprintf((char*)&blinkMessage[0], "AT+QPOWD\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(2000);
}

int connectNetwork()
{
    n = sprintf((char*)&blinkMessage[0], "=>Connect to network.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	// Prevent echo
	n = sprintf((char*)&blinkMessage[0], "ATE0\r\n");
	__HAL_UART_CLEAR_IT(&huart2, UART_CLEAR_NEF|UART_CLEAR_OREF);
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(2000);

	 CDC_Transmit_FS(receivedMessage, 20);

	// Check SIM card
	n = sprintf((char*)&blinkMessage[0], "AT+CPIN?\r\n");
	__HAL_UART_CLEAR_IT(&huart2, UART_CLEAR_NEF|UART_CLEAR_OREF);
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	 CDC_Transmit_FS(receivedMessage, 20);

	if (strstr((char*)receivedMessage, "READY") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "SIM card Ready!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "SIM card Not Ready!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	// Set full functionality
	n = sprintf((char*)&blinkMessage[0], "AT+CFUN=1\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set full functionality!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	/*
	// Set full functionality
	n = sprintf((char*)&blinkMessage[0], "AT+COPS?\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(5000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "Verizon") != NULL || strstr((char*)receivedMessage, "MLTNetwork") != NULL )
	{
		n = sprintf((char*)&blinkMessage[0], "Found Operator!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "Not Found Operator!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}
	*/

	// Set IoT mode
	n = sprintf((char*)&blinkMessage[0], "AT+QCFG=\"iotopmode\",0,1\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set IoT mode!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "Set IoT mode Failed!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	return 1;
}



void sendDataMQTT()
{
	// Set Will to 0
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCFG=\"will\",0\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set Will to 0!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Set Timeout to 60
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCFG=\"timeout\",0,60,3,0\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set Timeout to 60!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Set Keep alive to 60
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCFG=\"keepalive\",0,60\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set keep alive to 60!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Open network
	//n = sprintf((char*)&blinkMessage[0], "at+qmtopen=0,\"ec2-54-226-136-203.compute-1.amazonaws.com\",1883\r\n");
	// 3.210.5.128
	n = sprintf((char*)&blinkMessage[0], "at+qmtopen=0,\"demo.traccar.org\",1883\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(3000);

	if (strstr((char*)receivedMessage, "+QMTOPEN: 0,0") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Open Network!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Connect network
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCONN=0,\"clientExample\"\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "+QMTCONN: 0,0,0") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Connect to a network!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}


	// Publishing Sensor Data
	n = sprintf((char*)&blinkMessage[0], "AT+QMTPUB=0,0,0,0,\"william/topic1\"\r\n");
	__HAL_UART_CLEAR_IT(&huart2, UART_CLEAR_NEF|UART_CLEAR_OREF);
	// HAL_UART_Receive_IT(&hlpuart1, receiveData, 500);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(500);

    // Upload Stored Sensor Data, Status, Battery Voltage
    n = sprintf((char*)&blinkMessage[0], "=>Uploading stored sensor data, status and battery voltage.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	uint8_t i = 0;
	n = sprintf((char*)&blinkMessage[0], "SP %d %d %d EP", latitude, longitude, altitude);

    CDC_Transmit_FS(blinkMessage, n);
	end = 0;


	// n = sprintf((char*)&blinkMessage[0], "this is test");
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(500);
	uint8_t ctrl_z = 0x1A;
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, &ctrl_z, 1, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "+QMTPUB: 0,0,0") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "publish data to a server!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Disconnect network
	n = sprintf((char*)&blinkMessage[0], "AT+QMTDISC=0\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&huart2, receivedMessage, 200);
	HAL_UART_Transmit(&huart2, blinkMessage, n, 1000);
	HAL_Delay(1000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Disconnect from a network!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}
	return 1;

}

void sendDataSMS()
{
}

void Process()
{
	measureAccelerometer();

	// Decide the car is moving or not
	// If moving, send the data, and if not, not send the data
	if (xout > 100 || yout > 100)
	{
		// Get the GPS location data from UBX-G7020
		measureGPSLocation();

		// Booting the SIM7000
		BootSIM7000();

		if (connectNetwork() == 1)
		{
			// Download Configuration Update
			n = sprintf((char*)&blinkMessage[0], "=>Sending sensor data\r\n");
			CDC_Transmit_FS(blinkMessage, n);

			// Sending the data via MQTT
			sendDataMQTT();
			// sendDataSMS();
		}
		else
		{
			n = sprintf((char*)&blinkMessage[0], "=>Failed to connect to the network\r\n");
			CDC_Transmit_FS(blinkMessage, n);
		}

		ShutdownSIM7000();
	}


	// Sleep the MCU
	uint8_t alarmMinute = 0;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);

	// Read the interval in EEPROM
	uint32_t address;
	uint32_t interval;
	address = EEPROM_CONFIG;
	HAL_FLASHEx_DATAEEPROM_Unlock();  //Unprotect the EEPROM to allow writing
	interval = *((uint32_t *) address);
    HAL_FLASHEx_DATAEEPROM_Lock();  // Reprotect the EEPROM

	if (interval == 0)
		interval = 5;

	alarmMinute = sTime.Minutes + interval;
	// if alarmHour is larger than 23, then set to 0 and increase the weekday
	if (alarmMinute > 59)
	{
		alarmMinute -= 60;
		if (sTime.Hours == 23)
		{
			sAlarm.AlarmTime.Hours = 0;
			sAlarm.AlarmDateWeekDay = sDate.WeekDay + 1;
			if (sAlarm.AlarmDateWeekDay > RTC_WEEKDAY_SUNDAY)
				sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
		}
		else
			sAlarm.AlarmTime.Hours = sTime.Hours + 1;
	}
	else
	{
		sAlarm.AlarmTime.Hours = sTime.Hours;
		sAlarm.AlarmDateWeekDay = sDate.WeekDay;
	}

	// set the alarm time
	sAlarm.AlarmTime.Minutes = alarmMinute;
	sAlarm.AlarmTime.Seconds = sTime.Seconds;
	sAlarm.AlarmTime.SubSeconds = sTime.SubSeconds;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_PM;
	sAlarm.Alarm = RTC_ALARM_A;

	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);
	uint16_t n = 0;
	n = sprintf((char*)&blinkMessage[0], "=>Measurement:  %d Time:%d:%d   alarm:%d:%d.\r\n", sDate.WeekDay, sTime.Hours, sTime.Minutes, sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
    CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(500);
	__HAL_RCC_PWR_CLK_ENABLE(); 		// Enable Power Control clock
	HAL_PWREx_EnableUltraLowPower(); 	// Ultra low power mode
	HAL_PWREx_EnableFastWakeUp(); 		// Fast wake-up for ultra low power mode
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	HAL_PWR_EnterSTANDBYMode();
}
