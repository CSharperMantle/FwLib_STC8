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

#ifndef __FW_DS3231_H__
#define __FW_DS3231_H__


#include "fw_hal.h"

#define DS3231_I2C_ADDR                 0xD0        /**< iic device address */ 

#define DS3231_REG_SECOND               0x00        /**< second register */
#define DS3231_REG_MINUTE               0x01        /**< minute register */
#define DS3231_REG_HOUR                 0x02        /**< hour register */
#define DS3231_REG_WEEK                 0x03        /**< week register */
#define DS3231_REG_DATE                 0x04        /**< date register */
#define DS3231_REG_MONTH                0x05        /**< month register */
#define DS3231_REG_YEAR                 0x06        /**< year register */
#define DS3231_REG_ALARM1_SECOND        0x07        /**< alarm1 second register */
#define DS3231_REG_ALARM1_MINUTE        0x08        /**< alarm1 minute register */
#define DS3231_REG_ALARM1_HOUR          0x09        /**< alarm1 hour register */
#define DS3231_REG_ALARM1_WEEK          0x0A        /**< alarm1 week register */
#define DS3231_REG_ALARM2_MINUTE        0x0B        /**< alarm2 minute register */
#define DS3231_REG_ALARM2_HOUR          0x0C        /**< alarm2 hour register */
#define DS3231_REG_ALARM2_WEEK          0x0D        /**< alarm2 week register */
#define DS3231_REG_CONTROL              0x0E        /**< control register */
#define DS3231_REG_STATUS               0x0F        /**< status register */
#define DS3231_REG_XTAL                 0x10        /**< xtal register */
#define DS3231_REG_TEMPERATUREH         0x11        /**< temperature high register */
#define DS3231_REG_TEMPERATUREL         0x12        /**< temperature low register */


typedef enum
{
    DS3231_BOOL_FALSE = 0x00,        /**< disable function */
    DS3231_BOOL_TRUE  = 0x01,        /**< enable function */
} DS3231_Bool_t;

typedef enum
{
    DS3231_ALARM_1 = 0x00,        /**< alarm 1 */
    DS3231_ALARM_2 = 0x01,        /**< alarm 2 */
} DS3231_Alarm_t;

typedef enum
{
    DS3231_AM = 0x00,        /**< am */
    DS3231_PM = 0x01,        /**< pm */
} DS3231_AmPm_t;

typedef enum
{
    DS3231_PIN_SQUARE_WAVE = 0x00,        /**< square wave pin */
    DS3231_PIN_INTERRUPT   = 0x01,        /**< interrupt pin */
} DS3231_PinType_t;

typedef enum
{
    DS3231_FORMAT_12H = 0x01,        /**< 12h format */
    DS3231_FORMAT_24H = 0x00,        /**< 24h format */
} DS3231_HourFormat_t;

typedef enum
{
    DS3231_STATUS_ALARM_2 = (1 << 1),        /**< alarm 2 status */
    DS3231_STATUS_ALARM_1 = (1 << 0),        /**< alarm 1 status */
} DS3231_AlarmStatus_t;

typedef enum
{
    DS3231_ALARM1_MODE_ONCE_A_SECOND                 = 0x0F,        /**< interrupt once a second */
    DS3231_ALARM1_MODE_SECOND_MATCH                  = 0x0E,        /**< interrupt second match */
    DS3231_ALARM1_MODE_MINUTE_SECOND_MATCH           = 0x0C,        /**< interrupt minute second match */
    DS3231_ALARM1_MODE_HOUR_MINUTE_SECOND_MATCH      = 0x08,        /**< interrupt hour minute second match */
    DS3231_ALARM1_MODE_DATE_HOUR_MINUTE_SECOND_MATCH = 0x00,        /**< interrupt date hour minute second match */
    DS3231_ALARM1_MODE_WEEK_HOUR_MINUTE_SECOND_MATCH = 0x10,        /**< interrupt week hour minute second match */
} DS3231_Alarm1Mode_t;

typedef enum
{
    DS3231_ALARM2_MODE_ONCE_A_MINUTE                = 0x07,        /**< interrupt once a minute */
    DS3231_ALARM2_MODE_MINUTE_MATCH                 = 0x06,        /**< interrupt minute match */
    DS3231_ALARM2_MODE_HOUR_MINUTE_MATCH            = 0x04,        /**< interrupt hour minute match */
    DS3231_ALARM2_MODE_DATE_HOUR_MINUTE_MATCH       = 0x00,        /**< interrupt data hour minute match */
    DS3231_ALARM2_MODE_WEEK_HOUR_MINUTE_MATCH       = 0x10,        /**< interrupt week hour minute match */
} DS3231_Alarm2Mode_t;

typedef struct ds3231_time_s
{
    uint16_t year;                 /**< year */
    uint8_t month;                 /**< month */
    uint8_t week;                  /**< week */
    uint8_t date;                  /**< date */
    uint8_t hour;                  /**< hour */
    uint8_t minute;                /**< minute */
    uint8_t second;                /**< second */
    DS3231_HourFormat_t format;    /**< hour format */
    DS3231_AmPm_t am_pm;           /**< am pm */
} DS3231_Time_t;

typedef struct ds3231_handle_s
{
    uint8_t (*iic_init)(void);                                                          /**< point to a iic_init function address */
    uint8_t (*iic_deinit)(void);                                                        /**< point to a iic_deinit function address */
    uint8_t (*iic_write)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);        /**< point to a iic_write function address */
    uint8_t (*iic_read)(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);         /**< point to a iic_read function address */
    void (*debug_print)(const char *const fmt, ...);                                    /**< point to a debug_print function address */
    void (*receive_callback)(uint8_t type);                                             /**< point to a receive_callback function address */
    void (*delay_ms)(uint32_t ms);                                                      /**< point to a delay_ms function address */
    uint8_t inited;                                                                     /**< inited flag */
} ds3231_handle_t;


uint8_t DS3231_Init(void);
uint8_t DS3231_GetTime(DS3231_Time_t *t);
uint8_t DS3231_ReadAll(uint8_t *buf);


#endif
