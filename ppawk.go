package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"unicode"
)

func kind(x rune) rune {
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
		k := kind(r)
		for r := range in {
			if kind(r) == k {
				cur += string(r)
			} else {
				ch <- cur
				cur = string(r)
				k = kind(r)
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

func main() {
	in, err := make_in_chan(os.Args)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	print_chan(lex(in))
}
