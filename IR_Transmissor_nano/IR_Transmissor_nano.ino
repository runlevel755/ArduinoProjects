#include <SPI.h>
#include <MFRC522.h>
#include <IRremote.hpp>

// ============================================================
// PINES - ARDUINO NANO
// ============================================================

#define RFID_SS_PIN   10
#define RFID_RST_PIN   9
#define IR_SEND_PIN    3

// RC522:
// SDA/SS -> D10
// RST    -> D9
// MOSI   -> D11
// MISO   -> D12
// SCK    -> D13
// IRQ    -> sin conectar
// VCC    -> 3.3V
// GND    -> GND
//
// IR LED:
// D3 -> 220 ohm -> ánodo LED IR
// cátodo LED IR -> GND


// ============================================================
// RFID
// ============================================================

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);


// ============================================================
// TARJETAS RFID
// ============================================================

const byte UID_TARJETA[] = {
  0xE9, 0xBC, 0xE5, 0x6E
};

const byte UID_LLV[] = {
  0xDB, 0x5E, 0x9F, 0x1B
};

const byte UID_BRILLO[] = {
  0x94, 0x97, 0x1E, 0xD2
};


// ============================================================
// PROTOCOLO IR PINRUTAR
// ============================================================

#define PINRUTAR_ADDRESS 0xB708

#define CMD_ON           0x00
#define CMD_OFF          0x02
#define CMD_RED          0x10
#define CMD_WHITE        0x08
#define CMD_BRIGHT_DOWN  0x0D
#define CMD_BRIGHT_UP    0x0E


// ============================================================
// ESTADO DE LA BOMBILLA
// ============================================================
//
// brillo:
// 0 = BAJO
// 1 = MEDIO
// 2 = ALTO
//

struct EstadoBombilla {
  bool encendida;
  bool roja;
  byte brillo;
};

EstadoBombilla estado = {false, true, 0};


// ============================================================
// COMPARAR UID
// ============================================================

bool compararUID(const byte *uid1, const byte *uid2) {

  for (byte i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }

  return true;
}


// ============================================================
// ENVIAR COMANDO IR
// ============================================================

void enviarPinrutar(byte comando) {

  IrSender.sendNEC(PINRUTAR_ADDRESS, comando, 0);

  delay(200);
}


// ============================================================
// RESTAURAR BRILLO
// ============================================================
//
// Al cambiar entre ROJO y BLANCO, la bombilla pasa
// físicamente a brillo ALTO.
//
// Por eso debemos devolverla al brillo que tenía.
//

void restaurarBrillo(byte brilloObjetivo) {

  if (brilloObjetivo == 0) {

    // ALTO -> BAJO
    enviarPinrutar(CMD_BRIGHT_DOWN);
    enviarPinrutar(CMD_BRIGHT_DOWN);

  } else if (brilloObjetivo == 1) {

    // ALTO -> MEDIO
    enviarPinrutar(CMD_BRIGHT_DOWN);

  } else {

    // Ya está en ALTO.
  }
}


// ============================================================
// MOSTRAR ESTADO
// ============================================================

void mostrarEstado() {

  Serial.println(F("--------------------------------"));

  Serial.print(F("Bombilla: "));
  if (estado.encendida) {
    Serial.println(F("ON"));
  } else {
    Serial.println(F("OFF"));
  }

  Serial.print(F("Color: "));
  if (estado.roja) {
    Serial.println(F("ROJO"));
  } else {
    Serial.println(F("BLANCO"));
  }

  Serial.print(F("Brillo: "));

  if (estado.brillo == 0) {
    Serial.println(F("BAJO"));
  } else if (estado.brillo == 1) {
    Serial.println(F("MEDIO"));
  } else {
    Serial.println(F("ALTO"));
  }

  Serial.println(F("--------------------------------"));
}


// ============================================================
// DIAGNOSTICO RFID
// ============================================================

void diagnosticoRFID() {

  Serial.println();
  Serial.println(F("========== DIAGNOSTICO RFID =========="));

  Serial.println(F("RC522:"));
  Serial.println(F("  SS   -> D10"));
  Serial.println(F("  RST  -> D9"));
  Serial.println(F("  MOSI -> D11"));
  Serial.println(F("  MISO -> D12"));
  Serial.println(F("  SCK  -> D13"));

  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.print(F("VersionReg: 0x"));

  if (version < 0x10) {
    Serial.print('0');
  }

  Serial.println(version, HEX);

  if (version == 0x88) {
    Serial.println(F("RC522 detectado correctamente."));
  } else {
    Serial.println(F("AVISO: VersionReg inesperado."));
  }

  Serial.println(F("======================================="));
}


// ============================================================
// DIAGNOSTICO TARJETAS
// ============================================================

void diagnosticoTarjetas() {

  Serial.println();
  Serial.println(F("========== TARJETAS CONFIGURADAS =========="));

  Serial.println(F("TARJETA PRINCIPAL"));
  Serial.println(F("  UID: E9 BC E5 6E"));
  Serial.println(F("  Funcion: ON / OFF"));
  Serial.println();

  Serial.println(F("LLAVERO"));
  Serial.println(F("  UID: DB 5E 9F 1B"));
  Serial.println(F("  Funcion: ROJO / BLANCO"));
  Serial.println();

  Serial.println(F("TARJETA BRILLO"));
  Serial.println(F("  UID: 94 97 1E D2"));
  Serial.println(F("  Funcion: BAJO / MEDIO / ALTO"));

  Serial.println(F("============================================"));
}


// ============================================================
// SINCRONIZAR BOMBILLA AL ARRANQUE
// ============================================================
//
// Estado inicial deseado:
//
// OFF
// ROJO
// BRILLO BAJO
//
// Secuencia:
// 1. OFF
// 2. ROJO
// 3. BAJAR BRILLO dos veces
//

void sincronizarBombilla() {

  Serial.println();
  Serial.println(F("========== SINCRONIZACION =========="));

  Serial.println(F("-> OFF"));
  enviarPinrutar(CMD_OFF);

  Serial.println(F("-> ROJO"));
  enviarPinrutar(CMD_RED);

  Serial.println(F("-> BRILLO BAJO"));
  enviarPinrutar(CMD_BRIGHT_DOWN);
  enviarPinrutar(CMD_BRIGHT_DOWN);

  estado.encendida = false;
  estado.roja = true;
  estado.brillo = 0;

  Serial.println(F("Bombilla sincronizada."));
  mostrarEstado();

  Serial.println(F("===================================="));
}


// ============================================================
// ESPERAR A RETIRAR RFID
// ============================================================
//
// Evita que una tarjeta mantenida sobre el lector
// ejecute la accion varias veces.
//

void esperarRetiradaRFID() {

  delay(300);

  while (rfid.PICC_IsNewCardPresent() ||
         rfid.PICC_ReadCardSerial()) {

    delay(50);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  Serial.println(F("Tarjeta retirada."));
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F(" IR TRANSMISOR RFID NANO V2.2"));
  Serial.println(F("========================================"));

  // ----------------------------------------------------------
  // SPI
  // ----------------------------------------------------------

  SPI.begin();

  // ----------------------------------------------------------
  // RFID
  // ----------------------------------------------------------

  rfid.PCD_Init();

  delay(100);

  diagnosticoRFID();
  diagnosticoTarjetas();

  // ----------------------------------------------------------
  // IR
  // ----------------------------------------------------------

  IrSender.begin(IR_SEND_PIN);

  Serial.println();
  Serial.println(F("IR configurado en D3."));

  // ----------------------------------------------------------
  // SINCRONIZACION
  // ----------------------------------------------------------

  sincronizarBombilla();

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F(" SISTEMA LISTO"));
  Serial.println(F("========================================"));

  mostrarEstado();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // ¿Hay una tarjeta nueva?
  // ----------------------------------------------------------

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }


  // ----------------------------------------------------------
  // MOSTRAR UID DETECTADO
  // ----------------------------------------------------------

  Serial.println();
  Serial.println(F("========== RFID DETECTADO =========="));

  Serial.print(F("UID: "));

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print('0');
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(' ');
    }
  }

  Serial.println();


  // ==========================================================
  // TARJETA PRINCIPAL
  // ==========================================================

  if (rfid.uid.size == 4 &&
      compararUID(rfid.uid.uidByte, UID_TARJETA)) {

    Serial.println(F("TARJETA PRINCIPAL"));

    if (estado.encendida) {

      Serial.println(F("Accion: APAGAR"));

      enviarPinrutar(CMD_OFF);

      estado.encendida = false;

    } else {

      Serial.println(F("Accion: ENCENDER"));

      enviarPinrutar(CMD_ON);

      estado.encendida = true;
    }

    mostrarEstado();

    esperarRetiradaRFID();

    return;
  }


  // ==========================================================
  // LLAVERO
  // ==========================================================

  if (rfid.uid.size == 4 &&
      compararUID(rfid.uid.uidByte, UID_LLV)) {

    Serial.println(F("LLAVERO"));

    byte brilloAnterior = estado.brillo;

    if (estado.roja) {

      Serial.println(F("Accion: BLANCO"));

      enviarPinrutar(CMD_WHITE);

      estado.roja = false;

    } else {

      Serial.println(F("Accion: ROJO"));

      enviarPinrutar(CMD_RED);

      estado.roja = true;
    }

    // El cambio de color pone físicamente la bombilla
    // en brillo ALTO, así que restauramos el anterior.
    restaurarBrillo(brilloAnterior);

    estado.brillo = brilloAnterior;

    mostrarEstado();

    esperarRetiradaRFID();

    return;
  }


  // ==========================================================
  // TARJETA BRILLO
  // ==========================================================

  if (rfid.uid.size == 4 &&
      compararUID(rfid.uid.uidByte, UID_BRILLO)) {

    Serial.println(F("TARJETA BRILLO"));

    if (estado.brillo == 0) {

      // BAJO -> MEDIO

      Serial.println(F("Accion: BRILLO MEDIO"));

      enviarPinrutar(CMD_BRIGHT_UP);

      estado.brillo = 1;

    } else if (estado.brillo == 1) {

      // MEDIO -> ALTO

      Serial.println(F("Accion: BRILLO ALTO"));

      enviarPinrutar(CMD_BRIGHT_UP);

      estado.brillo = 2;

    } else {

      // ALTO -> BAJO

      Serial.println(F("Accion: BRILLO BAJO"));

      enviarPinrutar(CMD_BRIGHT_DOWN);
      enviarPinrutar(CMD_BRIGHT_DOWN);

      estado.brillo = 0;
    }

    mostrarEstado();

    esperarRetiradaRFID();

    return;
  }


  // ==========================================================
  // TARJETA DESCONOCIDA
  // ==========================================================

  Serial.println(F("TARJETA NO CONFIGURADA"));

  Serial.println(F("No se realiza ninguna accion."));

  Serial.println(F("===================================="));

  esperarRetiradaRFID();
}