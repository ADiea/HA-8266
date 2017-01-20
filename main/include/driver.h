#ifndef __DRIVER_H_
#define __DRIVER_H_

#include "Wiring/WVector.h"
#include <SmingCore/SmingCore.h>

enum eDriverError
{
	drvErrOK = 0,
	drvErrIO = 1,
	drvErrParam = 2,
	drvErrMalloc = 3,
	drvErrBus = 4,
	drvErrTimeout = 5,
	drvErrCRC = 6,
	drvErrNullDevice = 7,
	drvErrOther = 99
};

enum eDriverOp
{
	drvEnable =  0,
	drvDisable =  1,
};

enum eDriverState
{
	drvDisabled = 0,
	drvEnabled = 1,
	drvBusy = 2,
};

class CGenericDriver
{
public:
	CGenericDriver():m_State(drvDisabled), m_lastError(drvErrOK){};

	virtual eDriverError setup(eDriverOp op = drvEnable) = 0;
	virtual eDriverState getState(){return m_State;}
	virtual bool isBusy(){return (m_State == drvBusy) ;}
	virtual bool isEnabled(){return (m_State == drvEnabled || m_State == drvBusy) ;}

	eDriverError getLastError() const {return m_lastError;}

	virtual ~CGenericDriver(){};

protected:
	eDriverState m_State;
	eDriverError m_lastError;
};

#include "drv/drvBusMaster.h"
#include "drv/drvIO.h"
#include "drv/drvRadio.h"
#include "drv/drvSDCard.h"
#include "drv/drvWiFi.h"
#include "drv/drvSHT21.h"
#include "drv/drvGesture.h"
#include "drv/drvHaptic.h"
#include "drv/drvLCD.h"

#endif /*__DRIVER_H_*/
