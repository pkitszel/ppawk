package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"unicode"
)

type kind uint8

const (
	PLAIN kind = iota
	SHEBANG
	COMMENT
	KEYWORD
)

var color = map[kind]string{
	PLAIN:   "\033[0m",
	KEYWORD: "\033[0;32m",
}

type parslet struct {
	what kind
	text string
}

func parse(in <-chan string) <-chan parslet {
	out := make(chan parslet)
	go parse_run(in, out)
	return out
}

func parse_run(in <-chan string, out chan parslet) {
	for t := range in {
		var k kind
		switch {
		case t == "print":
			k = KEYWORD
		}
		out <- parslet{k, t}
	}
	close(out)
}

func kind_lex(x rune) rune {
	switch {
	case x == '_' || x == '\n':
		return x
	case unicode.IsLetter(x):
		return '_'
	case unicode.IsDigit(x):
		return '_'
	case unicode.IsSpace(x):
		return ' '
	case unicode.IsPunct(x) || unicode.IsSymbol(x):
		return '!'
	case unicode.IsControl(x):
		return 0
	default:
		return 1
	}
}

func lex(in <-chan rune) <-chan string {
	ch := make(chan string)
	go func() {
		r := <-in
		cur := string(r)
		k := kind_lex(r)
		for r := range in {
			if kind_lex(r) == k {
				cur += string(r)
			} else {
				ch <- cur
				cur = string(r)
				k = kind_lex(r)
			}
		}
		ch <- cur
		close(ch)
	}()
	return ch
}

func make_in_chan(args []string) (<-chan rune, error) {
	in := os.Stdin
	var err error
	if len(args) == 2 && args[1] != "-" {
		in, err = os.Open(args[1])
		if err != nil {
			return nil, err
		}
	}
	ch := make(chan rune)
	go func() {
		buf := bufio.NewReader(in)
		for {
			b, _, err := buf.ReadRune()
			if err != nil {
				if err != io.EOF {
					fmt.Fprintln(os.Stderr, "make_in_chan", err)
				}
				break
			}
			ch <- b
		}
		close(ch)
	}()
	return ch, nil
}

func print_chan(in <-chan string) {
	for tok := range in {
		fmt.Print(tok)
	}
}

func color_parslets(in <-chan parslet) <-chan string {
	ch := make(chan string)
	go func() {
		plain := color[PLAIN]
		for p := range in {
			c, ok := color[p.what]
			if !ok {
				c = plain
			}
			ch <- c + p.text + plain
		}
		close(ch)
	}()
	return ch
}

func main() {
	in, err := make_in_chan(os.Args)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	print_chan(color_parslets(parse(lex(in))))
}
