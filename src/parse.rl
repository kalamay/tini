#include "../include/tini.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>

%%{
	machine tini;

	action mark { mark = p; }

	action push_line {
		if (ctx->linepos == ctx->nlines) { BAIL(TINI_LINE_COUNT); }
		ctx->lines[ctx->linepos++] = ctx->offset;
		ctx->offset = (p - txt) + 1;
		ctx->cs = cs;
		if (key) {
			*sectail = key;
			sectail = keytail;
			key = NULL;
		}
	}

	action set_section {
		sec = get_node(ctx, TINI_SECTION, get_column(ctx, mark - txt), p - mark);
		if (sec == NULL) { BAIL(TINI_NODE_COUNT); }
		sectail = &sec->next;
	}

	action store_label {
		struct tini *label = get_node(ctx, TINI_LABEL, get_column(ctx, mark - txt), p - mark);
		if (label == NULL) { BAIL(TINI_NODE_COUNT); }
		*sectail = label;
		sectail = &label->next;
	}

	action store_key {
		key = get_node(ctx, TINI_KEY, get_column(ctx, mark - txt), p - mark);
		if (key == NULL) { BAIL(TINI_NODE_COUNT); }
		keytail = &key->next;
	}

	action store_value {
		struct tini *val = get_node(ctx, TINI_VALUE, get_column(ctx, mark - txt), p - mark);
		if (val == NULL) { BAIL(TINI_NODE_COUNT); }
		*keytail = val;
		keytail = &val->next;
	}

	ws      = [\t\v\f\r ];
	nl      = '\n' >push_line;
	name    = ( alpha | digit | '-' | '_' | '.' )+;
	string  = ( name | ':' )+;
	key     = string >mark %store_key;
	value   = [^\n]* >mark %store_value;
	comment = ( '#' | ';' ) [^\n]*;
	sname   = name >mark %set_section;
	slabel  = ':' ws* ( string >mark %store_label );
	section = '[' ws* sname ws* ( ':' slabel ws* )? ']';
	setting = key ws* '=' ws* ( value >mark );
	line    = ( comment | section | setting ) {,1} nl;

	main := line*;
}%%

static uint32_t
get_column(struct tini_ctx *ctx, uint32_t offset)
{
	return (uint16_t)(offset - ctx->offset);
}

static struct tini *
get_node(struct tini_ctx *ctx, enum tini_type type, uint16_t column, uint16_t length)
{
	struct tini *node = NULL;
	if (ctx->nused[0] + ctx->nused[1] < ctx->nnodes) {
		bool sec = type == TINI_SECTION;
		size_t idx = sec ? ctx->nnodes - ++ctx->nused[1] : ctx->nused[0]++;
		node = &ctx->nodes[idx];
		node->type = type;
		node->linepos = ctx->linepos;
		node->column = column;
		node->length = length;
		node->next = NULL;
		if (sec) {
			ctx->section = node;
		}
	}
	return node;
}

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
	p += ctx->offset;

	const char *mark = p;
	int cs = ctx->cs;
	enum tini_result rc = TINI_SUCCESS;
	struct tini *sec, **sectail = NULL, *key = NULL, **keytail = NULL;

	if (cs < 0) {
		cs = %%{ write start; }%%;
	}

	ctx->nused[0] = 0;
	ctx->nused[1] = 0;

	// if we have an existing section we need to do two things:
	//   1) move the section to the first section slot
	//   2) save the line position information
	{
		uint16_t column = 0;
		uint32_t length = 0;
		struct tini *tmp = ctx->section;

		if (tmp) {
			column = tini_column(ctx, tmp);
			length = tini_length(ctx, tmp);
			ctx->lines[0] = ctx->lines[tmp->linepos];
			ctx->linestart = tini_line(ctx, tmp);
		}
		else {
			ctx->linestart = 0;
		}

		ctx->linepos = 0;

		sec = get_node(ctx, TINI_SECTION, column, length);
		sectail = &sec->next;

		if (tmp) {
			sec->column = 1;
			ctx->linepos++;
		}
	}

	ctx->txt = txt;
	ctx->txtlen = txtlen;
	ctx->nerr = 0;

	%% write exec;

	if (cs < %%{ write first_final; }%%) {
		p++;
		cs = -1;
		BAIL(TINI_SYNTAX);
	}

done:
	return rc;
}

