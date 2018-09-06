#include <lext/lext.h> // lxt_*

#include <stdio.h> // printf, fopen, fread, fclose, FILE, SEEK_END, SEEK_SET
#include <stdint.h> // int32_t
#include <stddef.h> // size_t
#include <stdlib.h> // malloc, free

int32_t
main(int32_t argc, char const * const argv[])
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
    
    // generate a result
    char result[64];
    
    lxt_gen(result, sizeof(result), string);
    
    printf("%s\n", result);
    
    // clean up
    fclose(file);
    free(string);
    
    return 0;
}
