// ============================================================================
//               SYSTEME COMPLET DE DETECTION DE FUITE D'EAU
//                    ESP32 + BLYNK + LCD + RELAIS
// ============================================================================
//
// PROJET IoT : Surveillance intelligente de fuite d'eau
//
// ---------------------------------------------------------------------------
// FONCTIONNALITES
// ---------------------------------------------------------------------------
//
// ✅ Mesure débit entrée
// ✅ Mesure débit sortie
// ✅ Affichage LCD 16x2
// ✅ Affichage Moniteur Série
// ✅ Envoi vers Blynk Mobile
// ✅ Envoi vers Dashboard Web Blynk
// ✅ Détection de fuite
// ✅ Arrêt automatique pompe
// ✅ Activation buzzer
// ✅ Notification Blynk
// ✅ Surveillance WiFi
// ✅ Surveillance Blynk
//
// ---------------------------------------------------------------------------
// UNITE UTILISEE
// ---------------------------------------------------------------------------
//
// mL/s  (millilitres par seconde)
//
// Car votre projet utilise :
// - petits tuyaux
// - mini pompe 12V
// - petit débit
//
// ============================================================================



// ============================================================================
//                           CONFIGURATION BLYNK
// ============================================================================

#define BLYNK_TEMPLATE_ID "" // Bien vouloir renseigner votre propre Template ID

#define BLYNK_TEMPLATE_NAME "Water pipe leak detection system"

#define BLYNK_AUTH_TOKEN "" // Bien vouloir renseigner votre propre Auth Token

#define BLYNK_PRINT Serial



// ============================================================================
//                               LIBRAIRIES
// ============================================================================

#include <WiFi.h>

#include <WiFiClient.h>

#include <BlynkSimpleEsp32.h>

#include <LiquidCrystal_I2C.h>

#include <Wire.h>



// ============================================================================
//                              WIFI
// ============================================================================

char ssid[] = ""; // Bien vouloir renseigner votre propre SSID

char pass[] = ""; // Bien vouloir renseigner votre propre mot de passe WiFi



// ============================================================================
//                              LCD I2C
// ============================================================================

// Adresse I2C = 0x27
// Taille LCD = 16 colonnes x 2 lignes

LiquidCrystal_I2C lcd(0x27, 16, 2);



// ============================================================================
//                       BROCHES ESP32 REELLES
// ============================================================================

// ---------------------- CAPTEUR ENTREE ----------------------

#define SENSOR1 27

// ---------------------- CAPTEUR SORTIE ----------------------

#define SENSOR2 14

// ---------------------- RELAIS POMPE ------------------------

#define RELAY 18

// ---------------------- BUZZER ------------------------------

#define BUZZER 19



// ============================================================================
//                       VARIABLES CAPTEUR 1
// ============================================================================

// Compteur impulsions capteur 1

volatile int pulseCount1 = 0;

// Débit calculé

float flowRate1 = 0.0;

// Temps précédent

unsigned long previousMillis1 = 0;

// Facteur calibration

float calibrationFactor1 = 8.57;



// ============================================================================
//                       VARIABLES CAPTEUR 2
// ============================================================================

volatile int pulseCount2 = 0;

float flowRate2 = 0.0;

unsigned long previousMillis2 = 0;

float calibrationFactor2 = 8.57;



// ============================================================================
//                           VARIABLES SYSTEME
// ============================================================================

// Etat fuite

bool leakageDetected = false;



// ============================================================================
//                    INTERRUPTION CAPTEUR 1
// ============================================================================
//
// Cette fonction s'exécute automatiquement
// lorsqu'une impulsion est détectée
//
// ============================================================================

void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;
}



// ============================================================================
//                    INTERRUPTION CAPTEUR 2
// ============================================================================

void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}



// ============================================================================
//                              SETUP
// ============================================================================
//
// Cette fonction démarre UNE FOIS
//
// ============================================================================

void setup()
{

  // ==========================================================================
  // MONITEUR SERIE
  // ==========================================================================

  Serial.begin(115200);

  Serial.println("=======================================");
  Serial.println(" SYSTEME DETECTION FUITE D'EAU ");
  Serial.println("=======================================");



  // ==========================================================================
  // CONFIGURATION DES BROCHES
  // ==========================================================================

  pinMode(SENSOR1, INPUT_PULLUP);

  pinMode(SENSOR2, INPUT_PULLUP);

  pinMode(RELAY, OUTPUT);

  pinMode(BUZZER, OUTPUT);



  // ==========================================================================
  // ETAT INITIAL
  // ==========================================================================

  // Pompe ACTIVE

  digitalWrite(RELAY, HIGH);

  // Buzzer OFF

  digitalWrite(BUZZER, LOW);



  // ==========================================================================
  // INITIALISATION LCD
  // ==========================================================================

  Wire.begin(21, 22);

  lcd.init();

  lcd.backlight();

  lcd.setCursor(0, 0);

  lcd.print("Demarrage...");

  lcd.setCursor(0, 1);

  lcd.print("ESP32 SYSTEM");

  delay(2000);



  // ==========================================================================
  // CONFIGURATION INTERRUPTIONS
  // ==========================================================================

  attachInterrupt(
    digitalPinToInterrupt(SENSOR1),
    pulseCounter1,
    FALLING
  );

  attachInterrupt(
    digitalPinToInterrupt(SENSOR2),
    pulseCounter2,
    FALLING
  );



  // ==========================================================================
  // CONNEXION WIFI
  // ==========================================================================

  Serial.println("Connexion WiFi...");

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Connexion WiFi");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {

    delay(500);

    Serial.print(".");
  }

  Serial.println("");

  Serial.println("WiFi Connecte");



  // ==========================================================================
  // CONNEXION BLYNK
  // ==========================================================================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Connexion Blynk");

  Serial.println("Connexion Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk Connecte");



  // ==========================================================================
  // MESSAGE LCD
  // ==========================================================================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Systeme Pret");

  lcd.setCursor(0, 1);

  lcd.print("Surveillance");



  // ==========================================================================
  // NOTIFICATION DEMARRAGE
  // ==========================================================================

  Blynk.logEvent(
    "system_start",
    "Le systeme de detection a demarre"
  );
}



// ============================================================================
//                             LOOP
// ============================================================================
//
// Cette fonction tourne en boucle
//
// ============================================================================

void loop()
{

  // ==========================================================================
  // MAINTIENT CONNEXION BLYNK
  // ==========================================================================

  Blynk.run();



  // ==========================================================================
  // CALCUL DEBIT CAPTEUR 1
  // ==========================================================================

  if (millis() - previousMillis1 >= 1000)
  {

    detachInterrupt(digitalPinToInterrupt(SENSOR1));



    // ------------------------------------------------------------------------
    // CALCUL DEBIT mL/s
    // ------------------------------------------------------------------------

    flowRate1 =
      ((1000.0 / (millis() - previousMillis1))
       * pulseCount1)
      / calibrationFactor1;



    previousMillis1 = millis();

    pulseCount1 = 0;



    attachInterrupt(
      digitalPinToInterrupt(SENSOR1),
      pulseCounter1,
      FALLING
    );



    // ------------------------------------------------------------------------
    // ENVOI BLYNK
    // ------------------------------------------------------------------------

    Blynk.virtualWrite(V0, flowRate1);



    // ------------------------------------------------------------------------
    // MONITEUR SERIE
    // ------------------------------------------------------------------------

    Serial.print("Debit Entree : ");

    Serial.print(flowRate1);

    Serial.println(" mL/s");
  }



  // ==========================================================================
  // CALCUL DEBIT CAPTEUR 2
  // ==========================================================================

  if (millis() - previousMillis2 >= 1000)
  {

    detachInterrupt(digitalPinToInterrupt(SENSOR2));



    flowRate2 =
      ((1000.0 / (millis() - previousMillis2))
       * pulseCount2)
      / calibrationFactor2;



    previousMillis2 = millis();

    pulseCount2 = 0;



    attachInterrupt(
      digitalPinToInterrupt(SENSOR2),
      pulseCounter2,
      FALLING
    );



    // ------------------------------------------------------------------------
    // ENVOI BLYNK
    // ------------------------------------------------------------------------

    Blynk.virtualWrite(V1, flowRate2);



    // ------------------------------------------------------------------------
    // MONITEUR SERIE
    // ------------------------------------------------------------------------

    Serial.print("Debit Sortie : ");

    Serial.print(flowRate2);

    Serial.println(" mL/s");
  }



  // ==========================================================================
  // AFFICHAGE LCD
  // ==========================================================================

  lcd.setCursor(0, 0);

  lcd.print("E:");

  lcd.print(flowRate1, 1);

  lcd.print(" ");

  lcd.setCursor(8, 0);

  lcd.print("S:");

  lcd.print(flowRate2, 1);

  lcd.print(" ");



  // ==========================================================================
  // DETECTION FUITE
  // ==========================================================================

  // Si débit entrée supérieur débit sortie

  if ((flowRate1 - flowRate2) > 2)
  {

    leakageDetected = true;



    // ------------------------------------------------------------------------
    // MESSAGE SERIE
    // ------------------------------------------------------------------------

    Serial.println("!!!!! FUITE DETECTEE !!!!!");



    // ------------------------------------------------------------------------
    // ARRET POMPE
    // ------------------------------------------------------------------------

    digitalWrite(RELAY, LOW);



    // ------------------------------------------------------------------------
    // ACTIVATION BUZZER
    // ------------------------------------------------------------------------

    digitalWrite(BUZZER, HIGH);



    // ------------------------------------------------------------------------
    // LCD
    // ------------------------------------------------------------------------

    lcd.setCursor(0, 1);

    lcd.print("Fuite Detectee");



    // ------------------------------------------------------------------------
    // BLYNK
    // ------------------------------------------------------------------------

    Blynk.virtualWrite(V3, 1);

    Blynk.virtualWrite(V5, 1);



    // ------------------------------------------------------------------------
    // NOTIFICATION
    // ------------------------------------------------------------------------

    Blynk.logEvent(
      "flow_notify",
      "Fuite d'eau detectee"
    );
  }

  else
  {

    leakageDetected = false;



    // ------------------------------------------------------------------------
    // POMPE ACTIVE
    // ------------------------------------------------------------------------

    digitalWrite(RELAY, HIGH);



    // ------------------------------------------------------------------------
    // BUZZER OFF
    // ------------------------------------------------------------------------

    digitalWrite(BUZZER, LOW);



    // ------------------------------------------------------------------------
    // LCD
    // ------------------------------------------------------------------------

    lcd.setCursor(0, 1);

    lcd.print("Systeme Normal");



    // ------------------------------------------------------------------------
    // BLYNK
    // ------------------------------------------------------------------------

    Blynk.virtualWrite(V3, 0);

    Blynk.virtualWrite(V5, 0);
  }



  // ==========================================================================
  // SURVEILLANCE WIFI
  // ==========================================================================

  if (WiFi.status() == WL_CONNECTED)
  {

    Blynk.virtualWrite(V6, "WiFi OK");
  }

  else
  {

    Blynk.virtualWrite(V6, "WiFi OFF");

    Serial.println("WiFi Deconnecte");
  }



  // ==========================================================================
  // SURVEILLANCE BLYNK
  // ==========================================================================

  if (Blynk.connected())
  {

    Blynk.virtualWrite(V7, "Blynk OK");
  }

  else
  {

    Blynk.virtualWrite(V7, "Blynk OFF");

    Serial.println("Blynk Hors Ligne");
  }



  // ==========================================================================
  // TEMPS FONCTIONNEMENT
  // ==========================================================================

  unsigned long uptime = millis() / 1000;

  Serial.print("Temps fonctionnement : ");

  Serial.print(uptime);

  Serial.println(" secondes");



  Serial.println("-----------------------------------");



  // ==========================================================================
  // DELAI
  // ==========================================================================

  delay(1000);
}