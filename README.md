# LEXT

[![code style: compliant](https://img.shields.io/badge/code%20style-compliant-000000.svg)](https://github.com/jhauberg/comply)

LEXT, or Lexical Templates, is a [format](#format-specification) and zero-dependency library for recursively generating pseudo-random sequences of text that strictly adheres to a specific pattern.

```lxt
# lxt for magical weaponry

type (Axe, Sword)
element (Earth, Wind, Water, Fire)
prefix (Frozen, Fiery)

common <@type of @element>
magic <@prefix @common>
```

```console
Fiery Sword of Water
Axe of Earth
```

## Usage

Integrating and using the LEXT library is relatively straightforward.

You can build it as a static library that you link into your program, or just drop its few sources directly into your existing project. See [Building](#building) for instructions on building the library from source.

### Example

Here's a [small program](/example/hello.c) that generates and prints `Hello World`:

```c
#include <lext/lext.h>

int32_t
main(void)
{
    char const * const format = "word (World, Hello) sequence <@word @word>";
    char buffer[64];

    uint32_t seed = 12345;

    lxt_gen(buffer, sizeof(buffer), format, (struct lxt_opts) {
        .generator = NULL,
        .seed = &seed
    });
    
    printf("%s\n", buffer);
    
    return 0;
}
```

*Note that given the same seed, LEXT will generate an identical result on any platform.*

Take a look in [examples](/example) for more samples of usage.

### CLI

The project provides [a basic CLI](/cli) for using LXT-patterns from a terminal. You just have to build it.

## Building

LEXT can be easily built as a static library by using the included [CMake](https://cmake.org) scripts.

To generate build files for your system, make your way to the root of the LEXT repository and run CMake:

```console
$ cd lext
$ cmake .
```

## Format Specification

The LEXT format is simple and consist of only two basic concepts; **containers** and **generators**.

It's essentially nothing more than plain old recursive substitution, but formalized. There are no complicated algorithms or procedures involved.

### Containers

A container holds all the pieces of text that can be *sequenced* by a *generator*.

The following example defines a container named `letter` that holds the strings/characters `a`, `b`, `c` and `d`:

```
letter (a, b, c, d)
```

### Generators

A generator defines the *format* and *sequence* of a generated output.

In this example, a generator `scramble` is defined. This generator then defines a sequence of 3 variables (indicated by a word starting with `@`), each pointing to a *container* named `letter`:

```
scramble <@letter, @letter, @letter>
```

When this generator is invoked and a result is to be sequenced, each variable will be replaced by a randomly picked item from the `letter` container. For instance, one result could be `c, a, b`, but another could just as well be `a, b, c`.

#### Sequences

A sequence can hold a variable that points to another generator. This makes sequencing fully recursive, allowing for more complex patterns.

For example, expanding on the previous `scramble` generator:

```
example <a few letters: @scramble>
```

When invoked, the `example` generator will sequence the initial text `a few letters:` before finally resolving and sequencing the `scramble` generator. This results in an output like `a few letters: b, c, a`.

## License

LEXT is a Free Open-Source Software project released under the [MIT License](LICENSE).
