unsigned char Prescaler = 0;
#define PrescalerOverflowValue 4
ISR(TIMER2_OVF_vect)
{
  if (Prescaler < PrescalerOverflowValue)
    Prescaler++;
  else {
    Prescaler = 0;
    Multiplex();
  }
}


unsigned char CurrentLED = 1;
unsigned int LEDLayers[4];

void Multiplex(void)
{     
  switch (CurrentLED)
  {
    case 0:
      digitalWrite(38, LOW);
      break;
    case 1:
      digitalWrite(39, LOW);
      break;
    case 2:
      digitalWrite(40, LOW);
      break;
    case 3:
      digitalWrite(41, LOW);               
      break;  
  }     
  
  CurrentLED++;
  if (CurrentLED > 3)
    CurrentLED = 0;  
    
  PORTA = (LEDLayers[CurrentLED] & 0xFF);
  PORTC = ((LEDLayers[CurrentLED] & 0xFF00) >> 8);  
  
  switch (CurrentLED)
  {
    case 0:
      digitalWrite(38, HIGH);
      break;
    case 1:
      digitalWrite(39, HIGH);
      break;
    case 2:
      digitalWrite(40, HIGH);
      break;
    case 3:
      digitalWrite(41, HIGH);                 
      break;  
  }    
}

int JumpingPixels[16][5]; // X, Y, Z, Interval, Time
char JumpingPixelsCount = 0;

void setup(void) { 
  //Set the pin we want the ISR to toggle for output.  
  pinMode(38,OUTPUT);   
  digitalWrite(38, LOW);  
  
  pinMode(39,OUTPUT);  
  digitalWrite(39, LOW);  
  
  pinMode(40,OUTPUT);    
  digitalWrite(40, LOW);  
  
  pinMode(41,OUTPUT);  
  digitalWrite(41, LOW);  
  
  DDRA = 0xFF; // Port A as Output
  PORTA = 0x00;  
  DDRC = 0xFF; // Port C as Output
  PORTC = 0x00;
  
  //Start up the serial port
  Serial.begin(19200);
  
  //Signal the program start
  Serial.println("LED Cube Controller");
  
  // Enable Timer 2 interrupt (also used for PWM though)
  // This interrupt is divided by a prescaler, and takes care of the multiplexing
  TCCR2B = TCCR2B & 0b11111000 | 0x02; // Divisor = 8
  TIMSK2 = 1<<TOIE2;
  
  ClearCube();   
}

int globalI;
void loop(void)
{
  ClearCube();   
  Effect_JumpPixelInit(500, 3000);
  for (globalI = 0; globalI < 300; globalI++)
    Effect_JumpPixel();
  
  for (globalI = 0; globalI < 50; globalI++) {
    Effect_TurningX();
    delay(100);
  }
  for (globalI = 0; globalI < 50; globalI++) {
    Effect_TurningY();
    delay(100);
  }
  for (globalI = 0; globalI < 50; globalI++) {
    Effect_TurningZ();
    delay(100);
  }  
}

unsigned int SetLED(char x, char y, char z, char set_clear)
{
  char bitPosition;
  unsigned int temp;
     
  if ((y % 2) == 0)
    bitPosition = 15 - (((3-y) * 4) + (3-x));     
  else
    bitPosition = 15 - (((3-y+1) * 4) - (3-x+1));

  if (set_clear == 0)
    LEDLayers[z] &= ~(1 << bitPosition);
  else
    LEDLayers[z] |= (1 << bitPosition);
}

void ClearCube(void)
{
  LEDLayers[0] = 0x0000;
  LEDLayers[1] = 0x0000;
  LEDLayers[2] = 0x0000;
  LEDLayers[3] = 0x0000;  
}

void Effect_JumpPixelInit(int timeRange_min, int timeRange_max) {
  char x, y, i;
    
  i = 0;  
  for (y = 0; y < 4; y++)   {   
    for (x = 0; x < 4; x++) {     
      JumpingPixels[i][0] = x;
      JumpingPixels[i][1] = y;
      JumpingPixels[i][2] = 0;
      randomSeed(analogRead(0)*analogRead(1));             
      JumpingPixels[i][3] = random(timeRange_min, timeRange_max);
      JumpingPixels[i][4] = 0;  
      i++;
    }
  }   

  JumpingPixelsCount = i;
}

void Effect_JumpPixel(void) {
  char i;
  
  for (i = 0; i < JumpingPixelsCount; i++)
  {
    if (JumpingPixels[i][4] <= 50) {
      JumpPixel(i);
      JumpingPixels[i][4] = JumpingPixels[i][3];
    } else {
      JumpingPixels[i][4] -= 50;
    }
  }
  delay(50);
}  
  
void JumpPixel(char PixelID) {
  signed char dir;
  char i;
  
  if (JumpingPixels[PixelID][2] == 0) dir = 1; // increase
  else dir = -1; // decrease
  
  for (i = 0; i < 3; i++)
  {
    SetLED(JumpingPixels[PixelID][0], JumpingPixels[PixelID][1], JumpingPixels[PixelID][2], 0);
    JumpingPixels[PixelID][2] += dir;
    SetLED(JumpingPixels[PixelID][0], JumpingPixels[PixelID][1], JumpingPixels[PixelID][2], 1);
    delay(25);
  }
}


const char TurningSteps[6][4][2] = {  { {0,0},{1,1},{2,2},{3,3} },  { {1,0},{1,1},{2,2},{2,3} },  { {2,0},{2,1},{1,2},{1,3} },  { {3,0},{2,1},{1,2},{0,3} },  { {3,1},{2,1},{1,2},{0,2} },  { {3,2},{2,2},{1,1},{0,1} }  };
char EffectTurningStep = 0;
void Effect_TurningZ(void)
{
  char i;
  
  ClearCube();  
  for (i = 0; i < 4; i++)
  {
    SetLED(TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 0, 1);
    SetLED(TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 1, 1);
    SetLED(TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 2, 1);
    SetLED(TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 3, 1);    
  }
  
  if (EffectTurningStep < 5) EffectTurningStep++;
  else EffectTurningStep = 0;    
}

void Effect_TurningX(void)
{
  char i;
  
  ClearCube();  
  for (i = 0; i < 4; i++)
  {
    SetLED(0, TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 1);
    SetLED(1, TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 1);
    SetLED(2, TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 1);
    SetLED(3, TurningSteps[EffectTurningStep][i][0], TurningSteps[EffectTurningStep][i][1], 1);    
  }
  
  if (EffectTurningStep < 5) EffectTurningStep++;
  else EffectTurningStep = 0;    
}

void Effect_TurningY(void)
{
  char i;
  
  ClearCube();  
  for (i = 0; i < 4; i++)
  {
    SetLED(TurningSteps[EffectTurningStep][i][0], 0, TurningSteps[EffectTurningStep][i][1], 1);
    SetLED(TurningSteps[EffectTurningStep][i][0], 1, TurningSteps[EffectTurningStep][i][1], 1);
    SetLED(TurningSteps[EffectTurningStep][i][0], 2, TurningSteps[EffectTurningStep][i][1], 1);
    SetLED(TurningSteps[EffectTurningStep][i][0], 3, TurningSteps[EffectTurningStep][i][1], 1);    
  }
  
  if (EffectTurningStep < 5) EffectTurningStep++;
  else EffectTurningStep = 0;  
}


