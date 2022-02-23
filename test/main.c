#include <lext/lext.h> // lxt_*

#include <assert.h> // assert
#include <string.h> // strcmp

static
void
test_truncation(void)
{
    enum lxt_error error;
    char buffer[8];
    
    error = lxt_gen(buffer, sizeof(buffer),
                    "a (truncated_entry) seq <@a>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    // note only 7 characters; last byte reserved for null-terminator
    assert(strcmp(buffer, "truncat") == 0);
    
    error = lxt_gen(buffer, sizeof(buffer),
                    "a (entry) seq <@a + @a>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry +") == 0);
}

static
void
test_invalid_template(void)
{
    enum lxt_error error;
    char buffer[64];

    // bad container name
    error = lxt_gen(buffer, sizeof(buffer),
                    "conta iner (entry) sequence <@container>",
                    LXT_OPTS_NONE);

    assert(error == LXT_ERROR_INVALID_TEMPLATE);

    // bad generator name
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequ ence <@container>",
                    LXT_OPTS_NONE);

    assert(error == LXT_ERROR_INVALID_TEMPLATE);
}

static
void
test_escapes(void)
{
    enum lxt_error error;
    char buffer[64];

    // literal backslash
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence <\\ @container>",
                    LXT_OPTS_NONE);

    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "\\ entry") == 0);

    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence <\\\\@container>",
                    LXT_OPTS_NONE);

    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "\\\\entry") == 0);

    // LEXT does not support escaping reserved characters as literals;
    // in this case, 2 variables are parsed, though only one resolves
    // (i.e. each '@' is effectively one variable)
    error = lxt_gen(buffer, sizeof(buffer),
                "container (entry) sequence <\\@@container>",
                LXT_OPTS_NONE);

    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "\\entry") == 0);

    // similarly, entry delimiters can not be escaped
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (a\\,b\\,a) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "b\\") == 0);
}

static
void
test_newlines_and_whitespace(void)
{
    enum lxt_error error;
    char buffer[64];
    
    // should not append blank entry to container
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) empty (   ) sequence <@empty>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "") == 0);
    
    // should trim entries
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (      a, a  ,   a  ) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "a") == 0);

    // handle newlines
    error = lxt_gen(buffer, sizeof(buffer),
                "container (\nentry,\nentry,\nentry\n) sequence <@container>",
                LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);

    // ...and whitespace
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (\n    entry,\n    entry,\n    entry\n) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);

    error = lxt_gen(buffer, sizeof(buffer),
                    "container (\n    entry,\n    entry with whitespace,\n    entry\n) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry with whitespace") == 0);
}

static
void
test_quotes(void)
{
    enum lxt_error error;
    char buffer[64];

    // quotes are part of container entries
    error = lxt_gen(buffer, sizeof(buffer),
                "container (a, \"b\", \"c\") sequence <@container>",
                LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "\"b\"") == 0);

    // ...and do not need to be balanced
    error = lxt_gen(buffer, sizeof(buffer),
            "container (a, \"b, \"c\") sequence <@container>",
            LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "\"b") == 0);

    // quotes are part of sequence text too
    error = lxt_gen(buffer, sizeof(buffer),
                "container (a, b, c) sequence <\"text @container\">",
                LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "\"text b\"") == 0);
}

static
void
test_various(void)
{
    enum lxt_error error;
    char buffer[64];
    
    // should parse template
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);
    
    // should parse container/entries without separating whitespace
    error = lxt_gen(buffer, sizeof(buffer),
                    "container(entry) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);
    
    // should parse generator/sequence without separating whitespace
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence<@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);
    
    // should parse variables without separating whitespace
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence <@container@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entryentry") == 0);
    
    // should parse template even without any whitespace
    error = lxt_gen(buffer, sizeof(buffer),
                    "container(entry)sequence<@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);
    
    // should allow multiple identical entries
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry, entry, entry) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry") == 0);
    
    // should blank out empty container variables
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) empty () sequence <@empty>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "") == 0);
    
    // should blank out 0-length variables
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence <@container @>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "entry ") == 0);

    // generator delimiters in entries
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (ab<c>de(fg) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "ab<c>de(fg") == 0);
    
    // container delimiters in sequence
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (a) sequence <@container, 1<2, (@container)>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "a, 1<2, (a)") == 0);
    
    // should skip variables leading to infinite recursion
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) sequence <@sequence a>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, " a") == 0);

    error = lxt_gen(buffer, sizeof(buffer),
                    "container (entry) <@container>",
                    LXT_OPTS_NONE);

    assert(error == LXT_ERROR_GENERATOR_NOT_FOUND);

    error = lxt_gen(buffer, sizeof(buffer),
                    "container (a) sequence <@@@@@container@ @>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "a ") == 0);

    error = lxt_gen(buffer, sizeof(buffer),
                    "container (b,,,b,b,) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "b") == 0);
}

int32_t
main(void)
{
    test_various();
    test_quotes();
    test_invalid_template();
    test_escapes();
    test_newlines_and_whitespace();
    test_truncation();
    
    return 0;
}
