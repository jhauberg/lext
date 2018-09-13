#include <lext/lext.h> // lxt_*

#include <stdio.h> // printf

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
