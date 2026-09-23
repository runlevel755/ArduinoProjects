# Mis Proyectos de Arduino 🚀

¡Bienvenido a mi repositorio personal de Arduino! Aquí guardo y organizo todos los códigos, circuitos y experimentos que voy desarrollando.

---

## 📚 Librerías Requeridas

Para poder compilar los proyectos de este repositorio, asegúrate de tener instaladas las siguientes librerías desde el **Gestor de Librerías** del Arduino IDE:

*   **IRremote** (por Armin Joachimsmeyer) – Utilizada para decodificar señales de controles remotos por infrarrojos.
*   **MFRC522** (por githubcommunity) – Utilizada para leer y escribir tarjetas/llaveros RFID mediante el módulo RC522.

---

## 💡 Proyecto Destacado: Control de LED por Infrarrojos (IR)

Este proyecto consiste en conectar un receptor IR y un LED a un **Arduino Nano** para controlar el encendido, apagado o la intensidad del LED utilizando un mando a distancia.

### 🔌 Circuito en la Protoboard
El circuito está diseñado protegiendo el LED mediante una resistencia en el ánodo, actuando como un puente para evitar el paso directo de corriente sin filtrar:

*   **Pin Digital 9 (Arduino)** ➡️ Conectado a la *Fila 50, Columna D*.
*   **Resistencia (220 Ω)** ➡️ Conectada entre la *Fila 50, Columna C* y la *Fila 45, Columna C* (hace de puente).
*   **Ánodo del LED (Pata larga)** ➡️ Conectado a la *Fila 45, Columna A*.
*   **Cátodo del LED (Pata corta)** ➡️ Conectado directamente a la línea de **GND (-)** del Arduino.

### 📡 Especificaciones del Control Remoto
Las señales de este proyecto han sido analizadas mediante el receptor IR arrojando los siguientes datos técnicos:
*   **Protocolo:** NEC Extendido (16 bits de dirección)
*   **Dirección (Address):** `0xB708`
*   **Comando del botón (Command):** `0x10` (Corresponde a la trama de datos puros `0xEF10B708`)

---

## 🛠️ Notas de Configuración (Hardware clon/compatible)
Si utilizas una placa Arduino Nano compatible y el IDE no la detecta o da error al subir el código:
1. Asegúrate de tener instalado el driver **CH340**.
2. En el menú del Arduino IDE, ve a *Herramientas > Procesador* y selecciona **ATmega328P (Old Bootloader)**.
