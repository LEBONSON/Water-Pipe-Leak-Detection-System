// =========================================================================
//        SYSTEME DE DETECTION DE FUITE D'EAU AVEC ESP32 + BLYNK IoT
// =========================================================================
// Auteur : LEBONI BAKLA LIONEL
// Carte : ESP32 ESP-WROOM-32
// Fonctionnalités :
// - Surveillance de 2 capteurs de débit
// - Détection de fuite
// - Contrôle automatique de pompe
// - Notifications Blynk
// - Affichage LCD I2C
// - Surveillance connexion WiFi/Blynk
// =========================================================================

// ======================= BLYNK CONFIGURATION =============================
#define BLYNK_TEMPLATE_ID   "" // Bien vouloir renseigner votre propre Template ID
#define BLYNK_TEMPLATE_NAME "Water pipe leak detection system"
#define BLYNK_AUTH_TOKEN    ""  // Bien vouloir renseigner votre propre Auth Token

#define BLYNK_PRINT Serial

// ======================= LIBRAIRIES ======================================
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

// ======================= WIFI ============================================
char ssid[] = "";  // Bien vouloir renseigner votre propre SSID
char pass[] = "";  // Bien vouloir renseigner votre propre mot de passe WiFi

// ======================= LCD =============================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======================= BROCHES ESP32 ===================================
#define SENSOR1 27
#define SENSOR2 14

#define RELAY   18
#define BUZZER  19

#define SDA_LCD 21
#define SCL_LCD 22

// ======================= VARIABLES CAPTEURS ==============================

// -------- Capteur 1 ----------
volatile byte pulseCount1 = 0;

float calibrationFactor1 = 6.0;
float flowRate1 = 0.0;
unsigned long oldTime1 = 0;

// -------- Capteur 2 ----------
volatile byte pulseCount2 = 0;

float calibrationFactor2 = 6.0;
float flowRate2 = 0.0;
unsigned long oldTime2 = 0;

// ======================= ETAT SYSTEME ====================================
bool leakageDetected = false;
bool blynkConnected = false;

// ======================= INTERRUPTIONS ===================================
void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;
}

void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}

// ======================= BLYNK CONNECT ===================================
BLYNK_CONNECTED()
{
  Serial.println("Blynk Connecte");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Blynk Connecte");
  lcd.setCursor(0, 1);
  lcd.print("Systeme Pret");

  Blynk.logEvent("system_connected", "ESP32 connecte a Blynk");

  blynkConnected = true;
}

// ======================= SETUP ===========================================
void setup()
{
  Serial.begin(115200);

  // ================= LCD ==================
  Wire.begin(SDA_LCD, SCL_LCD);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Demarrage...");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 SYSTEM");

  delay(2000);

  // ================= BROCHES ==============
  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);

  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Etat initial
  digitalWrite(RELAY, HIGH);
  digitalWrite(BUZZER, LOW);

  // ================= INTERRUPTIONS ========
  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);

  // ================= WIFI =================
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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecte");

  delay(1500);

  // ================= BLYNK ================
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connexion Blynk");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Systeme Pret");

  Blynk.logEvent("system_start", "Systeme de detection demarre");
}

// ======================= LOOP ============================================
void loop()
{
  Blynk.run();

  // ================= VERIFICATION WIFI ==================
  if (WiFi.status() != WL_CONNECTED)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Perdu");

    Serial.println("WiFi Deconnecte");

    digitalWrite(BUZZER, HIGH);

    delay(500);

    digitalWrite(BUZZER, LOW);

    return;
  }

  // ================= CALCUL CAPTEUR 1 ===================
  if ((millis() - oldTime1) > 1000)
  {
    detachInterrupt(digitalPinToInterrupt(SENSOR1));

    flowRate1 = ((1000.0 / (millis() - oldTime1)) * pulseCount1) / calibrationFactor1;

    oldTime1 = millis();

    pulseCount1 = 0;

    attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);

    // ENVOI BLYNK
    Blynk.virtualWrite(V0, flowRate1);

    Serial.print("Debit 1 : ");
    Serial.print(flowRate1);
    Serial.println(" L/min");
  }

  // ================= CALCUL CAPTEUR 2 ===================
  if ((millis() - oldTime2) > 1000)
  {
    detachInterrupt(digitalPinToInterrupt(SENSOR2));

    flowRate2 = ((1000.0 / (millis() - oldTime2)) * pulseCount2) / calibrationFactor2;

    oldTime2 = millis();

    pulseCount2 = 0;

    attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);

    // ENVOI BLYNK
    Blynk.virtualWrite(V1, flowRate2);

    Serial.print("Debit 2 : ");
    Serial.print(flowRate2);
    Serial.println(" L/min");
  }

  // ================= AFFICHAGE LCD ======================
  lcd.setCursor(0, 0);
  lcd.print("F1:");
  lcd.print(flowRate1, 1);
  lcd.print("L ");

  lcd.setCursor(9, 0);
  lcd.print("F2:");
  lcd.print(flowRate2, 1);

  // ================= DETECTION FUITE ====================
  if ((flowRate1 - flowRate2) > 2.0 && flowRate1 > 2)
  {
    if (!leakageDetected)
    {
      leakageDetected = true;

      Serial.println("FUITE DETECTEE");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("!!! FUITE !!!");

      lcd.setCursor(0, 1);
      lcd.print("Pompe Stoppee");

      // Relais OFF
      digitalWrite(RELAY, LOW);

      // Buzzer ON
      digitalWrite(BUZZER, HIGH);

      // Blynk
      Blynk.virtualWrite(V3, 1);
      Blynk.virtualWrite(V5, 1);

      Blynk.logEvent("flow_notify", "Fuite d'eau detectee !");
    }
  }
  else
  {
    if (leakageDetected)
    {
      leakageDetected = false;

      Serial.println("SYSTEME NORMAL");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Systeme Normal");

      // Pompe ON
      digitalWrite(RELAY, HIGH);

      // Buzzer OFF
      digitalWrite(BUZZER, LOW);

      // Blynk
      Blynk.virtualWrite(V3, 0);
      Blynk.virtualWrite(V5, 0);

      Blynk.logEvent("system_normal", "Systeme revenu a la normale");
    }
  }

  delay(500);
}