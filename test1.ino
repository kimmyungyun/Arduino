//CS - Pin 4 SCK - Pin 13
//MOSI - Pin 11  MISO - Pin 12
//VCC - 5V , GND- GND

const int redbtn = 9;   //left btn
const int whitebtn = 8; //right btn
unsigned long pos = 0;    //현재 위치 변수.

#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>                        // 시리얼 통신에 필요한 라이브러리를 불러옵니다.
SoftwareSerial BTSerial(2, 3);                        // 시리얼 통신핀을 설정합니다.
// 이름을 BTSerial 로 설정하고, 2번 핀을 RX, 3번 핀을 TX로 설정합니다
File myFile;
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
  int i;
  pinMode(redbtn, INPUT_PULLUP);
  pinMode(whitebtn, INPUT_PULLUP);
  Serial.begin(115200);                                     // 아두이노의 시리얼 속도를 9600 으로 설정
  BTSerial.begin(115200);                                  // 블루투스의 시리얼 속도를 9600 으로 설정
  //SD카드 초기화 SD.begin(4)는 CS핀 번호
  if (!SD.begin(4)) {
    return;
  }
  // test.txt 파일을 쓰기 위해서 Open한다
  //여기서 블루투스 로부터 파일 명을 받아 온 후 만들어야 할듯.
  //파일 쓰기 이어서 써진다. 전에꺼 지우고, 다시 만들어서 해야되.
  myFile = SD.open("test1.txt", FILE_WRITE);

}

//파일 입력을 할 때 처음 입력한 부분이 사라지는 현상이 있는듯...
//노란의 ㄴ 이 사라짐... 이걸 해결할 방법을 모색해야 할듯..
void loop()
{
  int i;
  if (BTSerial.available()) {
    byte ch = (byte)BTSerial.read();
    // Serial.println(ch);
    // Serial.println("Here1");
    if (ch == 128 ) {   //파일 전송 시작을 알리는 프로토콜 인식
      status = 1;
      myFile.write(0b0); //아무것도 없는 거 일단 한번 입력해봄.
      Serial.println("Here2");
    }
    while (status == 1) {   //파일 데이터 프로토콜 인식
      ch = (byte)BTSerial.read();
      if (((ch >> 6) & 0b11) == 0b00) {
        //Print(ch);
        buffer[(num % 50)] = ch;
        num++;
        Serial.println(num);
        if ((num % 50) == 0)
        {
          myFile.write(buffer, 50);
        }

      }
      else if ( ((ch >> 6) & 0b11) == 0b01) { //파일 전송 끝을 알리는 프로토콜 인식
        status = 3;   //종료를 알리는 status
        Serial.println("Here3");
        for (int i = 0; i < (num % 50); i++)
          myFile.write(buffer[i]);

        myFile.close();
        break;
      }
      // Serial.println(num);
    }
  }
  while (status == 2 || status == 3 || status == 4) //파일 전송 및 입력이 다 끝났다면...
  {
    if (status == 3) {
      myFile = SD.open("test1.txt", FILE_READ);     //파일 여는 것을 한번만 실행하기 위해서.
      status = 2;
      continue;
    }
    if (myFile) {
      if (status == 2) { //임시 status 값. 처음 읽는거 확인.
        status = 4;
        byte ch = myFile.read();
        Print(ch);
        Serial.println(myFile.position());  //myFile.position() 함수가 현재 위치를 반환.
        Serial.println(pos);                //맨 처음 부분이 1인듯. 원래 0부터 나올텐데 여기서는 0으로 돌아가질 않음.
      }
      if (status == 4) {  //한번 읽고 난뒤에 버튼을 누른다면..
        if (digitalRead(whitebtn) == LOW && myFile.available()) {
          pos++;
          myFile.seek(pos);
          byte ch = myFile.read();
          Print(ch);
          //Serial.println(num);
          Serial.println(myFile.position());
          Serial.println(pos);
        }
        if (digitalRead(redbtn) == LOW && myFile.available()) {
          pos--;
          myFile.seek(pos);
          byte ch = myFile.read();
          Print(ch);
          //Serial.println(num);
          Serial.println(myFile.position());
          Serial.println(pos);
        }
      }
    }
  }
  delay(175);    //채터링 방지 이거를 파일 전송및 입력이 다끝난 후..
  // 파일을 읽어 올 때 안에다가 넣어야 하지만 지금 실험중이기 때문에 밖에 꺼내놓음.
}

void Print(char ch) {
  int i;
  for (i = 5; i >= 0; i--)
  {
    if (((ch >> i) & 0x1) == 1)
      Serial.print('O');
    else if (((ch >> i) & 0x1) == 0)
      Serial.print('X');
    if (i == 4 || i == 2)
      Serial.println();
  }
  Serial.println("");
  Serial.println("========");
}

