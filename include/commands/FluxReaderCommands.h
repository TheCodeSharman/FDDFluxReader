#ifndef ENTER_BOOTLOADER_COMMAND_H
#define ENTER_BOOTLOADER_COMMAND_H

#include "FluxReaderCommandParser.h"
#include "utils/stm32utils.h"

class EnterBootloaderCommand : public FluxReaderCommand {
    public:
        EnterBootloaderCommand() {
            command = "bootloader";
        }
        virtual void execute() {
            enter_dfu_bootloader();
        }

};

class NullCommand : public FluxReaderCommand {
    public:
        NullCommand() {
            command = "";
        }
        virtual void execute() {
            // Do nothing
        }

};

#endif