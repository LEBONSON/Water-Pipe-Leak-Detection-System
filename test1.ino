// =========================================================================
// 1. IDENTIFIANTS BLYNK IoT
// =========================================================================
#define BLYNK_TEMPLATE_ID   ""
#define BLYNK_TEMPLATE_NAME "Water pipe leak detection system"
#define BLYNK_AUTH_TOKEN    ""
#define BLYNK_PRINT Serial

// =========================================================================
// 2. BIBLIOTHÈQUES
// =========================================================================
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// =========================================================================
// 3. CONFIGURATION
// =========================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

char ssid[] = "";
char pass[] = "";

#define SENSOR1  27
#define SENSOR2  14
#define RELAY    18
#define BUZZER   19

// =========================================================================
// 4. VARIABLES (avec protection)
// =========================================================================
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;
float flowRate1 = 0;
float flowRate2 = 0;
unsigned long lastRead1 = 0;
unsigned long lastRead2 = 0;
bool wifiConnected = false;

// =========================================================================
// 5. INTERRUPTIONS (très légères)
// =========================================================================
void IRAM_ATTR pulseCounter1() {
  pulseCount1++;
}

void IRAM_ATTR pulseCounter2() {
  pulseCount2++;
}

// =========================================================================
// 6. SETUP - Initialisation progressive
// =========================================================================
void setup() {
  Serial.begin(115200);
  delay(2000);  // CRUCIAL : attendre 2 secondes
  
  Serial.println();
  Serial.println("=== SYSTEME DETECTION FUITES EAU ===");
  
  // ---- ÉTAPE 1: Broches (sans interruptions) ----
  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(RELAY, LOW);
  digitalWrite(BUZZER, LOW);
  
  // ---- ÉTAPE 2: LCD (avec vérification) ----
  Wire.begin();
  delay(100);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Leakage");
  lcd.setCursor(0, 1);
  lcd.print("Monitor System");
  delay(2000);
  
  // ---- ÉTAPE 3: WiFi (avec timeout) ----
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi...");
  
  Serial.print("Connexion WiFi...");
  WiFi.begin(ssid, pass);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println(" OK !");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println(" FAIL");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi FAIL");
    lcd.setCursor(0, 1);
    lcd.print("Check network");
    delay(3000);
  }
  
  // ---- ÉTAPE 4: Blynk (seulement si WiFi OK) ----
  if (wifiConnected) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Blynk...");
    
    Serial.print("Blynk...");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    delay(2000);
    Serial.println(" OK");
    lcd.setCursor(0, 1);
    lcd.print("Connected");
    delay(1000);
  }
  
  // ---- ÉTAPE 5: Interruptions (EN TOUT DERNIER) ----
  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
  
  lcd.clear();
  Serial.println("Systeme pret !");
  lcd.setCursor(0, 0);
  lcd.print("Ready");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring...");
  delay(1000);
  lcd.clear();
}

// =========================================================================
// 7. LOOP - Lecture capteurs (sans blocage)
// =========================================================================
void loop() {
  // Maintien connexion Blynk
  if (wifiConnected) {
    Blynk.run();
  }
  
  unsigned long now = millis();
  
  // ---- CAPTEUR 1 (Entrée) ----
  if (now - lastRead1 >= 1000) {
    // Lecture sécurisée des pulses
    noInterrupts();
    int count1 = pulseCount1;
    pulseCount1 = 0;
    interrupts();
    
    // Calcul débit (L/min)
    flowRate1 = count1 / 6.0;
    lastRead1 = now;
    
    // Affichage série
    Serial.print("E: ");
    Serial.print(flowRate1, 1);
    Serial.print(" L/min  ");
    
    // Affichage LCD ligne 0
    lcd.setCursor(0, 0);
    lcd.print("E:");
    lcd.print(flowRate1, 1);
    lcd.print(" L/min  ");
    
    // Envoi Blynk
    if (wifiConnected) {
      Blynk.virtualWrite(V1, flowRate1);
    }
  }
  
  // ---- CAPTEUR 2 (Sortie) ----
  if (now - lastRead2 >= 1000) {
    // Lecture sécurisée des pulses
    noInterrupts();
    int count2 = pulseCount2;
    pulseCount2 = 0;
    interrupts();
    
    // Calcul débit (L/min)
    flowRate2 = count2 / 6.0;
    lastRead2 = now;
    
    // Affichage série
    Serial.print("S: ");
    Serial.print(flowRate2, 1);
    Serial.println(" L/min");
    
    // Affichage LCD ligne 1
    lcd.setCursor(0, 1);
    lcd.print("S:");
    lcd.print(flowRate2, 1);
    lcd.print(" L/min  ");
    
    // Envoi Blynk
    if (wifiConnected) {
      Blynk.virtualWrite(V2, flowRate2);
    }
  }
  
  // ---- DETECTION FUITE ----
  if (flowRate1 > 1.0 && flowRate2 < (flowRate1 - 0.5)) {
    // ALERTE FUITE
    digitalWrite(RELAY, LOW);
    digitalWrite(BUZZER, HIGH);
    
    // Affichage alerte (mais pas trop souvent)
    static unsigned long lastAlert = 0;
    if (now - lastAlert > 2000) {
      lastAlert = now;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("!!! FUITE !!!");
      lcd.setCursor(0, 1);
      lcd.print("POMPE COUPEE");
      Serial.println("*** ALERTE FUITE DETECTEE ***");
      
      if (wifiConnected) {
        Blynk.virtualWrite(V3, 1);
        Blynk.virtualWrite(V5, 1);
        Blynk.logEvent("leak_alert", "Fuite d'eau detectee !");
      }
    }
  } else {
    // PAS DE FUITE
    digitalWrite(RELAY, HIGH);
    digitalWrite(BUZZER, LOW);
    
    if (wifiConnected) {
      Blynk.virtualWrite(V3, 0);
      Blynk.virtualWrite(V5, 0);
    }
  }
  
  delay(50);  // Petit délai pour stabiliser
}