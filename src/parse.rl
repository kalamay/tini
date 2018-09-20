#include "../include/tini.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>

%%{
	machine tini;

	action mark { mark = p; }

	action mark_line {
		bol = p + 1;
		line++;
	}

	action set_section { SET(section); labelp = NULL; }
	action set_label   { SET(label); labelp = &label; }
	action set_key     { SET(key); }
	action set_value   { SET(value); }

	action load_section {
		global_section = false;
		SECTION();
	}

	action assign {
		if (global_section && !has_section) {
			SECTION();
		}
		enum tini_result rc = load.assign ?
			load.assign(&load, &key, &value, ctx->udata) :
			TINI_UNUSED_SECTION;
		if (rc != TINI_SUCCESS) {
			tini_add_error(ctx, select_error(&key, &value, rc), NULL, rc);
		}
	}

	ws      = [\t\v\f\r ];
	nl      = '\n' >mark_line;
	name    = ( alpha | digit | '-' | '_' | '.' )+;
	string  = ( name | ':' )+;
	key     = string >mark %set_key;
	value   = [^\n]* >mark %set_value;
	comment = ( '#' | ';' ) [^\n]*;
	sname   = name >mark %set_section;
	slabel  = ':' ws* ( string >mark %set_label );
	section = '[' ws* sname ws* ( ':' slabel ws* )? ']';
	field   = key ws* '=' ws* ( value >mark );
	line    = ( comment | ( section %load_section ) | ( field %assign ) ) {,1} nl;

	main := line*;
}%%

static const struct tini *
select_error(const struct tini *key, const struct tini *value, enum tini_result rc)
{
	switch (rc) {
	case TINI_SUCCESS: return NULL;
	case TINI_SYNTAX: return key;
	case TINI_STRING_TOO_BIG: return value;
	case TINI_BOOL_FORMAT: return value;
	case TINI_INTEGER_FORMAT: return value;
	case TINI_INTEGER_TOO_SMALL: return value;
	case TINI_INTEGER_TOO_BIG: return value;
	case TINI_INTEGER_NEGATIVE: return value;
	case TINI_NUMBER_FORMAT: return value;
	case TINI_INVALID_TYPE: return value;
	case TINI_UNUSED_SECTION: return key;
	case TINI_UNUSED_KEY: return key;
	case TINI_MISSING_SECTION: return key;
	case TINI_MISSING_KEY: return key;
	}
	return key;
}

#define SECTION() do { \
	load.nfields = 0; \
	load.target = NULL; \
	load.assign = tini_assign; \
	enum tini_result rc = ctx->load_section ? \
		ctx->load_section(&load, &section, labelp, ctx->udata) : \
		TINI_UNUSED_SECTION; \
	has_section = rc == TINI_SUCCESS; \
	if (!has_section) { \
		tini_add_error(ctx, &section, NULL, rc); \
	} \
} while (0)

#define SET(n) do { \
	(n).start = mark; \
	(n).length = p - mark; \
	(n).line_start = bol; \
	(n).line = line; \
	(n).column = mark - bol; \
} while (0)

#define BAIL(c) do { \
	rc = (c); \
	goto done; \
} while (0)

enum tini_result
tini_parse(struct tini_ctx *ctx,
		const char *txt, size_t txtlen,
		int flags)
{
	(void)flags;

	const char *p = txt;
	const char *pe = p + txtlen;
	const char *mark = p;
	const char *bol = p;
	size_t line = 0;
	int cs = %%{ write start; }%%;

	struct tini section = { .type = TINI_SECTION };
	struct tini label = { .type = TINI_LABEL };
	struct tini key = { .type = TINI_KEY };
	struct tini value = { .type = TINI_VALUE };
	struct tini *labelp = NULL;
	bool global_section = true;
	bool has_section = false; 

	struct tini_section load = {};

	ctx->txt = txt;
	ctx->txtlen = txtlen;
	ctx->nerr = 0;

	%% write exec;

	if (cs < %%{ write first_final; }%%) {
		p++;
		SET(value);
		value.length = 1;
		value.type = TINI_NONE;
		tini_add_error(ctx, &value, NULL, TINI_SYNTAX);
	}

	return ctx->nerr ? ctx->err[0].code : TINI_SUCCESS;
}

