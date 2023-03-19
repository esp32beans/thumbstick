/*
 * MIT License
 *
 * Copyright (c) 2023 esp32beans@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Changing the 0 to 1 produces lots of useful debug but it greatly
// slows the program. Leave this off for production mode.
#define DBG_Serial if (0) Serial

// Instantiate SparkFun Qwiic Joystick
#include <Wire.h>
#include "SparkFun_Qwiic_Joystick_Arduino_Library.h"
JOYSTICK Joystick;

// Instantiate Adafruit TinyUSB HID Joystick
#include "Adafruit_TinyUSB.h"

// USB HID joystck with 16 buttons and 2 16-bit axes
// Works with Xbox Adaptive Controller
uint8_t const desc_hid_report[] = {
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x04,        // Usage (Joystick)
  0xA1, 0x01,        // Collection (Application)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x35, 0x00,        //   Physical Minimum (0)
  0x45, 0x01,        //   Physical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x10,        //   Report Count (16)
  0x05, 0x09,        //   Usage Page (Button)
  0x19, 0x01,        //   Usage Minimum (0x01)
  0x29, 0x10,        //   Usage Maximum (0x10)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
  0x16, 0x01, 0x80,  //   Logical Minimum (-32767)
  0x36, 0x01, 0x80,  //   Physical Minimum (-32767)
  0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
  0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
  0x09, 0x30,        //   Usage (X)
  0x09, 0x31,        //   Usage (Y)
  0x75, 0x10,        //   Report Size (16)
  0x95, 0x02,        //   Report Count (2)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              // End Collection
};

// USB HID object. For ESP32 these values cannot be changed after this
// declaration desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report),
    HID_ITF_PROTOCOL_NONE, 2, false);

typedef struct __attribute__((__packed__)) XAC_joystick {
  uint16_t buttons;
  int16_t x;
  int16_t y;
} XAC_joystick_t;

XAC_joystick_t USB_Joystick;

void setup() {
  Serial.begin(115200);

  // Setup the SparkFun Qwiic joystick
  if (Joystick.begin() == false)
  {
    while (!Serial) delay(1);
    Serial.println("Joystick does not appear to be connected. Please check wiring.");
    while(1) delay(1);
  }

  // Setup the USB joystick
  memset(&USB_Joystick, 0, sizeof(USB_Joystick));

  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  usb_hid.begin();

  // wait until device mounted
  while( !TinyUSBDevice.mounted() ) delay(1);

  while (!Serial && millis() < 3000) delay(1);
  Serial.println();
  Serial.println("USB analog small joytick");
}

void loop() {
  if ( !usb_hid.ready() ) return;

  DBG_Serial.print("X: ");
  DBG_Serial.print(Joystick.getHorizontal());
  USB_Joystick.x = map(Joystick.getHorizontal(), 0, 1023, -32767, 32767);
  DBG_Serial.print(" USB_Joystick.x: ");
  DBG_Serial.print(USB_Joystick.x);

  DBG_Serial.print(" Y: ");
  DBG_Serial.print(Joystick.getVertical());
  USB_Joystick.y = map(Joystick.getVertical(), 0, 1023, -32767, 32767);
  DBG_Serial.print(" USB_Joystick.y: ");
  DBG_Serial.print(USB_Joystick.y);
  
  DBG_Serial.print(" Button: ");
  int btn = !Joystick.getButton();
  DBG_Serial.println(btn);
  USB_Joystick.buttons &= ~1;
  USB_Joystick.buttons |= btn;
  usb_hid.sendReport(0, &USB_Joystick, sizeof(USB_Joystick));

  delay(1);
}
