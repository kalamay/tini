#include "../include/tini.h"

#include <inttypes.h>
#include <stdlib.h>

bool
tini_eq(const struct tini *node, const char *val, size_t len)
{
	return node && node->length == len &&
		memcmp(node->start, val, len) == 0;
}

enum tini_result
tini_str(char *t, size_t len, const struct tini *value)
{
	size_t vlen = value->length;
	if (value && vlen < len) {
		memcpy(t, value->start, vlen);
		t[vlen] = '\0';
		return TINI_SUCCESS;
	}
	return TINI_STRING_TOO_BIG;
}

enum tini_result
tini_bool(bool *t, const struct tini *value)
{
	if (value) {
		const char *s = value->start;
		switch (value->length) {
		case 1:
			switch (*s) {
			case 't': case 'T': case 'y': case 'Y': case '1': *t = true; return 0;
			case 'f': case 'F': case 'n': case 'N': case '0': *t = false; return 0;
			}
			break;
		case 2:
			if (!strncasecmp(s, "on", 2)) { *t = true; return 0; }
			if (!strncasecmp(s, "no", 2)) { *t = false; return 0; }
			break;
		case 3:
			if (!strncasecmp(s, "yes", 3)) { *t = true; return 0; }
			if (!strncasecmp(s, "off", 3)) { *t = false; return 0; }
			break;
		case 4:
			if (!strncasecmp(s, "true", 4)) { *t = true; return 0; }
			break;
		case 5:
			if (!strncasecmp(s, "false", 5)) { *t = false; return 0; }
			break;
		}
	}
	return TINI_BOOL_FORMAT;
}

enum tini_result
tini_int(int64_t *t, uint8_t base, const struct tini *value)
{
	if (value) {
		const char *s = value->start;
		char *end;
		*t = strtoll(s, &end, base);
		if (end == s + value->length) {
			return TINI_SUCCESS;
		}
	}
	return TINI_INTEGER_FORMAT;
}

enum tini_result
tini_double(double *t, const struct tini *value)
{
	if (value) {
		const char *s = value->start;
		char *end;
		*t = strtod(s, &end);
		if (end == s + value->length) {
			return TINI_SUCCESS;
		}
	}
	return TINI_NUMBER_FORMAT;
}

