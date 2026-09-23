#include <SPI.h>
#include <MFRC522.h>
#include <IRremote.hpp>

// =====================================================
// PINES
// =====================================================

#define RFID_SS_PIN 53
#define RFID_RST_PIN 5
#define IR_SEND_PIN 9

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// =====================================================
// UID DE LOS RFID
// =====================================================

// Tarjeta principal → ON / OFF
byte UID_TARJETA[] = {
  0xE9, 0xBC, 0xE5, 0x6E
};

// Llavero → ROJO / BLANCO
byte UID_LLV[] = {
  0xDB, 0x5E, 0x9F, 0x1B
};

// Segunda tarjeta → BRILLO
byte UID_BRILLO[] = {
  0x94, 0x97, 0x1E, 0xD2
};

// =====================================================
// PROTOCOLO PINRUTAR
// =====================================================

#define PINRUTAR_ADDRESS 0xB708

#define CMD_ON           0x00
#define CMD_OFF          0x02
#define CMD_RED          0x10
#define CMD_WHITE        0x08
#define CMD_BRIGHT_DOWN  0x0D
#define CMD_BRIGHT_UP    0x0E

// =====================================================
// ESTADO DE LA BOMBILLA
// =====================================================

struct EstadoBombilla {
  bool encendida;
  bool roja;
  byte brillo;
};

// Estado inicial conocido:
// APAGADA + ROJA + BRILLO BAJO
EstadoBombilla estado = {
  false,
  true,
  0
};

// brillo:
// 0 = BAJO
// 1 = MEDIO
// 2 = ALTO

// =====================================================
// COMPARAR UID
// =====================================================

bool compararUID(byte *uid1, byte *uid2) {

  for (byte i = 0; i < 4; i++) {

    if (uid1[i] != uid2[i]) {
      return false;
    }
  }

  return true;
}

// =====================================================
// ENVIAR COMANDO PINRUTAR
// =====================================================

void enviarPinrutar(byte comando) {

  IrSender.sendNEC(PINRUTAR_ADDRESS, comando, 0);

  delay(200);
}

// =====================================================
// RESTAURAR BRILLO
// =====================================================
//
// Al cambiar de color, la bombilla se pone
// automáticamente en BRILLO ALTO.
//
// Esta función vuelve al nivel que tenía
// antes del cambio de color.
//
// 0 = BAJO  → 2 pulsaciones DOWN
// 1 = MEDIO → 1 pulsación DOWN
// 2 = ALTO  → ninguna pulsación
//
// =====================================================

void restaurarBrillo(byte brilloObjetivo) {

  if (brilloObjetivo == 0) {

    Serial.println("    Restaurando brillo BAJO...");

    enviarPinrutar(CMD_BRIGHT_DOWN);

    delay(200);

    enviarPinrutar(CMD_BRIGHT_DOWN);
  }

  else if (brilloObjetivo == 1) {

    Serial.println("    Restaurando brillo MEDIO...");

    enviarPinrutar(CMD_BRIGHT_DOWN);
  }

  else {

    Serial.println("    Brillo ya estaba en ALTO.");
  }
}

// =====================================================
// MOSTRAR ESTADO
// =====================================================

void mostrarEstado() {

  Serial.println();
  Serial.println("---------- ESTADO BOMBILLA ----------");

  Serial.print("Encendida: ");

  if (estado.encendida) {
    Serial.println("SI");
  }
  else {
    Serial.println("NO");
  }

  Serial.print("Color: ");

  if (estado.roja) {
    Serial.println("ROJO");
  }
  else {
    Serial.println("BLANCO");
  }

  Serial.print("Brillo: ");

  if (estado.brillo == 0) {
    Serial.println("BAJO");
  }
  else if (estado.brillo == 1) {
    Serial.println("MEDIO");
  }
  else {
    Serial.println("ALTO");
  }

  Serial.println("-------------------------------------");
}

// =====================================================
// DIAGNOSTICO RFID
// =====================================================

void diagnosticoRFID() {

  Serial.println();
  Serial.println("========================================");
  Serial.println("          DIAGNOSTICO RFID");
  Serial.println("========================================");

  Serial.print("SS (SDA): D");
  Serial.println(RFID_SS_PIN);

  Serial.print("RST: D");
  Serial.println(RFID_RST_PIN);

  Serial.println("SPI:");
  Serial.println("  MOSI -> D51");
  Serial.println("  MISO -> D50");
  Serial.println("  SCK  -> D52");
  Serial.println("  SS   -> D53");

  Serial.println();
  Serial.println("Comprobando comunicacion con RC522...");

  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.print("VersionReg = 0x");

  if (version < 0x10) {
    Serial.print("0");
  }

  Serial.println(version, HEX);

  if (version == 0x00 || version == 0xFF) {

    Serial.println();
    Serial.println("!!! AVISO: EL RC522 NO RESPONDE CORRECTAMENTE !!!");
    Serial.println("Revisa alimentacion, SPI, SS y RST.");
  }
  else {

    Serial.println();
    Serial.println("RC522 responde correctamente.");
  }

  Serial.println("========================================");
}

// =====================================================
// CONFIGURACION / CABECERAS DE LAS TARJETAS
// =====================================================

void diagnosticoTarjetas() {

  Serial.println();
  Serial.println("========================================");
  Serial.println("       CONFIGURACION DE TARJETAS");
  Serial.println("========================================");

  Serial.println();
  Serial.println("TARJETA PRINCIPAL");
  Serial.println("  UID: E9 BC E5 6E");
  Serial.println("  Funcion: ON / OFF");

  Serial.println();
  Serial.println("LLAVERO");
  Serial.println("  UID: DB 5E 9F 1B");
  Serial.println("  Funcion: ROJO / BLANCO");

  Serial.println();
  Serial.println("TARJETA BRILLO");
  Serial.println("  UID: 94 97 1E D2");
  Serial.println("  Funcion: BAJO / MEDIO / ALTO");

  Serial.println();
  Serial.println("========================================");
}

// =====================================================
// SINCRONIZAR BOMBILLA AL ARRANCAR
// =====================================================

void sincronizarBombilla() {

  Serial.println();
  Serial.println("========================================");
  Serial.println("       SINCRONIZANDO BOMBILLA");
  Serial.println("========================================");

  // ---------------------------------------------------
  // 1. APAGAR
  // ---------------------------------------------------

  Serial.println();
  Serial.println("1. APAGANDO...");

  enviarPinrutar(CMD_OFF);

  delay(300);

  // ---------------------------------------------------
  // 2. ESTABLECER COLOR ROJO
  // ---------------------------------------------------

  Serial.println("2. ESTABLECIENDO COLOR ROJO...");

  enviarPinrutar(CMD_RED);

  delay(300);

  // ---------------------------------------------------
  // 3. ESTABLECER BRILLO BAJO
  // ---------------------------------------------------

  Serial.println("3. ESTABLECIENDO BRILLO BAJO...");

  // Desde ALTO hasta BAJO necesitamos
  // dos pulsaciones de bajar brillo.

  enviarPinrutar(CMD_BRIGHT_DOWN);

  delay(200);

  enviarPinrutar(CMD_BRIGHT_DOWN);

  delay(300);

  // ---------------------------------------------------
  // ESTADO CONOCIDO
  // ---------------------------------------------------

  estado.encendida = false;
  estado.roja = true;
  estado.brillo = 0;

  Serial.println();
  Serial.println("Sincronizacion completada.");

  mostrarEstado();
}

// =====================================================
// ESPERAR A QUE RETIREN EL RFID
// =====================================================

void esperarRetiradaRFID() {

  Serial.println("    Retira el RFID...");

  delay(300);

  while (rfid.PICC_IsNewCardPresent()) {
    delay(100);
  }

  Serial.println("    RFID retirado.");
  Serial.println();
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("========================================");
  Serial.println("      CONTROL RFID + IR PINRUTAR");
  Serial.println("========================================");

  // ---------------------------------------------------
  // SPI
  // ---------------------------------------------------

  Serial.println();
  Serial.println("Inicializando sistema...");

  Serial.println();
  Serial.println("Inicializando SPI...");

  SPI.begin();

  Serial.println("SPI iniciado.");

  // ---------------------------------------------------
  // RFID
  // ---------------------------------------------------

  Serial.println();
  Serial.println("Inicializando RFID...");

  rfid.PCD_Init();

  delay(100);

  Serial.println("RFID inicializado.");

  diagnosticoRFID();

  diagnosticoTarjetas();

  // ---------------------------------------------------
  // IR
  // ---------------------------------------------------

  Serial.println();
  Serial.println("Inicializando IR...");

  Serial.print("Pin de transmision IR: D");
  Serial.println(IR_SEND_PIN);

  IrSender.begin(IR_SEND_PIN);

  Serial.println("IR listo.");

  // ---------------------------------------------------
  // SINCRONIZAR BOMBILLA
  // ---------------------------------------------------

  sincronizarBombilla();

  // ---------------------------------------------------
  // SISTEMA LISTO
  // ---------------------------------------------------

  Serial.println();
  Serial.println("========================================");
  Serial.println("             SISTEMA LISTO");
  Serial.println("========================================");

  Serial.println();
  Serial.println("Controles:");

  Serial.println("Tarjeta principal  -> ON / OFF");
  Serial.println("Llavero            -> ROJO / BLANCO");
  Serial.println("Tarjeta brillo     -> BAJO / MEDIO / ALTO");

  Serial.println();

  mostrarEstado();

  Serial.println();
  Serial.println("Esperando RFID...");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ---------------------------------------------------
  // NO HAY TARJETA
  // ---------------------------------------------------

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // ---------------------------------------------------
  // NO SE PUDO LEER
  // ---------------------------------------------------

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // ===================================================
  // RFID DETECTADO
  // ===================================================

  Serial.println();
  Serial.println("========================================");
  Serial.println("          RFID DETECTADO");
  Serial.println("========================================");

  Serial.print("UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();
  Serial.println();

  // ===================================================
  // TARJETA PRINCIPAL → ON / OFF
  // ===================================================

  if (compararUID(rfid.uid.uidByte, UID_TARJETA)) {

    Serial.println("RFID IDENTIFICADO: TARJETA PRINCIPAL");

    if (estado.encendida) {

      Serial.println("Accion: APAGAR");

      enviarPinrutar(CMD_OFF);

      estado.encendida = false;
    }
    else {

      Serial.println("Accion: ENCENDER");

      enviarPinrutar(CMD_ON);

      estado.encendida = true;
    }

    mostrarEstado();

    esperarRetiradaRFID();
  }

  // ===================================================
  // LLAVERO → ROJO / BLANCO
  // ===================================================

  else if (compararUID(rfid.uid.uidByte, UID_LLV)) {

    Serial.println("RFID IDENTIFICADO: LLAVERO");

    // Guardamos el brillo ANTES de cambiar de color.
    // El cambio de color lleva físicamente la bombilla
    // a brillo ALTO.

    byte brilloAnterior = estado.brillo;

    if (estado.roja) {

      Serial.println("Accion: CAMBIAR A BLANCO");

      enviarPinrutar(CMD_WHITE);

      delay(300);

      // Restaurar brillo anterior
      restaurarBrillo(brilloAnterior);

      estado.roja = false;
    }
    else {

      Serial.println("Accion: CAMBIAR A ROJO");

      enviarPinrutar(CMD_RED);

      delay(300);

      // Restaurar brillo anterior
      restaurarBrillo(brilloAnterior);

      estado.roja = true;
    }

    // El brillo no cambia.
    estado.brillo = brilloAnterior;

    mostrarEstado();

    esperarRetiradaRFID();
  }

  // ===================================================
  // TARJETA BRILLO → BAJO / MEDIO / ALTO
  // ===================================================

  else if (compararUID(rfid.uid.uidByte, UID_BRILLO)) {

    Serial.println("RFID IDENTIFICADO: TARJETA BRILLO");

    // -------------------------------------------------
    // BAJO → MEDIO
    // -------------------------------------------------

    if (estado.brillo == 0) {

      Serial.println("Accion: BRILLO MEDIO");

      enviarPinrutar(CMD_BRIGHT_UP);

      estado.brillo = 1;
    }

    // -------------------------------------------------
    // MEDIO → ALTO
    // -------------------------------------------------

    else if (estado.brillo == 1) {

      Serial.println("Accion: BRILLO ALTO");

      enviarPinrutar(CMD_BRIGHT_UP);

      estado.brillo = 2;
    }

    // -------------------------------------------------
    // ALTO → BAJO
    // -------------------------------------------------

    else {

      Serial.println("Accion: BRILLO BAJO");

      // Desde ALTO hasta BAJO hacen falta
      // dos pulsaciones de bajar brillo.

      enviarPinrutar(CMD_BRIGHT_DOWN);

      delay(200);

      enviarPinrutar(CMD_BRIGHT_DOWN);

      estado.brillo = 0;
    }

    mostrarEstado();

    esperarRetiradaRFID();
  }

  // ===================================================
  // RFID DESCONOCIDO
  // ===================================================

  else {

    Serial.println("RFID NO RECONOCIDO.");
    Serial.println("No se realiza ninguna accion.");

    esperarRetiradaRFID();
  }

  // ===================================================
  // FINALIZAR LECTURA RFID
  // ===================================================

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}