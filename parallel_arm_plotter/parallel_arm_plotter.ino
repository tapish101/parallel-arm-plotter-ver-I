
#include <Servo.h>

Servo penservo;
Servo xservo;
Servo yservo;


#define LINE_BUFFER_LENGTH 512


const int penZUp = 23;
const int penZDown = 40;


/* global variables   */
struct point { 
  float x; 
  float y; 
  float z; 
};

struct point actuatorPos;

// drawing settings  
float StepInc = 1;
int StepDelay = 0;
int LineDelay = 50;
int penDelay = 50;


// writing area
const float p = 2.36;
const float v = 57.295;

float Xmax =60;
float Xmin =-60;
float Ymax =45;
float Ymin =-45;

boolean verbose = false;


void setup() {

  Serial.begin( 9600 );
  Serial.begin(9600);
  penservo.attach(6);
  xservo.attach(3);
  yservo.attach(9);

  Serial.print("X goes from "); 
  Serial.print(Xmin); 
  Serial.print(" to "); 
  Serial.print(Xmax); 
  Serial.println(" mm."); 
  Serial.print("Y goes from "); 
  Serial.print(Ymin); 
  Serial.print(" to "); 
  Serial.print(Ymax); 
  Serial.println(" mm."); 

  penUp();
  delay(500);
  xservo.write(90);
  yservo.write(90);
  delay(500);
  
}

void loop() 
{

  
  delay(200);
  char line[ LINE_BUFFER_LENGTH ];
  char c;
  int lineIndex;
  bool lineIsComment, lineSemiColon;

  lineIndex = 0;
  lineSemiColon = false;
  lineIsComment = false;

  while (1) {
    while ( Serial.available()>0 ) {
      c = Serial.read();
      if (( c == '\n') || (c == '\r') ) {       
        if ( lineIndex > 0 ) {                        
          line[ lineIndex ] = '\0';                   
          if (verbose) { 
            Serial.print( " Received: "); 
            Serial.println( line ); 
          }
          processIncomingLine( line, lineIndex );
          lineIndex = 0;
        } 
        else { 
          //Empty comment line, skip block.  
        }
        lineIsComment = false;
        lineSemiColon = false;
        Serial.println("ok");    
      } 
      else {
        if ( (lineIsComment) || (lineSemiColon) ) {   
          if ( c == ')' )  lineIsComment = false;     
        } 
        else {
          if ( c <= ' ' ) {                           
          } 
          else if ( c == '/' ) {                    
          } 
          else if ( c == '(' ) {                    
            lineIsComment = true;
          } 
          else if ( c == ';' ) {
            lineSemiColon = true;
          } 
          else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
            Serial.println( "ERROR - buffer overflow" );
            lineIsComment = false;
            lineSemiColon = false;
          } 
          else if ( c >= 'a' && c <= 'z' ) {        
            line[ lineIndex++ ] = c-'a'+'A';
          } 
          else {
            line[ lineIndex++ ] = c;
          }
        }
      }
    }
  }
}

void processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  char buffer[ 64 ];                               
  struct point newPos;

  newPos.x = 0.0;
  newPos.y = 0.0;
  
  while( currentIndex < charNB ) {
    switch ( line[ currentIndex++ ] ) {             
    case 'U':
      penUp(); 
      break;
    case 'D':
      penDown(); 
      break;
    case 'G':
      buffer[0] = line[ currentIndex++ ];         
      buffer[1] = '\0';

      switch ( atoi( buffer ) ){                   // working with g command
      case 0:                                   //no need to diff b/w move or move faster 
      break;
      case 1:
        // imp:: Here we have assume that the gcode string received have X vaiue before Y
        char* indexX = strchr( line+currentIndex, 'X' ); 
        char* indexY = strchr( line+currentIndex, 'Y' );
        if ( indexY <= 0 ) {
          newPos.x = atof( indexX + 1); 
          newPos.y = actuatorPos.y;
        } 
        else if ( indexX <= 0 ) {
          newPos.y = atof( indexY + 1);
          newPos.x = actuatorPos.x;
        } 
        else {
          newPos.y = atof( indexY + 1);
          indexY = '\0';
          newPos.x = atof( indexX + 1);
        }
        drawLine(newPos.x, newPos.y );
   
        actuatorPos.x = newPos.x;
        actuatorPos.y = newPos.y;
        break;
      }
      break;
    case 'M':
      buffer[0] = line[ currentIndex++ ];        // /!\ M commands must not increase 3-digit
      buffer[1] = line[ currentIndex++ ];
      buffer[2] = line[ currentIndex++ ];
      buffer[3] = '\0';
      switch ( atoi( buffer ) ){
      case 300:
        {
          char* indexS = strchr( line+currentIndex, 'S' );
          float Spos = atof( indexS + 1);
          if (Spos == 30) { 
            penDown(); 
          }
          if (Spos == 50) { 
            penUp(); 
          }
          break;
        }
      case 114:                                //report position 
        Serial.print( "Absolute position : X = " );
        Serial.print( actuatorPos.x );
        Serial.print( "  -  Y = " );
        Serial.println( actuatorPos.y );
        break;
      default:
        Serial.print( "Unrecognnized M command");
        Serial.println( buffer );
      }
    }
  }
}


void drawLine(float x1, float y1){
  
  if (verbose){
    Serial.print("fx1, fy1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }  

  // setting limits for each axis
  if (x1 >= Xmax) { 
    x1 = Xmax; 
  }
  if (x1 <= Xmin) { 
    x1 = Xmin; 
  }
  if (y1 >= Ymax) { 
    y1 = Ymax; 
  }
  if (y1 <= Ymin) { 
    y1 = Ymin; 
  }

  if(x1 < -12.7){
    float tx1= atan((abs(x1)-12.7)/(117-y1));
    float dx1= sqrt(sq(abs(x1)-12.7) + sq(117-y1));
    float cx1= acos(dx1/180);
    float anglex1= 135 - v*(tx1 + cx1);

    float ty1= atan((abs(x1)+12.7)/(117-y1));
    float dy1= sqrt(sq(abs(x1)+12.7) + sq(117-y1));
    float cy1= acos(dy1/180);
    float angley1= 45 + v*(cy1-ty1); 

    servowrite(anglex1,angley1);
 }

  else if(x1 >= -12.7 && x1<0){
    
    float tx1= atan((12.7-abs(x1))/(117-y1));
    float dx1= sqrt(sq(12.7-abs(x1)) + sq(117-y1));
    float cx1= acos(dx1/180);
    float anglex1= 135 - v*(cx1-tx1);

    float ty1= atan((abs(x1)+12.7)/(117-y1));
    float dy1= sqrt(sq(abs(x1)+12.7) + sq(117-y1));
    float cy1= acos(dy1/180);
    float angley1= 45 + v*(cy1-ty1); 

    servowrite(anglex1,angley1);
    
  }

  else if(x1 >=0 && x1 < 12.7){
    
    float tx1= atan((x1+12.7)/(117-y1));
    float dx1= sqrt(sq(12.7+x1) + sq(117-y1));
    float cx1= acos(dx1/180);
    float anglex1= 135 - v*(cx1-tx1);

    float ty1= atan((12.7-x1)/(117-y1));
    float dy1= sqrt(sq(12.7-x1) + sq(117-y1));
    float cy1= acos(dy1/180);
    float angley1= 45 + v*(cy1-ty1); 

    servowrite(anglex1,angley1);
    
  }

  else if( x1 >= 12.7){
    
    float tx1= atan((x1+12.7)/(117-y1));
    float dx1= sqrt(sq(x1+12.7) + sq(117-y1));
    float cx1= acos(dx1/180);
    float anglex1= 135 - v*(cx1-tx1);

    float ty1= atan((x1-12.7)/(117-y1));
    float dy1= sqrt(sq(x1-12.7) + sq(117-y1));
    float cy1= acos(dy1/180);
    float angley1= 45 + v*(cy1+ty1); 

    servowrite(anglex1,angley1);
    
  }
}


void servowrite(float a, float b){
  xservo.write(lroundf(a));
  yservo.write(lroundf(b));
}



void penUp(){ 
  if(penservo.read()!= penZUp){
    penservo.write(penZDown-10);
    for(int i=0; i<=7; i++){
      penservo.write(penZDown-10-i);
      delay(50);
    }
  }
}


void penDown() {
    if(penservo.read()!= penZDown){
    for(int j=0; j<=7; j++){
      penservo.write(penZUp+j);
      delay(50);
    }
    penservo.write(penZDown);
  }
}
  
