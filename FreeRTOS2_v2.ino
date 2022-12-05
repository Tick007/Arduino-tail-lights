#include <Arduino_FreeRTOS.h>
//#include <FreeRTOS_SAMD21.h>
#include "StringSplitter.h"
#include <EEPROM.h>
#include <Arduino.h>
//#include <U8g2lib.h>
//#define FREERTOS_CONFIG_H
//#define configTICK_RATE_HZ                      1500




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
//int led_pin = 3;  ///////Внешний диод
int led_red_pin = 6;  ///////Внешний красный диод для фар rear lights
int led_redbrake_pin = 5;//3 - for pro mini soldered;//////Внешний красный диод для тормоза brake led
int rear_white_pin = 9;  ///////Внешний белый диод для заднего хода /rear drive white led
byte PWM_PIN = 10; /////////Вход PWM для чтения сигнала
byte PWM_MODE_PIN = 18;//18 - leonardo/pro micro; 11-pro mini /////////Вход PWM для чтения сигнала установки режима
int break_count; ///////Количество нажатий на педаль тормоза
int freq_zero = 1480;
int range_ps=2;
bool isReversed = false;/////////////Показатель, смены зад/вперед
int isReversed_EEPROM_CELL = 0;
const int REAR_LIGHT_LEVEL = 30;
const int REAR_BRAKE_LEVEL = 150;
const int BRAKE_YARKOST_DEFAULT=10;///0-255


#if defined (__AVR_ATmega328P__)
    char BOARD[]      = {"UNO"};
#elif defined (__AVR_ATmega2560__)
    char BOARD[]      = {"Mega2560"};
#elif defined (__AVR_ATmega168__)
    char BOARD[]      = {"Mini 168"};
#elif defined (__SAM3X8E__)
    char BOARD[]      = {"DUE"};
#elif defined (__SAM21D__)
    char BOARD[]      = {"Zero"};
#elif defined (__AVR_ATmega32U4__)
    char BOARD[]      = {"Leonardo"};
#else
    char BOARD[] =  {"device type not defined"};
#endif
String board = String(BOARD);

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef  U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 2, 3, U8X8_PIN_NONE);

#ifdef __SAM3X8E__
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

void setup() {
  // put your setup code here, to run once:

  //u8g2.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(led_pin, OUTPUT);
 isReversed = EEPROM.read(isReversed_EEPROM_CELL);

/*
 xTaskCreate(
    TaskDrawDisplay,
      "DrawDisplay",  // A name just for humans
      128,  // This stack size can be checked & adjusted by reading the Stack Highwater
      NULL, //Parameters for the task
      3,  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      NULL); //Task Handle
*/
 //////////Если это указано, то при закрытом serial порте работа не начинается
/*
while (SERIAL) {
    // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
        SERIAL.begin(115200); 
     }
*/
       // initialize serial communication at 9600 bits per second:
  //SERIAL.begin(115200);
  //SERIAL.begin(9600);

/*
xTaskCreate(
    TaskSerialRead
    ,  "SerialRead"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL ); //Task Handle

*/



 xTaskCreate(
    TaskPwmRead
    ,  "PwmRead"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &TaskHandle_toggle ); //Task Handle

/*
 xTaskCreate(
    TaskPwmModeRead
    ,  "PwmModeRead"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &TaskHandle_mode ); //Task Handle
*/
/*
 xTaskCreate(
    TaskToggleLed
    ,  "TogleLed"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &TaskHandle_toggle ); //Task Handle
*/

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
/*
void TaskSerialRead( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
    SERIAL.println("cycle in");
    String income_str;
    for (;;) // A Task shall never return or exit.
      {

      if (SERIAL.available() > 0) {
        // read the incoming byte:
        income_str = SERIAL.readString();
        income_str.trim();

        StringSplitter *splitter = new StringSplitter(income_str,'/',2);
        int itemCount = splitter->getItemCount();
        if(itemCount>0 && itemCount<=2){
          incoming = splitter->getItemAtIndex(0);
          if(itemCount==2)timeout = (splitter->getItemAtIndex(1)).toInt();
        }

        if(incoming!=""){
          if(incoming == "on")  {
            toggle=true;
            //vTaskResume(TaskHandle_toggle);
          }else if(incoming.equals("off") || incoming.equals("of"))  {
            toggle=false;
            //vTaskSuspend(TaskHandle_toggle);
            digitalWrite(LED_BUILTIN, LOW); 
          }else{
            
          }
   
          // say what you got:
          SERIAL.println(board + " has received: "+incoming);
          SERIAL.println("toggle: "+String(toggle));
          incoming = "";
          SERIAL.flush();
        }
        
        
      } 

      vTaskDelay(10);  // one tick delay (15ms) in between reads for stability
  }

}
*/




///////////////Читаем состояние на управляющем канале. Если идет чтение в отдельном таске, то периодически каждые 5 секунд летит помеха на другой вход (который читается в другом таске)
void TaskPwmModeRead( void *pvParameters __attribute__((unused)) ){
  int mode_value = 0;/////////////Текущее значение управляющего сигнала
  int prev_mode_value = 0; ///////Предыдущее значение управляющего сигнала
  int value_diff = 0;/////////////Разница значений управляющего сигнала
  int deviance = 1000;
  unsigned long time_read; ////////////Секунд от начала работы платы 
  unsigned long time_prev;////////Предыдущее значение секунд от начала работы платы на каждом цикле опроса порта PWM сигнала
  unsigned long swith_time;//Время когда было поменяно значение режима работы
  unsigned long diff;
  for (;;) // A Task shall never return or exit.
      {


        
        time_read = millis();
        mode_value = pulseIn(PWM_MODE_PIN, HIGH);
        value_diff = abs (mode_value -prev_mode_value);
        

        if(value_diff>deviance){
         //SERIAL.println(String(value_diff));
         // SERIAL.println("change");
          //SERIAL.println(mode_value);
          //SERIAL.println(prev_mode_value);
          //vTaskDelay(100);

          if((time_read-swith_time)>2000){
              swith_time = time_read;////////////Время когда поменялось значение
              isReversed = !isReversed;///////////////Отключил пока смену значения. Порт без подключенного приемника сильно щумит
          
              for (int k=0;k<(isReversed==true?2:3);k++){ //////////////////Мигаем для подтверждения
                analogWrite(rear_white_pin, 200);
                vTaskDelay(10);
                analogWrite(rear_white_pin, 0);
                vTaskDelay(10);
              }
          }
                                          
        }
        vTaskDelay(50);
        time_prev = time_read;
        prev_mode_value = mode_value;
      }


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
        /*
        if(isReversed==true){//////Мигаем в обратном направлении
           pwm_over_value=freq_zero-((freq_zero/100)*range_ps);
           pwm_below_value=freq_zero+((freq_zero/100)*range_ps);
        }
        */

        //SERIAL.println("qqq");

        /* Опрос порта управления */
        time_read = millis();
        mode_value = pulseIn(PWM_MODE_PIN, HIGH);
        value_diff = abs (mode_value -prev_mode_value);
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

         /* Опрос порта газа*/
        tormoz = 0;
        //Простой не эффективный метод
        pwm_value = pulseIn(PWM_PIN, HIGH);
        //SERIAL.println(pwm_value);
        //int diff = pwm_prev_value-pwm_value;
        //if(diff>20 || diff<-20)SERIAL.println(diff);
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


/*
void TaskDrawDisplay( void *pvParameters __attribute__((unused)) ){
  for (;;) // A Task shall never return or exit.
      {
       u8g2.clearBuffer();
       u8g2.setFont(u8g2_font_ncenB08_tr);
       u8g2.drawStr(0,10, "ideaspark 0.96'' OLED");
       vTaskDelay(500); 
    }
}
*/


/*
void TaskToggleLed( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
    for (;;) // A Task shall never return or exit.
    {
      if(toggle==true) { 
       // SERIAL.println("blinking");
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        vTaskDelay(timeout);  // one tick del1
        
      }
      
      digitalWrite(LED_BUILTIN, LOW); 
       vTaskDelay(timeout);  // one tick delay (15ms) in between reads for stability
       //SERIAL.println("blink");
    } 
}
*/
