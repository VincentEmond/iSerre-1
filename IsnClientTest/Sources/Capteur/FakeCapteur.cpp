/*
 * FakeCapteur.cpp
 *
 *  Created on: 2017-03-13
 *      Author: iSerre
 */
#include "FakeCapteur.h"


FakeCapteur::FakeCapteur(IsnClient* client)
{
	_mesure = 13.37;
	_samplingRate = 30;
	_client = client;
	_initTimer = false;
}

void FakeCapteur::exec()
{
	if(!_initTimer)
	{
		_samplingTimer.start(_samplingRate * 1000);
		_initTimer = true;
	}

	if (_samplingTimer.isTimeUp())
	{
		_client->sendMeasure(_mesure);
		_samplingTimer.start(_samplingRate * 1000);
	}
}


