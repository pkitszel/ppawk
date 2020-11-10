#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <string>
#include <vector>
using namespace std;

#define die(str) ({ perror(str); int rc = __LINE__ % 100; rc += 101 * !rc; exit(rc); 0;})

struct lexem : std::string {
	lexem() {}
	lexem(char c) :std::string(1, c) {}
	lexem(const std::string &s) :std::string(s) {}
	lexem(const char *s) :std::string(s) {}
	explicit operator bool() const noexcept { return !empty(); }
	char first() const { return at(0); }
	bool operator==(char c) const { return *this == string(1, c); }
};

typedef void (* char2fun_t)();
char2fun_t &c2f(lexem x) {
	static char2fun_t tab[256];
	if (x.size() == 1) {
		return tab[(unsigned char) x.first()];
	}
	return tab[0];
}

struct color_stack {
	static stack<int> s;

	color_stack(int c) { s.push(c); }
	~color_stack() { s.pop(); }

	static void ensure_color() {
		static int prev = -1;
		int color = s.empty() ? -1 : s.top();
		if (prev == color) {
			return;
		}
		prev = color;
		if (color == -1) {
			printf("\e[0m");
		} else {
			color += color < 8 ? 30 : 90-8;
			printf("\e[0;%dm", color);
		}
	}
};
stack<int> color_stack::s;

#define PASTE_HELPER(a,b) a ## b
#define PASTE(a,b) PASTE_HELPER(a,b)
#define with_color color_stack PASTE(dummy_with_color, __LINE__) =

bool is_kw(const lexem &lx) {
	if (c2f('{')) {
		// in rule/toplevel
		return lx == "function";
	}
	// inside action
	static vector<lexem> kws = {
		"break",
		"case",
		"continue",
		"default",
		"delete",
		"do",
		"else",
		"exit",
		"for",
		"if",
		"in",
		"next",
		"nextfile",
		"print",
		"printf",
		"return",
		"switch",
		"while",
	};
	return binary_search(kws.begin(), kws.end(), lx);
}

static void print(const lexem &lx) {
	vector<color_stack> v;
	if (is_kw(lx)) {
		v.emplace_back(3);
	}
	color_stack::ensure_color();
	printf("%s", lx.c_str());
}

struct lexer {
	lexem next() {
		char first = next_char();
		string tok(1, first);
		if (isnameletter(first)) {
			while (char x = next_char()) {
				if (!isnameletter(x)) {
					give_back(x);
					break;
				}
				tok += x;
			}
		} else if (isplusorminus(first)) {
			while (char x = next_char()) {
				if (x != first) {
					give_back(x);
					break;
				}
				tok += x;
			}
		} else if (!first) {
			tok.clear(); // EOF
		}
		if (!isspace(curr[0]) || curr[0] == '\n') {
			prev = curr;
		}
		curr = tok;
		return tok;
	}

	bool is_div_infix() const {
		if (prev.empty()) {
			return false;
		}
		char first = prev[0];
		if (suggests_infix(first)) {
			return true;
		}
		if (isplusorminus(first)) {
			return prev.size() % 2 == 0;
		}
		if (prev == "print" || prev == "printf") {
			return false;
		}
		return isnameletter(first);
	}

private:
	string prev, curr;

	char next_char() {
		char x = (char) getchar();
		return x == EOF ? 0 : x;
	}

	void give_back(char x) {
		ungetc(x, stdin);
	}

	static bool isnameletter(char x) {
		return x == '_' || isalnum(x);
	}

	static bool isplusorminus(char x) {
		return x == '+' || x == '-';
	}

	static bool suggests_infix(char x) {
		return x == '\"' || x == ')' || x == '.' || x == ']';
	}
};
lexer lx;

void run_escape();
void run_regex();

void run_comment() {
	lexem nxt = lx.next();
	with_color(nxt == '!' ? 1 : 8);
	print("#" + nxt);
	while (lexem nxt = lx.next()) {
		print(nxt);
		if (nxt == '\n') {
			break;
		}
	}
}

void run_string() {
	with_color 6;
	print('"');
	while (lexem nxt = lx.next()) {
		if (nxt == '\\') {
			run_escape();
			continue;
		}
		print(nxt);
		if (nxt == '"') {
			break;
		}
	}
}

void run_common(lexem nxt) {
	if (auto f = c2f(nxt)) {
		if (f == run_regex && lx.is_div_infix()) {
			print('/');
			return;
		}
		f();
		return;
	}
	print(nxt);
}

void run_action() {
	c2f('{') = nullptr;
	with_color 4; // todo =10 with 16 colors
	print('{');
	with_color 7; // normal body
	int lvl = 1;
	while (lexem nxt = lx.next()) {
		lvl += (nxt == '{') - (nxt == '}');
		if (!lvl) {
			break;
		}
		run_common(nxt);
	}
	with_color 4; // todo =10 with 16 colors
	print('}');
}

void run() {
	with_color 2;
	while (lexem nxt = lx.next()) {
		c2f('{') = run_action;
		run_common(nxt);
	}
}

void run_escape() {
	print('\\');
	print(lx.next());
}

void run_bracket() {
	{ with_color 4; print('['); }
	int inside = 0, neg = 0;
	with_color 5;
	for (lexem nxt; (nxt = lx.next()); ++inside) {
		if (nxt == '^' && inside == 0) {
			with_color 4;
			print(nxt);
			neg = 1;
			continue;
		}
		if (nxt == ']') {
			if (inside == neg) {
				print(nxt);
				continue;
			}
			with_color 4;
			print(nxt);
			break;
		}
		print(nxt);
		if (nxt == '\n') {
			break;
		}
	}
}

void run_regex() {
	with_color 5;
	print('/');
	while (lexem nxt = lx.next()) {
		if (nxt == '\\') {
			run_escape();
			continue;
		}
		if (nxt == '[') {
			run_bracket();
			continue;
		}
		print(nxt);
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
