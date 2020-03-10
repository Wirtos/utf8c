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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "utf8c.h"


char *utf8_next(const char *begin, const char *end) {
    if (begin == end) {
        return NULL;
    }

    if ((*begin & 0x80) == 0x0) {
        begin += 1;
    } else if ((*begin & 0xE0) == 0xC0) {
        begin += 2;
    } else if ((*begin & 0xF0) == 0xE0) {
        begin += 3;
    } else if ((*begin & 0xF8) == 0xF0) {
        begin += 4;
    }

    return (char *) begin;
}

char *utf8_prior(const char *begin, const char *end) {

    if (begin == end) {
        return NULL;
    }

    do {
        begin--;
    } while ((*begin & 0xC0) == 0x80);

    return (char *) begin;
}

char *utf8_advance(const char *begin, size_t n, const char *end) {
    size_t i;
    char *(*iter_function)(const char *, const char *);

    iter_function = (begin < end)
                    ? utf8_next
                    : utf8_prior;

    for (i = 0; i < n; i++) {
        begin = iter_function(begin, end);
        if (begin == NULL) {
            return NULL;
        }
    }
    return (char *) begin;
}


char *utf8_repeat(const char *str, size_t n) {
    size_t len, i;
    char *new_str, *start;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str);
    new_str = (char *) malloc(sizeof(char) * (len * n + 1));
    if (new_str == NULL) {
        return NULL;
    }

    start = new_str;

    for (i = 0; i < n; i++) {
        memcpy(new_str, str, len);
        new_str += len;
    }
    *new_str = '\0';
    return start;
}

size_t utf8_distance(const char *begin, const char *end) {
    size_t dist;
    char *(*iter_function)(const char *, const char *);

    iter_function = (begin < end)
                    ? utf8_next
                    : utf8_prior;
    dist = 0;

    while ((begin = iter_function(begin, end))) {
        dist++;
    }

    return dist;
}

char *utf8_reverse(char str[]) {
    size_t len;
    char tmp, *src, *dst, *start, *end, *it;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str);

    if (len <= 1) { /* Can't do much to 1 character string, return it unchanged*/
        return str;
    }

    start = &str[0];
    end = &str[len];
    it = start;

    /* Iterate through the string and reverse the order of utf-8 grapheme's
     * multibyte octets, so we can restore them in the next step*/
    while ((it = utf8_next(it, end))) {
        if (it - str > 1) {
            src = str;
            dst = it - 1; /* Point to the last octet of current grapheme instead of first octet of next grapheme*/
            while (src < dst) {
                tmp = *src;
                *src++ = *dst;
                *dst-- = tmp;
            }
        }
        str = it;
    }

    /* Iterate through whole string octet-by-octet and reverse it.
     * This will reverse early reversed utf-8 graphemes back to the valid state*/
    src = start;
    dst = end - 1; /* Skip null character*/
    while (src < dst) {
        tmp = *src;
        *src++ = *dst;
        *dst-- = tmp;
    }
    return start;
}

char *utf8_strcpy(const char *str) {
    size_t len;
    char *new_str;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str);
    new_str = (char *) malloc(sizeof(char) * (len + 1));
    if (new_str != NULL) {
        memcpy(new_str, str, len + 1);
    }
    return new_str;
}

char *utf8_substr(const char *str, size_t pos, size_t count) {
    size_t len, dist, new_len;
    const char *start, *end, *it_start, *it_end;
    char *new_str;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str);
    start = &str[0];
    end = &str[len];
    dist = utf8_distance(start, end);

    if (dist < pos) {
        return NULL;
    }


    it_start = utf8_advance(start, pos, end);
    it_end = (count == (size_t) -1)
             ? end
             : utf8_advance(it_start, count, end);

    if (it_end == NULL) {
        it_end = end;
    }

    new_len = it_end - it_start;

    new_str = malloc(sizeof(char) * (new_len + 1));
    if (new_str == NULL) {
        return NULL;
    }

    memcpy(new_str, it_start, it_end - it_start);
    new_str[new_len] = '\0';
    return new_str;
}


char *utf8_join(const char *str, const char *joiner) {
    char *new_str, *new_start;
    const char *start, *end, *it;
    size_t len, dist, joiner_len, n_octets;

    if (str == NULL || joiner == NULL) {
        return NULL;
    }

    len = strlen(str);
    if (len < 2) { /* Can't do much to palindrome-ish string, return unchanged copy*/
        return utf8_strcpy(str);
    }

    joiner_len = strlen(joiner);
    if (joiner_len == 0) { /* Can't do much with empty joiner, return unchanged copy*/
        return utf8_strcpy(str);
    }

    start = &str[0];
    end = &str[len];

    dist = utf8_distance(start, end);

    new_str = (char *) malloc(sizeof(char) * (len + joiner_len * (dist - 1) + 1));
    if (new_str == NULL) {
        return NULL;
    }
    new_start = new_str;
    it = start;
    while ((it = utf8_next(it, end))) {
        n_octets = it - str;
        memcpy(new_str, str, n_octets);
        new_str += n_octets;
        if (it == end) break;
        memcpy(new_str, joiner, joiner_len);
        new_str += joiner_len;
        str = it;
    }

    *new_str = '\0';
    return new_start;
}

char *utf8_strcat(const char *a, const char *b) {
    size_t len_a, len_b;
    char *new_str;

    if (a == NULL || b == NULL) {
        return NULL;
    }

    len_a = strlen(a);
    len_b = strlen(b);

    new_str = (char *) malloc(sizeof(char) * (len_a + len_b + 1));
    if (new_str == NULL) {
        return NULL;
    }

    memcpy(new_str, a, len_a);
    memcpy(new_str + len_a, b, len_b);
    new_str[len_a + len_b] = '\0';
    return new_str;
}

char *utf8_mvstrcat(char *a, char *b) {
    size_t len_a, len_b;
    char *new_str;

    if (a == NULL || b == NULL) {
        return NULL;
    }

    len_a = strlen(a);
    len_b = strlen(b);

    new_str = (char *) realloc(a, sizeof(char) * (len_a + len_b + 1));
    if (new_str == NULL) {
        free(a);
        free(b);
    } else {
        memcpy(new_str + len_a, b, len_b);
        new_str[len_a + len_b] = '\0';
        free(b);
    }

    return new_str;
}

char *utf8_vstrcat(size_t n_str, ...) {
    size_t len, i, index, temp_len;
    char *str, *new_str;
    va_list args;

    if (n_str < 1) {
        return NULL;
    }

    va_start(args, n_str);
    for (i = 0, len = 0; i < n_str; i++) {
        str = va_arg(args, char *);
        if (str == NULL) {
            return NULL;
        }
        len += strlen(str);
    }
    va_end(args);

    new_str = (char *) malloc(sizeof(char) * (len + 1));
    if (new_str == NULL) {
        return NULL;
    }

    va_start(args, n_str);
    for (i = 0, index = 0; i < n_str; i++) {
        str = va_arg(args, char *);
        temp_len = strlen(str);
        memcpy(&new_str[index], str, temp_len);
        index += temp_len;
    }
    va_end(args);

    new_str[len] = '\0';
    return new_str;
}

char *utf8_vmvstrcat(size_t n_strings, ...) {
    size_t len, index, i, temp_len;
    char *str, *new_str;
    va_list args;

    if (n_strings < 1) {
        return NULL;
    }

    va_start(args, n_strings);
    for (i = 0, len = 0; i < n_strings; i++) {
        str = va_arg(args, char *);
        if (str == NULL) {
            va_end(args);
            goto fail;
        }
        len += strlen(str);
    }
    va_end(args);

    va_start(args, n_strings);
    str = va_arg(args, char *);
    temp_len = strlen(str);
    new_str = (char *) realloc(str, sizeof(char) * (len + 1));
    if (new_str == NULL) {
        va_end(args);
        goto fail;
    }
    for (i = 1, index = temp_len; i < n_strings; i++) {
        str = va_arg(args, char *);
        temp_len = strlen(str);
        memcpy(&new_str[index], str, temp_len);
        index += temp_len;
        free(str);
    }
    va_end(args);
    new_str[len] = '\0';
    return new_str;

    fail: /* Just free strings as they are invalid*/
    va_start(args, n_strings);
    for (i = 0; i < n_strings; i++) {
        str = va_arg(args, char *);
        free(str);
    }
    va_end(args);
    return NULL;

}

char **utf8_to_arr(const char *str) {
    size_t dist, len, i;
    const char *start, *end, *next;
    char **arr, *grapheme;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str);
    start = &str[0];
    end = &str[len];

    dist = utf8_distance(start, end);

    arr = malloc(sizeof(char *) * (dist + 1));

    if (arr == NULL) {
        return NULL;
    }

    for (i = 0; i < dist; i++) {
        next = utf8_next(str, end);

        grapheme = malloc(sizeof(char) * (next - str + 1));
        if (grapheme == NULL) {
            goto fail;
        }
        memcpy(grapheme, str, next - str);
        grapheme[next - str] = '\0';
        arr[i] = grapheme;
        str = next;
    }
    arr[dist] = NULL;
    return arr;

    fail:
    while (i-- > 0) {
        free(arr[i]);
    }
    free(arr);
    return NULL;
}

void utf8_arr_free(char **arr) {
    char **start;

    if (arr == NULL) {
        return;
    }
    start = arr;
    while (*arr != NULL) {
        free(*arr++);
    }
    free(start);
}

char *utf8_to_str(const char **arr) {
    size_t len, index, temp_len;
    const char **start;
    char *new_str;

    if (arr == NULL) {
        return NULL;
    }

    start = arr;

    len = 0;
    while (*arr) {
        len += strlen(*arr);
    }
    arr = start;

    new_str = malloc(sizeof(char) * (len + 1));
    if (new_str == NULL) {
        return NULL;
    }

    index = 0;
    while (*arr) {
        temp_len = strlen(new_str);
        memcpy(&new_str[index], *arr, temp_len);
        index += temp_len;
        arr++;
    }
    new_str[len] = '\0';
    return new_str;
}

