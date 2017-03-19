/*
 * FakeCapteur.cpp
 *
 *  Created on: 2017-03-13
 *      Author: iSerre
 */
#include "FakeCapteur.h"


FakeCapteur::~FakeCapteur() {}
CommonSensor::~CommonSensor() {}

FakeCapteur::FakeCapteur(IsnClient* client)
{
	_mesure = 13.37;
	_client = client;
}

void FakeCapteur::init_sensor()
{

}

void FakeCapteur::read_sensor(float* val)
{
	*val = _mesure;
}

void FakeCapteur::exec()
{

}


