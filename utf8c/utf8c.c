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
#include <errno.h>
#include "utf8c.h"

const size_t utf8_npos = (size_t) -1;

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
        if (begin == NULL) { /* Can't advance string as n is bigger than number of code points available*/
            return NULL;
        }
    }
    return (char *) begin;
}


char *utf8_repeat(const char *str, size_t n) {
    size_t len, i;
    char *new_str, *begin;

    if (str == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len = strlen(str);
    new_str = (char *) malloc(sizeof(char) * (len * n + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    begin = new_str;
    for (i = 0; i < n; i++) {
        memcpy(new_str, str, sizeof(char) * len);
        new_str += len;
    }
    *new_str = '\0';
    return begin;
}

size_t utf8_distance(const char *begin, const char *end) {
    size_t dist;
    char *(*iter_function)(const char *, const char *);

    dist = 0;

    iter_function = (begin < end)
                    ? utf8_next
                    : utf8_prior;

    while ((begin = iter_function(begin, end))) {
        dist++;
    }

    return dist;
}

char *utf8_reverse(char str[]) {
    size_t len;
    char tmp, *src, *dst, *begin, *end, *it;

    if (str == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len = strlen(str);

    if (len <= 1) { /* Can't do much to 1 character string, leave it unchanged*/
        return str;
    }

    begin = &str[0];
    end = &str[len];
    it = begin;

    /* Iterate through the string and reverse the order of utf-8 grapheme's
     * multibyte octets, so we can restore them in the next iteration*/
    while ((it = utf8_next(it, end))) {
        if (it - str > 1) { /* Only proceed if code point is multi-byte*/
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
    src = begin;
    dst = end - 1; /* Skip null character*/
    while (src < dst) {
        tmp = *src;
        *src++ = *dst;
        *dst-- = tmp;
    }
    return begin;
}

char *utf8_strcpy(const char *str) {
    size_t len;
    char *new_str;

    if (str == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len = strlen(str);
    new_str = (char *) malloc(sizeof(char) * (len + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }
    memcpy(new_str, str, sizeof(char) * (len + 1));
    return new_str;
}

char *utf8_substr(const char *str, size_t off, size_t count) {
    size_t len, new_len;
    const char *begin, *end, *it_begin, *it_end;
    char *new_str;

    if (str == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len = strlen(str);
    begin = &str[0];
    end = &str[len];

    it_begin = utf8_advance(begin, off, end);

    if (it_begin == NULL) { /* Can't advance string's `off`set, it goes beyond last string code point*/
        errno = UTF8_ERANGE;
        return NULL;
    }

    it_end = (count == utf8_npos)
             ? end
             : utf8_advance(it_begin, count, end);

    if (it_end == NULL) {
        it_end = end; /* Count goes beyond last string code point, use `end` as last valid code point position*/
    }

    new_len = it_end - it_begin; /* Number of octets(bytes) between begin position and end position*/

    new_str = malloc(sizeof(char) * (new_len + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    memcpy(new_str, it_begin, sizeof(char) * new_len);
    new_str[new_len] = '\0';
    return new_str;
}


char *utf8_join(const char *str, const char *joiner) {
    char *new_str, *new_begin;
    const char *begin, *end, *it;
    size_t len, dist, joiner_len, n_octets;

    if (str == NULL || joiner == NULL) {
        errno = UTF8_EINVAL;
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

    begin = &str[0];
    end = &str[len];

    dist = utf8_distance(begin, end);

    new_str = (char *) malloc(sizeof(char) * (joiner_len * (dist - 1) + len + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    new_begin = new_str;
    it = begin;
    while ((it = utf8_next(it, end))) {
        n_octets = it - str;
        memcpy(new_str, str, sizeof(char) * n_octets);
        new_str += n_octets;
        if (it == end) break;
        memcpy(new_str, joiner, sizeof(char) * joiner_len);
        new_str += joiner_len;
        str = it;
    }

    *new_str = '\0';
    return new_begin;
}

char *utf8_strcat(const char *a, const char *b) {
    size_t len_a, len_b;
    char *new_str;

    if (a == NULL || b == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len_a = strlen(a);
    len_b = strlen(b);

    new_str = (char *) malloc(sizeof(char) * (len_a + len_b + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    memcpy(new_str, a, sizeof(char) * len_a);
    memcpy(new_str + len_a, b, sizeof(char) * len_b);
    new_str[len_a + len_b] = '\0';
    return new_str;
}

char *utf8_mvstrcat(char *a, char *b) {
    size_t len_a, len_b;
    char *new_str;

    if (a == NULL || b == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len_a = strlen(a);
    len_b = strlen(b);
    new_str = (char *) realloc(a, sizeof(char) * (len_a + len_b + 1));
    if (new_str == NULL) {
        free(a);
        free(b);
        errno = UTF8_ENOMEM;
        return NULL;
    }

    memcpy(new_str + len_a, b, sizeof(char) * len_b);
    new_str[len_a + len_b] = '\0';
    free(b);
    return new_str;
}

char *utf8_vstrcat(size_t n_str, ...) {
    size_t len, i, temp_len;
    char *str, *new_str, *new_begin;
    va_list args;

    if (n_str < 1) {
        return NULL;
    }

    va_start(args, n_str);
    for (i = 0, len = 0; i < n_str; i++) {
        str = va_arg(args, char *);
        if (str == NULL) {
            errno = UTF8_EINVAL;
            return NULL;
        }
        len += strlen(str);
    }
    va_end(args);

    new_str = (char *) malloc(sizeof(char) * (len + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    new_begin = new_str;
    va_start(args, n_str);
    for (i = 0; i < n_str; i++) {
        str = va_arg(args, char *);
        temp_len = strlen(str);
        memcpy(new_str, str, sizeof(char) * temp_len);
        new_str += temp_len;
    }
    va_end(args);

    *new_str = '\0';
    return new_begin;
}

char *utf8_vmvstrcat(size_t n_strings, ...) {
    size_t len, i, temp_len;
    char *str, *new_str, *new_begin;
    va_list args;

    if (n_strings < 1) {
        return NULL;
    }

    va_start(args, n_strings);
    for (i = 0, len = 0; i < n_strings; i++) {
        str = va_arg(args, char *);
        if (str == NULL) {
            va_end(args);
            errno = UTF8_EINVAL;
            goto fail;
        }
        len += strlen(str);
    }
    va_end(args);

    va_start(args, n_strings);
    str = va_arg(args, char *);
    new_str = (char *) realloc(str, sizeof(char) * (len + 1));
    if (new_str == NULL) {
        va_end(args);
        errno = UTF8_ENOMEM;
        goto fail;
    }

    new_begin = new_str;
    new_str += strlen(new_str);

    for (i = 1; i < n_strings; i++) {
        str = va_arg(args, char *);
        temp_len = strlen(str);
        memcpy(new_str, str, sizeof(char) * temp_len);
        new_str += temp_len;
        free(str);
    }
    va_end(args);
    *new_str = '\0';
    return new_begin;

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
    const char *begin, *end, *next;
    char **arr, *grapheme;

    if (str == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    len = strlen(str);
    begin = &str[0];
    end = &str[len];

    dist = utf8_distance(begin, end);

    arr = malloc(sizeof(char *) * (dist + 1));

    if (arr == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    for (i = 0; i < dist; i++) {
        next = utf8_next(str, end);
        grapheme = malloc(sizeof(char) * (next - str + 1));
        if (grapheme == NULL) {
            errno = UTF8_ENOMEM;
            goto fail;
        }
        memcpy(grapheme, str, sizeof(char) * (next - str));
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
    char **arr_begin;

    if (arr == NULL) {
        errno = UTF8_EINVAL;
        return;
    }
    arr_begin = arr;
    while (*arr != NULL) {
        free(*arr++);
    }
    free(arr_begin);
}

char *utf8_to_str(char *const *arr) {
    size_t len, temp_len;
    char *const *arr_begin;
    char *new_str, *begin;

    if (arr == NULL) {
        errno = UTF8_EINVAL;
        return NULL;
    }

    arr_begin = arr;

    len = 0;
    while (*arr) {
        len += strlen(*arr++);
    }

    new_str = malloc(sizeof(char) * (len + 1));
    if (new_str == NULL) {
        errno = UTF8_ENOMEM;
        return NULL;
    }

    arr = arr_begin;
    begin = new_str;

    while (*arr) {
        temp_len = strlen(*arr);
        memcpy(new_str, *arr++, sizeof(char) * temp_len);
        new_str += temp_len;
    }
    *new_str = '\0';
    return begin;
}

