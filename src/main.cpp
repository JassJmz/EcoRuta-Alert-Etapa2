
// ============================================
// EcoRuta Alert — Firmware ESP32-S3
// Módulos: 3.3.1 GPS + 3.3.2 Actuadores + 3.3.3 Comunicación
// Guadalupe Victoria, Durango
// ============================================

#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --------------------------------------------
// MÓDULO 3.3.1 — CONFIGURACIÓN GPS
// --------------------------------------------
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD   9600
#define INTERVALO_LECTURA 5000

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

float latitud  = 0.0;
float longitud = 0.0;
bool  gpsValido = false;
unsigned long ultimaLectura = 0;

void gpsSetup() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("[GPS] NEO-6M v2 inicializado");
  Serial.println("[GPS] Esperando señal satelital...");
}

bool leerGPS() {
  unsigned long inicio = millis();

  while (millis() - inicio < 1000) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
  }

  if (gps.location.isValid() && gps.location.isUpdated()) {
    latitud  = gps.location.lat();
    longitud = gps.location.lng();
    gpsValido = true;

    Serial.print("[GPS] Lat: ");  Serial.print(latitud, 6);
    Serial.print(" | Lng: ");     Serial.println(longitud, 6);
    Serial.print("[GPS] Satélites: "); Serial.println(gps.satellites.value());
    return true;

  } else {
    gpsValido = false;
    Serial.println("[GPS] Sin señal válida. Reintentando...");
    return false;
  }
}

// --------------------------------------------
// MÓDULO 3.3.2 — CONTROL DE ACTUADORES
// --------------------------------------------
#define LAT_MIN   24.40
#define LAT_MAX   24.50
#define LNG_MIN  -104.18
#define LNG_MAX  -104.08
#define MAX_ERRORES 5

bool envioActivo    = false;
int contadorErrores = 0;
int contadorEnvios  = 0;

bool coordenadasEnRango(float lat, float lng) {
  return (lat >= LAT_MIN && lat <= LAT_MAX &&
          lng >= LNG_MIN && lng <= LNG_MAX);
}

void controlarEnvio(bool gpsOk, float lat, float lng) {
  if (contadorErrores >= MAX_ERRORES) {
    envioActivo = false;
    Serial.println("[ACTUADOR] PARADA DE EMERGENCIA: demasiados errores");
    return;
  }

  if (gpsOk && coordenadasEnRango(lat, lng)) {
    envioActivo = true;
    contadorErrores = 0;
    Serial.println("[ACTUADOR] Envío ACTIVADO — coordenadas válidas");
  } else {
    envioActivo = false;
    contadorErrores++;
    Serial.print("[ACTUADOR] Envío DESACTIVADO — errores: ");
    Serial.println(contadorErrores);
  }
}

// --------------------------------------------
// MÓDULO 3.3.3 — COMUNICACIÓN WiFi/HTTP
// --------------------------------------------
const char* ssid     = "";
const char* password = "";
const char* serverURL = "https://webhook.site/73b0fcd6-a022-4b71-932a-5dc4c2dd6a44";

void wifiSetup() {
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Conectando");

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Conectado exitosamente");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] ERROR: No se pudo conectar");
  }
}

bool enviarCoordenadas(float lat, float lng) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] ERROR: Sin conexión WiFi");
    return false;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"proyecto\":\"EcoRuta Alert\",";
  payload += "\"lat\":"  + String(lat, 6) + ",";
  payload += "\"lng\":"  + String(lng, 6) + ",";
  payload += "\"satelites\":" + String(gps.satellites.value()) + ",";
  payload += "\"ts\":"   + String(millis());
  payload += "}";

  Serial.print("[HTTP] Enviando: "); Serial.println(payload);

  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    Serial.println("[HTTP] Datos enviados correctamente");
    contadorEnvios++;
    http.end();
    return true;
  } else {
    Serial.print("[HTTP] ERROR código: ");
    Serial.println(httpCode);
    http.end();
    return false;
  }
}

// --------------------------------------------
// SETUP Y LOOP PRINCIPAL
// --------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("=================================");
  Serial.println("  EcoRuta Alert — Firmware v1.0");
  Serial.println("  Guadalupe Victoria, Durango");
  Serial.println("=================================");
  gpsSetup();
  wifiSetup();
}

void loop() {
  unsigned long ahora = millis();

  if (ahora - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = ahora;

    Serial.println("\n--- Ciclo de lectura ---");

    // PASO 1: Leer GPS
    bool lecturaOk = leerGPS();

    // PASO 2: Validar y controlar envío
    controlarEnvio(lecturaOk, latitud, longitud);

    // PASO 3: Enviar si está activo
    if (envioActivo) {
      bool enviado = enviarCoordenadas(latitud, longitud);
      Serial.print("[LOOP] Total envíos exitosos: ");
      Serial.println(contadorEnvios);
    }

    Serial.println("--- Fin de ciclo ---");
  }

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}