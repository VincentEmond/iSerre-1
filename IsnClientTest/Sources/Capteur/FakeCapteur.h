/*
 * FakeCapteur.h
 *
 *  Created on: 2017-03-13
 *      Author: iSerre
 */

#ifndef SOURCES_CAPTEUR_FAKECAPTEUR_H_
#define SOURCES_CAPTEUR_FAKECAPTEUR_H_

#include "..\iSN\mqUtil.h"
#include "..\iSN\iSN.h"

class FakeCapteur
{
	public:
		FakeCapteur(IsnClient* client);
		void exec();
	private:
		float _mesure;
		XTimer _samplingTimer;
		int _samplingRate;
		IsnClient* _client;
		bool _initTimer;
};



#endif /* SOURCES_CAPTEUR_FAKECAPTEUR_H_ */
