#define  DELAY_START   LOW          // blink detection logic
#define  DELAY_END   !DELAY_START   // this is the inverse of the above
#include "ADS1X15.h"

ADS1115 ADS(0x48);

  //NOTE:  Below is the strings used to add to the data sent to the nextion display
  //       and to verify the end of the string for incoming data.
  //       Replace 3x Serial1.write(oxff);
  String        endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));
  unsigned long initDelayLong = 0;  //NOTE:4,294,967,295
  int           initDelayLength = 10000; //Cycle length in 1/1000 sec

  unsigned long initDelayHeight = 0;  //NOTE:4,294,967,295
  int           initDelayLengthHeight = 250; //Cycle length in 1/1000 sec

  String  dfd  = "";                // data from display

  //Declare global variable for inductive probe
  int inputPin = 2;               // choose the input pin for inductive probe 
  float start;                    //start timer for couting rotation rate
  float stop;                     //stop timer for counting rotation rate
  float duration;                 // total duration between the start and stop
  float dia = 29.7/2.54/12;       // diameter of the wheel the blade is on in cm converted to feet
  float sfm;                      //surface feet per min
  float realFeed;                 //actual feed rade
  
  //Height Measurment variables
  float height;
  float theta;
  float heightVal;

  //Declare Variables for linear pot
  bool cycle = true;
  int analogPin=A0;                     //What pin to put the analog input on
  float valOld=0;       //assume starting at zero
  float valNew=0;       //assume staring at zero to prevent error
  float thetaOld;       //prior blade angle in radians
  float thetaNew;       //future blade anglein radians
  float feedRate;

  //Machine constants for feed calc
  float pivOff=52/25.4;                 //distance from pivot to table in inches
  float py=14/25.4;                     //setback distance from pivto to hydrolic in inches
  float px=135/25.4;                    //horizontal distance from pivot to hydro pivot in inches
  float pHf=sqrt(pow(py,2)+pow(px,2));  //distance form pivto to hydro pivot in inches
  float hl=29.8/2.54;                   //distance from pivot to upper hydro attachement point
  float li=11.1417;                     //inches length of hydrolic when shut (at 90 deg)
  float b=13;                           //inches distance from pivot to fence
  float ang=atan(5.0548/10.5875);       // angle that forms the right triangle to the pivot
  float offset=0.069057;                //inches offset for pot calibration
  float slope=0.0038396;                //pots in/counts slope
  float posOld;                  //start the old and new position to zero
  float posNew;                  //start the old and new position to zero

  //Variables for screen
  float bladeSlope=0.0168;  //starting values for a 5/8 blade
  float bladeOffset=-.1413; //starting values for a 5/8 blade
  float feedRateIdeal=1.5;  //inches per minute
  float partArea=1.0;       //in in^2
  float partHeight=0.75;    //in inches
  float partWidth=0.75;     //width of part
  float pIncrease=100;      //How much to up the SFM

  int targetSFM=100;        //startign for a 4" 316 stainless piece
  int newParm=0;            //only run the operation once
  int testVal=0;            //start screen number
  int isNew=1;              //used to indicate prior blade position

  //setup variables
  float pi=3.14159265358;   //value of pi
  float zeroSaw;            //setup the saw at to zero its location on startup 

void setup() {
  pinMode(inputPin, INPUT);     // declare channel as input
  //Serial.begin(9600);
  Serial1.begin(9600);
  ADS.begin();
  ADS.setGain(0);
  zeroSaw=slope*analogRead(analogPin)+offset;  //this is in inches
}

// ________________________________________ Main Loop_________________________
void loop() {
  //NOTE : Collect Characters from the display
  if(Serial1.available()){
    lcdInput();
    delay(20); 
  }

  //Find feed rate based on given parameters
  //Serial.println(newParm);
  if(newParm==1){
    feedRateCalc();
    screenUpdate(); 
  }
  
  if(testVal==1){
      readSFM();
  }
  //NOTE: ASYNC Delay Code rate
  if(testVal==0 && millis() > initDelayHeight){
    initDelayHeight+=initDelayLengthHeight;
    readHeight();
  }

  //NOTE: ASYNC Delay Code rate
  if(testVal==0 && millis() > initDelayLong){
    initDelayLong+=initDelayLength;
    readFeed();
  }

  //NOTE : Something sent from the Nextion I.E get request
  //if(dfd.endsWith(endChar)){
  //  Serial.println(dfd);
  //  Serial.println("error");
  //  dfd = "";
  //}
}

// __________________________________________ SFM adjust_______________________________
void sfmAdjust(){
  //hold this to adjust the sfm based on the width of material
  //y is the percent increase partWidth is the width of the part
  pIncrease=.1361*pow(partWidth,2)-4.5406*partWidth+15.67;
  targetSFM=targetSFM*(1+.01*pIncrease);
  //Serial.println(String(targetSFM));
}

// _________________________________________ feed rate calc _______________________________
void feedRateCalc(){
  float rate=bladeSlope*targetSFM+bladeOffset; //in^2/min
  float time=1/rate*partArea; 
  feedRateIdeal=partHeight/time;
  //initDelayLength=int(.025*(60)/(.25*feedRateIdeal)*1000);
}

// ________________________________________  Handle LCD Inputs _____________________________
void lcdInput(){
  dfd += char(Serial1.read());  //read in and concatonate the data
  if(dfd.length()>3 && dfd.substring(0,3)!="N:P") dfd="";
  else{
  //NOTE : If the string ends with a ? then command is complete
    if(dfd.substring((dfd.length()-1),dfd.length()) == "?"){
    //Add in commands here
      if(dfd.substring(3,12)=="bladeType"){
        if(dfd.substring(12,13)=="1"){
          bladeSlope=0.051;
          bladeOffset=-3.224;
        }
        if(dfd.substring(12,13)=="2"){
          bladeSlope=0.0432;
          bladeOffset=0.2484;
        }
        if(dfd.substring(12,13)=="3"){
          bladeSlope=0.0353;
          bladeOffset=-.1965;
        }
        if(dfd.substring(12,13)=="4"){
          bladeSlope=0.0238;
          bladeOffset=0.2354;
        }
        if(dfd.substring(12,13)=="5"){
          bladeSlope=0.0168;
          bladeOffset=-.1413;
        }
        if(dfd.substring(12,13)=="6"){
          bladeSlope=0.0098;
          bladeOffset=0.1988;
        }
        newParm=1; 
      }
      if(dfd.substring(3,10)=="sfmTest"){
        testVal=1;
      }
      if(dfd.substring(3,9)=="tarSFM"){
        targetSFM=dfd.substring(9,dfd.length()-1).toInt();
        newParm=1;
      }
      if(dfd.substring(3,10)=="areaVal"){
        partArea=dfd.substring(10,dfd.length()-1).toFloat()/10000;
        newParm=1;
      }
      if(dfd.substring(3,12)=="heightVal"){
        partHeight=dfd.substring(12,dfd.length()-1).toFloat()/1000;
        newParm=1;
      }
      if(dfd.substring(3,11)=="widthVal"){
        partWidth=dfd.substring(11,dfd.length()-1).toFloat()/1000;
        sfmAdjust();
        newParm=1;
      }
    dfd="";
    }
  }
}

// ______________________________ Read the surafce feet of the blade (program pauses while running) _____________________
void readSFM(){
  Serial1.print("mon.sfm.bco=50712"+endChar);//on
  for (int i=0;i<2;i++){
    while(digitalRead(inputPin) != DELAY_START   )  //pauses program untill reading starts
    start = micros();
    while(digitalRead(inputPin) != DELAY_END   )  //pauses until the pin goes high
    duration = (micros()-start)/1000000;  //length of time this process takes
  }
  sfm = pi*dia*60/duration;
  Serial1.print("mon.sfm.val=" + String(sfm,0) + endChar);
  delay(20);
  Serial1.print("mon.sfm.bco=65535" + endChar);//off color
  delay(20);
  Serial1.print("mon.bt0.val=0" + endChar);//off button
  delay(20);
  //Serial.println(String(duration));
  //Serial.println(String(sfm));
  //adjust the target feed based on measured SFM
  float rate=bladeSlope*sfm+bladeOffset; //in^2/min
  float time=1/rate*partArea; 
  realFeed=partHeight/time;
  Serial1.print("mon.feed.val=" + String(realFeed*1000,0) + endChar);
  delay(20);
  testVal=0;
}
// __________________________  What is the current Height of the Blade ____________________________________
void readHeight(){
  int16_t val_0 = ADS.readADC(0);  
  float f = ADS.toVoltage(1)*val_0;  // voltage factor

  heightVal=slope*f+offset-zeroSaw; //in inches move it 8 counts for zero position
  theta=acos((pow(pHf,2)+pow(hl,2)-pow(li+heightVal,2))/(2*pHf*hl))-atan(py/px);
  height=b*tan(theta+ang-pi/2)+pivOff;

  Serial1.print("mon.pos.val=" +String(height*100-pivOff*100,0) + endChar);
  delay(20);

  if(height-pivOff<1.05*partHeight-pivOff && isNew==1)
  {
    Serial1.print("mon.pos.pco=56160");
    delay(20);
    isNew=0;
  }
  if(height-pivOff>=1.05*partHeight-pivOff && isNew==0)
  {
    Serial1.print("mon.pos.pco=0");
    delay(20);
    isNew=1;
  }
}

// __________________________ read the Feed rate of the bandsaw _____________________________________________
void readFeed(){

  if(cycle==true){
    start=millis();
    int16_t val_0 = ADS.readADC(0);  
    float f = ADS.toVoltage(1)*val_0;  // voltage factor
    valOld=slope*f+offset-zeroSaw; //in inches move it 8 counts for zero position  
  }
  else{
    stop=millis();
      int16_t val_0 = ADS.readADC(0);  
      float f = ADS.toVoltage(1)*val_0;  // voltage factor
    valNew=slope*f+offset-zeroSaw;   //move it 8 counts
  }
  thetaOld=acos((pow(pHf,2)+pow(hl,2)-pow(li+valOld,2))/(2*pHf*hl))-atan(py/px);
  thetaNew=acos((pow(pHf,2)+pow(hl,2)-pow(li+valNew,2))/(2*pHf*hl))-atan(py/px);

  posOld=b*tan(thetaOld+ang-pi/2)+pivOff;
  posNew=b*tan(thetaNew+ang-pi/2)+pivOff;

  feedRate=(posOld-posNew)/(stop/1000-start/1000)*60; //change units to in/min
  feedRate=fabs(feedRate);
  if(feedRate<.07){
    Serial1.print("mon.LF.val=0" + endChar); //needed to display a zero value
    //Serial.println("feed is 0");
    delay(40);
  }else{
    Serial1.print("mon.LF.val=" + String(feedRate*1000,0) + endChar);
    //Serial.println("feedrate : " + String(feedRate,3));
    delay(40);
  }
  
  cycle=!cycle;
  
  //Serial.println("countsOld : " +String((valOld-offset+0.0997738)/slope));
  //Serial.println("countsNew : " +String((valNew-offset+0.0997738)/slope));
  //Serial.println("ValOld : " + String(valOld,4));
  //Serial.println("ValNew : " + String(valNew,4));
  //Serial.println("posOld : " + String(posOld,4));
  //Serial.println("posNew : " + String(posNew,4));
  //Serial.println("feedrate : " + String(feedRate*1000,0));
  //Serial.println("Height : " + String(posNew*1000-pivOff,3));
  //Serial.println("------------------------------------------------------");
}

// ____________________________ Nextion Screen Update code ________________________________
void screenUpdate(){
  Serial1.print("mon.tarSFM.val=" + String(targetSFM)+endChar);
  delay(20);
  Serial1.print("mon.feed.val=" + String(feedRateIdeal*1000,0) + endChar);
  delay(20);
  //Serial.println("pageSFM.tarSFM.val=" + String(targetSFM));
  //Serial.println("pageFeed.feed.val=" + String(feedRateIdeal,4));
  newParm=0;
} 