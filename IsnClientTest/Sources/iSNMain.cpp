/*
 * iSNMain.c
 *
 *  Created on: 2017-03-11
 *      Author: iSerre
 */
#define KINETIS
#include "iSNMain.h"
#include "iSN\iSN.h"
#include "uart_xb.h"
#include "iSN\ZbeeStack.h"
#include "Capteur\FakeCapteur.h"


using namespace tomyClient;

void iSNMain()
{

	XBeeConfig theAppConfig = {
			57600,          //Baudrate (bps)
			0,              //Serial PortNo
			0               //Device (for linux App)
	};

	Network net(UART2_IDX, &uart_xb_InitConfig0, &uart_xb_State);
	net.initialize(theAppConfig);

	IsnClient client(&net, ISN_SENSOR_TEMP);
	FakeCapteur capteur(&client);
	client.setSensor(&capteur);

	while (true)
	{
		client.exec();
		capteur.exec();
	}
}



