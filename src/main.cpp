#include "translator.h"
#include "executor.h"
#include "drawer.h"
#include "error_codes.h"
#include "util.h"
#include <csignal>
#include <vector>
#include <ncurses.h>

namespace int6502 {
	int run(const char* filename) {
		std::vector<uint8_t> code;
		
		int res = translate(filename, code);
		if (res != EXIT_SUCCESS) return res;
		
		return executeCode(code);
	}
}


void end_ncurses() {
	int6502::restoreDefaultColors();
	nodelay(stdscr, false);
	keypad(stdscr, false);
	echo();
	curs_set(true);
	endwin();
}

void on_signal(int signum) {
	end_ncurses();
	std::signal(signum, SIG_DFL);
}


int main(int argc, const char* args[]) {
	using namespace int6502;

	if (argc != 2) {
		return error(ARGUMENTS_ERROR, "Usage: %s <file>", args[0]);
	}
	
	std::atexit(end_ncurses);
	std::signal(SIGABRT, on_signal);
	std::signal(SIGSEGV, on_signal);
	
	initscr();
	
	if (!has_colors() || !can_change_color()) {
		end_ncurses();
		return error(COLOR_NOT_SUPPORTED_ERROR, "Your terminal does not support colors");
	}

	start_color();
	curs_set(false);
	noecho();
	keypad(stdscr, true);
	saveDefaultColors();
	
	return run(args[1]);
}
