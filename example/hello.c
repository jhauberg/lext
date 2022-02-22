#include <lext/lext.h> // lxt_*

#include <stdio.h> // printf

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
