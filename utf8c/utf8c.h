/*
MIT License

Copyright (c) 2020 Wirtos_new

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef UTF8C_H
#define UTF8C_H 1.0f

#include "stddef.h"

/* `begin`  - pointer to a string
 * `end`    - pointer to a string which we will not go beyond. Can also be a pointer to \0
 * return   - pointer to a first octet of a next grapheme after `begin` or NULL if `begin` is the `end`*/
char *utf8_next(const char *begin, const char *end);

/* `begin`  - pointer to any octet of a valid grapheme
 * `end`    - pointer to a string which we will not go beyond.
 * return   - pointer to a first octet of a prior grapheme before `begin` or NULL if `begin` is the `end`*/
char *utf8_prior(const char *begin, const char *end);

/* `begin`  - pointer to a string
 * `end`    - pointer to a string which we will not go beyond.
 * `n`      - number of code points to advance `begin`
 * return   - pointer to a first octet of grapheme advanced for `n` or NULL if `n` is too big*/
char *utf8_advance(const char *begin, size_t n, const char *end);


/* `str`    - pointer to a string, must be \0 terminated
 * `n`      - number of times to repeat `str`
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of `str` repeated for `n`
 *     times or NULL if allocation fails. MUST BE FREED MANUALLY*/
char *utf8_repeat(const char *str, size_t n);


/* `begin`  - pointer to a string
 * `end`    - pointer to a string which we will not go beyond. Can also be a pointer to \0
 * return   - a number of graphemes between `start` and `end`*/
size_t utf8_distance(const char *begin, const char *end);

/* `str`    - pointer to a string, must be \0 terminated AND NOT READ-ONLY
 *     *str = "literal" read-only const declaration, use str[] = "literal" or DYNAMICALLY allocated string instead
 *     or your executable might crash,
 * return   - pointer to a first octet of reversed string*/
char *utf8_reverse(char str[]);

/* `str`    - pointer to a string, must be \0 terminated
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of `str`. MUST BE FREED MANUALLY*/
char *utf8_strcpy(const char *str);

/* `str`    - pointer to a string, must be \0 terminated
 * `off`    - number of offset code points to advance starting from `str` beginning
 * `count`  - number of code points to include starting from `off`
 * return   - pointer to the beginning of DYNAMICALLY allocated substring of `str`
 *     or NULL if allocation fails. MUST BE FREED MANUALLY*/
char *utf8_substr(const char *str, size_t off, size_t count);

/* `str`    - pointer to a string, must be \0 terminated
 * `joiner` - pointer to a valid first octet of a string to be joined, must be \0 terminated`
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of `str` joined with `joiner`
 *     or NULL if allocation fails. MUST BE FREED MANUALLY*/
char *utf8_join(const char *str, const char *joiner);

/* `a`      - pointer to a string, must be \0 terminated
 * `b`      - pointer to a string, must be \0 terminated
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of `a` and `b` concatenated together
 *     or NULL if allocation fails. MUST BE FREED MANUALLY*/
char *utf8_strcat(const char *a, const char *b);

/* `a`      - pointer to a string, must be \0 terminated AND DYNAMICALLY ALLOCATED
 * `b`      - pointer to a string, must be \0 terminated AND DYNAMICALLY ALLOCATED
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of `a` and `b` concatenated together
 *     or NULL if allocation fails. MUST BE FREED MANUALLY. `a` and `b` are now invalid pointers as
 *     they're moved to the new string*/
char *utf8_mvstrcat(char *a, char *b);

/* `n_str`  - number of input strings
 * `... `   - pointers to valid strings
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of strings concatenated together
 *     or NULL if allocation fails. MUST BE FREED MANUALLY*/
char *utf8_vstrcat(size_t n_str, ...);

/* `n_str`  - number of input strings
 * `... `   - pointers to valid strings, strings must be \0 terminated AND DYNAMICALLY ALLOCATED
 * return   - pointer to the beginning of DYNAMICALLY allocated copy of strings concatenated together
 *     or NULL if allocation fails. MUST BE FREED MANUALLY. Strings passed as `...` are now invalid pointers as
 *     they're moved to the new string*/
char *utf8_vmvstrcat(size_t n_strings, ...);

/* `str`    - pointer to a string, must be \0 terminated
 * return   - pointer to the beginning of DYNAMICALLY allocated array of strings extracted from `str`.
 *     Every string is a valid standalone utf8 grapheme(visible "character") to be printed.
 *       Must be exactly utf8_distance(`str`) + 1 length and each element must be \0 terminated string
 *       + last element will be NULL.
 *       in <- "аabcㅊ"
 *      out -> [
 *           "а\0" (two-byte("\xd0\xb0") Cyrillic "а"),
 *           "a\0",
 *           "b\0",
 *           "c\0",
 *           "ㅊ\0" (three-byte("\xe3\x85\x8a") Hangul "ㅊ"),
 *           NULL
 *          ]
 *     or NULL pointer if allocation fails. MUST BE FREED MANUALLY with utf8_array_free */
char **utf8_to_arr(const char *str);

/* `arr`    - pointer to a valid array allocated with utf8_to_array. MUST CORRESPOND to each utf8_to_array usage*/
void utf8_arr_free(char **arr);

/* `arr`    - pointer to a valid array allocated with utf8_to_array.
 * return   - pointer to the beginning of DYNAMICALLY allocated string created from `arr`
 *     or NULL if allocation fails. MUST BE FREED MANUALLY*/
char *utf8_to_str(const char **arr);

#endif /* UTF8C_H*/
