/************************************************************************************************************************
 * Includes
 ***********************************************************************************************************************/
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "tm1637.h"

#include "mein.h"
#include "led.h"
#include "midi.h"
#include "misc.h"

#include "os.h"

 /************************************************************************************************************************
 * Defines
 ***********************************************************************************************************************/

 // POT_MODE_DEFAULT    POT_MODE_CC
 #define POT_MODE_CC

 #define DEFAULT_STACK_SIZE	512

 #define MAIN_STACK_SIZE 512

 #define TASK_MAIN_PRIO 10

 /************************************************************************************************************************
 * Global variables
 ***********************************************************************************************************************/

//#ifndef true
//#error "true is not defined"
//#endif

bool msElapsed = false;
uint8 midiRxBuffer[3] = {0};

 /************************************************************************************************************************
 * Static variables
 ***********************************************************************************************************************/

//// buffers --------------------------------
//
//static AnalogBuffer_t potBuffer;
//
//// states --------------------------------
//
//static bool activeSteps[16] = {
//    1, 0, 1, 1, 0, 0, 0, 0, 
//    1, 0, 0, 0, 1, 1, 0, 0
//};
//static Mode_e currentMode = MODE_DEFAULT;       // the currently active mode of the device
//static uint8 currentStep = 0;                   // the step that is currently being played
//static uint8 selectedStep = 0;                  // the step that is selected by the user for editing -- 0xDD: no selection | 0-15: selected
//static int8 stepPitch[16];                     // array that stores the currently set pitch for every step
//
//// flags --------------------------------
//
//static bool buttonsPressed[16] = {false};
//static bool stepUpdate = true;
//static bool dispUpdate = true;
//static bool ledUpdate = true;
//static bool noteOn = false;
//
//// timing --------------------------------
//
//static uint16 msCounter = 0;
//static uint16 stepTime_ms = 500;
//static uint8 gate_percent = 50;

// tasks --------------------------------

//OS_STK taskStackInit[128];
//OS_STK taskStackMain[MAIN_STACK_SIZE];

/************************************************************************************************************************
 * Function declarations
 ***********************************************************************************************************************/

//void InitTask(void * arg);
//void MainTask(void * arg);
//static void ReadButtons();

/************************************************************************************************************************
* Task definitions
***********************************************************************************************************************/

//void InitTask(void * arg)
//{
//    OS_CPU_SysTickInit(HAL_RCC_GetHCLKFreq()/(INT32U)OS_TICKS_PER_SEC);
//    OSTaskCreate(MainTask, (void*)0, &taskStackMain[MAIN_STACK_SIZE-1], TASK_MAIN_PRIO);
//    OSTaskDel(OS_PRIO_SELF);
//}

void MainTask( void * parameters )
{   
    UNUSED(parameters);
    
    while(1)
    {
        //// ------------ stepping ---------
        //if (stepUpdate)
        //{
        //    if (true == activeSteps[currentStep])
        //    {
        //        MIDI_SendNoteOn(20 + stepPitch[currentStep], 0, 20, 0);
        //        noteOn = true;
        //    }
        //    stepUpdate = false; 
        //}
//
        //// ------------ updating LEDs ---------
        //if(ledUpdate)
        //{
        //    switch(currentMode)
        //    {
        //        case MODE_PITCH:
        //        {
        //            LED_Set(activeSteps);
        //            ledUpdate = false;
        //            break;
        //        }
        //        default:
        //        {
        //            if (msCounter == 0)
        //            {
        //                bool ledStateBuff[16];
        //                memcpy(ledStateBuff, activeSteps, 16);
        //                ledStateBuff[currentStep] = !ledStateBuff[currentStep];
        //                LED_Set(ledStateBuff);
        //            }
        //            if (msCounter == 50)
        //            {
        //                LED_Set(activeSteps);
        //                ledUpdate = false;
        //            }
        //        }
        //    }
        //}
//
        //// ------------ updating display ---------
        //if(dispUpdate)
        //{
        //    switch(currentMode)
        //    {
        //        case MODE_PITCH:
        //        {
        //            tm1637_SetModeAndValue(currentMode, stepPitch[selectedStep], (selectedStep != 0xDD));
        //            dispUpdate = false;
        //            break;
        //        }
        //        default:
        //        {
        //            tm1637_SetDefault();
        //            dispUpdate = false;
        //        }
        //    }
        //}
//
        //if (msCounter >= ((stepTime_ms * gate_percent)/100))
        //{
        //    if(noteOn)
        //    {
        //        MIDI_SendNoteOff(20 + stepPitch[currentStep], 0, 0);
//
        //        noteOn = false;
        //    }
        //}
//
        //// ----- timing related stuff -----
        //if(msElapsed)
        //{
        //    msCounter++;
        //    msElapsed = false;
        //}
        //if(msCounter >= stepTime_ms)
        //{
        //    msCounter = 0;
        //    if(currentStep >= 15) 
        //    {
        //        currentStep = 0;
        //    }
        //    else 
        //    { 
        //        currentStep++;
        //    }
        //    stepUpdate = true;
        //    ledUpdate = true;
        //}
        
        // ----- reading potentiometers -----
        #ifdef POT_MODE_DEFAULT
            uint16 tempo_percent = MAX((uint8)SATURATE((((fabs((float)SATURATE(potBuffer.tempo, 4024) - 4024.0))/4024.0)*100.0), 100.0), 1);
            gate_percent = (uint8)SATURATE((((fabs((float)SATURATE(potBuffer.gate, 4024) - 4024.0))/4024.0)*100.0), 99.0);
        #else
        #ifdef POT_MODE_CC
            uint16 cc1 = MAX((uint8)SATURATE((((fabs((float)SATURATE(potBuffer.tempo, 4024) - 4024.0))/4024.0)*127.0), 127.0), 1);
            uint16 cc2 = MAX((uint8)SATURATE((((fabs((float)SATURATE(potBuffer.gate, 4024) - 4024.0))/4024.0)*127.0), 127.0), 1);
            uint16 cc3 = MAX((uint8)SATURATE((((fabs((float)SATURATE(potBuffer.dummy1, 4024) - 4024.0))/4024.0)*127.0), 127.0), 1);
            uint16 cc4 = MAX((uint8)SATURATE((((fabs((float)SATURATE(potBuffer.dummy2, 4024) - 4024.0))/4024.0)*127.0), 127.0), 1);
            uint16 tempo_percent = 50;
        #endif
        #endif
        
        stepTime_ms = CalculateStepTimeMs(tempo_percent);
        
        // ----- reading Buttons -----
        ReadButtons();


        // ------ send MIDI command ---------
    }   
}

 /************************************************************************************************************************
 * Private function bodies
 ***********************************************************************************************************************/

void MainInit()
{
    // peripheral init --------------------------------

    //HAL_ADC_Start_DMA(&hadc1, (uint32*)&potBuffer, 4);
    //HAL_TIM_Base_Start_IT(&htim1);

    //HAL_TIM_Base_Start(&htim2);
    __HAL_DBGMCU_FREEZE_TIM2();

    //HAL_UART_Receive_IT(&huart1, midiRxBuffer, 3);

    tm1637_SetUsDelayFunction(DelayUs);

    // app init --------------------------------------

    MIDI_SendAllNotesOff(0);
    tm1637_SetModeAndValue(0, 1, 0);

    // OS ------------------------------------------

    OS_CPU_SR cpu_sr = 0;
    
    //OS_ENTER_CRITICAL();
    //OSInit();
    //OSTaskCreate(InitTask, (void*)0, &taskStackInit[127], 0);
    //OS_EXIT_CRITICAL();
    //OSStart();
    //int asd = 5;
}


/**
 * @brief Reads through the multiplexed buttons, and updates the application state accordingly
 */
void ReadButtons()
{
    bool shift = false;
    if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(SHIFT_GPIO_Port, SHIFT_Pin))
    {
        shift = true;
    }
    else
    {
        shift = false;
    }

    for (int i = 0; i < 16; i++)
    {
        uint8 i_btn = (i < 8) ? (i + 8) : (i - 8);
        HAL_GPIO_WritePin(BTN_SEL0_GPIO_Port, BTN_SEL0_Pin, ((i_btn>>0) & 1));
        HAL_GPIO_WritePin(BTN_SEL1_GPIO_Port, BTN_SEL1_Pin, ((i_btn>>1) & 1));
        HAL_GPIO_WritePin(BTN_SEL2_GPIO_Port, BTN_SEL2_Pin, ((i_btn>>2) & 1));
        HAL_GPIO_WritePin(BTN_SEL3_GPIO_Port, BTN_SEL3_Pin, ((i_btn>>3) & 1));
        
        if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(BTN_IN_GPIO_Port, BTN_IN_Pin))
        {
            if (false == buttonsPressed[i])
            {
                buttonsPressed[i] = true;

                if(true == shift)
                {
                    switch(i)
                    {
                        case 0:
                        {
                            ChangeMode(MODE_PITCH);
                            break;
                        }
                        default:
                        {
                            ChangeMode(MODE_DEFAULT);
                        }
                    }
                }
                else
                {
                    switch(currentMode)
                    {
                        case MODE_PITCH:
                        {
                            selectedStep = i;
                            dispUpdate = true;
                            break;
                        }
                        default:
                        {
                            activeSteps[i] = !activeSteps[i];
                            ledUpdate = true;
                        }
                    }
                }
            }
        }
        else
        {
            if (true == buttonsPressed[i])
            {
                buttonsPressed[i] = false;
            }
        }
    }
}



 /************************************************************************************************************************
 * Global function bodies
 ***********************************************************************************************************************/

/**
 * @brief Implements a delay in us.
 */
void DelayUs(uint16_t time)
{
	uint16_t time0 = TIM2->CNT;
	uint16_t time1;
	uint16_t deltaTime = 0;
	do {
		time1 = TIM2->CNT;
		if(time0 <= time1)
		{
			deltaTime = time1 - time0;
		}
		else
		{
			deltaTime = (0xFFFF - time0) + time1;
		}
	} while (deltaTime < time);
}


void ChangeMode(Mode_e mode)
{
    if(mode != currentMode)
    {
        currentMode = mode;
    }
    
    if(MODE_PITCH == mode)
    {
        selectedStep = 0xDD;
    }

    dispUpdate = true;
    ledUpdate = true;
}


void RotInput(Direction_e dir)
{
    switch(currentMode)
    {
        case MODE_PITCH:
        {
            if(selectedStep <= 15)
            {
                if(DIR_LEFT == dir)
                {
                    stepPitch[selectedStep]--;
                }
                if(DIR_RIGHT == dir)
                {
                    stepPitch[selectedStep]++;
                }
                dispUpdate = true;
            }
            break;
        }
        default:
        {}
    }
}