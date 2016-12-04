/*
 * Menu.h
 *
 *  Created on: 4 oct. 2016
 *      Author: Julien
 */

#ifndef SOURCES_MENU_H_
#define SOURCES_MENU_H_

#include "stdint.h"
#include "stdlib.h"

#define MENU_MAX_ITEM_NAME_SIZE 30U
#define MENU_MAX_ITEM_COUNT 5U

typedef void(*func_ptr)(void);

void Menu_init(char* menu_string);

void Menu_add_item(char* item_name, func_ptr function);

void Menu_print(void);

void Menu_choose(uint8_t item_number);

#endif /* SOURCES_MENU_H_ */
