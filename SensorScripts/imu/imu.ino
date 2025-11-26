// MPU-9250/6500 Sensor Reader for XIAO ESP32 S3
// Reads data from SDA & syncs w/ SCL and outputs to Serial Monitor and Plotter
// 
// NO EXTERNAL LIBRARIES NEEDED - Uses raw I2C commands
//
// Wiring (with internal pull-ups):
// MPU SDA  -> ESP32 S3 GPIO 1 (A0/D0, top pin left side)
// MPU SCL  -> ESP32 S3 GPIO 2 (A1/D1, 2nd from top left side)
// MPU VCC  -> 3.3V
// MPU GND  -> GND
// MPU AD0  -> GND (sets I2C address to 0x68)

#include <Wire.h>

#define MPU9250_ADDRESS 0x68
#define MAG_ADDRESS     0x0C
#define WHO_AM_I        0x75
#define PWR_MGMT_1      0x6B
#define INT_PIN_CFG     0x37
#define USER_CTRL       0x6A
#define ACCEL_XOUT_H    0x3B
#define GYRO_XOUT_H     0x43
#define TEMP_OUT_H      0x41
#define MAG_XOUT_L      0x03
#define MAG_CNTL1       0x0A
#define MAG_ST2         0x09

bool hasMagnetometer = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("MPU-9250/6500 Sensor Test");
  Serial.println("Initializing sensor...");
  
  // Enable internal pull-up resistors
  pinMode(1, INPUT_PULLUP);  // SDA
  pinMode(2, INPUT_PULLUP);  // SCL
  
  delay(200); // Wait for sensor power-up

  // Initialize I2C
  Wire.begin(1, 2); // SDA = GPIO 1, SCL = GPIO 2
  Wire.setClock(400000); // 400kHz
  
  delay(100);
  
  // Check WHO_AM_I register
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(WHO_AM_I);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDRESS, 1);
  uint8_t whoami = Wire.read();
  
  Serial.print("WHO_AM_I: 0x");
  Serial.println(whoami, HEX);
  
  if (whoami == 0x71 || whoami == 0x73) {
    Serial.println("MPU-9250/9255 Found!");
    hasMagnetometer = true;
  } else if (whoami == 0x70) {
    Serial.println("MPU-6500 Found!");
    hasMagnetometer = false;
  } else {
    Serial.print("Unknown device: 0x");
    Serial.println(whoami, HEX);
  }
  
  // Wake up MPU (clear sleep mode bit)
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(PWR_MGMT_1);
  Wire.write(0x00);
  Wire.endTransmission(true);
  
  delay(100);
  
  // If MPU-9250, enable magnetometer bypass mode
  if (hasMagnetometer) {
    // Enable I2C bypass to access magnetometer
    Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(INT_PIN_CFG);
    Wire.write(0x02); // Enable bypass mode
    Wire.endTransmission(true);
    
    delay(10);
    
    // Check if magnetometer is accessible
    Wire.beginTransmission(MAG_ADDRESS);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.println("Magnetometer found at 0x0C!");
      
      // Set magnetometer to continuous measurement mode (100Hz)
      Wire.beginTransmission(MAG_ADDRESS);
      Wire.write(MAG_CNTL1);
      Wire.write(0x16); // Continuous mode 2 (100Hz), 16-bit output
      Wire.endTransmission(true);
      
      delay(10);
    } else {
      Serial.println("Magnetometer not found!");
      hasMagnetometer = false;
    }
  }
  
  Serial.println("Configuration complete!");
  Serial.println("");
  
  if (hasMagnetometer) {
    Serial.println("AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Temp,MagX,MagY,MagZ");
  } else {
    Serial.println("AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Temp");
  }
}

void loop() {
  int16_t accelX, accelY, accelZ;
  int16_t gyroX, gyroY, gyroZ;
  int16_t temp;
  
  // Read accelerometer data (6 bytes starting at ACCEL_XOUT_H)
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDRESS, 6);
  
  accelX = (Wire.read() << 8) | Wire.read();
  accelY = (Wire.read() << 8) | Wire.read();
  accelZ = (Wire.read() << 8) | Wire.read();
  
  // Read temperature (2 bytes)
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(TEMP_OUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDRESS, 2);
  
  temp = (Wire.read() << 8) | Wire.read();
  
  // Read gyroscope data (6 bytes starting at GYRO_XOUT_H)
  Wire.beginTransmission(MPU9250_ADDRESS);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDRESS, 6);
  
  gyroX = (Wire.read() << 8) | Wire.read();
  gyroY = (Wire.read() << 8) | Wire.read();
  gyroZ = (Wire.read() << 8) | Wire.read();
  
  // Convert to proper units
  // Accel: default range is ±2g, sensitivity = 16384 LSB/g
  float accelX_g = accelX / 16384.0;
  float accelY_g = accelY / 16384.0;
  float accelZ_g = accelZ / 16384.0;
  
  // Convert to m/s^2
  float accelX_mps2 = accelX_g * 9.80665;
  float accelY_mps2 = accelY_g * 9.80665;
  float accelZ_mps2 = accelZ_g * 9.80665;
  
  // Gyro: default range is ±250°/s, sensitivity = 131 LSB/(°/s)
  float gyroX_dps = gyroX / 131.0;
  float gyroY_dps = gyroY / 131.0;
  float gyroZ_dps = gyroZ / 131.0;
  
  // Temperature: (TEMP_OUT - RoomTemp_Offset)/Temp_Sensitivity + 21°C
  float temperature = (temp / 333.87) + 21.0;
  
  // Read magnetometer if available
  int16_t magX = 0, magY = 0, magZ = 0;
  if (hasMagnetometer) {
    // Check if data is ready (ST1 register bit 0)
    Wire.beginTransmission(MAG_ADDRESS);
    Wire.write(0x02); // ST1 register
    Wire.endTransmission(false);
    Wire.requestFrom(MAG_ADDRESS, 1);
    uint8_t st1 = Wire.read();
    
    if (st1 & 0x01) { // Data ready
      // Read magnetometer data (7 bytes: X_L, X_H, Y_L, Y_H, Z_L, Z_H, ST2)
      Wire.beginTransmission(MAG_ADDRESS);
      Wire.write(MAG_XOUT_L);
      Wire.endTransmission(false);
      Wire.requestFrom(MAG_ADDRESS, 7);
      
      magX = Wire.read() | (Wire.read() << 8);
      magY = Wire.read() | (Wire.read() << 8);
      magZ = Wire.read() | (Wire.read() << 8);
      Wire.read(); // ST2 register (must read to trigger next measurement)
    }
  }
  
  // Convert magnetometer to µT (sensitivity is 0.6 µT/LSB for 16-bit mode)
  float magX_uT = magX * 0.6;
  float magY_uT = magY * 0.6;
  float magZ_uT = magZ * 0.6;
  
  // Print CSV format for Serial Plotter
  Serial.print(accelX_mps2);
  Serial.print(",");
  Serial.print(accelY_mps2);
  Serial.print(",");
  Serial.print(accelZ_mps2);
  Serial.print(",");
  Serial.print(gyroX_dps);
  Serial.print(",");
  Serial.print(gyroY_dps);
  Serial.print(",");
  Serial.print(gyroZ_dps);
  Serial.print(",");
  Serial.print(temperature);
  
  if (hasMagnetometer) {
    Serial.print(",");
    Serial.print(magX_uT);
    Serial.print(",");
    Serial.print(magY_uT);
    Serial.print(",");
    Serial.print(magZ_uT);
  }
  
  Serial.println();
  
  delay(20); // 50Hz sampling
}