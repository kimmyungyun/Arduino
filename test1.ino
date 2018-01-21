//CS - Pin 4 SCK - Pin 13  
//MOSI - Pin 11  MISO - Pin 12  
//VCC - 5V , GND- GND

const int redbtn = 9;   //left btn
const int whitebtn = 8; //right btn

#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>                        // 시리얼 통신에 필요한 라이브러리를 불러옵니다.
SoftwareSerial BTSerial(2, 3);                        // 시리얼 통신핀을 설정합니다. 
   // 이름을 BTSerial 로 설정하고, 2번 핀을 RX, 3번 핀을 TX로 설정합니다
File myFile;
int status = 0;   // 1은 파일 전송 시작. 2는 파일전송 끝.
int num=0;
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
  pinMode(redbtn,INPUT_PULLUP);
  pinMode(whitebtn,INPUT_PULLUP);
  Serial.begin(115200);                                     // 아두이노의 시리얼 속도를 9600 으로 설정
  BTSerial.begin(115200);                                  // 블루투스의 시리얼 속도를 9600 으로 설정
 //SD카드 초기화 SD.begin(4)는 CS핀 번호
  if(!SD.begin(4)){
    return;
  }
    // test.txt 파일을 쓰기 위해서 Open한다
  //여기서 블루투스 로부터 파일 명을 받아 온 후 만들어야 할듯.
  //파일 쓰기 이어서 써진다. 전에꺼 지우고, 다시 만들어서 해야되.
  //myFile = SD.open("test1.txt",FILE_WRITE);
  myFile = SD.open("test1.txt");

}

void loop()
{
  int i;

        if(myFile){
          if(digitalRead(whitebtn) == LOW && myFile.available()){
       byte ch = myFile.read();
       for(i=5;i>=0;i--)
                {
                    if(((ch>>i)&0x1) == 1)
                       Serial.print('O');  
                  else if(((ch>>i)&0x1)==0)
                     Serial.print('X');  
                if(i==4 || i==2)
                   Serial.println(); 
          }
         Serial.println("");
         Serial.println("========");
         num++;
         Serial.println(num);
          }
           if(digitalRead(redbtn) == LOW && myFile.available()){
            (&myFile-1);
       byte ch = myFile.read();
       for(i=5;i>=0;i--)
                {
                    if(((ch>>i)&0x1) == 1)
                       Serial.print('O');  
                  else if(((ch>>i)&0x1)==0)
                     Serial.print('X');  
                if(i==4 || i==2)
                   Serial.println(); 
          }
         Serial.println("");
         Serial.println("========");
         num++;
         Serial.println(num);
          }
    }
  if (BTSerial.available()){
    byte ch=(byte)BTSerial.read();
   // Serial.println(ch);
   // Serial.println("Here1");
    if(ch == 128 ){
      status = 1;
      Serial.println("Here2");
    }
    while(status == 1){    
      ch=(byte)BTSerial.read();
      if(((ch>>6)&0b11) == 0b00){
     for(i=5;i>=0;i--)
                {
                    if(((ch>>i)&0x1) == 1)
                       Serial.print('O');  
                  else if(((ch>>i)&0x1)==0)
                     Serial.print('X');  
                if(i==4 || i==2)
                   Serial.println(); 
          }
         Serial.println("");
         Serial.println("========");
         buffer[(num%50)]=ch;
         num++;
         Serial.println(num);
         if((num%50)==0)
         {
          myFile.write(buffer,50);
         }
         
      }
        else if( ((ch>>6)&0b11) == 0b01){
        status = 2;
        Serial.println("Here3");
        for(int i=0;i<(num%50);i++)
          myFile.write(buffer[i]);
          
        myFile.close();
        break;
      }
     // Serial.println(num);
    }
  }
  while(status == 2)  //파일 전송 및 입력이 다 끝났다면... 
  {
        if(myFile){
          if(digitalRead(whitebtn) == LOW && myFile.available()){
       byte ch = myFile.read();
       for(i=5;i>=0;i--)
                {
                    if(((ch>>i)&0x1) == 1)
                       Serial.print('O');  
                  else if(((ch>>i)&0x1)==0)
                     Serial.print('X');  
                if(i==4 || i==2)
                   Serial.println(); 
          }
         Serial.println("");
         Serial.println("========");
         num++;
         Serial.println(num);
          }
    }
  
  }
    delay(175);    //채터링 방지 이거를 파일 전송및 입력이 다끝난 후..
    // 파일을 읽어 올 때 안에다가 넣어야 하지만 지금 실험중이기 때문에 밖에 꺼내놓음.
}
