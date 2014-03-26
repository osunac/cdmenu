/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Ricardo Liang
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ctype.h>
#include <form.h>
#include <menu.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

FIELD *field[2] = { NULL, NULL };
FORM *form = NULL;
ITEM **entries = NULL;
ITEM **buffer = NULL;
MENU *menu = NULL;

size_t text_length(void)
{
	char *text = field_buffer(field[0], 0);

	size_t l;
	for (l = strlen(text); l > 0 && isspace(text[l - 1]); --l) {}

	return l;
}

void text_copy(char *destination, size_t length)
{
	strncpy(destination, field_buffer(field[0], 0), length);
	destination[length] = '\0';
}

void update_screen(void)
{
	refresh();
	wrefresh(form_win(form));
	wrefresh(menu_win(menu));
}

void update_menu(void)
{
	unpost_menu(menu);
	set_menu_items(menu, buffer);
	post_menu(menu);
}

void update_buffer(void)
{
	form_driver(form, REQ_VALIDATION);

	size_t size = text_length();
	char filter[size + 1];
	text_copy(filter, size);

	size_t d = 0;
	ITEM **e;
	for (e = entries; *e != NULL; ++e)
		if (filter[0] == '\0' || strstr(item_name(*e), filter) != NULL)
			buffer[d++] = *e;
	buffer[d] = NULL;

	update_menu();
}

void init_form(void)
{
	field[0] = new_field(1, COLS, 0, 0, 0, 0);

	form = new_form(field);
	set_form_win(form, newwin(1, COLS, 0, 0));
	post_form(form);
}

void init_menu(void)
{
	menu = new_menu(buffer);
	set_menu_win(menu, newwin(LINES, COLS, 1, 0));
	post_menu(menu);

	update_menu();
	update_screen();
}

void init_curses(void)
{
	FILE *temp = stdout;
	stdout = stderr;
	stderr = temp;

	atexit((void (*)(void))endwin);
	signal(SIGSEGV, exit);
	initscr();

	curs_set(0);
	keypad(stdscr, true);
	noecho();
	raw();
}

void init_entries(const char **strings, size_t length)
{
	size_t i, e = 0;
	for (i = 0; i < length; ++i) {
		if (strings[i][0] != '\0') {
			entries[e] = buffer[e] = new_item(strings[i], strings[i]);
			++e;
		}
	}

	entries[e] = buffer[e] =  NULL;
}

void init(ITEM **a, ITEM **b, const char **strings, size_t length)
{
	entries = a;
	buffer = b;

	init_entries(strings, length);
	init_curses();
	init_form();
	init_menu();
}

void main_loop(void)
{
	for (;;) {
		int key = getch();
		switch (key) {
		case 3: // c-c
		case 27: // esc
			return;
		case 10: // return
			fputs(item_name(current_item(menu)), stderr);
			return;
		case 6: // c-f
		case 9: // tab
		case 258: // down
			menu_driver(menu, REQ_NEXT_ITEM);
			break;
		case 2: // c-b
		case 353: // backtab
		case 259: // up
			menu_driver(menu, REQ_PREV_ITEM);
			break;
		case 8: // c-h
		case 260: // left
			form_driver(form, REQ_PREV_CHAR);
			break;
		case 12: // c-l
		case 261: // right
			form_driver(form, REQ_NEXT_CHAR);
			break;
		case 360: // end
			form_driver(form, REQ_END_LINE);
			break;
		case 262: // home
			form_driver(form, REQ_BEG_LINE);
			break;
		case 32: // space
			form_driver(form, REQ_INS_CHAR);
			update_buffer();
			break;
		case 330: // delete
			form_driver(form, REQ_DEL_CHAR);
			update_buffer();
			break;
		case 263: // backspace
			form_driver(form, REQ_DEL_PREV);
			update_buffer();
			break;
		case 21: // c-u
			form_driver(form, REQ_CLR_FIELD);
			update_buffer();
			break;
		case 23: // c-w
			form_driver(form, REQ_DEL_WORD);
			update_buffer();
			break;
		default:
			form_driver(form, key);
			update_buffer();
		}
		update_screen();
	}
}

int main(int argc, const char **argv)
{
	ITEM *a[argc], *b[argc];
	init(a, b, argv + 1, argc - 1);
	main_loop();
	return 0;
}

