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

class Capteur
{
public:
	virtual void setConfig(IsnConfiguration* conf) = 0;
};

class FakeCapteur : public Capteur
{
	public:
		FakeCapteur(IsnClient* client);
		void exec();
		void setConfig(IsnConfiguration* conf);
	private:
		float _mesure;
		XTimer _samplingTimer;
		int _samplingRate;
		IsnClient* _client;
		IsnConfigurationTemperature _config;
		bool _initTimer;
};



#endif /* SOURCES_CAPTEUR_FAKECAPTEUR_H_ */
