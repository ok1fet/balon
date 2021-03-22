/* FINALNI VERZE
   Simple Arduino Morse Beacon
   Written by Mark VandeWettering K6HX  http://brainwagon.org
   Modified OK1FET 2015/06/12 doplnen:
   pin (15) D9 NF vystup 
   pin (16) D10 PTT urcene pro napajeni vysilace na 433MHz 
   pin (18) D12 je pripojeno cidlo DS18B20 odpor 4K7 na plus
   2015/09/05 doplneno mereni napeti
   2015/09/20 zmena zpusobu mereni teploty pomoci knihovny Dallas
   2015/09/20 nastaven SLEEP mod pro setreni baterek
   FIO deska
*/

// knihovny pro teplotni cidlo
#include <OneWire.h>
#include <DallasTemperature.h>
// knihovny pro sleep mod
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
// zde se bude ukládat zda přišel impuls z watchdog timeru
// hodnota 1 simuluje impuls po zapnutí, aby jsme nečekali
volatile int impuls_z_wdt=1;
// zde se ukládají impulsy
volatile int citac_impulsu= 2;// 2*8s = 16 sekund spanek
// zde nastavíme potřebný počet impulsů
// podle nastavení WDT viz níže je jeden impuls 8 sekund
volatile int impulsu_ke_spusteni = 2;// 2*8s = 16 sekund spanek
 
float memV ;
float temperature;
float StartTemperature;
float v;
  
//////// impuls z WATCHDOG TIMERU /////////////////
ISR(WDT_vect) 
{
  //když je proměnná impuls_z_wdt na 0 
  if(impuls_z_wdt == 0) 
  {
    // zapiš do proměnné 1
    impuls_z_wdt=1;
  }
}
/////POZOR UPRAVENO . R ; 9 N /////////////////////////////////////////
struct t_mtab {  char c, pat; } ;

struct t_mtab morsetab[] = {

  {    '+', 42  }  ,
  {    '=', 49  }  ,  
  {    '-', 97  }  ,
  {    '.', 10  }  ,
  {    ',', 115 }  ,
  {    '?', 76  }  ,
  {    '/', 41  }  ,
  {    'A', 6   }  ,
  {    'B', 17  }  ,
  {    'C', 21  }  ,
  {    'D', 9   }  ,
  {    'E', 2   }  ,
  {    'F', 20  }  ,
  {    'G', 11  }  ,
  {    'H', 16  }  ,
  {    'I', 4   }  ,
  {    'J', 30  }  ,
  {    'K', 13  }  ,
  {    'L', 18  }  ,
  {    'M', 7   }  ,
  {    'N', 5   }  ,
  {    'O', 15  }  ,
  {    'P', 22  }  ,
  {    'Q', 27  }  ,
  {    'R', 10  }  ,
  {    'S', 8   }  ,
  {    'T', 3   }  ,
  {    'U', 12  }  ,
  {    'V', 24  }  ,
  {    'W', 14  }  ,
  {    'X', 25  }  ,
  {    'Y', 29  }  ,
  {    'Z', 19  }  ,
  {    '1', 62  }  ,
  {    '2', 60  }  ,
  {    '3', 56  }  ,
  {    '4', 48  }  ,
  {    '5', 32  }  ,
  {    '6', 33  }  ,
  {    '7', 35  }  ,
  {    '8', 39  }  ,
  {    '9',  5  }  ,
  {    '0', 63  }} ;
///////////////////////////////////////////////////

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))
#define SPEED  (12)
#define DOTLEN  (1200/SPEED)
#define DASHLEN  (3*(1200/SPEED))

int key = 13 ; 
int repro =  9;//A1;//9 ;
int ptt = 8;//A0; //10 ;
int DS18B20_Pin = 4; //čidlo je připojeno na pinu 12

OneWire DS(DS18B20_Pin);
DallasTemperature sensors( & DS );// Pass our oneWire reference to Dallas Temperature. oneWire musi byt male O

void sendmsg(char *str){
    while (*str)
    send(*str++) ;
 // Serial.println("");
}

void vario(){
   float va= (StartTemperature - temperature)*68;// teplota na zemi, zmerena, teplotni rozdil, nadmorska vyska
  //Serial.println(memV,0);
  //Serial.println(va,0);

  if((va-memV)>40 ){
    //Serial.println("stoupa");
    sendmsg("U") ;
      }
      if((va-memV)>10 ){
    //Serial.println("stoupa");
    sendmsg("U") ;
      }
      if((memV-va)>40 ){
    //Serial.println("klesa");
    sendmsg("D") ;
      }
   if((memV-va)>10 ){
    //Serial.println("klesa");
    sendmsg("D") ;
      }
   memV = va;
   delay(500);
}

long mereniT(){
  sensors.requestTemperatures(); 
  temperature = (sensors.getTempCByIndex(0));
//  Serial.println("Teplota");
//  Serial.print(temperature);
}

void vyska(){
  v= (StartTemperature - temperature)*68 ;// teplota na zemi, zmerena, teplotni rozdil, nadmorska vyska
  //v= (StartTemperature - temperature)*68 + 250;// teplota na zemi, zmerena, teplotni rozdil, nadmorska vyska
  float floatVal1 = v;
  
  char charVal1[10];    //temporarily holds data from vals 
  String stringVal1 = " ";     //data on buff is copied to this string
  dtostrf(floatVal1, 4, 0, charVal1);// pocet mist pred desetinou a za ni

  for(int i=0;i<4;i++)// pocet znaku ktere se budou vycitat jako chr
  {
//   Serial.println(charVal1[i]);
    send(charVal1[i]);
  }
}

void sendtemp() {
  float floatVal = temperature;
  //Serial .println(temperature);//
  //float floatVal = -10.16; //pro testovani
  char charVal[10];    //temporarily holds data from vals 
  String stringVal = " ";     //data on buff is copied to this string
  dtostrf(floatVal, 3, 1, charVal);// pocet mist pred desetinou a za ni

  for(int i=0;i<5;i++)// pocet znaku ktere se budou vycitat jako chr
  {                   // musi byt nastaveno 0-5 aby bylo mozne zpracova zapornou dvou cifernou hodnotu 
    //Serial.println(charVal[i]);//
    send(charVal[i]);
  }
}

void sendbatt(){
  float Battery_Vcc_V =readVcc()/1000.0;
 //  Serial.println(Battery_Vcc_V,1); 
 float floatVal = Battery_Vcc_V;
  char charVal[10];    //temporarily holds data from vals 
  String stringVal = " ";     //data on buff is copied to this string
  dtostrf(floatVal, 1, 1, charVal);// pocet mist pred desetinou a za ni

  for(int i=0;i<3;i++)// pocet znaku ktere se budou vycitat jako chr
  {
//    Serial.println(charVal[i]);
    send(charVal[i]);
  }
}

 
long readVcc(){                         // function to read battery value - still in developing phase
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(20); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void enterSleep(void){
  //nastavení nejúspornějšího módu
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // spánkový režim je povolený
  sleep_enable();  
  // spuštění režimu spánku
  sleep_mode();
  
  // tady bude program pokračovat když se probudí
  
  // spánek zakázán
  sleep_disable();   
  //znovu zapojení všech funkcí 
  power_all_enable();
 //  sensors.begin();// Start up the library 
}

void Ton5s(){
  tone(repro,1100,5000);
  digitalWrite(key, HIGH) ;
  delay(5000);
  digitalWrite(key, LOW) ;
  delay(500);
 }

void setup() {
  pinMode(key, OUTPUT) ;
  pinMode(ptt, OUTPUT);  
  pinMode(repro, OUTPUT); 
  sensors.begin();// Start up the library 
  Serial.begin(9600) ;
  Serial.println("Simple Arduino Morse Beacon k6hx@arrl.net modified ok1fet") ;

   // nastavení WATCHDOG TIMERU    
  MCUSR &= ~(1<<WDRF); // neřešte
  WDTCSR |= (1<<WDCE) | (1<<WDE); // neřešte
  
  // nastavení času impulsu
  WDTCSR = 1<<WDP0 | 1<<WDP3; // 8 sekund, WDTCSR = B0110 --> 1 sekunda
  WDTCSR |= _BV(WDIE); //neřešte

/////////////////////////////////////////////
  sensors.requestTemperatures(); 
  StartTemperature = (sensors.getTempCByIndex(0));
  Serial.println("StartTeplota");
  Serial.print(StartTemperature);
///////////////////////////////////////////////  
}

void loop() {
  //když je impuls z WATCHDOG TIMERU a zároveň i potřebný jejich počet
  if ((impuls_z_wdt == 1) & (impulsu_ke_spusteni == citac_impulsu))
  {
///////////////////////////////////////////////////////////////
  
//  sendTempP(); // i v mrazaku neklesla teplota po 20°C
//  sendmsg("CP ") ;
 
  mereniT(); 
  Serial.println(" ");
  digitalWrite(ptt, HIGH) ;
  Ton5s();
  
//  Serial.println("StartTeplota");
//  Serial.print(StartTemperature);
  
  vario();
  //sendmsg(" = ") ;
  //sendmsg("BALLOON BEACON ALT ") ;
  //sendmsg("ALT ") ;
  vyska ();
  sendmsg("M ") ;
  //sendmsg("TEMP ") ;
  sendtemp();
  sendmsg("C ") ;
 
  //sendmsg("BATT ") ;
  sendbatt();
  sendmsg("V +") ;
  
  digitalWrite(ptt, LOW) ;
  
  //////////////////////////////////////////////////////////////
    
    citac_impulsu = 0;// vynuluj čítač
    impuls_z_wdt = 0; // vynuluj impuls    
    enterSleep();// znovu do spánku
  }
  else
  {
    enterSleep();//znovu do spánku
  }
   citac_impulsu++; // inpuls se přičte i když nic neproběhlo 
   }
   /////////////////////////////////////////////////////////////


void dash(){
 tone(repro,1100,DASHLEN);
  digitalWrite(key, HIGH) ;
  delay(DASHLEN);
  digitalWrite(key, LOW) ;
  delay(DOTLEN) ;
}

void dit() {
  tone(repro,1100,DOTLEN);
  digitalWrite(key, HIGH) ;
  delay(DOTLEN);
  digitalWrite(key, LOW) ;
  delay(DOTLEN);
}

void send(char c){
  int i ;
  if (c == ' ') {
    Serial.print(c) ;
    delay(7*DOTLEN) ;
    return ;
  }
  for (i=0; i<N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat ;
      Serial.print(morsetab[i].c) ;

      while (p != 1) {
        if (p & 1)
          dash() ;
        else
          dit() ;
        p = p / 2 ;
      }
      delay(2*DOTLEN) ;
      return ;
    }
  }
  /* if we drop off the end, then we send a space */
  Serial.print("?") ;
}
