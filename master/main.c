/****************************************************************
************				INCLUDE					*************
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"

/****************************************************************
************				DEFINE					*************
*****************************************************************/
//16Mhz Speed
#define P1_MOVE (*((volatile unsigned long *)0x40024040))	// PE4
#define P2_MOVE (*((volatile unsigned long *)0x40024080))	// PE5
#define ONE_SEC 16000000	//One Second
#define DEBOUNCE 1600000 //100 ms
#define CELEBRATION_TIME 12	//celebration time

//Players
struct Player {
	unsigned int isReady, caught, winner;
};
struct Player p1 = {0,26,0};
struct Player p2 = {0,26,0};

//Functions
void Update_Screen(void);
void GPIOPortE_Handler(void);
void SysTick_Handler(void);
void Game_Init(void);
void PortE_Init(void);
void SysTick_Init(unsigned long delay);
// Interrupt definitions
void EnableInterrupts(void);    // Enable interrupts
void DisableInterrupts(void);   // Disable interrupts
void WaitForInterrupt(void);    // Wait for interrupt
//Game States
enum state_t { PREGAME, STARTING, GAME, POSTGAME };
// Global Variables
unsigned long lastClick[2];
unsigned int count;
unsigned int flag;
unsigned int countdown;
enum state_t state;

/****************************************************************
*************				INITIALIZE				*************
*****************************************************************/
//Initialize SysTick
void SysTick_Init(unsigned long delay) {
    NVIC_ST_CTRL_R = 0;             // disable SysTick during setup
    NVIC_ST_RELOAD_R = delay - 1;   // reload value
    NVIC_ST_CURRENT_R = 0;          // any write to current clears it
    NVIC_SYS_PRI3_R &= 0x00FFFFFF;  // Clear priority bits
    NVIC_SYS_PRI3_R |= 0x40000000;  // priority 2
    NVIC_ST_CTRL_R = 0x07;          // enable SysTick with processor clock
}

//Initialize Port E
//Pin 0-1 Joystick buttons
//Pin 4-5 Slave interrupts
void PortE_Init(void) {
	//PE0 Player1 PE1 Player2
	SYSCTL_RCGC2_R |= 0x00000010;           // activate clock for Port E
	while((SYSCTL_PRGPIO_R & 0x10) == 0);   // ready?
	GPIO_PORTE_LOCK_R = 0x4C4F434B;         // unlock GPIO Port E
	GPIO_PORTE_CR_R |= 0x03;                // allow changes to PE1-0
	GPIO_PORTE_AMSEL_R &= ~0x03;            // disable analog functions
	GPIO_PORTE_PCTL_R &= ~0x000000FF;       // configure PE1-0 as GPIO
	GPIO_PORTE_DIR_R &= ~0x03;              // PE1-0: Input
	GPIO_PORTE_AFSEL_R &= ~0x03;            // disable alternate functions
	GPIO_PORTE_DEN_R |= 0x03;               // enable digital functions
	//PE4 Player1Snake PE5 Player2Snake XX11XXXX
	GPIO_PORTE_AMSEL_R &= ~0x30;            // disable analog functions
	GPIO_PORTE_PCTL_R &= ~0x00FF0000;       // configure PE4-5 as GPIO
	GPIO_PORTE_DIR_R |= 0x30;             	// PE4-5: Output
	GPIO_PORTE_AFSEL_R &= ~0x30;            // disable alternate functions
	GPIO_PORTE_DEN_R |= 0x30;               // enable digital functions
	//Configure interrupts
	GPIO_PORTE_IS_R &= ~0x03;               // PE0-1 is edge-sensitive
	GPIO_PORTE_IBE_R &= ~0x03;              // PE0-1 is not both edges
	GPIO_PORTE_IEV_R |= 0x03;               // PE0-1 rising-edge event
	GPIO_PORTE_ICR_R |= 0x03;               // Clear flag
	GPIO_PORTE_IM_R |= 0x03;                // Arm interrupt on PE0-1
	NVIC_PRI1_R &= 0xFFFFFF00;              // Clear priority bits of Port E
	NVIC_PRI1_R |= 0x20;                    // Port E prority 1
	NVIC_EN0_R |= 0x10;                     // Enable NVIC interrupt
}

/****************************************************************
*************			GAME FUNCTIONS				*************
*****************************************************************/
// Set beginning status for players
void Game_Init(void) {
	state = PREGAME;	//set state PREGAME
	p1.isReady = 0;
	p1.caught = 26;
	p1.winner = 0;
	p2.isReady = 0;
	p2.caught = 26;
	p2.winner = 0;
	lastClick[0] = NVIC_ST_RELOAD_R;
	lastClick[1] = NVIC_ST_RELOAD_R;
}

/****************************************************************
*************			INTERRUPT HANDLERS			*************
*****************************************************************/
//Systick Handler for the phases of the game and measuring reaction time
void SysTick_Handler(void) {
	if (state == PREGAME || state == GAME) {
	return;
  }
	else if (state == STARTING) {
		++count;
		Update_Screen();
		if(flag==count) {
			state=GAME;
		}
	return;
	}
	else if (state == POSTGAME) {
		++count;
		Update_Screen();
		if(flag==count) {
			Game_Init();
		}
	return;
  }
}

//Get button press
void GPIOPortE_Handler(void) {
	unsigned int player = (GPIO_PORTE_RIS_R & 0x03) - 1; //P1 is 0, P2 is 1
	GPIO_PORTE_ICR_R |= (GPIO_PORTE_RIS_R & 0x03); //Acknowledge the flag of current player pin
	//debounce
	if(lastClick[player] - NVIC_ST_CURRENT_R < DEBOUNCE) { //Debounce
		return;
	}
	lastClick[player] = NVIC_ST_CURRENT_R; //not debounce get the current value for next check
	
	if (state == PREGAME) {
		if(player == 0) { //p1
			p1.isReady = p1.isReady ^ 1; //ready or unready
		}
		if(player == 1) { //p2
			p2.isReady = p2.isReady ^ 1; //ready or unready
		}
		if((p1.isReady + p2.isReady)>1) { //both players ready
			state = STARTING;
			count = 0;
			flag = 3;
		}
	return;
	} 
	else if (state == STARTING) {
		if(player == 0) { //p1
			p1.isReady = p1.isReady ^ 1; //ready or unready
			state=PREGAME;
		}
		if(player == 1) { //p2
			p2.isReady = p2.isReady ^ 1; //ready or unready
			state=PREGAME;
		}
	return;
	} 
	else if (state == GAME) {
		if(player == 0) { //p1
			P1_MOVE = 0x10;
			p1.caught++;
			p2.caught--;
			P1_MOVE = 0x00;
		}
		if(player == 1) { //p2
			P2_MOVE = 0x20;
			p2.caught++;
			p1.caught--;
			P2_MOVE = 0x00;
		}
		if(p1.caught == 0) { // p1 is caught
			state = POSTGAME;
			p2.winner = 1;
			count = 0;
			flag = CELEBRATION_TIME; //celebration
		return;
		}
		else if(p2.caught == 0) { //p2 is caught
			state = POSTGAME;
			p1.winner = 1;
			count = 0;
			flag = CELEBRATION_TIME; //celebration
		return;
		}
	return;
	}
	else if (state == POSTGAME) { //ignore presses while celebration is on
	return;
	}
}

void Update_Screen(void) {
	char* result;
	if(state == PREGAME) {
		Nokia5110_Clear(); // Clear LCD screen
		Nokia5110_SetCursor(1, 1);
		Nokia5110_OutString(((char*) "P1"));
		Nokia5110_SetCursor(8, 1);
		Nokia5110_OutString(((char*) "P2"));
	
		Nokia5110_SetCursor(0, 3);
		if(p1.isReady == 0)
			Nokia5110_OutString(((char*) "UNRDY"));
		else if(p1.isReady == 1)
			Nokia5110_OutString(((char*) "READY"));
		
		Nokia5110_SetCursor(7, 3);
		if(p2.isReady == 0)
			Nokia5110_OutString(((char*) "UNRDY"));
		else if(p2.isReady == 1)
			Nokia5110_OutString(((char*) "READY"));
	return;
	}
	else if(state == STARTING) {
		Nokia5110_Clear(); // Clear LCD screen
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString(((char*) "GET READY"));
		Nokia5110_SetCursor(5, 4);
	return;	
	}
	else if(state == GAME) {
		Nokia5110_Clear(); // Clear LCD screen
		Nokia5110_SetCursor(1, 1);
		Nokia5110_OutString(((char*) "P1"));
		Nokia5110_SetCursor(8, 1);
		Nokia5110_OutString(((char*) "P2"));
	
		Nokia5110_SetCursor(1, 3);
		snprintf(result, 3, "%d", p1.caught);
		Nokia5110_OutString(result); // Display P1 caught
		
		Nokia5110_SetCursor(8, 3);
		snprintf(result, 3, "%d", p2.caught);
		Nokia5110_OutString(result); // Display P1 caught
	return;	
	}
	else if(state == POSTGAME) {
		Nokia5110_Clear(); // Clear LCD screen
		Nokia5110_SetCursor(1, 3);
		Nokia5110_OutString(((char*) "GAME OVER!"));

		if(p1.winner==1) {
			Nokia5110_SetCursor(3, 1);
			Nokia5110_OutString("P1 WINS"); // Display the winner
		}
		else if(p2.winner==1) {
			Nokia5110_SetCursor(3, 1);
			Nokia5110_OutString("P2 WINS"); // Display the winner
		}
	return;
	}
}

/****************************************************************
*************			MAIN FUNCTION				*************
*****************************************************************/
int main(void) {
    Nokia5110_Init();
    PortE_Init();
    SysTick_Init(ONE_SEC);
    EnableInterrupts();
    Game_Init();
    while (1) {
		Update_Screen();
		WaitForInterrupt();
    }
}
