// XIAO ESP32-S3 unified firmware for Edge Impulse Data Forwarder
// IMU: MPU-9250/6500 on I2C (ACCEL + GYRO ONLY, NO MAG)
// Flex: 2 sensors (voltage divider) on A8, A3
// EMG: on A10 with envelope smoothing
//
// Axis order for Edge Impulse:
// ax, ay, az, gx, gy, gz, flex1, flex2, emg1

#include <Wire.h>

// ---------- IMU (MPU-9250/6500) I2C CONFIG ----------
#define MPU9250_ADDRESS 0x68
#define WHO_AM_I        0x75
#define PWR_MGMT_1      0x6B
#define INT_PIN_CFG     0x37
#define ACCEL_XOUT_H    0x3B
#define GYRO_XOUT_H     0x43
#define TEMP_OUT_H      0x41

// I2C pins (your wiring: IMU SDA -> A4, SCL -> A5)
#define I2C_SDA_PIN A4
#define I2C_SCL_PIN A5

// ---------- FLEX + EMG ANALOG PINS ----------
const int FLEX1_PIN = A8;   // flex sensor 1
const int FLEX2_PIN = A3;   // flex sensor 2
const int EMG_PIN   = A10;  // EMG analog input

// Flex "on/off" parameters
const int FLEX_ON_THRESHOLD = 3500;  // "on" when raw >= 3500
const int FLEX_ON_VAL       = 4095;  // value to output when "on"

// ---------- SAMPLING CONFIG ----------
const int SAMPLE_RATE_HZ             = 100;                      // 100 Hz
const unsigned long SAMPLE_PERIOD_MS = 1000 / SAMPLE_RATE_HZ;   // 10 ms

unsigned long last_sample_ms = 0;

// ---------- EMG ENVELOPE STATE ----------
const int   ENVELOPE_WINDOW   = 32;      // ~320 ms at 100 Hz
const float BASELINE_ALPHA    = 0.001f;  // slow baseline tracking

int   envBuffer[ENVELOPE_WINDOW];
long  envSum       = 0;
int   envIndex     = 0;
float baseline     = 0.0f;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // ADC resolution: 0..4095 for ~0..3.3 V
    analogReadResolution(12);
    analogSetPinAttenuation(EMG_PIN, ADC_11db);

    pinMode(FLEX1_PIN, INPUT);
    pinMode(FLEX2_PIN, INPUT);
    pinMode(EMG_PIN,  INPUT);

    // Init EMG envelope buffer
    for (int i = 0; i < ENVELOPE_WINDOW; i++) {
        envBuffer[i] = 0;
    }

    // I2C setup on your pins
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    pinMode(I2C_SCL_PIN, INPUT_PULLUP);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);   // SDA, SCL
    Wire.setClock(400000);                  // 400 kHz

    delay(100);

    // ---- WHO_AM_I: just to confirm MPU is alive ----
    Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(WHO_AM_I);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU9250_ADDRESS, 1);
    if (Wire.available()) {
        uint8_t whoami = Wire.read();
        (void)whoami;
    }

    // ---- Wake up MPU (clear sleep bit) ----
    Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(PWR_MGMT_1);
    Wire.write(0x00);     // Clear sleep
    Wire.endTransmission(true);
    delay(100);

    last_sample_ms = millis();
}

void loop() {
    unsigned long now = millis();
    if (now - last_sample_ms < SAMPLE_PERIOD_MS) {
        return; // not time yet
    }
    last_sample_ms += SAMPLE_PERIOD_MS;

    // ---------- 1) Read IMU (accel, gyro, temp) ----------
    int16_t accelX_raw, accelY_raw, accelZ_raw;
    int16_t gyroX_raw,  gyroY_raw,  gyroZ_raw;
    int16_t temp_raw;

    // Accelerometer (6 bytes from ACCEL_XOUT_H)
    Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU9250_ADDRESS, 6);
    accelX_raw = (Wire.read() << 8) | Wire.read();
    accelY_raw = (Wire.read() << 8) | Wire.read();
    accelZ_raw = (Wire.read() << 8) | Wire.read();

    // Temperature (2 bytes) – unused but read to keep sequence clean
    Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(TEMP_OUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU9250_ADDRESS, 2);
    temp_raw = (Wire.read() << 8) | Wire.read();
    (void)temp_raw;

    // Gyroscope (6 bytes from GYRO_XOUT_H)
    Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU9250_ADDRESS, 6);
    gyroX_raw = (Wire.read() << 8) | Wire.read();
    gyroY_raw = (Wire.read() << 8) | Wire.read();
    gyroZ_raw = (Wire.read() << 8) | Wire.read();

    // Convert accel to g, then to m/s^2 (±2g, 16384 LSB/g)
    float ax = (accelX_raw / 16384.0f) * 9.80665f;
    float ay = (accelY_raw / 16384.0f) * 9.80665f;
    float az = (accelZ_raw / 16384.0f) * 9.80665f;

    // Convert gyro to deg/s (±250 dps, 131 LSB/(deg/s))
    float gx = gyroX_raw / 131.0f;
    float gy = gyroY_raw / 131.0f;
    float gz = gyroZ_raw / 131.0f;

    // ---------- 2) Read + threshold flex ----------
    int flex1_raw = analogRead(FLEX1_PIN);  // 0..4095
    int flex2_raw = analogRead(FLEX2_PIN);  // 0..4095

    int flex1 = (flex1_raw >= FLEX_ON_THRESHOLD) ? FLEX_ON_VAL : 0;
    int flex2 = (flex2_raw >= FLEX_ON_THRESHOLD) ? FLEX_ON_VAL : 0;

    // ---------- 3) Read + process EMG (envelope) ----------
    int raw = analogRead(EMG_PIN);      // 0..4095

    // Baseline removal (slow high-pass)
    baseline += BASELINE_ALPHA * (raw - baseline);
    float centered = raw - baseline;

    // Rectify
    int rectified = abs((int)centered);

    // Moving-average envelope
    envSum -= envBuffer[envIndex];
    envSum += rectified;
    envBuffer[envIndex] = rectified;
    envIndex = (envIndex + 1) % ENVELOPE_WINDOW;

    int envelope = (envSum / ENVELOPE_WINDOW) * 2;   // *2 for more range

    if (envelope < 0)     envelope = 0;
    if (envelope > 4095)  envelope = 4095;

    int emg1 = envelope;

    // ---------- 4) Print one CSV line ----------
    // Order: ax, ay, az, gx, gy, gz, flex1, flex2, emg1

    Serial.print(ax, 5);   Serial.print(",");
    Serial.print(ay, 5);   Serial.print(",");
    Serial.print(az, 5);   Serial.print(",");
    Serial.print(gx, 5);   Serial.print(",");
    Serial.print(gy, 5);   Serial.print(",");
    Serial.print(gz, 5);   Serial.print(",");
    Serial.print(flex1);   Serial.print(",");
    Serial.print(flex2);   Serial.print(",");
    Serial.println(emg1);
}
