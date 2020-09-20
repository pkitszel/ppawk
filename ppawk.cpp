#include <cstdio>
#include <cstdlib>

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

typedef void (* char2fun_t)();
char2fun_t &c2f(char x) {
	static char2fun_t tab[256];
	return tab[(unsigned char) x];
}

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

void run_common(char nxt) {
	if (auto f = c2f(nxt)) {
		f();
		return;
	}
	putchar(nxt);
}

void run_action() {
	c2f('{') = nullptr;
	set_color(4); // todo =10 with 16 colors
	putchar('{');
	int lvl = 1;
	while (char nxt = next()) {
		lvl += (nxt == '{') - (nxt == '}');
		if (!lvl) {
			set_color(4); // todo =10 with 16 colors
			putchar('}');
			break;
		}
		set_color(3);
		run_common(nxt);
	}
}

void run() {
	while (char nxt = next()) {
		set_color(2);
		c2f('{') = run_action;
		run_common(nxt);
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

void init_c2f() {
	c2f('\\') = run_escape;
	c2f('"') = run_string;
	c2f('/') = run_regex;
	c2f('#') = run_comment;
}

int main(int argc, char **argv) {
	if (argc == 2) {
		freopen(argv[1], "r", stdin) || die("freopen");
	}
	init_c2f();
	run();
}
