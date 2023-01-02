
#define ANALOG_IN 0

// --------------------------------------------
// Messfunktionen 
// --------------------------------------------

// Notiz: delay() < 1ms compiliert, tut aber nichts                                              
// analogRead() ist träge, 0,1ms auf dem esp8266
// Wechselspannungsmessung bzw. nur positive Halbwelle kann hier vom adc gemessen werden
// 50hz = 20ms -> 200 Samples je Sinus 
// Um dieses Signal zu messen muss die Anzahl der Samples mindest ein Sinus sein, oder genau ein vielfaches davon
const int sampleCount = 600;

// Values - obacht, nur single threaded zu verwenden
int values[sampleCount];
// iterieren über array über samplecount oder (sizeof(values) / sizeof(values[0]))

// über n Sinus (obere Halbwelle) sampeln
void sampleValues() {
  Serial.println("sampleValues()");
  // hintereinander weg samplen, ohne Unterbrechung
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    values[i] = analogRead(ANALOG_IN);
  } 
}

// --------------------------------------------
// Hilfsfunktionen
// --------------------------------------------

void exValue(int i, int j) {
 int temp = values[i];
 values[i] = values[j];
 values[j] = temp;
}
void qSort(int loIdx, int highIdx) {
 int i = loIdx;
 int j = highIdx;
 int pivot = values[loIdx + (highIdx - loIdx) / 2];
 while (i <= j) {
   while (values[i] < pivot) {
     i++;
   }
   while (values[j] > pivot) {
     j--;
   }
   if (i <= j) {
     exValue(i, j);
     i++;
     j--;
   }
 }
 if (loIdx < j)
   qSort(loIdx, j);
 if (i < highIdx)
   qSort(i, highIdx);
}
void sortValues() {
  Serial.println("sortValues()");
  qSort(0, sampleCount - 1); 
  Serial.println("sortValues() finished");
}

// --------------------------------------------
// Messfunktion Peak
// --------------------------------------------

// für die Peak Messung
// nach dem sortieren die höchsten n values wegschmeißen 
const int skipValues = 5;
// und über die n niedrigeren mitteln
const int measureValues = 10;

// Messen peak bzw. high
int getMeasurementPeak() {
  Serial.println("getMeasurementPeak()");

  sampleValues();

  yield();
  ESP.wdtFeed();

  /* einfach den Max wert suchen
  int measurementValue = 0; 
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    if (values[i] > measurementValue) {
      measurementValue = values[i];
    }
  }
  return measurementValue;
  */

  // ausgehend von einer sinusförmigen Stromaufnahme
  // sortieren
  sortValues();
  // die oberen n values wegschmeißen, ausreißer
  // und über die darauf folgenden values den scheitelwert ermitteln
  int measurementValue = 0; 
  int measurementValueCount = 0;
  for (int i = sampleCount-1-skipValues-measureValues; i < sampleCount-1-skipValues ; i++) 
  {
    measurementValue += values[i];
    measurementValueCount += 1;
  }
  measurementValue = measurementValue / measurementValueCount;
  return measurementValue;
}

// --------------------------------------------
// Messfunktion Mean
// --------------------------------------------

int getMeasurementMean() {
  Serial.println("getMeasurementMean()");

  sampleValues();

  yield();
  ESP.wdtFeed();

  // ausgehend von einer nicht sinus förmigen stromaufnahme den ungefähren strom ermitteln
  // durchschnitt durch aufintegrieren und wieder durch die messwerte teilen
  int measurementValue = 0; 
  int measurementValueCount = 0;
  for (int i = 0; i < sampleCount-1; i++) 
  {
    measurementValue += values[i];
    measurementValueCount += 1;
  }
  measurementValue = measurementValue / measurementValueCount;
  return measurementValue;
}

// --------------------------------------------
// Messfunktion (Wrapper)
// --------------------------------------------

// Skalierung passend zum eingeseetzten getMeasurement()
#define CALCURRENTPOWER 360
#define CALCURRENTVALUE 90

int getMeasurement() {
  Serial.println("getMeasurement()");
  return getMeasurementMean();
}

// --------------------------------------------
// Messung über längeren Zeitraum
// --------------------------------------------

// Für die Messung von dynamischen Lasten muss über einen längeren Zeitraum gemittelt werden
const int avgCount = 100;
bool avgActive = false;
int avgValue = 0;

void doAvgMeasurement() {
  Serial.println("doAvgMeasurement");

  for (int i = 0; i < 3; i++) 
  {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH 
    delay(100);
  } 

  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

  avgValue = 0;
  for (int i = 0; i <= (avgCount-1); i++) 
  {
      avgValue += getMeasurement();

      yield();    
      ESP.wdtFeed(); 
  }
  Serial.println("done");
  Serial.println(avgCount);
  Serial.println(avgValue);
  avgValue = avgValue / avgCount;
  Serial.println(avgValue);

  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH  
}

// --------------------------------------------
// Debugging Funktion
// --------------------------------------------

// Debugging
String getValuesDebug() {
  Serial.println("getValuesDebug()");
  String valuesStr = "";

  sampleValues();
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    valuesStr += values[i];
    valuesStr += ";";
  }  
  valuesStr += "\r\n";

  sortValues();
  for (int i = 0; i < sampleCount - 1 ; i++) 
  {
    valuesStr += values[i];
    valuesStr += ";";
  }  
  valuesStr += "\r\n";

  valuesStr += getMeasurementPeak();
  
  valuesStr += "\r\n";

  valuesStr += getMeasurementMean();
  
  valuesStr += "\r\n";

  return valuesStr;
}

