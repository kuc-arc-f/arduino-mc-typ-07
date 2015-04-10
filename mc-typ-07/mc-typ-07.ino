/*
 mc-typ-07 ver :1.0
*/
#include <MsTimer2.h>
#include <SPI.h>
#include <Ethernet.h>

/*device  */
String MC_ID ="0";
String mRestKey="";
char server[] = "dns123.com";  //DNS

#define mSecHttp 60000

#define SENSOR   0
#define SENSOR_1 1
#define SENSOR_2 2
#define SENSOR_3 3

#define RELAY_PIN    5
#define RELAY_PIN_2  6
#define RELAY_PIN_3  7
#define RELAY_PIN_4  8

#define mSTAT_HTTP  1
#define mSTAT_VALVE 2
/* http */
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192,168,1,200 );
EthernetClient client;

//Struct
struct stParam{
 String stat;
 String statCode;
 String moi_num;
 String vnum_1;
 String vnum_2;
 String vnum_3;
 String vnum_4;
 String kai_num_1;
 String kai_num_2;
 String kai_num_3;
 String kai_num_4;
};

boolean mNOW_CONE=false;
String mResponse3 ="";
int mSTAT =0;
int mCtOpen   = 0;
int mValveGet = 20;

void timer_func() {
  if(mSTAT == mSTAT_HTTP){
    mResponse3="";
    proc_start();
  }
}

void proc_start() {
  int iSen =0;
  int iSen2=0;
  int iSen3=0;
  int iSen4=0;
  
  iSen =analogRead(SENSOR);
  iSen2 =analogRead(SENSOR_1);
  iSen3 =analogRead(SENSOR_2);
  iSen4 =analogRead(SENSOR_3);
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
  delay(1000);  
  String sSen  = String(iSen);
  String sSen2  = String(iSen2);
  String sSen3  = String(iSen3);
  String sSen4  = String(iSen4);
  
  //MC_ID
  Serial.println("===== request =====");
  Serial.println("SEN_1=" + sSen );

  Serial.println("connecting...");
  String sHost ="Host: ";
  sHost.concat(server);  
  if (client.connect(server, 80)) {
    Serial.println("connected");
    String sReq= "GET /agrisv2/php/mc_post2.php?mc_id=";
    sReq.concat(MC_ID);
    sReq.concat("&rkey=");
    sReq.concat(mRestKey);
    sReq.concat("&snum_1=");
    sReq.concat(sSen);
    sReq.concat("&snum_2=");
    sReq.concat(sSen2);
    sReq.concat("&snum_3=");
    sReq.concat(sSen3);
    sReq.concat("&snum_4=");
    sReq.concat(sSen4);
    sReq.concat(" HTTP/1.1");
Serial.println(sReq);    
    client.println(sReq);
    client.println( sHost );
    client.println("Connection: close");
    client.println();
    mNOW_CONE =true;
  } 
  else {
    Serial.println("connection failed");
  }

}

void proc_valve(){
  String sCt = String(mCtOpen);
Serial.println("proc_valve=" + sCt);
  if(mCtOpen >= mValveGet){
    digitalWrite(RELAY_PIN   , LOW);
    digitalWrite(RELAY_PIN_2 , LOW);
    digitalWrite(RELAY_PIN_3 , LOW);
    digitalWrite(RELAY_PIN_4 , LOW);
    
    mSTAT = mSTAT_HTTP;
    mCtOpen=0;
  }
  mCtOpen ++;
  delay( 1000 );
}

void proc_http(){
  if(mNOW_CONE==true){
    if (client.available()) {
      char c = client.read();
//Serial.print(c);
      mResponse3.concat(c);
    }
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
      mNOW_CONE=false;
  Serial.print("==== response ==== \n");
  int iSt = mResponse3.indexOf("web-response1=");
  iSt = iSt+ 14;
  String sRes4 = mResponse3.substring(iSt );
  Serial.println("res=" + sRes4);
  Serial.print("==== response.END ==== \n");
      struct stParam param;
      param.stat     = sRes4.substring(0,1);
      param.statCode = sRes4.substring(1,4);
      param.moi_num  = sRes4.substring(4,8);
      param.vnum_1   = sRes4.substring(8, 9);
      param.vnum_2   = sRes4.substring(9, 10);
      param.vnum_3   = sRes4.substring(10,11);
      param.vnum_4   = sRes4.substring(11,12);      
      param.kai_num_1= sRes4.substring(12,15);      
      param.kai_num_2= sRes4.substring(15,18);      
      param.kai_num_3= sRes4.substring(18,21);      
      param.kai_num_4= sRes4.substring(21,24); 
      
      if(param.stat=="1"){
        if((param.vnum_1=="1") || (param.vnum_2=="1") || (param.vnum_3=="1") || (param.vnum_4=="1")){
          mSTAT =mSTAT_VALVE;
          mValveGet =param.kai_num_1.toInt();
          if(param.vnum_1=="1"){
  Serial.print( "vnum_1.STart\n" );
            digitalWrite(RELAY_PIN   , HIGH);
          }
          if(param.vnum_2=="1"){
  Serial.print( "vnum_2.STart\n" );
            digitalWrite(RELAY_PIN_2 , HIGH);
          }
          if(param.vnum_3=="1"){
  Serial.print( "vnum_3.STart\n" );
            digitalWrite(RELAY_PIN_3 , HIGH);
          }
          if(param.vnum_4=="1"){
  Serial.print( "vnum_4.STart\n" );
            digitalWrite(RELAY_PIN_4 , HIGH);
          }
          
        }
      }
    } //client.connected
  } //mNOW_CONE
}

void setup() {
    mSTAT = mSTAT_HTTP;
  pinMode(SENSOR   ,INPUT);
  pinMode(SENSOR_1 ,INPUT);  
  pinMode(SENSOR_2 ,INPUT);
  pinMode(SENSOR_3 ,INPUT);    
  pinMode(RELAY_PIN   ,OUTPUT);
  pinMode(RELAY_PIN_2 ,OUTPUT);
  pinMode(RELAY_PIN_3 ,OUTPUT);
  pinMode(RELAY_PIN_4 ,OUTPUT);
  
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  MsTimer2::set( mSecHttp, timer_func);
  MsTimer2::start();
}

void loop()
{
  if(mSTAT == mSTAT_HTTP){
    proc_http();
  }
  else if(mSTAT == mSTAT_VALVE){
    proc_valve();
  }
}









