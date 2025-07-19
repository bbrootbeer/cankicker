void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);  // wait up to 3 seconds
  Serial.println("Hello world!");
}

void loop() {}