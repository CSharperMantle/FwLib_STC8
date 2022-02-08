// Copyright 2021 IOsetting <iosetting(at)outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "fw_hal.h"
#include <string.h>

__CODE uint8_t  DEVICEDESC[18];
__CODE uint8_t  CONFIGDESC[41];
__CODE uint8_t  HIDREPORTDESC[27];
__CODE uint8_t  LANGIDDESC[4];
__CODE uint8_t  MANUFACTDESC[8];
__CODE uint8_t  PRODUCTDESC[30];
__XDATA uint8_t HidFreature[64];
__XDATA uint8_t HidInput[64];
__XDATA uint8_t HidOutput[64];
usb_request_t usb_request;
EP0STAGE Ep0Stage;

void USB_Init();
uint8_t CalCheckSum(uint8_t *buf, uint8_t len);

void main()
{
    GPIO_P3_SetMode(GPIO_Pin_0|GPIO_Pin_1, GPIO_Mode_Input_HIP);
    USB_Init();
    EXTI_Global_SetIntState(HAL_State_ON);

    memset(HidOutput, 0x00, sizeof(HidOutput));
    HidOutput[0] = 0xaa;
    HidOutput[1] = 0x55;
    HidOutput[2] = 0x02;
    HidOutput[3] = 0x01;
    HidOutput[4] = 0x02;
    HidOutput[5] = 0x01;
    HidOutput[6] = 0x02;
    HidOutput[7] = CalCheckSum(HidOutput,7);

    while (1);
}

void USB_Init()
{
    SYS_EnableOscillator48M();
    USB_SetClockSource(USB_ClockSource_6M);
    USB_SetEnabled(HAL_State_ON);
    USB_SetDpDmPullUp(HAL_State_ON);
    EXTI_USB_SetIntPriority(EXTI_IntPriority_Highest);

    USB_WriteReg(FADDR, 0x00);
    USB_WriteReg(POWER, 0x08);
    USB_WriteReg(INTRIN1E, 0x3f);
    USB_WriteReg(INTROUT1E, 0x3f);
    USB_WriteReg(INTRUSBE, 0x00);
    USB_WriteReg(POWER, 0x01);
    Ep0Stage.bStage = USB_CtrlState_Idle;

    EXTI_USB_SetIntState(HAL_State_ON);
}

INTERRUPT(USB_Routine, EXTI_VectUSB)
{
    uint8_t intrusb;
    uint8_t intrin;
    uint8_t introut;
    uint8_t csr;
    uint8_t cnt;
    uint16_t len;
    intrusb = USB_ReadReg(INTRUSB);
    intrin = USB_ReadReg(INTRIN1);
    introut = USB_ReadReg(INTROUT1);
    if (intrusb & RSTIF)
    {
        USB_SelectEndPoint(1);
        USB_WriteReg(INCSR1, INCLRDT);
        USB_SelectEndPoint(1);
        USB_WriteReg(OUTCSR1, OUTCLRDT);
        Ep0Stage.bStage = USB_CtrlState_Idle;
    }
    if (intrin & EP0IF)
    {
        USB_SelectEndPoint(0);
        csr = USB_ReadReg(CSR0);
        if (csr & STSTL)
        {
            USB_WriteReg(CSR0, csr & ~STSTL);
            Ep0Stage.bStage = USB_CtrlState_Idle;
        }
        if (csr & SUEND)
        {
            USB_WriteReg(CSR0, csr | SSUEND);
        }
        switch (Ep0Stage.bStage)
        {
            case USB_CtrlState_Idle:
                if (csr & OPRDY)
                {
                    Ep0Stage.bStage = USB_CtrlState_SettingUp;
                    USB_ReadFIFO(FIFO0, (uint8_t *)&usb_request);
                    ((uint8_t *)&Ep0Stage.wResidue)[0] = usb_request.wLength.bb.bh;
                    ((uint8_t *)&Ep0Stage.wResidue)[1] = usb_request.wLength.bb.bl;
                    switch (usb_request.bmRequestType & REQUEST_TYPE_MASK)
                    {
                        case USB_RequestType_Standard:
                            switch (usb_request.bRequest)
                            {
                                case USB_StdReq_SetAddress:
                                    USB_WriteReg(FADDR, usb_request.wValue.bb.bl);
                                    break;
                                
                                case USB_StdReq_SetConfiguration:
                                    USB_SelectEndPoint(1);
                                    USB_WriteReg(INCSR2, INMODEIN);
                                    USB_WriteReg(INMAXP, 8);
                                    USB_SelectEndPoint(1);
                                    USB_WriteReg(INCSR2, INMODEOUT);
                                    USB_WriteReg(OUTMAXP, 8);
                                    USB_SelectEndPoint(0);
                                    break;
                                
                                case USB_StdReq_GetDescriptor:
                                    Ep0Stage.bStage = USB_CtrlState_DataIn;
                                    switch (usb_request.wValue.bb.bh)
                                    {
                                        case USB_DescriptorType_Device:
                                            Ep0Stage.pData = DEVICEDESC;
                                            len = sizeof(DEVICEDESC);
                                            break;

                                        case USB_DescriptorType_Configuration:
                                            Ep0Stage.pData = CONFIGDESC;
                                            len = sizeof(CONFIGDESC);
                                            break;

                                        case USB_DescriptorType_String:
                                            switch (usb_request.wValue.bb.bl)
                                            {
                                                case 0:
                                                    Ep0Stage.pData = LANGIDDESC;
                                                    len = sizeof(LANGIDDESC);
                                                    break;

                                                case 1:
                                                    Ep0Stage.pData = MANUFACTDESC;
                                                    len = sizeof(MANUFACTDESC);
                                                    break;

                                                case 2:
                                                    Ep0Stage.pData = PRODUCTDESC;
                                                    len = sizeof(PRODUCTDESC);
                                                    break;

                                                default:
                                                    Ep0Stage.bStage = USB_CtrlState_Stalled;
                                                    break;
                                            }
                                            break;

                                        case USB_DescriptorType_Report:
                                            Ep0Stage.pData = HIDREPORTDESC;
                                            len = sizeof(HIDREPORTDESC);
                                            break;

                                        default:
                                            Ep0Stage.bStage = USB_CtrlState_Stalled;
                                            break;
                                    }
                                    if (len < Ep0Stage.wResidue)
                                    {
                                        Ep0Stage.wResidue = len;
                                    }
                                    break;

                                default:
                                    Ep0Stage.bStage = USB_CtrlState_Stalled;
                                    break;
                            }
                            break;

                        case USB_RequestType_Class:
                            switch (usb_request.bRequest)
                            {
                                case USB_HidReq_GetReport:
                                    Ep0Stage.pData = HidFreature;
                                    Ep0Stage.bStage = USB_CtrlState_DataIn;
                                    break;

                                case USB_HidReq_SetReport:
                                    Ep0Stage.pData = HidFreature;
                                    Ep0Stage.bStage = USB_CtrlState_DataOut;
                                    break;

                                case USB_HidReq_SetIdle:
                                    break;

                                case USB_HidReq_GetIdle:
                                case USB_HidReq_GetProtocol:
                                case USB_HidReq_SetProtocol:
                                default:
                                    Ep0Stage.bStage = USB_CtrlState_Stalled;
                                    break;
                            }
                            break;
                            
                        default:
                            Ep0Stage.bStage = USB_CtrlState_Stalled;
                            break;
                    }

                    switch (Ep0Stage.bStage)
                    {
                        case USB_CtrlState_DataIn:
                            USB_WriteReg(CSR0, SOPRDY);
                            goto L_Ep0SendData;
                            break;

                        case USB_CtrlState_DataOut:
                            USB_WriteReg(CSR0, SOPRDY);
                            break;

                        case USB_CtrlState_SettingUp:
                            USB_WriteReg(CSR0, SOPRDY | DATEND);
                            Ep0Stage.bStage = USB_CtrlState_Idle;
                            break;

                        case USB_CtrlState_Stalled:
                            USB_WriteReg(CSR0, SOPRDY | SDSTL);
                            Ep0Stage.bStage = USB_CtrlState_Idle;
                            break;
                    }
                }
                break;

            case USB_CtrlState_DataIn:
                if (!(csr & IPRDY))
                {
                    L_Ep0SendData:
                    cnt = Ep0Stage.wResidue > 64 ? 64 : Ep0Stage.wResidue;
                    USB_WriteFIFO(FIFO0, Ep0Stage.pData, cnt);
                    Ep0Stage.wResidue -= cnt;
                    Ep0Stage.pData += cnt;
                    if (Ep0Stage.wResidue == 0)
                    {
                        USB_WriteReg(CSR0, IPRDY | DATEND);
                        Ep0Stage.bStage = USB_CtrlState_Idle;
                    }
                    else
                    {
                        USB_WriteReg(CSR0, IPRDY);
                    }
                }
                break;

            case USB_CtrlState_DataOut:
                if (csr & OPRDY)
                {
                    cnt = USB_ReadFIFO(FIFO0, Ep0Stage.pData);
                    Ep0Stage.wResidue -= cnt;
                    Ep0Stage.pData += cnt;
                    if (Ep0Stage.wResidue == 0)
                    {
                        USB_WriteReg(CSR0, SOPRDY | DATEND);
                        Ep0Stage.bStage = USB_CtrlState_Idle;
                    }
                    else
                    {
                        USB_WriteReg(CSR0, SOPRDY);
                    }
                }
                break;
        }
    }
    
    if (intrin & EP1INIF)
    {
        USB_SelectEndPoint(1);
        csr = USB_ReadReg(INCSR1);
        if (csr & INSTSTL)
        {
            USB_WriteReg(INCSR1, INCLRDT);
        }
        if (csr & INUNDRUN)
        {
            USB_WriteReg(INCSR1, 0);
        }
    }
    
    if (introut & EP1OUTIF)
    {
        USB_SelectEndPoint(1);
        csr = USB_ReadReg(OUTCSR1);
        if (csr & OUTSTSTL)
        {
            USB_WriteReg(OUTCSR1, OUTCLRDT);
        }
        if (csr & OUTOPRDY)
        {
            USB_ReadFIFO(FIFO1, HidInput);
            USB_WriteReg(OUTCSR1, 0);
            
            if((HidInput[0]==0xaa) && (HidInput[1]==0x55) && (HidInput[2]==0x01))
            {
                USB_SelectEndPoint(1);
                USB_WriteFIFO(FIFO1, HidOutput, 64);
                USB_WriteReg(INCSR1, INIPRDY);
            }
        }
    }
}

__CODE uint8_t DEVICEDESC[18] =
{
    0x12,       //bLength(18);
    0x01,       //bDescriptorType(Device);
    0x00,0x02,  //bcdUSB(2.00);
    0x00,       //bDeviceClass(0);
    0x00,       //bDeviceSubClass0);
    0x00,       //bDeviceProtocol(0);
    0x40,       //bMaxPacketSize0(64);
    0x54,0x53,  //idVendor(5354);
    0x80,0x43,  //idProduct(4380);
    0x00,0x01,  //bcdDevice(1.00);
    0x01,       //iManufacturer(1);
    0x02,       //iProduct(2);
    0x00,       //iSerialNumber(0);
    0x01,       //bNumConfigurations(1);
};

__CODE uint8_t CONFIGDESC[41] =
{
    0x09,       //bLength(9);
    0x02,       //bDescriptorType(Configuration);
    0x29,0x00,  //wTotalLength(41);
    0x01, //bNumInterfaces(1);
    0x01, //bConfigurationValue(1);
    0x00, //iConfiguration(0);
    0x80, //bmAttributes(BUSPower);
    0x32, //MaxPower(100mA);
    0x09, //bLength(9);
    0x04, //bDescriptorType(Interface);
    0x00, //bInterfaceNumber(0);
    0x00, //bAlternateSetting(0);
    0x02, //bNumEndpoints(2);
    0x03, //bInterfaceClass(HID);
    0x00, //bInterfaceSubClass(0);
    0x00, //bInterfaceProtocol(0);
    0x00, //iInterface(0);
    0x09, //bLength(9);
    0x21, //bDescriptorType(HID);
    0x01,0x01, //bcdHID(1.01);
    0x00, //bCountryCode(0);
    0x01, //bNumDescriptors(1);
    0x22, //bDescriptorType(HID Report);
    0x1b,0x00, //wDescriptorLength(27);
    0x07, //bLength(7);
    0x05, //bDescriptorType(Endpoint);
    0x81, //bEndpointAddress(EndPoint1 as IN);
    0x03, //bmAttributes(Interrupt);
    0x40,0x00, //wMaxPacketSize(64);
    0x01, //bInterval(10ms);
    0x07, //bLength(7);
    0x05, //bDescriptorType(Endpoint);
    0x01, //bEndpointAddress(EndPoint1 as OUT);
    0x03, //bmAttributes(Interrupt);
    0x40,0x00, //wMaxPacketSize(64);
    0x01, //bInterval(10ms);
};

__CODE uint8_t HIDREPORTDESC[27] =
{
    0x05,0x0c, //USAGE_PAGE(Consumer);
    0x09,0x01, //USAGE(Consumer Control);
    0xa1,0x01, //COLLECTION(Application);
    0x15,0x00, // LOGICAL_MINIMUM(0);
    0x25,0xff, // LOGICAL_MAXIMUM(255);
    0x75,0x08, // REPORT_SIZE(8);
    0x95,0x40, // REPORT_COUNT(64);
    0x09,0x01, // USAGE(Consumer Control);
    0xb1,0x02, // FEATURE(Data,Variable);
    0x09,0x01, // USAGE(Consumer Control);
    0x81,0x02, // INPUT(Data,Variable);
    0x09,0x01, // USAGE(Consumer Control);
    0x91,0x02, // OUTPUT(Data,Variable);
    0xc0, //END_COLLECTION;
};

__CODE uint8_t LANGIDDESC[4] =
{
    0x04,0x03,
    0x09,0x04,
};

__CODE uint8_t MANUFACTDESC[8] =
{
    0x08,0x03,
    'S',0,
    'T',0,
    'C',0,
};

__CODE uint8_t PRODUCTDESC[30] =
{
    0x1e,0x03,
    'S',0,
    'T',0,
    'C',0,
    '8',0,
    ' ',0,
    'F',0,
    'w',0,
    'L',0,
    'i',0,
    'b',0,
    ' ',0,
    'H',0,
    'I',0,
    'D',0,
};

uint8_t CalCheckSum(uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t cs=0;
    for (i = 0; i < len; i++)
    {
        cs += buf[i];
    }
    return cs;
}