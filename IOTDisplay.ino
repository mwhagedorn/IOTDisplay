 


#include <string.h>
#include <MatrixDisplay.h>
#include "DisplayToolbox.h"
#include "font.h"
#include <SPI.h>
#include <WiFly.h>
#include "Credentials.h"
#include <Time.h>
#include <avr/pgmspace.h>
#include <Stream.h>
#include <Queue.h>
#include "ArduinoMiniJSON.h"




//LED Matrix Set

const byte numLines = 4;        // Max number of lines
const int displayTime = 2500;  // Length of time in milliseconds to display each line
const byte gapTime = 20;        // Time gap between lines
const byte maxLineLength = 100; // Maximum length of a line in characters

char textLines[numLines][maxLineLength];     // Array of text lines

int textLength;          // Holds current line length in pixels
int strLength;           // Holds current line length in characters
byte currentLine = 0;     // Index of current line


#define numDefaults 3 //array size  

// Character lookup string -- used to map the characters from the keyboard onto the font data array
// (This should probably be moved to program memory in some form in the future)

const char charLookup[] PROGMEM = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*(),-.?></\\|[]_=+:'\"{}";


// Memory test
extern int __bss_end;
extern int *__brkval;

// Macro to make it the initDisplay function a little easier to understand
#define setMaster(dispNum, CSPin) initDisplay(dispNum,CSPin,true)
#define setSlave(dispNum, CSPin) initDisplay(dispNum,CSPin,false)

// Initialize matrix
MatrixDisplay disp(2,7,8, true);
DisplayToolbox toolbox(&disp);

// Prepare boundaries
uint8_t X_MAX = 0;
uint8_t Y_MAX = 0;

#define is_printable(c) (!(c&0x80))  

//WiFly Setup
//WiFlyServer server(3030);

 prog_char rssiMsg[] PROGMEM = "Checking RSSI...";
 prog_char iotTasksMsg[] PROGMEM = "IOT Tasks...";
 prog_char iotNewsMsg[] PROGMEM = "IOT News...";
 prog_char networkErr[] PROGMEM = "  Network Err  ";
 prog_char memoryErr[] PROGMEM = "   MEMORY LOW!  ";
 prog_char msg1[] PROGMEM = " Unable To Associate ";
 prog_char msg2[] PROGMEM = " Howdy ";
 prog_char msg3[] PROGMEM = " Welcome to LSRC 2012 ";


 PROGMEM const char *stringTable[] = 
 {
	rssiMsg,
	iotTasksMsg,
	iotNewsMsg,
	networkErr,
	memoryErr,
	msg1,
	msg2,
	msg3

};


// ThingSpeak Settings
const char *thingSpeakAddress = "thingspeak.appsdynamic.com";


//IOT Assistant setup
const char *serverName = "iot.appsdynamic.com";

char buffer[30];

const unsigned long              // Time limits, expressed in milliseconds:
  connectTimeout  = 15L * 1000L, // Max time to retry server link
  responseTimeout = 30L * 1000L; // Max time to wait for data from server
  
 
 long
  // Time since last print
  grabTime(void),
  strToLong(String s),
  delayLeft = 0;
 

 	WiFlyClient iotClient(serverName,80); 
  WiFlyClient thingSpeakClient(thingSpeakAddress,80);
 ArduinoMiniJSON jsonParser; 
 Queue myQueue;
 boolean
  readString(char *, int),
  sendStringToDisplay(char *),
	isParentKey(const char* key);
 
 void
  parse(void),
  setupDefaultText(void),
  setupDisplayBounds(void),
  setupDisplayOutputPins(void),
  setupWiFlyConnection(void),
  setFirstLineOfBufferToIpAddress(void),
  digitalClockDisplay(void),
  nextLine(void),
  prevLine(void),
  scrollText(char*),
  fixedText(char*),
  drawString(int, uint8_t, char*),
  drawChar(int, int, char),
  Flush_RX(void),
  digitalClockDisplay(void),
  printDigits(int),
  lowMemoryAlert(),
  sendToDisplay(char* msg),
  checkFreeMemory(),
  processInput(char *buffer,WiFlyClient &client),
  sendCurrentLineToDisplay(void),
	checkForInboundSocketInformation(void),
  checkIOTPortal(const char*),
	parse(const char*),
	parseSPITime(char* resultString),
	updateThingSpeak(const char* data),
	parseSPIRSSI(char *);
 
byte
	indexOf(const char* str, char c),
	clockIndex = 1;

 int
  timedRead(void),  
  getFreeMemory(void),
	updateDisplayHandler(unsigned long),
	hideIPAddressHandler(unsigned long),
	iotNewsHandler(unsigned long),
	iotTasksHandler(unsigned long),
	showMemHandler(unsigned long),
  showRSSIHandler(unsigned long),
	thingSpeakHandler(unsigned long now),
  grabRSSI(void);

unsigned long startTime, t;

 char* 
	sendSPICommand(int);




void setup() {

   Serial.begin(9600);
   Serial.println(F("what up g!"));
   setupDefaultText();
   setupDisplayBounds();
   setupDisplayOutputPins();
   setupWiFlyConnection();
   setFirstLineOfBufferToIpAddress();
   setTime(grabTime()); // Set the system clock to match the WiFly RTC
   adjustTime(-5*3600L); // Adjust the time for Eastern Time Zone (GMT-4 as of 3/11)
   digitalClockDisplay(); // Display the system clock time to the Serial monitor 

   
	 myQueue.scheduleFunction(updateDisplayHandler, "Display", 500, 6000);
	 myQueue.scheduleFunction(showMemHandler, "MEM", 5000, 120000);
	 myQueue.scheduleFunction(hideIPAddressHandler, "Time", 10000, 0);
	 myQueue.scheduleFunction(showRSSIHandler, "RSSI", 11000, 340000); 
	 myQueue.scheduleFunction(iotTasksHandler, "IOTT", 17000, 70000);
	 myQueue.scheduleFunction(iotNewsHandler, "IOTN", 37000, 150000);
	 myQueue.scheduleFunction(thingSpeakHandler, "TS", 2000, 60000);
	
	 
	 
	 while(1) {
	        myQueue.Run(millis());	
	        delay(10);
	 }
} 

void loop(){



}

int iotTasksHandler(unsigned long now)
{
	 myQueue.scheduleChangeFunction("IOTN",(now+600000), 60000);
	 strcpy_P(buffer, (PGM_P)pgm_read_word(&(stringTable[1])));
	 sendStringToDisplay(buffer);
	 memset(buffer,0,sizeof(buffer));
	 Serial.println(F("Tasks Handler"));
	 checkIOTPortal("tasks");
	 //Serial.println(millis()-now);
	 myQueue.scheduleChangeFunction("IOTN",(now+60000), 150000);
}

int iotNewsHandler(unsigned long now)
{
	 strcpy_P(buffer, (PGM_P)pgm_read_word(&(stringTable[2])));
	 sendStringToDisplay(buffer);
	 memset(buffer,0,sizeof(buffer));
	 Serial.println(F("News Handler"));
	 checkIOTPortal("news");
	// Serial.println(millis()-now);
}

int updateDisplayHandler(unsigned long now)
{
    Serial.println(F("Upd Display"));
		sendCurrentLineToDisplay();
		nextLine();
		digitalClockDisplay();
	  checkFreeMemory();
	  // for(int i=0;i<3;i++){
	  // 			Serial.println(textLines[i]);
	  // 		}
}

int hideIPAddressHandler(unsigned long now){
	Serial.println(F("Time Only Display"));
	strlcpy(textLines[0], textLines[1], maxLineLength);
	clockIndex = 0;
	sendCurrentLineToDisplay();
  nextLine();
 
}

int showRSSIHandler(unsigned long now){
	Serial.println(F("RSSI Handler"));
	strcpy_P(buffer, (char*)pgm_read_word(&(stringTable[0])));
	sendStringToDisplay(buffer);
	memset(buffer,0,sizeof(buffer));
	int dbm = grabRSSI();
	sprintf(buffer,"%d dbm",dbm);
  sendStringToDisplay(buffer);
	memset(buffer,0,sizeof(buffer));
}

int showMemHandler(unsigned long now){
	 sprintf(buffer, " %03d Bytes", getFreeMemory()); 
	 sendStringToDisplay(buffer);
	 memset(buffer,0,sizeof(buffer));
}

int thingSpeakHandler(unsigned long now){
		int dbm = grabRSSI();
		long mem = getFreeMemory();		
		sprintf(buffer, "field1=%d&field2=%d",dbm, mem);
		updateThingSpeak(buffer);
		memset(buffer,0,sizeof(buffer));
		
}

void checkFreeMemory(){
  if(getFreeMemory() < 80){
   lowMemoryAlert();
  }
}

 //updateThingSpeak("field1="+analogPin0);
 void updateThingSpeak(const char* data){
    Serial.println(F("Connecting to Thingspeak..."));
    while((thingSpeakClient.connect() == false) && ((millis() - startTime) < connectTimeout));
 

    	if(thingSpeakClient.connected()) { // Success!
 			thingSpeakClient.print(F("POST /update HTTP/1.1\n"));
 		  thingSpeakClient.print(F("Host: "));
			thingSpeakClient.println(thingSpeakAddress);
 		  thingSpeakClient.print(F("Connection: close\n"));
 			thingSpeakClient.print("X-THINGSPEAKAPIKEY: ");
 			thingSpeakClient.println(writeAPIKey);
 	    thingSpeakClient.print(F("Content-Type: application/x-www-form-urlencoded\n"));
 	    thingSpeakClient.print(F("Content-Length: "));
 	    thingSpeakClient.print(strlen(data));
 	    thingSpeakClient.print(F("\n\n"));
 
 	    thingSpeakClient.print(data);
 
 	  }
 }
 
void checkIOTPortal(const char* itemName){
   Serial.println(F("Connecting to IOT..."));
   while((iotClient.connect() == false) && ((millis() - startTime) < connectTimeout));

   	if(iotClient.connected()) { // Success!
	     Serial.print(F("OK\r\nIssuing HTTP request..."));
	     // URL-encode queryString to client stream:
	     iotClient.print(F("GET "));
		 	 iotClient.print(url);
			 iotClient.print(F(" HTTP/1.1\r\nHost: "));
	     iotClient.println(serverName);
	     iotClient.println(F("User-Agent:IOTDisplay"));
	     iotClient.println(F("Connection: close\r\n"));
	     Serial.print(F("OK\r\nAwaiting results (if any)..."));
	
	     t = millis();
	     while((!iotClient.available()) && ((millis() - t) < responseTimeout));
	
	     if(iotClient.find("\r\n\r\n")) { // Skip HTTP response header
		        Serial.println(F("OK\r\nProcessing results..."));
						parse(itemName);
				};
	   } else { // Couldn't contact server
	     Serial.println(F("failed"));
	   }
}

boolean isParentKey(const char* key){
	if((strcmp(key,"tasks") == 0) || 
	    (strcmp(key,"news") == 0)){
		return true;
	}
	return false;
}

void parse(const char* parent){
	jsonParser.clearState();
	char *key;
	boolean inObject = false;	
	while (iotClient.connected()) {
		if (iotClient.available()) {
			char c = iotClient.read();
  		jsonParser.handleInput(c);
      if( jsonParser.getParseState() == JSON_PARSE_HAVEKEY ) {
 				key = jsonParser.getKey();
   			if(strcmp(key,parent) == 0 ){
 					//Serial.println("parent");
 					inObject = true;
 				}else{
 					if(isParentKey(key) == true && inObject == true ){
						//if the key is another parent key
						//and you are already inObject
						//then you are now outOf the object
						//Serial.println("next");
						inObject = false;
					}	
 				}			
	    }//havekey
	
		  if( jsonParser.getParseState() == JSON_PARSE_HAVEVAL && inObject == true ) {
		    key = jsonParser.getKey();
		    if( strcmp("name", key) == 0 ) {
					if(currentLine == 0){
						//dont load the zeroth
						nextLine();
				 	}
		     	strlcpy(textLines[currentLine], jsonParser.getVal() , maxLineLength);  
	     	 	nextLine();
				}   	   
		    Serial.print( key );
		    Serial.print( "\t = \"" );
  		  Serial.print( jsonParser.getVal() );
  		   Serial.println( "\"" ); 
			}
		}  
	}
}



void checkForInboundSocketInformation(){
	// //check for new tcp socket information
	//   WiFlyClient client = server.available(); 
	//  
	//   if (client) {
	// 	byte bufferLength=0;
	// 	while (client.connected()) {
	// 		if (client.available()) {
	// 			char c = client.read();
	// 			Serial.print(c);
	// 			if(bufferLength <  100 && c != '\n'){
	// 		  	commandBuffer[bufferLength++]=c;
	// 			}else{
	// 		  	Serial.println(F("buffer"));
	// 		  	c='\n';
	// 			}
	// 			//store the recieved chracters in a string
	// 			//if the character is an "end of line" the whole message is recieved
	// 			if (c == '\n') {
	// 			  commandBuffer[bufferLength]='\0';
	// 			  Serial.print(F("client:"));//print it to the serial
	// 			  Serial.println(commandBuffer);
	// 			  client.println(F("AOK"));
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	// give the Client time to receive the data
	// 	delay(1);
	// 	// close the connection:
	// 	Serial.println(F("close"));
	// 	client.stop();
	// 	
	// 	memset(textLines[currentLine],0,maxLineLength-1);
	// 	if(textLines[currentLine][0] == '\v'){
	// 		Serial.println(F("next"));
	// 		nextLine();
	// 	}else{
	// 			strlcpy(textLines[currentLine],commandBuffer,sizeof(textLines[currentLine]) );
	// 	}
	// 	memset(commandBuffer   , 0, sizeof(commandBuffer-1));
	// }else{
	// 		Serial.println(F("no client"));
	// }
}

void sendCurrentLineToDisplay(void){
	for(int i=0;i<3;i++){
		char* msg = textLines[currentLine];   
  	if(sendStringToDisplay(msg) == true){
			break;
	   }else{
			nextLine();
		}
	}//retries
}

boolean sendStringToDisplay(char* newMessage){
	  //Serial.println("sendStringToDisplay");
  	strLength = strlen(newMessage); // Number of characters in the string
  	textLength = (strLength*6); 
		if(strLength > 1){
    	if(textLength < (X_MAX +1)){
       fixedText(newMessage);
       delay(displayTime);
       disp.clear(); 
       disp.syncDisplays();
       delay(gapTime); 
     	}else{
       scrollText(newMessage); 
       delay(gapTime);
     	}
			return true;
	   }else{
			return false;
		}
}

void setupDisplayBounds(){
   // Fetch display bounds 
  X_MAX = disp.getDisplayCount() * (disp.getDisplayWidth()-1)+1;
  Y_MAX = disp.getDisplayHeight()-1;
}

void setupDisplayOutputPins(){
    disp.setMaster(0,4);
    disp.setSlave(1,5);
}

void setupDefaultText(){
  for(byte i=5; i<numDefaults+5; i++){
		strcpy_P(buffer, (char*)pgm_read_word(&(stringTable[i])));
    strlcpy(textLines[i],buffer, maxLineLength);
		memset(buffer,0,sizeof(buffer));
  }
}

void setupWiFlyConnection(){
  WiFly.begin();
  Serial.println(F("\n\r\n\rWiFly Message Board"));
  //if(!WiFly.join(ssid,passphrase)){
  if(!WiFly.join(ssid)){
     Serial.println(F("fail"));
     //network failure message
     strcpy_P(buffer, (PGM_P)pgm_read_word(&(stringTable[3])));
		 sendStringToDisplay(buffer);
		 memset(buffer,0,sizeof(buffer));
     while(1){
       //hang on fail
     }
   }
  
  //server.begin(); 
  Serial.print(F("IP:"));
  Serial.println(WiFly.ip());

}

void setFirstLineOfBufferToIpAddress(){
  strlcpy(textLines[0], WiFly.ip(), maxLineLength);
}


void nextLine(){
   if(currentLine < (numLines-1)){
      currentLine++; 
   }else{
     currentLine = 0;
   }    
}

void prevLine(){
  if(currentLine > 0){
     currentLine--; 
  } else {
     currentLine = (numLines -1);
  }
}

void scrollText(char* text){
  byte y=1;
  int endPos = 0 - textLength;
  int loopCount = 0;
  for(int Xpos = X_MAX; Xpos > endPos; Xpos--){
    disp.clear();
    drawString(Xpos,y,text); 
    disp.syncDisplays(); 
  }
}



// Write a line of static (non-scrolling) text to the disp
void fixedText(char* text){
	//Serial.println("fixedText");
  byte y = 1;
  byte x = 0;
  disp.clear();
  drawString(x,y,text);
  disp.syncDisplays(); 
}


// Output a string to the display
void drawString(int x, uint8_t y, char* c){
  for(char i=0; i< strLength; i++){
    if(is_printable(c[i])){
      drawChar(x, y, c[i]);
    }
    x+=6; // Width of each glyph
  }
}


// Output a single character to the display
void drawChar(int x, int y, char c){
	byte dots;

	c = indexOf(charLookup,c);
	if(c < 0){
		Serial.println(F("no map"));
		return;
	}

	for (char col=0; col< 5; col++) {
		if((x+col+1)>0 && x < X_MAX){ // dont write to the display buffer if the location is out of range
			dots = pgm_read_byte_near(&myfont[c][col]);
			for (char row=0; row < 7; row++) {
			if (dots & (64>>row))   	     // only 7 rows.
				toolbox.setPixel(x+col, y+row, 1);
			else 
				toolbox.setPixel(x+col, y+row, 0);
			}
		}
	}
}

byte indexOf(const char *str, char c)
{
    const prog_char* ptr;
    byte index;

    ptr = strchr_P(str, c);
    if (ptr == NULL)
    {
        return -1;
    }

    index = ptr - str;
    return index;
} 


// Warn if we're running out of RAM
void lowMemoryAlert(){
  for(byte i=0; i < 3; i++){
    byte y = 1;
    byte x = 0;
    disp.clear();
		//memory low
    strcpy_P(buffer, (char*)pgm_read_word(&(stringTable[4])));
		Serial.println(buffer);
		sendStringToDisplay(buffer);
		memset(buffer,0,sizeof(buffer));
    disp.syncDisplays();
    delay(500);
    disp.clear();
    disp.syncDisplays();
    delay(500);
    disp.clear();
  }
}

// Memory check
int getFreeMemory(){
  int free_memory;
  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);
    Serial.println(free_memory);
  return free_memory;
}

int grabRSSI(void)
{
	SpiSerial.begin(); 
	delay(250); // The delay buffer before and after were pulled from the WiFly library itself
	SpiSerial.print(F("$$$"));
	delay(250); // The delay buffer before and after were pulled from the WiFly library itself
	SpiSerial.println("\n"); // The WiFly library also recommended two carraige returns to clear things up
	Flush_RX(); // I added this last bit to clear out the receive buffer in preparation for pulling the time
	// Without this Flush_RX() here, potentially the first couple results returned would not be accurate
	SpiSerial.println(F("show rssi")); 
	t = millis();
	while((!SpiSerial.available()) && ((millis() - t) < responseTimeout));
	byte linesSoFar=0; // Variable to track the line number in the output we receive
	byte afterEquals=0; // Flag to trigger after equals sign -- when the RTC value begins
  byte openParen=0;
	byte closeParen=0;

	byte charNumber=0; // Counter to store the position number inside the UNIX timestamp char above

	 // This for loop runs with a total delay of 100ms to ensure that the results of the command have been received
	 // before allowing the program to loop and accept more input. Without this forced acceptance of incoming data,
	 // unexpected results can be seen.
	 for(byte z=0;z<=100;z++) 
	   {     
	    // Loop while there is incoming data from the WiFly module(in response to our show t t command above)
	    while(SpiSerial.available() > 0) 
	      {
	       char c = SpiSerial.read(); // Read in one character
				 Serial.print(c);
	       if(c=='\n') linesSoFar++; // If the character read in is a line feed, then add one to the counter for number of lines
				 if(c==')') break;
	       // Potential lines might look like: RSSI=(-xx) dbm
 	       if(linesSoFar==1) // The first line in the results contains the RSSI value
	         {
	          if(afterEquals==1) // Only perform the code below if our character position on the RTC line is after the =
	          { 
		          if(openParen == 1){
								buffer[charNumber] = (char)c; charNumber++; 
							}

						} // Store the character into our character array
	          if(c=='=') afterEquals=1; // If the current character is an equals, the next character begins our RTC value -- throw flag
	          if(c=='(') openParen=1;
	         }
	      }
	    delay(1); // Delay for 1ms
	   }
	 buffer[charNumber]= '\0';
	 Serial.print(F("RSSI:"));
   int dbm= atoi(buffer);
	 return dbm;
	  	
}

long grabTime(void)
{
	 SpiSerial.begin(); 
	 delay(250); // The delay buffer before and after were pulled from the WiFly library itself
	 SpiSerial.print(F("$$$"));
	 delay(250); // The delay buffer before and after were pulled from the WiFly library itself
	 SpiSerial.println("\n"); // The WiFly library also recommended two carraige returns to clear things up
	 Flush_RX(); // I added this last bit to clear out the receive buffer in preparation for pulling the time
	 // Without this Flush_RX() here, potentially the first couple results returned would not be accurate
	 SpiSerial.println(F("show t t"));
	 t = millis();
   while((!SpiSerial.available()) && ((millis() - t) < responseTimeout));
	 byte linesSoFar=0; // Variable to track the line number in the output we receive
	 byte afterEquals=0; // Flag to trigger after equals sign -- when the RTC value begins

	 byte charNumber=0; // Counter to store the position number inside the UNIX timestamp char above

	 // This for loop runs with a total delay of 100ms to ensure that the results of the command have been received
	 // before allowing the program to loop and accept more input. Without this forced acceptance of incoming data,
	 // unexpected results can be seen.
	for(byte z=0;z<=100;z++) 
   {     
    // Loop while there is incoming data from the WiFly module(in response to our show t t command above)
    while(SpiSerial.available() > 0) 
      {
       char c = SpiSerial.read(); // Read in one character
       if(c=='\n') linesSoFar++; // If the character read in is a line feed, then add one to the counter for number of lines
       // Potential lines might look like: RTC=1331449728
       if(linesSoFar==3) // The third line in the results contains the RTC value
         {
          if(afterEquals==1) // Only perform the code below if our character position on the RTC line is after the =
            { buffer[charNumber] = (char)c; charNumber++; } // Store the character into our character array
          if(c=='=') afterEquals=1; // If the current character is an equals, the next character begins our RTC value -- throw flag
         }
      }
    delay(1); // Delay for 1ms
   }
   buffer[charNumber]= '\0';
   //Serial.println(resultString);
   long returnL = atol(buffer); // Convert our character aray into a long
   return(returnL);
}


// I found this command to be needed so that the receive buffers are flushed
// This helps normalize output so that the time grabbing works consistently
void Flush_RX(void)
{
 for(byte z=0;z<=100;z++) 
   {     
    // Loop while there is incoming data from the WiFly module(in response to our show t t command above)
    while(SpiSerial.available() > 0) 
      	SpiSerial.read(); // Read in one character
    delay(1);
   }
 return;
}



// Part of the standard Time library examples
void digitalClockDisplay(void)
{
	memset(textLines[clockIndex],0,9); 
  sprintf(textLines[clockIndex], " %02d:%02d:%02d", hourFormat12(), minute(), second()); 
    // note the time that the connection was made or attempted:
}

// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------

// Read from client stream with a 5 second timeout.  Although an
// essentially identical method already exists in the Stream() class,
// it's declared private there...so this is a local copy.
int timedRead(void) {
  unsigned long start = millis();
  while((!iotClient.available()) && ((millis() - start) < 5000L));
  return iotClient.read();
}











