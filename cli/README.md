# LEXT CLI

This application provides a basic command-line interface for interacting with the LEXT library.

## Usage

```console
LEXT is Lexical Templates

Usage:
  lext <amount> -f <file>
  lext <amount> -p <pattern>
  lext -v | --version
  lext -h | --help
```

### Examples

Generate 5 results from a given pattern.

```console
$ lext 5 -p "letter (a, b, c) seq <@letter, @letter, @letter>"
```

Generate 5 results from a pattern in a file.

```console
$ lext 5 -f "simple.lxt"
```
