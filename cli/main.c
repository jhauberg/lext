#include <lext/lext.h> // lxt_gen, lxt_opts, LXT_VERSION_*

#include <stdio.h> // printf, fprintf, fopen, fclose, fread, FILE, SEEK_*
#include <stdlib.h> // malloc, free
#include <stddef.h> // size_t, NULL
#include <stdbool.h> // bool
#include <stdint.h> // int32_t
#include <string.h> // strcmp
#include <time.h> // time

static
void
generate(char const * const pattern, uint32_t amount)
{
    uint32_t seed = (uint32_t)time(NULL);
    
    for (uint32_t i = 0; i < amount; i++) {
        char buffer[256];
        
        lxt_gen(buffer, sizeof(buffer), pattern, (struct lxt_opts) {
            .generator = NULL,
            .seed = &seed
        });
        
        printf("%s\n", buffer);
    }
}

static
int32_t
read_file(char ** const buffer, char const * const filename)
{
    FILE * const file = fopen(filename, "rb");
    
    if (file == NULL) {
        fprintf(stderr, "Could not open file '%s'\n", filename);
        
        return -1;
    }
    
    // move to end of file
    fseek(file, 0, SEEK_END);
    // determine total length
    size_t const length = ftell(file);
    // move to beginning of file
    fseek(file, 0, SEEK_SET);
    // allocate enough memory to buffer entire file
    *buffer = malloc(length + 1);
    // read entire file as one chunk
    fread(*buffer, length, 1, file);
    // null-terminate the buffer for good measure
    buffer[length] = 0;
    
    if (fclose(file) != 0) {
        free(*buffer);
        
        fprintf(stderr, "Could not close file '%s'\n", filename);
        
        return -1;
    }
    
    return 0;
}

int32_t
main(int32_t const argc, char ** const argv)
{
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 ||
            strcmp(argv[1], "-h") == 0) {
            printf("LEXT is Lexical Templates\n\n");
            // note that we don't return immediately; proceed to show usage
        } else if (strcmp(argv[1], "--version") == 0 ||
                   strcmp(argv[1], "-v") == 0) {
            printf("LEXT %d.%d.%d\n",
                   LXT_VERSION_MAJOR,
                   LXT_VERSION_MINOR,
                   LXT_VERSION_PATCH);
            
            return 0;
        }
    }
    
    if (argc != 4) {
        printf("Usage:\n"
               "  lext <amount> -f <file>\n"
               "  lext <amount> -p <pattern>\n"
               "  lext -v | --version\n"
               "  lext -h | --help\n");
        
        return -1;
    }
    
    char * const param_amount = argv[1];
    char * const param_input_type = argv[2];
    char * const param_input = argv[3];
    
    int32_t amount = atoi(param_amount);
    
    if (amount < 0) {
        amount = 0;
    }
    
    char * pattern = NULL;
    
    bool buffer_allocated = false;
    
    if (strcmp(param_input_type, "-p") == 0) {
        // input is a pattern
        pattern = param_input;
    } else if (strcmp(param_input_type, "-f") == 0) {
        // input is a file
        char const * const filename = param_input;
        
        if (read_file(&pattern, filename) != 0) {
            return -1;
        }
        
        buffer_allocated = true;
    }
    
    generate(pattern, (uint32_t)amount);
    
    if (buffer_allocated) {
        free(pattern);
    }
    
    return 0;
}
