const int sensorPin = 2;
const int measureInterval =500;
volatile int pulseConter;

// YF-S201
const float factorK = 7.5;

// FS300A
//const float factorK = 5.5;

// FS400A
//const float factorK = 3.5;

float volume = 0;
float flow_Lmin=0;
long t0 = 0;

#include <LiquidCrystal.h> // Add LiquidCrystal library
LiquidCrystal lcd(6, 7, 8, 9, 10, 11); // creation of object LCD


void ISRCountPulse()
{
  pulseConter++;
}

float GetFrequency()
{
  pulseConter = 0;

  interrupts();
  delay(measureInterval);
  noInterrupts();

  return (float)pulseConter * 1000 / measureInterval;
}

void SumVolume(float dV)
{
  volume += dV / 60 * (millis() - t0) / 1000.0;
  t0 = millis();
}

void setup()
{
  Serial.begin(9600);
  lcd.begin(16,2);
  attachInterrupt(digitalPinToInterrupt(sensorPin), ISRCountPulse, RISING);
  t0 = millis();
}

void loop()
{
  // obtener frecuencia en Hz
  float frequency = GetFrequency();

  // calcular caudal L/min
   flow_Lmin = frequency / factorK;
  SumVolume(flow_Lmin);

  LCD_1602();

//  Serial.print(" Caudal: ");
//  Serial.print(flow_Lmin, 3);
//  Serial.print(" (L/min)\tConsumo:");
//  Serial.print(volume, 1);
//  Serial.println(" (L)");
 
}

void LCD_1602()
{
// Imprimo la cabecera del sistema
lcd.clear();
lcd.setCursor(1,0);
lcd.print("-> ");
lcd.print(flow_Lmin, 3);
lcd.print(" L/Min");
lcd.setCursor(2,1);
lcd.print("-> ");
lcd.print(volume);
lcd.print(" Litros");
}
