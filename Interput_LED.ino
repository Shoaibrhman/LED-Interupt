
// 💡 Corrected Code for Older ESP32 Core API (e.g., v3.3.2)

hw_timer_t *timer = NULL;
volatile bool ledState = false;
const int ledPin = BUILTIN_LED;

// Define alarm values (in counts)
// The frequency is set to 1MHz (1 us per count) in setup.
const uint64_t FAST_ALARM = 200000; // 0.2 seconds -> 200ms
const uint64_t SLOW_ALARM = 2000000; // 2 seconds -> 2000ms
const uint64_t DEFAULT_ALARM = 1000000; // 1 second -> 1000ms

// Current alarm value
volatile uint64_t currentAlarm = DEFAULT_ALARM;

// ⚡ Interrupt Service Routine (ISR)
// IRAM_ATTR is crucial for functions called by interrupts
void IRAM_ATTR onTimer() {
  ledState = !ledState;
  // timerWrite(timer, 0); // Not needed for the old API's timerAlarm/autoreload
}

// ⚙ Function to change the timer's alarm
// --- USING OLDER FUNCTIONS ---
void setAlarm(uint64_t newAlarm) {
  // In the older API, you can't easily disable and write separately.
  // The 'timerAlarm' function combines setting the alarm value and enabling it.
  
  // The timerAlarm function:
  // 1. Sets the next alarm count.
  // 2. Sets the autoreload state (true).
  // 3. Enables the alarm.
  timerAlarm(timer, newAlarm, true, 0); 
  
  currentAlarm = newAlarm;
  
  // Convert microseconds to seconds for user feedback
  float seconds = (float)newAlarm / 1000000.0;
  Serial.printf("Blinking set to %0.2f seconds interval.\n", seconds);
}


// 🛠 Setup Function
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED off

  // 1. Initialize the timer
  // --- USING OLDER FUNCTION ---
  // The older timerBegin only takes the frequency (in Hz).
  // 1 MHz frequency means 1 microsecond per count.
  timer = timerBegin(1000000); 
  
  // 2. Attach the ISR
  // --- USING OLDER FUNCTION ---
  // The older timerAttachInterrupt does not take a third argument (edge/level).
  timerAttachInterrupt(timer, &onTimer); 
  
  // 3. Set the initial alarm and enable it
  // --- USING OLDER FUNCTION ---
  // timerAlarm(timer, microseconds, autoreload, count_to_clear_interrupts) 
  // We use 0 as the last argument, which is a common value in this older API usage.
  timerAlarm(timer, DEFAULT_ALARM, true, 0); 
  
  Serial.println("Timer configured and blinking at default (1 second) interval.");
  Serial.println("Send 'F' for fast (0.2s), 'S' for slow (2s), or 'X' for default (1s) from Serial Monitor.");
  Serial.println("---");
}

// 🔄 Main Loop Function
void loop() {
  // Check if a timer interrupt has occurred and update the LED state
  digitalWrite(ledPin, ledState);

  // Handle Serial Input
  if (Serial.available()) {
    char command = Serial.read();
    
    // Convert to uppercase for flexible input
    command = toupper(command); 

    if (command == 'F') {
      if (currentAlarm != FAST_ALARM) {
        setAlarm(FAST_ALARM);
      }
    } else if (command == 'S') {
      if (currentAlarm != SLOW_ALARM) {
        setAlarm(SLOW_ALARM);
      }
    } else if (command == 'X') {
      if (currentAlarm != DEFAULT_ALARM) {
        setAlarm(DEFAULT_ALARM);
      }
    } else if (command != '\n' && command != '\r') {
      // Ignore newline and carriage return characters
      Serial.printf("Unknown command '%c'. Please send 'F', 'S', or 'X'.\n", command);
    }
  }

  // Small delay to prevent serial check from monopolizing the CPU
  delay(10); 
}