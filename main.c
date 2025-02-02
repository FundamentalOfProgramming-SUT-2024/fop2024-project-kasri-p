#include "menu.h"

void boarder();

int main()
{
	setlocale(LC_ALL, "");
	initscr();
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	boarder();
	music();                          
	const char *title_art[] = {
		"╔═══════════════════════════════╗",
		"║             ROUGE             ║",
		"╚═══════════════════════════════╝"};
	attron(COLOR_PAIR(1) | A_BOLD);
	for (int i = 0; i < 3; i++)
	{
		mvprintw(LINES / 2 - 1 + i, COLS / 2 - 16, "%s", title_art[i]);
	}
	attroff(COLOR_PAIR(1) | A_BOLD);

	mvprintw(LINES / 2 + 3, COLS / 2 - 12, "press any key to continue");
	getch();
	signup_login();
	endwin();
	return 0;
}
