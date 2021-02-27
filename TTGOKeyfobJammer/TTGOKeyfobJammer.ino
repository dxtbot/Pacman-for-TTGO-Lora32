////////////////////////////////////////////////////////////////
// signal packman //                                        ////
// because the jamming signal sounds like pacman eating ,// ////
// build 28-02-2021 Made by DexterBot //                    ////
////////////////////////////////////////////////////////////////
#include "images.h"
#include <SPI.h>
#include <LoRa.h>
#define CONFIG_MOSI 27
#define CONFIG_MISO 19
#define CONFIG_CLK  5
#define CONFIG_NSS  18
#define CONFIG_RST  23
#define CONFIG_DIO0 26
#define LORA_PERIOD 433
#define BAND 433910000
#include <Wire.h>
#include "SSD1306Wire.h"
#define OLED_CLASS_OBJ  SSD1306Wire
#define OLED_ADDRESS    0x3C
#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_RST    -1
#define LED1    25
//array of frequencies valid for the application to change
long int frequencies[3] = {433910000, 315000000, 868000000};
//controls the current frequency index in the array
int indexFreq = 0;
volatile int incomingPacketSize;
int rssi = 0;
OLED_CLASS_OBJ tft(OLED_ADDRESS, OLED_SDA, OLED_SCL);
int cpo = 0;
void setup() {
  pinMode(LED1, OUTPUT);
  // Serial.begin(9600);
  //  while (!Serial);
  LoRa.setGain(20);
  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
  LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
  LoRa.begin(frequencies[indexFreq]);
  LoRa.receive();
  tft.init();
  tft.setTextAlignment(TEXT_ALIGN_CENTER);
  tft.flipScreenVertically();
  tft.clear();
  tft.drawXbm(7, 5, Pacman_width, Pacman_height, Pacman_bits);
  tft.display();
  delay(2000);
  tft.clear();
  tft.drawString(57, 20, "!   DXT PACMAN   !");
  tft.drawString(60, 40, "V1.01");
  tft.display();
  delay(2000);
  digitalWrite(LED1, HIGH);
  tft.clear();
  tft.drawXbm(10, 1, Hacker_width, Hacker_height, Hacker_Logo_bits);
  tft.display();
  delay(2000);
  digitalWrite(LED1, LOW);
}

void loop() {
  cpo = cpo + 1;
  if (cpo == 2500)cpo = 0;
  if (cpo > 1500) {
    tft.setFont(ArialMT_Plain_10);
    tft.clear();
    tft.drawXbm(10, 1, Hacker_width, Hacker_height, Hacker_Logo_bits);
    tft.display();
    rssi = LoRa.rssi();
  }
  if (cpo < 1500) {
    tft.setFont(ArialMT_Plain_10);
    tft.clear();
    digitalWrite(LED1, LOW);
    tft.drawString(57, 3, " !   DXT JAMMER   !");
    tft.drawString(62, 30, "{ # LISTEN ON FREQ # }");
    tft.drawString(60, 51, "433.910 Mhz");
    tft.display();
    rssi = LoRa.rssi();
  }
  if (rssi < -10 && rssi > -80) {
    tft.clear();
    tft.drawString(57, 3, "  ! ACTIVE !");
    tft.setFont(ArialMT_Plain_24);
    tft.drawString(60, 20, "JAMMING");
    tft.display();
    for (int i = 0; i <= 25; i++) {
      int progress = (i * 4) % 100;
      tft.drawProgressBar(0, 53, 120, 10, progress);
      tft.display();
      LoRa.beginPacket();
      digitalWrite(LED1, HIGH);
      LoRa.print("111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
      LoRa.print("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print("ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890");
      LoRa.print("010000100001001001010110011001001000101010101010101010100101010101010010101010100101001010101010");
      LoRa.print(" Pacman wil eat your signal !       Pacman wil eat you !       Pacman wil eat your signal !     ");
      LoRa.endPacket();
    }
    tft.clear();
    tft.drawXbm(7, 5, Pacman_width, Pacman_height, Pacman_bits);
    tft.display();
    delay(200);
    LoRa.receive();
    delay(200);
  }
}
