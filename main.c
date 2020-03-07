#include <stdio.h>
#include <string.h>
#include <utf8c.h>
#include <stdlib.h>

/* Read about definitions first https://stackoverflow.com/a/27331885/9997709*/
int main() {

    /* strlen - amount of octets NOT "characters"*/
    char
            str[] = "лаба",
            *str_ptr = str,
            *start = &str[0],
            *end = &str[strlen(str)],
            *sub,
            *joined,
            *repeated,
            *it,
            *vcat,
            *vmvcat,
            **arr,
            **arr_start;

    printf("number of 'characters': %zu, octets: %zu\n",
           utf8_distance(start, end),
           strlen(str)
    );

    printf("advance from start: %s\n", utf8_advance(start, 2, end));
    printf("advance from end: %s\n", utf8_advance(end, 3, start));

    sub = utf8_substr(str, 1, 2);
    printf("substr: %s\n", sub);
    free(sub);


    vcat = utf8_vstrcat(2, "012", " 3456 789");
    puts(vcat);
    free(vcat);


    vmvcat = utf8_vmvstrcat(3, utf8_strcpy("012"), utf8_strcpy(" 3456 "), utf8_join("789", "-"));
    puts(vmvcat);
    free(vmvcat);

    repeated = utf8_repeat(str, 3);
    printf("repeated: %s\n", repeated);
    free(repeated);

    utf8_reverse(str);
    printf("reversed: %s\n", str);

    joined = utf8_join(str, "_+_");
    printf("joined: %s\n", joined);
    free(joined);
    utf8_reverse(str);

    putchar('\n');

    arr = utf8_to_arr(str);
    arr_start = arr;
    while (*arr != NULL) {
        puts(*arr++);
    }
    utf8_arr_free(arr_start);
    putchar('\n');


    it = str; /* `str` is currently pointing to the start of our string*/

    /* `it` will be NULL at the end of the operation*/
    while ((it = utf8_next(it, end))) {
        /* Real visible "character"(aka grapheme) can be represented using 1/4 bytes(byte is called `char` in c).
         * We're calling these code units "octets". To print one grapheme we need to print all its octets.*/
        for (char *octet = str_ptr; octet != it; octet++) { /* Go from start pointer(`str`) to end pointer(`it`)*/
            putchar(*octet);
        }
        putchar('\n');
        str_ptr = it;
    }

    puts("|");
    it = str_ptr; /* `str` is currently pointing to the end of our string - '\0'*/

    /* `it` will be NULL at the end of the operation*/
    while ((it = utf8_prior(it, start))) {
        /* Print octets starting from the beginning of prior "character"
         * until we hit first octet pointer of next "character". Same way as utf8_next, but backwards.*/
        for (char *octet = it; octet != str_ptr; octet++) {
            putchar(*octet);
        }
        putchar('\n');
        str_ptr = it; /* Set `str` to be previous result of utf8_prior*/
    }
    return EXIT_SUCCESS;
}
