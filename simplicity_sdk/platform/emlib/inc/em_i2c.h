/***************************************************************************//**
 * @file
 * @brief Inter-integrated circuit (I2C) peripheral API
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

#ifndef EM_I2C_H
#define EM_I2C_H

#include "em_device.h"
#if defined(I2C_COUNT) && (I2C_COUNT > 0)

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup i2c I2C - Inter-Integrated Circuit
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/**
 * @brief
 *   Standard mode max frequency assuming using 4:4 ratio for Nlow:Nhigh.
 * @details
 *   From I2C specification: Min Tlow = 4.7us, min Thigh = 4.0us,
 *   max Trise=1.0us, max Tfall=0.3us. Since ratio is 4:4, have to use
 *   worst case value of Tlow or Thigh as base.
 *
 *   1/(Tlow + Thigh + 1us + 0.3us) = 1/(4.7 + 4.7 + 1.3)us = 93458Hz
 * @note
 *   Due to chip characteristics, max value is somewhat reduced.
 */
#if defined(_SILICON_LABS_32B_SERIES_0) \
  && (defined(_EFM32_GECKO_FAMILY)      \
  || defined(_EFM32_TINY_FAMILY)        \
  || defined(_EFM32_ZERO_FAMILY)        \
  || defined(_EFM32_HAPPY_FAMILY))
#define I2C_FREQ_STANDARD_MAX    93000
#elif defined(_SILICON_LABS_32B_SERIES_0) \
  && (defined(_EFM32_GIANT_FAMILY)        \
  || defined(_EFM32_WONDER_FAMILY))
#define I2C_FREQ_STANDARD_MAX    92000
#elif defined(_SILICON_LABS_32B_SERIES_1)
// None of the chips on this platform has been characterized on this parameter.
// Use same value as on Wonder until further notice.
#define I2C_FREQ_STANDARD_MAX    92000
#elif defined(_SILICON_LABS_32B_SERIES_2)
#define I2C_FREQ_STANDARD_MAX   100000
#else
#error "Unknown device family."
#endif

/**
 * @brief
 *   Fast mode max frequency assuming using 6:3 ratio for Nlow:Nhigh.
 * @details
 *   From I2C specification: Min Tlow = 1.3us, min Thigh = 0.6us,
 *   max Trise=0.3us, max Tfall=0.3us. Since ratio is 6:3, have to use
 *   worst case value of Tlow or 2xThigh as base.
 *
 *   1/(Tlow + Thigh + 0.3us + 0.3us) = 1/(1.3 + 0.65 + 0.6)us = 392157Hz
 */
#define I2C_FREQ_FAST_MAX        392157

/**
 * @brief
 *   Fast mode+ max frequency assuming using 11:6 ratio for Nlow:Nhigh.
 * @details
 *   From I2C specification: Min Tlow = 0.5us, min Thigh = 0.26us,
 *   max Trise=0.12us, max Tfall=0.12us. Since ratio is 11:6, have to use
 *   worst case value of Tlow or (11/6)xThigh as base.
 *
 *   1/(Tlow + Thigh + 0.12us + 0.12us) = 1/(0.5 + 0.273 + 0.24)us = 987167Hz
 */
#define I2C_FREQ_FASTPLUS_MAX    987167

/**
 * @brief
 *   Indicate plain write sequence: S+ADDR(W)+DATA0+P.
 * @details
 *   @li S - Start
 *   @li ADDR(W) - address with W/R bit cleared
 *   @li DATA0 - Data taken from buffer with index 0
 *   @li P - Stop
 */
#define I2C_FLAG_WRITE          0x0001

/**
 * @brief
 *   Indicate plain read sequence: S+ADDR(R)+DATA0+P.
 * @details
 *   @li S - Start
 *   @li ADDR(R) - Address with W/R bit set
 *   @li DATA0 - Data read into buffer with index 0
 *   @li P - Stop
 */
#define I2C_FLAG_READ           0x0002

/**
 * @brief
 *   Indicate combined write/read sequence: S+ADDR(W)+DATA0+Sr+ADDR(R)+DATA1+P.
 * @details
 *   @li S - Start
 *   @li Sr - Repeated start
 *   @li ADDR(W) - Address with W/R bit cleared
 *   @li ADDR(R) - Address with W/R bit set
 *   @li DATAn - Data written from/read into buffer with index n
 *   @li P - Stop
 */
#define I2C_FLAG_WRITE_READ     0x0004

/**
 * @brief
 *   Indicate write sequence using two buffers: S+ADDR(W)+DATA0+DATA1+P.
 * @details
 *   @li S - Start
 *   @li ADDR(W) - Address with W/R bit cleared
 *   @li DATAn - Data written from buffer with index n
 *   @li P - Stop
 */
#define I2C_FLAG_WRITE_WRITE    0x0008

/** Use 10 bit address. */
#define I2C_FLAG_10BIT_ADDR     0x0010

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/** Clock low to high ratio settings. */
typedef enum {
  i2cClockHLRStandard  = _I2C_CTRL_CLHR_STANDARD,      /**< Ratio is 4:4 */
  i2cClockHLRAsymetric = _I2C_CTRL_CLHR_ASYMMETRIC,    /**< Ratio is 6:3 */
  i2cClockHLRFast      = _I2C_CTRL_CLHR_FAST           /**< Ratio is 11:3 */
} I2C_ClockHLR_TypeDef;

/** Return codes for single Controller mode transfer function. */
typedef enum {
  /* In progress code (>0) */
  i2cTransferInProgress = 1,    /**< Transfer in progress. */

  /* Complete code (=0) */
  i2cTransferDone       = 0,    /**< Transfer completed successfully. */

  /* Transfer error codes (<0). */
  i2cTransferNack       = -1,   /**< NACK received during transfer. */
  i2cTransferBusErr     = -2,   /**< Bus error during transfer (misplaced START/STOP). */
  i2cTransferArbLost    = -3,   /**< Arbitration lost during transfer. */
  i2cTransferUsageFault = -4,   /**< Usage fault. */
  i2cTransferSwFault    = -5    /**< SW fault. */
} I2C_TransferReturn_TypeDef;

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/** I2C initialization structure. */
typedef struct {
  /** Enable I2C peripheral when initialization completed. */
  bool                 enable;

  /** Set to Controller (true) or Target (false) mode */
  bool                 master;

  /**
   * I2C reference clock assumed when configuring bus frequency setup.
   * Set it to 0 if currently configured reference clock will be used
   * This parameter is only applicable if operating in Controller mode.
   */
  uint32_t             refFreq;

  /**
   * (Max) I2C bus frequency to use. This parameter is only applicable
   * if operating in Controller mode.
   */
  uint32_t             freq;

  /** Clock low/high ratio control. */
  I2C_ClockHLR_TypeDef clhr;
} I2C_Init_TypeDef;

/** Suggested default configuration for I2C initialization structure. */
#define I2C_INIT_DEFAULT                                                   \
  {                                                                        \
    true,                  /* Enable when initialization done. */          \
    true,                  /* Set to Controller mode. */                   \
    0,                     /* Use currently configured reference clock. */ \
    I2C_FREQ_STANDARD_MAX, /* Set to standard rate assuring being */       \
    /*                        within I2C specification. */                 \
    i2cClockHLRStandard    /* Set to use 4:4 low/high duty cycle. */       \
  }

/**
 * @brief
 *   Master mode transfer message structure used to define a complete
 *   I2C transfer sequence (from start to stop).
 * @details
 *   The structure allows for defining the following types of sequences
 *   (refer to defines for sequence details):
 *   @li #I2C_FLAG_READ - Data read into buf[0].data
 *   @li #I2C_FLAG_WRITE - Data written from buf[0].data
 *   @li #I2C_FLAG_WRITE_READ - Data written from buf[0].data and read
 *     into buf[1].data
 *   @li #I2C_FLAG_WRITE_WRITE - Data written from buf[0].data and
 *     buf[1].data
 */
typedef struct {
  /**
   * @brief
   *   Address to use after (repeated) start.
   * @details
   *   Layout details, A = Address bit, X = don't care bit (set to 0):
   *   @li 7 bit address - Use format AAAA AAAX
   *   @li 10 bit address - Use format XXXX XAAX AAAA AAAA
   */
  uint16_t addr;

  /** Flags defining sequence type and details, see I2C_FLAG_ defines. */
  uint16_t flags;

  /**
   * Buffers used to hold data to send from or receive into, depending
   * on sequence type.
   */
  struct {
    /** Buffer used for data to transmit/receive, must be @p len long. */
    uint8_t  *data;

    /**
     * Number of bytes in @p data to send or receive. Notice that when
     * receiving data to this buffer, at least 1 byte must be received.
     * Setting @p len to 0 in the receive case is considered a usage fault.
     * Transmitting 0 bytes is legal, in which case only the address
     * is transmitted after the start condition.
     */
    uint16_t len;
  } buf[2];
} I2C_TransferSeq_TypeDef;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

uint32_t I2C_BusFreqGet(I2C_TypeDef *i2c);
void I2C_BusFreqSet(I2C_TypeDef *i2c,
                    uint32_t freqRef,
                    uint32_t freqScl,
                    I2C_ClockHLR_TypeDef i2cMode);
void I2C_Enable(I2C_TypeDef *i2c, bool enable);
void I2C_Init(I2C_TypeDef *i2c, const I2C_Init_TypeDef *init);

/***************************************************************************//**
 * @brief
 *   Clear one or more pending I2C interrupts.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] flags
 *   Pending I2C interrupt source to clear. Use a bitwise logic OR combination of
 *   valid interrupt flags for the I2C module (I2C_IF_nnn).
 ******************************************************************************/
__STATIC_INLINE void I2C_IntClear(I2C_TypeDef *i2c, uint32_t flags)
{
#if defined (I2C_HAS_SET_CLEAR)
  i2c->IF_CLR = flags;
#else
  i2c->IFC = flags;
#endif
}

/***************************************************************************//**
 * @brief
 *   Disable one or more I2C interrupts.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] flags
 *   I2C interrupt sources to disable. Use a bitwise logic OR combination of
 *   valid interrupt flags for the I2C module (I2C_IF_nnn).
 ******************************************************************************/
__STATIC_INLINE void I2C_IntDisable(I2C_TypeDef *i2c, uint32_t flags)
{
#if defined (I2C_HAS_SET_CLEAR)
  i2c->IEN_CLR = flags;
#else
  i2c->IEN &= ~(flags);
#endif
}

/***************************************************************************//**
 * @brief
 *   Enable one or more I2C interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   I2C_IntClear() prior to enabling the interrupt.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] flags
 *   I2C interrupt sources to enable. Use a bitwise logic OR combination of
 *   valid interrupt flags for the I2C module (I2C_IF_nnn).
 ******************************************************************************/
__STATIC_INLINE void I2C_IntEnable(I2C_TypeDef *i2c, uint32_t flags)
{
#if defined (I2C_HAS_SET_CLEAR)
  i2c->IEN_SET = flags;
#else
  i2c->IEN |= flags;
#endif
}

/***************************************************************************//**
 * @brief
 *   Get pending I2C interrupt flags.
 *
 * @note
 *   Event bits are not cleared by the use of this function.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @return
 *   I2C interrupt sources pending. A bitwise logic OR combination of valid
 *   interrupt flags for the I2C module (I2C_IF_nnn).
 ******************************************************************************/
__STATIC_INLINE uint32_t I2C_IntGet(I2C_TypeDef *i2c)
{
  return i2c->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending I2C interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note
 *   Interrupt flags are not cleared by the use of this function.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @return
 *   Pending and enabled I2C interrupt sources
 *   Return value is the bitwise AND of
 *   - the enabled interrupt sources in I2Cn_IEN and
 *   - the pending interrupt flags I2Cn_IF
 ******************************************************************************/
__STATIC_INLINE uint32_t I2C_IntGetEnabled(I2C_TypeDef *i2c)
{
  uint32_t ien;

  ien = i2c->IEN;
  return i2c->IF & ien;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending I2C interrupts from SW.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] flags
 *   I2C interrupt sources to set to pending. Use a bitwise logic OR combination
 *   of valid interrupt flags for the I2C module (I2C_IF_nnn).
 ******************************************************************************/
__STATIC_INLINE void I2C_IntSet(I2C_TypeDef *i2c, uint32_t flags)
{
#if defined (I2C_HAS_SET_CLEAR)
  i2c->IF_SET = flags;
#else
  i2c->IFS = flags;
#endif
}

void I2C_Reset(I2C_TypeDef *i2c);

/***************************************************************************//**
 * @brief
 *   Get Target address used for I2C peripheral (when operating in Target mode).
 *
 * @details
 *   For 10-bit addressing mode, the address is split in two bytes, and only
 *   the first byte setting is fetched, effectively only controlling the 2 most
 *   significant bits of the 10-bit address. Full handling of 10-bit addressing
 *   in Target mode requires additional SW handling.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @return
 *   I2C Target address in use. The 7 most significant bits define the actual
 *   address, the least significant bit is reserved and always returned as 0.
 ******************************************************************************/
__STATIC_INLINE uint8_t I2C_SlaveAddressGet(I2C_TypeDef *i2c)
{
  return ((uint8_t)(i2c->SADDR));
}

/***************************************************************************//**
 * @brief
 *   Set Target address to use for I2C peripheral (when operating in Target mode).
 *
 * @details
 *   For 10- bit addressing mode, the address is split in two bytes, and only
 *   the first byte is set, effectively only controlling the 2 most significant
 *   bits of the 10-bit address. Full handling of 10-bit addressing in Target
 *   mode requires additional SW handling.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C Target address to use. The 7 most significant bits define the actual
 *   address, the least significant bit is reserved and always set to 0.
 ******************************************************************************/
__STATIC_INLINE void I2C_SlaveAddressSet(I2C_TypeDef *i2c, uint8_t addr)
{
  i2c->SADDR = (uint32_t)addr & 0xfe;
}

/***************************************************************************//**
 * @brief
 *   Get Target address mask used for I2C peripheral (when operating in Target
 *   mode).
 *
 * @details
 *   The address mask defines how the comparator works. A bit position with
 *   value 0 means that the corresponding Target address bit is ignored during
 *   comparison (don't care). A bit position with value 1 means that the
 *   corresponding Target address bit must match.
 *
 *   For 10-bit addressing mode, the address is split in two bytes, and only
 *   the mask for the first address byte is fetched, effectively only
 *   controlling the 2 most significant bits of the 10-bit address.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @return
 *   I2C Target address mask in use. The 7 most significant bits define the
 *   actual address mask, the least significant bit is reserved and always
 *   returned as 0.
 ******************************************************************************/
__STATIC_INLINE uint8_t I2C_SlaveAddressMaskGet(I2C_TypeDef *i2c)
{
  return ((uint8_t)(i2c->SADDRMASK));
}

/***************************************************************************//**
 * @brief
 *   Set Target address mask used for I2C peripheral (when operating in Target
 *   mode).
 *
 * @details
 *   The address mask defines how the comparator works. A bit position with
 *   value 0 means that the corresponding Target address bit is ignored during
 *   comparison (don't care). A bit position with value 1 means that the
 *   corresponding Target address bit must match.
 *
 *   For 10-bit addressing mode, the address is split in two bytes, and only
 *   the mask for the first address byte is set, effectively only controlling
 *   the 2 most significant bits of the 10-bit address.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] mask
 *   I2C Target address mask to use. The 7 most significant bits define the
 *   actual address mask, the least significant bit is reserved and should
 *   be 0.
 ******************************************************************************/
__STATIC_INLINE void I2C_SlaveAddressMaskSet(I2C_TypeDef *i2c, uint8_t mask)
{
  i2c->SADDRMASK = (uint32_t)mask & 0xfe;
}

I2C_TransferReturn_TypeDef I2C_Transfer(I2C_TypeDef *i2c);
I2C_TransferReturn_TypeDef I2C_TransferInit(I2C_TypeDef *i2c,
                                            I2C_TransferSeq_TypeDef *seq);

/** @} (end addtogroup i2c) */

#ifdef __cplusplus
}
#endif

#endif /* defined(I2C_COUNT) && (I2C_COUNT > 0) */
#endif /* EM_I2C_H */
