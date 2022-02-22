# LEXT

[![code style: compliant](https://img.shields.io/badge/code%20style-compliant-000000.svg)](https://github.com/jhauberg/comply)

LEXT, or Lexical Templates, is a [format specification](#format-specification) and [zero-dependency library](#usage) for generating pseudo-random sequences of text through recursive substitution.

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

**Disclaimer:**

Both library and specification has been built and established as an exercise more so than an attempt at making an actually useful thing.

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
    char const * const format = "word (World, Hello) hello <@word @word>";
    char buffer[64];

    uint32_t seed = 1234;

    lxt_gen(buffer, sizeof(buffer), format, (struct lxt_opts) {
        // select any random generator; in this case always `hello`
        .generator = LXT_OPTS_GENERATOR_ANY,
        .seed = &seed
    });
    
    printf("%s\n", buffer);
    
    return 0;
}
```

*Note that given the same options (seed and generator), LEXT will generate identical results on any platform.*

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

The LEXT format is simple and consist of only two basic concepts; [containers](#containers) and [generators](#generators).

### Containers

A container holds all the pieces of text that can be *sequenced* by a *generator*.

A LEXT can have any number of containers.

The following example defines a container named `letter` that holds the strings `a`, `b`, `c` and `d` (each string delimited by a single comma `,`):

```
letter (a, b, c, d)
```

### Generators

A generator defines the *format* and *sequence* of a generated output.

A LEXT can have any number of generators.

In this example, a generator `scramble` is defined. This generator then defines a sequence of text including 3 variables (indicated by a word starting with `@`), each pointing to a *container* named `letter`:

```
scramble <@letter, @letter, @letter>
```

*Note that commas in sequences are _not_ delimiters; they are part of the text.*

When this generator is invoked and a result is to be sequenced, each variable will be replaced by a randomly picked item from the `letter` container. For instance, a result could be `c, a, b`, `a, b, c` or even `a, a, a`.

#### Sequencing

A generator can also sequence a variable that points to another generator. This makes sequencing fully recursive, allowing for more complex patterns.

For example, here's a generator with a sequence that refers to the `scramble` generator:

```
example <a few letters: @scramble>
```

When invoked, the `example` generator will sequence the initial text `a few letters: ` before finally resolving and sequencing the `scramble` generator. This results in an output like `a few letters: b, c, a`.

## License

LEXT is a Free Open-Source Software project released under the [MIT License](LICENSE).
