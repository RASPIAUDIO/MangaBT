#include "Arduino.h"
#include "BluetoothA2DPSink.h"
#include <NeoPixelBus.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"
extern "C"
{
//#include "hal_i2c.h"
#include "tinyScreen128x64.h"
}
#include "esp_mac.h"


int Napp;
#define Nbt 1
#define Ntst 2

#define I2S_BCLK    5
#define I2S_LRC     25
#define I2S_DOUT    26
#define PUSH        GPIO_NUM_0
#define ROTARY_A    GPIO_NUM_32
#define ROTARY_B    GPIO_NUM_19
#define SDAA         18
#define SCLL         23
#define LED2        GPIO_NUM_4

#define SDD GPIO_NUM_34     
#define SD_CS         13
#define SPI_MOSI      15
#define SPI_MISO      2
#define SPI_SCK       14
//////////////////////////////
// NeoPixel led control
/////////////////////////////
#define PixelCount 1
#define PixelPin 22
RgbColor RED(255, 0, 0);
RgbColor GREEN(0, 255, 0);
RgbColor BLUE(0, 0, 255);
RgbColor YELLOW(255, 128, 0);
RgbColor WHITE(255, 255, 255);
RgbColor BLACK(0, 0, 0);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

Audio audio(false, I2S_DAC_CHANNEL_BOTH_EN, 1);
File root;
File F;
int stop;

BluetoothA2DPSink a2dp_sink;

TaskHandle_t Tencoder;
int N = 0;
char b[40];
uint8_t c[20];
int PN = -1;
int P = 0;
int vol = 7;
int Pvol;
bool mute = false;
#define volMAX 21.

char title[80];
char artist[80];
bool toDisplay = true;
char connection[80] = "";
int connection_state;
int audio_state;
#define Disconnected 0
#define Connecting 1
#define Connected 2
#define Disconnecting 3
#define Suspended 0
#define Stopped 1
#define Started 2


uint8_t mac[6] = "";
char BTName[20];
const uint8_t voltb[22]={   0,  1,  2,  3,  4 , 6 , 8, 10, 12, 14, 17,
                           20, 23, 27, 30 ,34, 38, 43 ,48, 52, 58, 64}; //22 elements



void test(void)
{
 ////////////////////////////////////////////////////////////////
// init NeoPixel led handle
///////////////////////////////////////////////////////////////
  strip.Begin();  
///////////////////////////////////////////////////////////////
// init spdif led
///////////////////////////////////////////////////////////////
  gpio_reset_pin(LED2);
  gpio_set_direction(LED2, GPIO_MODE_OUTPUT); 


///////////////////////////////////////////////////////////////
// test
//////////////////////////////////////////////////////////////   
 
   clearBuffer();
   drawBigStrC(16,"Factory");
   drawBigStrC(40,"Test");   
   sendBuffer();
   delay(1000);


///////////////////////////////////////////////////////////////
// test #1 
// LED1 and LED2
///////////////////////////////////////////////////////////////
   clearBuffer();
   drawStrC(10, "Test #1: leds");
   drawBigStrC(24,"PUSH x4");
   sendBuffer();

   while(gpio_get_level(PUSH) == 1) delay(100);

   strip.SetPixelColor(0, RED);
   strip.Show();
   gpio_set_level(LED2, 1);
   delay(500);

   while(gpio_get_level(PUSH) == 1) delay(100);
      
   strip.SetPixelColor(0, GREEN);
   strip.Show();
   gpio_set_level(LED2, 1);
   delay(500);   

   while(gpio_get_level(PUSH) == 1) delay(100);
   
   strip.SetPixelColor(0, WHITE);
   strip.Show();
   gpio_set_level(LED2, 0);
   delay(500);   

   while(gpio_get_level(PUSH) == 1) delay(100);
     
   strip.SetPixelColor(0, BLACK);
   strip.Show();

   clearBuffer();
   drawStrC(10, "Test #1: leds");
   drawBigStrC(24,"OK");
   sendBuffer();

   delay(1000);
////////////////////////////////////////////////////
// test#2
// Rotational encoder
////////////////////////////////////////////////////

   clearBuffer();
   drawStrC(10, "Test #2: encoder");
   drawBigStrC(24,"TURN");
   drawBigStrC(44,"LEFT");  
 
   sendBuffer();   
   delay(1000);
   N = 0;
   while(N > -16)
   {
    sprintf(b,"%d", N);
    clearBuffer();
    drawBigStrC(24, b);
    sendBuffer();
    delay(100);
   }
   delay(1000);
   clearBuffer();
   drawStrC(10, "Test #2");
   drawBigStrC(24,"OK");
   sendBuffer();
   delay(1000);
   clearBuffer();
   drawStrC(10, "Test #2: encoder");
   drawBigStrC(24,"TURN");   
   drawBigStrC(44,"RIGHT");

   sendBuffer();   
   delay(2000);
    N = 0;
   while(N < 16)
   {
    sprintf(b,"%d", N);
    clearBuffer();
    drawBigStrC(24, b);
    sendBuffer();
    delay(100);
   }
   delay(1000);
   clearBuffer();
   drawStrC(10, "Test #2: encoder");
   drawBigStrC(24,"OK");
   sendBuffer();
   delay(1000); 



/////////////////////////////////////////////////
// test#3
// SD
/////////////////////////////////////////////////   
   clearBuffer();
   drawStrC(10, "Test #3: SD");
   drawBigStrC(24,"PUSH");
   sendBuffer();
   while(gpio_get_level(PUSH) == 1) delay(100);   

   gpio_reset_pin(SDD);
   gpio_set_direction(SDD, GPIO_MODE_INPUT);  
   gpio_set_pull_mode(SDD, GPIO_PULLUP_ONLY);
   if(gpio_get_level(SDD) == 0)
   {
   clearBuffer();   
   drawStrC(10, "Test #3");
   drawStrC(24,"INSERT SD !");
   sendBuffer();
   }
   delay(1000);
   while(gpio_get_level(SDD) == 0) delay(100);

   SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
   if(!SD.begin(SD_CS))printf("init. SD failed !\n");   
   F = SD.open("/test", "w");
   F.write((const uint8_t*)"1234567890", 11);
   F.close();

   F = SD.open("/test", "r");
   int l = F.read(c, 11);
   F.close();

   if((l != 11) || (strcmp("1234567890",(char*) c) != 0))
   {
   clearBuffer();
   drawStrC(10, "Test #3: SD");
   drawBigStrC(24,"FAILED");
   sendBuffer();
   }
   else
   {
   clearBuffer();
   drawStrC(10, "Test #3: SD");
   drawBigStrC(24,"OK");
   sendBuffer();
   }
   SD.remove("/test");
/////////////////////////////////////////////////////////////
// test#4
// sound
/////////////////////////////////////////////////////////////   
   delay(1000);
   clearBuffer();
   drawStrC(10, "Test #4: sound");
   drawBigStrC(24,"PUSH");
   sendBuffer();

   while(gpio_get_level(PUSH) == 1) delay(100);    
   audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
   i2s_set_clk((i2s_port_t)1, 44100, (i2s_bits_per_sample_t)16, (i2s_channel_t)2);   
   if(!SPIFFS.begin())Serial.println("Erreur SPIFFS");
   //SPIFFS.format();
   File root = SPIFFS.open("/");
   audio.connecttoFS(SPIFFS, "/music.mp3");
   audio.setVolume(21); 
   clearBuffer();   
   drawStrC(10, "Test #4");
   drawBigStrC(24,"Music!..");
   sendBuffer();
   stop = millis() + 12000; 
}



uint64_t t0, t1;
uint32_t dt;                           
void read_data_stream(const uint8_t *data, uint32_t length)
{
      float V;
      V = voltb[vol];
      V = V / volMAX;
      int16_t *ptr16 = (int16_t*) data; 
      uint16_t len16 = length/2;
      for(int j=0;j<len16;j++){
           ptr16[j] = V * ptr16[j];
      }
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  toDisplay = true;
  if(id == 1) strcpy(title, (char*)text);
  if(id == 2) strcpy(artist, (char*)text);
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
  connection_state = state;
  toDisplay = true;
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  Serial.println(a2dp_sink.to_str(state));  
  audio_state = state;
  toDisplay = true;
}


void encoder(void* data)
{ 
  int va, vb;
  int32_t ta = 0;
  int32_t tb = 0;
  uint32_t to = 0;
  uint32_t dt;
  int v;
#define dtMax 30  
  while(true)
  { 
////////////////////////////////////////////     
// Push
//   <500 ms ==> P = 1
//   >500 ms ==> P = 2   
////////////////////////////////////////////  
    v = gpio_get_level(PUSH);
    if(to == 0)
    {
      if(v == 0) to = millis();
    }
    else
    {
      if(v == 1)
      {
        dt = millis() - to;
        if(dt < 100) P = 0;
        else if((millis() - to) < 500) P = 1 ;
        else  P = 2;
        to = 0;
      }
    }
/////////////////////////////////////////////////
// rotactor
/////////////////////////////////////////////////    
    va = gpio_get_level(ROTARY_A);
    vb = gpio_get_level(ROTARY_B);
    if((va == 1) && (ta == -1))ta = 0;
    if((vb == 1) && (tb == -1))tb = 0;
    if((va == 0) && (ta == 0)) ta = millis();
    if((vb == 0) && (tb == 0)) tb = millis();
    if((ta > 0) && (tb > 0))
    {
      dt = ta - tb;
      
  //    if(ta > tb) N++; else N--;
      if((ta > tb) && (dt < dtMax)) N++; else N--;
      if((ta < tb) && (dt > -dtMax))N--; else N++;
      
      ta=tb=-1;
    }
    delay(5);
  }
}

void setup() {
  Serial.begin(115200); 
///////////////////////////////////////////
//Building an unique BT name 
/////////////////////////////////////////// 
    esp_read_mac(mac, ESP_MAC_BT);
    sprintf(BTName, "MNCast-%x%x%x%x%x%x", mac[0],mac[1],mac[2],mac[3], mac[4], mac[5]);
    Serial.println(BTName);

    static const i2s_pin_config_t pin_config =
    {
      .bck_io_num = 5,
      .ws_io_num = 25,
      .data_out_num = 26,
      .data_in_num = I2S_PIN_NO_CHANGE 
    };

///////////////////////////////////////////////////////////////
//init rotactor
///////////////////////////////////////////////////////////////

  gpio_reset_pin(PUSH);
  gpio_set_direction(PUSH, GPIO_MODE_INPUT); 
  gpio_set_pull_mode(PUSH, GPIO_PULLUP_ONLY); 

  gpio_reset_pin(ROTARY_A);
  gpio_set_direction(ROTARY_A, GPIO_MODE_INPUT); 
  gpio_set_pull_mode(ROTARY_A, GPIO_PULLUP_ONLY); 

  gpio_reset_pin(ROTARY_B);
  gpio_set_direction(ROTARY_B, GPIO_MODE_INPUT);   
  gpio_set_pull_mode(ROTARY_B, GPIO_PULLUP_ONLY); 

  
//////////////////////////////////////////////////////////////  
// init tiny screen handler
///////////////////////////////////////////////////////////////
   tinySsd_init(SDAA, SCLL, 0, 0x3C, 1);
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////
// init encoder handler as a core#0 task
/////////////////////////////////////
   xTaskCreatePinnedToCore(encoder, "encoder", 5000, NULL, 5, &Tencoder, 0);


//////////////////////////////////////////////////////////////

char mes[] = "apps.raspiaudio.com";
char mesA[17];
  for(int i=0;i<2;i++)
  {
   for(int j=0;j<19;j++)
   {
   clearBuffer();
   drawStrC(14,"Check");
   drawStrC(30, "our Apps Store");
   strncpy(mesA, &mes[j], 16);
   drawStr(55, 0, mesA);
   sendBuffer();
   if(j == 0)delay(1000); else delay(200);         
   }
   delay(300);
  }

   
   clearBuffer();
   drawBigStrC(16,"Manga");
   drawBigStrC(40,"Cast");   
   sendBuffer();
   delay(2000);

//////////////////////////////////////////////////////////////
// Factory test
//////////////////////////////////////////////////////////////
    if( P > 0)
    {
    P = 0;
 //   a2dp_sink.end(true); 
   clearBuffer();
   drawBigStrC(20,"Wait..."); 
   sendBuffer();    
    delay(2000);
    Napp = Ntst;
    test();
    }
///////////////////////////////////////////////////////////////
// bluetooth player
///////////////////////////////////////////////////////////////
   else
   {
    clearBuffer();
    drawStrC(15, "Connect to");
    drawStrC(35,BTName);
    sendBuffer(); 
    Napp = Nbt;
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_stream_reader(read_data_stream);
    a2dp_sink.set_on_connection_state_changed(connection_state_changed); 
    a2dp_sink.set_on_audio_state_changed(audio_state_changed);   
    a2dp_sink.set_pin_config(pin_config); 
    a2dp_sink.set_auto_reconnect(true);   
    a2dp_sink.start(BTName, true);
}
}
void loop() {
if(Napp == Nbt)
{
///////////////////////////////////////////////
// mute / unmute
//////////////////////////////////////////////  
     if(P == 1)
     {
      P = 0;
      if(mute == false)
      {
        a2dp_sink.pause();
        mute = true;
      }
      else
      {
        a2dp_sink.play();
        mute = false;
      }
     }

///////////////////////////////////////////////
// forward
//////////////////////////////////////////////
    if(P == 2)
    {
      P = 0;
      a2dp_sink.next();
      mute = false;
      toDisplay = true;
    }
     
  
///////////////////////////////////////////
//volume
///////////////////////////////////////////  
     if((N - PN) != 0)
     {
     vol = vol + (N - PN); 
     if(vol > volMAX) vol = volMAX;
     if(vol < 0) vol = 0;
     Serial.println(N); Serial.println(PN); Serial.println(vol);       
     PN = N;
     toDisplay = true;
     }  

//////////////////////////////////////////
//display
/////////////////////////////////////////
       
     if(toDisplay == true)
     {   
      char c;
      clearBuffer();
      if(connection_state == Connected)
      {
          if(audio_state != Started)
          {
            clearBuffer();
            drawStrC(30, "waiting audio...");
            sendBuffer();
            toDisplay = false;
          } 
          else            
        {       
        drawStr(56,2,"C");    
      if(strlen(artist) <= 16) drawStrC(2, artist);
      else
      {
        artist[32] = 0;
        drawStr(12, 0, artist+16);
        c = artist[16];
        artist[16] = 0;
        drawStrC(2, artist);
        artist[16] = c;
      }
      
      drawHLine(18, 0,128);
      if(strlen(title) <= 16) drawStrC(34, title);
      else
      {
        title[32] = 0;
        drawStr(40, 0, title +16);
        c = title[16];
        title[16] = 0;
        drawStrC(30, title);
        title[16] = c;
      } 
      drawHLine(50, 0,128);          
      drawRectangle(56, 14, 6, 100*vol/volMAX);
      drawFrame(56, 14, 6,100);
      
      sendBuffer();
      toDisplay = false;      
       }
       }
    else
    {
    clearBuffer();
    drawStrC(15, "Connect to");
    drawStrC(35,BTName);
    sendBuffer(); 
    toDisplay = false;
    }
     }

     
}
 if(Napp == Ntst)
 {
    audio.loop();
    if(millis() > stop)
    {
      audio.stopSong();
      clearBuffer();
      drawStrC(10, "Test #4: sound");
      drawBigStrC(24,"OK");
      sendBuffer();  
      delay(5000);
      ESP.restart();
    }

 }
}
