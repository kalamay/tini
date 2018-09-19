#include "mu.h"
#include "../include/tini.h"

static const char small[] = 
	"global1 = true\n"
	"global2 = 12345\n"
	"\n"
	"[section1]\n"
	"name = stuff\n"
	;

static const char types[] =
	"[strings]\n"
	"value1 = test\n"
	"\n"
	"[booleans]\n"
	"value1 = true\n"
	"value2 = yes\n"
	"value3 = on\n"
	"value4 = false\n"
	"value5 = no\n"
	"value6 = off\n"
	"\n"
	"[ints]\n"
	"value1 = 123\n"
	"value2 = -123\n"
	"value3 = 0xffff\n"
	"value4 = 0644\n"
	"\n"
	"[doubles]\n"
	"value1 = 1.23\n"
	;

static void
test_basic(void)
{
	// allow us to scan upto 256 nodes and 128 lines in a single pass
	struct tini_ctx ctx = tini_ctx_make(256, 128);
	struct tini *global, *section1, *node;
	bool bval;
	char str[32];
	int64_t ival;
	double dval;

	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_SUCCESS);

	global = tini_section(&ctx, NULL, 0);
	mu_assert_ptr_ne(global, NULL);

	section1 = tini_section(&ctx, "section1", 8);
	mu_assert_ptr_ne(section1, NULL);

	node = tini_key(&ctx, global, "global1", 7);
	mu_assert_str_eq(tini_str(&ctx, tini_value(node), str, sizeof(str)), "true");
	mu_assert_ptr_eq(tini_value(tini_value(node)), NULL);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), TINI_SUCCESS);
	mu_assert_int_eq(bval, true);

	node = tini_key(&ctx, global, "global2", 7);
	mu_assert(tini_streq(&ctx, tini_value(node), "12345"));
	mu_assert_int_eq(tini_int(&ctx, tini_value(node), 10, &ival), TINI_SUCCESS);
	mu_assert_int_eq(ival, 12345);
	mu_assert_int_eq(tini_double(&ctx, tini_value(node), &dval), TINI_SUCCESS);
	mu_assert_flt_eq(ival, 12345.0);
	mu_assert_ptr_eq(tini_value(tini_value(node)), NULL);

	node = tini_key(&ctx, section1, "name", 4);
	mu_assert_str_eq(tini_str(&ctx, tini_value(node), str, sizeof(str)), "stuff");
	mu_assert_ptr_eq(tini_value(tini_value(node)), NULL);
}

static void
test_positions(void)
{
	// this is *just* large enough to parse the above small input
	// there are 8 nodes because of the implicit global section
	struct tini_ctx ctx = tini_ctx_make(8, 5);
	struct tini *global, *section1, *node;

	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_SUCCESS);

	global = tini_section(&ctx, NULL, 0);
	mu_assert_ptr_ne(global, NULL);
	mu_assert_int_eq(global->type, TINI_SECTION);
	mu_assert_int_eq(global->used, true);
	mu_assert_int_eq(tini_offset(&ctx, global), 0);
	mu_assert_int_eq(tini_length(&ctx, global), 0);
	mu_assert_int_eq(tini_line(&ctx, global), 0);
	mu_assert_int_eq(tini_column(&ctx, global), 0);

	section1 = tini_section(&ctx, "section1", 8);
	mu_assert_ptr_ne(section1, NULL);
	mu_assert_int_eq(section1->type, TINI_SECTION);
	mu_assert_int_eq(section1->used, true);
	mu_assert_int_eq(tini_offset(&ctx, section1), 33);
	mu_assert_int_eq(tini_length(&ctx, section1), 8);
	mu_assert_int_eq(tini_line(&ctx, section1), 3);
	mu_assert_int_eq(tini_column(&ctx, section1), 1);

	node = tini_section(&ctx, "foo", 3);
	mu_assert_ptr_eq(node, NULL);

	node = tini_key(&ctx, global, "global1", 7);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 0);
	mu_assert_int_eq(tini_length(&ctx, node), 7);
	mu_assert_int_eq(tini_line(&ctx, node), 0);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 10);
	mu_assert_int_eq(tini_length(&ctx, node), 4);
	mu_assert_int_eq(tini_line(&ctx, node), 0);
	mu_assert_int_eq(tini_column(&ctx, node), 10);

	node = tini_key(&ctx, global, "global2", 7);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 15);
	mu_assert_int_eq(tini_length(&ctx, node), 7);
	mu_assert_int_eq(tini_line(&ctx, node), 1);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 25);
	mu_assert_int_eq(tini_length(&ctx, node), 5);
	mu_assert_int_eq(tini_line(&ctx, node), 1);
	mu_assert_int_eq(tini_column(&ctx, node), 10);

	node = tini_key(&ctx, section1, "name", 4);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 43);
	mu_assert_int_eq(tini_length(&ctx, node), 4);
	mu_assert_int_eq(tini_line(&ctx, node), 4);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 50);
	mu_assert_int_eq(tini_length(&ctx, node), 5);
	mu_assert_int_eq(tini_line(&ctx, node), 4);
	mu_assert_int_eq(tini_column(&ctx, node), 7);
}

static void
test_node_count(void)
{
	struct tini_ctx ctx = tini_ctx_make(7, 5);
	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_NODE_COUNT);
}

static void
test_line_count(void)
{
	struct tini_ctx ctx = tini_ctx_make(8, 4);
	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_LINE_COUNT);
}

static void
test_resume_node(void)
{
	struct tini_ctx ctx = tini_ctx_make(7, 5);
	struct tini *global, *section1, *node;

	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_NODE_COUNT);

	global = tini_section(&ctx, NULL, 0);
	mu_assert_ptr_ne(global, NULL);
	mu_assert_int_eq(global->type, TINI_SECTION);
	mu_assert_int_eq(global->used, true);
	mu_assert_int_eq(tini_offset(&ctx, global), 0);
	mu_assert_int_eq(tini_length(&ctx, global), 0);
	mu_assert_int_eq(tini_line(&ctx, global), 0);
	mu_assert_int_eq(tini_column(&ctx, global), 0);

	section1 = tini_section(&ctx, "section1", 8);
	mu_assert_ptr_ne(section1, NULL);
	mu_assert_int_eq(section1->type, TINI_SECTION);
	mu_assert_int_eq(section1->used, true);
	mu_assert_int_eq(tini_offset(&ctx, section1), 33);
	mu_assert_int_eq(tini_length(&ctx, section1), 8);
	mu_assert_int_eq(tini_line(&ctx, section1), 3);
	mu_assert_int_eq(tini_column(&ctx, section1), 1);

	node = tini_section(&ctx, "foo", 3);
	mu_assert_ptr_eq(node, NULL);

	node = tini_key(&ctx, global, "global1", 7);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 0);
	mu_assert_int_eq(tini_length(&ctx, node), 7);
	mu_assert_int_eq(tini_line(&ctx, node), 0);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 10);
	mu_assert_int_eq(tini_length(&ctx, node), 4);
	mu_assert_int_eq(tini_line(&ctx, node), 0);
	mu_assert_int_eq(tini_column(&ctx, node), 10);

	node = tini_key(&ctx, global, "global2", 7);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 15);
	mu_assert_int_eq(tini_length(&ctx, node), 7);
	mu_assert_int_eq(tini_line(&ctx, node), 1);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 25);
	mu_assert_int_eq(tini_length(&ctx, node), 5);
	mu_assert_int_eq(tini_line(&ctx, node), 1);
	mu_assert_int_eq(tini_column(&ctx, node), 10);

	node = tini_key(&ctx, section1, "name", 4);
	mu_assert_ptr_eq(node, NULL);

	// parse more input, this will clear all keys and all but the
	// current section
	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_SUCCESS);

	section1 = tini_section(&ctx, "section1", 8);
	mu_assert_ptr_ne(section1, NULL);
	mu_assert_int_eq(section1->type, TINI_SECTION);
	mu_assert_int_eq(section1->used, true);
	mu_assert_int_eq(tini_offset(&ctx, section1), 33);
	mu_assert_int_eq(tini_length(&ctx, section1), 8);
	mu_assert_int_eq(tini_line(&ctx, section1), 3);
	mu_assert_int_eq(tini_column(&ctx, section1), 1);

	node = tini_key(&ctx, section1, "name", 4);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 43);
	mu_assert_int_eq(tini_length(&ctx, node), 4);
	mu_assert_int_eq(tini_line(&ctx, node), 4);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 50);
	mu_assert_int_eq(tini_length(&ctx, node), 5);
	mu_assert_int_eq(tini_line(&ctx, node), 4);
	mu_assert_int_eq(tini_column(&ctx, node), 7);
}

static void
test_resume_line(void)
{
	struct tini_ctx ctx = tini_ctx_make(8, 4);
	struct tini *global, *section1, *node;

	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_LINE_COUNT);

	global = tini_section(&ctx, NULL, 0);
	mu_assert_ptr_ne(global, NULL);
	mu_assert_int_eq(global->type, TINI_SECTION);
	mu_assert_int_eq(global->used, true);
	mu_assert_int_eq(tini_offset(&ctx, global), 0);
	mu_assert_int_eq(tini_length(&ctx, global), 0);
	mu_assert_int_eq(tini_line(&ctx, global), 0);
	mu_assert_int_eq(tini_column(&ctx, global), 0);

	section1 = tini_section(&ctx, "section1", 8);
	mu_assert_ptr_ne(section1, NULL);
	mu_assert_int_eq(section1->type, TINI_SECTION);
	mu_assert_int_eq(section1->used, true);
	mu_assert_int_eq(tini_offset(&ctx, section1), 33);
	mu_assert_int_eq(tini_length(&ctx, section1), 8);
	mu_assert_int_eq(tini_line(&ctx, section1), 3);
	mu_assert_int_eq(tini_column(&ctx, section1), 1);

	node = tini_section(&ctx, "foo", 3);
	mu_assert_ptr_eq(node, NULL);

	node = tini_key(&ctx, global, "global1", 7);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 0);
	mu_assert_int_eq(tini_length(&ctx, node), 7);
	mu_assert_int_eq(tini_line(&ctx, node), 0);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 10);
	mu_assert_int_eq(tini_length(&ctx, node), 4);
	mu_assert_int_eq(tini_line(&ctx, node), 0);
	mu_assert_int_eq(tini_column(&ctx, node), 10);

	node = tini_key(&ctx, global, "global2", 7);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 15);
	mu_assert_int_eq(tini_length(&ctx, node), 7);
	mu_assert_int_eq(tini_line(&ctx, node), 1);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 25);
	mu_assert_int_eq(tini_length(&ctx, node), 5);
	mu_assert_int_eq(tini_line(&ctx, node), 1);
	mu_assert_int_eq(tini_column(&ctx, node), 10);

	node = tini_key(&ctx, section1, "name", 4);
	mu_assert_ptr_eq(node, NULL);

	// parse more input, this will clear all keys and all but the
	// current section
	mu_assert_int_eq(tini_parse(&ctx, small, sizeof(small)-1, 0), TINI_SUCCESS);

	section1 = tini_section(&ctx, "section1", 8);
	mu_assert_ptr_ne(section1, NULL);
	mu_assert_int_eq(section1->type, TINI_SECTION);
	mu_assert_int_eq(section1->used, true);
	mu_assert_int_eq(tini_offset(&ctx, section1), 33);
	mu_assert_int_eq(tini_length(&ctx, section1), 8);
	mu_assert_int_eq(tini_line(&ctx, section1), 3);
	mu_assert_int_eq(tini_column(&ctx, section1), 1);

	node = tini_key(&ctx, section1, "name", 4);
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_KEY);
	mu_assert_int_eq(node->used, true);
	mu_assert_int_eq(tini_offset(&ctx, node), 43);
	mu_assert_int_eq(tini_length(&ctx, node), 4);
	mu_assert_int_eq(tini_line(&ctx, node), 4);
	mu_assert_int_eq(tini_column(&ctx, node), 0);

	node = node->next;
	mu_assert_ptr_ne(node, NULL);
	mu_assert_int_eq(node->type, TINI_VALUE);
	mu_assert_int_eq(tini_offset(&ctx, node), 50);
	mu_assert_int_eq(tini_length(&ctx, node), 5);
	mu_assert_int_eq(tini_line(&ctx, node), 4);
	mu_assert_int_eq(tini_column(&ctx, node), 7);
}

static void
test_types(void)
{
	struct tini_ctx ctx = tini_ctx_make(256, 128);
	struct tini *section, *node;
	char sval[32];
	bool bval;
	int64_t ival;
	double dval;

	mu_assert_int_eq(tini_parse(&ctx, types, sizeof(types)-1, 0), TINI_SUCCESS);

	section = tini_section(&ctx, "strings", 7);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_str_eq(tini_str(&ctx, tini_value(node), sval, sizeof(sval)), "test");

	section = tini_section(&ctx, "booleans", 8);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), 0);
	mu_assert_int_eq(bval, true);
	node = tini_key(&ctx, section, "value2", 6);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), 0);
	mu_assert_int_eq(bval, true);
	node = tini_key(&ctx, section, "value3", 6);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), 0);
	mu_assert_int_eq(bval, true);
	node = tini_key(&ctx, section, "value4", 6);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), 0);
	mu_assert_int_eq(bval, false);
	node = tini_key(&ctx, section, "value5", 6);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), 0);
	mu_assert_int_eq(bval, false);
	node = tini_key(&ctx, section, "value6", 6);
	mu_assert_int_eq(tini_bool(&ctx, tini_value(node), &bval), 0);
	mu_assert_int_eq(bval, false);

	section = tini_section(&ctx, "ints", 4);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_int(&ctx, tini_value(node), 0, &ival), 0);
	mu_assert_int_eq(ival, 123);
	node = tini_key(&ctx, section, "value2", 6);
	mu_assert_int_eq(tini_int(&ctx, tini_value(node), 0, &ival), 0);
	mu_assert_int_eq(ival, -123);
	node = tini_key(&ctx, section, "value3", 6);
	mu_assert_int_eq(tini_int(&ctx, tini_value(node), 0, &ival), 0);
	mu_assert_int_eq(ival, 0xffff);
	node = tini_key(&ctx, section, "value4", 6);
	mu_assert_int_eq(tini_int(&ctx, tini_value(node), 0, &ival), 0);
	mu_assert_int_eq(ival, 0644);

	section = tini_section(&ctx, "doubles", 7);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_double(&ctx, tini_value(node), &dval), 0);
	mu_assert_flt_eq(dval, 1.23);
}

static void
test_set(void)
{
	struct tini_ctx ctx = tini_ctx_make(256, 128);
	struct tini *section, *node;
	char small[2], big[32];
	bool bval;
	int64_t ival;
	double dval;

	mu_assert_int_eq(tini_parse(&ctx, types, sizeof(types)-1, 0), TINI_SUCCESS);

	section = tini_section(&ctx, "strings", 7);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), small), TINI_STRING_SIZE);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), big), TINI_SUCCESS);
	mu_assert_str_eq(big, "test");
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), bval), TINI_BOOL_FORMAT);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), ival), TINI_INTEGER_FORMAT);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), dval), TINI_NUMBER_FORMAT);

	section = tini_section(&ctx, "booleans", 8);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), bval), TINI_SUCCESS);
	mu_assert_int_eq(bval, true);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), ival), TINI_INTEGER_FORMAT);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), dval), TINI_NUMBER_FORMAT);

	section = tini_section(&ctx, "ints", 4);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), bval), TINI_BOOL_FORMAT);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), ival), TINI_SUCCESS);
	mu_assert_int_eq(ival, 123);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), dval), TINI_SUCCESS);
	mu_assert_flt_eq(dval, 123.0);

	section = tini_section(&ctx, "doubles", 7);
	node = tini_key(&ctx, section, "value1", 6);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), bval), TINI_BOOL_FORMAT);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), ival), TINI_INTEGER_FORMAT);
	mu_assert_int_eq(tini_set(&ctx, tini_value(node), dval), TINI_SUCCESS);
	mu_assert_flt_eq(dval, 1.23);
}

static void
test_sections(void)
{
	struct tini_ctx ctx = tini_ctx_make(256, 128);
	struct tini *section;

	mu_assert_int_eq(tini_parse(&ctx, types, sizeof(types)-1, 0), TINI_SUCCESS);

	section = tini_next_section(&ctx, NULL);
	mu_assert(tini_streq(&ctx, section, ""));
	section = tini_next_section(&ctx, section);
	mu_assert(tini_streq(&ctx, section, "strings"));
	section = tini_next_section(&ctx, section);
	mu_assert(tini_streq(&ctx, section, "booleans"));
	section = tini_next_section(&ctx, section);
	mu_assert(tini_streq(&ctx, section, "ints"));
	section = tini_next_section(&ctx, section);
	mu_assert(tini_streq(&ctx, section, "doubles"));
	section = tini_next_section(&ctx, section);
	mu_assert_ptr_eq(section, NULL);
}

int
main(void)
{
	mu_init("parse");

	mu_run(test_basic);
	mu_run(test_positions);
	mu_run(test_node_count);
	mu_run(test_line_count);
	mu_run(test_resume_node);
	mu_run(test_resume_line);
	mu_run(test_types);
	mu_run(test_set);
	mu_run(test_sections);
}

