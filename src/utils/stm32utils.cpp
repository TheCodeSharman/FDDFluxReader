#include "utils/stm32utils.h"
/* 
    Activates the system bootloader via software, avoiding the need to physically
    pressed the BOOT0 button to update the firmware.

    Also there seems to be an issue with the bootloader not always establishing the correct
    USB interface clocks from a physical reset. Activating by software is more reliable.

    On page 56 of the boot mode application note (see reference below):

    "At bootloader startup, the HSITRIM value is set to 0 (in HSITRIM bits on RCC_CR register)
    instead of default value (16), as a consequence a deviation is generated in crystal measurement.
    For better results, use the smallest supported crystal value (i.e. 4 MHz)."

    So I'm assuming this explains why software activation is more reliable: HSITRIM will be correctly
    set at the point this code is executed. (The black pill I'm using uses a 25Mhz crystal)

    Software activation of system bootloader procedure
    ==================================================

    (code derived from: 
        https://techoverflow.net/2021/10/17/implementing-stm32-dfu-bootloader-firmware-upgrade-in-marlin-using-m997/
        https://stm32f4-discovery.net/2017/04/tutorial-jump-system-memory-software-stm32/
        https://github.com/akospasztor/stm32-bootloader)

    See reference docs:
     https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf

    "In addition to patterns described above, user can execute bootloader by performing a jump
    to system memory from user code. Before jumping to bootloader user must:
    • Disable all peripheral clocks
    • Disable used PLL
    • Disable interrupts
    • Clear pending interrupts" (page 28)

    Also see: 
        https://www.st.com/resource/en/reference_manual/dm00119316-stm32f411xc-e-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf

    "After this startup delay is over, the CPU fetches the top-of-stack value from address 0x0000 0000, then starts code 
    execution from the boot memory starting from 0x0000 0004." (page 41)

    And to remap memory using SYSCFG_MEMRMP register (page 139):

    "This register is used for specific configurations on memory remap:

    • Two bits are used to configure the type of memory accessible at address 0x0000 0000.
      These bits are used to select the physical remap by software and so, bypass the BOOT
      pins.
        
    • After reset these bits take the value selected by the BOOT pins. When booting from
      main Flash memory with BOOT0 pin set to 0 this register takes the value 0x00."

    So the following code:
        - deinitalizes any PLL and peripheral clocks, disable any interrupts
        - remaps the the memory available at 0x00000000 to the system ROM
        - loads the stack pointer from 0x00000000
        - jumps to the address stored at 0x00000004
*/

/*
    The following code is used to set the stack pointer:

    __set_MSP(*((uint32_t*) 0x00000000));

    But this dereferences a NULL pointer which gcc considers undefined behaviour
    and will insert an undef instruction trap instead due to the optimiser option
    -fisolate-erroneous-paths-dereference.
    
    (see https://bugs.launchpad.net/gcc-arm-embedded/+bug/1479823)

    So to avoid this we disable this optimizer option just for the scope of
    this function.
*/
__attribute__((optimize("no-isolate-erroneous-paths-dereference"))) 
__attribute__ ((noreturn))
void enter_dfu_bootloader() {
    
    // Deinitialize the STM32 - resets PLL and disables interrupts
    __HAL_RCC_DMA1_CLK_DISABLE();
    HAL_RCC_DeInit();
    HAL_DeInit();
    
    // Shutdown the SysTick peripheral clock - oddly HAL_DeInit()
    // isn't enough??
    SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

    // Remap system bootloader to page 0x00000000
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    // Load the stack pointer from the address stored at 0x00000000
    __set_MSP(*((uint32_t*) 0x00000000));

    // Jump to the address stored at 0x00000004
    ((void (*)(void)) *((uint32_t*) 0x00000004))();
    while(1);
}