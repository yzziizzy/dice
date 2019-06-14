#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "dicefile.h"
#define NAME_MAX 1000


static int str_trim_eq(const char *a, const char *b);
static int dicefile_read_expression(FILE *f, const char *read_buf, size_t read_buf_len, char *out_buf, size_t buf_len);


// Scans a dicefile for a line with the given name.
// Fills out_buff with the dice expression.
// Returns 1 on success, 0 on failure.
int dicefile_find_line(const char *filename, const char *line_name, char *out_buf, size_t buf_len) {
    if (strlen(line_name) >= NAME_MAX) {
        fprintf(stderr, "Provided name is too long (%d is maximum length).\n", NAME_MAX);
        return 0;
    }
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open dicefile \"%s\".\n", filename);
        return 0;
    }
    // If find_name is true, we're looking for a colon to terminate the name. 
    // Otherwise, we're looking for a newline to terminate the string.
    char find_name = 1;
    
    // If started_line is false, it means we have yet to find a non-whitespace character.
    char started_line = 0;
    
    #define B_MAX (NAME_MAX * 2)
    char b[B_MAX];
    char *start = b;
    int end = 0;
    int i = 0;
    
    while (1) {
        int status = fread(b + i, 1, B_MAX - i, f);
        if (status == 0) {
            break;
        }
        
        end += status;
        while (i < end) {
            if (!started_line) {
                if (b[i] == '#') {
                    find_name = 0;
                }
                if (!isspace(b[i])) {
                    started_line = 1;
                }
            }
            if (find_name) {
                if (b[i] == ':') {
                    find_name = 0;
                    b[i] = '\0';
                    if (str_trim_eq(line_name, start)) {
                        // We've found the line we're looking for. Read everything in to out_buf.
                        return dicefile_read_expression(f, b + i, end - i, out_buf, buf_len);
                    }
                }
            }
            if (b[i] == '\n') {
                find_name = 1;
                start = b + i + 1;
                started_line = 0;
            }
            ++i;
        }
        
        if (find_name) {
            // Move the start of the name back to the start of the buffer
            end -= (start - b);
            i -= (start - b);
            memmove(b, start, end);
        }
        else {
            // Just ignore what's in the buffer
            end = i = 0;
        }
    }
    
    fprintf(stderr, "Could not find entry for %s\n", line_name);
    return 0;
}

// For internal use by dicefile_find_line
// read_buf is what's left in the read buffer to copy in to out_buff. Starts on a null char.
// read_buf_len is the number of valid chars in read_buf
// out_buf is the output buffer
// buf_len is the length of the out_buf
static int dicefile_read_expression(FILE *f, const char *read_buf, size_t read_buf_len, char *out_buf, size_t buf_len) {
    char *cursor = out_buf;
    // All the +1 and -1 is because read_buf starts on a null that needs to be skipped.
    if (read_buf_len > buf_len + 1) {
        read_buf_len = buf_len + 1;
    }
    if (read_buf_len > 0) {
        memcpy(cursor, read_buf + 1, read_buf_len - 1);
        cursor += read_buf_len - 1;
        buf_len -= read_buf_len - 1;
    }
    while (buf_len > 0) {
        int status = fread(cursor, 1, buf_len, f);
        if (status == 0) {
            break;
        }
        cursor += status;
        buf_len -= status;
    }
    char *end = cursor;
    cursor = out_buf;
    while (cursor != end) {
        if (*cursor == '\n') {
            *cursor = '\0';
            return 1;
        }
        ++cursor;
    }
    
    fprintf(stderr, "Could not fit dice expression inside of output buffer (expression too large)\n");
    return 0;
}

// Returns whether two strings are equal after trimming whitespace off of the end of both strings, and compressing all consecutive whitespace into a single space character
static int str_trim_eq(const char *a, const char *b) {
    // Skip leading whitespace for both a and b
    while (*a && isspace(*a)){
        ++a;
    }
    while (*b && isspace(*b)){
        ++b;
    }
    while (*a && *b) {
        if (isspace(*a)) {
            // Skip all whitespace
            while (*a && isspace(*a)) {
                ++a;
            }
            if (*a) {
                // We still have more of a to compare
                
                // Make sure *b is whitespace
                if (!*b || !isspace(*b)) {
                    return 0;
                }
                // Skip all the remaining whitespace in b
                while (*b && isspace(*b)) {
                    ++b;
                }
            } 
            else {
                // We've hit the end of a. Make sure we hit the end of b as well with no more characters.
                while (*b && isspace(*b)) {
                    ++b;
                }
                if (*b) {
                    // We still have more of b left.
                    return 0;
                }
                return 1;
            }
        }
        else {
            if (*a != *b) {
                return 0;
            }
            ++a;
            ++b;
        }
    }
    // Skip any trailing whitespace for both a and b
    while (*a && isspace(*a)){
        ++a;
    }
    while (*b && isspace(*b)){
        ++b;
    }
    return *a == *b;
}
