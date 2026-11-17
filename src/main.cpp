#include "AudioTools.h"      // Version-independent I2S abstraction
#include "BluetoothA2DPSink.h"  // Core A2DP receiver functionality
#include "AudioTools/AudioLibs/I2SCodecStream.h"
#include "AudioBoard.h"
#include "esp_bt.h"
#include <Arduino.h>


// --- ESP32 Catphones PCB ---
#define PIN_I2C_SCL  32
#define PIN_I2C_SDA  33
#define PIN_MCLK      0
#define PIN_BCLK     27
#define PIN_WS       25
#define PIN_DOUT     26
#define PIN_DIN      35
#define PIN_PA_EN    21

//#define BATTERY_ADC_PIN 34
//#define TEMT6000_ADC_PIN 39
#define LED_PIN 15
//#define KEEP_ALIVE_PIN 23
//#define AUX_PIN 5 

#define BT_DEVICE_NAME "CatPhones-7.2"

//inputs (battery-Voltage, light sensor, aux cable detection, Bluetooth audio, MIC input)

DriverPins pins;
AudioBoard *board = nullptr;
I2SCodecStream *i2s = nullptr;
BluetoothA2DPSink *a2dp_sink = nullptr;

// Receives metadata: track title, artist, album, duration, etc.
void avrc_metadata_callback(uint8_t attribute_id, const uint8_t *data) {
    // attribute_id identifies which metadata field (ESP_AVRC_MD_ATTR_*)
    // data is null-terminated string with the value
    Serial.printf("AVRC metadata: attr 0x%x = %s\n", attribute_id, data);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("boot");
  //pinMode(KEEP_ALIVE_PIN, OUTPUT);
  //digitalWrite(KEEP_ALIVE_PIN, HIGH);  // keep the ESP32 awake

  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, LOW);
  //analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
  //pinMode(TEMT6000_ADC_PIN, INPUT);
  //analogSetPinAttenuation(TEMT6000_ADC_PIN, ADC_11db);
  //pinMode(AUX_PIN, INPUT_PULLUP);
    
    
  pins.addI2C(PinFunction::CODEC, PIN_I2C_SCL, PIN_I2C_SDA);
  pins.addI2S(PinFunction::CODEC, PIN_MCLK, PIN_BCLK, PIN_WS, PIN_DOUT, PIN_DIN);
  pins.addPin(PinFunction::PA, PIN_PA_EN, PinLogic::Output);

  board = new AudioBoard(AudioDriverES8388, pins);
  i2s = new I2SCodecStream(*board);

  auto cfg = i2s->defaultConfig(TX_MODE);
  cfg.sample_rate = 44100;
  cfg.channels = 2;
  cfg.bits_per_sample = 16;
  Serial.println("starting i2s/codec");
  if (!i2s->begin(cfg)) Serial.println("i2s begin FAILED");
  i2s->setVolume(1);
  
  a2dp_sink = new BluetoothA2DPSink(*i2s);
  a2dp_sink->set_on_volumechange([](int v) { i2s->setVolume(v / 127.0f); });
  Serial.println("starting a2dp");
  a2dp_sink->start(BT_DEVICE_NAME);


  esp_bredr_tx_power_set(ESP_PWR_LVL_N3, ESP_PWR_LVL_P3); // call after a2dp_sink->start()

  Serial.printf("Lower CPU frequency to 80MHz \n");

  // After connection established, reduce CPU frequency
  setCpuFrequencyMhz(80);  // Down from 240MHz
}

void loop() {
   
  delay(1000);

  analogWrite(LED_PIN, 200);

  //uint32_t adcMilliVolts = analogReadMilliVolts(BATTERY_ADC_PIN);
  //float batteryVoltage = (adcMilliVolts * 2.0f) / 1000.0f;
  //uint32_t lightMilliVolts = analogReadMilliVolts(TEMT6000_ADC_PIN);
  //uint16_t lightAdc = analogRead(TEMT6000_ADC_PIN);
  //Serial.printf("Battery: %.2f V | B rightness: %u ADC, %u mV\n", batteryVoltage, lightAdc, lightMilliVolts);

  //if (batteryVoltage < 2.9f) {
    //Serial.println("Battery low, shutting down...");
    //analogWrite(LED_PIN, 0);
    //delay(1000);
    //digitalWrite(KEEP_ALIVE_PIN, LOW);  // allow the ESP32 to sleep
  //} 
  //if (digitalRead(AUX_PIN) == LOW) {
    //Serial.println("AUX cable detected");
  //}

}