#include <Arduino.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define OLED_I2C_ADDR 0x3C /* OLED module I2C address */

#define PIN_NRF24L01_CE 7
#define PIN_NRF24L01_CSN 8
#define PIPE_NUMBER 0
#define NRF24L01_COMM_CHANNEL 124

//Set up OLED display
SSD1306AsciiWire oled_display;

//create an RF24 object
RF24 radio(PIN_NRF24L01_CE, PIN_NRF24L01_CSN);

int pin_Status_Led = LED_BUILTIN;

//address through which two modules communicate.
uint8_t pipe_address[6] = "N0001";
unsigned long t_last_packet = 0;
unsigned long t_receive_time = 0;
unsigned long last_received_counter = 0;
unsigned long current_received_counter = 0;
const char empty_line[] = "            ";

void Gpio_Init(void);
void Ssd1306_Oled_Init(void);
void Nrf24l01_Receiver_Init(void);
void Nrf24l01_Cont_Receive(void);

void setup() {
  Serial.begin(9600);
  Gpio_Init();
  Ssd1306_Oled_Init();
  Nrf24l01_Receiver_Init();
}

void loop() {
  Nrf24l01_Cont_Receive();
  
}

void Gpio_Init(void) {
  //pinMode(pin_Manual_Switch, INPUT_PULLUP); //INPUT => reverse logic!
  //pinMode(pin_Reed_Switch, INPUT_PULLUP); //INPUT => reverse logic!
  pinMode(PIN_NRF24L01_CE, OUTPUT);
  pinMode(PIN_NRF24L01_CSN, OUTPUT);
  pinMode(pin_Status_Led, OUTPUT);
  digitalWrite(pin_Status_Led, LOW);
}

void Ssd1306_Oled_Init(void) {
  Wire.begin();
  oled_display.begin(&Adafruit128x32, OLED_I2C_ADDR);
  oled_display.clear();
  oled_display.setFont(fixed_bold10x15);
  oled_display.setRow(0);
  oled_display.println(F("nRF24L01"));
  oled_display.println(F("Receiver"));
  delay(1000);
  
}

void Nrf24l01_Receiver_Init(void) {
  char *tempPointer;
  //set the address
  digitalWrite(pin_Status_Led, HIGH);
  oled_display.clear();
  oled_display.print(F("Pipe: "));
  tempPointer=(char *)&pipe_address[0];
  oled_display.println(tempPointer);
  radio.begin();
  //radio.setChannel(NRF24L01_COMM_CHANNEL);
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.disableDynamicPayloads();
  radio.setRetries(10,15);
  radio.openReadingPipe(PIPE_NUMBER, pipe_address);
  oled_display.print(F("Open "));
  //Set module as receiver
  radio.startListening();
  digitalWrite(pin_Status_Led, LOW);
  oled_display.print(F("Start"));
  delay(1000);
  oled_display.clear();
}

void Nrf24l01_Cont_Receive(void) {
  oled_display.setCol(0);
  oled_display.setRow(0);
  if (radio.available())
  {
    //oled_display.print(F("Receiving  "));
    char received_text[12] = {0};
    radio.read(&received_text, sizeof(received_text));
    t_last_packet=millis();
    current_received_counter=atol(received_text);
    oled_display.print(F("T:"));
    oled_display.print(empty_line);
    oled_display.setCol(20);
    oled_display.println((millis()-t_last_packet)/100);
    oled_display.print(empty_line);
    oled_display.setCol(0);
    oled_display.print(received_text);
    oled_display.print(F(","));
    oled_display.println(current_received_counter-last_received_counter);
    last_received_counter=current_received_counter;
    //oled_display.println(F("Received:  "));
    //oled_display.print(F("           "));
    //oled_display.setCol(0);
    //oled_display.print(received_text);
  }
  else {
    if(millis() % 200 == 0) {
      oled_display.setCol(0);
      oled_display.print(F("T:"));
      oled_display.print(empty_line);
      oled_display.setCol(20);
      oled_display.print((millis()-t_last_packet)/100);
    }    
  }
}