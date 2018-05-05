//CS - Pin 4 SCK - Pin 13
//MOSI - Pin 11  MISO - Pin 12
//VCC - 5V , GND- GND

//  Backpanel P20 with 5 braille cells
//  -----------------------------------------------------------------------------------------------             7 +200V  <-------------------------- DC-DC +185V
// |  cell 0   |   cell 1  |  cell 2   |  cell 3   |  cell 4   |  cell 5   |  cell 6   |  cell 7   | <-- Cable  6 n.c.
//  ------------------------------------------------------------------------------------------------            5 GND    <----   Arduino GND    ---> DC-DC GND
// |  1 oo 8   |  1 oo 8   |  1 oo 8   |  1 oo 8   |  1 oo 8   |  values of the pins               |            4 CLK    <----   Arduino pin 8
// |  2 oo 16  |  2 oo 16  |  2 oo 16  |  2 oo 16  |  2 oo 16  |                                   |            3 STRB   <----   Arduino pin 7
// |  4 oo 32  |  4 oo 32  |  4 oo 32  |  4 oo 32  |  4 oo 32  |                                   |            2 Din    <----   Arduino pin 6
// | 64 oo 128 | 64 oo 128 | 64 oo 128 | 64 oo 128 | 64 oo 128 |                                   |            1 +5V    <----   Arduino +5V    ---> DC-DC +5V
//  -----------------------------------------------------------------------------------------------                              Arduino pin 5  ---> DC-DC /ON

const int ON     =  5;  // DC-DC Converter ON Pin. You can use it to switch off the cell power in a sleeping mode 
const int DATA   =  6;  // Backpanel Pin 2 (Din)
const int STROBE =  7;  // Backpanel Pin 3 (STRB)
const int CLOCK  =  8;  // Backpanel Pin 4 (CLK)

const int cellCount = 8;
byte cells[cellCount];

const int redbtn = 0;   //left btn
const int whitebtn = 1; //right btn
unsigned long pos = 1;    //현재 위치 변수.
unsigned long pos2 = 5;
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>                        // 시리얼 통신에 필요한 라이브러리를 불러옵니다.
SoftwareSerial BTSerial(3, 2);                        // 시리얼 통신핀을 설정합니다.
// 이름을 BTSerial 로 설정하고, 2번 핀을 RX, 3번 핀을 TX로 설정합니다
File myFile, myFile2;
int status = 0;   // 1은 파일 전송 시작. 3은 파일전송 끝. 2가 파일 읽어오기 시작.
int num = 0;
unsigned long size = 0;   //점자 파일 크기.
byte buffer[50];
/*
  CS       ---------------------- Pin 4

  SCK    ---------------------- Pin 13

  MOSI  ---------------------- Pin 11

  MISO  ---------------------- Pin 12

  VCC    ---------------------- 5V

  GND    ---------------------- GND
*/
void setup()
{
  pinMode(ON,     OUTPUT);  // Braille cell Setup
  pinMode(DATA,   OUTPUT);
  pinMode(STROBE, OUTPUT);
  pinMode(CLOCK,  OUTPUT);

  digitalWrite(ON, 0);  // 0=ON, 1=OFF  //DC-DC Converter On   0 = ON, 1 = OFF
  
  int i = 0;
  pinMode(redbtn, INPUT_PULLUP);
  pinMode(whitebtn, INPUT_PULLUP);
  Serial.begin(115200);                                     // 아두이노의 시리얼 속도를 9600 으로 설정
  BTSerial.begin(115200);                                  // 블루투스의 시리얼 속도를 9600 으로 설정
  //SD카드 초기화 SD.begin(4)는 CS핀 번호
  if (!SD.begin(4)) {
    return;
  }

  if (SD.exists("test1.txt"))
    SD.remove("test1.txt");
  // test.txt 파일을 쓰기 위해서 Open한다
  //여기서 블루투스 로부터 파일 명을 받아 온 후 만들어야 할듯.
  //파일 쓰기 이어서 써진다. 전에꺼 지우고, 다시 만들어서 해야되.
  myFile = SD.open("test1.txt", FILE_WRITE);
}

void loop()
{
  int i;
  if (BTSerial.available()) {
    byte ch = (byte)BTSerial.read();
    // Serial.println(ch);
    // Serial.println("Here1");
    if (ch == 128 ) {   //파일 전송 시작을 알리는 프로토콜 인식
      status = 1;
      Serial.println("Here2");
    }
    while (status == 1) {   //파일 데이터 프로토콜 인식
      if (BTSerial.available()) {
        ch = (byte)BTSerial.read();
        if (((ch >> 6) & 0b11) == 0b00) {
          Print(ch);
          buffer[(num % 50)] = ch;
          num++;
          Serial.println(num);
          if ((num % 50) == 0)
            myFile.write(buffer, 50);
        }
        else if ( ((ch >> 6) & 0b11) == 0b01) { //파일 전송 끝을 알리는 프로토콜 인식
          status = 3;   //종료를 알리는 status
          for (int i = 0; i < (num % 50); i++)
            myFile.write(buffer[i]);

          myFile.close();
          Serial.println("Here3");
          break;
        }
        // Serial.println(num);
      }
    }
  }
  while (status == 2 || status == 3 || status == 4) //파일 전송 및 입력이 다 끝났다면...
  {
    if (status == 3) {
      //파일 여는 것을 한번만 실행하기 위해서.

      ReSize(); //파일 크기를 5의 배수로 만드는 작업.
      Serial.println(size);
      myFile2 = SD.open("test1.txt");   //다시 열음.
      Serial.println(size);
      status = 2;

      continue;
    }
    if (myFile2) {
      if (status == 2) { //임시 status 값. 처음 읽는거 확인.
        status = 4;
        Serial.println("Here999");
        for (i = pos; i <= pos2; i++) {
          myFile2.seek(i);
          byte ch = myFile2.read();
          Print(ch);
        }
        Serial.println(myFile2.position());  //myFile.position() 함수가 현재 위치를 반환.
        Serial.println(pos);                //맨 처음 부분이 1인듯. 원래 0부터 나올텐데 여기서는 0으로 돌아가질 않음.
      }
      if (status == 4) {  //한번 읽고 난뒤에 버튼을 누른다면..
        if (digitalRead(whitebtn) == HIGH && (myFile2.available()+1)) {
          if ((pos2 + 5) <= size) {
            pos = pos + 5;
            pos2 = pos2 + 5;
          }
        }
        if (digitalRead(redbtn) == HIGH && (myFile2.available()+1)) {
          if (pos > 1 && pos2 > 6) {
            pos = pos - 5;
            pos2 = pos2 - 5;
          }
        }

        for (i = pos; i <= pos2; i++) {
          myFile2.seek(i);
          byte ch = myFile2.read();
          Print(ch);
          Read_Braille(ch,i); // 점자 튀어나오기.
          Serial.print(pos);
          Serial.print(" ");
          Serial.println(pos2);
          Serial.println(myFile2.available()+1);
        }

        delay(175);     //채터링 방지 이거를 파일 전송및 입력이 다끝난 후..
      }
    }
  }
}
void ReSize() {
  int Resize_i;
  byte tmp = 0x000000;
  myFile2 = SD.open("test1.txt", FILE_WRITE);
  size = myFile2.size();
  for (Resize_i = 0; Resize_i < (5 - (size % 5) + 5 ); Resize_i++)
  {
    myFile2.write(tmp);
  }
  myFile2.close();
  size = size + 5 - (size % 5) + 5;

}
void Print(char ch) {
  int i;
  for (i = 0; i < 3; i++)
  {
    if (((ch >> i) & 0b1) == 0b1)
      Serial.print("O");
    else if (((ch >> i) & 0b1) == 0b0)
      Serial.print("X");
    if (((ch >> (i + 3)) & 0b1) == 0b1)
      Serial.print("O");
    else if (((ch >> (i + 3)) & 0b1) == 0b0)
      Serial.print("X");
    Serial.println("");
  }
  Serial.println("");
  Serial.println("========");

}

void Read_Braille(char ch, int idx){
    int i;
  for (i = 0; i<5;i++)  //점자 전부다 내리기.(초기화)
  {
    cells[i] = 0;
  }
  for (i = 0; i <= 5; i++)
  {
    if(((ch>>i) & 0b1))
      cells[idx % 5] += Pow(2,i);  //cells의 인덱스 값 잘 정해야됨. 아직 안정함.
  }
  Flush();
  delay(50);
}

// Send the data
void Flush ()
{
  // This example is for one P20 backpanel. If you are using others you have to change the bit order!
  // P20: 6,7,2,1,0,5,4,3
  // P16: 6,2,1,0,7,5,4,3
  // B11: 0,1,2,3,4,5,6,7  
  // Rotate the bytes from the right side. Cell 0 comes first and is on the left position.
  
  for (int i = 0; i < cellCount; i++)
  {
    if ( bitRead(cells[i], 6) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 7) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 2) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 1) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 0) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 5) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 4) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
    if ( bitRead(cells[i], 3) )   {digitalWrite(DATA, 0);} else {digitalWrite(DATA, 1);}     digitalWrite(CLOCK, 1); digitalWrite(CLOCK, 0);
  }

  digitalWrite(STROBE, 1);  // Strobe on
  digitalWrite(STROBE, 0);  // Strobe off
}

