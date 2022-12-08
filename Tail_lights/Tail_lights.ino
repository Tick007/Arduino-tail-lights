#include <Arduino_FreeRTOS.h>
#include "StringSplitter.h"
#include <EEPROM.h>
#include <Arduino.h>


void TaskSerialRead(void *pvParameters );
void TaskToggleLed(void *pvParameters );
void TaskPwmRead(void *pvParameters );
void TaskPwmModeRead(void *pvParameters );
TaskHandle_t TaskHandle_toggle;
TaskHandle_t TaskHandle_tormoz;
TaskHandle_t TaskHandle_mode;

bool toggle=true; //////////////флаг работает или нет  мигание
String incoming; // for incoming serial data
int timeout=20;
bool isFlashing = false;/////////Мигает задний или нет
int led_red_pin = 6;  ///////Внешний красный диод для фар rear lights
int led_redbrake_pin = 5;//5 or 3; putt yours according to soldering//////Внешний красный диод для тормоза brake led
int rear_white_pin = 9;  ///////Внешний белый диод для заднего хода /rear drive white led
byte PWM_PIN = 10; /////////Throtle PWM read
byte PWM_MODE_PIN = 18;//18 - leonardo/pro micro; 11-pro mini /////////Direction selection pwm read
int break_count; ///////Количество нажатий на педаль тормоза
int freq_zero = 1480; /////////PWM zero level
int range_ps=2;
bool isReversed = false;/////////////Показатель, смены зад/вперед
int isReversed_EEPROM_CELL = 0;
const int REAR_LIGHT_LEVEL = 30;
const int REAR_BRAKE_LEVEL = 150;
const int BRAKE_YARKOST_DEFAULT=10;///0-255
const bool monitorModeport = false; ////////////monitor or not pwm mode port. If not - than direction will switch to opposite every power on

#ifdef __SAM3X8E__
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(led_pin, OUTPUT);
 isReversed = EEPROM.read(isReversed_EEPROM_CELL);
 if(monitorModeport==false) {
  isReversed=!isReversed;
  EEPROM.write(isReversed_EEPROM_CELL, isReversed);
 }
//SERIAL.begin(115200);


 xTaskCreate(
    TaskPwmRead
    ,  "PwmRead"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &TaskHandle_toggle ); //Task Handle

  xTaskCreate(
    TaskBlinkTormoz
    ,  "BlinkTormoz"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &TaskHandle_tormoz ); //Task Handle

  ////////////////////////////////////////Зануляем при инициализации  
  if (isFlashing==false) {
    vTaskSuspend(TaskHandle_tormoz);
        break_count=0;
  }
}



void loop() {
  // put your main code here, to run repeatedly:

}


/////////////////////Основной цикл чтения канала газа
void TaskPwmRead( void *pvParameters __attribute__((unused)) ){


  float tormoz = 0;
  float koef = 3.9379;
  int pwm_value = 0;
  int pwm_prev_value;
  int pwm_prev_prev_value;

 float pwm_over_value=freq_zero+((freq_zero/100)*range_ps);
 float pwm_below_value=freq_zero-((freq_zero/100)*range_ps);


 int mode_value = 0;/////////////Текущее значение управляющего сигнала
 int value_diff = 0;/////////////Разница значений управляющего сигнала
 unsigned long time_read; ////////////Секунд от начала работы платы 
 int deviance = 1000; ////////////////Разница частоты PWM для переключения режима
 int prev_mode_value = 0; ///////Предыдущее значение управляющего сигнала
 unsigned long swith_time;//Время когда было поменяно значение режима работы 
 
  for (;;) // A Task shall never return or exit.
      {

        if(monitorModeport==true){
          /* Опрос порта управления */
          time_read = millis();
          mode_value = pulseIn(PWM_MODE_PIN, HIGH);
          value_diff = abs (mode_value -prev_mode_value);
          //SERIAL.println(mode_value);
          
          if(value_diff>deviance){
            if((time_read-swith_time)>2000){
              //SERIAL.println("qqq");
                swith_time = time_read;////////////Время когда поменялось значение
                isReversed = !isReversed;///////////////Отключил пока смену значения. Порт без подключенного приемника сильно щумит
                EEPROM.write(isReversed_EEPROM_CELL, isReversed);
                for (int k=0;k<(isReversed==true?2:3);k++){ //////////////////Мигаем для подтверждения
                  analogWrite(rear_white_pin, 125);
                  vTaskDelay(10);
                  analogWrite(rear_white_pin, 0);
                  vTaskDelay(10);
                }
            }
          }
          /* Опрос порта управления конец*/
       }

         /* Опрос порта газа*/
        tormoz = 0;
        //Простой не эффективный метод
        pwm_value = pulseIn(PWM_PIN, HIGH);
        if(pwm_value==0)continue;//////Если этого нет, то при выключенном приемники (pwm_value = 0) всё время мигает задний;
        
        lightsControll( pwm_over_value, pwm_below_value, pwm_value, pwm_prev_value, koef); 

        pwm_prev_prev_value = pwm_prev_value; 
        prev_mode_value = mode_value;
        pwm_prev_value = pwm_value;

        /* Опрос порта газа конец*/
        
        vTaskDelay(1);
        
      }
}





void TaskBlinkTormoz( void *pvParameters __attribute__((unused)) ){
  for (;;) // A Task shall never return or exit.
  
{
  
  if(isFlashing==true){
     
     analogWrite(led_redbrake_pin, 255);
      vTaskDelay( 4);
      analogWrite(led_redbrake_pin, 0);
      vTaskDelay(3);
    }
    
    
  vTaskDelay(1);
  }
       
}
