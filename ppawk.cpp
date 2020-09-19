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
	while (char nxt = next()) {
		if (nxt == '}') {
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

void run() {
	while (char nxt = next()) {
		switch (nxt) {
		case EOF: break;
		case '#': run_comment(); continue;
		case '\n': putchar(nxt); continue;
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
