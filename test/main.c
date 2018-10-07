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
    
    // delimiters in entries
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (ab<c>de(fg) sequence <@container>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "ab<c>de(fg") == 0);
    
    // delimiters in sequence
    error = lxt_gen(buffer, sizeof(buffer),
                    "container (a) sequence <@container, 1<2, (@container)>",
                    LXT_OPTS_NONE);
    
    assert(error == LXT_ERROR_NONE);
    assert(strcmp(buffer, "a, 1<2, (a)") == 0);
}

int32_t
main(void)
{
    test_various();
    test_truncation();
    
    return 0;
}
