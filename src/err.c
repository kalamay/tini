#include "../include/tini.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#define LOC "\x1b[1m"
#define ERR "\x1b[1;31m"
#define RNG "\x1b[1;32m"
#define RST "\x1b[0m"

#define ERR_MAX (sizeof(((struct tini_ctx *)0)->err)/sizeof(((struct tini_ctx *)0)->err[0]))

const char *
tini_msg(enum tini_result rc)
{
	switch (rc) {
	case TINI_SUCCESS:           return "ok";
	case TINI_SYNTAX:            return "invalid syntax";
	case TINI_STRING_TOO_BIG:    return "string value too large";
	case TINI_BOOL_FORMAT:       return "invalid boolean format";
	case TINI_INTEGER_FORMAT:    return "invalid integer format";
	case TINI_INTEGER_TOO_SMALL: return "integer too small";
	case TINI_INTEGER_TOO_BIG:   return "integer too large";
	case TINI_INTEGER_NEGATIVE:  return "integer must be unsigned";
	case TINI_NUMBER_FORMAT:     return "invalid number format";
	case TINI_INVALID_TYPE:      return "invalid type";
	case TINI_UNUSED_SECTION:    return "unsupported section";
	case TINI_UNUSED_KEY:        return "unsupported key";
	case TINI_MISSING_SECTION:   return "section not allowed";
	case TINI_MISSING_KEY:       return "key not allowed";
	}
	return "unknown error";
}

void
tini_add_error(struct tini_ctx *ctx, const struct tini *node,
		const char *msg,
		enum tini_result code)
{
	if (ctx->nerr < sizeof(ctx->err)/sizeof(ctx->err[0])) {
		ctx->err[ctx->nerr] = (struct tini_error){
			.node = *node,
			.msg = msg,
			.code = code,
		};
	}
	ctx->nerr++;
}

static const char *
msg(const struct tini_error *err)
{
	return err->msg ? err->msg : tini_msg(err->code);
}

void
tini_print_errors(const struct tini_ctx *ctx, const char *path, FILE *out)
{
	unsigned nerr = ctx->nerr;
	if (nerr == 0) { return; }
	if (nerr > ERR_MAX) { nerr = ERR_MAX; }

	for (unsigned i = 0; i < nerr; i++) {
		tini_errorf(ctx, &ctx->err[i].node, path, out, "%s", msg(&ctx->err[i]));
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
	int ln = node->line;
	int col = node->column;
	const char *p = node->line_start;
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

	pe += node->length;
	if (tty) { fwrite(RNG, 1, sizeof(RNG) - 1, out); }
	fputc('^', out);
	for (p++; p<pe; p++) { fputc('~', out); }
	if (tty) { fwrite(RST, 1, sizeof(RST) - 1, out); }
	fputc('\n', out);
}

