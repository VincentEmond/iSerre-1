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
	_client = client;
	_initTimer = false;
}

void FakeCapteur::setConfig(IsnConfiguration* conf)
{
	IsnConfigurationTemperature* confTemp = static_cast<IsnConfigurationTemperature*>(conf);
	_config = *confTemp;
	_initTimer = true;
}

void FakeCapteur::exec()
{
	if(!_initTimer)
	{
		_samplingTimer.start(_config.getSamplingRate() * 1000);
	}

	if (_samplingTimer.isTimeUp())
	{
		_client->sendMeasure(_mesure);
		_samplingTimer.start(_config.getSamplingRate() * 1000);
	}
}


