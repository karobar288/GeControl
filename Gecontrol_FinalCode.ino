#include <SoftwareSerial.h>                    //Bluetooth (UART)
#include <Adafruit_BMP280.h>                   //Sensor temp i wilgotnosci
#include <Wire.h>                              //Dla interfejsu I2C (BMP280 i SSD1306)
#include <Stepper.h>                           //Silnik krokowy 28BYJ-48 + Sterownik ULN2003
#include <Adafruit_GFX.h>                      //Biblioteka graficzna do ssd1306
#include <Adafruit_SSD1306.h>                  //Wyświetlacz OLED ssd1306

int state;                                     //Zmienna dla bluetooth
int kr1s = digitalRead(6);                     //Stan krańcówki prawej
int kr2s = digitalRead(5);                     //Stan krańcówki lewej
#define Led  13                                //Listwa Led
#define Pump  7                                //Pompa wody

Adafruit_SSD1306 display(4);                   //Ustawienie resetu wyświetlacza na 4. nóżce
Adafruit_BMP280 bmp;                           //I2C Sensor temp i wilgotnosci
Stepper mojSilnik(32, 8, 10, 9, 11);           //Ilość kroków silnika oraz piny Arduino do sterowania silnikiem
SoftwareSerial BTSerial(2, 3);                 //Piny RX i TX do wysyłania i odbierania danych przez Bluetooth


void setup() {

  digitalWrite(Pump, LOW);                     //Wyłączenie pompy
  BTSerial.begin(9600);                        //Bluetooth baud rate: 9600
  pinMode(2, INPUT);                           //Bluetooth RX 
  pinMode(3, OUTPUT);                          //Bluetooth TX
  pinMode(5, INPUT);                           //Krańcówka lewa
  pinMode(6, INPUT);                           //Krańcówka prawa
  pinMode(Led, OUTPUT);                        //Listwa led 
  pinMode(Pump, OUTPUT);                       //Pompa wody


  bmp.begin();                                      //inicjalizacja czujnika bmp280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */



  display.begin(SSD1306_SWITCHCAPVCC,0x3C);     //Inicjalizacja wyświetlacza oled
  display.clearDisplay();                       //Wyczyszczenie wyświetlacza oled

  OledTemp();                                   //Procedura wyświetlenia temperatury na ssd1306

  kr1s = digitalRead(6);                        //Stan krańcówki prawej
  kr2s = digitalRead(5);                        //Stan krańcówki lewej
  mojSilnik.setSpeed(600);                      //Podajemy prędkość obrotu wyrażona w rpm  


  while(kr1s == LOW && kr2s == LOW)
  {
    kr1s = digitalRead(6);                     //Stan krańcówki prawej
    mojSilnik.step(-1);                        //Ruch silnikiem w prawo
  }
  mojSilnik.step(750);                         //ruch silnikiem w lewo do pierwszego leja

}

void loop() {

  BTSerial.print(bmp.readTemperature());       //wysłanie danych temperatury przez bluetooth
  BTSerial.print(" C");

  OledTemp();                                  //Procedura wyświetlenia temperatury na ssd1306

    kr1s = digitalRead(6);                     //Stan krańcówki prawej
    kr2s = digitalRead(5);                     //Stan krańcówki lewej

    for(int i=0;i<=30;i++)
    {
      delay(50);

      if(BTSerial.available() > 0)
      {
      state = BTSerial.read();                 //ustawienie funkcji warunkowej odczytu portu szeregowego 
      } 

      if (state == '2')
      { 
        digitalWrite(Led, HIGH);               //Włączenie listwy led
        delay(50);
      }   

      else if (state == '3') 
      {  
        digitalWrite(Led, LOW);                //Wyłączenie listwy led
        delay(50);   
      }
      
      else if (state == '4')
      {  
        for(int i=0;i<1000;i++)
        { 
          kr2s = digitalRead(5);               //Stan krańcówki lewej
          if(kr2s == LOW){
          mojSilnik.step(1);}                  //Ruch silnikiem w lewo
        }  
        state = 0;
      }

      else if (state == '5')
      {       
        for(int i=0;i<1000;i++)
        { 
          kr1s = digitalRead(6);               //Stan krańcówki prawej
          if(kr1s == LOW){
          mojSilnik.step(-1);}                 //Ruch silnikiem w prawo
        }  
        state = 0;
      }
      
      else if (state == '6') 
      {
        digitalWrite(Pump, HIGH);              //Uruchomienie pompy
        delay(50); 
      }
      
      else if (state == '7')
      {
        digitalWrite(Pump, LOW);               //Wyłączenie pompy
        delay(50); 
      }
  }                                            //for 30
}                                              //void loop




void OledTemp()
{
  display.fillRect(32, 16, 100, 126, BLACK);   //Zakrycie przestrzeni w której poprzednio wyświetlała się temperatura 
  display.setTextSize(1);                      //Ustawienie wielkości czcionki
  display.setTextColor(WHITE);                 //Ustawienie koloru tekstu na biały
  display.setCursor(40,4);                     //Ustawienie pozycji kursora 
  display.print("GeControl");                  //Wyświetlenie napisu
  display.display();                           //Wysłanie ustawień i treści do wyświetlenia na wyświetlaczu
  display.setTextSize(2);                      //Ustawienie wielkości czcionki
  display.setCursor(32,16);                    //Ustawienie pozycji kursora
  display.print(bmp.readTemperature());        //Wyświetlenie temperatury z czujnika bmp280
  display.println("C");         
  display.display();                           //Wysłanie ustawień i treści do wyświetlenia na wyświetlaczu
}