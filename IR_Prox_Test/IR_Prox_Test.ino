#include <Wire.h> //I2C Library
#include <Adafruit_MLX90614.h> //IR Temperatur Sensor Libary
#include <Adafruit_SSD1306.h> //OLED Library
#include <millisDelay.h> //library looping


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define IRPin 7         //pin 26 for IR Proximity sensor
#define GREEN_LED  4   // pin D3 for green LED 
#define RED_LED  3     // pin D5 for red LED
#define buzzer  2      // pin D7 for buzzer   

int IROutput= HIGH; //MLX output
bool measurement = false;    //check temperature measurement running

const unsigned long interval_sensor = 50; // interval refresh sensor in mS 
millisDelay sensorDelay;                  // the delay object
const unsigned long interval_display = 500; // interval refresh OLED display in mS 
millisDelay displayDelay;                   // the delay object
const unsigned long delay_hold_red = 5000; // delay hold red LED in mS
const unsigned long delay_hold_green = 1000; // delay hold green LED in mS
millisDelay holdDelay;                       // the delay object

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Declaration for an MLX90614 sensor connected to I2C (SDA, SCL pins)
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  mlx.begin();//initialize MLX90614 sensor
  
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // infinite loop to initialize the OLED display
    Serial.println(F("OLED Display allocation failed"));
    for(;;);
  }
  delay(1000);
  //set pins mode
  pinMode(IRPin, 0);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  //set OLED parameters
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 20);
  display.println("Initializing");
  display.display();
  delay(250);
  display.clearDisplay();

  displayDelay.start(interval_display); //refresh display

}

void loop() {
  static float temperature = -1;  //initial condition
  
  IROutput= digitalRead(IRPin);
  if (IROutput == LOW && measurement == false){
    sensorDelay.start(interval_sensor); //read sensor
    displayDelay.finish();              //finish interval refresh for display
    measurement = true;                  //change state of measurement
  }

  if (measurement == true)
  {
    //if sensor reading
    temperature = GetTemp(); //get temperature in celcius
    
  }else
  {
    temperature = -1;  //marker if sensor not reading temperature
  }
  
  ShowTemp(temperature );   //display temperature to OLED
  holdReading();  //call  holdReading function

}
float GetTemp()
{
  static int index = 0;
  static float temptot = 0;
  float averageTemp = 0;
  if (sensorDelay.justFinished()) 
  {
    // read sensor and repeat
    sensorDelay.repeat(); // repeat
    temptot += mlx.readObjectTempC(); //add the reading to the total
    //  temptot +=  mlx.readObjectTempF();
    index++; //increment index
    if(index==19)
    {
      //if already 20 measurement
      averageTemp = temptot/20;     //calculate average
      temptot = 0;            //zero total
      index = 0;          //zero index
      
      sensorDelay.stop();     //stop reading
      displayDelay.finish();  //complete the refresh interval so that it shows immediately
      
      //return averageTemp;           //give results
    }
  }
  return averageTemp; //As long as there are no results, give a value of result = 0
}
void ShowTemp(float temperature)
{
  if (displayDelay.justFinished()) 
  {
    displayDelay.repeat(); // repeat
    //show temperature
    if(temperature == -1)
    {
      //if there are no object in front of sensor
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(35, 5);
      display.print("-----");
      display.setCursor(105, 20);
      display.print("");
      display.setTextSize(2);
      display.setCursor(35, 40);
      display.print("-----");
      display.setCursor(105, 46);
      display.print("");
      display.display();
    }else if(temperature == 0)
    {
      //if still reading temperature
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20, 25);
      display.println("WAIT ....");
      display.display();
    }else
    {
      //if there is a new result
      temperature += 0.5; // sensor temperature calibration
      
      display.clearDisplay();     
      
      if( temperature > 36.5)
      {
        display.setTextSize(3);
        display.setCursor(5, 20);
        display.print(temperature,1);      
        //display.print("F"); 
        display.print("C");       
      }
      else
      {
      display.setTextSize(3);
      display.setCursor(10, 20);
      display.print(temperature,1);      
      //display.print(" F");
      display.print("C ");
      }
      display.display();
      if (temperature > 38) 
      {
        //if the temperature is too high
        digitalWrite(RED_LED, HIGH);
        holdDelay.start(delay_hold_red);  //run delay for red LED or temp > 38
      }else
      {
        //if the temperature is normal
        digitalWrite(GREEN_LED, HIGH);
        holdDelay.start(delay_hold_green);  //run delay for green LED or temp < 38
      }
      digitalWrite(buzzer, HIGH);
      
      displayDelay.stop();          //stop refresh display
    }
  }
}
void  holdReading()
{
  if (holdDelay.justFinished()) {
    // if holddelay finish
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(buzzer, LOW);
    measurement = false; //allow new measurements
    
    displayDelay.start(interval_display); //restart the OLED display 
  }
}
