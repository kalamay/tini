#ifndef TINI_H
#define TINI_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdio.h>

enum tini_result
{
	TINI_SUCCESS,
	TINI_SYNTAX,
	TINI_STRING_TOO_BIG,
	TINI_BOOL_FORMAT,
	TINI_INTEGER_FORMAT,
	TINI_INTEGER_TOO_SMALL,
	TINI_INTEGER_TOO_BIG,
	TINI_INTEGER_NEGATIVE,
	TINI_NUMBER_FORMAT,
	TINI_INVALID_TYPE,
	TINI_UNUSED_SECTION,
	TINI_UNUSED_KEY,
	TINI_MISSING_SECTION,
	TINI_MISSING_KEY,
};

enum tini_type
{
	TINI_NONE,

	// node types
	TINI_SECTION,
	TINI_LABEL,
	TINI_KEY,
	TINI_VALUE,

	// value types
	TINI_STRING,
	TINI_BOOL,
	TINI_SIGNED,
	TINI_UNSIGNED,
	TINI_NUMBER,
	TINI_NODE,
};

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
		double: TINI_NUMBER, \
		struct tini: TINI_NODE)

struct tini
{
	const char *start;
	uint32_t length;
	enum tini_type type;
	const char *line_start;
	uint32_t line;
	uint32_t column;
};

struct tini_error
{
	const struct tini node;
	const char *msg;
	enum tini_result code;
};

struct tini_field
{
	const char *const name;
	const size_t size;
	const size_t offset;
	const enum tini_type type;
};

#define tini_field_make_as(_struct, _member, _name) \
	((struct tini_field) { \
		.name = _name, \
		.size = sizeof(((_struct *)0)->_member), \
		.offset = offsetof(_struct, _member), \
		.type = tini_type(((_struct *)0)->_member) \
	})

#define tini_field_make(_struct, _member) \
	tini_field_make_as(_struct, _member, #_member)

struct tini_section
{
	const struct tini_field *fields;
	size_t nfields;
	void *target;
	enum tini_result (*assign)(
			const struct tini_section *section,
			const struct tini *key,
			const struct tini *value,
			void *udata);
};

#define tini_section_set(section, _target, _fields) do { \
	struct tini_section *__tmp = (section); \
	__tmp->fields = (_fields); \
	__tmp->nfields = sizeof(_fields) / sizeof((_fields)[0]); \
	__tmp->target = (_target); \
} while (0)

struct tini_ctx
{
	const char *txt;
	size_t txtlen;
	struct tini_error err[10];
	unsigned nerr;
	enum tini_result (*load_section)(
			struct tini_section *section,
			const struct tini *name,
			const struct tini *label,
			void *udata);
	void *udata;
};

#define tini_ctx_make(_load_section, _udata) { \
	.nerr = 0, \
	.load_section = (_load_section), \
	.udata = (_udata), \
}

extern enum tini_result
tini_parse(struct tini_ctx *ctx,
		const char *txt, size_t txtlen,
		int flags);

extern bool
tini_eq(const struct tini *node, const char *val, size_t len);

#define tini_streq(node, str) __extension__ ({ \
	const char *__str = (str); \
	tini_eq(node, __str, strlen(__str)); \
})

extern enum tini_result
tini_str(char *target, size_t len, const struct tini *value);

extern enum tini_result
tini_int(int64_t *target, uint8_t base, const struct tini *value);

extern enum tini_result
tini_bool(bool *target, const struct tini *value);

extern enum tini_result
tini_double(double *target, const struct tini *value);

extern void
tini_add_error(struct tini_ctx *ctx, const struct tini *node,
		const char *msg,
		enum tini_result code);

extern void
tini_print_errors(const struct tini_ctx *ctx,
		const char *path, FILE *out);

extern void __attribute__ ((format (printf, 5, 6)))
tini_errorf(const struct tini_ctx *ctx, const struct tini *node,
		const char *path, FILE *out,
		const char *fmt, ...);

extern const char *
tini_msg(enum tini_result rc);

extern const struct tini_field *
tini_field_find(const struct tini_section *s,
		const char *name, size_t namelen);

extern enum tini_result
tini_set(void *target,
		const struct tini_field *field,
		const struct tini *value);

extern enum tini_result
tini_assign(const struct tini_section *section,
		const struct tini *key,
		const struct tini *value,
		void *udata);

#endif

