
// ----------------------------------------------------------------------------

struct _calibrationvalue {
  int value;
  int power;
};

// Skalierung zum eingeseetzten getMeasurement() und Modul TA12-200
const int calibrationvaluecount = 10;
const _calibrationvalue calibrationvalues[calibrationvaluecount] = {
  {0,0},
  {5,5},
  {9,37},
  {16,75},
  {24,112},
  {31,150},
  {39,187},
  {47,225},
  {54,262},
  {62,300}
};

/*
// Skalierung zum eingeseetzten getMeasurement() und Modul TA12-100 (Messmodul des Akuus)
const int calibrationvaluecount = 5;
const _calibrationvalue calibrationvalues[calibrationvaluecount] = {
  {0,0},
  {5,5},
  {25,130},
  {77,590},
  {125,940}
};
*/

int getPowerFromValue(int value) {
  // suche Eintrag <= Value und >= Value
  int ilow=-1;
  int ihigh=-1;
  for (int i = 0; i < calibrationvaluecount; i++) {
    if (calibrationvalues[i].value <= value) {
      ilow = i;
    }
    if ( (calibrationvalues[i].value >= value) && (ihigh < 0) ) {
      ihigh = i;
    }
  }
  if ( ilow == ihigh) {
    // genauer Treffer in der Tabelle, hier brauch nichts berechnet zu werden
    return calibrationvalues[ilow].power; 
  }
  if (ilow < 0) {
    // außerhalb Kalibrationsbereich (low), darf nicht vorkommen, Leistungswert für 0 Value muss angegeben sein
    return 0;
  }
  if (ihigh < 0) {
    // außerhalb Kalibrationsbreich (high), letztes oberes wertepar verwenden
    ilow = calibrationvaluecount-2;
    ihigh = calibrationvaluecount-1;
  }
  //Serial.println(String(ilow)+"/"+String(ihigh)); // Debug

  int deltavalue = calibrationvalues[ihigh].value - calibrationvalues[ilow].value;
  int deltapower = calibrationvalues[ihigh].power - calibrationvalues[ilow].power;
  float faktor = float(deltapower) / float(deltavalue);
  //Serial.println(String(faktor)); // Debug

  float result = float(calibrationvalues[ilow].power) + ( float(value - calibrationvalues[ilow].value) * faktor );
  //Serial.println(String(result)); // Debug 

  return int(result);
}

// ----------------------------------------------------------------------------
