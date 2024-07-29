#ifndef INT6502_DRAW_H
#define INT6502_DRAW_H

#include <mutex>
#include <cstdint>

namespace int6502 {
	static const int
			WIDTH = 32,
			HEIGHT = 32,
			WIDTH_CHARS = WIDTH * 2,
			HEIGHT_CHARS = HEIGHT,
			WIN_WIDTH = WIDTH_CHARS + 4,
			WIN_HEIGHT = HEIGHT_CHARS + 2;
	
	
	extern volatile bool stopped;
	
	// Читает 1024 байта из переданного указателя и отображает соответствующие цвета.
	// При нажатии клавиши записывает её код в inputMem
	// Выполняется, пока stopped == false
	extern void draw(uint8_t* inputMem, uint8_t* gpuMem);
	
	
	// Сохраняет цвета и цветовые пары
	extern void saveDefaultColors();
	
	// Восстанавливает сохранённые цвета и цветовые пары
	extern void restoreDefaultColors();
}

#endif /* INT6502_DRAW_H */
