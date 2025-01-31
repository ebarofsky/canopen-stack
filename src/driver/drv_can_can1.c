/******************************************************************************
   Copyright 2020 Embedded Office GmbH & Co. KG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "drv_can_can1.h"
#include "stm32f4xx_hal.h"

/******************************************************************************
* PRIVATE TYPE DEFINITION
******************************************************************************/

typedef struct BAUDRATE_TBL_T {
    uint32_t Baudrate;
    uint32_t Prescaler;
    uint32_t SyncJumpWidth;
    uint32_t TimeSeg1;
    uint32_t TimeSeg2;
} BAUDRATE_TBL;

typedef struct PIN_ASSIGN_T {
    GPIO_TypeDef *Port;
    uint16_t      Pin; 
    uint8_t       Alternate;
} PIN_ASSIGN;

extern uint8_t CANRxCount;
extern void MX_CAN1_Init(void);

/******************************************************************************
* PRIVATE DEFINES
******************************************************************************/

/* default pin assignment: CAN_RX -> PB8, CAN_TX -> PB9 */
#define CAN1_PIN_RX_SEL  0
#define CAN1_PIN_TX_SEL  0

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

static PIN_ASSIGN Can1Pin_Rx[] = {
    { GPIOA, GPIO_PIN_11, GPIO_AF9_CAN1 },  /* #0: PA11 */
    { GPIOB, GPIO_PIN_8,  GPIO_AF9_CAN1 },  /* #1: PB8  */
    { GPIOD, GPIO_PIN_0,  GPIO_AF9_CAN1 },  /* #3: PD0  */
    { GPIOH, GPIO_PIN_14, GPIO_AF9_CAN1 },  /* #4: PH14 */
};
static PIN_ASSIGN Can1Pin_Tx[] = {
    { GPIOA, GPIO_PIN_12, GPIO_AF9_CAN1 },  /* #0: PA12 */
    { GPIOB, GPIO_PIN_9,  GPIO_AF9_CAN1 },  /* #1: PB9  */
    { GPIOD, GPIO_PIN_1,  GPIO_AF9_CAN1 },  /* #2: PD1  */
    { GPIOH, GPIO_PIN_13, GPIO_AF9_CAN1 }   /* #3: PH13 */
};

static BAUDRATE_TBL BaudrateTbl[] = {
//    {   10000, 250, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_2TQ },  /* SP: 88,9%, ERR:     0% */
//    {   20000, 125, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_2TQ },  /* SP: 88,9%, ERR:     0% */
//    {   50000,  60, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_2TQ },  /* SP: 88,9%, ERR:     0% */
    {  500000,   4, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_2TQ },  /* SP: 88,9%, ERR:     0% */
    {  125000,  18, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ },  /* SP: 88,9%, ERR:     0% */
    {  250000,   9, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ },  /* SP: 88,9%, ERR:     0% */

//    {  800000,   3, CAN_SJW_1TQ, CAN_BS1_12TQ, CAN_BS2_3TQ },  /* SP: 84,2%, ERR: -1,32% */
//    { 1000000,   2, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_2TQ },  /* SP: 86,7%, ERR:     0% */
    { 0, 0, 0, 0, 0 }
};

static CAN_HandleTypeDef DrvCan1;

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void    DrvCanInit   (void);
static int16_t DrvCanEnable (uint32_t);
static int16_t DrvCanSend   (CO_IF_FRM *frm);
static int16_t DrvCanRead   (CO_IF_FRM *frm);
static void    DrvCanReset  (void);
static void    DrvCanClose  (void);

/******************************************************************************
* PUBLIC VARIABLE
******************************************************************************/

const CO_IF_CAN_DRV STM32F4xx_CAN1_CanDriver = {
    DrvCanInit,
    DrvCanEnable,
    DrvCanRead,
    DrvCanSend,
    DrvCanReset,
    DrvCanClose
};

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

///* ST HAL CAN Receive Interrupt Handler */
//void CAN1_RX0_IRQHandler(void)
//{
//    HAL_CAN_IRQHandler(&DrvCan1);
//}

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void DrvCanInit(void)
{
/*
 * Init covered
 */
//    GPIO_InitTypeDef gpio = {0};
//
//    /* Peripheral clocks enable (for simplicity: enable all possible ports) */
//    __HAL_RCC_CAN1_CLK_ENABLE();
//    __HAL_RCC_GPIOA_CLK_ENABLE();
////    __HAL_RCC_GPIOB_CLK_ENABLE();
////    __HAL_RCC_GPIOD_CLK_ENABLE();
////    __HAL_RCC_GPIOH_CLK_ENABLE();
////
//    /* setup CAN RX and TX pins */
//    gpio.Mode      = GPIO_MODE_AF_PP;
//    gpio.Pull      = GPIO_NOPULL;
//    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
//    gpio.Alternate = Can1Pin_Rx[CAN1_PIN_RX_SEL].Alternate;
//    gpio.Pin       = Can1Pin_Rx[CAN1_PIN_RX_SEL].Pin;
//    HAL_GPIO_Init(Can1Pin_Rx[CAN1_PIN_RX_SEL].Port, &gpio);
//    gpio.Alternate = Can1Pin_Rx[CAN1_PIN_TX_SEL].Alternate;
//    gpio.Pin       = Can1Pin_Tx[CAN1_PIN_TX_SEL].Pin;
//    HAL_GPIO_Init(Can1Pin_Tx[CAN1_PIN_TX_SEL].Port, &gpio);
//
//    /* CAN interrupt init */
//    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

static int16_t DrvCanEnable(uint32_t baudrate)
{
    uint8_t idx = 0, timeoutCount = 0, lastCount = 0, baudFound = false;
    HAL_StatusTypeDef err, lastVal = HAL_ERROR;
    CAN_RxHeaderTypeDef frmHead;
    uint8_t frmData[8] = { 0 };

    //TODO: add in function to have this not be autobaud if desired
//    if(baudrate > 0)
//    {
//        /* find the given baudrate in baudrate table */
//        while (BaudrateTbl[idx].Baudrate != 0)
//        {
//            if (baudrate == BaudrateTbl[idx].Baudrate) {
//                break;
//            }
//            idx++;
//
//            if (baudrate != BaudrateTbl[idx].Baudrate) {
//                while(1);    /* error not handled */
//            }
//        }
//    }
//    else
//    {

    while (BaudrateTbl[idx].Baudrate != 0)
    {
        /* can controller mode */
        DrvCan1.Instance  = CAN1;
        DrvCan1.Init.Mode = CAN_MODE_SILENT;

        /* baudrate settings */
        DrvCan1.Init.Prescaler     = BaudrateTbl[idx].Prescaler;
        DrvCan1.Init.SyncJumpWidth = BaudrateTbl[idx].SyncJumpWidth;
        DrvCan1.Init.TimeSeg1      = BaudrateTbl[idx].TimeSeg1;
        DrvCan1.Init.TimeSeg2      = BaudrateTbl[idx].TimeSeg2;

        /* feature select */
        DrvCan1.Init.TimeTriggeredMode    = DISABLE;
        DrvCan1.Init.AutoBusOff           = DISABLE;
        DrvCan1.Init.AutoWakeUp           = DISABLE;
        DrvCan1.Init.AutoRetransmission   = DISABLE;
        DrvCan1.Init.ReceiveFifoLocked    = DISABLE;
        DrvCan1.Init.TransmitFifoPriority = DISABLE;

        HAL_CAN_Init(&DrvCan1);
        HAL_CAN_Start(&DrvCan1);

        //2s timeout per bitrate
        while(timeoutCount < 200)
        {
            //If we have received any messages
            if(CANRxCount > 0)
            {
                // If we have a fresh message
                if(lastCount != CANRxCount)
                {
                    err = HAL_CAN_GetRxMessage(&DrvCan1, CAN_RX_FIFO0, &frmHead, &frmData[0]);
                    if (err == HAL_OK)
                    {
                        if(DrvCan1.ErrorCode == HAL_CAN_ERROR_NONE)
                        {
                            //Found the baudrate with no error code
                            baudFound = true;
                            break;
                        }
                    }
                    lastCount = CANRxCount;
                }
            }
            HAL_Delay(10);
            timeoutCount++;
        }

        if(baudFound == false)
        {
            idx++;
        }
        else
        {
            break;
        }
    }

    if(BaudrateTbl[idx].Baudrate != 0)
    {
        HAL_CAN_DeInit(&DrvCan1);

        DrvCan1.Init.Mode = CAN_MODE_NORMAL;

        HAL_CAN_Init(&DrvCan1);
        HAL_CAN_Start(&DrvCan1);

        return(0);
    }
    else
    {
        return(-1);
    }
//    }
}

static int16_t DrvCanSend(CO_IF_FRM *frm)
{
    HAL_StatusTypeDef   result;
    CAN_TxHeaderTypeDef frmHead;
    uint32_t            mailbox;

    /* RTR is not supported */
    frmHead.RTR   = 0;

    /* extended identifiers are not supported */
    frmHead.ExtId = 0;
    frmHead.IDE   = 0;

    /* fill identifier, DLC and data payload in transmit buffer */
    frmHead.StdId = frm->Identifier;
    frmHead.DLC   = frm->DLC;
    result = HAL_CAN_AddTxMessage(&DrvCan1, &frmHead, &frm->Data[0], &mailbox);
    if (result != HAL_OK) {
        return (-1);
    }
    return (0u);
}

static int16_t DrvCanRead (CO_IF_FRM *frm)
{
    HAL_StatusTypeDef err;
    CAN_RxHeaderTypeDef frmHead;
    uint8_t frmData[8] = { 0 };
    uint8_t n;

    err = HAL_CAN_GetRxMessage(&DrvCan1, CAN_RX_FIFO0, &frmHead, &frmData[0]);
    if (err != HAL_OK) {
        return (-1);
    }

    /* fill CAN frame on success */
    frm->Identifier  = frmHead.StdId;
    frm->DLC         = frmHead.DLC;
    for (n = 0; n < 8; n++) {
        frm->Data[n] = frmData[n];
    }
    return (frm->DLC);
}

static void DrvCanReset(void)
{
    HAL_CAN_Init(&DrvCan1);
    HAL_CAN_Start(&DrvCan1);
}

static void DrvCanClose(void)
{
    HAL_CAN_Stop(&DrvCan1);
}
