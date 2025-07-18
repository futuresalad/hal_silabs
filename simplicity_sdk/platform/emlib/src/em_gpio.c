/***************************************************************************//**
 * @file
 * @brief General Purpose IO (GPIO) peripheral API
 *   devices.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "em_gpio.h"

#if defined(GPIO_COUNT) && (GPIO_COUNT > 0)

/***************************************************************************//**
 * @addtogroup gpio GPIO - General Purpose Input/Output
 * @brief General Purpose Input/Output (GPIO) API
 * @details
 *  This module contains functions to control the GPIO peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The GPIO peripheral is used for pin configuration
 *  and direct pin manipulation and sensing as well as routing for peripheral
 *  pin connections.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** Validation of the pin typically usable in assert statements. */
#define GPIO_DRIVEMODE_VALID(mode)    ((mode) <= 3)
#define GPIO_STRENGTH_VALID(strength) (!((strength)                          \
                                         & ~(_GPIO_P_CTRL_DRIVESTRENGTH_MASK \
                                             | _GPIO_P_CTRL_DRIVESTRENGTHALT_MASK)))
/** @endcond */

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Sets the pin location of the debug pins (Serial Wire interface).
 *
 * @note
 *   Changing the pins used for debugging uncontrolled, may result in a lockout.
 *
 * @param[in] location
 *   The debug pin location to use (0-3).
 ******************************************************************************/
void GPIO_DbgLocationSet(unsigned int location)
{
#if defined (_GPIO_ROUTE_SWLOCATION_MASK)
  EFM_ASSERT(location < AFCHANLOC_MAX);

  GPIO->ROUTE = (GPIO->ROUTE & ~_GPIO_ROUTE_SWLOCATION_MASK)
                | (location << _GPIO_ROUTE_SWLOCATION_SHIFT);
#elif defined (_GPIO_ROUTELOC0_SWVLOC_MASK)
  EFM_ASSERT(location < AFCHANLOC_MAX);

  GPIO->ROUTELOC0 = (GPIO->ROUTELOC0 & ~_GPIO_ROUTELOC0_SWVLOC_MASK)
                    | (location << _GPIO_ROUTELOC0_SWVLOC_SHIFT);
#else
  (void)location;
#endif
}

#if defined (_GPIO_P_CTRL_DRIVEMODE_MASK)
/***************************************************************************//**
 * @brief
 *   Sets drive mode for a GPIO port.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] mode
 *   Drive mode to use for the port.
 ******************************************************************************/
void GPIO_DriveModeSet(GPIO_Port_TypeDef port, GPIO_DriveMode_TypeDef mode)
{
  EFM_ASSERT(GPIO_PORT_VALID(port) && GPIO_DRIVEMODE_VALID(mode));

  GPIO->P[port].CTRL = (GPIO->P[port].CTRL & ~(_GPIO_P_CTRL_DRIVEMODE_MASK))
                       | (mode << _GPIO_P_CTRL_DRIVEMODE_SHIFT);
}
#endif

#if defined (_GPIO_P_CTRL_DRIVESTRENGTH_MASK)
/***************************************************************************//**
 * @brief
 *   Sets the drive strength for a GPIO port.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] strength
 *   The drive strength to use for the port.
 ******************************************************************************/
void GPIO_DriveStrengthSet(GPIO_Port_TypeDef port,
                           GPIO_DriveStrength_TypeDef strength)
{
  EFM_ASSERT(GPIO_PORT_VALID(port) && GPIO_STRENGTH_VALID(strength));
  BUS_RegMaskedWrite(&GPIO->P[port].CTRL,
                     _GPIO_P_CTRL_DRIVESTRENGTH_MASK | _GPIO_P_CTRL_DRIVESTRENGTHALT_MASK,
                     strength);
}
#endif

/***************************************************************************//**
 * @brief
 *   Configure the GPIO external pin interrupt.
 *
 * @details
 *   It is recommended to disable interrupts before configuring the GPIO pin interrupt.
 *   See @ref GPIO_IntDisable() for more information.
 *
 *   The GPIO interrupt handler must be in place before enabling the
 *   interrupt.
 *
 *   Notice that any pending interrupt for the selected interrupt is cleared
 *   by this function.
 *
 * @note
 *   On series 0 devices, the pin number parameter is not used. The
 *   pin number used on these devices is hardwired to the interrupt with the
 *   same number. @n
 *   On series 1 devices, the pin number can be selected freely within a group.
 *   Interrupt numbers are divided into 4 groups (intNo / 4) and valid pin
 *   number within the interrupt groups are:
 *       0: pins 0-3   (interrupt number 0-3)
 *       1: pins 4-7   (interrupt number 4-7)
 *       2: pins 8-11  (interrupt number 8-11)
 *       3: pins 12-15 (interrupt number 12-15)
 *
 * @param[in] port
 *   The port to associate with the @p pin.
 *
 * @param[in] pin
 *   The pin number on the port.
 *
 * @param[in] intNo
 *   The interrupt number to trigger.
 *
 * @param[in] risingEdge
 *   Set to true if the interrupt will be enabled on the rising edge. Otherwise, false.
 *
 * @param[in] fallingEdge
 *   Set to true if the interrupt will be enabled on the falling edge. Otherwise, false.
 *
 * @param[in] enable
 *   Set to true if the interrupt will be enabled after the configuration is complete.
 *   False to leave disabled. See @ref GPIO_IntDisable() and @ref GPIO_IntEnable().
 ******************************************************************************/
void GPIO_ExtIntConfig(GPIO_Port_TypeDef port,
                       unsigned int pin,
                       unsigned int intNo,
                       bool risingEdge,
                       bool fallingEdge,
                       bool enable)
{
#if defined (_GPIO_EXTIPSELH_MASK)
  uint32_t tmp = 0;
#endif
#if !defined(_GPIO_EXTIPINSELL_MASK)
  (void)pin;
#endif

  EFM_ASSERT(GPIO_PORT_PIN_VALID(port, pin));
#if defined(_GPIO_EXTIPINSELL_MASK)
  EFM_ASSERT(GPIO_INTNO_PIN_VALID(intNo, pin));
#endif

  /* The EXTIPSELL register controls pins 0-7 and EXTIPSELH controls
   * pins 8-15 of the interrupt configuration. */
  if (intNo < 8) {
    BUS_RegMaskedWrite(&GPIO->EXTIPSELL,
                       _GPIO_EXTIPSELL_EXTIPSEL0_MASK
                       << (_GPIO_EXTIPSELL_EXTIPSEL1_SHIFT * intNo),
                       (uint32_t)port << (_GPIO_EXTIPSELL_EXTIPSEL1_SHIFT * intNo));
  } else {
#if defined(_GPIO_EXTIPSELH_MASK)
    tmp = intNo - 8;
#if defined(_GPIO_EXTIPSELH_EXTIPSEL0_MASK)
    BUS_RegMaskedWrite(&GPIO->EXTIPSELH,
                       _GPIO_EXTIPSELH_EXTIPSEL0_MASK
                       << (_GPIO_EXTIPSELH_EXTIPSEL1_SHIFT * tmp),
                       (uint32_t)port << (_GPIO_EXTIPSELH_EXTIPSEL1_SHIFT * tmp));
#elif defined(_GPIO_EXTIPSELH_EXTIPSEL8_MASK)
    BUS_RegMaskedWrite(&GPIO->EXTIPSELH,
                       _GPIO_EXTIPSELH_EXTIPSEL8_MASK
                       << (_GPIO_EXTIPSELH_EXTIPSEL9_SHIFT * tmp),
                       (uint32_t)port << (_GPIO_EXTIPSELH_EXTIPSEL9_SHIFT * tmp));
#else
#error Invalid GPIO_EXTIPINSELH bit fields
#endif
#endif /* #if defined(_GPIO_EXTIPSELH_MASK) */
  }

#if defined(_GPIO_EXTIPINSELL_MASK)

  /* The EXTIPINSELL register controls interrupt 0-7 and EXTIPINSELH controls
   * interrupt 8-15 of the interrupt/pin number mapping. */
  if (intNo < 8) {
    BUS_RegMaskedWrite(&GPIO->EXTIPINSELL,
                       _GPIO_EXTIPINSELL_EXTIPINSEL0_MASK
                       << (_GPIO_EXTIPINSELL_EXTIPINSEL1_SHIFT * intNo),
                       (uint32_t)((pin % 4) & _GPIO_EXTIPINSELL_EXTIPINSEL0_MASK)
                       << (_GPIO_EXTIPINSELL_EXTIPINSEL1_SHIFT * intNo));
  } else {
#if defined (_GPIO_EXTIPINSELH_EXTIPINSEL8_MASK)
    BUS_RegMaskedWrite(&GPIO->EXTIPINSELH,
                       _GPIO_EXTIPINSELH_EXTIPINSEL8_MASK
                       << (_GPIO_EXTIPINSELH_EXTIPINSEL9_SHIFT * tmp),
                       (uint32_t)((pin % 4) & _GPIO_EXTIPINSELH_EXTIPINSEL8_MASK)
                       << (_GPIO_EXTIPSELH_EXTIPSEL9_SHIFT * tmp));
#endif
#if defined (_GPIO_EXTIPINSELH_EXTIPINSEL0_MASK)
    BUS_RegMaskedWrite(&GPIO->EXTIPINSELH,
                       _GPIO_EXTIPINSELH_EXTIPINSEL0_MASK
                       << (_GPIO_EXTIPINSELH_EXTIPINSEL1_SHIFT * tmp),
                       (uint32_t)((pin % 4) & _GPIO_EXTIPINSELH_EXTIPINSEL0_MASK)
                       << (_GPIO_EXTIPSELH_EXTIPSEL1_SHIFT * tmp));
#endif
  }
#endif

  /* Enable/disable the rising edge interrupt. */
  BUS_RegBitWrite(&(GPIO->EXTIRISE), intNo, risingEdge);

  /* Enable/disable the falling edge interrupt. */
  BUS_RegBitWrite(&(GPIO->EXTIFALL), intNo, fallingEdge);

  /* Clear any pending interrupt. */
  GPIO_IntClear(1 << intNo);

  /* Finally enable/disable interrupt. */
  BUS_RegBitWrite(&(GPIO->IEN), intNo, enable);
}

#if _SILICON_LABS_32B_SERIES > 0
/***************************************************************************//**
 * @brief
 *   Configure EM4WU pins as external level-sensitive interrupts.
 *
 * @details
 *   It is recommended to disable interrupts before configuring the GPIO pin interrupt.
 *   See @ref GPIO_IntDisable() for more information.
 *
 *   The GPIO interrupt handler must be in place before enabling the
 *   interrupt.
 *
 *   Notice that any pending interrupt for the selected interrupt is cleared
 *   by this function.
 *
 * @note
 *   The selected port/pin must be mapped to an existant EM4WU interrupt.
 *   Each EM4WU signal is connected to a fixed pin.
 *   Refer to the Alternate Function Table in the device Datasheet for the
 *   location of each EM4WU signal. For example, on xG22 device, the interrupt
 *   of EM4WU6 is fixed to pin PC00.
 *
 * @param[in] port
 *   The port to associate with the @p pin.
 *
 * @param[in] pin
 *   The pin number on the port.
 *
 * @param[in] intNo
 *   The EM4WU interrupt number to trigger.
 *
 * @param[in] polarity
 *   true = Active high level-sensitive interrupt.
 *   false = Active low level-sensitive interrupt.
 *
 * @param[in] enable
 *   Set to true if the interrupt will be enabled after the configuration is complete.
 *   False to leave disabled. See @ref GPIO_IntDisable() and @ref GPIO_IntEnable().
 ******************************************************************************/
void GPIO_EM4WUExtIntConfig(GPIO_Port_TypeDef port,
                            unsigned int pin,
                            uint32_t intNo,
                            bool polarity,
                            bool enable)
{
  EFM_ASSERT(GPIO_PORT_PIN_VALID(port, pin));

  // GPIO pin mode set.
  GPIO_PinModeSet(port, pin, gpioModeInputPullFilter, (unsigned int)!polarity);

  // Enable EM4WU function and set polarity
  uint32_t polarityMask = (uint32_t)polarity << (intNo + _GPIO_EM4WUEN_EM4WUEN_SHIFT);
  uint32_t pinmask =  1UL << (intNo + _GPIO_EM4WUEN_EM4WUEN_SHIFT);

  GPIO_EM4EnablePinWakeup(pinmask, polarityMask);

  // Enable EM4WU interrupt
#if defined(_SILICON_LABS_32B_SERIES_1)
  BUS_RegBitWrite(&(GPIO->IEN), intNo + _GPIO_IEN_EM4WU_SHIFT, enable);
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  BUS_RegBitWrite(&(GPIO->IEN), intNo + _GPIO_IEN_EM4WUIEN_SHIFT, enable);
#else
  BUS_RegBitWrite(&(GPIO->IEN), intNo + _GPIO_IEN_EM4WUIEN0_SHIFT, enable);
#endif
}
#endif

/***************************************************************************//**
 * @brief
 *   Set the mode for a GPIO pin.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] pin
 *   The pin number in the port.
 *
 * @param[in] mode
 *   The desired pin mode.
 *
 * @param[in] out
 *   A value to set for the pin in the DOUT register. The DOUT setting is important for
 *   some input mode configurations to determine the pull-up/down direction.
 ******************************************************************************/
void GPIO_PinModeSet(GPIO_Port_TypeDef port,
                     unsigned int pin,
                     GPIO_Mode_TypeDef mode,
                     unsigned int out)
{
  EFM_ASSERT(GPIO_PORT_PIN_VALID(port, pin));

  /* If disabling a pin, do not modify DOUT to reduce the chance of */
  /* a glitch/spike (may not be sufficient precaution in all use cases). */
  if (mode != gpioModeDisabled) {
    if (out) {
      GPIO_PinOutSet(port, pin);
    } else {
      GPIO_PinOutClear(port, pin);
    }
  }

  /* There are two registers controlling the pins for each port. The MODEL
   * register controls pins 0-7 and MODEH controls pins 8-15. */
  if (pin < 8) {
    // Cast parameter [mode] to 32 bits to fix C99 Undefined Behavior (see SEI CERT C INT34-C)
    // Compiler assigned 8 bits for enum. Same thing for other branch.
    BUS_RegMaskedWrite(&(GPIO->P[port].MODEL), 0xFu << (pin * 4), (uint32_t)mode << (pin * 4));
  } else {
    BUS_RegMaskedWrite(&(GPIO->P[port].MODEH), 0xFu << ((pin - 8) * 4), (uint32_t)mode << ((pin - 8) * 4));
  }

  if (mode == gpioModeDisabled) {
    if (out) {
      GPIO_PinOutSet(port, pin);
    } else {
      GPIO_PinOutClear(port, pin);
    }
  }
}

/***************************************************************************//**
 * @brief
 *   Get the mode for a GPIO pin.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] pin
 *   The pin number in the port.
 *
 * @return
 *   The pin mode.
 ******************************************************************************/
GPIO_Mode_TypeDef GPIO_PinModeGet(GPIO_Port_TypeDef port,
                                  unsigned int pin)
{
  EFM_ASSERT(GPIO_PORT_PIN_VALID(port, pin));

  if (pin < 8) {
    return (GPIO_Mode_TypeDef) ((GPIO->P[port].MODEL >> (pin * 4)) & 0xF);
  } else {
    return (GPIO_Mode_TypeDef) ((GPIO->P[port].MODEH >> ((pin - 8) * 4)) & 0xF);
  }
}

#if defined(_GPIO_EM4WUEN_MASK)
/**************************************************************************//**
 * @brief
 *   Enable GPIO pin wake-up from EM4. When the function exits,
 *   EM4 mode can be safely entered.
 *
 * @note
 *   It is assumed that the GPIO pin modes are set correctly.
 *   Valid modes are @ref gpioModeInput and @ref gpioModeInputPull.
 *
 * @param[in] pinmask
 *   A bitmask containing the bitwise logic OR of which GPIO pin(s) to enable.
 *   See Reference Manuals for a pinmask to the GPIO port/pin mapping.
 * @param[in] polaritymask
 *   A bitmask containing the bitwise logic OR of GPIO pin(s) wake-up polarity.
 *   See Reference Manuals for pinmask-to-GPIO port/pin mapping.
 *****************************************************************************/
void GPIO_EM4EnablePinWakeup(uint32_t pinmask, uint32_t polaritymask)
{
  EFM_ASSERT((pinmask & ~_GPIO_EM4WUEN_MASK) == 0);

#if defined(_GPIO_EM4WUPOL_MASK)
  EFM_ASSERT((polaritymask & ~_GPIO_EM4WUPOL_MASK) == 0);
  #if defined(GPIO_HAS_SET_CLEAR)
  GPIO->EM4WUPOL_CLR = pinmask;                   /* Set the wakeup polarity. */
  GPIO->EM4WUPOL_SET = pinmask & polaritymask;
  #else
  GPIO->EM4WUPOL &= ~pinmask;
  GPIO->EM4WUPOL |= pinmask & polaritymask;
  #endif
#elif defined(_GPIO_EXTILEVEL_MASK)
  EFM_ASSERT((polaritymask & ~_GPIO_EXTILEVEL_MASK) == 0);
  #if defined(GPIO_HAS_SET_CLEAR)
  GPIO->EXTILEVEL_CLR = pinmask;
  GPIO->EXTILEVEL_SET = pinmask & polaritymask;
  #else
  GPIO->EXTILEVEL &= ~pinmask;
  GPIO->EXTILEVEL |= pinmask & polaritymask;
  #endif
#endif
  #if defined(GPIO_HAS_SET_CLEAR)
  GPIO->EM4WUEN_SET = pinmask;                    /* Enable wakeup. */
  #else
  GPIO->EM4WUEN |= pinmask;
  #endif

  GPIO_EM4SetPinRetention(true);            /* Enable the pin retention. */

#if defined(_GPIO_CMD_EM4WUCLR_MASK)
  GPIO->CMD = GPIO_CMD_EM4WUCLR;            /* Clear the wake-up logic. */
#else
  GPIO_IntClear(pinmask);
#endif
}
#endif

/** @} (end addtogroup gpio) */

#endif /* defined(GPIO_COUNT) && (GPIO_COUNT > 0) */
