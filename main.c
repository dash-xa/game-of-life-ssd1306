/****************************************************************/
/* Greg Whitmore                                                */
/* greg@gwdeveloper.net                                         */
/* www.gwdeveloper.net                                          */
/****************************************************************/
/* released under the "Use at your own risk" license            */
/* use it how you want, where you want and have fun             */
/* debugging the code.                                          */
/* MSP430G2553                                                  */
/****************************************************************/

#include <msp430.h>

#include <msp430f5529.h>
#include <stdio.h>
#include <math.h>
#include "Oled_SSD1306.h"
#include "icons.h"

// defines
#define RED BIT0
#define GREEN BIT6

// global variables
unsigned char buttonOn;
unsigned char buttonInvert;
unsigned char buttonContrast = 0x01;

// prototypes
void init_Clock(void);
void init_UCB0(void);
void init_Timer0_A0(void);
void init_GPIO(void);

// main function configures hardware and runs demo program
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // hold watchdog

    SSD1306PinSetup();
    init_GPIO();
    init_Clock();
    init_UCB0();
    init_Timer0_A0();

    // Clear any oscillator faults
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); // Clear XT2, XT1, DCO fault flags
        SFRIFG1 &= ~OFIFG;                         // Clear the oscillator fault interrupt flag
    } while (SFRIFG1 & OFIFG);                      // Test oscillator fault flag

    // Enable Watchdog Timer as interval timer sourced from ACLK
    WDTCTL = WDTPW | WDTTMSEL | WDTSSEL__ACLK | WDTIS_3; // WDT as interval timer, ACLK, ~32768 divider
    SFRIE1 |= WDTIE;                                   // Enable WDT interrupt


    __enable_interrupt();

//    WDTCTL = WDTPW + WDTTMSEL + WDTSSEL + WDTIS1;   // set watchdog as debouncing interval

    SSD1306Init();
    clearScreen();

    // loop to for visual of framerate
    int k;
    for (k=0;k<30;k++)
    {
        Fill_RAM_PAGE(1, 0xff);
        Fill_RAM_PAGE(3, 0xff);
        Fill_RAM_PAGE(5, 0xff);
        Fill_RAM_PAGE(7, 0xff);

        __delay_cycles(1000);

        clearScreen();

        Fill_RAM_PAGE(2, 0xff);
        Fill_RAM_PAGE(4, 0xff);
        Fill_RAM_PAGE(6, 0xff);

        clearScreen();

        __delay_cycles(1000);
    }

    // display brain icon
    imageDraw(brain, 0, 28);

    __delay_cycles(32000000);

    clearScreen();

    // display demo text with outline
    stringDraw(2, 30, "MSP430G2553");
    stringDraw(4, 14, "USCI OLED BOOSTER");
    stringDraw(6, 38, "43oh.com");

    // outline
    verticalLine(0,0,64);
    verticalLine(128, 0, 64);
    horizontalLine(1,126,0);
    horizontalLine(1,126,64);

    // loop in LPM3
    while (1)
    {
        LPM3;

        //__no_operation();

    }
}

// set clock to max
//void init_Clock(void)
//{
//    BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;
//
//    if (CALBC1_16MHZ != 0xFF)
//    {
//        __delay_cycles(100000);
//        DCOCTL = 0x00;
//        BCSCTL1 = CALBC1_16MHZ;
//        DCOCTL = CALDCO_16MHZ;
//    }
//
//    BCSCTL1 |= XT2OFF + DIVA_0;
//    BCSCTL3 = XT2S_0 + LFXT1S_0 + XCAP_1;
//}

void init_Clock(void) {
    // Set up the DCO to run at 16 MHz
    UCSCTL3 = SELREF__REFOCLK; // Set FLL reference to REFO
    UCSCTL4 = SELA__REFOCLK;   // Set ACLK to REFO

    __bis_SR_register(SCG0);   // Disable the FLL control loop
    UCSCTL0 = 0x0000;          // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;       // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_0 + 487;    // Set DCO Multiplier for 16MHz
                               // (N + 1) * FLLRef = Fdco
                               // (487 + 1) * 32768 = 16MHz
    __bic_SR_register(SCG0);   // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 16 MHz / 32,768 Hz = 500000 = MCLK cycles for DCO to settle
    __delay_cycles(500000);

    // Loop until XT1, XT2 & DCO fault flag is cleared
    do {
        // Clear XT2,XT1,DCO fault flags
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);
        // Clear fault flags
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG); // Test oscillator fault flag

    // Now that we're running at 16 MHz, adjust the rest of your clocks as needed
}


// gpio init buttons and leds
void init_GPIO(void)
{
    P1OUT |= BIT6;
    P1DIR |= BIT0 + BIT6;

    // LP S2 button
    P1OUT |= BIT3;
    P1REN = BIT3;
    P1IES = BIT3;
    P1IFG = 0;
    P1IE = BIT3;

    // OLED booster buttons
    P2OUT |= BIT3 + BIT4;
    P2REN = BIT3 + BIT4;
    P2IES = BIT3 + BIT4;
    P2IFG = 0;
    P2IE = BIT3 + BIT4;

}

void init_UCB0(void)
{
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_1 + UCSYNC;
    UCB0CTL1 = UCSSEL_2 + UCSWRST;
    UCB0BR0 = 32;
    UCB0CTL1 &= ~UCSWRST;
}

// timer0_a0 on 1s wakeup as activity indicator
void init_Timer0_A0(void)
{
    TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_4 + CCIE;
    TA0CCR0 = 32768;
    TA0CTL = TASSEL_1 + ID_0 + MC_1;
}

// toggle LEDs at 1s interval
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA0_isr(void)
{
    P1OUT ^= RED + GREEN;

    LPM3_EXIT;

    //__no_operation();
}

// S2 on Launchpad turns display on or off
//#pragma vector=PORT1_VECTOR
//__interrupt void port1_isr(void)
//{
//    P1IFG &= ~BIT3;         // clear P1.3 button flag
//    P1IE &= ~BIT3;          // clear P1.3 interrupt
//
//    IFG1 &= ~WDTIFG;
//    WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
//    IE1 |= WDTIE;
//
//    // do something here
//    if (buttonOn == 1)
//    {
//        buttonOn = 0;
//        Set_Display_On_Off(0);
//    }
//    else
//    {
//        buttonOn = 1;
//        Set_Display_On_Off(1);
//    }
//}

// left button inverts display; right button adjusts contrast
//#pragma vector=PORT2_VECTOR
//__interrupt void port2_isr(void)
//{
//    if (P2IFG & BIT3)
//    {
//        P2IFG &= ~(BIT3 + BIT4);            // clear P2 button flag
//        P2IE &= ~(BIT3 + BIT4);         // clear P2 interrupt
//
//        IFG1 &= ~WDTIFG;
//        WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
//        IE1 |= WDTIE;
//
//        // do something here
//        if (buttonContrast < SSD1306_MAXCONTRAST)
//        {
//            buttonContrast += 25;
//            Set_Contrast_Control(buttonContrast);
//        }
//        else
//        {
//            buttonContrast = 0x01;
//            Set_Contrast_Control(buttonContrast);
//        }
//
//        //__no_operation();
//
//    }
//
//    if (P2IFG & BIT4)
//    {
//        P2IFG &= ~(BIT3 + BIT4);            // clear P2 button flag
//        P2IE &= ~(BIT3 + BIT4);         // clear P2 interrupt
//
//        IFG1 &= ~WDTIFG;
//        WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
//        IE1 |= WDTIE;
//
//        // do something here
//        if (buttonInvert == 1)
//        {
//            buttonInvert = 0;
//            Set_Inverse_Display(0);
//        }
//        else
//        {
//            buttonInvert = 1;
//            Set_Inverse_Display(1);
//        }
//
//        //__no_operation();
//
//    }
//
//}

// wdt timer is used for debouncing
#pragma vector=WDT_VECTOR
__interrupt void watchdog_isr(void)
{
//    IE1 &= ~WDTIE;
    // Enable Watchdog Timer interrupt
    SFRIE1 |= WDTIE;


    P1IFG &= ~BIT3;         // clear P1.3 button flag
    P1IE |= BIT3;           // re-enable P1.3 interrupt

    P2IFG &= ~(BIT3 + BIT4);
    P2IE |= BIT3 + BIT4;
}
