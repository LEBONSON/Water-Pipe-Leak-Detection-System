Voici un README professionnel, complet et prêt pour GitHub/GitLab pour votre projet IoT de détection de fuite d’eau avec ESP32 et Blynk.

# 💧 Système Intelligent de Détection de Fuite d’Eau

## ESP32 • Blynk IoT • Surveillance Temps Réel • Sécurité Automatique

---

# 📌 Présentation du Projet

Ce projet consiste à développer un système IoT intelligent capable de :

* surveiller en temps réel le débit d’eau dans une canalisation ;
* détecter automatiquement une fuite ;
* couper instantanément une pompe à eau ;
* avertir localement et à distance l’utilisateur ;
* afficher les informations sur écran LCD, application mobile et dashboard web Blynk.

Le système utilise un microcontrôleur **ESP32**, deux capteurs de débit à effet Hall et la plateforme **Blynk IoT Cloud**.

---

# 🎯 Objectifs du Projet

Le projet permet :

✅ La surveillance continue d’un réseau hydraulique
✅ La détection précoce des fuites
✅ La réduction des pertes d’eau
✅ La sécurisation automatique du système
✅ La supervision locale et distante
✅ L’automatisation industrielle ou domestique

---

# ⚙️ Fonctionnalités Principales

## ✅ Surveillance Temps Réel

Le système mesure :

* le débit d’entrée (`Flow 1`)
* le débit de sortie (`Flow 2`)

Unité utilisée :

```text
mL/s (millilitres par seconde)
```

---

## ✅ Détection Automatique de Fuite

Le système compare les deux débits :

```text
Si Flow1 > Flow2
```

et si la différence dépasse un seuil défini :

* la pompe est arrêtée ;
* le buzzer est activé ;
* une notification Blynk est envoyée ;
* le LCD affiche l’alerte ;
* le dashboard Blynk est mis à jour.

---

## ✅ Affichage Local LCD I2C

L’écran LCD affiche :

```text
F1: 5.2 mL/s
F2: 4.9 mL/s
```

---

## ✅ Supervision Cloud avec Blynk IoT

Compatible avec :

* 📱 Application mobile Blynk
* 🌐 Dashboard Web Blynk
* 🔔 Notifications Push
* 📊 Jauges Temps Réel

---

## ✅ Moniteur Série Arduino

Le système affiche :

* débit entrée ;
* débit sortie ;
* état WiFi ;
* état Blynk ;
* état pompe ;
* état buzzer ;
* état fuite ;
* temps de fonctionnement ;
* adresse IP.

---

# 🧠 Architecture du Système

```text
Capteur 1 ─────► ESP32 ◄───── Capteur 2
                     │
                     │
          ┌──────────┴──────────┐
          │                     │
       Relais                LCD I2C
          │
       Pompe 12V
          │
       Buzzer
          │
       Blynk IoT
```

---

# 🛠️ Composants Matériels

| Composant                | Quantité  |
| ------------------------ | --------- |
| ESP32 ESP-WROOM-32       | 1         |
| Capteur de débit YF-S201 | 2         |
| LCD I2C 16x2             | 1         |
| Module relais 5V         | 1         |
| Buzzer actif             | 1         |
| Mini pompe à eau 12V     | 1         |
| Alimentation 12V         | 1         |
| Breadboard               | 1         |
| Fils Dupont              | Plusieurs |

---

# 🔌 Connexions ESP32

| Composant       | GPIO ESP32 |
| --------------- | ---------- |
| Capteur Débit 1 | GPIO27     |
| Capteur Débit 2 | GPIO14     |
| Relais Pompe    | GPIO18     |
| Buzzer          | GPIO19     |
| LCD SDA         | GPIO21     |
| LCD SCL         | GPIO22     |

---

# ☁️ Configuration Blynk IoT

## Informations du Template

```cpp
#define BLYNK_TEMPLATE_ID "TMPL2suV0_Vm9"
#define BLYNK_TEMPLATE_NAME "Water pipe leak detection system"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"
```

---

# 📲 Datastreams Virtuels

| Virtual Pin | Fonction        |
| ----------- | --------------- |
| V0          | Débit Capteur 1 |
| V1          | Débit Capteur 2 |
| V3          | Statut Fuite    |
| V5          | Statut Buzzer   |
| V6          | État WiFi       |
| V7          | État Blynk      |

---

# 📡 Événements Blynk

Créer les événements suivants :

| Nom Événement  | Code           |
| -------------- | -------------- |
| System Start   | `system_start` |
| Leak Detection | `flow_notify`  |

---

# 🧮 Logique de Détection

```text
SI (Flow1 - Flow2 > 2 mL/s)

ALORS :

→ Arrêter la pompe
→ Activer le buzzer
→ Envoyer notification
→ Mettre dashboard à jour
```

---

# 💻 Moniteur Série Arduino

Configurer :

| Paramètre    | Valeur       |
| ------------ | ------------ |
| Baud Rate    | 115200       |
| Fin de ligne | Both NL & CR |

---

# 📚 Bibliothèques Requises

Installer depuis Arduino IDE :

* WiFi
* WiFiClient
* Blynk
* LiquidCrystal_I2C
* Wire

---

# 🔧 Installation du Projet

## 1️⃣ Installer Arduino IDE

Télécharger :

[https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

---

## 2️⃣ Ajouter ESP32

Dans :

```text
Fichier → Préférences
```

Ajouter :

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

---

## 3️⃣ Installer les bibliothèques

Depuis :

```text
Croquis → Inclure une bibliothèque → Gérer les bibliothèques
```

Installer :

* Blynk
* LiquidCrystal_I2C

---

## 4️⃣ Configurer le WiFi

Modifier :

```cpp
char ssid[] = "Votre_WiFi";
char pass[] = "MotDePasse";
```

---

## 5️⃣ Téléverser le Code

Sélectionner :

```text
ESP32 Dev Module
```

Puis téléverser.

---

# 🧪 Dépannage LCD

## Si le LCD bloque :

### Causes possibles :

* alimentation insuffisante ;
* bruit électrique de la pompe ;
* adresse I2C incorrecte ;
* utilisation excessive de `lcd.clear()`.

---

## Solutions :

✅ Utiliser alimentation externe 12V
✅ Ajouter condensateur 1000uF
✅ Vérifier adresse I2C (`0x27` ou `0x3F`)
✅ Éviter `lcd.clear()` dans `loop()`
✅ Utiliser masse commune GND

---

# ⚡ Recommandations d’Alimentation

| Équipement | Alimentation         |
| ---------- | -------------------- |
| ESP32      | USB                  |
| Pompe 12V  | Alimentation Externe |
| Relais     | 5V                   |

⚠️ IMPORTANT :

Toutes les masses GND doivent être connectées ensemble.

---

# 📈 Calibration des Capteurs

Valeur par défaut :

```cpp
float calibrationFactor = 8.57;
```

Cette valeur dépend du modèle de capteur utilisé.

---

# 🛡️ Sécurité Intégrée

Le système protège :

✅ la pompe ;
✅ le circuit hydraulique ;
✅ les pertes d’eau ;
✅ les surconsommations ;
✅ les fuites invisibles.

---

# 🚀 Améliorations Futures

* Intelligence artificielle prédictive
* Historique Cloud
* Dashboard avancé
* Notifications Telegram
* SMS GSM
* Batterie de secours
* Contrôle vocal
* MQTT
* Firebase
* Node-RED

---

# 🌍 Applications Possibles

* Maisons intelligentes
* Agriculture intelligente
* Réservoirs d’eau
* Systèmes industriels
* Irrigation automatique
* Smart Cities

---

# 👨‍💻 Auteur

Projet développé avec :

* ESP32
* Arduino IDE
* Blynk IoT
* C++
* IoT Embedded Systems

---

# 📄 Licence

Projet open-source destiné à :

* l’éducation ;
* la recherche ;
* l’apprentissage IoT ;
* les projets embarqués.

Utilisation libre à des fins pédagogiques.

---

# ⭐ Support

Si ce projet vous aide :

⭐ Ajouter une étoile au dépôt GitHub
🍴 Forker le projet
📢 Partager le projet

---

# 📬 Contact

Projet IoT Embedded Systems
ESP32 • Blynk • Water Monitoring • Smart Detection
