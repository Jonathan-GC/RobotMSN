
#include <Wire.h>
#include <EEPROM.h>

#include <Arduino.h>
//#include "MultiDriver.h"
//#include "SyncDriver.h"
#include "BasicStepperDriver.h" // generic

#define DEBUG 0

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 64
#define MICROSTEPS 1
#define MOTOR_X_RPM 400
#define MOTOR_Y_RPM 400
#define MOTOR_Z_RPM 400

// X motor
#define DIR_X 2
#define STEP_X 5
// Y motor
#define DIR_Y 3
#define STEP_Y 6


// Z motor
#define DIR_Z 4
#define STEP_Z 7


#define STOPPER_PIN_Y 10
#define STOPPER_PIN_Z 11
#define STOPPER_PIN_X 9


/*************************************
  DECLARACION DE OBJETOS PRINCIPALES
*************************************/
//Declaracion de motores
BasicStepperDriver stepperX(MOTOR_STEPS, DIR_X, STEP_X);
BasicStepperDriver stepperY(MOTOR_STEPS, DIR_Y, STEP_Y);
BasicStepperDriver stepperZ(MOTOR_STEPS, DIR_Z, STEP_Z);
void movimientoJoyStick(short, BasicStepperDriver, byte, byte);
//Declaracion del controlador
//SyncDriver controller(stepperX, stepperY, stepperZ);


/*************************************
*************************************/

/*************************************
  VARIABLES GLOBALES
*************************************/
char dato = 0;
int grados[3] = {0, 0, 0};
int posiciones[50];

// ****************************
//                            *
//          Joystick          *
//                            *
// ****************************
short Joystick1 [3] = {A0,A1, A6};
short Joystick2 [2] = {A6,A7};

void setup() {
  pinMode(STOPPER_PIN_Y,INPUT_PULLUP);
  pinMode(STOPPER_PIN_Z,INPUT_PULLUP);
  pinMode(STOPPER_PIN_X,INPUT_PULLUP);
  Serial.begin(9600);
   pinMode(8,OUTPUT);
  stepperX.begin(MOTOR_X_RPM, MICROSTEPS);
  stepperY.begin(MOTOR_Y_RPM, MICROSTEPS);
  stepperZ.begin(MOTOR_Z_RPM, MICROSTEPS);

  ActivarMotores(false);
  

}

void loop() {
  if(Serial.available()>0){
    dato = Serial.read();
  }

  switch(dato){
    case 'h':
      goToHome();
      dato=0;
      break;

    case 'o':
      ActivarMotores(0);
      break;
    
    case 'g':
      Serial.println(grados[0]);
      break;

    case 'p':
      ActivarMotores(1);
      //movimientoJoyStick(0, stepperX, 0);
      break;
      
    case 'j':
      ActivarMotores(1);
      short JoyIzqX = analogRead(Joystick1[0]);
      short JoyIzqY = analogRead(Joystick1[1]);
      short JoyIzqZ = analogRead(Joystick1[2]);

      movimientoJoyStick(JoyIzqX, stepperX, 0, 100);
      movimientoJoyStick(JoyIzqY, stepperY, 1, 100);
      movimientoJoyStick(JoyIzqZ, stepperZ, 2, 100);
      
      break;

  }

}

void movimientoJoyStick(short joy, BasicStepperDriver Motor, byte pos, byte limCentral = 112){
    
    //Valores del joystick para evitar que se mueva en el centro o rango de NO movimiento cuando se suelta
    //El valor no entra linealizado es decir va de 0 a 1023
    if (joy < (512-limCentral) || joy > (512+limCentral)){

      //linealizo de -10 a 10 para la suavidad
      joy = map(joy,0,1023, -10, 10);
      //guardar en variable global
      grados[pos] += joy;
      Serial.print(joy); Serial.print("\t\t");  Serial.println(grados[0]);
      Motor.move(convertirGrados(joy));
    }
    
    else{
      Motor.stop();
    }
}



//Funcion para hablitar y desabilitar motores de una manera entendible
void ActivarMotores(bool Activar){

   pinMode(8,OUTPUT);
   // Simbolo ! para negar 
   digitalWrite(8,!Activar);
}




int convertirGrados(short degree){
  return 11.3222*degree;
}

boolean goToHome_X(){
  ActivarMotores(true);
  if (digitalRead(STOPPER_PIN_X) == 0){
        //Serial.println("STOPPER REACHED");
        //stepperY.startBrake();
        stepperX.stop();
        return true;
  }else{

    unsigned wait_time_micros = stepperX.nextAction();
    return false;
  }

}

boolean goToHome_Y(){
  ActivarMotores(true);
  if (digitalRead(STOPPER_PIN_Y) == 1){
        //Serial.println("STOPPER REACHED");
        //stepperY.startBrake();
        stepperY.stop();
        return true;
  }else{

    unsigned wait_time_micros = stepperY.nextAction();
    return false;
  }
}

boolean goToHome_Z(){
  ActivarMotores(true);
  if (digitalRead(STOPPER_PIN_Z) == 1){
        //Serial.println("STOPPER REACHED");
        //stepperY.startBrake();
        stepperZ.stop();
        return true;
  }else{

    unsigned wait_time_micros = stepperZ.nextAction();
    return false;
  }
}

void goToHome(){
  ActivarMotores(1);
  boolean flag = false;

  //Setear X
  stepperX.startMove(100 * MOTOR_STEPS * MICROSTEPS);
  do{
    flag = goToHome_X();
    if(flag){
      //Moverse ciertos grados hacia adelante
      stepperX.move(convertirGrados(-25));
    }
  }while(!flag);

  flag = false;

  //Setear Y
  stepperY.startMove(-100 * MOTOR_STEPS * MICROSTEPS);
  do{
    flag = goToHome_Y();
    if(flag){
      //Moverse ciertos grados hacia adelante
      stepperY.move(convertirGrados(25));
    }
  }while(!flag);

  //Setear Z
  stepperZ.startMove(100 * MOTOR_STEPS * MICROSTEPS);
  do{
    flag = goToHome_Z();
    if(flag){
      //Moverse ciertos grados hacia adelante
      stepperZ.move(convertirGrados(-45));
    }
  }while(!flag);

  ActivarMotores(0);
}