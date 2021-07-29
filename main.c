/* ========================================
 * LAST UPDATED: 18/10/2018
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// Variables
int debug_mode = 1;
int count;
int distance_measured;
char dist[16];
char output[50];

int16 count1 = 0, count2 = 0;
char string1[16], string2[16];
int distanceR = 0, distanceL = 0, distanceS = 0, distanceF, distanceB;
int distR[5], distL[5], distS[5];
int speed, distance;
int pwm_max = 65535;

// Stacking/detecting variables
int stack_counter = 0, row_counter = 1, save_counter1, save_counter2;
int order[3];
int order0 = 0, order1 = 0, order2 = 0;

// Function Definitions
void StepperControl(int direction, double stepsec);
void Grabber(int close);

void TurnLeft(int speed, int distance);
void TurnRight(int speed, int distance);
void GoForward(int speed, int distance);
void GoBackward(int speed, int distance);

void FrontBackSideUltra(int front, int back, int side);
void Alignment(int front, int back, int side);
void LaneAlign(int front, int back);
void startup();
void StackToPuck(void);
void TurnRight45(int distance);
void TrashPuck(void);
void ChooseRow(int row_counter);
void DetectPuck(void);
void PuckToStack(void);
void PuckToHome(void);

int colour_sense(int mode);

CY_ISR (debug){
    CyDelay(10);
    debug_mode = 1;
    debug_ClearPending();
    
}

CY_ISR (ultrasonic){
    
    Timer_1_ReadStatusRegister();
    count = Timer_1_ReadCounter();
 
    if (Control_Reg_Read() == 0b011){ // BACK LEFT
        distance_measured = (-0.1682*count) + 1696.6;
    }
    
    if (Control_Reg_Read() == 0b100){ // BACK RIGHT
        distance_measured = (-0.1721*count) + 1734.4;    
    }
    
    if (Control_Reg_Read() == 0b010){ // RIGHT SIDE
        distance_measured = (-0.1704*count) + 1716.3;    
    }
    
    if (Control_Reg_Read() == 0b001){ // FRONT RIGHT
        distance_measured = (-0.17*count) + 1717.2;    
    }
    
    if (Control_Reg_Read() == 0b000){ // FRONT LEFT
        distance_measured = (-0.1708*count) + 1730.5;    
    }
    
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    Clock_1_Start();
    Clock_2_Start();
    Clock_3_Start();
    UART_1_Start();
    Decoder_L_Start();
    Decoder_R_Start();
    PWM_L_Start();
    PWM_R_Start();
    Timer_1_Start();
    PWM_1_Start();
    ADC_Start();
    TIA_Start();
    IDAC_Start();
    IDAC_SetValue(10);
    ADC_StartConvert();
    AMux_Start();
    
    debug_StartEx(debug);
    isr_1_StartEx(ultrasonic);
    
    for(;;)
    {
        if (debug_mode == 1){
            
            stack_counter = 0;
            save_counter1 = 0;
            save_counter2 = 0;
            order0 = 0;
            order1 = 0;
            order2 = 0;
            distanceF = 0, distanceB = 0, distanceS = 0, distanceR = 0, distanceL = 0;
            
            row_counter = 1;
            
            startup();
            
            while (stack_counter != 3){
                ChooseRow(row_counter);
                DetectPuck();
            }
            
            PuckToHome();
            
            
        }
         
         debug_mode = 0;
 
    }
}
void startup(void){

    LEDR_side_Write(1);
    
    // Scan side pucks, store in memory, keep moving forward (for 3 pucks)
    
    GoBackward(0.9*pwm_max, 10000);

    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
    
    while (distanceB > 287){
        GoBackward(0.7*pwm_max, 750);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    order0 = colour_sense(0);
    int counter = 0;
    while (order0 == 0){
        order0 = colour_sense(0);   
        counter++;
        if (counter == 4){
            order0 = 2;   
        }
    }
    
    if (order0 == 1){
            LEDR_Write(1);   
            CyDelay(500);
            LEDR_Write(0);
        } else if (order0 == 2){
            LEDG_Write(1);   
            CyDelay(500);
            LEDG_Write(0);
        } else if (order0 == 3){
            LEDB_Write(1);   
            CyDelay(500);
            LEDB_Write(0);
        }
    
    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
    
    /*while (distanceB < 330){
        GoForward(0.7*pwm_max, 500);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }*/ 
    GoForward(0.7*pwm_max, 5000);
    counter = 1;
    order1 = colour_sense(0);
        while (order1 == 0){
        order1 = colour_sense(0);  
        counter++;
        if (counter == 4){
            order1 = 2;
        }
        }
        
    if (order1 == 1){
        LEDR_Write(1);   
        CyDelay(500);
        LEDR_Write(0);
    } else if (order1 == 2){
        LEDG_Write(1);   
        CyDelay(500);
        LEDG_Write(0);
    } else if (order1 == 3){
        LEDB_Write(1);   
        CyDelay(500);
        LEDB_Write(0);
    }
        
    /*while (distanceB < 380){
        GoForward(0.7*pwm_max, 500);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }*/
    
    GoForward(0.7*pwm_max, 5000);
    
    order2 = colour_sense(0);
    counter = 0;
    while (order2 == 0){
        order2 = colour_sense(0);  
        counter++;
        if (counter == 4){
            order2 = 2;
    }
    }
    
    if (order2 == 1){
            LEDR_Write(1);   
            CyDelay(500);
            LEDR_Write(0);
        } else if (order2 == 2){
            LEDG_Write(1);   
            CyDelay(500);
            LEDG_Write(0);
        } else if (order2 == 3){
            LEDB_Write(1);   
            CyDelay(500);
            LEDB_Write(0);
        }
    
    TurnRight45(30000);
    GoForward(0.9*pwm_max, 10000);
    TurnLeft(0.9*pwm_max, 5000);
    //Alignment(1, 0, 0);
   
    FrontBackSideUltra(1, 0, 0);
    distanceF = (distanceR + distanceL)/2;
    
    while(distanceF > 150){
        GoForward(0.9*pwm_max, 3000);   
        FrontBackSideUltra(1, 0, 0);
        distanceF = (distanceR + distanceL)/2;
    }
    
    while (distanceF > 110){ 
        GoForward(0.9*pwm_max, 1000);
        FrontBackSideUltra(1, 0, 0);
        distanceF = (distanceR + distanceL)/2;
    
    }
    
    TurnLeft(0.9*pwm_max, 14000);
    Alignment(1, 0, 0);
    GoBackward(0.9*pwm_max, 30000);
    GoBackward(0.9*pwm_max, 25000);
    
    TurnRight(0.9*pwm_max, 7000);
    GoForward(0.9*pwm_max, 5000);
    TurnLeft(0.9*pwm_max, 7000);
    GoBackward(0.9*pwm_max, 5000);
    
    StepperControl(1, 1.5);
    
    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
    
    while (distanceB > 75){
        GoBackward(0.9*pwm_max, 4000);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    while (distanceB > 50){
        GoBackward(0.9*pwm_max, 1000);   // Can edit this to improve accuracy 
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    Alignment(0, 1, 0);
    
    Control_Reg_L_Write(0b00);
    Control_Reg_L_Write(0b00);
    
}
void TurnRight45(int distance){
    
    Decoder_L_SetCounter(0);
    Decoder_R_SetCounter(0);

    // Initialise wheel and direction
    PWM_L_WriteCompare(60000);
    PWM_R_WriteCompare(30000);
    Control_Reg_L_Write(0b01);
    Control_Reg_R_Write(0b10);

    count1 = abs(Decoder_L_GetCounter());
    count2 = abs(Decoder_R_GetCounter());

    while((count1 < distance)) {     // May need to convert distance to ticks (or decoder counts to distance)
        
        count1 = abs(Decoder_L_GetCounter());
        count2 = abs(Decoder_R_GetCounter());
        
        sprintf(string1, "Left Wheel: %i. - Right Wheel: %i. \n", count1, count2);   
        UART_1_PutString(string1);
    }

    Control_Reg_L_Write(0b00);
    Control_Reg_R_Write(0b00);
    
    CyDelay(100);    
}
void LaneAlign(int front, int back){
    
    if (front == 1){
        FrontBackSideUltra(1, 0, 0);
        distanceF = (distanceR + distanceL)/2;
        
        while (distanceF > 135){
            GoForward(0.7*pwm_max, 4000);    
            FrontBackSideUltra(1, 0, 0);
            distanceF = (distanceR + distanceL)/2;
        }
        while (distanceF > 120){
            GoForward(0.7*pwm_max, 750);
            FrontBackSideUltra(1, 0, 0);
            distanceF = (distanceR + distanceL)/2;
        }
        /*while (distanceF < 100){
            GoBackward(0.5*pwm_max, 500);
            CyDelay(100);
            FrontBackSideUltra(1, 0, 0);
            distanceF = (distanceR + distanceL)/2;    
        }
        Alignment(1, 0, 0); */
    }    
    else if (back == 1){
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
        
        while(distanceB > 110){
            GoBackward(0.7*pwm_max, 4000);
            FrontBackSideUltra(0, 1, 0);
            distanceB = (distanceR + distanceL)/2;
        }
        while (distanceB > 100){
            GoBackward(0.7*pwm_max, 750);
            FrontBackSideUltra(0, 1, 0);
            distanceB = (distanceR + distanceL)/2;
        }
        /* while (distanceB < 100){
            GoForward(0.5*pwm_max, 750);
            CyDelay(100);
            FrontBackSideUltra(0, 1, 0);
            distanceB = (distanceR + distanceL)/2;    
        }
        Alignment(0, 1, 0); */ 
    }  
}
void StackToPuck(void){
    
    // This function assumes that you start at the default stack position
    CyDelay(100);
    GoBackward(0.7*pwm_max, 30000);
    GoBackward(0.7*pwm_max, 25000);
    Alignment(0, 1, 0);
    
    FrontBackSideUltra(0, 0, 1);
    
    if (distanceS > 130){
        TurnRight(0.7*pwm_max, 7000);
        GoForward(0.7*pwm_max, 5000);
        TurnLeft(0.7*pwm_max, 7000);
        GoBackward(0.7*pwm_max, 4000);
    }
    
    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
    
    while (distanceB > 50){
        GoBackward(0.7*pwm_max, 1000);   // Can edit this to improve accuracy 
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    Alignment(0, 1, 0);
}
void PuckToStack(void){
   
    // This function assumes that you start at the default puck position
    CyDelay(100);
    GoForward(0.7*pwm_max, 30000); // EDIT
    GoForward(0.7*pwm_max, 25000);
    
    FrontBackSideUltra(0, 0, 1);
    
    if (distanceS > 120){
        Alignment(0, 0, 1);    
        
    }
    
    //Alignment(1, 0, 0);
    
    FrontBackSideUltra(1, 0, 0);
    distanceF = (distanceR + distanceL)/2;
    
    while (distanceF > 110){
        GoForward(0.7*pwm_max, 250);    
        FrontBackSideUltra(1, 0, 0);
        distanceF = (distanceR + distanceL)/2;
    } 
    
    Alignment(1, 0, 0); 
    CyDelay(100);
}
void ChooseRow(int row_counter){
  
    StepperControl(0, 1.5);
    
    if (row_counter == 1){
        
        GoForward(0.9*pwm_max, 9000); // FIND EXACT DISTANCE
        TurnLeft(0.9*pwm_max, 14000);
        
        Alignment(0, 1, 0); // Align with back wall
        GoForward(0.9*pwm_max, 1000);
    }
    
    else if (row_counter == 2){
        
        GoForward(0.9*pwm_max, 4000); // FIND EXACT DISTANCE
        TurnLeft(0.9*pwm_max, 14000);
        
        Alignment(0, 1, 0); // Align with back wall
        GoForward(0.9*pwm_max, 2000);
        
    }
    
    else if (row_counter == 3){
        
        TurnLeft(0.9*pwm_max, 14000);
        Alignment(0, 1, 0); // Align with back wall
        GoForward(0.9*pwm_max, 2000);
        //TurnLeft(0.5*pwm_max, 1000);
    }
}
void DetectPuck(void){
    
    // This function assumes that a row has already been chosen
    
    int current_order;
    
    if (stack_counter == 0){
        current_order = order0;    
    }
    else if (stack_counter == 1){
        current_order = order1;    
    }
    else if (stack_counter == 2){
        current_order = order2;    
    }
    
    int puck_colour;
    puck_colour = colour_sense(1);
    
    while (puck_colour == 0){ 
        GoForward(0.9*pwm_max, 1000);
        puck_colour = colour_sense(1);
    } // If no puck is detected, move forward
    
    GoForward(0.5*pwm_max, 250);
    
    if (puck_colour == current_order){
        
        Grabber(1);
        //puck_colour = colour_sense(1);
        StepperControl(1, 1.5);
        
        if (row_counter == 3){
            row_counter = 1;    
        }
        else {
            row_counter++;
        }
        
    } // PICKED THE RIGHT PUCK
    else {
        
        GoBackward(0.7*pwm_max, 4000);
        puck_colour = colour_sense(1);
        StepperControl(1, 1.5);
        
        if (row_counter == 3){
            row_counter = 1;    
        }
        else {
            row_counter++;
        }
    } // GO TO NEXT ROW
    
    // Go to Default Puck Area
    
    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
    
    LaneAlign(0, 1);
    TurnRight(0.7*pwm_max, 14000);
    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
        
    while(distanceB > 50){
        GoBackward(0.9*pwm_max, 750);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    Alignment(0, 1, 0);    
    /*sprintf(dist, "Puck_colour = %d\n Stack_counter = %d\n Order[0] = %d\n", puck_colour, stack_counter, order0);
    UART_1_PutString(dist);*/
    
    
    if ((puck_colour != 0) && (stack_counter == 0)){
        
        if (puck_colour == order0){
            
            PuckToStack(); // NEEDS ALIGNMENT
            
            StepperControl(0, 1.5);
            Grabber(0);
            StepperControl(1, 1.5);
            stack_counter = 1;
               
            StackToPuck();
            
        } // PUCK NEEDS TO BE STACKED
    }
    else if ((puck_colour != 0) && (stack_counter == 1)){
        
        if (puck_colour == order1){
            PuckToStack();
            StepperControl(0, 1);
            Grabber(0);
            StepperControl(1, 1);
            stack_counter = 2;
            
            StackToPuck();
            
            
        } // PUCK NEEDS TO BE STACKED  
    }
    else if ((puck_colour != 0) && (stack_counter == 2)){
        if (puck_colour == order2){
            
            PuckToStack();
            StepperControl(0, 0.5);
            Grabber(0);
            StepperControl(1, 0.5);
            stack_counter = 3;
           
        } // PUCK NEEDS TO BE STACKED 
    }     
} 
void TrashPuck(void){
            
    GoForward(0.9*pwm_max, 20000); // EDIT
    TurnLeft(0.9*pwm_max, 14000);
    StepperControl(0, 1.5);
    GoForward(0.9*pwm_max, 10000);
    Grabber(0);
    StepperControl(1, 1.5);
    GoBackward(0.9*pwm_max, 10000);
    //Alignment(0, 1, 0);
    LaneAlign(0, 1);
    Alignment(0, 1, 0);
    TurnRight(0.7*pwm_max, 14000);
    GoBackward(0.7*pwm_max, 20000);
    //Alignment(0, 1, 0);
    
    while (distanceB > 75){
        GoBackward(0.9*pwm_max, 2000);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    while (distanceB > 50){
        GoBackward(0.9*pwm_max, 750);   // Can edit this to improve accuracy 
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    Alignment(0, 1, 0);
    
} // Will drive to trash area from DEFAULT PUCK, and drop the puck, and then return to PUCK area (default position)
void PuckToHome(void){
    GoBackward(0.9*pwm_max, 5000);
    TurnLeft(0.9*pwm_max, 14000);
    Alignment(0, 1, 0);
    
    FrontBackSideUltra(0, 1, 0);
    distanceB = (distanceR + distanceL)/2;
    
    while (distanceB > 75){
        GoBackward(0.9*pwm_max, 750);
        FrontBackSideUltra(0, 1, 0);
        distanceB = (distanceR + distanceL)/2;
    }
    
    GoForward(0.9*pwm_max, 30000);
    GoForward(0.9*pwm_max, 5000);
    TurnLeft(0.9*pwm_max, 14000);
    Alignment(1, 0, 0);
    
    FrontBackSideUltra(1, 0, 0);
    distanceF = (distanceR + distanceL)/2;
    int distanceF_ref = distanceF;
    
    while (distanceF_ref + 150 > distanceF){
        GoBackward(0.9*pwm_max, 750);
        FrontBackSideUltra(1, 0, 0);
        distanceF = (distanceR + distanceL)/2;    
    }
    
} 

void FrontBackSideUltra(int front, int back, int side){

    if (back == 1){
        for(int i=0; i<5; i++){
        
            Control_Reg_Write(0b100); 
            
            BackR_Trigger_Write(1);
            CyDelayUs(10);
            BackR_Trigger_Write(0);
            CyDelay(10);   
            
            distR[i] = distance_measured; 
            
            Control_Reg_Write(0b011);

            BackL_Trigger_Write(1);
            CyDelayUs(10);
            BackL_Trigger_Write(0);
            CyDelay(10);
            
            distL[i] = distance_measured;
            
        }
    }
    else if (front == 1){
        for(int i=0; i<5; i++){
        
            Control_Reg_Write(0b001); 
            
            FrontR_Trigger_Write(1);
            CyDelayUs(10);
            FrontR_Trigger_Write(0);
            CyDelay(10);   
            
            distR[i] = distance_measured; 
            
            Control_Reg_Write(0b000);

            FrontL_Trigger_Write(1);
            CyDelayUs(10);
            FrontL_Trigger_Write(0);
            CyDelay(10);
            
            distL[i] = distance_measured;
            
        }
    }
    else if (side == 1){
        
        Control_Reg_Write(0b010); 
           
        for (int i = 0; i<5; i++){
            Side_Trigger_Write(1);
            CyDelayUs(10);
            Side_Trigger_Write(0);
            CyDelay(10);   
        
            distS[i] = distance_measured;
        }
    }
    
    distanceR = (distR[1] + distR[2] + distR[3])/3;
    distanceL = (distL[1] + distL[2] + distL[3])/3;
    distanceS = (distS[1] + distS[2] + distS[3])/3;
    
    //distanceR = (distR[0] + distR[1] + distR[2] + distR[3] + distR[4])/5;
    //distanceL = (distL[0] + distL[1] + distL[2] + distL[3] + distL[4])/5;
    
    /*sprintf(string1, "\nRight distance = %i mm. - Left distance = %i mm. - Side distance: = %i mm.\n", distanceR, distanceL, distanceS);
    UART_1_PutString(string1); */

}

void Alignment(int front, int back, int side){
    
    if (front == 1){
        FrontBackSideUltra(1, 0, 0);
            
        for (int i = 0; i < 2; i++){
            if (distanceR < distanceL){

                while(distanceR < distanceL + 3){
                
                    TurnRight(0.5*pwm_max, 150);
                    
                    FrontBackSideUltra(1, 0, 0);
                }
            }
            else if (distanceR > distanceL){
                    
                while(distanceL < distanceR + 3){
                    
                    TurnLeft(0.5*pwm_max, 150);
                    
                    FrontBackSideUltra(1, 0, 0);
                }
                //TurnRight(0.5*pwm_max, 400);
            }
        
        FrontBackSideUltra(1, 0, 0);
        }
    }
    else if(back == 1){
        FrontBackSideUltra(0, 1, 0);
        
        for (int i = 0; i < 2; i++){
            if (distanceR > distanceL){
                while(distanceR > distanceL + 3){
                
                    TurnRight(0.5*pwm_max, 150);
                    
                    FrontBackSideUltra(0, 1, 0);
                }
            }
            else if (distanceL > distanceR){           
                while(distanceL > distanceR + 3){
                    
                    TurnLeft(0.5*pwm_max, 150);
                    
                    FrontBackSideUltra(0, 1, 0);
                }
            }
        FrontBackSideUltra(0, 1, 0);
        }
    }
    else if(side == 1){
        FrontBackSideUltra(0, 0, 1);
        
        while(distanceS > 120){ // TEST THIS
            TurnRight(0.7*pwm_max, 5000);
            GoForward(0.7*pwm_max, 1000);
            TurnLeft(0.7*pwm_max, 4500);
            GoBackward(0.7*pwm_max, 1000);
            FrontBackSideUltra(0, 0, 1);
        }
    }
}
   
void StepperControl(int direction, double stepsec){
    
    // Direction = 1 is UP
    // Direction = 0 is DOWN
    
    // For LIFT control 
        
    LiftDirection_Write(direction); 
    
    for(int i = 0; i < stepsec*1000; i++){
            
        LiftStep_Write(1);
        CyDelayUs(500);
        LiftStep_Write(0);
        CyDelayUs(500);
               
    }
    
    CyDelay(100);
}

void Grabber(int close){
        
    if (close == 1){
        PWM_1_WriteCompare(2000);
        CyDelay(100);
    }
    else if (close == 0){
        PWM_1_WriteCompare(1500);
        CyDelay(100);
    }
}

void GoBackward(int speed, int distance){
    
    CyDelay(50);
    Decoder_L_SetCounter(0);
    Decoder_R_SetCounter(0);
    
    // May need variable to adjust speed for wheel correction
    
    PWM_L_WriteCompare(speed + 2500);
    PWM_R_WriteCompare(speed);
    Control_Reg_L_Write(0b10);
    Control_Reg_R_Write(0b01);
    
    count1 = abs(Decoder_L_GetCounter());
    count2 = abs(Decoder_R_GetCounter());
    
    int speedcon = speed;
    
    while(((count1 < distance) || (count2 < distance))){     // May need to convert distance to ticks (or decoder counts to distance)
        
        count1 = abs(Decoder_L_GetCounter());
        count2 = abs(Decoder_R_GetCounter());
        
        sprintf(string1, "Left Wheel: %i. - Right Wheel: %i. \n", count1, count2);   
        //UART_1_PutString(string1);
        
        // WHEEL CORRECTION CODE
        
        
        if(count1 > count2){    // If left wheel is faster than right wheel
            speedcon--;         // Decrease speed of left wheel
            PWM_L_WriteCompare(speedcon + 2500);
        
            if (speedcon == 10){
                speedcon++;
            }
        }
        
        else if (count2 > count1){  // If right wheel is faster than left wheel
            speedcon++;             // Increase speed of left wheel
            PWM_L_WriteCompare(speedcon + 2500);
            
            if (speedcon == 65000){
                speedcon--;
            }
        }        
    }
    
    UART_1_PutString("END.");
    Control_Reg_L_Write(0b00);
    Control_Reg_R_Write(0b00);    
    
    CyDelay(50);
}

void GoForward(int speed, int distance){

    CyDelay(50);
    Decoder_L_SetCounter(0);
    Decoder_R_SetCounter(0);
    
    // May need variable to adjust speed for wheel correction
    
    // Initialise wheel and direction
    PWM_L_WriteCompare(speed + 2500);
    PWM_R_WriteCompare(speed);
    Control_Reg_L_Write(0b01);
    Control_Reg_R_Write(0b10);
    
    count1 = abs(Decoder_L_GetCounter());
    count2 = abs(Decoder_R_GetCounter());
    
    int speedcon = speed;
    
    while(((count1 < distance) || (count2 < distance))){     // May need to convert distance to ticks (or decoder counts to distance)
        
        count1 = abs(Decoder_L_GetCounter());
        count2 = abs(Decoder_R_GetCounter());
        
        sprintf(string1, "Left Wheel: %i. - Right Wheel: %i. \n", count1, count2);   
        //UART_1_PutString(string1);
        
        // WHEEL CORRECTION CODE
        
        if(count1 > count2){    // If left wheel is faster than right wheel
            speedcon--;         // Decrease speed of left wheel
            PWM_L_WriteCompare(speedcon + 2500);
        
            if (speedcon == 10){
                speedcon++;
            }
        }
        
        else if (count2 > count1){  // If right wheel is faster than left wheel
            speedcon++;             // Increase speed of left wheel
            PWM_L_WriteCompare(speedcon + 2500);
            
            if (speedcon == 65000){
                speedcon--;
            }
        }
    }
    
    UART_1_PutString("END.");
    Control_Reg_L_Write(0b00);
    Control_Reg_R_Write(0b00);    
    
    CyDelay(50);
}

void TurnLeft(int speed, int distance){
    
    CyDelay(50);
    Decoder_L_SetCounter(0);
    Decoder_R_SetCounter(0);
    
    // May need variable to adjust speed for wheel correction
    // FIND NUMBER OF TICKS REQUIRED FOR 90 DEGREE TURN
    
    PWM_L_WriteCompare(speed);
    PWM_R_WriteCompare(speed);
    Control_Reg_L_Write(0b10);
    Control_Reg_R_Write(0b10);
    
    count1 = abs(Decoder_L_GetCounter());
    count2 = abs(Decoder_R_GetCounter());
    
    int speedcon = speed;
    
    while(((count1 < distance) || (count2 < distance))){     // May need to convert distance to ticks (or decoder counts to distance)
        
        count1 = abs(Decoder_L_GetCounter());
        count2 = abs(Decoder_R_GetCounter());
        
        sprintf(string1, "Left Wheel: %i. - Right Wheel: %i. \n", count1, count2);   
        //UART_1_PutString(string1);
        
        if(count1 > count2){    // If left wheel is faster than right wheel
            speedcon--;         // Decrease speed of left wheel
            PWM_L_WriteCompare(speedcon);
        
            if (speedcon == 10){
                speedcon++;
            }
        }
        
        else if (count2 > count1){  // If right wheel is faster than left wheel
            speedcon++;             // Increase speed of left wheel
            PWM_L_WriteCompare(speedcon);
            
            if (speedcon == 250){
                speedcon--;
            }
        }
    }
    
    UART_1_PutString("END.");
    Control_Reg_L_Write(0b00);
    Control_Reg_R_Write(0b00);
    
    CyDelay(50);
}

void TurnRight(int speed, int distance){
    
    CyDelay(50);
    Decoder_L_SetCounter(0);
    Decoder_R_SetCounter(0);
    
    // May need variable to adjust speed for wheel correction
    // FIND NUMBER OF TICKS REQUIRED FOR 90 DEGREE TURN
    
    PWM_L_WriteCompare(speed);
    PWM_R_WriteCompare(speed);
    Control_Reg_L_Write(0b01);
    Control_Reg_R_Write(0b01);
    
    count1 = abs(Decoder_L_GetCounter());
    count2 = abs(Decoder_R_GetCounter());
    
    int speedcon = speed;
    
    while(((count1 < distance) || (count2 < distance))){     // May need to convert distance to ticks (or decoder counts to distance)
        
        count1 = abs(Decoder_L_GetCounter());
        count2 = abs(Decoder_R_GetCounter());
    
        sprintf(string1, "Left Wheel: %i. - Right Wheel: %i. \n", count1, count2);   
        //UART_1_PutString(string1);
        
        if(count1 > count2){    // If left wheel is faster than right wheel
            speedcon--;        // Decrease speed of left wheel
            PWM_L_WriteCompare(speedcon);
        
            if (speedcon == 10){
                speedcon++;
            }
        }
        
        else if (count2 > count1){  // If right wheel is faster than left wheel
            speedcon++;            // Increase speed of left wheel
            PWM_L_WriteCompare(speedcon);
            
            if (speedcon == 250){
                speedcon++;
            }
        }

    }
    
    UART_1_PutString("END.");
    
    // Stop Wheels
    Control_Reg_L_Write(0b00);
    Control_Reg_R_Write(0b00);
    
    // Measuring distance to wall after turning
    
    CyDelay(50);
}

int colour_sense(int mode){
    //Input: mode 0 = side, mode 1 = front
    //Output: 0 = None, 1 = Red, 2 = Green, 3 = Blue
    
    int raw_intensity[5];
    char output[50];
    //int colour_code;
    int ambient[5];
    int average_ambient;
    int average_intensity;
    int intensity;
    int intensity_red, intensity_green, intensity_blue;
        int i;
        int colour;
        int LEDR_status, LEDG_status, LEDB_status;
        //LEDStatus = LEDR_Read();
        
            LEDR_Write(0);   //active high using transistor
            LEDG_Write(0);
            LEDB_Write(0);
            LEDR_side_Write(0);
            LEDG_side_Write(0);
            LEDB_side_Write(0);
            
            LEDR_status = LEDR_Read();
            LEDG_status = LEDG_Read();
            LEDB_status = LEDB_Read();
            
            if (mode == 0){
                AMux_Select(0);
            } else if (mode == 1){
                AMux_Select(1);   
            }
            
            
            //sprintf(output, "All LEDs are %d \n", LEDB_status);
            //UART_PutString(output);

             for (i = 0; i <= 4; i++){
                    CyDelay(100);
                    
                    ADC_StartConvert();
                    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
                    ambient[i] = ADC_GetResult32();

                     if (i == 4){
                        average_ambient = (ambient[0] + ambient[1] + ambient[2] + ambient[3] + ambient[4]) / 5;
                        sprintf(output, "average ambient = %d \n ", average_ambient);
                        UART_1_PutString(output);
                        }
                }
            
            for (colour = 1; colour <= 3; colour++){

                if (colour == 1){
                    if (mode == 0){
                        LEDR_side_Write(1);
                        LEDG_side_Write(0);
                        LEDB_side_Write(0);
                    } else if (mode == 1){
                        LEDR_Write(1);
                        LEDG_Write(0);
                        LEDB_Write(0);
                        LEDR_status = LEDR_Read();
                        //sprintf(output, "LEDR is %d \n", LEDR_status);
                        //UART_PutString(output);
                    }
                    
                    for (i = 0; i <= 4; i++){
                        
                        CyDelay(50);
                        
                        ADC_StartConvert();
                        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
                        raw_intensity[i] = ADC_GetResult32();

                        
                        if (i == 4){
                            average_intensity = (raw_intensity[0] + raw_intensity[1] + raw_intensity[2] + raw_intensity[3] + raw_intensity[4]) / 5;

                            
                            intensity_red = average_intensity - average_ambient;
                            sprintf(output, "Red = %d \n ", intensity_red);
                            UART_1_PutString(output);
                        
                        }
                    }
                }
                else if (colour == 2){

                    if (mode == 0){
                        LEDR_side_Write(0);
                        LEDG_side_Write(1);
                        LEDB_side_Write(0);
                    } else if (mode == 1){
                        LEDR_Write(0);
                        LEDG_Write(1);
                        LEDB_Write(0);
                        LEDG_status = LEDG_Read();
                        //sprintf(output, "LEDG is %d \n", LEDG_status);
                        //UART_PutString(output);
                    }
                    
                    for (i = 0; i <= 4; i++){
                        
                        CyDelay(50);
                        
                        ADC_StartConvert();
                        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
                        raw_intensity[i] = ADC_GetResult32();

                                   
                        if (i == 4){
                            average_intensity = (raw_intensity[0] + raw_intensity[1] + raw_intensity[2] + raw_intensity[3] + raw_intensity[4]) / 5;

                            
                            intensity_green = average_intensity - average_ambient;
                            sprintf(output, "Green %d \n", intensity_green);
                            UART_1_PutString(output);
                        
                        }
                    }
                }
                else if (colour == 3){
                    if (mode == 0){
                        LEDR_side_Write(0);
                        LEDG_side_Write(0);
                        LEDB_side_Write(1);
                    } else if (mode == 1){
                        LEDR_Write(0);
                        LEDG_Write(0);
                        LEDB_Write(1);
                        LEDB_status = LEDB_Read();
                        //sprintf(output, "%d \n", LEDB_status);
                        //UART_PutString(output);
                    }
                    
                    for (i = 0; i <= 4; i++){
                        
                        CyDelay(50);
                        
                        ADC_StartConvert();
                        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
                        raw_intensity[i] = ADC_GetResult32();

                                   
                        if (i == 4){
                            average_intensity = (raw_intensity[0] + raw_intensity[1] + raw_intensity[2] + raw_intensity[3] + raw_intensity[4]) / 5;

                            
                            intensity_blue = average_intensity - average_ambient;
                            sprintf(output, "Blue %d \n\n", intensity_blue);
                            UART_1_PutString(output);
                        
                        }
                    }
                    LEDB_Write(0);
                    LEDB_side_Write(0);
                    
                }
            }
            
            
            if (intensity_blue > intensity_green && intensity_blue > intensity_red && intensity_blue > 7000 && intensity_green < (0.5*intensity_blue) && intensity_red < (0.3*intensity_blue)){
                sprintf(output, "3 \n\n");
                UART_1_PutString(output);
                return 3;
                
            } else if (mode == 0 && intensity_green > intensity_red && intensity_green > intensity_blue && intensity_green > 4000){
                    sprintf(output, "2 \n\n");
                    UART_1_PutString(output);
                    return 2;
                
            } else if (mode == 1 && (intensity_green > 1000 && intensity_green < 5000) && (intensity_blue > 3000 && intensity_blue < 7000)){
                    sprintf(output, "2 \n\n");
                    UART_1_PutString(output);
                    return 2;
                    
            } else if (mode == 0 && intensity_red > intensity_green && intensity_red > intensity_blue && intensity_red > 5000){
                sprintf(output, "1 \n\n");
                UART_1_PutString(output);
                return 1;
                
            } else if (mode == 1 && intensity_red > intensity_green && intensity_red > intensity_blue && intensity_red > 6000){
                sprintf(output, "1 \n\n");
                UART_1_PutString(output);
                return 1;
                
            } else if (mode == 1 && intensity_red > 5000 && intensity_green > 2000 && intensity_blue > 10000){
                sprintf(output, "0 \n\n");
                UART_1_PutString(output);
                return 0;
            }
            else {
                sprintf(output, "0 \n\n");
                UART_1_PutString(output);
                return 0;
            }
                
         
            
        }

/*  [] END OF FILE */

/* ULTRASONIC CALIBRATION CODE

            Control_Reg_Write(0b010);
            
            UART_1_PutString("START.\n");
            int i = 0;
            for(i = 0; i < 5; i++){
            
            Side_Trigger_Write(1);
            CyDelayUs(10);
            Side_Trigger_Write(0);
            CyDelay(50);
            
            sprintf(dist, "Timer value = %i mm\n", distance_measured);
            UART_1_PutString(dist);
            
            }
            
            UART_1_PutString("END.\n"); 

*/