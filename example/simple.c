#include <lext/lext.h> // lxt_*

#include <stdio.h> // printf, fopen, fread, fclose, FILE, SEEK_END, SEEK_SET
#include <stdint.h> // int32_t
#include <stddef.h> // size_t
#include <stdlib.h> // malloc, free

int32_t
main(void)
{
    FILE * const file = fopen("simple.lxt", "rb");

    // read in entire file
    fseek(file, 0, SEEK_END);
    
    size_t const length = ftell(file);
    
    fseek(file, 0, SEEK_SET);
    
    char * const string = malloc(length + 1);
    
    fread(string, length, 1, file);
    
    // null-terminate the string
    string[length] = '\0';
    
    uint32_t seed = 2147483647;
    
    for (int32_t i = 0; i < 5; i++) {
        // generate a result
        char result[64];
        
        enum lxt_error error;
        
        error = lxt_gen(result, sizeof(result), string, (struct lxt_opts) {
            .generator = NULL,
            .seed = &seed
        });
        
        if (error == LXT_ERROR_NONE) {
            printf("%s\n", result);
        }
    }
    
    // clean up
    fclose(file);
    free(string);
    
    return 0;
}
