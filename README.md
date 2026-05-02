# EcoRuta Alert — Firmware ESP32-S3

Sistema de geolocalización GPS para monitoreo de rutas de recolección de residuos sólidos urbanos en Guadalupe Victoria, Durango.

## Descripción

Firmware desarrollado en C/C++ con PlatformIO para ESP32-S3. Lee coordenadas GPS en tiempo real mediante el módulo NEO-6M v2 y las envía a un servidor vía WiFi/HTTP POST en formato JSON.

## Hardware

| Componente | Descripción |
|---|---|
| ESP32-S3 DevKit | Microcontrolador principal |
| GPS NEO-6M v2 | Sensor de posición GNSS |

## Conexiones

| Pin ESP32-S3 | Pin GPS |
|---|---|
| GPIO 16 (RX2) | TX |
| GPIO 17 (TX2) | RX |
| 3.3V | VCC |
| GND | GND |

## Módulos del firmware

- **3.3.1** Lectura de sensor GPS (TinyGPS++ / UART2 / 9600 baud)
- **3.3.2** Control de actuadores (validación ON/OFF condicional)
- **3.3.3** Comunicación WiFi/HTTP POST con payload JSON
- **3.3.4** Loop principal (ciclo cada 5 segundos)

## Instalación

1. Clonar el repositorio
2. Abrir con PlatformIO en Visual Studio Code
3. Configurar credenciales WiFi en `src/main.cpp`
4. Build y Upload al ESP32-S3

## Equipo

- Marco Isai Palacio Lopez
- Tania Elizabeth Serrano Hernandez
- Jassiel Obed Jimenez Ochoa
- Bryan Emmanuel Barboza Carrillo

**Instituto Tecnológico Superior de la Región de los Llanos — 2026**
