#include <FastLED.h>



//pins de l'arduino sur lesquels sont branches les boutons
#define CL_G 5
#define CL_D 6
#define WARN 7
#define PHARE1 8
#define PHARE2 9
#define ECO_SW 16

#define BP_DELAY 100
//pins de l'arduino sur lesquels sont branches les voyants
#define LED_CL_G 10
#define LED_CL_D 11
#define LED_R 12
#define LED_B 13
#define LED_G 2
#define BAT_INDIC 15

#define PHARE 17

//pins de l'arduino sur lesquels sont branches les capteurs
#define REMORQUE 4
#define FREIN 0
#define BAT_LEV 14


//ruban led
#define DATA_PIN 3
//longueurs des segments
#define L_AVANT 20
#define L_MILIEU 12
#define L_ARRIERE 12
#define L_AILE_M 16
#define L_AILE_COTE 24
#define L_REM_M 16
#define L_REM_COTE 20

#define NUM_LEDS (L_AVANT+L_MILIEU+L_ARRIERE+L_AILE_M+L_AILE_COTE+L_REM_M+L_REM_COTE)*2

//definission des couleurs que l'on va utiliser
#define ROUGE CRGB(255,0,0)
#define BLEU CRGB(0,0,255)
#define ROUGE_LOW CRGB(96,0,0)
#define ORANGE CRGB(255,32,0)
#define BLANC CRGB(255,255,255)
#define BLANC_LOW CRGB(128,128,128)

//nombre de modes alternatifs/decoratifs
#define NB_FUN_MODE 2

CRGB leds[NUM_LEDS];
void init_IO();//défini les modes d'utilisations des Inputs/Ouptuts
void check_Input();//défini le mode de clignotants suivant les boutons
void phare();//controle le phare et le voyant de phare suivant la position du levier de controle
void clignotement();//gère l'alternance allumé/éteint des clignotants

void outLights(); //affiche les clignotants sur les leds
void feuxPos(); // affiche les feux de positions
void etoile(); //mode alternatif etoile
void police();//mode alternatif police

bool cmdWarnOn, cmdCL_G_On, cmdCL_D_On=cmdWarnOn=cmdCL_G_On = false;
bool CL_D_On, CL_G_On, WarnOn = CL_D_On = CL_G_On = false;
long int prevCL_G = millis(), prevCL_D = millis();
bool CL_G_PrevState, CL_D_PrevState = CL_G_PrevState = HIGH;
bool remorque, eco;
bool phareState;



bool funMode = false; //mode alternatif
int funModeNum = 0;


//définitions des numéros de leds suivant leur position sur le vélo
const unsigned char AvGauche[L_AVANT] = {51,32,50,33,49,34,48,35,47,36,46,37,45,38,44,39,43,40,42,41};
const unsigned char AvDroit[L_AVANT] = {12,31,13,30,14,29,15,28,16,27,17,26,18,25,19,24,20,23,21,22};
const unsigned char CoGauche[L_MILIEU] = {63,52,62,53,61,54,60,55,59,56,58,57};
const unsigned char CoDroit[L_MILIEU] = {0,11,1,10,2,9,3,8,4,7,5,6};
const unsigned char ArGauche[L_ARRIERE] = {70,69,71,68,72,67,73,66,74,65,75,64};
const unsigned char ArDroit[L_ARRIERE] = {81,82,80,83,79,84,78,85,77,86,76,87};
const unsigned char AiGauche[L_AILE_COTE] = {119,96,118,97,117,98,116,99,115,100,114,101,113,102,112,103,111,104,110,105,109,106,108,107};
const unsigned char AiDroit[L_AILE_COTE] = {136,159,137,158,138,157,139,156,140,155,141,154,142,153,143,152,144,151,145,150,146,149,147,148};
const unsigned char AiCentreGauche[L_AILE_M] = {127,88,126,89,125,90,124,91,123,92,122,93,121,94,120,95};
const unsigned char AiCentreDroit[L_AILE_M] = {128,167,129,166,130,165,131,164,132,163,133,162,134,161,135,160};
const unsigned char ReGauche[L_REM_COTE] = {184,203,185,202,186,201,187,200,188,199,189,198,190,197,191,196,192,195,193,194};
const unsigned char ReDroit[L_REM_COTE] = {223,204,222,205,221,206,220,207,219,208,218,209,217,210,216,211,215,212,214,213};
const unsigned char ReCentreGauche[L_REM_M] = {183,168,182,169,181,170,180,171,179,172,178,173,177,174,176,175};
const unsigned char ReCentreDroit[L_REM_M] = {224,239,225,238,226,237,227,236,228,235,229,234,230,233,231,232};

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  init_IO();
  Serial.begin(9600);//initialisation de la liaison série pour le déboggage

  //initialisation des variables d'etat du vélo
  remorque = false; 
  eco = false;
  phareState = false; //false == phare off
  FastLED.clear(); //extinction de toute les leds
  FastLED.show();
}


void loop() {
  int i;
  // put your main code here, to run repeatedly:
  if(eco) FastLED.setBrightness(96); else FastLED.setBrightness(192); //luminosité générale du ruban led
  check_Input();
  if(funMode){
    switch(funModeNum){
      case 0 :
        etoile();
        break;
      default:
        FastLED.setBrightness(128);
        police();
        break;
    }
  }
  else{
    FastLED.clear();
    phare();
    feuxPos();
    clignotement();
    outLights();
    FastLED.show();
  }
}


void etoile(){
  static uint8_t hue = 0;
  static uint8_t hueDelta = 5;
  int i;
  for(i=0; i<L_AVANT; i++){
    leds[AvGauche[i]] = leds[AvDroit[i]] = CHSV(hue,255,255);
    FastLED.show();
    hue += hueDelta;
  }
  for(i=0; i<L_MILIEU; i++){
    leds[CoGauche[i]] = leds[CoDroit[i]] = CHSV(hue,255,255);
    FastLED.show();
    hue += hueDelta;
  }
  for(i=0; i<L_ARRIERE; i++){
    leds[ArGauche[i]] = leds[ArDroit[i]] = CHSV(hue,255,255);
    FastLED.show();
    hue += hueDelta;
  }
  
  if(digitalRead(REMORQUE) == LOW){
    for(i=L_REM_M-1; i>=0; i--){
      leds[ReCentreGauche[i]] = leds[ReCentreDroit[i]] = CHSV(hue,255,255);
      FastLED.show();
      hue += hueDelta;
    }
    for(i=L_REM_COTE-1; i>=0; i--){
      leds[ReGauche[i]] = leds[ReDroit[i]] = CHSV(hue,255,255);
      FastLED.show();
      hue += hueDelta;
    }
  }
  else{
    for(i=L_AILE_M-1; i>=0; i--){
      leds[AiCentreGauche[i]] = leds[AiCentreDroit[i]] = CHSV(hue,255,255);
      FastLED.show();
      hue += hueDelta;
    }
    for(i=L_AILE_COTE-1; i>=0; i--){
      leds[AiGauche[i]] = leds[AiDroit[i]] = CHSV(hue,255,255);
      FastLED.show();
      hue += hueDelta;
    }
  }
}

void outLights(){
  int i;
  static bool CL_D_OnPrev = false;
  static bool CL_G_OnPrev = false;
  static int n = 1;

  EVERY_N_MILLISECONDS(18){
    n++;
    if(n>20)n=20;
  }
  if(CL_D_On && !CL_D_OnPrev || CL_G_On && !CL_G_OnPrev) n=1;
  CL_D_OnPrev = CL_D_On;
  CL_G_OnPrev = CL_G_On;
  if(CL_D_On){
    
    digitalWrite(LED_CL_D, HIGH);
    //lightStrip
    for(i = 0; i<L_AVANT; i++){
      if(i<n)
        leds[AvDroit[i]] = ORANGE;
    }
    for(i = 0; i<L_MILIEU; i++){
      if(i<n)
        leds[CoDroit[i]] = ORANGE;
    }
    for(i = 0; i<L_ARRIERE; i++){
      if(i<n)
        leds[ArDroit[i]] = ORANGE;
    }
    if(!remorque){
      if(phareState){
        for(i=0; i<L_AILE_COTE*3/4; i++){
          if(i<n)
            leds[AiDroit[i]] = ORANGE;
        }
      }
      else{
        for(i=0; i<L_AILE_COTE; i++){
          if(i<n)
            leds[AiDroit[i]] = ORANGE;
        }  
      }
    }
    else{
      for(i=0; i<L_REM_COTE; i++){
        if(i<n)
          leds[ReDroit[i]] = ORANGE;
      }
    }
    
  }
  else{
    digitalWrite(LED_CL_D, LOW);
    //lightStrip
  }
  if(CL_G_On){
    digitalWrite(LED_CL_G, HIGH);
    //lightStrip
    
    for(i = 0; i<L_AVANT; i++){
      if(i<n)
        leds[AvGauche[i]] = ORANGE;
    }
    for(i = 0; i<L_MILIEU; i++){
      if(i<n)
        leds[CoGauche[i]] = ORANGE;
    }
    for(i = 0; i<L_ARRIERE; i++){
      if(i<n)
        leds[ArGauche[i]] = ORANGE;
    }
    if(!remorque){
      if(phareState){
        for(i=0; i<L_AILE_COTE*3/4; i++){
          if(i<n)
            leds[AiGauche[i]] = ORANGE;
        }
      }
      else{
        for(i=0; i<L_AILE_COTE; i++){
          if(i<n)
            leds[AiGauche[i]] = ORANGE;
        }
      }
    }
    else{
      for(i=0; i<L_REM_COTE; i++){
        if(i<n)
          leds[ReGauche[i]] = ORANGE;
      }
    }
  }
  else{
    digitalWrite(LED_CL_G, LOW);
    //lightStrip
  }

  if(digitalRead(FREIN) == LOW){
    if(!remorque){
      for(i=0; i<L_AILE_M; i++){
        leds[AiCentreGauche[i]] = leds[AiCentreDroit[i]] = ROUGE;
      }
    }
    else{
      for(i=0; i<L_REM_M; i++){
        leds[ReCentreGauche[i]] = leds[ReCentreDroit[i]] = ROUGE;
      }
    }
  }

  
}
void clignotement(){

  if(!(cmdCL_D_On || cmdWarnOn)){
    CL_D_On = false;
  }
  if(!(cmdCL_G_On || cmdWarnOn)){
    CL_G_On = false;
  }
  if(!cmdWarnOn){
    WarnOn = false;
  }
  EVERY_N_MILLISECONDS(600){
    if(cmdCL_D_On ){
      CL_D_On = !CL_D_On;
    }
    if(cmdCL_G_On){
      CL_G_On = !CL_G_On;
    }
    if(cmdWarnOn){
      WarnOn = !WarnOn;
      CL_D_On = WarnOn;
      CL_G_On = WarnOn;
    }
  }
  Serial.println(WarnOn);
  
}


void check_Input(){

  if(digitalRead(CL_D) != CL_D_PrevState && prevCL_D < millis() - BP_DELAY){//changement d'etat sur BP
    CL_D_PrevState = digitalRead(CL_D);
    prevCL_D = millis();
    if(digitalRead(CL_D) == HIGH){ //bouton enfonce
      cmdCL_D_On = !cmdCL_D_On;
      if(cmdCL_G_On)cmdCL_G_On = false;
    }
  }
  if(digitalRead(CL_G) != CL_G_PrevState && prevCL_G < millis() - BP_DELAY){//changement d'etat sur BP
    CL_G_PrevState = digitalRead(CL_G);
    prevCL_G = millis();
    if(digitalRead(CL_G) == HIGH){ //bouton enfonce
      cmdCL_G_On = !cmdCL_G_On;
      if(cmdCL_D_On)cmdCL_D_On = false;
    }
  }
  if(digitalRead(CL_G) != digitalRead(CL_D)){
    delay(50);
    if(digitalRead(CL_G) != digitalRead(CL_D)){
      funMode = false;
    }
  }
  if(digitalRead(CL_G) == HIGH && digitalRead(CL_D) == HIGH){
    delay(100);
    if(digitalRead(CL_G) == HIGH && digitalRead(CL_D) == HIGH){
      funMode = true;
      funModeNum = (funModeNum+1)%NB_FUN_MODE;
      delay(100);
    }
  }
  cmdWarnOn = digitalRead(WARN);

  EVERY_N_SECONDS(5){
    int remorqueMes = 0;
    int i;
    for(i=0;i<10; i++){
      if(!digitalRead(REMORQUE))
        remorqueMes++;
      delay(5);
    }
    if(remorqueMes>5){
      remorque = true;
    }
    else{
      remorque = false;
    }
  }
  eco = !digitalRead(ECO_SW);

  if(analogRead(BAT_LEV) < 1023 && !eco){
    EVERY_N_MILLISECONDS(200){
      digitalWrite(BAT_INDIC, !digitalRead(BAT_INDIC));
    }
  }
  if(eco) digitalWrite(BAT_INDIC, HIGH);
  
}


void phare(){
  int i;
  if(digitalRead(PHARE1)==HIGH && digitalRead(PHARE2)==LOW){//eteint
    phareState = false;
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);

    digitalWrite(PHARE,LOW);
  }
  else if(digitalRead(PHARE1)==HIGH && digitalRead(PHARE2)==HIGH){//croisement
    phareState = true;
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);

    for(i=0; i<L_AVANT; i++){
      leds[AvGauche[i]] = leds[AvDroit[i]]= BLANC_LOW;
    }

    digitalWrite(PHARE,LOW);
  }
  else if(digitalRead(PHARE1)==LOW && digitalRead(PHARE2)==HIGH){//route
    phareState = true;
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);

    for(i=0; i<L_AVANT; i++){
      leds[AvGauche[i]] = leds[AvDroit[i]]= BLANC_LOW;
    }

    digitalWrite(PHARE,HIGH);
  }
}


void feuxPos(){
  int i;
  if(phareState){
    if(!remorque){
      if(!(cmdCL_G_On || cmdWarnOn)){
        for(i=0;i<L_ARRIERE; i++){

          leds[ArGauche[i]] = ROUGE_LOW;
        }
      }
      if(!(cmdCL_D_On || cmdWarnOn)){
        for(i=0;i<L_ARRIERE; i++){

          leds[ArDroit[i]] = ROUGE_LOW;
        }
      }
        for(i=0;i<L_AILE_COTE/2; i++){
            leds[AiDroit[i+L_AILE_COTE/2]] = leds[AiGauche[i+L_AILE_COTE/2]] = ROUGE_LOW;
        }
      for(i=0;i<L_AILE_M/4; i++){
          leds[AiCentreDroit[i]] = leds[AiCentreGauche[i]] = ROUGE_LOW;
      }
    }
    else{

      if(!(cmdCL_D_On || cmdCL_G_On || cmdWarnOn)){
        for(i=0;i<L_REM_COTE; i++){
          if(!(i%2)){
            leds[ReGauche[i]] = leds[ReDroit[i]] = ROUGE_LOW;
          }
        }
      }
      for(i=0;i<L_REM_M; i++){
        if(i%2){
          leds[ReCentreGauche[i]] = leds[ReCentreDroit[i]] = ROUGE_LOW;
        }
      }
    }
  }
}


void init_IO(){
  pinMode(CL_G, INPUT_PULLUP);
  pinMode(CL_D, INPUT_PULLUP);
  pinMode(WARN, INPUT_PULLUP);
  pinMode(PHARE1, INPUT_PULLUP);
  pinMode(PHARE2, INPUT_PULLUP);
  pinMode(FREIN, INPUT_PULLUP);
  pinMode(REMORQUE, INPUT_PULLUP);
  pinMode(ECO_SW, INPUT_PULLUP);
  pinMode(BAT_LEV, INPUT);

  pinMode(BAT_INDIC, OUTPUT);
  pinMode(PHARE, OUTPUT);
  pinMode(LED_CL_G, OUTPUT);
  pinMode(LED_CL_D, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}


void police(){
  int i;
  static bool alt = false;
  FastLED.clear();
  EVERY_N_MILLISECONDS(400){
    alt = !alt;
  }
  for(i=0; i<L_AVANT; i++){
    if(alt ^ (i>L_AVANT/2))
      leds[AvGauche[i]] = leds[AvDroit[i]] = ROUGE;
    else
      leds[AvGauche[i]] = leds[AvDroit[i]] = BLEU;
  }
  for(i=0; i<L_MILIEU; i++){
    if(alt ^ (i>L_MILIEU/2))
      leds[CoGauche[i]] = leds[CoDroit[i]] = ROUGE;
    else
      leds[CoGauche[i]] = leds[CoDroit[i]] = BLEU;
  }
  for(i=0; i<L_ARRIERE; i++){
    if(alt ^ (i>L_ARRIERE/2))
      leds[ArGauche[i]] = leds[ArDroit[i]] = ROUGE;
    else
      leds[ArGauche[i]] = leds[ArDroit[i]] = BLEU;
  }
  
  if(digitalRead(REMORQUE) == LOW){
    for(i=L_REM_M-1; i>=0; i--){
      if(alt ^ (i>L_REM_M/2))
        leds[ReCentreGauche[i]] = leds[ReCentreDroit[i]] = ROUGE;
      else
        leds[ReCentreGauche[i]] = leds[ReCentreDroit[i]] = BLEU;
    }
    for(i=L_REM_COTE-1; i>=0; i--){
      if(alt ^ (i>L_REM_M/2))
        leds[ReGauche[i]] = leds[ReDroit[i]] = ROUGE;
      else
        leds[ReGauche[i]] = leds[ReDroit[i]] = BLEU;
    }
  }
  else{
    for(i=L_AILE_M-1; i>=0; i--){
      if(alt ^ (i>L_AILE_M/2))
        leds[AiCentreGauche[i]] = leds[AiCentreDroit[i]] = ROUGE;
      else
        leds[AiCentreGauche[i]] = leds[AiCentreDroit[i]] = BLEU;

    }
    for(i=L_AILE_COTE-1; i>=0; i--){
      if(alt ^ (i>L_AILE_COTE/2))
        leds[AiGauche[i]] = leds[AiDroit[i]] = ROUGE;
      else
        leds[AiGauche[i]] = leds[AiDroit[i]] = BLEU;
    }
  }
  FastLED.show();
}
