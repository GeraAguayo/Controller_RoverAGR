/*
Controller
Rover AGR
Gerardo Aguayo, 2025
*/
#include <SoftwareSerial.h>

//HC12 Config
const uint32_t HC12_BAUD = 2400;
#define HC12_RX 10
#define HC12_TX 11
SoftwareSerial HC12(HC12_TX, HC12_RX);

//OLED Display config 
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(
  U8G2_R0,
  /* reset=*/ U8X8_PIN_NONE
);
void splash_screen();
void print_display( const char* msg);
void print_log_display(const char* log);
void drawCenteredText(const char* msg, int y_center);
char map_str[3];

//Commands for MCM
//Motor map
char motor_map[6][5] = {
    { 'M', '0', ' ', ' ', '\n' },
    { 'M', '1', ' ', ' ', '\n' },
    { 'M', '2', ' ', ' ', '\n' },
    { 'M', '3', ' ', ' ', '\n' },
    { 'M', '4', ' ', ' ', '\n' },
    { 'M', '5', ' ', ' ', '\n' }
};
int current_map = 0;
//Motion control
char forward[5]  = { 'F', ' ', ' ', ' ', '\n' };
char backward[5] = { 'B', ' ', ' ', ' ', '\n' };
char test[5]     = { 'T', ' ', ' ', ' ', '\n' };
char stop[5]     = { 'S', ' ', ' ', ' ', '\n' };
//Servo control
char servo_r[5]  = { 'R', ' ', ' ', ' ', '\n' };
char servo_l[5]  = { 'L', ' ', ' ', ' ', '\n' };
//Other controls
char toggle_light[5]  = { 'K', ' ', ' ', ' ', '\n' };
char cam_r[5]  = { 'C', 'R', ' ', ' ', '\n' };
char cam_l[5]  = { 'C', 'L', ' ', ' ', '\n' };

//Controller buttons
#define light_btn 2
#define cam_left_btn 3
#define cam_right_btn 4
#define brake_btn 5//
#define accelerator_btn 6//
#define map_btn 7//
#define reverse_btn 8//
bool light_input = false;
bool cam_left_input = false;
bool cam_right_input = false;
bool brake_input = false;
bool accelerator_input = false;
bool map_input = false;
bool map_input_prev = false;
bool reverse_input = false;


//Joystick config
#define joy_x A0
int starting_val_x;
const int DEAD_ZONE_JOYSTICK = 45;
int angle, prev_angle {};

void setup() {
  
  Serial.begin(9600);
  Wire.begin();
  u8g2.begin();
  delay(50);
  splash_screen();

  HC12.begin(2400);

  pinMode(light_btn, INPUT_PULLUP);
  pinMode(cam_left_btn, INPUT_PULLUP);
  pinMode(cam_right_btn, INPUT_PULLUP);
  pinMode(brake_btn, INPUT_PULLUP);
  pinMode(accelerator_btn, INPUT_PULLUP);
  pinMode(map_btn, INPUT_PULLUP);
  pinMode(reverse_btn, INPUT_PULLUP); 
  starting_val_x= analogRead(joy_x);
}

void loop(){
  //Read joystick input
  int x_value = analogRead(joy_x);
  //Calculate angle
  if (x_value <= 450) {
    angle = map(x_value, 0, 450, 0, 90);
  }
  else if (x_value >= 560) {
    angle = map(x_value, 560, 1023, 90, 180);
  }
  else {
    angle = 90;
  }
  angle = constrain(angle, 0, 180);
  //Send angle
  if (angle != prev_angle){
    Serial.print("Angle: ");
    Serial.println(angle);
    int arg_1 = angle / 100;
    int arg_2 = (angle % 100) / 10;
    int arg_3 = angle % 10;
    char par_1 = arg_1 + '0';
    char par_2 = arg_2 + '0';
    char par_3 = arg_3 +'0';
    //prepare value to send
    servo_r[1]  = par_1;
    servo_r[2]  = par_2;
    servo_r[3]  = par_3;
    servo_l[1]  = par_1;
    servo_l[2]  = par_2;
    servo_l[3]  = par_3;
    //Send and clear
    HC12.write(servo_r, 5);
    HC12.write(servo_l, 5);
    servo_r[1]  = ' ';
    servo_r[2]  = ' ';
    servo_r[3]  = ' ';
    servo_l[1]  = ' ';
    servo_l[2]  = ' ';
    servo_l[3]  = ' ';

    prev_angle = angle;

  }

  //Read buttons input
  light_input = digitalRead(light_btn);
  cam_left_input = digitalRead(cam_left_btn);
  cam_right_input = digitalRead(cam_right_btn);
  brake_input = digitalRead(brake_btn);
  accelerator_input = digitalRead(accelerator_btn);
  map_input = digitalRead(map_btn);
  reverse_input = digitalRead(reverse_btn);

  if (light_input == HIGH){
    Serial.println("Light btn pressed");
    HC12.write(toggle_light, 5);
  }
  if (cam_left_input){
    Serial.println("CAM Left btn pressed");
    HC12.write(cam_l, 5);
  }
  if (cam_right_input){
    Serial.println("CAM Right btn pressed");
    HC12.write(cam_r, 5);
  }
  if (brake_input){
    Serial.println("Brake btn pressed");
    HC12.write(stop, 5);
  }
  if (accelerator_input){
    Serial.println("Accelerator btn pressed");
    HC12.write(forward, 5);
  }
  if (reverse_input){
    Serial.println("Reverse btn pressed");
    HC12.write(backward, 5);
  }
  if (map_input == HIGH && map_input_prev == LOW){
    if (current_map < 5){
      current_map++;
    }
    else{
      current_map = 0;
    }
    HC12.write(motor_map[current_map], 5);
    Serial.print("Motor map changed to: ");
    Serial.println(current_map);
    map_str[0] = 'M';
    map_str[1] = current_map + '0';
    map_str[2] = '\0';
    if (current_map == 0){
      print_display("STOP",2);
    }
    else{
      print_display(map_str,2);
    }
  }
  map_input_prev = map_input;


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

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso16_tf);
  drawCenteredText("ROVER", 24);
  u8g2.sendBuffer();
  delay(800);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso18_tf);
  drawCenteredText("AGR", 24);
  u8g2.sendBuffer();
}

void print_display(const char* msg, int size) {
  u8g2.clearBuffer();

  if (size == 1)
    u8g2.setFont(u8g2_font_6x12_tf);
  else if (size == 2)
    u8g2.setFont(u8g2_font_logisoso16_tf);
  else
    u8g2.setFont(u8g2_font_logisoso18_tf);

  drawCenteredText(msg, 24);
  u8g2.sendBuffer();
}


