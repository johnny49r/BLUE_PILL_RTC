// STM32LIBS_REGS.h
// memory mapped register definitions for the STM32F10x MPU

#ifndef _STM32LIBS_REGS_H
#define _STM32LIBS_REGS_H

// STM32 Power control registers
#define PWR_REG_BASE    0x40007000UL
#define PWR_CR          (*(volatile uint32_t *)(PWR_REG_BASE))  // power control reg 
#define PWR_CSR         (*(volatile uint32_t *)(PWR_REG_BASE + 0x00000004UL))  // power control/status reg 

// power control defines
#define DBP             0x0100         // disable backup domain write protection

//#define PWR_INIT_ALL    0x0000017CUL   // allow backup domain access, 2.5V brownout, clr wkup & stdby
#define PWR_INIT_ALL    0x00000100UL   // allow backup domain access, 2.5V brownout, clr wkup & stdby

// Reset & clock control registers (32 bit regs)
#define RCC_REG_BASE    0x40021000UL
#define RCC_BDCR        (*(volatile uint32_t *)(RCC_REG_BASE + 0x00000020UL))  // RTC ctl reg low
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_REG_BASE + 0x0000001CUL)) 
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_REG_BASE + 0x00000018UL)) 
#define RCC_CIR         (*(volatile uint32_t *)(RCC_REG_BASE + 0x00000008UL)

// 
#define LSEON           0x00000001UL   // external 32.768 KHz clk enable (LSE)
#define LSERDY          0x00000002UL   // status - external clk is stable
#define LSEBYP          0x00000004UL   // external 32KHz osc bypass bit
#define LSE_CLK_SEL     0x00000100UL
#define BKP_RESET       0x00010000UL   // reset backup domain
#define RTC_ENAB        0x00008000UL   // sends clk to RTC
#define BDCR_INIT       0x00008101UL   // RTCEN, LSE clk, LSEON
#define PWREN           0x18000000UL   // power enab + backup enab

// RTC register memory mapped addresses
#define RTC_REG_BASE    0x40002800UL
#define RTC_CRH         (*(volatile uint32_t *)(RTC_REG_BASE))  // RTC ctl reg high
#define RTC_CRL         (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000004UL))  // RTC ctl reg low
#define RTC_PRLH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000008UL))  // RTC prescaler reg high
#define RTC_PRLL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x0000000CUL))  // RTC prescaler reg low
#define RTC_DIVH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000010UL))  // RTC prescaler divide reg high
#define RTC_DIVL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000014UL))  // RTC prescaler divide reg low
#define RTC_CNTH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000018UL))  // RTC count reg high
#define RTC_CNTL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x0000001CUL))  // RTC count reg low
#define RTC_ALRH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000020UL))  // RTC alarm reg high
#define RTC_ALRL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000024UL))  // RTC alarm reg low

// RTC CRL control reg bits
#define RTOFF           0x0020UL
#define CNF             0x0010UL
#define RSF             0x0008UL
#define RTC_CRL_ALARMF  0x0002UL
#define ALARMF_MASK     0x0010UL
#define RTOFF_RSF       0x0028UL

// RTC CRH control reg bits
#define RTC_ALRIE_MASK  0x0005UL
#define RTC_ALRIE       0x0002UL
#define RTC_SECIE       0x0001UL

// NVIC registers
#define NVIC_REG_BASE   0xE000E100UL
#define NVIC_ISER_BASE  NVIC_REG_BASE + 0x00000000UL
#define NVIC_ISER0      (*(volatile uint32_t *)(NVIC_ISER_BASE + 0x00000000UL)) 
#define NVIC_ISER1      (*(volatile uint32_t *)(NVIC_ISER_BASE + 0x00000004UL)) 
#define NVIC_ISER2      (*(volatile uint32_t *)(NVIC_ISER_BASE + 0x00000008UL)) 

#define NVIC_ICER_BASE  NVIC_REG_BASE + 0x00000080UL
#define NVIC_ICER0      (*(volatile uint32_t *)(NVIC_ICER_BASE + 0x00000000UL)) 
#define NVIC_ICER1      (*(volatile uint32_t *)(NVIC_ICER_BASE + 0x00000004UL)) 
#define NVIC_ICER2      (*(volatile uint32_t *)(NVIC_ICER_BASE + 0x00000008UL)) 

#define NVIC_ISPR_BASE  NVIC_REG_BASE + 0x00000100UL
#define NVIC_ISPR0      (*(volatile uint32_t *)(NVIC_ISPR_BASE + 0x00000000UL)) 
#define NVIC_ISPR1      (*(volatile uint32_t *)(NVIC_ISPR_BASE + 0x00000004UL)) 
#define NVIC_ISPR2      (*(volatile uint32_t *)(NVIC_ISPR_BASE + 0x00000008UL)) 

#define NVIC_ICPR_BASE  NVIC_REG_BASE + 0x00000180UL
#define NVIC_ICPR0      (*(volatile uint32_t *)(NVIC_ICPR_BASE + 0x00000000UL)) 
#define NVIC_ICPR1      (*(volatile uint32_t *)(NVIC_ICPR_BASE + 0x00000004UL)) 
#define NVIC_ICPR2      (*(volatile uint32_t *)(NVIC_ICPR_BASE + 0x00000008UL)) 

#define NVIC_IABR_BASE  NVIC_REG_BASE + 0x00000200UL
#define NVIC_IABR0      (*(volatile uint32_t *)(NVIC_IABR_BASE + 0x00000000UL)) 
#define NVIC_IABR1      (*(volatile uint32_t *)(NVIC_IABR_BASE + 0x00000004UL)) 
#define NVIC_IABR2      (*(volatile uint32_t *)(NVIC_IABR_BASE + 0x00000008UL)) 

#define NVIC_IPR_BASE  NVIC_REG_BASE + 0x00000300UL
#define NVIC_IPR0      (*(volatile uint32_t *)(NVIC_IPR_BASE + 0x00000000UL)) 
#define NVIC_IPR20     (*(volatile uint32_t *)(NVIC_IPR_BASE + 0x00000020UL)) 

#define NVIC_STIR_BASE  NVIC_REG_BASE + 0x00000E00UL
#define NVIC_STIR      (*(volatile uint32_t *)(NVIC_STIR_BASE + 0x00000000UL)) 

// EXTI registers
#define EXTI_REG_BASE   0x40010400UL
#define EXTI_IMR        (*(volatile uint32_t *)(EXTI_REG_BASE))  // EXTI interrupt ctl reg
#define EXTI_EMR        (*(volatile uint32_t *)(EXTI_REG_BASE + 0x00000004UL))  // EXTI event ctl reg
#define EXTI_RTSR       (*(volatile uint32_t *)(EXTI_REG_BASE + 0x00000008UL))  // EXTI rising edge sel reg
#define EXTI_FTSR       (*(volatile uint32_t *)(EXTI_REG_BASE + 0x0000000CUL))  // EXTI falling edge sel reg
#define EXTI_PR         (*(volatile uint32_t *)(EXTI_REG_BASE + 0x00000014UL))  // EXTI pending reg
#define EXTI_SWIER      (*(volatile uint32_t *)(EXTI_REG_BASE + 0x00000010UL))  // EXTI SW int reg

// EXTI control 
#define EXTI_LINE17     0x00020000UL

// interrupt vector table offsets
#define IVEC_BASE       0x00000000UL
#define IVEC_RTC_ALARM  (*(volatile uint32_t *)(IVEC_BASE + 0x000000E4UL))       // int vector for rtc alarm

// backup registers
#define BKP_REG_BASE    0x40006C00UL
#define BKP_REGS        (*(volatile uint32_t *)(BKP_REG_BASE + 0x00000004))
#define BKP_REGS1       (*(volatile uint32_t *)(BKP_REG_BASE + 0x00000014))
#define BKP_REGS2       (*(volatile uint32_t *)(BKP_REG_BASE + 0x00000018))
#define BKP_REGS3       (*(volatile uint32_t *)(BKP_REG_BASE + 0x0000001C))
#define BKP_REGS4       (*(volatile uint32_t *)(BKP_REG_BASE + 0x00000020))
#define BKP_CR          (*(volatile uint32_t *)(BKP_REG_BASE + 0x00000030))
#define BKP_CSR         (*(volatile uint32_t *)(BKP_REG_BASE + 0x00000034))

typedef struct {
   uint32_t bkup_regs[50];
}BACKUP_REGS;
typedef BACKUP_REGS *pBkpRegs;


#endif               // end _STM32LIBS_REGS_H