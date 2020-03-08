# utf8c
Efficient, pure C90 compliant library to manipulate utf-8 encoded strings.

## Methods

### char *utf8_next(const char *begin, const char *end);
Given the iterator to the beginning of the UTF-8 sequence, it returns the pointer to the beginning of the next sequence.
Returns NULL if it is the end.
```c
char *str_ptr = "test_лдж", *it, *end, *start;

start = &str_ptr[0];
end = &str_ptr[strlen(str_ptr)]; /* Pointer to the last element of the string*/

it = str_ptr;
while ((it = utf8_next(it, end))) {
        for (char *octet = str_ptr; octet != it; octet++) {
            /* Go from start pointer(`str_ptr`) to the start of new sequence it and print each sequence element*/
            putchar(*octet); 
        }
        putchar('\n');
        str_ptr = it; /* Set `str` to be current result of utf8_next*/
}

str_ptr = start;  /* Reset str_ptr position to initial state*/
```

### char *utf8_prior(const char *begin, const char *end);
Given a reference to an iterator pointing to an octet in a UTF-8 sequence, it decreases the iterator until it hits the beginning of the previous UTF-8 encoded code point and returns pointer to it. 
Returns NULL if it is the end.
```c
char *it;
it = end;
while ((it = utf8_prior(it, start))) {
        /* Print octets starting from the beginning of prior "character"
         * until we hit first octet pointer of sequence we started from. Same way as utf8_next, but backwards.*/
        for (char *octet = it; octet != str_ptr; octet++) {
            putchar(*octet);
        }
        putchar('\n');
        str_ptr = it; /* Set `str` to be current result of utf8_prior*/
}
```

### char *utf8_advance(const char *begin, size_t n, const char *end);
Advances an iterator by the specified number of code points within an UTF-8 sequence.
Can be used backwards.
```c
char *str_ptr = "test_лдж", *it, *end, *start;
start = &str_ptr[0];
end = &str_ptr[strlen(str_ptr)]; /* Pointer to the last element of the string*/

printf("advance from start: %s\n", utf8_advance(start, 2, end)); /* -> "st_лдж"*/
printf("advance from end: %s\n", utf8_advance(end, 3, start));   /* -> "лдж"*/
```

### char *utf8_repeat(const char *str, size_t n);
Returns allocated copy of str allocated for n times or empty string if n is 0.<br/>
Fallthrough: if `str` is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *res = utf8_repeat("test", 3); /* -> "testtesttest"*/
puts(res);
free(res); /* We don't need it anymore*/
```

### size_t utf8_distance(const char *begin, const char *end);
Given the iterators to two UTF-8 encoded code points in a sequence, returns the number of code points between them.
Can be used backwards.
```c
size_t x, y, z;
char *str1 = "test", *str2 = "тест", *start1, *end1, *start2, *end2;
start1 = &str1[0];
start2 = &str2[0];
end1 = &str1[strlen(str1)];
end2 = &str2[strlen(str2)];

printf("%zu, %zu\n", strlen(str1), strlen(str2)); /* -> 4, 8*/
printf("%zu, %zu\n", utf8_distance(start1, end1), utf8_distance(start2, end2)); /* -> 4, 4*/

printf("%zu", utf8_distance(end1, start1)); /* -> 4*/
```

### char *utf8_reverse(char str[]);
Reverses string in place. `str` **must** be mutable (read example below).<br/>
Fallthrough: if `str` is NULL, returns NULL.
```c
char str[] = "тест";
utf8_reverse(str);
puts(str); /* -> тсет*/


/* !BUT! */
char *str_read_only = "test";
/* This code will crash(SEGFAULT) at runtime, char *str = "string" declaration is read-only, 
* and function can't edit its elements in order to reverse it.
* HOW TO THEN? - Use char str[] = "string" syntax for writeable strings,
* or wrap string with utf8_strcpy before passing it.*/
utf8_reverse(str_read_only); /* Invalid*/
utf8_reverse("test"); /* Also invalid*/

char *res1 = utf8_reverse(utf8_strcpy(str_read_only)); /* Valid*/
char *res2 = utf8_reverse(utf8_strcpy("test")); /* Also valid*/
free(res1); /* must be freed, because utf8_strcpy allocates new memory*/
free(res2);
```

### char *utf8_strcpy(const char *str);
Creates a dynamically allocated copy of `str`.<br/>
Fallthrough: if `str` is NULL, returns NULL.<br/>
Must be freed manually with `free`.

### char *utf8_substr(const char *str, size_t off, size_t count);
Creates a dynamically allocated substring of string starting from `off`set number of code points 
and includes `count` of code points after it (if the string is shorter, as many characters as possible are used).
If `off`set is bigger than number of code points inside string it returns NULL.
If you want to include all the remaining string starting from `off`set, then pass `-1` as `count`.<br/>
Fallthrough: if `str` is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *str = "тест";
sub = utf8_substr(str, 1, 2);   /* -> "ес"*/
sub2 = utf8_substr(str, 1, -1); /* -> "ест"*/
printf("substr: %s\n", sub);
printf("substr: %s", sub2);
free(sub);
free(sub2);

```

### char *utf8_join(const char *str, const char *joiner);
Creates a dynamically allocated string joined with `joiner`.<br/>
Fallthrough: if `str` or `joiner` is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *res = utf8_join("тест", "-");
puts(res); /* -> "т-е-с-т"*/
free(res);
```

### char *utf8_strcat(const char *a, const char *b);
Concatenates two strings together by creating new dynamically allocated string.<br/>
Fallthrough: if `a` or `b` is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *res = utf8_strcat("test", "тест");
puts(res); /* -> "testтест"*/
free(res);
/*BUT*/
char *str1 = utf8_strcpy("тест"), *str2 = utf8_join("test", "-"); /* Dynamically allocate two strings*/
res = utf8_strcat(str1, str2);
puts(res); /* -> "тестt-e-s-t"*/
free(res);

/* str1 remains unchanged by this function so we can use it and must free it afterwards, same with str2*/
puts(str2); /* -> "t-e-s-t"*/
free(str1); 
free(str2);
```

### char *utf8_mvstrcat(char *a, char *b);
Concatenates two strings together by `m`o`v`ing them to the new string. Input strings **MUST** be dynamically allocated **only**,
SEGFAULT otherwise. `a` and `b` are **no longer valid** and can't be used after passing them to the function,
even if function fails.<br/>
Fallthrough: if `a` or `b` is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *str1 = utf8_strcpy("тест"), *str2 = utf8_join("test", "-"); /* Dynamically allocate two strings*/
char *res = utf8_mvstrcat(str1, str2);
puts(res); /* -> "тестt-e-s-t"*/

free(res);

/* BUT */

/* Notice the fact that we CAN'T use str1 and str2 anymore, as they're moved to the res. No need to free them also. */
puts(str2); /* !!!SEGFAULT, str2 is no more valid!*/

utf8_mvstrcat("test", "test"); /* !!!SEGFAULT, we can't move statically allocated strings!*/
```

### char *utf8_vstrcat(size_t n_str, ...);
Variadically concatenates `n_str` together by creating new dynamically allocated string. Acts the same as `utf8_strcat`.<br/>
Fallthrough: if any string is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *res = utf8_vstrcat(3, "test", " concat ", "тест");
puts(res); /* -> "test concat тест"*/
free(res);

char *str1 = "test", *str2 = utf8_join("test", "-"), *str3 = utf8_strcat(" con", "cat"); /* Dynamically allocate two strings*/
res = utf8_vstrcat(3, str1, str2, str3);
puts(res); /* -> "тестt-e-s-t concat"*/
free(res);

free(str2);
free(str3);
```


### char *utf8_vmvstrcat(size_t n_strings, ...);
Variadically concatenates `n_strings` strings together by `m`o`v`ing them to the new string. Input strings **MUST** be dynamically allocated **only**,
SEGFAULT otherwise.<br/>
Input strings are **no longer valid** and can't be used after passing them to the function, even if function fails.<br/>
Fallthrough: if any string is NULL, returns NULL.<br/>
Must be freed manually with `free`.
```c
char *str1 = utf8_strcpy("тест"), *str2 = utf8_join("test", "-"), *str3 = utf8_strcat(" con", "cat"); /* Dynamically allocate two strings*/
char *res = utf8_mvstrcat(str1, str2);
puts(res); /* -> "тестt-e-s-t concat"*/

free(res);

/* BUT */

/* Notice the fact that we CAN'T use any of input strings anymore, as they're moved to the res. No need to free them also. */
puts(str2); /* !!!SEGFAULT, str2 is no more valid!*/

utf8_vmvstrcat(2, "test", "test"); /* !!!SEGFAULT, we can't move statically allocated strings!*/
```

### char **utf8_to_arr(const char *str);
Creates new NULL-terminated array, where each utf-8 sequence of `str` is a separate string element.<br/>
Fallthrough: if `str` is NULL, returns NULL.<br/>
Must be freed manually with `utf8_arr_free`.
```c
char **arr = utf8_to_arr("аabcㅊ"); /* ->
          "а\0" (two-byte("\xd0\xb0") Cyrillic "а"),
          "a\0",
          "b\0",
          "c\0",
          "ㅊ\0" (three-byte("\xe3\x85\x8a") Hangul "ㅊ"),
          NULL
         ]
*/
utf8_arr_free(arr);
```

### void utf8_arr_free(char **arr);
Deallocates arr.<br/>
Fallthrough: if `arr` is NULL, returns NULL.


### char *utf8_to_str(const char **arr);
Creates new allocated string created from `arr` elements.<br/>
Must be freed manually with `free`.

# Code guidelines
Functions with allocations can return NULL if memory allocation fails, but checking each result will be a pain, right?<br/>
For example utf8_reverse returns the same input string pointer instead of allocating new one, but you can always do:
```c
char str[] = "string";
char *res = utf8_reverse(utf8_strcpy(str));
free(res);
```
Always chain few expressions if you don't need them right away and check for the final result in-place:
```c
char *res = utf8_vmvstrcat(3,
    utf8_mvstrcat(utf8_reverse(utf8_join("test", "--")), utf8_substr("string", 1, 3)),
    utf8_strcpy("test1"),
    utf8_join("test2", ".")
);
/* All the allocations like strcpy, join, substr are freed by respectable parent move functions 
 * like mvstrcar and vmvstrcat, so you only need to check and free final result!*/

if(res != NULL){
    puts(res);
    free(res);
}
```
#### Instead of this HUGE wall of code that does literally the same job:
```c
char *joined = utf8_join("test", "--");
if(joined == NULL){
   return;
}
utf8_reverse(joined);

char *cp =  utf8_strcpy("test1");
if(cp == NULL){
   free(joined);
}

char *joined2 = utf8_join("test2", ".");
if (joined2 == NULL){
   free(joined);
   free(cp);
   return;
}
char *sub = utf8_substr("string", 1, 3);
if(sub == NULL){
   free(joined);
   free(cp);
   free(joined2);
   return;  
}
char *cat = utf8_strcat(joined, sub);
if(cat == NULL){
   free(joined);
   free(cp);
   free(joined2);
   free(sub);
}

char *res = vstrcat(3, cat, cp, joined2);
if(res == NULL){
   free(joined);
   free(cp);
   free(joined2);
   free(sub);
   free(cat);
   return;
}

puts(res);

free(joined);
free(cp);
free(joined2);
free(sub);
free(cat);
free(res);
return;

```

#### A lot harder than the first example, isn't it? 
Don't forget to combine `fallthroughs` and `mv`* functions together in order to achieve readable, 
short and easy to debug code.