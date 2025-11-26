// Flex Sensor Reader for XIAO ESP32 S3
// Reads analog data from A1 (GPIO7) and outputs to Serial Monitor and Plotter

const int flexPin = A1;  // A1 maps to GPIO7 on XIAO ESP32 S3

void setup() {
  Serial.begin(115200);       // Start serial communication
  delay(1000);                // Give time for serial to initialize
  pinMode(flexPin, INPUT);    // Set A1 as input
}

void loop() {
  int flexValue = analogRead(flexPin);  // Read analog value from flex sensor

  // Output to Serial Monitor and Plotter
  Serial.println(flexValue);

  delay(50);  // Small delay for smoother plotting
}
