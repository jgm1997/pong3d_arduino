#include <Arduino.h>
#include "Keypad.h"
#include "LCD.h"
#include <stdio.h>
#include <stdlib.h>

//pines del LCD
#define PIN_RS A0
#define PIN_RW A1
#define PIN_EN A2

#define PIN_DB4 A8
#define PIN_DB5 A9
#define PIN_DB6 A10
#define PIN_DB7 A11

LCD lcd(
	PIN_RS, PIN_RW, PIN_EN,
	PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7
);

//pines L: filas
//pines R: columnas
#define KEYPAD_COL0_PIN 22
#define KEYPAD_COL1_PIN 24
#define KEYPAD_COL2_PIN 26
#define KEYPAD_COL3_PIN 28

#define KEYPAD_ROW0_PIN 23
#define KEYPAD_ROW1_PIN 25
#define KEYPAD_ROW2_PIN 27
#define KEYPAD_ROW3_PIN 29

Keypad keypad;

char characters[KEYPAD_ROWS*KEYPAD_COLS] = {
	'7',  '8',  '9',  '\0', //Cancel
	'4',  '5',  '6',  '\0', //OK
	'1',  '2',  '3',  '\0', //Borrar la fila
	'0',  '\0', '\0', '\0'
  //     <-    ->     
};

#define ACTION_CANCEL 3
#define ACTION_OK     7
#define ACTION_DELETE_ROW 11 
#define ACTION_PREVIOUS  13 
#define ACTION_NEXT   14
#define ARRAYSIZE 16
#define STATES 7

//Arrays con los nombres de los estados
char inicial[ARRAYSIZE] = "Menu";
char width[ARRAYSIZE] = "Width:";
char height[ARRAYSIZE] = "Height:";
char depth[ARRAYSIZE] = "Depth:";
char speed[ARRAYSIZE] = "Speed:";
char introduce[ARRAYSIZE] = "Value:";
char modified[ARRAYSIZE] = "Modified OK";

//puntero auxiliar que contiene el valor que introduce el usuario
char* p_aux;
//punteros que guardarán el valor de p_aux dependiendo del estado actual
char* p_printWidth;
char* p_printHeight;
char* p_printDepth;
char* p_printSpeed;
//valor que introduce el usuario en el LCD pasado a long (uso de atol)
long valueIntroduced;

//punteros a los arrays de estados
char* p_inicial = inicial;
char* p_width = width;
char* p_height = height;
char* p_depth = depth;
char* p_speed = speed;
char* p_introduce = introduce;
char* p_modified = modified;

//array con los punteros a los estados
char* states[STATES] = { p_inicial, p_width, p_height, p_depth, p_speed, p_introduce, p_modified };

//Struct con las variables para enviar al servidor
struct values{
  long width;
  long height;
  long depth;
  long speed;
}; 

values currentValues;

//Variable que contendrá el número del estado actual
uint8_t current = 0;
//numero de veces que el usuario ha pulsado OK
uint8_t oks;

void setup() {

  //INICIALIZACIONES

  Serial.begin(9600);
  valueIntroduced = 0;
  oks = 0;
  p_aux = NULL;
	keypad.initKeypad(
		KEYPAD_COL0_PIN, KEYPAD_COL1_PIN, KEYPAD_COL2_PIN, KEYPAD_COL3_PIN, 
		KEYPAD_ROW0_PIN, KEYPAD_ROW1_PIN, KEYPAD_ROW2_PIN, KEYPAD_ROW3_PIN
	);

	lcd.init();
  printState(current);

}


//Método que nos permite imprimir el estado actual por el LCD
//En el caso de que sea el estado de una variable, también mostramos su valor

void printState(uint8_t state){
  lcd.clear();
  switch(state){
    //Estado inicial
    case 0:
      lcd.moveCursor(1,7);
      lcd.print(states[0]);
      lcd.moveCursor(2,1);
      lcd.print("          Next->");
      break;
    //Estado anchura
    case 1:
      lcd.print(states[1]);lcd.print(p_printWidth);
      lcd.moveCursor(2,1);
      lcd.print("<-Exit    Next->");
      break;
    //Estado altura
    case 2:
      lcd.print(states[2]);lcd.print(p_printHeight);
      lcd.moveCursor(2,1);
      lcd.print("<-Prev    Next->");
      break;
    //Estado profundidad
    case 3:
      lcd.print(states[3]);lcd.print(p_printDepth);
      lcd.moveCursor(2,1);
      lcd.print("<-Prev    Next->");
      break;
    //Estado velocidad
    case 4:
      lcd.print(states[4]);lcd.print(p_printSpeed);
      lcd.moveCursor(2,1);
      lcd.print("<-Prev          ");
      break;
  }
}

/*
Método que nos permite asignar el valor que ha introducido
el usuario a cada variable del struct currentValues. Además,
también guardamos en un puntero el valor que recibimos cuando llamamos
a "lcd.readSecondRow()" para poder imprimirlo en el LCD ya que no podemos
imprimir variables de tipo long
*/
void assignValue(char* pointer, long value, uint8_t state){

    switch(state){
      case 1: //WIDTH
        currentValues.width = value;
        p_printWidth = pointer;
        break;
      case 2: //HEIGHT
        currentValues.height = value;
        p_printHeight = pointer;
        break;
      case 3: //DEPTH
        currentValues.depth = value;
        p_printDepth = pointer;
        break;
      case 4: //SPEED
        currentValues.speed = value;
        p_printSpeed = pointer;
        break;
    }
}

/*
Método que nos permite realizar diferentes acciones
dependiendo de que tecla se ha pulsado
*/

void action(uint8_t key) {
	char c = characters[key];
	if(!c) {
		switch(key) {
      //Borra la fila de abajo en el caso de que el usuario
      //se haya equivocado introduciendo el valor
			case ACTION_DELETE_ROW:
				lcd.delete_row(2);
				break;
      //Si no queremos introducir ningún valor, volvemos al estado anterior
      case ACTION_CANCEL:
        lcd.clear();
        current = current;
        printState(current);
        oks = 0;
        break;
      //Cambiamos al siguiente estado  
      case ACTION_NEXT:
        lcd.clear();
        current = (current+1) % 5;
        printState(current);
        break;
      //Volvemos al estado anterior
      case ACTION_PREVIOUS:
        lcd.clear();
        current = (current-1) % 5;
        printState(current);
        break;
      //Si pulsamos OK 1 vez, nos lleva al estado para introducir un valor
      //Si pulsamos OK 2 otra vez, confirmamos el valor introducido
      case ACTION_OK:
        oks = oks + 1;
        Serial.println(oks);
        if(oks%2 == 1){
          lcd.clear();
          lcd.print(states[5]);
          lcd.moveCursor(2,1);
          //si es un numero par de oks
        } else {
          p_aux = lcd.readSecondRow();
          valueIntroduced = atol(p_aux);
          assignValue(p_aux, valueIntroduced, current);
          delay(500);
          lcd.clear();
          lcd.print(states[6]);
          delay(800);
          lcd.clear();
          printState(current);
          oks = 0;
        }

        break;
		}
	} else {
			lcd.print(c);
     //Serial.println(c); 
		}
}

 
uint8_t key;

void loop() {
  
  key = keypad.readKeypad(BLOCK); // Polling
	action(key);

}
