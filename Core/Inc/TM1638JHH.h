#ifndef TM1638JHH_H
#define TM1638JHH_H

#include "stdint.h"

void EnviarByte(uint8_t byte);
void DIO_salida();
void DIO_entrada();
void IniciaComu();
void ParaComu();
void MostrarDisplay(uint8_t Dato, uint8_t Display);
void ActualizaDisplay();
uint8_t ConvertirHex7seg(uint8_t DatoHex);
uint8_t LeerBotones(void);
uint8_t RecibirByte();
void Reiniciar();
void Cuenta();
void Home();
void Record();
void MostrarRecord();
void MostrarRecordN();
void BorrarRecord();
void Vacío();
void Deleted();
void Probed();
void MostrarLed(uint8_t Dato, uint8_t Led);
void Leds(uint8_t leds);
void BarraProgreso();
void Progra();
#endif
