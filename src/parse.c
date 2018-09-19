
#line 1 "src/parse.rl"
#include "../include/tini.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>


#line 54 "src/parse.rl"


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
		cs = 10;
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

	
#line 102 "src/parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr1:
#line 12 "src/parse.rl"
	{
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
	goto st10;
tr10:
#line 10 "src/parse.rl"
	{ mark = p; }
#line 36 "src/parse.rl"
	{
		struct tini *val = get_node(ctx, TINI_VALUE, get_column(ctx, mark - txt), p - mark);
		if (val == NULL) { BAIL(TINI_NODE_COUNT); }
		*keytail = val;
		keytail = &val->next;
	}
#line 12 "src/parse.rl"
	{
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
	goto st10;
tr12:
#line 36 "src/parse.rl"
	{
		struct tini *val = get_node(ctx, TINI_VALUE, get_column(ctx, mark - txt), p - mark);
		if (val == NULL) { BAIL(TINI_NODE_COUNT); }
		*keytail = val;
		keytail = &val->next;
	}
#line 12 "src/parse.rl"
	{
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
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 170 "src/parse.c"
	switch( (*p) ) {
		case 10: goto tr1;
		case 35: goto st1;
		case 45: goto tr20;
		case 59: goto st1;
		case 91: goto st6;
		case 95: goto tr20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 58 )
			goto tr20;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr20;
	} else
		goto tr20;
	goto st0;
st0:
cs = 0;
	goto _out;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	if ( (*p) == 10 )
		goto tr1;
	goto st1;
tr20:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 206 "src/parse.c"
	switch( (*p) ) {
		case 9: goto tr2;
		case 32: goto tr2;
		case 45: goto st2;
		case 61: goto tr5;
		case 95: goto st2;
	}
	if ( (*p) < 48 ) {
		if ( 11 <= (*p) && (*p) <= 13 )
			goto tr2;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st2;
		} else if ( (*p) >= 65 )
			goto st2;
	} else
		goto st2;
	goto st0;
tr2:
#line 30 "src/parse.rl"
	{
		key = get_node(ctx, TINI_KEY, get_column(ctx, mark - txt), p - mark);
		if (key == NULL) { BAIL(TINI_NODE_COUNT); }
		keytail = &key->next;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 238 "src/parse.c"
	switch( (*p) ) {
		case 9: goto st3;
		case 32: goto st3;
		case 61: goto st4;
	}
	if ( 11 <= (*p) && (*p) <= 13 )
		goto st3;
	goto st0;
tr5:
#line 30 "src/parse.rl"
	{
		key = get_node(ctx, TINI_KEY, get_column(ctx, mark - txt), p - mark);
		if (key == NULL) { BAIL(TINI_NODE_COUNT); }
		keytail = &key->next;
	}
	goto st4;
tr9:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 263 "src/parse.c"
	switch( (*p) ) {
		case 10: goto tr10;
		case 32: goto tr9;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr9;
	goto tr8;
tr8:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 279 "src/parse.c"
	if ( (*p) == 10 )
		goto tr12;
	goto st5;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 9: goto st6;
		case 32: goto st6;
		case 45: goto tr14;
		case 95: goto tr14;
	}
	if ( (*p) < 48 ) {
		if ( 11 <= (*p) && (*p) <= 13 )
			goto st6;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr14;
		} else if ( (*p) >= 65 )
			goto tr14;
	} else
		goto tr14;
	goto st0;
tr14:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 313 "src/parse.c"
	switch( (*p) ) {
		case 9: goto tr15;
		case 32: goto tr15;
		case 45: goto st7;
		case 93: goto tr17;
		case 95: goto st7;
	}
	if ( (*p) < 48 ) {
		if ( 11 <= (*p) && (*p) <= 13 )
			goto tr15;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st7;
		} else if ( (*p) >= 65 )
			goto st7;
	} else
		goto st7;
	goto st0;
tr15:
#line 24 "src/parse.rl"
	{
		sec = get_node(ctx, TINI_SECTION, get_column(ctx, mark - txt), p - mark);
		if (sec == NULL) { BAIL(TINI_NODE_COUNT); }
		sectail = &sec->next;
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 345 "src/parse.c"
	switch( (*p) ) {
		case 9: goto st8;
		case 32: goto st8;
		case 93: goto st9;
	}
	if ( 11 <= (*p) && (*p) <= 13 )
		goto st8;
	goto st0;
tr17:
#line 24 "src/parse.rl"
	{
		sec = get_node(ctx, TINI_SECTION, get_column(ctx, mark - txt), p - mark);
		if (sec == NULL) { BAIL(TINI_NODE_COUNT); }
		sectail = &sec->next;
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 366 "src/parse.c"
	if ( (*p) == 10 )
		goto tr1;
	goto st0;
	}
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 144 "src/parse.rl"

	if (cs < 10) {
		p++;
		cs = -1;
		BAIL(TINI_SYNTAX);
	}

done:
	return rc;
}
