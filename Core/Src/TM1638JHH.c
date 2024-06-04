/* Autor: José Honorio, Referencia: TM1638_config.h by Hossein.M (https://github.com/Hossein-M98) */


//Librerias llamadas
#include "TM1638JHH.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

//Definicion de pines
#define STB GPIO_PIN_0 //Definiendo PIN A0 como STB.
#define CLK GPIO_PIN_1 //Definiendo PIN A1 como CLK.
#define DIO GPIO_PIN_4 //Definiendo PIN A4 como DIO.

//Definicion de variables que almacenan valores de nuestro cronometro
uint8_t ms = 0;
uint8_t sec = 0;
uint8_t min = 0;
uint8_t hour = 0;

///Definicion de variables que almacenan el mejor tiempo.
uint8_t Rms = 100;
uint8_t Rsec = 60;
uint8_t Rmin = 60;
uint8_t Rhour = 13;

//Llamado de variables externas, utilizadas para configurar el apartado programable.
extern uint8_t Regms;
extern uint8_t Regsec;
extern uint8_t Regmin;
extern uint8_t Reghour;
extern float timeRef;

//Variable que almacena cómo debe estar configurada la barra de luces led superior del TM1638.
uint8_t barra=0;

//Arreglo necesario para la conversión de Hexadecimal a 7segmentos
const uint8_t Hex7Seg[40] =
{
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x77, // A
  0x7c, // b
  0x39, // C
  0x5E, // d
  0x79, // E
  0x71, // F
  0x6F, // g
  0x3D, // G
  0x74, // h
  0x76, // H
  0x05, // i
  0x06, // I
  0x0D, // j
  0x30, // l
  0x38, // L
  0x54, // n
  0x37, // N
  0x5C, // o
  0x3F, // O
  0x73, // P
  0x67, // q
  0x50, // r
  0x6D, // S
  0x78, // t
  0x1C, // u
  0x3E, // U
  0x66, // y
  0x08, // .
  0x40, // -
  0x01  // Overscore
};

//Se inicia la comunicacion poniendo en bajo el STB
void IniciaComu(){
	HAL_GPIO_WritePin(GPIOA, STB, GPIO_PIN_RESET);
}

//Se finaliza la comunicacion poniendo en alto el STB
void ParaComu(){
	HAL_GPIO_WritePin(GPIOA, STB, GPIO_PIN_SET); //Se para la comunicacion
}

//Dio como salida
void DIO_salida(){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = DIO;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

//Dio como entrada
void DIO_entrada(){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = DIO;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}



//funcion para enviar un byte al TM1638
void EnviarByte(uint8_t byte){
	uint8_t bit; //variable que almacena bit a bit
	DIO_salida(); //DIO como salida
	//Funcion para enviar bit a bit iniciando por el menos significativo
    for (uint8_t i = 0; i< 8; i++) {
    	HAL_GPIO_WritePin(GPIOA, CLK, GPIO_PIN_RESET); //CLK en bajo
        bit = (byte >> i) & 1; //desplazamiento y operación AND para obtener cada bit
        HAL_GPIO_WritePin(GPIOA, DIO, bit); //El dato "bit" debe ser transmitido por el pin DIO
        HAL_GPIO_WritePin(GPIOA, CLK, GPIO_PIN_SET); //CLK en alto para que el TM1638 lea el bit en cada subida
    }
}


//funcion para recibir un byte proveniente del TM1638
uint8_t RecibirByte(){
	uint8_t byte=0; //representa valor inicial
	HAL_Delay(1); //tiempo de espera mínimo entre bytes "2us" según el datasheet
    for (uint8_t i = 0; i< 8; i++) {
        byte |= HAL_GPIO_ReadPin(GPIOA, DIO) << i; //Operacion OR y desplazamiento a la izquierda "i" veces.
        //Cada bit se almacena en el cambio de reset a set
        HAL_GPIO_WritePin(GPIOA, CLK, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, CLK, GPIO_PIN_SET);
    }
    return byte; //devuelve el byte recibido
}

//Pasarlo de Hexadecimal a 7 segmentos
uint8_t ConvertirHex7seg(uint8_t DatoHex){
	uint8_t Dato7seg = 0; //Dato en 7segmentos
	uint8_t Punto = DatoHex & 0x80; //Almacena el valor del bit más significativo
	DatoHex &= 0x7F; //Almacena el valor del resto de bits
	Dato7seg = Hex7Seg[DatoHex] | Punto; //Se busca el valor correspondiente en 7segmentos y almacena estado del punto en el MSB
	return Dato7seg;//retorna dato7seg
}

//funcion para mostrar valores en el display
void MostrarDisplay(uint8_t Dato, uint8_t Display){
	uint8_t Data=0x40; //Comando para indicar el modo incremento de direccion
	Dato=ConvertirHex7seg(Dato); //convertirmos el número que queremos ver en el display a 7seg
	IniciaComu();//inicia comunicacion
	EnviarByte(Data); //envia el primer byte para indicar el modo incremento de direccion
	ParaComu(); //para comunicacion
	Data = 0xC0 | (2*Display); //almacena la direccion del display
	IniciaComu();//inicia comunicacion
	EnviarByte(Data);//envia el byte que indica la direccion del display
	EnviarByte(Dato);//envia el byte que indica la informacion a ver en el display
	ParaComu();//para comunicacion
}

//funcion para controlar el ON/OFF del LED
void MostrarLed(uint8_t Dato, uint8_t Led){
	uint8_t Data=0x44; //Comando para indicar el modo direccion fija
	IniciaComu(); //inicia comunicacion
	EnviarByte(Data); //envia el primer byte para indicar el modo direccion fija
	ParaComu();//para comunicacion
	Data=0xC1 + (Led << 1); //almacena la direccion del LED
	IniciaComu(); //inicia comunicacion
	EnviarByte(Data);//envia el byte que indica la direccion del LED
	EnviarByte(Dato); //envia el byte que indica la informacion a ver en el LED
    ParaComu();//para comunicacion
}


uint8_t LeerBotones(void) {
	uint8_t botones=0; //valor inicial botones
	IniciaComu();//inicia comunicacion
	EnviarByte(0x42); //envia el primer byte para indicar el modo de lectura de botones
	HAL_GPIO_WritePin(GPIOA, CLK, GPIO_PIN_SET);  //CLK debe iniciar en SET
    DIO_entrada(); //Se configura DIO como entrada
	for (uint8_t i = 0; i < 4; i++){
		uint8_t v = RecibirByte() << i; //recibe la lectura de los botones
		botones |= v; //se almacena el byte leído en la variable botonoes
	}
	DIO_salida();//Se configura DIO como salida
	ParaComu(); //para comunicacion
	return botones; //Retorna el valor de botones
}



//----------------------Funciones secundarias-----------------------

void ActualizaDisplay(){
    // Actualizar milisegundos
	MostrarDisplay((ms % 10), 7);//display 7 de izq a der
	MostrarDisplay((ms / 10), 6);//display 6 de izq a der
	MostrarDisplay((ms / 10), 6);//display 6 de izq a der

    // Actualizar segundos
	MostrarDisplay((sec % 10) | 0x80, 5);//display 5 de izq a der
	MostrarDisplay(sec / 10, 4);//display 4 de izq a der

    // Actualizar minutos
	MostrarDisplay((min % 10) | 0x80, 3);//display 3 de izq a der
	MostrarDisplay(min / 10, 2);//display 2 de izq a der

    // Actualizar horas
	MostrarDisplay((hour % 10) |0x80, 1);//display 1 de izq a der
	MostrarDisplay(hour / 10, 0); //display 0 de izq a der

	if(barra==1){
		BarraProgreso(); //Muestra config 1 de la barra
	}

	if(barra==2){ //Muestra config 2 de la barra
		for(uint8_t i=0; i<8;i++){
				MostrarLed(1, i); //encender cada LED del 0 al 7
			}
	}

}

void BarraProgreso(){ //funcion que muestra una barra de progreso
  // Calcular el porcentaje de progreso
  float progreso = ((hour * 3600) + (min * 60) + sec + (ms / 100.0)) / (timeRef); //progreso
  uint8_t L2L = (uint8_t)(progreso * 8); // selecciona los LEDS a encenderse de acuerdo con el progreso

  // Actualizar LEDs
  for (uint8_t i = 0; i < 8; i++){
	  MostrarLed(i <  L2L ? 1 : 0, 7 - i);//enciende solo los permitidos
  }
}

void Leds(uint8_t leds){ // funcion que muestra el patron de leds traslado de izq a der
	if(leds==1){
		for(uint8_t i=0; i<9;i++){
				if(i>0){
					MostrarLed(0,i-1);//en caso i sea mayor a 0
				}
				if(i<8){
					MostrarLed(1, i);//en caso i sea menor a 8
				}
				if(i==8){
					MostrarLed(0,7); //si i llega a 8
				}
				HAL_Delay(50); //retraso de 50ms para transicion del LED
			}
	}
}


void Record(){ //almacenar el record
	if((hour<Rhour)|((hour==Rhour)&(min<Rmin))|((hour==Rhour)&(min==Rmin)&(sec<Rsec))|((hour==Rhour)&(min==Rmin)&(sec==Rsec))&(ms<Rms)){ //condicion para ser record
		Rms = ms;//almacena record de ms
		Rsec = sec;//almacena record de sec
		Rmin = min;//almacena record de min
		Rhour = hour;//almacena record de hour
	}
}

void Reiniciar(){//funcion para reiniciar la cuenta
	ms = 0;
	sec = 0;
	min = 0;
	hour = 0;
}

void BorrarRecord(){//funcion para reiniciar record
	Rms = 100;
	Rsec = 60;
	Rmin = 60;
	Rhour = 13;
}


void MostrarRecord(){ //Mostrar el mejor tiempo
	MostrarDisplay(38, 0); //-
	MostrarDisplay(31, 1); //r
	MostrarDisplay(14, 2); //E
	MostrarDisplay(12, 3); //C
	MostrarDisplay(28, 4); //O
	MostrarDisplay(31, 5); //r
	MostrarDisplay(13, 6);//d
	MostrarDisplay(38, 7); //-
	HAL_Delay(2000);
	//Muestra record
	MostrarDisplay(Rhour / 10, 0); //RECORD HORA
	MostrarDisplay((Rhour % 10) | 0x80, 1);//RECORD HORA
	MostrarDisplay(Rmin / 10, 2);//RECORD MIN
	MostrarDisplay((Rmin % 10) | 0x80, 3);//RECORD MIN
	MostrarDisplay(Rsec / 10, 4);//RECORD SEC
	MostrarDisplay((Rsec % 10) | 0x80, 5);//RECORD SEC
	MostrarDisplay(Rms / 10, 6);//RECORD MS
	MostrarDisplay(Rms % 10, 7);//RECORD MS
}

void MostrarRecordN(){ //MUESTRA RECORD
	MostrarDisplay(38, 0); //-
	MostrarDisplay(31, 1); //r
	MostrarDisplay(14, 2); //E
	MostrarDisplay(12, 3); //C
	MostrarDisplay(28, 4); //O
	MostrarDisplay(31, 5); //r
	MostrarDisplay(13, 6);//d
	MostrarDisplay(38, 7); //-
	HAL_Delay(2000);
	Vacío();
}


void Vacío(){//MUESTRA NO DATA
	MostrarDisplay(38, 0);//-
	MostrarDisplay(26, 1);//N
	MostrarDisplay(27, 2);//O
	MostrarDisplay(13, 3);//D
	MostrarDisplay(10, 4);//A
	MostrarDisplay(33, 5);//T
	MostrarDisplay(10, 6);//A
	MostrarDisplay(38, 7);//-
}

void Deleted(){ //MUESTRA DELETED
	MostrarDisplay(13, 0);//D
	MostrarDisplay(14, 1);//E
	MostrarDisplay(24, 2);//L
	MostrarDisplay(14, 3);//E
	MostrarDisplay(33, 4);//T
	MostrarDisplay(14, 5);//E
	MostrarDisplay(13, 6);//D
	MostrarDisplay(38, 7);//-
}

void Probed(){//MUESTRA PROBED
	MostrarDisplay(38, 0);//-
	MostrarDisplay(29, 1);//P
	MostrarDisplay(31, 2);//R
	MostrarDisplay(28, 3);//O
	MostrarDisplay(11, 4);//B
	MostrarDisplay(14, 5);//E
	MostrarDisplay(13, 6);//D
	MostrarDisplay(38, 7);//-
}

void Progra(){ //MUESTRA PROGRA
	MostrarDisplay(38, 0);//-
	MostrarDisplay(29, 1);//P
	MostrarDisplay(31, 2);//R
	MostrarDisplay(28, 3);//O
	MostrarDisplay(17, 4);//G
	MostrarDisplay(31, 5);//R
	MostrarDisplay(10, 6);//A
	MostrarDisplay(38, 7);//-
}

void Cuenta(){ //MUESTRA CUENTA
	MostrarDisplay(38, 0);//-
	MostrarDisplay(38, 1);//-
	MostrarDisplay(38, 2);//-
	MostrarDisplay(0, 3);//0
	MostrarDisplay(3, 4);//3
	MostrarDisplay(38, 5);//-
	MostrarDisplay(38, 6);//-
	MostrarDisplay(38, 7);//-
	HAL_Delay(1000);//RETRASO DE 1s
	MostrarDisplay(2, 4);//2
	HAL_Delay(1000);//RETRASO DE 1s
	MostrarDisplay(1, 4);//1
	HAL_Delay(1000);//RETRASO DE 1s
}

void Home(){//MUESTRA HOME
	MostrarDisplay(38, 0);//-
	MostrarDisplay(38, 1);//-
	MostrarDisplay(19, 2);//H
	MostrarDisplay(28, 3);//O
	MostrarDisplay(26, 4);//M
	MostrarDisplay(14, 5);//E
	MostrarDisplay(38, 6);//-
	MostrarDisplay(38, 7);//-
}












