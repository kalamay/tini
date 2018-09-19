#include "../include/tini.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#define LOC "\x1b[1m"
#define ERR "\x1b[1;31m"
#define RNG "\x1b[1;32m"
#define RST "\x1b[0m"

#define ERR_MAX (sizeof(((struct tini_ctx *)0)->err)/sizeof(((struct tini_ctx *)0)->err[0]))

bool
tini_validate(struct tini_ctx *ctx)
{
	struct tini *ne = ctx->nodes + ctx->nnodes, *n = ne - ctx->nused[1];
	for (--ne; ne >= n && ne->type == TINI_SECTION; --ne) {
		if (!ne->used) {
			if (ctx->nerr < ERR_MAX) {
				ctx->err[ctx->nerr] = (struct tini_error) { ne, TINI_UNUSED_SECTION };
			}
			ctx->nerr++;
		}

		struct tini *f = ne->next;
		for (; f; f = f->next) {
			if (f->type == TINI_KEY && !f->used) {
				if (ctx->nerr < ERR_MAX) {
					ctx->err[ctx->nerr] = (struct tini_error) { f, TINI_UNUSED_FIELD };
				}
				ctx->nerr++;
			}
		}
	}

	return ctx->nerr == 0;
}

static int
cmperr(const void *a, const void *b)
{
	const struct tini *na = a;
	const struct tini *nb = b;
	if (na->linepos < nb->linepos) { return -1; }
	if (na->linepos > nb->linepos) { return 1; }
	if (na->column < nb->column) { return -1; }
	if (na->column > nb->column) { return 1; }
	return 0;
}

static const char *
msg(enum tini_result rc)
{
	switch (rc) {
	case TINI_SUCCESS:           return "ok";
	case TINI_SYNTAX:            return "invalid syntax";
	case TINI_NODE_COUNT:        return "node buffer full";
	case TINI_LINE_COUNT:        return "line buffer full";
	case TINI_STRING_SIZE:       return "string value too large";
	case TINI_BOOL_FORMAT:       return "invalid boolean format";
	case TINI_INTEGER_FORMAT:    return "invalid integer format";
	case TINI_INTEGER_TOO_SMALL: return "integer too small";
	case TINI_INTEGER_TOO_BIG:   return "integer too large";
	case TINI_INTEGER_NEGATIVE:  return "integer must be unsigned";
	case TINI_NUMBER_FORMAT:     return "invalid number format";
	case TINI_INVALID_TYPE:      return "invalid type";
	case TINI_UNUSED_SECTION:    return "unsupported section";
	case TINI_UNUSED_FIELD:      return "unsupported field";
	}
}

void
tini_print_errors(struct tini_ctx *ctx, const char *path, FILE *out)
{
	unsigned nerr = ctx->nerr;
	if (nerr > ERR_MAX) { nerr = ERR_MAX; }

	qsort(ctx->err, nerr, sizeof(ctx->err[0]), cmperr);

	for (unsigned i = 0; i < nerr; i++) {
		tini_errorf(ctx, ctx->err[i].node, path, out, "%s", msg(ctx->err[i].code));
	}

	if (nerr < ctx->nerr) {
		fprintf(out, "showing %u of %u errors\n", nerr, ctx->nerr);
	}
	else if (nerr == 1) {
		fprintf(out, "showing 1 error\n");
	}
	else {
		fprintf(out, "showing %u errors\n", nerr);
	}
}

void
tini_errorf(const struct tini_ctx *ctx, const struct tini *node,
		const char *path, FILE *out, const char *fmt, ...)
{
	bool tty = isatty(fileno(out));
	int ln = tini_line(ctx, node);
	int col = tini_column(ctx, node);
	const char *p = ctx->txt + tini_bol(ctx, node);
	const char *pe = p + col;
	const char *eol = memchr(p, '\n', ctx->txtlen - (p - ctx->txt));
	if (eol == NULL) { eol = ctx->txt + ctx->txtlen; }

	if (tty) {
		fprintf(out, LOC "%s:%d:%d: " RST ERR "error: " RST , path, ln+1, col+1);
	}
	else {
		fprintf(out, "%s:%d:%d: error: ", path, ln+1, col+1);
	}

	va_list ap;
	va_start(ap, fmt);
	vfprintf(out, fmt, ap);
	va_end(ap);

	fprintf(out, "\n    %.*s\n    ", (int)(eol - p), p);

	for (; p < pe; p++) {
		if (*p == '\t') { fwrite("        ", 1, 8, out); }
		else { fputc(' ', out); }
	}

	pe += tini_length(ctx, node);
	if (tty) { fwrite(RNG, 1, sizeof(RNG)-1, out); }
	for (; p < pe; p++) { fputc('^', out); }
	if (tty) { fwrite(RST, 1, sizeof(RST)-1, out); }
	fputc('\n', out);
}

void
tini_print(const struct tini_ctx *ctx, FILE *out)
{
	struct tini *ne = ctx->nodes + ctx->nnodes, *n = ne - ctx->nused[1];
	for (--ne; ne >= n && ne->type == TINI_SECTION; --ne) {
		int slen = tini_length(ctx, ne), fcount = 0;
		if (slen > 0) {
			fprintf(out, "[%.*s]\n", slen, ctx->txt + tini_offset(ctx, ne));
		}
		struct tini *f = ne->next;
		for (; f; f = f->next) {
			int flen = tini_length(ctx, f);
			if (f->type == TINI_KEY) {
				fprintf(out, "%.*s = ", flen, ctx->txt + tini_offset(ctx, f));
				fcount++;
			}
			else if (f->type == TINI_VALUE) {
				fprintf(out, "%.*s\n", flen, ctx->txt + tini_offset(ctx, f));
			}
		}
		if (slen > 0 || fcount > 0) {
			fputc('\n', out);
		}
	}
}

