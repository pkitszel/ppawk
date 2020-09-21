#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stack>
#include <string>
using namespace std;

#define die(str) ({ perror(str); int rc = __LINE__ % 100; rc += 101 * !rc; exit(rc); 0;})

void reset_color() { printf("\e[0m"); }

void set_color(int color) {
	color += color < 8 ? 30 : 90-8;
	printf("\e[0;%dm", color);
}

struct with_color {
	static stack<int> s;

	with_color(int x) {
		s.push(x);
		print();
	}

	~with_color() {
		s.pop();
		print();
	}

	void print() {
		if (s.empty()) {
			reset_color();
		} else {
			set_color(s.top());
		}
	}
};
stack<int> with_color::s;
#define PASTE_HELPER(a,b) a ## b
#define PASTE(a,b) PASTE_HELPER(a,b)
#define with_color with_color PASTE(dummy_with_color, __LINE__) =

string the_last_chars;

string last_word() {
	istringstream iss(the_last_chars);
	while (iss >> the_last_chars) {
	}
	return the_last_chars;
}

bool is_div_infix() {
	char push = ' ';
	if (!the_last_chars.empty()) {
		push = the_last_chars.back();
		the_last_chars.pop_back();
	}
	bool ret = false;
	string lw = last_word();
	char second = lw.empty() ? ' ' : lw.back();
	char first = lw.size() <= 1 ? ' ' : lw[lw.size()-2];
	if (first == second && (first == '-' || first == '+')) {
		ret = true;
	} else if (second == '"' || second == ')' || second == ']' || second == '.') {
		ret = true;
	} else if (isalnum(second)) {
		static string noninfix[] = {
			"print",
			"printf",
			";print",
			";printf",
			"{print",
			"{printf",
			"}print",
			"}printf",
		};
		ret = find(noninfix, noninfix+8, lw) == noninfix+8;
	}
	the_last_chars += ' ';
	the_last_chars += push;
	return ret;
}

char next() {
	char x = (char) getchar();
	if (x == '\n') {
		the_last_chars.clear();
	} else {
		the_last_chars.push_back(x);
	}
	return x == EOF ? 0 : x;
}

void give_back(char x) {
	ungetc(x, stdin);
	the_last_chars.pop_back();
}

typedef void (* char2fun_t)();
char2fun_t &c2f(char x) {
	static char2fun_t tab[256];
	return tab[(unsigned char) x];
}

void run_escape();
void run_regex();

char run_to_nl(char to = 0) {
	char nxt;
	while ((nxt = next())) {
		if (nxt == to || nxt == '\n') {
			break;
		}
		putchar(nxt);
	}
	return nxt;
}

void run_comment() {
	char nxt = next();
	with_color(nxt == '!' ? 1 : 8);
	printf("#%c", nxt);
	run_to_nl();
	putchar('\n');
}

void run_string() {
	with_color 6;
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
		if (f == run_regex && is_div_infix()) {
			putchar('/');
			return;
		}
		f();
		return;
	}
	putchar(nxt);
}

void run_action() {
	c2f('{') = nullptr;
	with_color 3;
	{
		with_color 4; // todo =10 with 16 colors
		putchar('{');
	}
	int lvl = 1;
	while (char nxt = next()) {
		lvl += (nxt == '{') - (nxt == '}');
		if (!lvl) {
			with_color 4; // todo =10 with 16 colors
			putchar('}');
			break;
		}
		run_common(nxt);
	}
}

void run() {
	with_color 2;
	while (char nxt = next()) {
		c2f('{') = run_action;
		run_common(nxt);
	}
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

void run_bracket() {
	{ with_color 4; putchar('['); }
	int inside = 0, neg = 0;
	with_color 5;
	for (char nxt; (nxt = next()); ++inside) {
		if (nxt == '^' && inside == 0) {
			with_color 4;
			putchar(nxt);
			neg = 1;
			continue;
		}
		if (nxt == ']') {
			if (inside == neg) {
				putchar(nxt);
				continue;
			}
			with_color 4;
			putchar(nxt);
			break;
		}
		putchar(nxt);
		if (nxt == '\n') {
			break;
		}
	}
}

void run_regex() {
	with_color 5;
	putchar('/');
	while (char nxt = next()) {
		if (nxt == '\\') {
			run_escape();
			continue;
		}
		if (nxt == '[') {
			run_bracket();
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
