#include<LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(A3, A2);

//__________________________Water flow sensor Variables ___________________________
byte sensorInterrupt = 0;  
byte sensorPin       = 2;
float calibrationFactor = 4.5;
volatile byte pulseCount;  
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;

//__________________________Pump filling Mechanism __________________________________
bool fill_state = true;
int count_litres = 0;
int buzzer = 10;
int solenoid = A1;

//__________________________Users Management system _________________________________

String password = "";
String Room_number = "";
int program_stage = 0;

/*
 1 --> for room number
 2---> for password 
 *---->Starting and stopping  
 #---->Sending & excuting cmd 
 C---->Clar
 */
 
String Rooms[4] = {"200", "204", "208", "309"};
String Wapangaji[4]= {"Halifa", "Kareem", "Keneth", "Miriam"};
String Rooms_passcode[4] = {"1111", "2000", "2222", "1234"};
int Litre_used[4]= {0, 0, 0, 0};
int price_per_litre = 3;

class Mpangaji{

    public:
          String password="";
          int Litre_used=0;
          int Room_number=0;

    int get_index(String Room_no){
          for (int mpangaji = 0; sizeof(Rooms_passcode)/sizeof(Rooms_passcode[0]); mpangaji++){
              if (Rooms[mpangaji] == Room_no){
                return mpangaji;
              }
             continue;
          }
    }

   int get_litre_used(){
        int index = get_index(this->password);
        return this->Litre_used;
   }

   String get_room_number(){
        int index = get_index(this->password);
        return Rooms[index];
   }
   
}Jumanji;
// _________________________end of user Management Class _____________


//__________________________Configuring Lcd using I2C ________________
LiquidCrystal_I2C Lcd(0x3F, 16, 2);


//__________________________configuring Keypad________________________
const byte ROWS = 4;
const byte COLS = 4; 
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 11}; 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//__________________________end configuring keypad ________________________



void SendMessage(String message)
{
  mySerial.println();
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+255672056928\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println(message);// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  mySerial.println();
  delay(4000);
}


void setup(){
    pinMode(buzzer, OUTPUT);
    pinMode(solenoid, OUTPUT);
    mySerial.begin(115200); 
    Serial.begin(9600);
    SendMessage("Customer activen  ");
    Lcd.init();
    Lcd.backlight();
    render_initial_menu();
    keypad.addEventListener(keypadEvent);
    Serial.println("Setup completed");

    //_________Water flow sensor configuration______________
    pinMode(sensorPin, INPUT);
    digitalWrite(sensorPin, HIGH);
    pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void loop(){
          keypad.getKey();
}



void render_initial_menu(){
    Lcd.clear();
    Lcd.setCursor(0,0);
    Lcd.print("Water Billing");
    Lcd.setCursor(5, 1);
    Lcd.print("System");
    delay(1500);
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Ingiza namba ya ");
    Lcd.setCursor(3, 1);
    Lcd.print("Chumba : ");
    Lcd.setCursor(12, 1);

      pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;
  }



void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
    case PRESSED:
                if(key == 'C'){
                      if (program_stage == 0){
                              Room_number = "";
                              render_initial_menu();
                      }

                      else if (program_stage == 1){
                              password = "";
                              render_password_menu();
                      }
                }

                else if (key == '#'){
                    if (program_stage == 0){
                          Serial.println("Excuting stage 1"); 
                          Serial.println(Room_number);
                          handle_stage_1(Room_number);
                    }

                    else if (program_stage == 1){
                          Serial.println("Excuting stage 2");
                          Serial.println(password);
                          handle_stage_2(password);
                    }

                    else if (program_stage == 2){
                        Serial.println("Sending notification");
                        String Name = Wapangaji[Jumanji.get_index(Room_number)];
                        String notification_1 = "Name :"+Name;
                        String notification_2 = "Litre used : "+ String(count_litres);
                        String notification_3 = "Price : "+ String(count_litres * price_per_litre) + " TSh"; 
                        String sms = "Name : "+Name+"\n"+notification_2+"\n"+notification_3;
                        send_notification(notification_2, notification_3);
                        SendMessage(sms);
                        delay(2000);

                        
                    }
                }

                else if (key == '*'){
                         if (program_stage == 2 ){
                              if (fill_state == true){
                                  Serial.println("Excuting stage 3");
                                  fill_state = false;
                                  digitalWrite(solenoid, 1);
                                  render_counting_Litres();
                              }
                              else{
                                  Serial.println("breaking the loop");
                                  fill_state = true;
                                  digitalWrite(solenoid , 0);  
                              }
                         }
                }

                else{
                      Lcd.print(key);
                      if (program_stage == 0){
                            Room_number+=key;
                      }

                      else if (program_stage == 1 ){
                            password+=key;
                      }

                      else if (program_stage == 2){
                        
                      }

                      else{
                        
                      }
                }
        break;

    case RELEASED:
        break;

    case HOLD:
        break;
    }
}


void handle_stage_1(String Room_no){
      bool exist = false;
      for (int room = 0; room<sizeof(Rooms)/sizeof(Rooms[0]); room ++){
            if (Rooms[room] == Room_no){
                  Serial.println("Room exist");
                  exist = true;
                  render_password_menu();
            }
      }

      if (!exist){
            Serial.println("Invalid room number");
            reset_room_no();
      }
      
}

void handle_stage_2(String pass_word){
  bool verified = false;
  int index = Jumanji.get_index(Room_number);
  if (Rooms_passcode[index] == pass_word){
      Serial.println("valid password");
      draw_water_menu();
      
      
  }
  else{
    Serial.println("Invalid passcode");
    reset_password_menu();
  }
}

void send_notification(String notify_1, String notify_2){
      Lcd.clear();
      Lcd.setCursor(0, 0);
      Lcd.print(notify_1);
      Lcd.setCursor(0, 1);
      Lcd.print(notify_2);
      delay(1500);
      digitalWrite(buzzer,1);
      Lcd.clear();
      Lcd.setCursor(0, 1);
      Lcd.print("Sending ... ");
      delay(1500);
      digitalWrite(buzzer, 0);
      Room_number = "";
      password = "";
      program_stage = 0;
      count_litres = 0;
      render_initial_menu();
}



void reset_room_no(){
  Lcd.clear();
  Lcd.setCursor(0, 0);
  Lcd.print("Room Doesn't");
  Lcd.setCursor(0, 1);
  Lcd.print("Exist");
  delay(1000);
  Lcd.clear();
  Lcd.setCursor(0, 0);
  Lcd.print("System rebooting");
  delay(1000);
  Room_number = "";
  render_initial_menu();
}

void reset_password_menu(){
  Lcd.clear();
  Lcd.setCursor(0,0);
  Lcd.print("Invalid password");
  Lcd.setCursor(0,1);
  Lcd.print("Try again");
  delay(2000);
  Lcd.clear();
  Lcd.setCursor(0,0);
  Lcd.print("Rebooting password");
  Lcd.setCursor(8, 1);
  Lcd.print("Menu");
  delay(1500);
  password = "";
  render_password_menu();
}

void draw_water_menu(){
     Lcd.clear();
     Lcd.setCursor(0,0);
     Lcd.print("You're verified");
     delay(1000);
     Lcd.clear();
     Lcd.setCursor(0,0);
     Lcd.print("Press * to fill");
     Lcd.setCursor(0, 1);
     Lcd.print("Same key to stop");
     program_stage = 2;
}

void filling_text(){
      Lcd.clear();
      Lcd.setCursor(0,0);
      Lcd.print("Filling ");
      Lcd.setCursor(0, 1);
      Lcd.print("Litres: ");
}
void render_counting_Litres(){
      if (count_litres==0) filling_text();
      while (1 == 1){
            if((millis() - oldTime) > 1000){ 
                detachInterrupt(sensorInterrupt);
                flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
                oldTime = millis();
                flowMilliLitres = (flowRate / 60) * 1000;
                totalMilliLitres += flowMilliLitres;   
                unsigned int frac;
                Serial.print("Output: ");        
                Serial.print(totalMilliLitres/1000);
                Serial.println("L");
                pulseCount = 0;
                attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
                Lcd.print(totalMilliLitres/1000);
                Lcd.setCursor(10,1);
                char closed_key = keypad.getKey();
                if (closed_key == '*'){
                    Serial.println("water filled");
                    count_litres = totalMilliLitres/1000;   
                    Serial.println(count_litres); 
                    break;
                }
                delay(100);
      }}
      
      Serial.println("Done");
}

void render_password_menu(){
      program_stage = 1;
      Lcd.clear();
      Lcd.setCursor(0, 1);
      Lcd.print("Passcode : ");
      Lcd.setCursor(10, 1);
}


void pulseCounter(){
  pulseCount++;
}
