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

void run_escape();
void run_regex();

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

void run_string() {
	set_color(6);
	putchar('"');
	while (char nxt = next()) {
		if (nxt == '\\') {
			run_escape();
			continue;
		}
		putchar(nxt);
		if (nxt == '"') {
			break;
		}
	}
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
		if (nxt == '"') {
			run_string();
			set_color(3); // hack, todo: proper color stack
		} else {
			putchar(nxt);
		}
	}
}

void run_rule() {
	set_color(2);
	while (char nxt = next()) {
		if (nxt == '/') {
			run_regex();
			set_color(2); // hack, todo: proper color stack
			continue;
		}
		if (nxt == '{') {
			run_action();
			break;
		}
		putchar(nxt);
		if (nxt == '\n') {
			break;
		}
	}
	reset_color();
}

void run_escape() {
	int lvl = 1;
	const char bs = '\\';
	putchar(bs);
	while (char nxt = next()) {
		if (nxt != bs) {
			if (lvl % 2 == 0) {
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
			give_back(nxt);
			run_rule();
			continue;
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
