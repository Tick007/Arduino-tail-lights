/////////////Функция для работы с диодами фар 

void lightsControll(int pwm_over_value, int pwm_below_value, int pwm_value, int pwm_prev_value, float koef){
 float pwm_duty = 128;


  
  if ((pwm_value<pwm_below_value && isReversed==false ) || (pwm_value>pwm_over_value && isReversed==true )  ){ ///////////////////Тормоз, курок от себя
          //SERIAL.println("Тормоз");
          //SERIAL.println(String(pwm_value)+"=>"+String(pwm_below_value)+"; prev ="+String(pwm_prev_value)+"; diff = "+String(pwm_prev_diff));
          pwm_duty = abs((pwm_value-1000)/koef);
          //tormoz = 255;
          if(isFlashing==false) {
              isFlashing=true;////////////Вначале, т.к. в самом таске проверка на эту переменную
               
               if(break_count==0)vTaskResume(TaskHandle_tormoz); //////////////Запускает мигание каждое третье нажатие
              
              //SERIAL.println("Запуск мигания");
              break_count++;/////////////////////Считаем сколько раз подряд нажато
              analogWrite(led_red_pin, REAR_BRAKE_LEVEL);
          }
          if(break_count>=2){//////////////////////Второе подряд нажатие на тормоз
            //SERIAL.println(break_count); 
            //SERIAL.println(pwm_prev_value); 
            //SERIAL.println(pwm_value); 
            
            if (isFlashing==true) {
              isFlashing==false;
              vTaskSuspend(TaskHandle_tormoz);
            }
            
            analogWrite(rear_white_pin, REAR_LIGHT_LEVEL);
            //SERIAL.println("blink");
            analogWrite(led_red_pin, BRAKE_YARKOST_DEFAULT);

            
            
          }
          
          
        }
        if(pwm_value>=pwm_below_value && pwm_value <= pwm_over_value){/////////////Курок в нейтральной зоне, отпущен
          //SERIAL.println("Нейтраль");
           if(isFlashing==true) {
              //SERIAL.println("Остановка мигания");
              //SERIAL.println(String(pwm_value)+"=>"+String(pwm_below_value)+"; prev ="+String(pwm_prev_value)+"; diff = "+String(pwm_prev_diff));
              isFlashing = false;
              vTaskSuspend(TaskHandle_tormoz);
           }
           analogWrite(rear_white_pin, 0);
           analogWrite(led_red_pin, BRAKE_YARKOST_DEFAULT);
           analogWrite(led_redbrake_pin, 0); ////////////////Гасит, но кто его зажигает ?
        }
        
   
        if((pwm_value>pwm_over_value && isReversed==false ) || (pwm_value<pwm_below_value && isReversed==true)){ ////////////////////////Газ, курок на себя
          pwm_duty = 255-((2000-pwm_value)/koef);
          break_count=0;
        }


          if(pwm_duty<10)pwm_duty=0;
        
}
