[README.md in russian](https://github.com/petr590/Interpretator-6502/blob/master/README-ru.md)


This project is an assembler interpreter **6502** , written using the **NCurses** library.
After the program is launched, memory segments are available:
- **0x00** - **0xFF**: zero page.
The random generator is located at **0xFE**, and the last pressed arrow is at **0xFF**.
- **0x100** - **0x1FF**: stack.
- **0x200** - **0x5FF**: 32x32 pixel display.
- **0x600** - **0xFFF**: bytecode.
- **0x1000** - **0x1FFF**: the rest of the memory.

## Color list:
- 0x0: Black
- 0x1: White
- 0x2: Red
- 0x3: Blue
- 0x4: Purple
- 0x5: Green
- 0x6: Blue
- 0x7: Yellow
- 0x8: Orange
- 0x9: Brown
- 0xA: Light Red
- 0xB: Dark Grey
- 0xC: Grey
- 0xD: Light Green
- 0xE: Light Blue
- 0xF: Light Grey
When rendering the display, the highest half-byte is ignored.

## Building the project:
```sh
git clone https://github.com/petr590/Interpretator-6502.git
cd Interpretator-6502/
cmake . -DCMAKE_BUILD_TYPE=Release
make
```

## Launch:
`./int6502 <file>`

## Examples of 6502 assembler programs:
The **colors.6502** file contains code that displays all colors in the specified order.
In the file **2048.6502** The game code is 2048.
