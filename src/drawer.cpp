#include "drawer.h"
#include <chrono>
#include <thread>
#include <ncurses.h>

namespace int6502 {
	volatile bool stopped = false;
	
	
	struct Color {
		short r, g, b;
	};
	
	struct Pair {
		short fg, bg;
	};
	
	static const short DEFAULT_COLORS_COUNT = 16;
	
	Color defaultColors[DEFAULT_COLORS_COUNT];
	Pair defaultPairs[DEFAULT_COLORS_COUNT];
	
	
	void saveDefaultColors() {
		for (short i = 0; i < DEFAULT_COLORS_COUNT; ++i) {
			Color& color = defaultColors[i];
			Pair& pair = defaultPairs[i];
			
			color_content(i, &color.r, &color.g, &color.b);
			pair_content(i, &pair.fg, &pair.bg);
		}
	}
	
	void restoreDefaultColors() {
		for (short i = 0; i < DEFAULT_COLORS_COUNT; ++i) {
			Color& color = defaultColors[i];
			Pair& pair = defaultPairs[i];
			
			init_color(i, color.r, color.g, color.b);
			init_pair(i, pair.fg, pair.bg);
		}
	}
	
	
	
	static const std::chrono::milliseconds INTERVAL(100);
	
	static const int
			COLOR_ORANGE      = 0x8,
			COLOR_BROWN       = 0x9,
			COLOR_LIGHT_RED   = 0xA,
			COLOR_DARK_GREY   = 0xB,
			COLOR_GREY        = 0xC,
			COLOR_LIGHT_GREEN = 0xD,
			COLOR_LIGHT_BLUE  = 0xE,
			COLOR_LIGHT_GREY  = 0xF,
			COLOR_BORDER      = 0x10;
	
	
	
	void initRgbPair(int index, int color) {
		init_color(index,
				((color >> 16) & 0xFF) * 1000 / 255,
				((color >> 8)  & 0xFF) * 1000 / 255,
				((color)       & 0xFF) * 1000 / 255
		);
		
		init_pair(index, COLOR_WHITE, index);
	}
	
	
	void initColors() {
		initRgbPair(COLOR_BLACK,       0x000000);
		initRgbPair(COLOR_RED,         0xFF0000);
		initRgbPair(COLOR_GREEN,       0x00C000);
		initRgbPair(COLOR_YELLOW,      0xFFFF00);
		initRgbPair(COLOR_BLUE,        0x5770FF);
		initRgbPair(COLOR_MAGENTA,     0xC800FF);
		initRgbPair(COLOR_CYAN,        0x60fffa);
		initRgbPair(COLOR_WHITE,       0xFFFFFF);
		initRgbPair(COLOR_ORANGE,      0xFF8000);
		initRgbPair(COLOR_BROWN,       0xC04040);
		initRgbPair(COLOR_LIGHT_RED,   0xFFA0A0);
		initRgbPair(COLOR_DARK_GREY,   0x404040);
		initRgbPair(COLOR_GREY,        0x808080);
		initRgbPair(COLOR_LIGHT_GREEN, 0x00FF00);
		initRgbPair(COLOR_LIGHT_BLUE,  0x80C0FF);
		initRgbPair(COLOR_LIGHT_GREY,  0xC0C0C0);
		
		init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_WHITE);
	}
	
	
	
	
	int getColor(uint8_t byte) {
		switch (byte & 0xF) {
			case 0x0: return COLOR_BLACK;
			case 0x1: return COLOR_WHITE;
			case 0x2: return COLOR_RED;
			case 0x3: return COLOR_CYAN;
			case 0x4: return COLOR_MAGENTA;
			case 0x5: return COLOR_GREEN;
			case 0x6: return COLOR_BLUE;
			case 0x7: return COLOR_YELLOW;
			case 0x8: return COLOR_ORANGE;
			case 0x9: return COLOR_BROWN;
			case 0xA: return COLOR_LIGHT_RED;
			case 0xB: return COLOR_DARK_GREY;
			case 0xC: return COLOR_GREY;
			case 0xD: return COLOR_LIGHT_GREEN;
			case 0xE: return COLOR_LIGHT_BLUE;
			case 0xF: return COLOR_LIGHT_GREY;
			default: return COLOR_BLACK;
		}
	}
	
	
	
	int getStartY() {
		return std::max(0, (LINES - WIN_HEIGHT) / 2);
	}
	
	int getStartX() {
		return 0;
	}
	
	
	void drawBorder() {
		const int startY = getStartY();
		const int startX = getStartX();
			
		chtype border = ' ' | COLOR_PAIR(COLOR_BORDER);
		
		move(startY, startX);
		for (int x = 0; x < WIN_WIDTH; ++x) addch(border);
		
		move(startY + WIN_HEIGHT - 1, startX);
		for (int x = 0; x < WIN_WIDTH; ++x) addch(border);
		
		for (int y = 1; y < WIN_HEIGHT - 1; ++y) {
			mvaddch(startY + y, startX, border);
			addch(border);
			
			mvaddch(startY + y, startX + WIN_WIDTH - 2, border);
			addch(border);
		}
	}
	
	
	void update(uint8_t* inputMem, uint8_t* gpuMem) {
		const int startY = getStartY();
		const int startX = getStartX();
		
		for (int y = 0; y < HEIGHT; ++y) {
			move(startY + y + 1, startX + 2);
			
			for (int x = 0; x < WIDTH; ++x) {
				chtype c = ' ' | COLOR_PAIR(getColor(gpuMem[x + y * WIDTH]));
				
				addch(c);
				addch(c);
			}
		}
		
		refresh();
		
		int ch = getch();
		
		switch (ch) {
			case KEY_UP:    *inputMem = 'w'; break;
			case KEY_LEFT:  *inputMem = 'a'; break;
			case KEY_DOWN:  *inputMem = 's'; break;
			case KEY_RIGHT: *inputMem = 'd'; break;
			default:
				if (ch >= 0x20 && ch <= 0x7F) {
					*inputMem = uint8_t(ch);
				}
		}
	}
	
	
	void draw(uint8_t* inputMem, uint8_t* gpuMem) {
		nodelay(stdscr, true);
		initColors();
		drawBorder();
		
		while (!stopped) {
			update(inputMem, gpuMem);
			std::this_thread::sleep_for(INTERVAL);
		}
		
		update(inputMem, gpuMem);
		nodelay(stdscr, false);
	}
}
