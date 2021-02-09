//This code is developed by WirelessM8
//Определение управляющих Pin выходов

#define M1S 3         //M1 вала
#define DRV2_Enable 4 //!SLEEP вала
#define DRV1_Enable 5 //!SLEEP каретки
#define shaftdir 6    //DIR вала
#define shaftstep 7   //STEP вала
#define M0 8          //M0 каретки
#define M1 9          //M1 каретки
#define M2 10         //M2 каретки
#define nstep 11      //STEP каретки
#define ndir 12       //DIR каретки
//Базовые настройки программы
#define maxnPos 1000  //максимальная позиция каретки 

int curPos;       //текущая позиция в шагах
int shaftStepStr; //Счетчик шагов вала
int coilBeginPos; //Начало катушки в шагах
int coilEndPos;   //Конец катушки в шагах
int coilCount;    //Количество катушки в оборотах
byte shaftMode; //Текущий режим вала (Скорость)

// данные для SerialCOM
String SerialGot[5];

void shift(int steps)
{
  digitalWrite(DRV1_Enable, 1);

  bool dir;
  int a;
  a = steps;

  //Выбор направления движения
  if (a > 0)
  {
    dir = 0;
  }
  else
  {
    dir = 1;
    a = -a;
  }
  digitalWrite(ndir, dir);
  a = constrain(a, 0, maxnPos - curPos);

  //Цикл смещения
  while (a > 0)
  {
    //цикл для смещения с позиции простоя, итого производится a*50 шагов
    for (int i = 0; i < 50; i++)
    {
      digitalWrite(nstep, 1);
      delayMicroseconds(1000);
      digitalWrite(nstep, 0);
      delayMicroseconds(500);
    }
    a--;
    curPos = curPos - dir + (dir < 1);
  }

  curPos = constrain(curPos, 0, 800);
  digitalWrite(ndir, 0);
  digitalWrite(DRV1_Enable, 0);
  Serial.println("R");
}

void turn(boolean dir)
{
  digitalWrite(shaftdir, dir);
  for (int i = 0; i < (48 * pow(4, shaftMode)); i++)
  {
    digitalWrite(shaftstep, 1);
    delayMicroseconds(20000 / (shaftMode + 1));
    digitalWrite(shaftstep, 0);
    delayMicroseconds(2000 / (shaftMode + 1));
  }
  Serial.println("R");
}

// Раздел вспомогательных функций для GUI

void setShaftMode(byte Mode)
{
  shaftMode = constrain(Mode, 0, 6);
  digitalWrite(M1S, bitRead(shaftMode, 0));

}

void shaftStep()
{
  digitalWrite(shaftstep, 1);
  delay(4);
  digitalWrite(shaftstep, 0);
  delay(4);
}

void Parse(String Input[5])
{

  if (Input[0] == "t") //turn command
  {
    int a = Input[1].toInt();
    byte mode = Input[2].toInt();
    bool dir = a < 0;
    setShaftMode(mode);
    digitalWrite(DRV2_Enable, 1);
    for (int i = 0; i < abs(a); i++)
    {
      turn(dir);
    }
    digitalWrite(DRV2_Enable, 0);
  }

  if (Input[0] == "s") //shift command
  {
    shift(Input[1].toInt());
  }

  if (Input[0] == "PosGet") //curPos get command
  {
    Serial.println(curPos);
  }

  if (Input[0] == "PosSet") //curPos set command
  {
    curPos = Input[1].toInt();
  }

  if (Input[0] == "DebugLED")
  {

    digitalWrite(13, bitRead(Input[1].toInt(), 0));
  }
  SerialGot[0] = "";
  SerialGot[1] = "";
  SerialGot[2] = "";
  SerialGot[3] = "";
  SerialGot[4] = "";
  
}

// Конец раздела вспомогательных функций для GUI


void setup() {
  //Установка соединения 19200 бод
  Serial.begin(19200);
  //Установка режимов pin в "Вывод"
  pinMode(M0, 1);
  pinMode(M1, 1);
  pinMode(M2, 1);
  pinMode(ndir, 1);
  pinMode(nstep, 1);
  pinMode(shaftdir, 1); 
  pinMode(shaftstep, 1);
  pinMode(DRV1_Enable, 1);
  pinMode(DRV2_Enable, 1);
  pinMode(5, 1);
  pinMode(M1S, 1);

  //for Debug
  pinMode(13, 1);
}

void loop() {
  delay(100);
}

//Функция для получения данных с компьютера
void serialEvent() {
  byte i = 0;
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    if (inChar != '\n' and inChar != '\r')
    {
      if (inChar == ';') {
        i++;
      }
      else
      {
        SerialGot[i] += inChar;
      }
    }
    else
    {
      Parse(SerialGot);
    }
  }
}
