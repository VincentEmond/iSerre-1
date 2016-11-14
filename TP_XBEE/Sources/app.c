#include "app.h"

/**
 * Renvoie le code hex correspondant pour un caractère ascii
 * entre '0' et '9', 'a' et 'f' ou 'A' et 'F'.
 */
int ascii_to_hex(uint8_t ascii)
{
	// Ascii entre '0' et '9'
	if (ascii >= 0x30 && ascii <= 0x39 )
	{
		return ascii - 0x30;
	}
	// Ascii entre 'A' et 'F'
	else if (ascii >= 0x41 && ascii <= 0x46)
	{
		return ascii - 0x37;
	}
	// Ascii entre 'a' et 'f'
	else if (ascii >= 0x61 && ascii <= 0x66)
	{
		return ascii - 0x57;
	}
	else {
		return ascii;
	}
}

/**
 * Envoie une trame contenant le message spécifié au xbee spécifié
 */
void envoi_message()
{
	unsigned char adresse_saisie[16];
	uint64_t adresse;

	char message[XB_PAYLOAD_MAX_SIZE];
	uint8_t nbre_carac = 0;

	do {
		nbre_carac = lire_chaine("Saisir adresse destination (16): " , adresse_saisie, 16);
		if(nbre_carac != 0x10) {
			debug_printf("Veuillez reessayer. \n");
		}
	}
	while (nbre_carac != 0x10);

	int val = 0;
	for(int i = 0; i < 16; i++)
	{
		val = ascii_to_hex(adresse_saisie[i]);
		adresse <<= 4;
	    adresse |= (uint64_t)(val);
	}

	nbre_carac = lire_chaine("Saisir message : ", message, XB_PAYLOAD_MAX_SIZE);

	sendFrame(adresse, message, nbre_carac);

	debug_printf("Trame envoyee. \n\n");

}

void diffuser_message()
{
	uint8_t nbre_carac = 0;
	uint8_t message[XB_PAYLOAD_MAX_SIZE];
	uint64_t adresse = 0xffff; // adresse de diffusion

	nbre_carac = lire_chaine("Saisir message : ", message, XB_PAYLOAD_MAX_SIZE);

	sendFrame(adresse, message, nbre_carac);

	debug_printf("Trame envoyee. \n\n");
}

void lire_contenu_fifo(struct Fifo *fifo, uint8_t *contenu_fifo)
{
	uint8_t byte = 0;

	while(fifo->begin != fifo->end)
	{
		lire_fifo_buffer(fifo, &byte);

		if(byte == XB_FRAME_START_DELIMITER)
		{
			contenu_fifo[0] = byte;
			lire_fifo_buffer(fifo, &byte);
			contenu_fifo[1] = byte;
			lire_fifo_buffer(fifo, &byte);
			contenu_fifo[2] = byte;
			for (int i = 0; i <= contenu_fifo[2]; i++)
			{
				lire_fifo_buffer(fifo, &byte);
				contenu_fifo[i+3] = byte;
			}
			break;
		}
	}
}

void afficher_derniere_reception()
{
	afficher_chaine("Lecture du buffer de reception.. ", 1);

	uint8_t trame_brute[XB_PAYLOAD_MAX_SIZE] = {0};

	// Recuperer le contenu du fifo_buffer -> la trame brute
	lire_contenu_fifo(&fifo_xbee, trame_brute);

	int data_length = 0;
	if(fifo_xbee.end > fifo_xbee.begin) {
		data_length = fifo_xbee.end - fifo_xbee.begin + 1;
	}
	else if (fifo_xbee.begin == fifo_xbee.end) {
		afficher_chaine("Buffer de reception vide. \n", 1);
		return;
	}
	else {
		data_length = FIFO_SIZE - (fifo_xbee.begin - fifo_xbee.end) + 1;
	}

	xbee_rx_frame trame_formee = createRxFrame(trame_brute, data_length);
	afficher_chaine("Derniere trame recue: ", 1);
	afficher_chaine("\tAdresse : ", 0);
	afficher_hex(trame_formee.rx_header.source_address, 1);
	afficher_chaine("\tMessage : ", 1);
	for(int i = 0; i < trame_formee.payload_size; i++) {
		debug_printf(trame_formee.payload[i]);
	}

	afficher_chaine("\n", 1);

}

void afficher_menu()
{
	afficher_chaine("Communication entre modules Xbee..", 1);
	afficher_chaine("1. Envoyer message", 1);
	afficher_chaine("2. Diffuser message", 1);
	afficher_chaine("3. Recevoir message", 1);
}
void lire_reponse()
{
	uint8_t reponse = 0;
	if(lire_chaine("Reponse ? ", &reponse, 1) == 0xff)
		return;

	switch((char)reponse) {
	case '1':
		envoi_message();
		break;
	case '2':
		diffuser_message();
		break;
	case '3':
		afficher_derniere_reception(); // A tester
		break;
	default:
		break;
	}
}

void app_run()
{
	init_fifo(&fifo_xbee);
	afficher_menu();
	lire_reponse();
}





