/*
 * console.h
 *
 *  Created on: Nov 8, 2016
 *      Author: cheik
 */

#ifndef SOURCES_CONSOLE_H_
#define SOURCES_CONSOLE_H_

#include "DbgCs1.h"


int echo_true = 1;

void afficher_chaine(char* chaine, int retourLigne);
void afficher_chaine_buffer(char *buffer, uint8_t buffer_length, int retourLigne);
void afficher_hex(char *chaine, int retourLigne);
void afficher_entier(int entier);

uint8_t lire_chaine(char *question, char *input_buffer, uint8_t max);
char lire_caratere();
int lire_entier();




#endif /* SOURCES_CONSOLE_H_ */
