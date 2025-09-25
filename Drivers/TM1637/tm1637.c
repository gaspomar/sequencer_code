/*****************************************************************************************************************************************************
 * Driver for the serial interface of the TM1637 7segment display driver
 * 
 * Usage:
 *      1. Initialize the CLK and DIO pins of your microcontroller as follows:
 *          CLK: GPIO output push-pull
 *          DIO: PIO output open drain
 *      2. Define CLK_PORT, CLK_PIN, DIO_PORT and DIO_PIN below
 *      3. Set usDelayFunction
 * 
 *****************************************************************************************************************************************************/

#include "tm1637.h"
#include "main.h"       // fot accessing the user defined macros
#include "string.h"
#include <stdio.h>

#define TM1637_CLK_PORT    DISP_SCK_GPIO_Port
#define TM1637_CLK_PIN     DISP_SCK_Pin

#define TM1637_DIO_PORT    DISP_DIO_GPIO_Port
#define TM1637_DIO_PIN     DISP_DIO_Pin


 /************************************************************************************************************************
 * Types
 ***********************************************************************************************************************/
typedef enum {
    WRITE, READ
} DIO_Direction_e;


 /************************************************************************************************************************
 * Local variables
 ***********************************************************************************************************************/
static void (*DelayUs)(uint16_t timeUs);


 /************************************************************************************************************************
 * Local function prototypes
 ***********************************************************************************************************************/

static void tm1637_ReinitDIO(DIO_Direction_e dir);

static void tm1637_CLKhigh();
static void tm1637_CLKlow();
static void tm1637_DIOhigh();
static void tm1637_DIOlow();

static void tm1637_WaitForACK();

static void tm1637_StartPacket();
static void tm1637_StopPacket();

static void tm1637_SendPacket(uint8_t data);

static uint8_t tm1637_ConvertDecimalToData(uint8_t decimal);
static uint8_t tm1637_ConvertCharToData(char character);

static void tm1637_SetData(uint8_t data[4]);


 /************************************************************************************************************************
 * Interface function implementations
 ***********************************************************************************************************************/

void tm1637_SetUsDelayFunction(void (*DelayFunction)(uint16_t))
{
   DelayUs = DelayFunction;
}


uint8_t tm1637_ConvertDecimalToData(uint8_t decimal)
{
    // :gcbafed
    uint8_t returnData = 0;
    switch(decimal)
    {
        case 0:
        {
            returnData = 0b00111111;
            break;
        }
        case 1:
        {
            returnData = 0b00110000;
            break;
        }
        case 2:
        {
            returnData = 0b01011011;
            break;
        }
        case 3:
        {
            returnData = 0b01111001;
            break;
        }
        case 4:
        {
            returnData = 0b01110100;
            break;
        }
        case 5:
        {
            returnData = 0b01101101;
            break;
        }
        case 6:
        {
            returnData = 0b01101111;
            break;
        }
        case 7:
        {
            returnData = 0b00111000;
            break;
        }
        case 8:
        {
            returnData = 0b01111111;
            break;
        }
        case 9:
        {
            returnData = 0b01111101;
            break;
        }
        default:
        {}
    }
    return returnData;
}


static uint8_t tm1637_ConvertCharToData(char character)
{
    // :gcbafed
    uint8_t returnData = 0;
    switch(character)
    {
        case 'a':
        {
            returnData = 0b01111011;
            break;
        }
        case 'b':
        {
            returnData = 0b01100111;
            break;
        }
        case 'c':
        {
            returnData = 0b01000011;
            break;
        }
        case 'd':
        {
            returnData = 0b01110011;
            break;
        }
        case 'e':
        {
            returnData = 0b01001111;
            break;
        }
        case 'f':
        {
            returnData = 0b01001110;
            break;
        }
        case 'g':
        {
            returnData = 0b01111101;
            break;
        }
        case 'h':
        {
            returnData = 0b01100110;
            break;
        }
        case 'i':
        {
            //returnData = 0b00110000;
            returnData = 0b00100000;
            break;
        }
        case 'j':
        {
            returnData = 0b00110001;
            break;
        }
        case 'k':
        {
            returnData = 0b01101110;
            break;
        }
        case 'l':
        {
            returnData = 0b00000111;
            break;
        }
        case 'm':
        {
            returnData = 0b00101010;
            break;
        }
        case 'n':
        {
            returnData = 0b00111110;
            break;
        }
        case 'o':
        {
            returnData = 0b01100011;
            break;
        }
        case 'p':
        {
            returnData = 0b01011110;
            break;
        }
        case 'q':
        {
            returnData = 0b01111100;
            break;
        }
        case 'r':
        {
            returnData = 0b00011110;
            break;
        }
        case 's':
        {
            returnData = 0b01101101;
            break;
        }
        case 't':
        {
            returnData = 0b01000111;
            break;
        }
        case 'u':
        {
            returnData = 0b00110111;
            break;
        }
        case 'v':
        {
            returnData = 0b00110101;
            break;
        }
        case 'w':
        {
            returnData = 0b00010101;
            break;
        }
        case 'x':
        {
            returnData = 0b01110110;
            break;
        }
        case 'y':
        {
            returnData = 0b01110101;
            break;
        }
        case 'z':
        {
            returnData = 0b01011001;
            break;
        }
        case '0':
        {
            returnData = 0b00111111;
            break;
        }
        case '1':
        {
            returnData = 0b00110000;
            break;
        }
        case '2':
        {
            returnData = 0b01011011;
            break;
        }
        case '3':
        {
            returnData = 0b01111001;
            break;
        }
        case '4':
        {
            returnData = 0b01110100;
            break;
        }
        case '5':
        {
            returnData = 0b01101101;
            break;
        }
        case '6':
        {
            returnData = 0b01101111;
            break;
        }
        case '7':
        {
            returnData = 0b00111000;
            break;
        }
        case '8':
        {
            returnData = 0b01111111;
            break;
        }
        case '9':
        {
            returnData = 0b01111101;
            break;
        }
        case ' ':
        {
            returnData = 0b00000000;
            break;
        }
        default:
        {}
    }
    return returnData;
}


void tm1637_SetNumber(int16_t value)
{
    // TODO: error handling (range check)

    uint8_t data[4];
    uint8_t value_unsigned;
    uint8_t valueDigit[4];
    uint8_t numDigits = 4;

    uint8_t minus = (value < 0);
    if(minus)
    {
        value_unsigned = (uint8_t)(-value);
    }
    else
    {
        value_unsigned = (uint8_t)(value);
    }

    valueDigit[0] = value_unsigned % 10;
    valueDigit[1] = ((value_unsigned - valueDigit[0]) % 100) / 10;
    valueDigit[2] = (value_unsigned - valueDigit[1]*10 - valueDigit[0]) / 100;
    valueDigit[3] = (value_unsigned - valueDigit[1]*10 - valueDigit[0]) / 100;

    for(int i=3; i>0; i--)
    {
        if(0 == valueDigit[i])
        {
            numDigits--;
        }
        else
        {
            break;
        }
    }

    data[0] = (numDigits == 4) ? tm1637_ConvertDecimalToData(valueDigit[3]) : 0;
    data[1] = (numDigits >= 3) ? tm1637_ConvertDecimalToData(valueDigit[2]) : 0;
    data[2] = (numDigits >= 2) ? tm1637_ConvertDecimalToData(valueDigit[1]) : 0;
    data[3] = (numDigits >= 1) ? tm1637_ConvertDecimalToData(valueDigit[0]) : 0;
    
    if(minus)
    {
        if(numDigits < 4)
        {
            data[3-numDigits] = 0b01000000;
        }
    }

    data[0] = 0;

    tm1637_SetData(data);
}


void tm1637_SetWord(char* word, uint8_t size)
{
    uint8_t data[4];

    // TODO: error handling
    if(size > 4)
    {
        return;
    }

    data[0] = (size >= 1) ? tm1637_ConvertCharToData(word[0]) : 0;
    data[1] = (size >= 2) ? tm1637_ConvertCharToData(word[1]) : 0;
    data[2] = (size >= 3) ? tm1637_ConvertCharToData(word[2]) : 0;
    data[3] = (size >= 4) ? tm1637_ConvertCharToData(word[3]) : 0;

    tm1637_SetData(data);
}


void tm1637_SetWordAndNum(char* word, uint8_t size_word, uint8_t num)
{
    // TODO: error handling
    if(((size_word > 3) && (num > 9)) || ((size_word > 2) && (num > 99)) || ((size_word > 1) && (num > 999)) || ((size_word > 0) && (num > 9999)))
    {
        return;
    }

    uint8_t data[4];
    char numBuf[5];

    sprintf(numBuf, "%d", num);

    data[0] = (size_word >= 1) ? tm1637_ConvertCharToData(word[0]) : 0;
    data[1] = (size_word >= 2) ? tm1637_ConvertCharToData(word[1]) : 0;
    data[2] = (size_word >= 3) ? tm1637_ConvertCharToData(word[2]) : 0;
    data[3] = 0;

    data[0] = (num >= 9)    ? tm1637_ConvertCharToData(numBuf[0]) : data[0];
    data[1] = (num >= 99)   ? tm1637_ConvertCharToData(numBuf[1]) : data[1];
    data[2] = (num >= 999)  ? tm1637_ConvertCharToData(numBuf[2]) : data[2];
    data[3] = (num >= 9999) ? tm1637_ConvertCharToData(numBuf[3]) : data[3];

    tm1637_SetData(data);
}


void tm1637_SetDefault()
{
    uint8_t data[4] = {0b00001111, 0b00001001, 0b00001001, 0b00111001};
    tm1637_SetData(data);
}

/**
 * @brief Sets the display to show the current mode and value.
 * @param mode
 * @param value
 * @param showValue flag to turn off showing of value
 */
void tm1637_SetModeAndValue(uint8_t mode, int16_t value, uint8_t showValue)
{
    uint8_t data[4];
    uint8_t value_unsigned;
    uint8_t minus = (value < 0);
    if(minus)
    {
        value_unsigned = (uint8_t)(-value);
    }
    else
    {
        value_unsigned = (uint8_t)(value);
    }
    uint8_t valueDigit0 = value_unsigned % 10;
    uint8_t valueDigit1 = ((value_unsigned - valueDigit0) % 100) / 10;
    uint8_t valueDigit2 = (value_unsigned - valueDigit1*10 - valueDigit0) / 100;
    
    data[0] = tm1637_ConvertDecimalToData(mode);
    
    if(showValue)
    {
        if(valueDigit2)
        {
            data[1] = tm1637_ConvertDecimalToData(valueDigit2);
        }
        else
        {
            data[1] = 0;
        }

        if(valueDigit1)
        {
            data[2] = tm1637_ConvertDecimalToData(valueDigit1);
        }
        else
        {
            data[2] = 0;
        }

        data[3] = tm1637_ConvertDecimalToData(valueDigit0);
        
    }
    else
    {
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
    }

    if(minus)
    {
        if(data[2] == 0)
        {
            data[2] = 0b01000000;
        }
        else if(data[1] == 0)
        {
            data[1] = 0b01000000;
        }
    }

    tm1637_SetData(data);
}


void tm1637_SetData(uint8_t data[4])
{
    uint8_t commandBuff = 0;

    tm1637_StartPacket();
    commandBuff = 0x40;
    tm1637_SendPacket(commandBuff);
    tm1637_WaitForACK();
    tm1637_StopPacket();

    tm1637_StartPacket();
    commandBuff = 0xC0;
    tm1637_SendPacket(commandBuff);
    tm1637_WaitForACK();

    // sending data packets
    // digit4
    tm1637_SendPacket(data[3]);
    tm1637_WaitForACK();

    // digit3 + dots
    tm1637_SendPacket(data[2]);
    tm1637_WaitForACK();

    // digit2
    tm1637_SendPacket(data[1]);
    tm1637_WaitForACK();

    // digit1
    tm1637_SendPacket(data[0]);
    tm1637_WaitForACK();

    // unused
    tm1637_SendPacket(0);
    tm1637_WaitForACK();

    // unused
    tm1637_SendPacket(0);
    tm1637_WaitForACK();

    tm1637_StartPacket();
    commandBuff = 0x88;
    tm1637_SendPacket(commandBuff);
    tm1637_WaitForACK();

    tm1637_StopPacket();
}

 /************************************************************************************************************************
 * Local function implementations
 ***********************************************************************************************************************/

static void tm1637_CLKhigh()
{
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_SET);
}


static void tm1637_CLKlow()
{
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_RESET);
}


static void tm1637_DIOhigh()
{
    HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, GPIO_PIN_SET);
}


static void tm1637_DIOlow()
{
    HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, GPIO_PIN_RESET);
}


static void tm1637_WaitForACK()
{
    tm1637_ReinitDIO(READ);

    tm1637_CLKlow();

    DelayUs(20);
    while(GPIO_PIN_RESET != HAL_GPIO_ReadPin(TM1637_DIO_PORT, TM1637_DIO_PIN));

    tm1637_CLKhigh();
    DelayUs(20);
    tm1637_DIOlow();
    tm1637_ReinitDIO(WRITE);
    tm1637_CLKlow();

    DelayUs(20);
}


static void tm1637_StartPacket()
{
    tm1637_CLKhigh();
    DelayUs(20);
    tm1637_DIOhigh();
    DelayUs(40);

    tm1637_DIOlow();
    DelayUs(10);
    tm1637_CLKlow();
    DelayUs(10);
}


static void tm1637_StopPacket()
{
    DelayUs(20);
    tm1637_DIOlow();
    tm1637_CLKhigh();
}


static void tm1637_ReinitDIO(DIO_Direction_e dir)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    switch(dir)
    {
        case WRITE:
        {
            GPIO_InitStruct.Pin = TM1637_DIO_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            break;
        }
        case READ:
        {
            GPIO_InitStruct.Pin = TM1637_DIO_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
        }
        default:
        {}
    }

    HAL_GPIO_Init(TM1637_DIO_PORT, &GPIO_InitStruct);
}


static void tm1637_SendPacket(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        if ((data >> i) & 1)
        {
            tm1637_DIOhigh();
        }
        else
        {
            tm1637_DIOlow();
        }
        
        DelayUs(20);
        tm1637_CLKhigh();
        DelayUs(20);
        tm1637_CLKlow();
        int asd = 5;
    }
}
