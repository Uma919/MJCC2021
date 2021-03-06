#include "M5Atom.h"
#include "AtomSPK.h"
#include "cppQueue.h"

ATOMSPK atomSPK;
cppQueue que(sizeof(float), 30, FIFO);

/*--- Sensor ---*/
int pin = 32;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;  // sampele 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = -1;

/*--- Sound Processing ---*/
void playSound(){
    float TmpData[30];
    for(int i = 0; i < sizeof(TmpData) / sizeof(float); i ++) TmpData[i] = -1;
    
    /* Extract Sensor Data */
    int idx = 0;
    while(que.isEmpty() == false){
        float popTmpData;
        que.pop(&popTmpData);
        TmpData[idx] = popTmpData;
        idx ++;
        if((sizeof(TmpData) / sizeof(float)) == idx) break;
    }
    
    /* Play Sound */
    if(TmpData[0] !=  -1){
        for(int i = 0; i < sizeof(TmpData) / sizeof(float); i ++){
            if(TmpData[i] !=  -1){
                float pushTmpData = TmpData[i];
                float dt = constrain(pushTmpData, 0, 3000);
                float f_map = map(dt, 0, 3000,  500, 3000);
                float v_map = map(dt, 0, 3000, 5000, 1500);
                que.push(&pushTmpData);
                atomSPK.playBeep(int(f_map), 100, int(v_map), false);  // Play Sound
                delay(100);
            }
        }
        M5.dis.drawpix(0, 0xffffff);  // White Color LED
        delay(100);
    }
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
}

void loop(){
    /* Sensor */
    duration = pulseIn(pin, LOW);
    lowpulseoccupancy = lowpulseoccupancy + duration;

    if((millis() - starttime) > sampletime_ms){  //if the sample time == 30s
        ratio = lowpulseoccupancy / (sampletime_ms * 10.0);  // Integer percentage 0=>100
        concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;  // using spec sheet curve
        
        float popData;
        if(que.isFull()){
            que.pop(&popData);
        }
        que.push(&concentration);

        /* Debug */
        //Serial.print("push: ");
        //Serial.print(concentration);
        //Serial.print(", pop: ");
        //Serial.println(popData);
        
        lowpulseoccupancy = 0;
        starttime = millis();
    }
 
    /* Speaker */
    playSound();
    
    /* RGB LED */
    if(concentration >= 2000){
        M5.dis.drawpix(0, 0x00ff00);  // Red Color LED
    }else if(2000 > concentration && concentration > 1000){
        M5.dis.drawpix(0, 0xffff00);  // Yellow Color LED
    }else if(1000 >= concentration && concentration >= 0){
        M5.dis.drawpix(0, 0xff0000);  // Green Color LED
    }
}
