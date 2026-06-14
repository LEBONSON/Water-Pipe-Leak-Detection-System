
# 💧 Système Intelligent de Détection de Fuite d’Eau

**ESP32 · Blynk IoT · Surveillance Temps Réel · Sécurité Automatique**

**Auteur** : INGÉNIEUR DE GÉNIE NUMÉRIQUE – LEBONI BAKLA LIONEL  
**Contact** : [lebonilionel@gmail.com](mailto:lebonilionel@gmail.com)  
**GitHub** : [LEBONSON/Water-Pipe-Leak-Detection-System](https://github.com/LEBONSON/Water-Pipe-Leak-Detection-System)

---

## 📌 Contexte et histoire du projet

Ce projet a été initialement **demandé par un professeur** à un étudiant en Master Télécommunications. Le délai était **extrêmement court : 5 jours**, et le budget très serré.

Nous avons fait **ce que nous avons pu avec les moyens du bord** :
- Composants disponibles sur le **marché local** (pompe 12V au lieu de 220V, petits câbles, capteurs alternatifs).
- Adaptations matérielles en urgence pour que le système fonctionne malgré les contraintes.

**État du projet** :  
⚠️ Le projet n’est **pas totalement finalisé** (manque de temps et budget).  
✅ Mais la **preuve de concept est là** : nous recevons les données en temps réel sur trois interfaces (mobile Blynk, écran LCD, moniteur Arduino) et la logique de détection de fuite par zone fonctionne.

Ce projet m’a permis, à moi personnellement (ingénieur passionné de **cybersécurité offensive et défensive**, de SOC, de bug bounty, de sécurité des API, de DevOps, bref de **toute la tech**), de renouer avec l’IoT après 3 ans d’absence. Une émotion et une fierté – et l’espoir que, dans les années à venir, l’IoT devienne aussi un terrain de **bug bounty**.

---

## 🎯 Objectifs du projet

Le système vise à :

- ✅ Surveiller en continu le débit d’eau dans une canalisation (entrée vs sortie).
- ✅ Détecter automatiquement une fuite par différence de débit.
- ✅ Couper instantanément la pompe à eau via un relais.
- ✅ Avertir localement (écran LCD, buzzer) et à distance (notification Blynk).
- ✅ Afficher les mesures sur application mobile et dashboard web Blynk.

---

## ⚙️ Fonctionnalités principales

### Surveillance temps réel
- Mesure du débit d’entrée (`Flow1`) et de sortie (`Flow2`)
- Unité : `mL/s` (millilitres par seconde)

### Détection automatique de fuite
```
SI Flow1 - Flow2 > seuil (2 mL/s) ALORS :
   → Arrêt pompe (relais HIGH)
   → Buzzer activé
   → Notification Blynk "Water Leakage Detected!"
   → LCD affiche alerte
   → Dashboard Blynk mis à jour
```

### Affichage local LCD I2C
Exemple :
```
F1: 5.2 mL/s
F2: 4.9 mL/s
```

### Supervision cloud Blynk
- 📱 Application mobile
- 🌐 Dashboard Web
- 🔔 Notifications push
- 📊 Jauges temps réel

### Moniteur série Arduino
Affiche : débits, état WiFi, état Blynk, état pompe/buzzer, état fuite, temps de fonctionnement, adresse IP.

---

## 🧠 Architecture du système (schéma fonctionnel)

```
Capteur 1 (entrée) ──► ESP32 ◄── Capteur 2 (sortie)
                           │
            ┌──────────────┼──────────────┐
            │              │              │
         Relais          LCD I2C       Buzzer
            │
         Pompe 12V
            │
         Blynk IoT Cloud
```

---

## 🛠️ Composants matériels (et adaptations locales)

| Composant                   | Quantité | Remarque adaptation |
|-----------------------------|----------|----------------------|
| ESP32 ESP-WROOM-32          | 1        |                      |
| Capteur de débit (type YF-S201) | 2    | Trouvés en version "petits câbles" |
| LCD I2C 16x2                | 1        |                      |
| Module relais 5V            | 1        |                      |
| Buzzer actif                | 1        |                      |
| Pompe à eau                 | 1        | **12V** (prévue en 220V, adapté faute de mieux) |
| Alimentation 12V            | 1        |                      |
| Breadboard + fils Dupont    | -        | Fils de petite section |

> ⚠️ **Contrainte locale** : nous n’avons pas trouvé de pompe 220V, ni de câbles standards. Nous avons adapté le circuit avec une pompe 12V et des petits câbles – cela fonctionne, mais l’alimentation doit être soigneusement découplée.

---

## 🔌 Connexions ESP32 (pinout réel)

| Composant             | GPIO ESP32 |
|-----------------------|------------|
| Capteur débit 1 (entrée)  | GPIO27     |
| Capteur débit 2 (sortie)  | GPIO14     |
| Relais pompe              | GPIO18     |
| Buzzer                    | GPIO19     |
| LCD I2C SDA               | GPIO21     |
| LCD I2C SCL               | GPIO22     |

> Toutes les masses (GND) doivent être connectées ensemble.

---

## ☁️ Configuration Blynk IoT

### Template
```cpp
#define BLYNK_TEMPLATE_ID   "TMPL2suV0_Vm9"
#define BLYNK_TEMPLATE_NAME "Water pipe leak detection system"
#define BLYNK_AUTH_TOKEN    "BIGNOOGejB7XN5F-aAhRD_gQl3rqTEyE"
```

### Datastreams virtuels

| Virtual Pin | Fonction               |
|-------------|------------------------|
| V1          | Débit capteur entrée   |
| V2          | Débit capteur sortie   |
| V3          | Statut fuite (0/1)     |
| V5          | Statut buzzer          |

### Événements Blynk
- `system_start` – au démarrage
- `flow_notify` – lors d’une fuite détectée

---

## 🧮 Logique de détection (extrait du code)

```cpp
if ((flowRate2 < flowRate1) && (flowRate1 > 2.0)) {
    digitalWrite(relayPin, HIGH);   // coupe pompe
    digitalWrite(buzzerPin, HIGH);  // alarme
    Blynk.logEvent("flow_notify", "Water Leakage Detected!");
    Blynk.virtualWrite(V3, 1);
} else {
    digitalWrite(relayPin, LOW);
    digitalWrite(buzzerPin, LOW);
    Blynk.virtualWrite(V3, 0);
}
```

---

## 💻 Installation et utilisation

### 1. Installer Arduino IDE
[Télécharger Arduino IDE](https://www.arduino.cc/en/software)

### 2. Ajouter le support ESP32
Dans `Fichier → Préférences`, ajouter :
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Puis installer via `Gestionnaire de cartes`.

### 3. Installer les bibliothèques
- Blynk
- LiquidCrystal_I2C
- Wire

### 4. Configurer le WiFi
Modifier dans le code :
```cpp
char ssid[] = "Votre_SSID";
char pass[] = "Votre_PASSWORD";
```

### 5. Téléverser le code
Sélectionner la carte `ESP32 Dev Module`, puis `Téléverser`.

---

## 🧪 Dépannage (troubleshooting)

| Problème                        | Solution                                 |
|---------------------------------|------------------------------------------|
| Caractères illisibles sur série | Vitesse = 115200 baud                    |
| ESP32 redémarre en boucle       | Alimentation externe pour pompe 12V, condensateur 1000µF |
| LCD ne s’affiche pas            | Vérifier adresse I2C (0x27 ou 0x3F), GND commun |
| Blynk ne se connecte pas        | Utiliser `WiFi.begin()` + `Blynk.config()` (non bloquant) |

---

## ⚡ Recommandations d’alimentation

| Équipement   | Alimentation conseillée |
|--------------|--------------------------|
| ESP32        | USB (ou 5V régulé)       |
| Pompe 12V    | Alimentation externe 12V |
| Relais       | 5V (alimentation ESP32 ou externe) |

**Important** : toutes les masses (GND) doivent être reliées entre elles.

---

## 📈 Calibration des capteurs

Dans le code :
```cpp
float calibrationFactor = 8.57;   // à ajuster selon votre capteur
```
Cette valeur dépend du modèle de capteur (typiquement YF-S201 = 7.5, d’autres = 8.57).

---

## 🚀 Améliorations futures (si le projet est repris)

- Remplacer pompe 12V par pompe 220V.
- Ajouter mémoire flash pour historique.
- Connectivité 4G pour sites sans WiFi.
- Intelligence artificielle prédictive.
- Notifications Telegram / SMS.
- Dashboard plus complet.
- Boîtier étanche.

> Espoir : que l’étudiant bénéficiaire (qui a eu une excellente note) revienne plus tard pour optimiser et finaliser le projet.

---

## 🌍 Applications possibles

- Maisons intelligentes (détection fuite cave/salle de bain)
- Agriculture / irrigation
- Réservoirs et citernes
- Systèmes industriels
- Smart cities

---

## 🛡️ Sécurité intégrée (dans le système)

Le dispositif protège :
- la pompe contre la surchauffe (marche à sec)
- le réseau hydraulique contre les fuites importantes
- contre les pertes d’eau et surconsommation

> Note personnelle : je suis passionné de **cybersécurité offensive et défensive** (SOC, bug bounty, sécurité des API, DevOps). Ce projet IoT m’a fait plaisir – et j’imagine que, dans quelques années, l’IoT sera aussi une cible de bug bounty. Pour l’instant, je reste focus sur la cybersécurité traditionnelle, mais sans oublier cette expérience.

---

## 👨‍💻 Auteur et contact

**LEBONI BAKLA LIONEL**  
Ingénieur de Génie Numérique  
Passions : IoT, cybersécurité, bug bounty, DevOps, SOC, API security.  

📧 [lebonilionel@gmail.com](mailto:lebonilionel@gmail.com)  
🔗 [GitHub – Water Pipe Leak Detection System](https://github.com/LEBONSON/Water-Pipe-Leak-Detection-System)

---

## 📄 Licence

Projet open-source à vocation pédagogique et de recherche.  
Utilisation libre pour l’apprentissage, l’éducation ou les projets embarqués.

---

## ⭐ Support

Si ce projet vous est utile :
- ⭐ Mettez une étoile sur GitHub
- 🍴 Forkez-le
- 📢 Partagez-le

---

**Dernière mise à jour** : Mai 2026
```
