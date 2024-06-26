
// ----------------------------------------------------------------------------

struct _calibrationvalue {
  int value;
  int power;
};

/*
// Skalierung zum eingeseetzten getMeasurement() und Modul TA12-200
const int calibrationoffset = 0; // für die Kalibrierung auf 0 setzen
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
  //{100+calibrationoffset,500}
};
*/

// Skalierung zum eingeseetzten getMeasurement() und Modul TA12-100 (Messmodul des Akuus)

/*
// für Solar Garage 
const int calibrationoffset = 7; // für die Kalibrierung auf 0 setzen
const int calibrationvaluecount = 3; // tabelle nach offsetkorrektur
const _calibrationvalue calibrationvalues[calibrationvaluecount] = {
  {0,0},
  {5,5},
  {100+calibrationoffset,1000}
};
*/

// für Akkusystemcontroller
const int calibrationoffset = 0; // für die Kalibrierung auf 0 setzen
const int calibrationvaluecount = 13;  // tabelle nach offsetkorrektur
const _calibrationvalue calibrationvalues[calibrationvaluecount] = {
  {0,0},
  {5,5},
  {6,23},
  {8,52},
  {11,75},
  {15,102},
  {18,128},
  {21,153},
  {25,178},
  {29,205},
  {37,229},
  {43,238},
  {77,580}
};

int getPowerFromValue(int value) {
  // suche Eintrag <= Value und >= Value
  value = value - calibrationoffset;
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
