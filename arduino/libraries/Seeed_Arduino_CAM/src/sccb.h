/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 * Modify By Seeed 7.29/2021
 * Content: Adapted to Arduino framwork
 */
#ifndef __SCCB_H__
#define __SCCB_H__
#include <stdint.h>
#include "Arduino.h"
#include "Wire.h"

int SCCB_Init(TwoWire *sccb);
int SCCB_Deinit(TwoWire *sccb);
uint8_t SCCB_Probe(TwoWire *sccb);
uint8_t SCCB_Read(TwoWire *sccb, uint8_t slv_addr, uint8_t reg);
uint8_t SCCB_Write(TwoWire *sccb, uint8_t slv_addr, uint8_t reg, uint8_t data);
uint8_t SCCB_Read16(TwoWire *sccb, uint8_t slv_addr, uint16_t reg);
uint8_t SCCB_Write16(TwoWire *sccb, uint8_t slv_addr, uint16_t reg, uint8_t data);
#endif // __SCCB_H__
