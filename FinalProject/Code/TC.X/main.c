#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include <math.h>

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
void init_Timer3();
void init_Timer2();

void main() {
    //keep pic init, A4, B4, UART
    PIC_INIT();
    init_Timer3();
    init_Timer2();

    while (1) {
        //                LATBbits.LATB12 = 0;
        //                LATBbits.LATB13 = 1;
        //                LATBbits.LATB14 = 0;
        //                LATBbits.LATB15 = 1;
        OC1RS = 1999 / 2; //1999 is max
        LATBbits.LATB14 = 1;
        OC4RS = 1999 / 2; //1999 is max
        LATBbits.LATB12 = 1;
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

    TRISBbits.TRISB12 = 0;
    LATBbits.LATB12 = 0;
    TRISBbits.TRISB13 = 0;
    LATBbits.LATB13 = 0;
    TRISBbits.TRISB14 = 0;
    LATBbits.LATB14 = 0;
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 0;

    RPB15Rbits.RPB15R = 0b0101; //set B15 pin to correspond with OC1
    RPB13Rbits.RPB13R = 0b0101; //set B13 pin to correspond with OC4

}

void init_Timer3() {
    T3CONbits.TCKPS = 1; //Timer 3 Prescaler N = 2
    OC1CONbits.OCTSEL = 1; //set oc1 to use timer 3
    PR3 = 1999; //setting up to run 20kHz
    TMR3 = 0; //initialize TMR3 count to 0
    OC1CONbits.OCM = 0b110; //PWM mode without fault pin
    OC1RS = 0; //Duty Cycle = OC1RS/(PR3 + 1) = 75%
    OC1R = 0; //initialize before turning OC1 on
    T3CONbits.ON = 1;
    OC1CONbits.ON = 1;
}

void init_Timer2() {
    T2CONbits.TCKPS = 1; //Timer 2 Prescaler N = 2
    OC4CONbits.OCTSEL = 0; //set oc4 to use timer 2
    PR2 = 1999; //setting up to run 20kHz
    TMR2 = 0; //initialize TMR2 count to 0
    OC4CONbits.OCM = 0b110; //PWM mode without fault pin
    OC4RS = 0; //Duty Cycle = OC4RS/(PR2 + 1) = 75%
    OC4R = 0; //initialize before turning OC4 on
    T2CONbits.ON = 1;
    OC4CONbits.ON = 1;
}