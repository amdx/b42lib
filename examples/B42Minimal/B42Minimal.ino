/*
 * B42 protocol Arduino library
 *
 * Copyright (C) 2020 Archimedes Exhibitions GmbH
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * A minimal working example on how to use the B42 protocol to communicate with a host.
 *
 * HOST (computer) <--- B42 ---> BOARD (Arduino)
 *
 * This example uses 4 commands for the application protocol.
 * It can be tested with the B42 minimal examples from
 * https://github.com/amdx/pyb42
 */

#include <B42.h>

// application defined command codes (0x00 is reserved and should not be used)
const uint8_t MYCOMMAND_HELLO  = 0x01; // this is a board-to-host only command
const uint8_t MYCOMMAND_STATUS = 0x02;
const uint8_t MYCOMMAND_FOO    = 0x03;
const uint8_t MYCOMMAND_BAR    = 0x04;
// application defined status/error codes ([0x00..0x0F] used by B42 itself)
const uint8_t MYERROR_NOERROR        = B42_ERROR_NONE; // 0x00
const uint8_t MYERROR_INVALID_LENGTH = 0x10;

uint8_t sendData[B42_MAX_DATA_LENGTH]; // board-to-host data buffer
uint8_t fooCounter = 0;


/* application defined error handler */

void errorHandler(B42ReturnCode rc)
{
    // just relay the error code to the host via the STATUS command
    b42.sendFrame(MYCOMMAND_STATUS, 1, &rc);
}


/* application defined command handlers */

// this command returns the current status
B42ReturnCode cmdStatus(uint8_t dataLen, uint8_t* data)
{
    // we expect no data
    if (0 < dataLen) {
        // returning anything different from 0 triggers the error handler
        return MYERROR_INVALID_LENGTH;
    }

    sendData[0] = MYERROR_NOERROR; // status returned to host
    b42.sendFrame(MYCOMMAND_STATUS, 1, sendData);

    return MYERROR_NOERROR;
}

// this command returns an increasing number
B42ReturnCode cmdFoo(uint8_t dataLen, uint8_t* data)
{
    // we expect no data
    if (0 < dataLen) {
        // returning anything different from 0 triggers the error handler
        return MYERROR_INVALID_LENGTH;
    }

    // we fill/send only one data byte
    // watch out! all the data bytes will be truncated to 6 bits
    sendData[0] = fooCounter++;
    b42.sendFrame(MYCOMMAND_FOO, 1, sendData);

    return MYERROR_NOERROR;
}

// this command controls the onboard LED
B42ReturnCode cmdBar(uint8_t dataLen, uint8_t* data)
{
    // check if the payload length matches our expectations
    if (dataLen != 1) {
        return MYERROR_INVALID_LENGTH;
    }

    // do something on the hardware according to the data bytes
    digitalWrite(LED_BUILTIN, data[0] == 0 ? LOW : HIGH);

    // reply with the same payload
    b42.sendFrame(MYCOMMAND_BAR, 1, data);

    return MYERROR_NOERROR;
}


/* setup and main loop */

void setup()
{
    // initialize B42
    b42.begin();

    // instruct B42 to route errors to our error handler
    b42.setErrorHandler(errorHandler);

    // add a couple of command handlers
    b42.registerCommandHandler(MYCOMMAND_STATUS, cmdStatus);
    b42.registerCommandHandler(MYCOMMAND_FOO, cmdFoo);
    b42.registerCommandHandler(MYCOMMAND_BAR, cmdBar);

    // initialize the hardware we're going to use
    pinMode(LED_BUILTIN, OUTPUT);

    // send a message that tells the host we're ready to go
    b42.sendFrame(MYCOMMAND_HELLO, 0, NULL);
}

void loop()
{
    // this method checks for any message waiting in the serial buffer
    // call it often enough to ensure to pick up any request from the host
    // registered command/error handlers are called by this method
    b42.update();

    // do other tasks here ...
}
