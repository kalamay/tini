#include "../include/tini.h"

#include <inttypes.h>
#include <stdlib.h>

void
tini_reset(struct tini_ctx *ctx)
{
	ctx->nused[0] = 0;
	ctx->nused[1] = 0;
	ctx->section = NULL;
	ctx->linestart = 0;
	ctx->linepos = 0;
	ctx->offset = 0;
	ctx->cs = -1;
}

struct tini *
tini_section(const struct tini_ctx *ctx,
		const char *name, size_t namelen)
{
	struct tini *ne = ctx->nodes + ctx->nnodes, *n = ne - ctx->nused[1];
	for (--ne; ne >= n && ne->type == TINI_SECTION; --ne) {
		if (tini_eq(ctx, ne, name, namelen)) {
			ne->used = true;
			return ne;
		}
	}
	return NULL;
}

struct tini *
tini_next_section(const struct tini_ctx *ctx,
		const struct tini *section)
{
	struct tini *ne = ctx->nodes + ctx->nnodes;
	if (section == NULL) { section = ne; }
	section--;
	if (section >= ne || section < ne - ctx->nused[1]) { return NULL; }
	return (struct tini *)section;
}

struct tini *
tini_key(const struct tini_ctx *ctx, const struct tini *section,
		const char *name, size_t namelen)
{
	if (section) {
		struct tini *n = section->next;
		for (; n; n = n->next) {
			if (n->type == TINI_KEY) {
				if (tini_eq(ctx, n, name, namelen)) {
					n->used = true;
					return n;
				}
			}
			else if (n->type != TINI_VALUE) {
				break;
			}
		}
	}
	return NULL;
}

struct tini *
tini_value(const struct tini *node)
{
	if (node && (node->type == TINI_KEY || node->type == TINI_VALUE)) {
		struct tini *rc = node->next;
		if (rc && rc->type == TINI_VALUE) {
			return rc;
		}
	}
	return NULL;
}

bool
tini_eq(const struct tini_ctx *ctx, const struct tini *node,
		const char *val, size_t len)
{
	return node && tini_length(ctx, node) == len &&
		memcmp(ctx->txt + tini_offset(ctx, node), val, len) == 0;
}

char *
tini_str(const struct tini_ctx *ctx, const struct tini *node,
		char *dst, size_t len)
{
	if (node && tini_length(ctx, node) < len) {
		memcpy(dst, ctx->txt + tini_offset(ctx, node), tini_length(ctx, node));
		dst[tini_length(ctx, node)] = '\0';
		return dst;
	}
	return NULL;
}

enum tini_result
tini_bool(const struct tini_ctx *ctx, const struct tini *value,
		bool *out)
{
	if (value) {
		const char *s = ctx->txt + tini_offset(ctx, value);

		switch (tini_length(ctx, value)) {
		case 1:
			switch (*s) {
			case 't': case 'T': case 'y': case 'Y': case '1': *out = true; return 0;
			case 'f': case 'F': case 'n': case 'N': case '0': *out = false; return 0;
			}
			break;
		case 2:
			if (!strncasecmp(s, "on", 2)) { *out = true; return 0; }
			if (!strncasecmp(s, "no", 2)) { *out = false; return 0; }
			break;
		case 3:
			if (!strncasecmp(s, "yes", 3)) { *out = true; return 0; }
			if (!strncasecmp(s, "off", 3)) { *out = false; return 0; }
			break;
		case 4:
			if (!strncasecmp(s, "true", 4)) { *out = true; return 0; }
			break;
		case 5:
			if (!strncasecmp(s, "false", 5)) { *out = false; return 0; }
			break;
		}
	}
	return TINI_BOOL_FORMAT;
}

enum tini_result
tini_int(const struct tini_ctx *ctx, const struct tini *value,
		uint8_t base, int64_t *out)
{
	if (value) {
		const char *s = ctx->txt + tini_offset(ctx, value);
		char *end;
		*out = strtoll(s, &end, base);
		if (end == s + tini_length(ctx, value)) {
			return TINI_SUCCESS;
		}
	}
	return TINI_INTEGER_FORMAT;
}

enum tini_result
tini_double(const struct tini_ctx *ctx, const struct tini *value,
		double *out)
{
	if (value) {
		const char *s = ctx->txt + tini_offset(ctx, value);
		char *end;
		*out = strtod(s, &end);
		if (end == s + tini_length(ctx, value)) {
			return TINI_SUCCESS;
		}
	}
	return TINI_NUMBER_FORMAT;
}

