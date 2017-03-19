/*
 * FakeCapteur.h
 *
 *  Created on: 2017-03-13
 *      Author: iSerre
 */

#ifndef SOURCES_CAPTEUR_FAKECAPTEUR_H_
#define SOURCES_CAPTEUR_FAKECAPTEUR_H_

#include "iSN\mqUtil.h"
#include "iSN\iSN.h"

class CommonSensor
{
public:
	virtual void init_sensor() = 0;
	virtual void read_sensor(float* val) = 0;
	virtual ~CommonSensor();
};

class FakeCapteur : public CommonSensor
{
	public:
		FakeCapteur(IsnClient* client);
		~FakeCapteur();
		void exec();
		void init_sensor();
		void read_sensor(float* val);
	private:
		float _mesure;
		IsnClient* _client;
};



#endif /* SOURCES_CAPTEUR_FAKECAPTEUR_H_ */
