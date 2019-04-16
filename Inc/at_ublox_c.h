
#include "usb_device.h"
#include "usbd_cdc_if.h"

#ifndef __AT_UBLOX_C_H
#define __AT_UBLOX_C_H


#define NUM_SUPPORT_AT_CMD			(47)
#define MAX_MSG_LENGTH					(400)
#define MAX_GET_RESPONSE_LENGTH	(700)

#define AT_UGPS_ON_PARA						      (69)

typedef enum{
	AT_UBLOX_ATE = 0, 				// turn on turn off echo
	/* General commands */
	AT_UBLOX_CGMR = 1,        // read firmware version
	AT_UBLOX_CGSN = 2,        // read IMEI
	AT_UBLOX_CIMI = 29,       // read IMSI
	AT_UBLOX_CCID = 31,       // read CCID
	AT_UBLOX_COPS = 44,       // select operator
	AT_UBLOX_CREG = 45,       // network registration status
	/* SMS commands */
	AT_UBLOX_CSMS = 3,        // read supported message service, set to 1
	AT_UBLOX_CPMS = 4,        // set recv msg storage
	AT_UBLOX_CNMI = 5,				// set recv msg notify
	AT_UBLOX_CMGR = 6,        // read msg
	AT_UBLOX_CMGD = 7,        // delete msg
	AT_UBLOX_CMGF = 30,       // set text mode
	AT_UBLOX_CMGL = 32,       // list msg
	AT_UBLOX_CSDH = 46,       // config msg header
	/* GPIO commands */
	AT_UBLOX_UGPIOC = 8,      // GPIO config
	AT_UBLOX_UGPIOR = 9,      // read GPIO
	AT_UBLOX_UGPIOW = 10,     // set GPIO
	/* DNS commands */
	AT_UBLOX_UDNSRN = 11,
	AT_UBLOX_UDYNDNS = 12,
	/* GPRS commands */
	AT_UBLOX_CGATT = 33,      // check GPRS attachment
	AT_UBLOX_UPSD = 34,       // config APN
	AT_UBLOX_UPSDA = 35,      // set PDP context
	AT_UBLOX_USOCR = 36,      // set socket
	AT_UBLOX_USOCO = 37,      // open TCP
	AT_UBLOX_USOWR = 38,      // send data
	AT_UBLOX_USOCL = 39,			// close TCP
	/* HTTP commands */
	AT_UBLOX_UHTTP = 40,      // set HTTP parameters
	AT_UBLOX_UHTTPC = 41,     // send HTTP request
	/* Flash commands */
	AT_UBLOX_URDFILE = 42,    // read file stored in flash
	AT_UBLOX_UDELFILE = 43,   // delete file stored in flash
	/* GNSS commands */
	AT_UBLOX_UGPS = 13,       // GPS power management
	AT_UBLOX_UGIND = 14,      // URC mode
	AT_UBLOX_UGPRF = 15,      // config GNSS profile
	AT_UBLOX_UGUBX = 16,
	AT_UBLOX_UGTMR = 17,
	AT_UBLOX_UGZDA = 18,
	AT_UBLOX_UGGGA = 19,      // fix data
	AT_UBLOX_UGGLL = 20,      // geographic position
	AT_UBLOX_UGGSV = 21,      // number of GNSS satellites in view
	AT_UBLOX_UGRMC = 22,      // recommended minimum GNSS data
	AT_UBLOX_UGVTG = 23,      // course over ground and ground speed
	AT_UBLOX_UGGSA = 24,      // satellite information
	AT_UBLOX_ULOC = 25,       // config ULOC
	AT_UBLOX_UTIME = 26,
	AT_UBLOX_ULOCIND = 27,
	AT_UBLOX_ULOCGNSS = 28,   // config GNSS sensor
	MAX_AT_INDEX = NUM_SUPPORT_AT_CMD
} at_ublox_cmd_t;

typedef struct {
	char code[15];
	uint32_t timeout;
} at_cmd_info_t;


/* general commands */
/* revised */ uint8_t PingTest(char* pResp);
/* revised */ uint8_t SetEcho(uint8_t isEnable, char* pResp);
/* DUMMY implementation */ uint8_t ReadFirmware(char* pFirmVer, char* pResp);
/* revised */ uint8_t ReadIMEI(char* pIMEI, char* pResp);
/* revised */ uint8_t ReadIMSI(char* pIMSI, char* pResp);
/* revised */ uint8_t ReadSIMID(char* pSIMID, char* pResp);
uint8_t SetPowerMode(uint8_t powerMode, int32_t awakeTime, char* pResp);
uint8_t SwitchOff(char* pResp);

/* revised */ uint8_t SetOperator(uint8_t mode, char* pResp);
/* revised */ uint8_t ReadNetworkRegistrationStatus(uint8_t* stt, char* pResp);
/* revised */ uint8_t ReadNetworkOperator(char* pInfo, char* pResp);
/* test function, isn't used in app */ uint8_t TestATCommand(char* cmd, char* pResp);

/* SMS commands ..... */
/* revised */ uint8_t SMSSetTextMode(uint8_t isEnable, char* pResp);
/* revised */ uint8_t SMSSetShowHeader(uint8_t isEnable, char* pResp);
/* revised */ uint8_t SMSSelectService(uint8_t isPhase2Plus, char* pResp);
/* revised */ uint8_t SMSSetNewIndication(uint32_t indicateMode, char* pResp);
uint8_t SMSSetWaitingIndicaition(uint8_t isEnable, char* pResp);
/* revised */ uint8_t SMSSetPreferredStorage(uint8_t onSIM, char* pResp);
uint8_t SMSSaveSetting(char* pResp);
/* revised */  uint8_t SMSReadAt(uint8_t index, char* pSentNum, char* pContent, char* pResp);

  //uint8_t SMSReadNewest(char* pSentNum, char* pContent, char* pResp);
	uint8_t SMSReadOldest(uint8_t* hasSms, int32_t* pIndex, char* pSentNum, char* pContent, char* pResp);

	/* revised */ uint8_t SMSDeleteAt(int32_t index, char* pResp);
  /* revised */ uint8_t SMSDeleteAll(uint8_t includeUnread, char* pResp);
  //uint8_t SMSDeleteNewest(char* pResp);
	//uint8_t SMSDeleteOldest(char* pResp);

  /* GPIO commands */
  uint8_t GPIOSetMode(uint16_t pinIndex, uint16_t pinMode, char* pResp);
  uint8_t GPIORead(uint16_t pinIndex, uint8_t* pReadVal, char* pResp);
  uint8_t GPIOSet(uint16_t pinIndex, uint8_t setVal, char* pResp);

  /* DNS commands */
  uint8_t DNSResolveURL(char* pURL, char* pResolvedIP, char* pResp);
  uint8_t DNSSetDynamicUpdate(uint8_t isEnable, char* pResp);

	/* GPRS commands */
uint8_t GPRSSetAttach(uint8_t isAttach, char* pResp);
uint8_t GPRSIsAttach(uint8_t* pResult, char* pResp);

uint8_t GPRSSetAuthentication(uint8_t profileId, uint8_t type, char* pResp);
uint8_t GPRSSetAPN(uint8_t profileId, char* pAPN, char* pResp);
uint8_t GPRSSetUsername(uint8_t profileId, char* pUserName, char* pResp);
uint8_t GPRSSetPassword(uint8_t profileId, char* pPassword, char* pResp);


uint8_t GPRSSetPDP(uint8_t profileId, uint8_t isActive, char* pResp);

/* HTTP commands */
/* revised */ uint8_t HTTPResetParameters(uint8_t httpID, char* pResp);
/* revised */ uint8_t HTTPSetDomain(uint8_t httpID, char* domain, char* pResp);
/* revised */ uint8_t HTTPSetRemotePort(uint8_t httpID, uint32_t port, char* pResp);
/* revised */ uint8_t HTTPSetUsername(uint8_t httpID, char* pUsername, char* pResp);
/* revised */ uint8_t HTTPSetPassword(uint8_t httpID, char* pPassword, char* pResp);
/* revised */ uint8_t HTTPSetAuthentication(uint8_t httpID, uint8_t isEnable, char* pResp);
/* revised */ uint8_t HTTPSetCustomHeader(uint8_t httpID, uint8_t headerIdx, char* headerName, char* headerVal, char* pResp);
/* revised */ uint8_t HTTPGetRequest(uint8_t httpID, char* pUri, char* pSaveFileName, uint8_t* pResult, char* pResp);
/* dummy implement. need change later */ uint8_t HTTPParseResponse(char* fileName, char* pResp);


  /* GNSS commands */
uint8_t GNSSSetPower(uint8_t isOn, uint8_t aidMode, uint32_t GNSSSystem, char* pResp);
uint8_t GNSSSetURC(uint8_t isEnable, char* pResp);
uint8_t GNSSSetProfile(uint32_t code, char* pResp);
uint8_t GNSSSetSensor(void);
uint8_t GNSSConfigData(uint8_t sensor, uint8_t responseType, uint32_t timeout, uint16_t accuracy, char* pResp);
uint8_t GNSSSetFixData(uint8_t isEnable, char* pResp);
uint8_t GNSSGetFixData(char* pResp);
uint8_t GNSSSetGeoPos(uint8_t isEnable, char* pResp);
uint8_t GNSSGetGeoPos(char* pResp);
uint8_t GNSSSetNumOfSatellites(uint8_t isEnable, char* pResp);
uint8_t GNSSGetNumOfSatellites(char* pResp);
uint8_t GNSSSetRMC(uint8_t isEnable, char* pResp);
uint8_t GNSSGetRMC(char* pResp);
uint8_t GNSSSetVTG(uint8_t isEnable, char* pResp);
uint8_t GNSSGetVTG(char* pResp);
uint8_t GNSSSetSatelliteInfo(uint8_t isEnable, char* pResp);
uint8_t GNSSGetSatelliteInfo(char* pResp);

  uint8_t GNSSGetULOCData(uint8_t byGNSS, uint32_t timeOutInSec, uint32_t accuracyInMet, char* pData, char* pResp);

  /* general read commands */
  uint8_t Read(at_ublox_cmd_t readPara, uint32_t timeOut, char* pResp);
  uint8_t Read(at_ublox_cmd_t readPara, char* pWait, uint32_t timeOut, char* pResp);

  uint8_t Read(char* pATCmd, uint32_t timeOut, char* pWait, char* pResp);
  uint8_t ReadNoQuestionMark(char* pATCmd, uint32_t timeOut, char* pWait, char* pResp);


#endif /*  __AT_UBLOX_C_H */

