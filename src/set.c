#include "../include/tini.h"

static enum tini_result
set_string(struct tini_ctx *ctx, const struct tini *value,
		char *out, size_t len)
{
	uint32_t vlen = tini_length(ctx, value);
	if (vlen < len) {
		memcpy(out, ctx->txt + tini_offset(ctx, value), vlen);
		out[vlen] = '\0';
		return TINI_SUCCESS;
	}
	else {
		return TINI_STRING_SIZE;
	}
}

#define SETS(rc, out, type, val, min, max) do { \
	if (val < min) { rc = TINI_INTEGER_TOO_SMALL; } \
	else if (val > max) { rc = TINI_INTEGER_TOO_BIG; } \
	else { *(type *)out = (type)val; } \
} while (0)

static enum tini_result
set_signed(struct tini_ctx *ctx, const struct tini *value,
		void *out, size_t len)
{
	int64_t val;
	enum tini_result rc = tini_int(ctx, value, 0, &val);
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
set_unsigned(struct tini_ctx *ctx, const struct tini *value,
		void *out, size_t len)
{
	int64_t val;
	enum tini_result rc = tini_int(ctx, value, 0, &val);
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
set_number(struct tini_ctx *ctx, const struct tini *value,
		void *out, size_t len)
{
	double val;
	enum tini_result rc = tini_double(ctx, value, &val);
	if (rc == TINI_SUCCESS) {
		switch (len) {
		case sizeof(float): *(float *)out = (float)val; break;
		case sizeof(double): *(double *)out = val; break;
		}
	}
	return rc; }

enum tini_result
tini__set(struct tini_ctx *ctx, const struct tini *value,
		enum tini_type type, void *out, size_t len)
{
	if (value == NULL) {
		return TINI_MISSING_FIELD;
	}

	enum tini_result rc;
	switch (type) {
	case TINI_STRING:   rc = set_string(ctx, value, out, len);   break;
	case TINI_BOOL:     rc = tini_bool(ctx, value, out);         break;
	case TINI_SIGNED:   rc = set_signed(ctx, value, out, len);   break;
	case TINI_UNSIGNED: rc = set_unsigned(ctx, value, out, len); break;
	case TINI_NUMBER:   rc = set_number(ctx, value, out, len);   break;
	default:            rc = TINI_INVALID_TYPE;                  break;
	}

	if (rc != TINI_SUCCESS) {
		if (ctx->nerr < (sizeof(ctx->err)/sizeof(ctx->err[0]))) {
			ctx->err[ctx->nerr] = (struct tini_error) { value, rc };
		}
		ctx->nerr++;
	}

	return rc;
}

