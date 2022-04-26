/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 * 
 * Modify By Seeed 7.29/2021
 * Content: Adapted to Arduino framwor;
 *
 */

#include <stdbool.h>
#include <string.h>
#include "sccb.h"
#include "sensor.h"
#include <stdio.h>
#define TAG "SCCB"
#include "cam_log.h"


int SCCB_Init(TwoWire *sccb)
{
    sccb->begin();
    return 0;
}

int SCCB_Deinit(TwoWire *sccb)
{
    sccb->end();
    return 0;
}

uint8_t SCCB_Probe(TwoWire *sccb)
{
    uint8_t error = 0;
    uint8_t slave_addr = 0x0;
    for (size_t i = 0; i < CAMERA_MODEL_MAX; i++)
    {
        if (slave_addr == camera_sensor[i].sccb_addr)
        {
            continue;
        }
        slave_addr = camera_sensor[i].sccb_addr;
        sccb->beginTransmission(slave_addr);
        error = sccb->endTransmission();
        if (error == 0)
        {
            return slave_addr;
        }
    }
    return 0;
}

uint8_t SCCB_Read(TwoWire *sccb, uint8_t slv_addr, uint8_t reg)
{
    uint8_t data = 0;

    sccb->beginTransmission(slv_addr);
    sccb->write(reg);
    sccb->endTransmission();
    sccb->requestFrom(slv_addr, 1);
    if (sccb->available())
    {
        data = sccb->read();
    }

    return data;
}

uint8_t SCCB_Write(TwoWire *sccb, uint8_t slv_addr, uint8_t reg, uint8_t data)
{
    uint8_t error = 0;

    sccb->beginTransmission(slv_addr);
    sccb->write(reg);
    sccb->write(data);
    error = sccb->endTransmission();

    return error;
}

uint8_t SCCB_Read16(TwoWire *sccb, uint8_t slv_addr, uint16_t reg)
{
    uint8_t data = 0;

    sccb->beginTransmission(slv_addr);
    sccb->write(reg & 0xFF);
    sccb->write(reg >> 8);
    sccb->endTransmission();
    sccb->requestFrom(slv_addr, 1);
    if (sccb->available())
    {
        data = sccb->read();
    }

    return data;
}

uint8_t SCCB_Write16(TwoWire *sccb, uint8_t slv_addr, uint16_t reg, uint8_t data)
{
    uint8_t error = 0;

    sccb->beginTransmission(slv_addr);
    sccb->write(reg & 0xFF);
    sccb->write(reg >> 8);
    sccb->write(data);
    error = sccb->endTransmission();

    return error;
}
