/* IO.h*/
/* Includes ------------------------------------------------------------------*/
#ifndef __IO_H
#define __IO_H
#include "stm32f1xx_hal.h"

void ChairDownSet (void); // кнопка CHAIR DOWN / КРІСЛО ВНИЗ ввімкнута
void ChairDownReset (void); // кнопка CHAIR DOWN / КРІСЛО ВНИЗ вимкнута
void ChairUpSet (void); // кнопка CHAIR UP / КРІСЛО ВВЕРХ ввімкнута
void ChairUpReset (void); // кнопка CHAIR UP / КРІСЛО ВВЕРХ вимкнута
void BackrestUpSet (void); // кнопка BACKREST UP / СПИНКА ВВЕРХ ввімкнута
void BackrestUpReset (void); // кнопка BACKREST UP / СПИНКА ВВЕРХ вимкнута
void BackrestDownSet (void); // кнопка BACKREST DOWN / СПИНКА ВНИЗ ввімкнута
void BackrestDownReset (void); // кнопка BACKREST DOWN / СПИНКА ВНИЗ вимкнута
void SpittoonSet (void); // кнопка SPITTOON / СПЛЬОВУВАННЯ ввімкнута
void SpittoonReset (void); // кнопка SPITTOON / СПЛЬОВУВАННЯ вимкнута
void ASet (void); // кнопка А ввімкнута
void AReset (void); // кнопка А вимкнута
//void BSet (void); // кнопка B ввімкнута
//void BReset (void); // кнопка B вимкнута

void LightONSet (void); // кнопка LIGHT ON / СВІТИЛЬНИК ввімкнута
void LightONReset (void); // кнопка LIGHT ON / СВІТИЛЬНИК вимкнута
void WaterFSet (void); // кнопка WATER F / НАПОВНЕННЯ СКЛЯНКИ ввімкнута
void WaterFReset (void); // кнопка WATER F / НАПОВНЕННЯ СКЛЯНКИ вимкнута
void WaterSSet (void); // кнопка WATER S / ЗМИВ ПЛЮВАЛЬНИЦІ ввімкнута
void WaterSReset (void); // кнопка WATER S / ЗМИВ ПЛЮВАЛЬНИЦІ вимкнута
void CoagSet (void); // Coag / Коагулятор ввімкнуто від педалі
void CoagReset (void); // Coag / Коагулятор вимкнуто від педалі

void StatusLEDSet (void); // ввімкнення світлодіода, який сигналізує про натискання кнопок керування кріслом
void StatusLEDReset (void); // вимкнення світлодіода, який сигналізує про натискання кнопок керування кріслом
void ErrorLEDSet (void); // ввімкнення світлодіода, який сигналізує про передачу або відсутність передачі даних по CAN
void ErrorLEDReset (void); // вимкнення світлодіода, який сигналізує про передачу або відсутність передачі даних по CAN

uint8_t readButtonPedalONOFF (); // кнопка Pedal_ON/OFF / Педаль ВКЛ./ВИКЛ.
uint8_t readButtonCHIP (); // кнопка CHIP / ПРОДУВКА
uint8_t readButtonSPRAY (); // кнопка SPRAY / СПРЕЙ

#endif
