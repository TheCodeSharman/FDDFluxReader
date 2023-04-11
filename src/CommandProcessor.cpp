#include "CommandProcessor.h"


CommandProcessor::CommandProcessor(Stream& serialDevice, MultiTask& multitask, CommandParser& commandParser, std::vector<Command*>& commands )
    : serialDevice(serialDevice),
      multitask(multitask),
      commandParser(commandParser),
      commands(commands)
     {
}

Command* CommandProcessor::findCommand() {
    for( auto command : commands) {
        if ( command->match(commandParser) ) {
            return command;
        }
    }
    return NULL;
}

void CommandProcessor::processCommand() {
    auto command = findCommand();
    if (command == NULL) {
        unknownCommand();
    } else {
        command->execute();
    }
    commandParser.clear();
    ready();
}


void CommandProcessor::processSerial() {
  while ( Serial.available() > 0 ) {
      char inChar = Serial.read();
      echo(inChar);
      if ( commandParser.receive( inChar ) ) {
        processCommand();
      }
  }
}

void CommandProcessor::init() {
    multitask.every(0,std::bind(&CommandProcessor::processSerial, this));
} 