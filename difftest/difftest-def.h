#ifndef __DIFFTEST_DEF_H
#define __DIFFTEST_DEF_H

#if defined(CPU_NUTSHELL)
#elif defined(CPU_XIANGSHAN)
#elif defined(CPU_ROCKET_CHIP)
#else
// This is the default CPU
#define CPU_NUTSHELL
#endif

#if defined(CPU_XIANGSHAN) || defined(CPU_ROCKET_CHIP)
#define CONFIG_DIFF_FPU
#endif

#if defined(CPU_XIANGSHAN)
#define CONFIG_DIFF_DEBUG_MODE
#define CONFIG_DIFF_RVH
#define CONFIG_DIFF_RVV
#define CONFIG_DIFF_ZICOND
#define CONFIG_DIFF_ZICNTR
#define CONFIG_DIFF_ZIHPM
#define CONFIG_DIFF_SDTRIG
#endif

#if defined(CPU_NUTSHELL)
#define CONFIG_DIFF_ISA_STRING "rv64imaczicsr_zifencei"
#define CONFIG_MEMORY_SIZE     (2 * 1024 * 1024 * 1024UL)
#define CONFIG_FLASH_BASE      0x40000000UL
#define CONFIG_FLASH_SIZE      0x1000UL
#define CONFIG_PMP_NUM         0
#define CONFIG_PMP_MAX_NUM     0
#define CONFIG_TRIGGER_NUM     0
#define CONFIG_MAX_PADDR_BITS  32
#define CONFIG_MMU_CAPABILITY  IMPL_MMU_SV39
#define CONFIG_MISALIGNED      false

#elif defined(CPU_XIANGSHAN)
#ifdef CONFIG_DIFF_RVH
    #define RVH_ISA_STRING "H"
#else
    #define RVH_ISA_STRING ""
#endif
#ifdef CONFIG_DIFF_RVV
    #define RVV_ISA_STRING "V"
#else
    #define RVV_ISA_STRING ""
#endif
#ifdef CONFIG_DIFF_ZICOND
    #define ZICOND_ISA_STRING "_zicond"
#else
    #define ZICOND_ISA_STRING ""
#endif
#ifdef CONFIG_DIFF_ZICNTR
    #define ZICNTR_ISA_STRING "_zicntr"
#else
    #define ZICNTR_ISA_STRING ""
#endif
#ifdef CONFIG_DIFF_ZIHPM
    #define ZIHPM_ISA_STRING "_zihpm"
#else
    #define ZIHPM_ISA_STRING ""
#endif
#ifdef CONFIG_DIFF_SDTRIG
    #define SDTRIG_ISA_STRING "_sdtrig"
#else
    #define SDTRIG_ISA_STRING ""
#endif

#define CONFIG_DIFF_ISA_STRING \
    "RV64IMAFDC" \
    RVV_ISA_STRING \
    RVH_ISA_STRING \
    ZICOND_ISA_STRING \
    ZICNTR_ISA_STRING \
    ZIHPM_ISA_STRING \
    SDTRIG_ISA_STRING \
    "_zacas" \
    "_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx" \
    "_zimop_zcmop_zcb" \
    "_zknd_zkne_zknh_zksed_zksh" \
    "_zvbb" \
    "_zfa_zfh_zvfh" \
    "_svinval_sscofpmf" \
    "_svpbmt" \
    "_zicbom_zicboz" \
    "_sstc" \
    "_smrnmi_smdbltrp_ssdbltrp" \
    "_smmpm_smnpm_ssnpm" \

#define CONFIG_MEMORY_SIZE     (16 * 1024 * 1024 * 1024UL)
#define CONFIG_FLASH_BASE      0x10000000UL
#define CONFIG_FLASH_SIZE      0x100000UL
#define CONFIG_PMP_NUM         16
#define CONFIG_PMP_MAX_NUM     16
#define CONFIG_PMP_GRAN        12
#define CONFIG_TRIGGER_NUM     4
#define CONFIG_MAX_PADDR_BITS  48
#define CONFIG_MMU_CAPABILITY  IMPL_MMU_SV48
#define CONFIG_MISALIGNED      true

#elif defined(CPU_ROCKET_CHIP)
#define CONFIG_DIFF_ISA_STRING "rv64imafdczicsr_zifencei_zihpm_zicntr"
#define CONFIG_MEMORY_SIZE     (2 * 1024 * 1024 * 1024UL)
#define CONFIG_FLASH_BASE      0x10000000UL
#define CONFIG_FLASH_SIZE      0x10000UL
#define CONFIG_PMP_NUM         0
#define CONFIG_PMP_MAX_NUM     64
#define CONFIG_TRIGGER_NUM     0
#define CONFIG_MAX_PADDR_BITS  32
#define CONFIG_MMU_CAPABILITY  IMPL_MMU_SV39
#define CONFIG_MISALIGNED      false
#endif

#endif
