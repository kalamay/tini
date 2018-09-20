
#line 1 "src/parse.rl"
#include "../include/tini.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>


#line 53 "src/parse.rl"


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
	int cs = 14;

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

	
#line 91 "src/parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr1:
#line 12 "src/parse.rl"
	{
		bol = p + 1;
		line++;
	}
	goto st14;
tr10:
#line 10 "src/parse.rl"
	{ mark = p; }
#line 20 "src/parse.rl"
	{ SET(value); }
#line 27 "src/parse.rl"
	{
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
#line 12 "src/parse.rl"
	{
		bol = p + 1;
		line++;
	}
	goto st14;
tr12:
#line 20 "src/parse.rl"
	{ SET(value); }
#line 27 "src/parse.rl"
	{
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
#line 12 "src/parse.rl"
	{
		bol = p + 1;
		line++;
	}
	goto st14;
tr28:
#line 22 "src/parse.rl"
	{
		global_section = false;
		SECTION();
	}
#line 12 "src/parse.rl"
	{
		bol = p + 1;
		line++;
	}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 164 "src/parse.c"
	switch( (*p) ) {
		case 10: goto tr1;
		case 35: goto st1;
		case 59: goto st1;
		case 91: goto st6;
		case 95: goto tr29;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr29;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr29;
		} else if ( (*p) >= 65 )
			goto tr29;
	} else
		goto tr29;
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
tr29:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 202 "src/parse.c"
	switch( (*p) ) {
		case 9: goto tr2;
		case 32: goto tr2;
		case 61: goto tr5;
		case 95: goto st2;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st2;
		} else if ( (*p) >= 11 )
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
#line 19 "src/parse.rl"
	{ SET(key); }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 232 "src/parse.c"
	switch( (*p) ) {
		case 9: goto st3;
		case 32: goto st3;
		case 61: goto st4;
	}
	if ( 11 <= (*p) && (*p) <= 13 )
		goto st3;
	goto st0;
tr5:
#line 19 "src/parse.rl"
	{ SET(key); }
	goto st4;
tr9:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 253 "src/parse.c"
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
#line 269 "src/parse.c"
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
		case 95: goto tr14;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr14;
		} else if ( (*p) >= 11 )
			goto st6;
	} else if ( (*p) > 57 ) {
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
#line 305 "src/parse.c"
	switch( (*p) ) {
		case 9: goto tr15;
		case 32: goto tr15;
		case 58: goto tr17;
		case 93: goto tr18;
		case 95: goto st7;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st7;
		} else if ( (*p) >= 11 )
			goto tr15;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st7;
		} else if ( (*p) >= 65 )
			goto st7;
	} else
		goto st7;
	goto st0;
tr15:
#line 17 "src/parse.rl"
	{ SET(section); labelp = NULL; }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 336 "src/parse.c"
	switch( (*p) ) {
		case 9: goto st8;
		case 32: goto st8;
		case 58: goto st9;
		case 93: goto st13;
	}
	if ( 11 <= (*p) && (*p) <= 13 )
		goto st8;
	goto st0;
tr17:
#line 17 "src/parse.rl"
	{ SET(section); labelp = NULL; }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 354 "src/parse.c"
	if ( (*p) == 58 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 9: goto st10;
		case 32: goto st10;
		case 95: goto tr23;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr23;
		} else if ( (*p) >= 11 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr23;
		} else if ( (*p) >= 65 )
			goto tr23;
	} else
		goto tr23;
	goto st0;
tr23:
#line 10 "src/parse.rl"
	{ mark = p; }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 390 "src/parse.c"
	switch( (*p) ) {
		case 9: goto tr24;
		case 32: goto tr24;
		case 93: goto tr26;
		case 95: goto st11;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st11;
		} else if ( (*p) >= 11 )
			goto tr24;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st11;
		} else if ( (*p) >= 65 )
			goto st11;
	} else
		goto st11;
	goto st0;
tr24:
#line 18 "src/parse.rl"
	{ SET(label); labelp = &label; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 420 "src/parse.c"
	switch( (*p) ) {
		case 9: goto st12;
		case 32: goto st12;
		case 93: goto st13;
	}
	if ( 11 <= (*p) && (*p) <= 13 )
		goto st12;
	goto st0;
tr18:
#line 17 "src/parse.rl"
	{ SET(section); labelp = NULL; }
	goto st13;
tr26:
#line 18 "src/parse.rl"
	{ SET(label); labelp = &label; }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 441 "src/parse.c"
	if ( (*p) == 10 )
		goto tr28;
	goto st0;
	}
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 132 "src/parse.rl"

	if (cs < 14) {
		p++;
		SET(value);
		value.length = 1;
		value.type = TINI_NONE;
		tini_add_error(ctx, &value, NULL, TINI_SYNTAX);
	}

	return ctx->nerr ? ctx->err[0].code : TINI_SUCCESS;
}

