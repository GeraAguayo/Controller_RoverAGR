/*
Controller - Rover AGR
Gerardo Aguayo, 2025
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <U8g2lib.h>

// HC12 Config
#define HC12_RX 11
#define HC12_TX 10
SoftwareSerial HC12(10, 11); 

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Comandos
char motor_map[6][5] = { {'M','0',' ',' ','\n'}, {'M','1',' ',' ','\n'}, {'M','2',' ',' ','\n'}, {'M','3',' ',' ','\n'}, {'M','4',' ',' ','\n'}, {'M','5',' ',' ','\n'} };
char forward[5]  = { 'F', ' ', ' ', ' ', '\n' };
char backward[5] = { 'B', ' ', ' ', ' ', '\n' };
char stop_cmd[5] = { 'S', ' ', ' ', ' ', '\n' };
char servo_r[5]  = { 'R', ' ', ' ', ' ', '\n' };
char servo_l[5]  = { 'L', ' ', ' ', ' ', '\n' };
char cam_r[5]    = { 'C', 'R', ' ', ' ', '\n' };
char cam_l[5]    = { 'C', 'L', ' ', ' ', '\n' };

// Pins
#define cam_left_btn 3
#define cam_right_btn 4
#define brake_btn 5
#define accelerator_btn 6
#define map_btn 7
#define reverse_btn 8
#define joy_x A0

// Variables de estado
byte current_map = 0;
int prev_angle = 90;
unsigned long lastJoyTime = 0;
bool last_brake = LOW, last_accel = LOW, last_rev = LOW;
bool cam_left_prev = LOW, cam_right_prev = LOW, map_input_prev = LOW;
char map_str[3];

void setup() {
  Serial.begin(9600);
  HC12.begin(2400);
  u8g2.begin();
  splash_screen();

  pinMode(cam_left_btn, INPUT_PULLUP);
  pinMode(cam_right_btn, INPUT_PULLUP);
  pinMode(brake_btn, INPUT_PULLUP);
  pinMode(accelerator_btn, INPUT_PULLUP);
  pinMode(map_btn, INPUT_PULLUP);
  pinMode(reverse_btn, INPUT_PULLUP);
}

void loop() {
  if (millis() - lastJoyTime > 50) { 
    int x_value = analogRead(joy_x);
    int angle;
    if (x_value <= 450) angle = map(x_value, 0, 450, 0, 90);
    else if (x_value >= 560) angle = map(x_value, 560, 1023, 90, 180);
    else angle = 90;

    if (abs(angle - prev_angle) > 3) {
      char p1 = (angle / 100) + '0';
      char p2 = ((angle % 100) / 10) + '0';
      char p3 = (angle % 10) + '0';
      
      servo_r[1] = p1; servo_r[2] = p2; servo_r[3] = p3;
      servo_l[1] = p1; servo_l[2] = p2; servo_l[3] = p3;
      
      HC12.write(servo_r, 5);
      delay(5); 
      HC12.write(servo_l, 5);
      
      prev_angle = angle;
    }
    lastJoyTime = millis();
  }

  bool brake_in = !digitalRead(brake_btn);
  bool accel_in = !digitalRead(accelerator_btn);
  bool rev_in = !digitalRead(reverse_btn);
  bool m_btn = !digitalRead(map_btn);
  bool c_l = !digitalRead(cam_left_btn);
  bool c_r = !digitalRead(cam_right_btn);


  if (brake_in && !last_brake) { 
    HC12.write(stop_cmd, 5);
    Serial.println("Brake btn pressed");
    print_display("BRAKE", 2);
  }

  if (accel_in && !last_accel) { 
    HC12.write(forward, 5); 
    Serial.println("Accelerator btn pressed");
    print_display("FORWARD", 2);
  }

  if (rev_in && !last_rev) { 
    HC12.write(backward, 5); 
    Serial.println("Reverse btn pressed");
    print_display("REVERSE", 2);
  }

  if (m_btn && !map_input_prev) {
    if (current_map < 5) current_map++;
    else current_map = 0;
    
    HC12.write(motor_map[current_map], 5);
    Serial.print("Motor map changed to: ");
    Serial.println(current_map);
    
    map_str[0] = 'M';
    map_str[1] = current_map + '0';
    map_str[2] = '\0';

    if (current_map == 0) print_display("STOP", 2);
    else print_display(map_str, 2);
  }

  if (c_l && !cam_left_prev) {
    HC12.write(cam_l, 5);
    Serial.println("CAM Left btn pressed");
    print_display("CAM L", 1);
  }
  if (c_r && !cam_right_prev) {
    HC12.write(cam_r, 5);
    Serial.println("CAM Right btn pressed");
    print_display("CAM R", 1);
  }

  last_brake = brake_in;
  last_accel = accel_in;
  last_rev = rev_in;
  map_input_prev = m_btn;
  cam_left_prev = c_l;
  cam_right_prev = c_r;
}

void drawCenteredText(const char* msg, int y_center) {
  uint16_t w = u8g2.getStrWidth(msg);
  int x = (u8g2.getDisplayWidth() - w) / 2;
  u8g2.drawStr(x, y_center, msg);
}

void splash_screen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso16_tf);
  drawCenteredText("ROVER", 24);
  u8g2.sendBuffer();
  delay(800);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso18_tf);
  drawCenteredText("AGR", 24);
  u8g2.sendBuffer();
  delay(800);
}

void print_display(const char* msg, int size) {
  u8g2.clearBuffer();
  if (size == 1) u8g2.setFont(u8g2_font_6x12_tf);
  else if (size == 2) u8g2.setFont(u8g2_font_logisoso16_tf);
  else u8g2.setFont(u8g2_font_logisoso18_tf);
  drawCenteredText(msg, 24);
  u8g2.sendBuffer();
}