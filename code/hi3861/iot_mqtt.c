/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *******************************************************************************/
#define EVENT0 car_makes_u_turn
#define EVENT1 car_picks_up_person
#define EVENT2 car_reverses
#define EVENT3 car_starts
#define EVENT4 car_stops
#define EVENT5 car_turns_left
#define EVENT6 car_turns_right
#define EVENT7 hand_interacts_with_person_highfive
#define EVENT8 hand_interacts_with_person_holdhands
#define EVENT9 hand_interacts_with_person_shakehands
#define EVENT10 motorcycle_drops_off_person
#define EVENT11 motorcycle_makes_u_turn
#define EVENT12 motorcycle_picks_up_person
#define EVENT13 motorcycle_reverses
#define EVENT14 motorcycle_starts
#define EVENT15 motorcycle_stops
#define EVENT16 motorcycle_turns_left
#define EVENT17 motorcycle_turns_right
#define EVENT18 person_abandons_bag
#define EVENT19 person_abandons_package
#define EVENT20 person_carries_heavy_object
#define EVENT21 person_closes_car_door
#define EVENT22 person_closes_car_trunk
#define EVENT23 person_closes_facility_door
#define EVENT24 person_closes_motorcycle_trunk
#define EVENT25 person_comes_into_scene_through_structure
#define EVENT26 person_embraces_person
#define EVENT27 person_enters_car
#define EVENT28 person_enters_scene_through_structure
#define EVENT29 person_exits_car
#define EVENT30 person_exits_scene_through_structure
#define EVENT31 person_holds_hand
#define EVENT32 person_interacts_with_laptop
#define EVENT33 person_leaves_scene_through_structure
#define EVENT34 person_loads_car
#define EVENT35 person_loads_motorcycle
#define EVENT36 person_opens_car_door
#define EVENT37 person_opens_car_trunk
#define EVENT38 person_opens_facility_door
#define EVENT39 person_opens_motorcycle_trunk
#define EVENT40 person_picks_up_object
#define EVENT41 person_picks_up_object_from_floor
#define EVENT42 person_picks_up_object_from_shelf
#define EVENT43 person_picks_up_object_from_table
#define EVENT44 person_purchases_from_cashier
#define EVENT45 person_purchases_from_machine
#define EVENT46 person_puts_down_object
#define EVENT47 person_puts_down_object_on_floor
#define EVENT48 person_puts_down_object_on_shelf

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MQTTClient.h"
#include "wifi_connect.h"

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_i2c.h"
#include "iot_gpio.h"
#include "iot_errno.h"

#include "oled_ssd1306.h"

#define AHT20_BAUDRATE (400 * 1000)
#define AHT20_I2C_IDX 0

#define TASK_SLEEP_1000MS (1000)

#define MQTT_SERVERIP "vackyzhao.tpddns.cn"
#define MQTT_SERVERPORT 1883
#define MQTT_CMD_TIMEOUT_MS 2000
#define MQTT_KEEP_ALIVE_MS 60
#define MQTT_DELAY_2S 200
#define MQTT_DELAY_500_MS 50
#define MQTT_VERSION 3
#define MQTT_QOS 1
#define MQTT_TASK_STACK_SIZE (1024 * 10)

#define UART_BUFF_SIZE 100
#define U_SLEEP_TIME 100000

static unsigned char sendBuf[1000];
static unsigned char readBuf[1000];

Network network;

void Uart1GpioInit(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_0);
    // 设置GPIO0的管脚复用关系为UART1_TX Set the pin reuse relationship of GPIO0 to UART1_ TX
    IoSetFunc(IOT_IO_NAME_GPIO_0, IOT_IO_FUNC_GPIO_0_UART1_TXD);
    IoTGpioInit(IOT_IO_NAME_GPIO_1);
    // 设置GPIO1的管脚复用关系为UART1_RX Set the pin reuse relationship of GPIO1 to UART1_ RX
    IoSetFunc(IOT_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_UART1_RXD);
}

void Uart1Config(void)
{
    uint32_t ret;
    /* 初始化UART配置，波特率 9600，数据bit为8,停止位1，奇偶校验为NONE */
    /* Initialize UART configuration, baud rate is 9600, data bit is 8, stop bit is 1, parity is NONE */
    IotUartAttribute uart_attr = {
        .baudRate = 9600,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };
    ret = IoTUartInit(HI_UART_IDX_1, &uart_attr);
    if (ret != IOT_SUCCESS)
    {
        printf("Init Uart1 Falied Error No : %d\n", ret);
        return;
    }
}

void messageArrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len,
           data->topicName->lenstring.data, data->message->payloadlen, data->message->payload);
}

static void MQTTDemoTask(void)
{
    (void)arg;
    OledInit();
    OledFillScreen(0);
    IoTI2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);

    OledShowString(20, 3, "Initialing......", 1); /* 屏幕第20列3行显示1行 */
    const char *uartdata = "EVENT:47\n";
    const char *ackdata = "ACK\n";
    uint32_t countUart = 0;
    uint32_t len = 0;
    unsigned char uartReadBuff[UART_BUFF_SIZE] = {0};

    // 对UART1的一些初始化 Some initialization of UART1
    Uart1GpioInit();
    // 对UART1参数的一些配置 Some configurations of UART1 parameters
    Uart1Config();
    WifiConnect("NICT4F", "nict2021");
    printf("Starting ...\n");
    int rc, count = 0;
    MQTTClient client;

    NetworkInit(&network);
    printf("NetworkConnect  ...\n");

    NetworkConnect(&network, MQTT_SERVERIP, MQTT_SERVERPORT);
    printf("MQTTClientInit  ...\n");
    MQTTClientInit(&client, &network, MQTT_CMD_TIMEOUT_MS, sendBuf, sizeof(sendBuf), readBuf, sizeof(readBuf));

    MQTTString clientId = MQTTString_initializer;
    clientId.cstring = "hi3861";

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID = clientId;
    data.username.cstring = "hi3861";
    data.password.cstring = "1234asdf";
    data.willFlag = 0;
    data.MQTTVersion = MQTT_VERSION;
    data.keepAliveInterval = MQTT_KEEP_ALIVE_MS;
    data.cleansession = 1;

    printf("MQTTConnect  ...\n");
    rc = MQTTConnect(&client, &data);
    if (rc != 0)
    {
        printf("MQTTConnect: %d\n", rc);
        NetworkDisconnect(&network);
        MQTTDisconnect(&client);
        osDelay(MQTT_DELAY_2S);
    }

    printf("MQTTSubscribe  ...\n");
    rc = MQTTSubscribe(&client, "substopic", MQTT_QOS, messageArrived);
    if (rc != 0)
    {
        printf("MQTTSubscribe: %d\n", rc);
        osDelay(MQTT_DELAY_2S);
    }
    while (1)
    {
        // 通过UART1 发送数据 Send data through UART1
        // IoTUartWrite(HI_UART_IDX_1, (unsigned char*)uartdata, strlen(uartdata));
        //  通过UART1 接收数据 Receive data through UART1
        len = IoTUartRead(HI_UART_IDX_1, uartReadBuff, UART_BUFF_SIZE);
        if (len > 0)
        {
            // 把接收到的数据打印出来 Print the received data
            printf("Uart Read Data is: [ %d ] %s \r\n", countUart, uartReadBuff);
            countUart++;
            int uartCheck = 0;
            if (uartReadBuff[0] == 'E' && uartReadBuff[1] == 'V' && uartReadBuff[2] == 'E' && uartReadBuff[3] == 'N' && uartReadBuff[4] == 'T' && uartReadBuff[5] == ':')
            {
                // printf("%d,%d\n", uartReadBuff[6] - 48, uartReadBuff[7] - 48);
                uartCheck = 1;
            }
            int eventH = uartReadBuff[6] - 48;
            int eventL = uartReadBuff[7] - 48;
            int input_EVENT = eventH * 10 + eventL;
            if (uartCheck == 1)
            {
                IoTUartWrite(HI_UART_IDX_1, (unsigned char *)ackdata, strlen(ackdata));
                MQTTMessage message;
                char payload[30];

                message.qos = MQTT_QOS;
                message.retained = 0;
                message.payload = payload;
                (void)sprintf_s(payload, sizeof(payload), "UART RECIVED [ %d ] %d%d \r\n", countUart, eventH, eventL);
                message.payloadlen = strlen(payload);

                if ((rc = MQTTPublish(&client, "pubtopic", &message)) != 0)
                {
                    printf("Return code from MQTT publish is %d\n", rc);
                    NetworkDisconnect(&network);
                    MQTTDisconnect(&client);
                }
                OledFillScreen(0);
                switch (input_EVENT)
                {
                case 0:
                    OledShowString(1, 3, "EVENT0", 1);
                    break;
                case 1:
                    OledShowString(1, 3, "EVENT1", 1);
                    break;
                case 2:
                    OledShowString(1, 3, "EVENT2", 1);
                    break;
                case 3:
                    OledShowString(1, 3, "EVENT3", 1);
                    break;
                case 4:
                    OledShowString(1, 3, "EVENT4", 1);
                    break;
                case 5:
                    OledShowString(1, 3, "EVENT5", 1);
                    break;
                case 6:
                    OledShowString(1, 3, "EVENT6", 1);
                    break;
                case 7:
                    OledShowString(1, 3, "EVENT7", 1);
                    break;
                case 8:
                    OledShowString(1, 3, "EVENT8", 1);
                    break;
                case 9:
                    OledShowString(1, 3, "EVENT9", 1);
                    break;
                case 10:
                    OledShowString(1, 3, "EVENT10", 1);
                    break;
                case 11:
                    OledShowString(1, 3, "EVENT11", 1);
                    break;
                case 12:
                    OledShowString(1, 3, "EVENT12", 1);
                    break;
                case 13:
                    OledShowString(1, 3, "EVENT13", 1);
                    break;
                case 14:
                    OledShowString(1, 3, "EVENT14", 1);
                    break;
                case 15:
                    OledShowString(1, 3, "EVENT15", 1);
                    break;
                case 16:
                    OledShowString(1, 3, "EVENT16", 1);
                    break;
                case 17:
                    OledShowString(1, 3, "EVENT17", 1);
                    break;
                case 18:
                    OledShowString(1, 3, "EVENT18", 1);
                    break;
                case 19:
                    OledShowString(1, 3, "EVENT19", 1);
                    break;
                case 20:
                    OledShowString(1, 3, "EVENT20", 1);
                    break;
                case 21:
                    OledShowString(1, 3, "EVENT21", 1);
                    break;
                case 22:
                    OledShowString(1, 3, "EVENT22", 1);
                    break;
                case 23:
                    OledShowString(1, 3, "EVENT23", 1);
                    break;
                case 24:
                    OledShowString(1, 3, "EVENT24", 1);
                    break;
                case 25:
                    OledShowString(1, 3, "EVENT25", 1);
                    break;
                case 26:
                    OledShowString(1, 3, "EVENT26", 1);
                    break;
                case 27:
                    OledShowString(1, 3, "EVENT27", 1);
                    break;
                case 28:
                    OledShowString(1, 3, "EVENT28", 1);
                    break;
                case 29:
                    OledShowString(1, 3, "EVENT29", 1);
                    break;
                case 30:
                    OledShowString(1, 3, "EVENT30", 1);
                    break;
                case 31:
                    OledShowString(1, 3, "EVENT31", 1);
                    break;
                case 32:
                    OledShowString(1, 3, "EVENT32", 1);
                    break;
                case 33:
                    OledShowString(1, 3, "EVENT33", 1);
                    break;
                case 34:
                    OledShowString(1, 3, "EVENT34", 1);
                    break;
                case 35:
                    OledShowString(1, 3, "EVENT35", 1);
                    break;
                case 36:
                    OledShowString(1, 3, "EVENT36", 1);
                    break;
                case 37:
                    OledShowString(1, 3, "EVENT37", 1);
                    break;
                case 38:
                    OledShowString(1, 3, "EVENT38", 1);
                    break;
                case 39:
                    OledShowString(1, 3, "EVENT39", 1);
                    break;
                case 40:
                    OledShowString(1, 3, "EVENT40", 1);
                    break;
                case 41:
                    OledShowString(1, 3, "EVENT41", 1);
                    break;
                case 42:
                    OledShowString(1, 3, "EVENT42", 1);
                    break;
                case 43:
                    OledShowString(1, 3, "EVENT43", 1);
                    break;
                case 44:
                    OledShowString(1, 3, "EVENT44", 1);
                    break;
                case 45:
                    OledShowString(1, 3, "EVENT45", 1);
                    break;
                case 46:
                    OledShowString(1, 3, "EVENT46", 1);
                    break;
                case 47:
                    OledShowString(1, 3, "EVENT47", 1);
                    break;
                case 48:
                    OledShowString(1, 3, "EVENT48", 1);
                    break;
                default:
                    printf("未定义的event\n");
                    break;
                }
            }
        }
        osDelay(MQTT_DELAY_500_MS);
        osDelay(MQTT_DELAY_500_MS);
        osDelay(MQTT_DELAY_500_MS);
        osDelay(MQTT_DELAY_500_MS);
    }
}
static void MQTTDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "MQTTDemoTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MQTT_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)MQTTDemoTask, NULL, &attr) == NULL)
    {
        printf("[MQTT_Demo] Failed to create MQTTDemoTask!\n");
    }
}

APP_FEATURE_INIT(MQTTDemo);