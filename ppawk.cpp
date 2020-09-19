#include "/home/przemek/downloads/def.h"

#define die(str) ({ perror(str); int rc = __LINE__ % 100; rc += 101 * !rc; exit(rc); 0;})

void reset_color() { printf("\e[0m"); }

void set_color(int color) {
	color += color < 8 ? 30 : 90-8;
	printf("\e[0;%dm", color);
}

char next() {
	char x = (char) getchar();
	return x == EOF ? 0 : x;
}

void give_back(char x) { ungetc(x, stdin); }

void run_to_nl() {
	while (char nxt = next()) {
		putchar(nxt);
		if (nxt == '\n') {
			break;
		}
	}
}

void run_comment() {
	char nxt = next();
	set_color(nxt == '!' ? 1 : 8);
	printf("#%c", nxt);
	run_to_nl();
}

void run_action() {
	set_color(4); // todo =10 with 16 colors
	putchar('{');
	set_color(3);
	int lvl = 1;
	while (char nxt = next()) {
		lvl += (nxt == '{') - (nxt == '}');
		if (!lvl) {
			set_color(4); // todo =10 with 16 colors
			putchar('}');
			break;
		}
		putchar(nxt);
	}
}

void run_to_action() {
	while (char nxt = next()) {
		if (nxt == '{') {
			run_action();
			break;
		}
		putchar(nxt);
		if (nxt == '\n') {
			break;
		}
	}
}

void run_rule(char first) {
	set_color(2);
	putchar(first);
	run_to_action();
	reset_color();
}

void run_escape() {
	int lvl = 1;
	const char bs = '\\';
	putchar(bs);
	while (char nxt = next()) {
		if (nxt != bs) {
			if (lvl & 1) {
				give_back(nxt);
			} else {
				putchar(nxt);
			}
			break;
		}
		putchar(bs);
		++lvl;
	}
}

void run_regex() {
	set_color(5);
	putchar('/');
	while (char nxt = next()) {
		if (nxt == '\\') {
			run_escape();
			continue;
		}
		putchar(nxt);
		if (nxt == '/') {
			reset_color(); // todo: pop color from stack
			break;
		}
	}
}

void run() {
	while (char nxt = next()) {
		switch (nxt) {
		case '#': run_comment(); continue;
		case '\n': putchar(nxt); continue;
		case '/': run_regex(); continue;
		default:
			run_rule(nxt); continue;
			//~ reset_color(); putchar(nxt); continue;
		}
	}
	reset_color();
	putchar('\n');
}

int main(int argc, char **argv) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin) || die("freopen");
	}
	run();
}
