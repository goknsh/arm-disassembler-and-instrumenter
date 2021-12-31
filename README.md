# ARM Disassembler and Instrumenter
This project can disassemble ARM instructions and follow control flows.

### Usage
Currently, only programs that take in an interger value return an integer value are able to be disassembled and have their control flow followed. To use this program, simply create a function and compile it with this project, with `user_prog` as an alias to your function. This program will ask for a input to send your program, then it will disassemble and follow the control flow and finally output the result with your given input.

See the `Makefile` for comiplation instructions and running this program with the 4 example functions.

> NOTE: Currently, this program is not able to follow the `pop` instruction setting the `pc` register.