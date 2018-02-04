//CS - Pin 4 SCK - Pin 13
//MOSI - Pin 11  MISO - Pin 12
//VCC - 5V , GND- GND

const int redbtn = 9;   //left btn
const int whitebtn = 8; //right btn
unsigned long pos = 1;    //현재 위치 변수.
unsigned long pos2 = 5;
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>                        // 시리얼 통신에 필요한 라이브러리를 불러옵니다.
SoftwareSerial BTSerial(2, 3);                        // 시리얼 통신핀을 설정합니다.
// 이름을 BTSerial 로 설정하고, 2번 핀을 RX, 3번 핀을 TX로 설정합니다
File myFile, myFile2;
int status = 0;   // 1은 파일 전송 시작. 3은 파일전송 끝. 2가 파일 읽어오기 시작.
int num = 0;
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
      myFile2 = SD.open("test1.txt");     //파일 여는 것을 한번만 실행하기 위해서.
      status = 2;
      Serial.println("Here10");
      continue;
    }
    if (myFile2) {
      if (status == 2) { //임시 status 값. 처음 읽는거 확인.
        status = 4;
        for (i = pos; i <= pos2; i++) {
          myFile2.seek(i);
          byte ch = myFile2.read();
          Print(ch);
        }
        Serial.println(myFile2.position());  //myFile.position() 함수가 현재 위치를 반환.
        Serial.println(pos);                //맨 처음 부분이 1인듯. 원래 0부터 나올텐데 여기서는 0으로 돌아가질 않음.
      }
      if (status == 4) {  //한번 읽고 난뒤에 버튼을 누른다면..
        if (digitalRead(whitebtn) == LOW && myFile2.available()) {
          pos = pos + 5;
          pos2 = pos2 + 5;
        }
        if (digitalRead(redbtn) == LOW && myFile2.available()) {
          pos = pos - 5;
          pos2 = pos2 - 5;
        }

        for (i = pos; i <= pos2; i++) {
          myFile2.seek(i);
          byte ch = myFile2.read();
          Print(ch);
          Serial.println(myFile2.position());
          Serial.println(i);
        }
      }
      delay(175);     //채터링 방지 이거를 파일 전송및 입력이 다끝난 후..
    }
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

