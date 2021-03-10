#define Mrot 48
#define hmax 59000

#define M1S 3 //M1 вала
#define DRV1_Enable 5 //каретка
#define DRV2_Enable 4 //вал
#define shaftdir 6
#define shaftstep 7
#define M0 8
#define M1 9
#define M2 10
#define ndir 12
#define nstep 11
#define interv 1
int turns;
int h;
int coilstart;
int coilend;
int coilcount;

void recalib()
{

  shift(-700.0);
  turns=0;
}

void shift(int steps)
{
  digitalWrite(DRV1_Enable,1);
  delay(100); //Рудимент, вероятно не нужен
  bool dir;
  long int a;
  a = steps; //convert mm to steps 5900
  if (a > 0)
  {
    dir = 0; // chosing direction
  }
  else
  {
    dir = 1;
    a = -a;
  }
  digitalWrite(ndir, dir); //send direction to pin7
  while (a > 0 && h <= hmax && h >= 0)
  {
    for (int i=0;i<50;i++)
    {
    digitalWrite(nstep, 1);
    delayMicroseconds(800);
    digitalWrite(nstep, 0);
    delayMicroseconds(200);
    }
    a--;
    if (dir && h > 0) 
    {
      h--;
    }
    if (!dir && h < hmax) 
    {
      h++;
    }
  }
  digitalWrite(DRV1_Enable,0);
}

int readval()
{
  int x;
  Serial.flush();
  Serial.println("Введите значение");
  while(true)
  {
    if (Serial.available() > 0)
    {      
    x = Serial.parseInt();
    Serial.println(x);
    Serial.flush();
    return (x);
    }
  delay(100);
  }
}

int readvalf()
{
  float x;
  Serial.flush();
  Serial.println("Введите число с точкой");
  while(true)
  {
    if (Serial.available() > 0)
    {      
    x = Serial.parseFloat();
    Serial.println(x);
    Serial.flush();
    return(x);
    }
   delay(100);
  }
}

void turn(int n)
{
  digitalWrite(DRV2_Enable,1);
  int temp;
  temp=abs(n);
  if (n<0)
  {
   digitalWrite(shaftdir,0);
  }
  else
  {
   digitalWrite(shaftdir,1);
  }
  while (temp>0)
  {
    for (int i = 0; i<48; i++)
  {
    digitalWrite(shaftstep,1);
    delay(1);
    digitalWrite(shaftstep,0);
    delay(2);
  }
  temp--;
  }
  digitalWrite(DRV2_Enable,0);
}

void setup() {
  Serial.begin(9600);
  pinMode(M0, 1);
  pinMode(M1, 1);
  pinMode(M2, 1);
  pinMode(ndir, 1);
  pinMode(nstep, 1);
  pinMode(shaftdir,1);
  pinMode(shaftstep, 1);
  pinMode(DRV1_Enable, 1);
  pinMode(DRV2_Enable, 1);
  pinMode(5, 1);
  pinMode(M1S,1);
  digitalWrite(M1S,1);
  Serial.println("Загрузка завершена. Введите команду:");
}

void makecoil(int b,int e,int c)
{
  int dir = interv; 
  shift(dir);
  
  turn(4);
  turns++;
  Serial.println("//Пауза..");  
  Serial.flush();
  Serial.print(c);
  while(readval() != 1234)
  {
    delay(1000);
  }
  for (int i =(c-1) ;i>0 ; i-=1)
  {
    Serial.println(i);
    if (h>e) {dir=-interv;}
    if (h<b) {dir=interv;} 
    turn(4);
    shift(dir);
    turns+=1;
  }
}
void loop()
{
  char input ;
  
  if (Serial.available() > 0)
  {
    input = Serial.readString()[0];
    if (input == char("s"[0]))
    {
      //shift command
      Serial.println("Команда для сдвига каретки,число может быть отрицательным");
      shift(readvalf());
      Serial.println("Выполнен сдвиг каретки");
      
    }
    if (input==char("t"[0]))
    {
      //turn command
      Serial.println("Команда для вращения вала, ввести количество оборотов, может быть отрицательным");
      int temp = readval();
      turn(temp);
      turns+=temp;
      Serial.println("Выполнено вращение вала");
    }
    
    if (input==char("r"[0]))
    {
      Serial.println("Клибровка каретки...");
      recalib();
      h=0;
      Serial.println("Завершено");
    }
    
    if (input==char("b"[0]))
    {
      coilstart=h;
      Serial.println("Начало катушки установлено в текущую позицию каретки");     
    }
    
    if (input==char("e"[0]))
    {
      coilend=readval()+h;
      Serial.println("Конец катушки установлен");
      
    }
    
    if (input==char("n"[0]))
    {
      Serial.println("Введите число полных витков катушки");
      coilcount=readval();     
    }

     if (input==char("m"[0]))
    {
      Serial.println("Начинаю намотку");
      makecoil(coilstart,coilend,coilcount);
      
    }
    
     if (input==char("c"[0]))
    {
      Serial.println("На данный момент с запуска было сделано оборотов:");
      Serial.println(turns);
    }
     if (input==char("d"[0]))
    {
      Serial.println(h);
      Serial.println(coilstart);
      Serial.println(coilend);
    }
   
  }



}
