//
// *** WHOI changes: originally this source allowed only one power cycle. Changes were done to
// operate correctly in a loop. Additionally, originally the max time between cycles
// was 59 minutes. Changes were done to implement and support a proper calendar-based
// API (by setting useDateTimeMethod=true) that allowed completely arbitrary
// times between power loops.
//

// 
// Simple example showing how to set the RTC alarm pin to periodically wake up the Arduino
// which in turn wakes up the Rpi. You can use this to do things with the Rpi 
// at regular intervals i.e. like taking a photograph.
//
// Note, in this example the Rpi is woken up for every 5 minutes (you can change that).
// Once woken it is then shutdown after 1 minute (you can change that) unless the user 
// hasn't already shut the Rpi down. The system then goes into low power mode until 
// the next scheduled time interval to wakeup.
//
// This example shows an alternative method than using the Timer to do periodic wakeups.
// (as shown in WakePiPeriodically.ino example)
//
// To test on the RPi without power cycling and using the Arduino IDE
// to view the debug messages, comment out these line (with a //):
//
// SleepyPi.enablePiPower(true);
// SleepyPi.piShutdown();      
// SleepyPi.enableExtPower(false);  
// 
// Also either fit the Power Jumper or enable
// self-power. http://spellfoundry.com/sleepy-pi/programming-arduino-ide/
// 

// **** INCLUDES *****
#include "SleepyPi2.h"
#include <TimeLib.h>
#include <LowPower.h>
#include <PCF8523.h>
#include <Wire.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
}; 

const int LED_PIN = 13;

// Globals
// ++++++++++++++++++++ CHANGE ME ++++++++++++++++++
uint8_t  WakeUp_StartMinute   = 4;   // Minutes. Legacy input, ignore when using DateTime

unsigned long    MAX_RPI_TIME_TO_STAY_AWAKE_MS  = 300000;      // in milliseconds
#define kPI_CURRENT_THRESHOLD_MA   150                         // Shutdown current threshold in mA. When the
                                                               // when the Rpi is below this, it is "shutdown"
                                                               // This will vary from Rpi model to Rpi model
                                                               // and you will need to fine tune it for each Rpi
                                                               // See Code below for tuning code
const int DRY_RUN = 1;                                         // If this is 1, does a dry run, i.e.
                                                               // does not actually apply or remover power
                                                               // If it is 0, then actually applies and
                                                               // removes power from the sleepypi
int32_t secondsBetweenCycles = 3600;                           // Seconds between power cycles (for DateTime method)
int32_t secondsFirstLoop = 60;                                 // This is the time between the press of reset and the first
                                                               // powerup loop. This is typically shorter than
                                                               // secondsBetweenCycles as the first loop we just
                                                               // wants to see that it works
bool useDateTimeMethod = true;                                 // If true uses DateTime alarm method, if false uses legacy method
// ++++++++++++++++++++ END CHANGE ME ++++++++++++++++++

tmElements_t tm;
uint8_t   nextWakeTime;

// SS - This is for DateTime objects
DateTime nextWakeDateTime;

void alarm_isr()
{
    // Just a handler for the alarm interrupt.
    // You could do something here
    Serial.println("Alarm ISR triggered...");
}

void setup()
{
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);    
  digitalWrite(LED_PIN,LOW);    // Switch off LED
  
  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  Serial.println("Starting, but I'm going to go to sleep for a while...");
  delay(250);  
  
  SleepyPi.rtcInit(true);

  if (DRY_RUN == 0)
  {
    SleepyPi.enablePiPower(false); 
    SleepyPi.enableExtPower(false);
  }

  // Default the clock to the time this was compiled.
  // Comment out if the clock is set by other means
  // ...get the date and time the compiler was run
  // SS - the RTC clock is set by other means, commenting this
  //if (getDate(__DATE__) && getTime(__TIME__)) {
  //    // and configure the RTC with this info
  //    SleepyPi.setTime(DateTime(F(__DATE__), F(__TIME__)));
  //}  

  //printTimeNow();

  //PrintRTCRegisters();
  
  //Serial.print("Alarm Set for every: ");
  //Serial.print(WakeUp_StartMinute);     
  //Serial.println(" minutes");         

  if (useDateTimeMethod == true)
  {
    nextWakeDateTime = CalcNextWakeDateTime(secondsFirstLoop);
  }
  else
  {
    // for the first loop wake up after only one minute
    WakeUp_StartMinute = 1;
    // Calculate the initial Start Time
    nextWakeTime = CalcNextWakeTime();
    // For the following loops every 4 minutes
    WakeUp_StartMinute = 4;
  }
}

void loop() 
{
    unsigned long TimeOutStart, ElapsedTimeMs,TimeOutEnd ;
    bool pi_running;
  
    // Allow wake up alarm to trigger interrupt on falling edge.
    attachInterrupt(0, alarm_isr, FALLING);   // Alarm pin

    SleepyPi.enableWakeupAlarm(true);

    if (useDateTimeMethod == true)
    {
      // Use the overload that takes uint8_t day, uint_8 hour, uint_8 minute
      SleepyPi.setAlarm(nextWakeDateTime.day(), nextWakeDateTime.hour(), nextWakeDateTime.minute());
    }
    else
    {    
      // Setup the Alarm Time. Use the overload that takes uint8_t minute
      SleepyPi.setAlarm(nextWakeTime);     
    }
    
    PrintRTCRegisters();   // for debug
    
    delay(500);

    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low (which occurs when our alarm clock goes off)
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0); 
    
    SleepyPi.ackAlarm();

    if (useDateTimeMethod == true)
    {
      nextWakeDateTime = CalcNextWakeDateTime(secondsBetweenCycles);
    }
    else
    {
      // At this point we've woken up
      nextWakeTime = CalcNextWakeTime();
    }

    if (DRY_RUN == 0)
    {
      SleepyPi.enablePiPower(true);      
    }

    digitalWrite(LED_PIN,HIGH);   // Switch on LED
    Serial.println("I've Just woken up on the Alarm!");
    // Print the time
    printTimeNow();   
    delay(50);
    digitalWrite(LED_PIN,LOW);    // Switch off LED 

    // Do something on the Rpi here
    // Example: Take a Picture
    // This is the time that the Rpi will be "awake" for

    // Manage the Rpi shutdown. Give it a maximum time limit to stay awake
    // if it hasn't already shut down
    delay(10000);        // wait to boot up a bit - (this delay should really be added to MAX_RPI_TIME_TO_STAY_AWAKE_MS)
    Serial.println("Starting pi running monitoring loop");
    TimeOutStart = millis();
    ElapsedTimeMs = TimeOutStart;
    TimeOutEnd = TimeOutStart + MAX_RPI_TIME_TO_STAY_AWAKE_MS;
    pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
    // Wait until either the Pi shuts down through user action i.e sudo shutdown -h now
    // or the timer expires  
    while((pi_running == true) && (ElapsedTimeMs < TimeOutEnd))
    {
        pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
        ElapsedTimeMs = millis();
        digitalWrite(LED_PIN,HIGH); // SS - debug
        delay(1000);
        digitalWrite(LED_PIN,LOW); // SS - debug
        // For debug and tuning (you'll need a programming cable of equivalent. 
        // Uncomment and login to the Rpi. Issue a "sudo shutdown -h now" command and see what the 
        // Current changes to. Use this to work out a shutdown current threshold. 
        // Remember that the Timer here, will backstop this method and cut the power on time out.
        Serial.print(pi_running);
        Serial.print(" ");
        Serial.println(SleepyPi.rpiCurrent());
    }      

    // Start a shutdown
    if(pi_running == true){
        // Do a commanded shutdown
        if (DRY_RUN == 0)
        {
          SleepyPi.piShutdown();      
          SleepyPi.enableExtPower(false);
        } 
    }
    else {
        // Already shutdown so lets just cut the power
        if (DRY_RUN == 0)
        {
          SleepyPi.enablePiPower(false);
          SleepyPi.enableExtPower(false);
        }
    }

    Serial.print("Shutdown finished, current: ");
    Serial.print(SleepyPi.rpiCurrent());
    Serial.println(" mA");
     
    // SS - add delay before new loop to avoid cutting the processor's head
    delay(3000);
}

// **********************************************************************
// 
//  - Helper routines
//
// **********************************************************************
uint8_t CalcNextWakeTime(void)
{
  DateTime now = SleepyPi.readTime();
    
  nextWakeTime = now.minute() + WakeUp_StartMinute;
  if(nextWakeTime == 60){
      nextWakeTime = 0; 
  }
  else if (nextWakeTime > 60){
      nextWakeTime = nextWakeTime - 60;  
  }
 
  return nextWakeTime;
}

// SS - This should work for any timespan and also for leap years.
//      Returns a DateTime object
DateTime CalcNextWakeDateTime(int32_t loopSeconds)
{
  DateTime now = SleepyPi.readTime();

  TimeSpan delta(loopSeconds);
  nextWakeDateTime = now + delta;

  return nextWakeDateTime;
}

void printTimeNow()
{
    // Read the time
    DateTime now = SleepyPi.readTime();
    
    // Print out the time
    Serial.print("Ok, Time = ");
    print2digits(now.hour());
    Serial.write(':');
    print2digits(now.minute());
    Serial.write(':');
    print2digits(now.second());
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(now.day());
    Serial.write('/');
    Serial.print(now.month()); 
    Serial.write('/');
    Serial.print(now.year(), DEC);
    Serial.println();

    return;
}
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void PrintRTCRegisters(void)
{
  
      // Debug
      uint8_t reg_value;
      reg_value = SleepyPi.rtcReadReg(PCF8523_CONTROL_1);
      Serial.print("Control 1: 0x");
      Serial.println(reg_value,HEX);
      reg_value = SleepyPi.rtcReadReg(PCF8523_CONTROL_2);
      Serial.print("Control 2: 0x");
      Serial.println(reg_value, HEX);      
      reg_value = SleepyPi.rtcReadReg(PCF8523_CONTROL_3);
      Serial.print("Control 3: 0x");
      Serial.println(reg_value,HEX); 
      
      reg_value = SleepyPi.rtcReadReg(PCF8523_SECONDS);
      Serial.print("Seconds: ");
      Serial.println(reg_value,HEX);
      reg_value = SleepyPi.rtcReadReg(PCF8523_MINUTES);
      Serial.print("Minutes: ");
      Serial.println(reg_value,HEX);  
      reg_value = SleepyPi.rtcReadReg(PCF8523_HOURS);
      Serial.print("Hours: ");
      Serial.println(reg_value,HEX);  
      reg_value = SleepyPi.rtcReadReg(PCF8523_DAYS);
      Serial.print("Days: ");
      Serial.println(reg_value,HEX);   
      reg_value = SleepyPi.rtcReadReg(PCF8523_WEEKDAYS);
      Serial.print("Week Days: ");
      Serial.println(reg_value,HEX);    
      reg_value = SleepyPi.rtcReadReg(PCF8523_MONTHS);
      Serial.print("Months: ");
      Serial.println(reg_value,HEX);  
      reg_value = SleepyPi.rtcReadReg(PCF8523_YEARS);
      Serial.print("Years: ");
      Serial.println(reg_value,HEX); 
      
      reg_value = SleepyPi.rtcReadReg(PCF8523_MINUTE_ALARM);
      Serial.print("Minute Alarm: ");
      Serial.println(reg_value,HEX);      
      reg_value = SleepyPi.rtcReadReg(PCF8523_HOUR_ALARM);
      Serial.print("Hour Alarm: ");
      Serial.println(reg_value,HEX);  
      reg_value = SleepyPi.rtcReadReg(PCF8523_DAY_ALARM);
      Serial.print("Day Alarm: ");
      Serial.println(reg_value,HEX);      
      reg_value = SleepyPi.rtcReadReg(PCF8523_WEEKDAY_ALARM);
      Serial.print("Weekday Alarm: ");
      Serial.println(reg_value,HEX); 
      
      reg_value = SleepyPi.rtcReadReg(PCF8523_OFFSET);
      Serial.print("Offset: 0x");
      Serial.println(reg_value,HEX); 
      reg_value = SleepyPi.rtcReadReg(PCF8523_TMR_CLKOUT_CTRL);
      Serial.print("TMR_CLKOUT_CTRL: 0x");
      Serial.println(reg_value,HEX);  
      reg_value = SleepyPi.rtcReadReg(PCF8523_TMR_A_FREQ_CTRL);
      Serial.print("TMR_A_FREQ_CTRL: 0x");
      Serial.println(reg_value,HEX); 
      reg_value = SleepyPi.rtcReadReg(PCF8523_TMR_A_REG);
      Serial.print("TMR_A_REG: 0x");
      Serial.println(reg_value,HEX);     
      reg_value = SleepyPi.rtcReadReg(PCF8523_TMR_B_FREQ_CTRL);
      Serial.print("TMR_B_FREQ_CTRL: 0x");
      Serial.println(reg_value,HEX);    
       reg_value = SleepyPi.rtcReadReg(PCF8523_TMR_B_REG);
      Serial.print("TMR_B_REG: 0x");
      Serial.println(reg_value,HEX);     
 
}

