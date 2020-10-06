/*
  main.c: Main program code for the PDK temperature controlled PWM fan project.

  Copyright (C) 2020  serisman  <github@serisman.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  ----------------------------------------------------------------------
*/

#include <stdint.h>
#include <pdk/device.h>
#include <easy-pdk/calibrate.h>
#include "ntc.h"

#define FAN_BIT           3   // Fan is placed on the PA3 pin (Port A, Bit 3), which is also TM2PWM
#define NTC_BIT           4   // 10k NTC is placed on the PA4 pin (Port A, Bit 4), which is also Comp+/Comp-

#define TRIGGER_LOW       NTC_1K_TRIGGER_45C
#define TRIGGER_HIGH      NTC_1K_TRIGGER_50C

void everyTick() {
	static uint8_t pwm = 0;

	// Decrement Fan PWM duty if NTC is < low trigger point
	GPCS = (uint8_t)(GPCS_COMP_RANGE4 | (TRIGGER_LOW & 0x0F));
	if (pwm > 0 && (GPCC & (1 << GPCC_COMP_RESULT_BIT)) == GPCC_COMP_RESULT_NEGATIVE) {
		pwm--;
	}

	// Increment Fan PWM duty, +1 for each step >= high trigger point where NTC is > step trigger point
	for (uint8_t t=TRIGGER_HIGH; t<=0x0F; t++) {
		GPCS = (uint8_t)(GPCS_COMP_RANGE4 | t);
		if (pwm < 255 && (GPCC & (1 << GPCC_COMP_RESULT_BIT)) == GPCC_COMP_RESULT_POSITIVE) {
	    pwm++;
		} else {
			break;
		}
	}

	// Set Fan PWM
	if (pwm) {
		TM2B = pwm;
		TM2C = (uint8_t)(TM2C_MODE_PWM | TM2C_OUT_PA3 | TM2C_CLK_IHRC);  // Ensure PWM is enabled
	} else {
		TM2C = 0x00;          // Disable PWM
		PA &= ~(1<<FAN_BIT);  // Force FAN PWM Output LOW
	}
}

void interrupt(void) __interrupt(0) {
  if (INTRQ & INTRQ_T16) {        // T16 interrupt request?
    everyTick();
    INTRQ &= ~INTRQ_T16;          // Mark T16 interrupt request processed
  }
}

// Main program
void main() {

  // Disable wake-up on un-used pins to save power
  PADIER = 0x00;
#if defined(PBDIER)
  PBDIER = 0x00;
#endif
#if defined(PCDIER)
  PCDIER = 0x00;
#endif

  // Initialize pins
  PAC = (1<<FAN_BIT);         // Set Fan Pin as output

	// Setup PWM for Fan
	//  frequency = F_TIMER / (256 x TM2S[6:5] x (TM2S[4:0] + 1))
	//  duty = ((TM2B[7:0] + 1) / 256) * 100%
	TM2B = 0;
	TM2S = (uint8_t)(TM2S_SCALE_DIV2); // 25 kHz (12.8 mHz / (256 * 1 * 2))
	TM2C = (uint8_t)(TM2C_MODE_PWM | TM2C_OUT_PA3 | TM2C_CLK_IHRC);

  // Setup/enable the Comparator for checking Temperature (i.e. NTC(+) > VINT(-))
  GPCC = (uint8_t)(GPCC_COMP_ENABLE | GPCC_COMP_MINUS_VINT_R | GPCC_COMP_PLUS_PA4);

  // Setup timer16 (T16) interrupt to tick every 327.68 mS (3.05175 Hz)
  T16M = (uint8_t)(T16M_CLK_IHRC | T16M_CLK_DIV64 | T16M_INTSRC_15BIT);
  T16C = 0;

  INTEN = INTEN_T16;
  INTRQ = 0;
  __engint();                     // Enable global interrupts

  // Main processing loop
  while (1) {
    __stopexe();                  // Go to sleep (timer16 will wake us back up)
  }
}

// Startup code - Setup/calibrate system clock
unsigned char _sdcc_external_startup(void) {

  // Set FUSE:
  // - 1.8v LVR (if available)
  // - Normal IO Drive (if available)
  // - Fast Bootup (if available)
  // - Security Off
#if defined(FUSE_LVR_1V8) && defined(FUSE_IO_DRV_NORMAL) && defined(FUSE_BOOTUP_FAST)
  PDK_SET_FUSE(FUSE_BOOTUP_FAST | FUSE_IO_DRV_NORMAL | FUSE_LVR_1V8 | FUSE_SECURITY_OFF);
#elif defined(FUSE_LVR_1V8)
  PDK_SET_FUSE(FUSE_LVR_1V8 | FUSE_SECURITY_OFF);
#elif defined(FUSE_IO_DRV_NORMAL) && defined(FUSE_BOOTUP_FAST)
  PDK_SET_FUSE(FUSE_BOOTUP_FAST | FUSE_IO_DRV_NORMAL | FUSE_SECURITY_OFF);
#elif defined(FUSE_BOOTUP_FAST)
  PDK_SET_FUSE(FUSE_BOOTUP_FAST | FUSE_SECURITY_OFF);
#else
  PDK_SET_FUSE(FUSE_SECURITY_OFF);
#endif

  // Set LVR:
  // - 1.8v (if available)
  // - otherwise 2.0v (if available)
#if defined(MISCLVR_1V8)
  MISCLVR = (uint8_t) MISCLVR_1V8;
#elif defined(MISCLVR_2V)
  MISCLVR = (uint8_t) MISCLVR_2V;
#endif

  // Initialize the system clock (CLKMD register) for IHRC/64
  PDK_SET_SYSCLOCK(CLKMD_IHRC_DIV64);

  // Insert placeholder code to tell EasyPdkProg to calibrate the IHRC internal oscillator for 12.8 MHz operation.
  EASY_PDK_CALIBRATE_IHRC(12800000/64, TARGET_VDD_MV);

  return 0;   // Return 0 to inform SDCC to continue with normal initialization.
}
