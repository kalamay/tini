#include "../include/tini.h"

static bool
streq(const char *str, const void *mem, size_t memlen)
{
	size_t n = strlen(str);
	return n == memlen && memcmp(str, mem, n) == 0;
}

const struct tini_field *
tini_field_find(const struct tini_section *s,
		const char *name, size_t namelen)
{
	const struct tini_field *p = s->fields, *pe = p + s->nfields;
	for (; p < pe; p++) {
		if (streq(p->name, name, namelen)) {
			return p;
		}
	}
	return NULL;
}

enum tini_result
tini_assign(const struct tini_section *section,
		const struct tini *key,
		const struct tini *value,
		void *udata)
{
	(void)udata;

	const struct tini_field *f = tini_field_find(section, key->start, key->length);
	if (f == NULL) {
		return TINI_MISSING_KEY;
	}
	return tini_set_field(section->target, f, value);
}

#define SETS(rc, out, type, val, min, max) do { \
	if (val < min) { rc = TINI_INTEGER_TOO_SMALL; } \
	else if (val > max) { rc = TINI_INTEGER_TOO_BIG; } \
	else { *(type *)out = (type)val; } \
} while (0)

static enum tini_result
set_signed(void *out, size_t len, const struct tini *value)
{
	int64_t val;
	enum tini_result rc = tini_int(&val, 0, value);
	if (rc == TINI_SUCCESS) {
		switch (len) {
		case sizeof(int8_t):  SETS(rc, out, int8_t, val, INT8_MIN, INT8_MAX); break;
		case sizeof(int16_t): SETS(rc, out, int16_t, val, INT16_MIN, INT16_MAX); break;
		case sizeof(int32_t): SETS(rc, out, int32_t, val, INT32_MIN, INT32_MAX); break;
		case sizeof(int64_t): *(int64_t *)out = (int64_t)val; break;
		default: rc = TINI_INVALID_TYPE; break;
		}
	}
	return rc;
}

#define SETU(rc, out, type, val, max) do { \
	if (val < 0) { rc = TINI_INTEGER_NEGATIVE; } \
	else if (val > max) { rc = TINI_INTEGER_TOO_BIG; } \
	else { *(type *)out = (type)val; } \
} while (0)

static enum tini_result
set_unsigned(void *out, size_t len, const struct tini *value)
{
	int64_t val;
	enum tini_result rc = tini_int(&val, 0, value);
	if (rc == TINI_SUCCESS) {
		switch (len) {
		case sizeof(uint8_t):  SETU(rc, out, uint8_t, val, UINT8_MAX); break;
		case sizeof(uint16_t): SETU(rc, out, uint16_t, val, UINT16_MAX); break;
		case sizeof(uint32_t): SETU(rc, out, uint32_t, val, UINT32_MAX); break;
		case sizeof(uint64_t):
			if (val < 0) { rc = TINI_INTEGER_NEGATIVE; }
			else { *(uint64_t *)out = val; }
			break;
		default: rc = TINI_INVALID_TYPE; break;
		}
	}
	return rc;
}

static enum tini_result
set_number(void *out, size_t len, const struct tini *value)
{
	double val;
	enum tini_result rc = tini_double(&val, value);
	if (rc == TINI_SUCCESS) {
		switch (len) {
		case sizeof(float): *(float *)out = (float)val; break;
		case sizeof(double): *(double *)out = val; break;
		}
	}
	return rc;
}

enum tini_result
tini_set(void *t, size_t size, enum tini_type type,
		const struct tini *value)
{
	switch (type) {
	case TINI_STRING:   return tini_str(t, size, value);
	case TINI_BOOL:     return tini_bool(t, value);
	case TINI_SIGNED:   return set_signed(t, size, value);
	case TINI_UNSIGNED: return set_unsigned(t, size, value);
	case TINI_NUMBER:   return set_number(t, size, value);
	case TINI_NODE:     return (memcpy(t, value, sizeof(*value)), 0);
	default:            return TINI_INVALID_TYPE;
	}
}

enum tini_result
tini_set_field(void *target,
		const struct tini_field *field,
		const struct tini *value)
{
	if (field == NULL) {
		return TINI_UNUSED_KEY;
	}
	void *t = (char *)target + field->offset;
	return tini_set(t, field->size, field->type, value);
}

