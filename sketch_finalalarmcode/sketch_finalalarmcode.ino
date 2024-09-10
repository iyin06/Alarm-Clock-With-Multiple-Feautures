#include <DueTimer.h>
#include <RTCDue.h>
#include <LiquidCrystal.h>


#define BUZZER_PIN 11        // Pin connected to the buzzer
#define RESET_BUTTON 2       // Reset button connected to pin 2
#define SNOOZE_BUTTON 3      // Snooze button connected to pin 3
#define START_BUTTON 44      // Button connected to pin 44 (start stopwatch)
#define STOP_BUTTON 42       // Button connected to pin 42 (stop stopwatch)
#define SWITCH_BUTTON 52     // Button connected to pin 52 (to switch between clock and stopwatch modes)
#define STOPWATCH_BUTTON 50
#define LED1 10                // Pin connected to LED1
#define LED2 8                 // Pin connected to LED2


const unsigned long SNOOZE_DURATION = 30000;  // 30,000 milliseconds (30 seconds)

// Initialize the LCD (RS, E, D4, D5, D6, D7) pins
LiquidCrystal lcd(12, 13, 7, 6, 5, 4);


RTCDue rtc(XTAL);


int alarmHour = -1;
int alarmMinute = -1;

// Melody and duration arrays
int melody[] = {262, 294, 330, 349, 392, 440, 494};
int noteDuration = 200;

// Day names for printing the date
const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Alarm state variables
bool alarmRinging = false;
bool snoozeActive = false;
unsigned long snoozeStartTime;

// Stopwatch variables
bool stopwatchMode = false;
bool stopwatchRunning = false;
unsigned long stopwatchStartTime = 0;
unsigned long stopwatchElapsedTime = 0;

// Clock mode
bool clockMode = true;

void setup() {
   
    Serial.begin(9600);

    // Initialize pins
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RESET_BUTTON, INPUT_PULLUP);  // Enable internal pull-up resistor for reset button
    pinMode(SNOOZE_BUTTON, INPUT_PULLUP); // Enable internal pull-up resistor for snooze button
    pinMode(START_BUTTON, INPUT_PULLUP);  // Enable internal pull-up for start stopwatch button
    pinMode(STOP_BUTTON, INPUT_PULLUP);   // Enable internal pull-up for stop stopwatch button
    pinMode(SWITCH_BUTTON, INPUT_PULLUP); // Enable internal pull-up for switch button
    pinMode(STOPWATCH_BUTTON, INPUT_PULLUP);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    // Initialize RTC
    rtc.begin();
    rtc.setHours(11);
    rtc.setMinutes(30);
    rtc.setSeconds(0);
    rtc.setDay(14);
    rtc.setMonth(3);
    rtc.setYear(2025);

    // Initialize LCD
    lcd.begin(16, 2); // Initialize the LCD with 16 columns and 2 rows
}

void loop() {
    if (clockMode) {
        // Clock mode operations
        printDateAndTime();

        // Check if the current time matches the alarm time
        if (rtc.getHours() == alarmHour && rtc.getMinutes() == alarmMinute && rtc.getSeconds() == 0 && !alarmRinging) {
            alarmRinging = true;
            displayWakeUpMessage(); // Display wake-up message on the LCD
        }

        // Handle serial input for setting the alarm
        handleSerialInput();

        // Check for reset button press
        if (digitalRead(RESET_BUTTON) == LOW) {
            alarmRinging = false; // Stop the alarm
            displayStopMessage(); // Display stop message on the LCD
            delay(1000); // Debounce delay
        }

       
        if (digitalRead(SNOOZE_BUTTON) == LOW && alarmRinging) {
            snoozeActive = true;
            snoozeStartTime = millis(); // Record snooze start time
            lcd.clear(); // Clear the LCD display
            lcd.setCursor(0, 0);
            lcd.print("Snooze for 30s"); // Display snooze message
        }

        // If snooze is active and snooze period has elapsed, reset snooze state
        if (snoozeActive && (millis() - snoozeStartTime >= SNOOZE_DURATION)) {
            snoozeActive = false;
        }

        // If alarm is ringing and snooze is not active, play the alarm melody
        if (alarmRinging && !snoozeActive) {
            playAlarmRinger();
        }

    } else if (stopwatchMode) {
        // Stopwatch mode operations
        handleStopwatch();

        // Check for start button press to start the stopwatch
        if (digitalRead(START_BUTTON) == LOW) {
            if (!stopwatchRunning) {
                stopwatchRunning = true;
                stopwatchStartTime = millis(); // Record the start time
            }
            delay(200); // Debounce delay
        }

        // Check for stop button press to stop the stopwatch
        if (digitalRead(STOP_BUTTON) == LOW) {
            if (stopwatchRunning) {
                stopwatchRunning = false;
                stopwatchElapsedTime += millis() - stopwatchStartTime;
            }
            delay(200); // Debounce delay
        }

        // Check for reset button press to reset the stopwatch
        if (digitalRead(RESET_BUTTON) == LOW) {
            stopwatchRunning = false;
            stopwatchElapsedTime = 0; // Reset the elapsed time
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Stopwatch reset");
            delay(200); // Debounce delay
        }
    }

    // Check for switch button press to toggle between clock and stopwatch modes
    if (digitalRead(SWITCH_BUTTON) == LOW) {
        if (clockMode) {
            clockMode = false;
            stopwatchMode = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Stopwatch mode");
        } else {
            clockMode = true;
            stopwatchMode = false;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Clock mode");
        }
        delay(200); // Debounce delay
    }

    delay(1000);
}

void printDateAndTime() {
    // Display the date and time on the serial monitor
    Serial.print(dayNames[rtc.getDayofWeek()]);
    Serial.print(" ");
    Serial.print(rtc.getDay());
    Serial.print("/");
    Serial.print(rtc.getMonth());
    Serial.print("/");
    Serial.print(rtc.getYear());
    Serial.print("\t");
    Serial.print(rtc.getHours());
    Serial.print(":");
    Serial.print(rtc.getMinutes());
    Serial.print(":");
    Serial.println(rtc.getSeconds());

    // Display the date and time on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dayNames[rtc.getDayofWeek()]);
    lcd.print(" ");
    lcd.print(rtc.getDay());
    lcd.print("/");
    lcd.print(rtc.getMonth());
    lcd.print("/");
    lcd.print(rtc.getYear());
    lcd.setCursor(0, 1);
    lcd.print(WithZeros(rtc.getHours(), 2));
    lcd.print(":");
    lcd.print(WithZeros(rtc.getMinutes(), 2));
    lcd.print(":");
    lcd.print(WithZeros(rtc.getSeconds(), 2));
}

void handleStopwatch() {
    // Calculate the current elapsed time
    unsigned long currentTime = stopwatchRunning ? millis() - stopwatchStartTime + stopwatchElapsedTime : stopwatchElapsedTime;

    // Calculate hours, minutes, seconds, and milliseconds
    unsigned long hh = currentTime / 3600000; // Calculate hours
    unsigned long mm = (currentTime % 3600000) / 60000; // Calculate minutes
    unsigned long ss = (currentTime % 60000) / 1000; // Calculate seconds
    unsigned long ms = currentTime % 1000; // Calculate milliseconds

    // Display the stopwatch time on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stopwatch: ");
    lcd.setCursor(0, 1);
    // Display time in hh:mm:ss:ms format as specified
    lcd.print((hh / 10) % 10);
    lcd.print(hh % 10);
    lcd.print(":");
    lcd.print((mm / 10) % 10);
    lcd.print(mm % 10);
    lcd.print(":");
    lcd.print((ss / 10) % 10);
    lcd.print(ss % 10);
    //lcd.print(":");
    //lcd.print((ms / 100) % 10);
    //lcd.print((ms / 10) % 10);
    //lcd.print(ms % 10);
}

void displayWakeUpMessage() {
    // Display "Wake up princess" message on the LCD screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wake up princess");
}

void displayStopMessage() {
    // Display "Get up girl" message on the LCD screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Get up girl");
}

void playAlarmRinger() {
    // Play the alarm melody
    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
        int frequency = melody[i];

        // Flash LEDs
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        delay(100); // Adjust delay to control LED flashing speed
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        for (int j = 0; j < noteDuration * frequency / 1000; j++) {
            digitalWrite(BUZZER_PIN, HIGH);
            delayMicroseconds(500000 / frequency);
            digitalWrite(BUZZER_PIN, LOW);
            delayMicroseconds(500000 / frequency);
        }
        delay(50);
    }
}

void handleSerialInput() {
    // Handle serial input for setting the alarm
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        int index = input.indexOf(':');
        if (index != -1) {
            int inputHour = input.substring(0, index).toInt();
            int inputMinute = input.substring(index + 1).toInt();
            if (inputHour >= 0 && inputHour <= 23 && inputMinute >= 0 && inputMinute <= 59) {
                alarmHour = inputHour;
                alarmMinute = inputMinute;
                Serial.print("Alarm set for ");
                Serial.print(alarmHour);
                Serial.print(":");
                Serial.println(alarmMinute);
            }
        }
    }
}

// Function to add leading zeros to numbers
String WithZeros(int number, int length) {
    String result = String(number);
    while (result.length() < length) {
        result = "0" + result;
    }
    return result;
}
