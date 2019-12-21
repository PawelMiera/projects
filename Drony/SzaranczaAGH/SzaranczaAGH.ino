#include <ESP8266WiFi.h>
//ppm
#define CPU_MHZ 80
#define CHANNEL_NUMBER 8  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1100  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 2 //set PPM signal output pin on the arduino
#define DEBUGPIN 4

volatile unsigned long next;
volatile unsigned int ppm_running=1;

unsigned long time_now = 0;

int ppm[CHANNEL_NUMBER];

const byte captive_portal=0;
const byte DNS_PORT = 53;
const char* serverIndex = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

extern const char index_html[];

unsigned int alivecount=0;

// server
const char* ssid     = "ssid";
const char* password = "pass";
String Throttle="";
String Pitch="";
String Roll="";
String Yaw="";
String Input1="";
String Input2="";
String header;
// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request


unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;



void inline ppmISR(void){
  static boolean state = true;

  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    next = next + (PULSE_LENGTH * CPU_MHZ);
    state = false;
    alivecount++;
  } 
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      next = next + ((FRAME_LENGTH - calc_rest) * CPU_MHZ);
      calc_rest = 0;
      digitalWrite(DEBUGPIN, !digitalRead(DEBUGPIN));
    }
    else{
      next = next + ((ppm[cur_chan_numb] - PULSE_LENGTH) * CPU_MHZ);
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
  timer0_write(next);
}





void setup() {
// ppm

  
 // Wifi
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
    pinMode(sigPin,OUTPUT);
  digitalWrite(sigPin, !onState); //set the PPM signal pin to the default state (off)
  pinMode(DEBUGPIN,OUTPUT);
  digitalWrite(DEBUGPIN, !onState); //set the PPM signal pin to the default state (off)
       ppm[0]=1100; ppm[1]=1100; ppm[2]=1100; ppm[3]=1100;
       ppm[4]=1100; ppm[5]=1100; ppm[6]=1100; ppm[7]=1100;

      yield();

  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(ppmISR);
  next=ESP.getCycleCount()+1000;
  timer0_write(next);
  for(int i=0; i<CHANNEL_NUMBER; i++){
    ppm[i]= CHANNEL_DEFAULT_VALUE;
  }
  interrupts();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) { // loop while the client's connected
             
      while (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if(c=='e'){
         bool startThr=false;
         bool startPit=false;
         bool startRol=false;
         bool startYaw=false;
         bool startInp1=false;   
         bool startInp2=false;      

          for(int i=0;i<header.length();i++)
          {
             if(header[i]=='@')
             {
                startThr=true;
                i++;
             } 
             else if(header[i]=='#')
             {
                startThr=false;
                startPit=true;
                i++;
             } 
             else if(header[i]=='$')
             {
                startPit=false;
                startRol=true;
                i++;
             } 
             else if(header[i]=='%')
             {
                startRol=false;
                startYaw=true;
                i++;
             } 
             else if(header[i]=='^')
             {
                startInp1=true;
                startYaw=false;
                i++;
             } 
             else if(header[i]=='&')
             {
                startInp1=false;
                startInp2=true;
                i++;
             }
              else if(header[i]=='e')
             {
                startInp2=false;
                header="";
                break;
             }  

              if(startThr){
                Throttle+=header[i];
              }
              else if(startPit){
                Pitch+=header[i];             
              }
               else if(startRol){
                Roll+=header[i];             
              }
               else if(startYaw){
                Yaw+=header[i];             
              }   
               else if(startInp1){
                Input1+=header[i];             
              }  
               else if(startInp2){
                Input2+=header[i];             
              }
                                                                                                     
          }
//          Serial.println();
//          Serial.println();
//          Serial.println(Throttle);
//          Serial.println(Pitch);
//          Serial.println(Roll);
//          Serial.println(Yaw);
//          Serial.println(Input1);
//          Serial.println(Input2);

         ppm[0]=Throttle.toInt();
         ppm[1]=Pitch.toInt();
         ppm[2]=Roll.toInt();
         ppm[3]=Yaw.toInt();
         ppm[4]=Input1.toInt(); 
         ppm[5]=Input2.toInt();     
          Throttle="";
          Pitch="";
          Roll="";
          Yaw="";
          Input1="";
          Input2="";                     
        }
        else if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Szarancza AGH</h1>");
            client.println("<style>html { font-family: Helvetica; display: inline-block; background-color: #696969; margin: 0px auto; text-align: center;}");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
        yield();
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
