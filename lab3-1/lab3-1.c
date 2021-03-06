/*
Names: Ranjit Marathay, Jackie Sortor, Morgan Doyle
Section: 1B
Lab: Lab 3-1
Description: Smart car steering and motor speed
*/

#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init();
void Steering_Servo(void);
void Motor_Control(void);
void PCA_ISR ( void ) __interrupt 9;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned int PW_CENTER = 2769;
unsigned int PW_MIN_s = 1661;
unsigned int PW_MAX_s = 3877;
unsigned int PW_MIN_m = 2027;
unsigned int PW_MAX_m = 3504;
unsigned int PW_s;
unsigned int PW_m;
unsigned int flag = 0;
unsigned int counter = 0;

//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void){
    // initialize board
    Sys_Init();
    putchar(' '); //the quotes in this line may not format correctly
    Port_Init();
    XBR0_Init();
    PCA_Init();

    //print beginning message
    printf("Embedded Control Steering Calibration\n");
    // set the PCA output to a neutral setting
    PW_s = PW_CENTER;
    PW_m = PW_CENTER;
    PCA0CP0 = 0xFFFF - PW_m;
	  PCA0CP0 = 0xFFFF - PW_s;


    while(1){
      Steering_Servo();
      Motor_Control();
    }
}

//
// Set up ports for input and output
//
void Port_Init(){
  //P1MDOUT |= 0x04;  //set output pin for CEX0 in push-pull mode
  //P1MDOUT |= 0x10;  //set output pin for CEX2 in push-pull mode
  P1MDOUT |= 0x14;  //set output pin for CEX2 in push-pull mode
}

//
// Set up the crossbar
//
void XBR0_Init(){
  XBR0 = 0x27;  //configure crossbar as directed in the laboratory
  //Pin 12
}

//
// Set up Programmable Counter Array
//
void PCA_Init(void){
  PCA0CN |= 0x40;    /* Enable PCA counter */
  PCA0MD = 0x81;     /* SYSCLK/12, enable CF interrupts, suspend when idle */
  PCA0CPM0 = 0xC2;   /* CCM0 in 16-bit compare mode */
  PCA0CPM2 = 0XC2;
  EIE1 |= 0x08;
  EA = 1;
}

// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR ( void ) __interrupt 9{
    // reference to the sample code in Example 4.5 -Pulse Width Modulation
    // implemented using the PCA (Programmable Counter Array), p. 50 in Lab Manual.
    if(CF){
      CF = 0;
      PCA0 = 28614; // CHECK THIS
    }
    counter++;
    //PCA0CN &= 0x40;
    PCA0CN &= 0xC0; //Handle other PCA interrupt sources
}

void Steering_Servo(){
    char input;
    //wait for a key to be pressed
    input = getchar();
    if(input == 'r')  // single character input to increase the pulsewidth
    {
        PW_s = PW_s + 10;
        if(PW_s > PW_MAX_s){ // check if less than pulsewidth minimum
          PW_s = PW_MAX_s;    // set SERVO_PW to a minimum value
        }
    }
    else if(input == 'l')  // single character input to decrease the pulsewidth
    {
        PW_s = PW_s - 10;
        if(PW_s < PW_MIN_s){ // check if pulsewidth maximum exceeded
          PW_s = PW_MIN_s;     // set PW to a maximum value
        }
    }
    printf("\rServo PW: %u\n", PW_s);
    PCA0CP0 = 0xFFFF - PW_s;
}


void Motor_Control(){
  char input;
  input = getchar();	//input value
  if(input == 'f'){		//f is faster
    if(PW_m < PW_MIN_m){
      PW_m = PW_MIN_m;
    }
    else if(PW_m > PW_MAX_m){		//PW will not exceed max
      PW_m = PW_MAX_m;
    }
    else{
      if(PW_m == PW_MAX_m){
        PW_m = PW_MAX_m;
      }
      else{
        PW_m = PW_m + 20;		//if PW less than max add 20
      }
    }
  }
  else if(input == 's'){	//s is slower
    PW_m = PW_m - 20;
    if(PW_m > PW_MAX_m){
      PW_m = PW_MIN_m;
    }
    else if(PW_m < PW_MIN_m){
      PW_m = PW_MIN_m;
    }
  }
  printf("\rMotor PW: %u\n", PW_m);	//print PW
  PCA0CP2 = 0xFFFF - PW_m;
}
