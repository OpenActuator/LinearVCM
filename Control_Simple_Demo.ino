
const int HALL_SENSOR_PIN = A5;

// UNO PWM PIN : 3, 5, 6, 9, 10, 11
const byte PWM_PIN = 9;

#define INDEX_COUNT 12

float afTime[INDEX_COUNT] = {0.0f, 1.0f, 3.0f,  3.5f, 4.0f, 4.5f, 5.0f,  5.1f, 5.2f, 5.3f, 5.4f, 5.5f};
float afStroke[INDEX_COUNT] = {0.0f, -4.5f, 4.5f,  -4.5f, 4.5f, -4.5f, 4.5f,  -4.5f, 4.5f, -4.5f, 4.5f, 0.0f};
int aiStepCount[INDEX_COUNT] = {0, 500, 1000,  200, 200, 200, 200,  20, 20, 20, 20, 10};

double dDuty; 

double getCurrentStroke()
{
  int iHallSensor;
  
  // Hall Sensor : 348 ~ 681, Stroke : -5 ~ 6 mm
  iHallSensor = analogRead(HALL_SENSOR_PIN);

  // 0.42 는 전기가 인가되지 않은 상태에서 읽어지는 위치를 보고 보정한 값이다.
  return 0.0318*iHallSensor - 15.928 - 0.42;  
}

double getDuty(double dStroke)
{
  // Stroke : -4.5 ~ 4.5 mm, Duty : 0 ~ 100
  return 11.111 * dStroke + 50;
}

double getDeltaDuty(double dStroke)
{
  // Stroke : 0 ~ 9 mm, Duty : 0 ~ 100
  return 11.111 * dStroke;
}

int getPWM(double dDuty)
{
  // Duty : 0 ~ 100 %, PWM : 0 ~ 255
  return dDuty * (255.0 / 100.0);  
}

void setup()
{
  Serial.begin(9600);

  pinMode(PWM_PIN, OUTPUT);                  // sets the digital pin as output

  dDuty = 50;
  analogWrite(PWM_PIN, getPWM(dDuty));
}

void actionLoop(int index)
{
  unsigned long lgTime;
  
  //time = millis();
  //time = micros();

  double dTargetStroke, dCurrentStroke;  
  double dErrorStroke, dErrorDuty;   
   
  float fStartTime = afTime[index];
  float fEndTime = afTime[index+1];
  float fStartStroke = afStroke[index];
  float fEndStroke = afStroke[index+1];
  int iStepCount = aiStepCount[index+1];

  float fStepTime = (fEndTime - fStartTime) / iStepCount;
  float fStepStroke = (fEndStroke - fStartStroke) / iStepCount;

  int iCount = 0;
  float fTime = 0;
  lgTime = 0;  
  
  do
  {
    delayMicroseconds(1000);
    lgTime ++;
    fTime = lgTime / 1000.0f;

    if(fTime >= fStepTime)
    {
      iCount ++;
      lgTime = 0;
      fTime = 0;

      dTargetStroke = fStartStroke + fStepStroke * iCount;
    }

    dCurrentStroke = getCurrentStroke();
  
    dErrorStroke = dTargetStroke - dCurrentStroke;
    dErrorDuty = getDeltaDuty(dErrorStroke);
  
    // caution : Kp = 0.01;
    dDuty = dDuty + 0.01 * dErrorDuty;
  
    if(dDuty > 100) dDuty = 100;
    if(dDuty < 0) dDuty = 0;
      
    // I made 4 phase (A_1A, A_1B, B_1A, B_1B) using only one PWM Signal and 74LS04.
    // when A_1A is true, B_1A should be true and A_1B, B_1B should be false.
    analogWrite(PWM_PIN, getPWM(dDuty));

  }while(iCount <= iStepCount);  

}

void loop()
{
  for(int i=0; i<INDEX_COUNT-1; i++)
  {
    actionLoop(i);
  }
}

