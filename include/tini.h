#ifndef TINI_H
#define TINI_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdio.h>

#define TINI_STRLEN SIZE_MAX

enum tini_type
{
	TINI_NONE,

	// node types
	TINI_SECTION,
	TINI_KEY,
	TINI_VALUE,

	// value types
	TINI_STRING,
	TINI_BOOL,
	TINI_SIGNED,
	TINI_UNSIGNED,
	TINI_NUMBER
};

enum tini_result
{
	TINI_SUCCESS,
	TINI_SYNTAX,
	TINI_NODE_COUNT,
	TINI_LINE_COUNT,
	TINI_STRING_SIZE,
	TINI_BOOL_FORMAT,
	TINI_INTEGER_FORMAT,
	TINI_INTEGER_TOO_SMALL,
	TINI_INTEGER_TOO_BIG,
	TINI_INTEGER_NEGATIVE,
	TINI_NUMBER_FORMAT,
	TINI_INVALID_TYPE,
	TINI_UNUSED_SECTION,
	TINI_UNUSED_FIELD,
	TINI_MISSING_SECTION,
	TINI_MISSING_FIELD,
};

struct tini
{
	enum tini_type type:8;
	bool used;
	uint16_t linepos;
	uint16_t column;
	uint16_t length;
	struct tini *next;
};

struct tini_error
{
	const struct tini *node;
	enum tini_result code;
};

struct tini_ctx
{
	struct tini *const nodes;
	uint32_t *const lines;
	uint32_t const nnodes;
	uint32_t const nlines;
	uint32_t nused[2];
	struct tini *section;
	uint16_t linestart;
	uint16_t linepos;
	uint32_t offset;
	const char *txt;
	size_t txtlen;
	struct tini_error err[8];
	unsigned nerr;
	int cs;
};

#define tini_ctx_make(_nnodes, _nlines) { \
	.nodes = ((struct tini [(_nnodes)]){}), \
	.lines = ((uint32_t [(_nlines)]){}), \
	.nnodes = (_nnodes), \
	.nlines = (_nlines), \
	.nused = { 0, 0 }, \
	.section = NULL, \
	.linestart = 0, \
	.linepos = 0, \
	.offset = 0, \
	.txt = NULL, \
	.txtlen = 0, \
	.nerr = 0, \
	.cs = -1, \
}

#define tini_offset(ctx, node) __extension__ ({ \
	const struct tini *__node = (node); \
	tini_bol((ctx), __node) + __node->column; \
})
#define tini_length(ctx, node) ((node)->length)
#define tini_line(ctx, node) ((ctx)->linestart + (node)->linepos)
#define tini_column(ctx, node) ((node)->column)
#define tini_bol(ctx, node) ((ctx)->lines[(node)->linepos])

extern enum tini_result
tini_parse(struct tini_ctx *ctx,
		const char *txt, size_t txtlen,
		int flags);

extern void
tini_reset(struct tini_ctx *ctx);

extern struct tini *
tini_section(const struct tini_ctx *ctx,
		const char *name, size_t namelen);

extern struct tini *
tini_next_section(const struct tini_ctx *ctx,
		const struct tini *section);

extern struct tini *
tini_key(const struct tini_ctx *ctx, const struct tini *section,
		const char *name, size_t namelen);

extern struct tini *
tini_value(const struct tini *node);

extern bool
tini_eq(const struct tini_ctx *ctx, const struct tini *node,
		const char *val, size_t len);

#define tini_streq(ctx, node, str) __extension__ ({ \
	const char *__str = (str); \
	tini_eq(ctx, node, __str, strlen(__str)); \
})

extern char *
tini_str(const struct tini_ctx *ctx, const struct tini *node,
		char *dst, size_t len);

extern enum tini_result
tini_int(const struct tini_ctx *ctx, const struct tini *value,
		uint8_t base, int64_t *out);

extern enum tini_result
tini_bool(const struct tini_ctx *ctx, const struct tini *value,
		bool *out);

extern enum tini_result
tini_double(const struct tini_ctx *ctx, const struct tini *value,
		double *out);

extern enum tini_result
tini_validate(struct tini_ctx *ctx);

extern void
tini_print_errors(struct tini_ctx *ctx,
		const char *path, FILE *out);

extern void __attribute__ ((format (printf, 5, 6)))
tini_errorf(const struct tini_ctx *ctx, const struct tini *node,
		const char *path, FILE *out,
		const char *fmt, ...);

extern void
tini_print(const struct tini_ctx *ctx, FILE *out);

extern const char *
tini_msg(enum tini_result rc);

#define tini_type(v) _Generic((v), \
		char *: TINI_STRING, \
		bool: TINI_BOOL, \
		int8_t: TINI_SIGNED, \
		int16_t: TINI_SIGNED, \
		int32_t: TINI_SIGNED, \
		int64_t: TINI_SIGNED, \
		uint8_t: TINI_UNSIGNED, \
		uint16_t: TINI_UNSIGNED, \
		uint32_t: TINI_UNSIGNED, \
		uint64_t: TINI_UNSIGNED, \
		float: TINI_NUMBER, \
		double: TINI_NUMBER)

#define tini_set(ctx, value, dst) \
	tini__set((ctx), (value), tini_type(dst), \
			_Generic((dst), char *: (dst), default: &(dst)), sizeof(dst))

#define tini_setf(ctx, section, name, dst) __extension__ ({ \
	struct tini_ctx *__ctx = (ctx); \
	const char *__nm = (name); \
	tini_set(__ctx, \
			tini_value(tini_key(__ctx, (section), __nm, strlen(__nm))), \
			dst); \
})

extern enum tini_result
tini__set(struct tini_ctx *ctx, const struct tini *value,
		enum tini_type type, void *out, size_t len);

#endif

