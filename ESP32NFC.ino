// Biblioteca -----------------------------------------------------------------------------------------
#include <Arduino.h>
#include <PN532.h> // Responsável pelo funcionamento do Modulo NFC
#include <PN532_I2C.h>
#include "NetClient.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <NfcAdapter.h>
#include <pins_arduino.h>

// Declaração de Objetos ------------------------------------------------------------------------------
PN532_I2C pn532i2c(Wire);
PN532 nfcLowLevel(pn532i2c); // Declara objeto utilizando o objeto para comunicação
NfcAdapter nfc = NfcAdapter(pn532i2c);
#define PRX RX
#define PTX TX

// Variáveis Globais ----------------------------------------------------------------------------------

uint8_t SELECT_APDU[] = {
  0x00, /* CLA */
  0xA4, /* INS */
  0x04, /* P1  */
  0x00, /* P2  */
  0x07, /* Length of AID  */
  0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /* AID  */
  0x00  /* Le  */
};

static const String API_URL = "https://u0uz5ogpyf.execute-api.us-east-1.amazonaws.com/dev";

const uint8_t LED_SUCCESS = D5, LED_FAIL = D6, LED_LOADING = D7, LED_POWER = D8;

static bool loading = true;

int validateKey(String keyValue)
{
  String uri = API_URL + "/v1/keys/validate/" + keyValue;
  Serial.println("Request URI: ");
  Serial.println(uri);
  return NetClient::get(uri);
}

void validationCycle(String value) {

  int result = validateKey(value);
  if (result < 400)
  {
    Serial.println("Success validating key!");
    successLED();
  }
  else if (result == 404)
  {
    Serial.println("Failed to validate key");
    failureLED();
  }
  else
  {
    Serial.println("Error communicating with API.");
    Serial.println("Status code:");
    Serial.println(result);
    confusionLED();
  }
}

void startNFCLowLevelCommunication()
{

  nfcLowLevel.begin();

  uint32_t versionData = nfcLowLevel.getFirmwareVersion();

  if (!versionData)
  {
    confusionLED();
    while (1);
  }

  Serial.print("Found chip PN5"); Serial.println((versionData>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versionData>>16) & 0xFF, DEC); Serial.print('.'); Serial.println((versionData>>8) & 0xFF, DEC);

  // setup RFID tag reading
  nfcLowLevel.SAMConfig();

  nfc.begin();
  digitalWrite(LED_POWER, HIGH);
}

void ESPAndroidCommunicationLoop()
{
  Serial.println("Listening for tags...");
  if (nfcLowLevel.inListPassiveTarget()) {
    Serial.println("Found a tag nearby...");
    loadingLED();
    uint8_t response[255];
    uint8_t responseLength = sizeof(response);

    bool success = nfcLowLevel.inDataExchange(SELECT_APDU, sizeof(SELECT_APDU), response, &responseLength);
    loadingLED();
    if (success) {
      Serial.println("Communication started");
      char message[responseLength];
      nfcLowLevel.PrintHexChar(response, responseLength);
      uint32_t index;
      for (index = 0; index < responseLength; index++) {  
          loadingLED(); 
          message[index] = (char)response[index];
      }
      message[responseLength] = '\0';
      Serial.println(message);
      validationCycle(message);
    } else {
      Serial.println("Couldn't communicate with tag");
    }
  }
  
}

void setupConnection()
{
  NetClient::connect(&loadingLED);
  String jsonStart = "{\"content\": \"ESP conectado em ";
  String jsonEnding = "\",\"embeds\": null,\"attachments\": []}";
  String jsonData = jsonStart + "http://" + WiFi.localIP().toString() + "/" + " na rede " + String(NetClient::ssid) + jsonEnding;
  Serial.println(jsonData);

  pinMode(LED_SUCCESS, OUTPUT);
  pinMode(LED_FAIL, OUTPUT);

  int res = NetClient::post(
      "https://discord.com/api/webhooks/1098328141405110433/4iTX3nteVMre2KqymitRrOlopguGd67M011W2dnG_ve6E3W8PV87O-DoAk8X1k3fbn3q",
      jsonData);
  Serial.println("Webhook status code:");
  Serial.println(res);
}


void turnOffLEDS()
{
  Serial.println("Turning off LEDs");
  digitalWrite(LED_SUCCESS, LOW);
  digitalWrite(LED_FAIL, LOW);
  digitalWrite(LED_LOADING, LOW);
  // digitalWrite(POWER_LED, LOW);
}

void successLED()
{
  turnOffLEDS();
  digitalWrite(LED_SUCCESS, HIGH);
  delay(5000);
  digitalWrite(LED_SUCCESS, LOW);
}

void failureLED()
{
  turnOffLEDS();
  digitalWrite(LED_FAIL, HIGH);
  delay(2000);
  digitalWrite(LED_FAIL, LOW);
}

void confusionLED()
{
  int tries = 0;
  turnOffLEDS();
  for (tries; tries <= 3; tries++)
  {
    digitalWrite(LED_FAIL, LOW);
    digitalWrite(LED_SUCCESS, HIGH);
    delay(500);
    digitalWrite(LED_SUCCESS, LOW);
    digitalWrite(LED_FAIL, HIGH);
    delay(250);
    digitalWrite(LED_FAIL, LOW);
    digitalWrite(LED_SUCCESS, HIGH);
    delay(250);
    digitalWrite(LED_FAIL, HIGH);
    digitalWrite(LED_SUCCESS, LOW);
  }
  digitalWrite(LED_FAIL, LOW);
}

void loadingLED()
{
  digitalWrite(LED_LOADING, HIGH);
  delay(50);
  digitalWrite(LED_LOADING, LOW);
  delay(50);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_SUCCESS, OUTPUT);
  pinMode(LED_FAIL, OUTPUT);
  pinMode(LED_POWER, OUTPUT);
  pinMode(LED_LOADING, OUTPUT);
  // digitalWrite(LED_POWER, HIGH);
  setupConnection();
  startNFCLowLevelCommunication();
}


void loop()
{
  ESPAndroidCommunicationLoop();
    // digitalWrite(LED_SUCCESS, HIGH);
    // digitalWrite(LED_LOADING, HIGH);
    // digitalWrite(LED_FAIL, HIGH);
  // loadingLED();
  // delay(1000);

    // digitalWrite(LED_SUCCESS, LOW);
    // digitalWrite(LED_LOADING, LOW);
    // digitalWrite(LED_FAIL, LOW);
  // successLED();
  // delay(1000);
  // confusionLED();
  // delay(1000);
  delay(50);
}