#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include <math.h>
#include "UART.h"
#include "ws2812b.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF  // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 00000000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void PIC_INIT();
void blink();
unsigned short Convertto16bit(char channel, unsigned char v);

//global vars
int button_val = 0;

void main() {
    //keep pic init, A4, B4, UART
    PIC_INIT();
    UART1_INIT();
    ws2812b_setup();
    
    char debugging[100];
    
    // declaring led array and individual variables
    wsColor allLEDs[5]; 
    wsColor led_0;
    led_0.r = 255;
    led_0.g = 0;
    led_0.b = 0;
//    struct wsColor led_1;
//    struct wsColor led_2;
//    struct wsColor led_3;
//    struct wsColor led_4;
    
    float hue = 0;
    float inc = 45;
    float sat = 1;
    float brightness = 0.1;
          
    while (1) {
        //add heartbeat
        blink(); //ensure that program is not held
        //use HSBtoRGB to generate the color
        //put all the structs into an array of structs to send
        allLEDs[0] = HSBtoRGB(fmod(hue, 360), sat, brightness); 
//        sprintf(debugging, "output: %x, %x, %x\r\n", allLEDs[0].r, allLEDs[0].g, allLEDs[0].b);
//        NU32_WriteUART1(debugging);
        allLEDs[1] = HSBtoRGB((fmod(hue+inc, 360)), sat, brightness);        
        allLEDs[2] = HSBtoRGB(fmod(hue+2*inc, 360), sat, brightness);        
        allLEDs[3] = HSBtoRGB(fmod(hue+3*inc, 360), sat, brightness);        
        allLEDs[4] = HSBtoRGB(fmod(hue+4*inc, 360), sat, brightness);        
//        send array to HSBtoRGB into ws2812b_setColor
        ws2812b_setColor(allLEDs, 5);
        hue += 10;
    }
}

void PIC_INIT() {
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    __builtin_enable_interrupts();

    //initialize pins and write to output
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
}

void blink() {
    LATAbits.LATA4 = 1;
    _CP0_SET_COUNT(0);
    while (_CP0_GET_COUNT() <  24000000/2/20) {
    }
    LATAbits.LATA4 = 0;
    _CP0_SET_COUNT(0);
    while (_CP0_GET_COUNT() <  24000000/2/20) {
    }
}



