#include "M5Atom.h"
#include "AtomSPK.h"
#include "Ticker.h"
#include <cppQueue.h>

ATOMSPK atomSPK;
Ticker ticker;
cppQueue q(sizeof(float), 30, FIFO);

/*--- Sensor ---*/
int pin = 32;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;  // sampele 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

/*--- Ticker ---*/
const float period = 60 * 1;  // Speak once per 1 minutes

/*--- Interruption Processing ---*/
void speak(){
    float TmpData[30];
    
    while(q.isEmpty() == false){
        int idx = 0;
        float popTmpData;
        q.pop(&popTmpData);
        TmpData[idx] = popTmpData;
        idx ++;
    }

    if(sizeof(TmpData) / sizeof(float) > 0){
        for(int i = 0; i < sizeof(TmpData) / sizeof(float); i ++){
            float pushTmpData = TmpData[i]
            atomSPK.playBeep(pushTmpData, 200, 10000, false);
            q.push(&pushTmpData);        
        }
    }

    /* Only simple beep sounds
    if(concentration >= 3000){  // Warning
        for(int n = 0; n < 5; n ++){
            atomSPK.playBeep(2000, 200, 10000, false);
            atomSPK.playBeep(1000, 200, 10000, false);
        }
    }else if(3000 > concentration && concentration > 1000){
        for(int n = 0; n < 2; n ++){  // Caution
            atomSPK.playBeep(2000, 200, 10000, false);
            atomSPK.playBeep(1000, 200, 10000, false);
        }
    }else if(1000 >= concentration){  // Notice
        atomSPK.playBeep(1000, 200, 10000, false);
        atomSPK.playBeep(2000, 200, 10000, false);
    }
    */
}

/*--- Main Program ---*/
void setup(){
    /* ATOM Lite */
    M5.begin(true, false, true);

    /* Debug */
    Serial.begin(9600);

    /* Sensor */
    pinMode(pin, INPUT);
    starttime = millis();  // get the current time;

    /* Speaker */
    atomSPK.begin();

    /* Ticker */
    ticker.attach(period, speak);
}

void loop(){
    /* Sensing */
    duration = pulseIn(pin, LOW);
    lowpulseoccupancy = lowpulseoccupancy + duration;

    if((millis() - starttime) > sampletime_ms){  //if the sample time == 30s
        ratio = lowpulseoccupancy / (sampletime_ms * 10.0);  // Integer percentage 0=>100
        concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;  // using spec sheet curve
        
        float popData;
        if(q.isFull()){
            q.pop(&popData);
        }
        q.push(&concentration)

        /* Debug */
        //Serial.print(lowpulseoccupancy);
        //Serial.print(",");
        //Serial.print(ratio);
        //Serial.print(",");
        Serial.print("push: ");
        Serial.print(concentration);
        Serial.print(", pop: ");
        Serial.println(popData);
        
        /* Display */
        if(concentration >= 3000){
            M5.dis.drawpix(0, 0x00ff00);  // Red
        }else if(3000 > concentration && concentration > 1000){
            M5.dis.drawpix(0, 0xffff00);  // Yellow
        }else if(1000 >= concentration){
            M5.dis.drawpix(0, 0xff0000);  // Green
        }

        lowpulseoccupancy = 0;
        starttime = millis();
    }
}
