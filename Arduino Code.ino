#include <LiquidCrystal.h>

#define current_sensor A3
#define dc_bias_pin A5
#define _NUMBER_OF_SAMPLES 172
#define rs 10
#define enable 9
#define d4 7
#define d5 6
#define d6 5
#define d7 4
#define LED 13
#define MASTER_BUTTON_INT 3
#define ROTARY_CLK_INT 2
#define ROTARY_DT 8
#define ROTARY_BUTTON 11
#define LED 12

signed int samples_raw[_NUMBER_OF_SAMPLES];
signed int dc_bias;
float result;
float average_result;
LiquidCrystal lcd(rs, enable, d4, d5, d6, d7);
signed char direction = 0;
volatile bool current_state_CLK = 0;
volatile bool previous_state_CLK = 0;
volatile bool master_flag = 1;
int counter = 0;
signed char watt_num[3] = { 0 };
char watt_index = 0;
int watt_value = 0;
bool button_previous_state = 0;
bool button_current_state = 0;
float expected_current = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(ROTARY_BUTTON, INPUT);
  pinMode(MASTER_BUTTON_INT, INPUT);
  pinMode(ROTARY_CLK_INT, INPUT);
  pinMode(ROTARY_DT, INPUT);
  previous_state_CLK = digitalRead(ROTARY_CLK_INT);
  attachInterrupt(digitalPinToInterrupt(MASTER_BUTTON_INT), M_FLAG, RISING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_CLK_INT), Direction, CHANGE);
  Serial.begin(9600);
  button_previous_state = digitalRead(ROTARY_BUTTON);
  lcd.begin(16, 2);
  lcd.setCursor(5, 0);
  lcd.print("Welcome ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("AC Current");
  lcd.setCursor(2, 1);
  lcd.print("Sensing Meter");
  delay(2000);
  lcd.clear();
}

void loop() {
  digitalWrite(LED, LOW);
  if (master_flag == HIGH) {
    digitalWrite(LED, LOW);
    button_current_state = digitalRead(ROTARY_BUTTON);
    if (button_current_state == HIGH && button_previous_state == LOW) {
      watt_index++;
      if (watt_index > 2) {
        watt_index = 0;
      }
    }
    button_previous_state = button_current_state;
    if (direction == 1) {
      watt_num[watt_index] = watt_num[watt_index] + 1;
      if (watt_num[watt_index] > 9) {
        watt_num[watt_index] = 0;
      }
    } else if (direction == -1) {
      watt_num[watt_index] = watt_num[watt_index] - 1;
      if (watt_num[watt_index] < 0) {
        watt_num[watt_index] = 9;
      }
    } else {
    }
    direction = 0;
    watt_value = 100 * watt_num[0] + 10 * watt_num[1] + watt_num[2];
    lcd.setCursor(0, 0);
    lcd.print("Watt Value =");
    lcd.print(watt_value);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("Enter Watt Value");
    lcd.print("  ");
  } else if ((master_flag != HIGH)) {

    average_result = 0;
    for (unsigned char number_of_cycles = 0; number_of_cycles < 50; number_of_cycles++) {
      dc_bias = analogRead(dc_bias_pin);
      result = 0;
      for (unsigned int i = 0; i < _NUMBER_OF_SAMPLES; i++) {
        samples_raw[i] = analogRead(current_sensor);
      }
      for (unsigned int i = 0; i < _NUMBER_OF_SAMPLES; i++) {
        samples_raw[i] = samples_raw[i] - dc_bias;
        if (abs(samples_raw[i]) < 3) {
          samples_raw[i] = 0;
        }
        result = result + pow(samples_raw[i] * 5.0 / 1023.0, 2);
      }
      result = result / _NUMBER_OF_SAMPLES;
      result = sqrt(result);
      result = result * 5;

      average_result = average_result + result;
    }
    average_result = average_result / 50.0;
    expected_current = watt_value / 220.0;
    while (average_result - expected_current > 0.2 && master_flag != HIGH) {
      lcd.setCursor(0, 0);
      lcd.print("     ERROR      ");
      lcd.setCursor(0, 1);
      lcd.print("  HIGH CURRENT  ");
      digitalWrite(LED, HIGH);
      for (unsigned char number_of_cycles = 0; number_of_cycles < 50; number_of_cycles++) {
        dc_bias = analogRead(dc_bias_pin);
        result = 0;
        for (unsigned int i = 0; i < _NUMBER_OF_SAMPLES; i++) {
          samples_raw[i] = analogRead(current_sensor);
        }
        for (unsigned int i = 0; i < _NUMBER_OF_SAMPLES; i++) {
          samples_raw[i] = samples_raw[i] - dc_bias;
          if (abs(samples_raw[i]) < 3) {
            samples_raw[i] = 0;
          }
          result = result + pow(samples_raw[i] * 4.85  / 1023.0, 2);
        }
        result = result / _NUMBER_OF_SAMPLES;
        result = sqrt(result);
        result = result * 5;

        average_result = average_result + result;
      }
      average_result = average_result / 50.0;
    }



    lcd.setCursor(0, 0);
    lcd.print("AC I RMS: ");
    lcd.print(average_result);
    lcd.print("     ");
    lcd.setCursor(0, 1);
    lcd.print("Watt Value =");
    lcd.print(watt_value);
    lcd.print("  ");
  }
}


void M_FLAG() {
  master_flag = !master_flag;
  direction = 0;
}

void Direction() {
  current_state_CLK = digitalRead(ROTARY_CLK_INT);

  if (current_state_CLK == HIGH && previous_state_CLK == LOW) {
    if (digitalRead(ROTARY_DT) != current_state_CLK) {
      direction = 1;
    } else {
      direction = -1;
    }
  }
  previous_state_CLK = current_state_CLK;
}