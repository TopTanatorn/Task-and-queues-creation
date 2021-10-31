#include <Wire.h>
#include <SparkFun_APDS9960.h>
typedef struct {
  char name[16];     // task name
  int  led_pin;      // LED pin number
  int  delay_ms;     // LED toggle delay (msec)
} TaskParams_t;

#define LED_ON   LOW
#define LED_OFF  HIGH 

#define NUM_TASKS  4
int active_task = 0;
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;
int currentL = 0;

TaskHandle_t ledTasks[ NUM_TASKS ];

const TaskParams_t taskParams[ NUM_TASKS ] = {
   {"Task1", 2, 1000},
   {"Task2", 12, 1000},
   {"Task3", 2, 1000},
   {"Task4", 12, 1000},
};

void LED_Blink_Task( void * pvParameters ) {
   TaskParams_t *params = (TaskParams_t *)pvParameters;
   char *task_name = params->name;
   int led_pin = params->led_pin;
   int delay_ticks = params->delay_ms / portTICK_PERIOD_MS; 

   pinMode( led_pin, OUTPUT );
   digitalWrite( led_pin, LED_OFF ); 
   //vTaskSuspend( NULL );   // suspend itself

   while (1) { // endless loop
      digitalWrite( led_pin, LED_ON );   
      vTaskDelay( delay_ticks );
      digitalWrite( led_pin, LED_OFF ); 
      vTaskDelay( delay_ticks );
      Serial.printf( "%s, LED Pin:%d, TickCount: %ld\n", 
                     task_name, led_pin, xTaskGetTickCount() ); 
      vTaskSuspend( NULL );   // suspend itself
   }
}

void setup() {
   Serial.begin( 115200 );
   vTaskPrioritySet( NULL, 2 ); // change the priority of loopTask to 2

   // create new tasks and add them to the list
   for ( int i=0; i < NUM_TASKS; i++ ) { 
        xTaskCreate(
          LED_Blink_Task,           // Task function
          taskParams[i].name,       // Name of task
          8196,                    // Stack size of task 1024x8 = 8K
          (void *)&taskParams[i],   // Parameter of the task 
          1,                        // Priority of the task
          &ledTasks[i] );           // Task handle
   }
    if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}

void loop() {
  vTaskResume( ledTasks[currentL] );
   handleGesture();
}
void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        currentL++;
        currentL =abs(currentL%4);
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        currentL--;
        currentL =abs(currentL%4);
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      
    }
  }
}
