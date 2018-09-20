#include "mu.h"
#include "../include/tini.h"

struct small
{
	bool global1;
	unsigned global2;
	struct small_section1 {
		char name[16];
	} section1;
};

static const struct tini_field small_global[] = {
	tini_field_make(struct small, global1),
	tini_field_make(struct small, global2),
};

static const struct tini_field small_section1[] = {
	tini_field_make(struct small_section1, name),
};

static enum tini_result
load_small(struct tini_section *section,
			const struct tini *name,
			const struct tini *label,
			void *udata)
{
	(void)label;

	struct small *target = udata;
	if (name->length == 0) {
		tini_section_set(section, target, small_global);
		return TINI_SUCCESS;
	}
	if (tini_streq(name, "section1")) {
		tini_section_set(section, &target->section1, small_section1);
		return TINI_SUCCESS;
	}
	return TINI_MISSING_SECTION;
}


struct types
{
	struct types_strings {
		char value1[16];
	} strings;
	struct types_booleans {
		bool value1;
		bool value2;
		bool value3;
		bool value4;
		bool value5;
		bool value6;
	} booleans;
	struct types_ints {
		uint8_t value1;
		int value2;
		unsigned short value3;
		unsigned int value4;
	} ints;
	struct types_doubles {
		double value1;
	} doubles;
};

static const struct tini_field types_strings[] = {
	tini_field_make(struct types_strings, value1),
};

static const struct tini_field types_booleans[] = {
	tini_field_make(struct types_booleans, value1),
	tini_field_make(struct types_booleans, value2),
	tini_field_make(struct types_booleans, value3),
	tini_field_make(struct types_booleans, value4),
	tini_field_make(struct types_booleans, value5),
	tini_field_make(struct types_booleans, value6),
};

static const struct tini_field types_ints[] = {
	tini_field_make(struct types_ints, value1),
	tini_field_make(struct types_ints, value2),
	tini_field_make(struct types_ints, value3),
	tini_field_make(struct types_ints, value4),
};

static const struct tini_field types_doubles[] = {
	tini_field_make(struct types_doubles, value1),
};

static enum tini_result
load_types(struct tini_section *section,
			const struct tini *name,
			const struct tini *label,
			void *udata)
{
	(void)label;

	struct types *target = udata;
	if (tini_streq(name, "strings")) {
		tini_section_set(section, &target->strings, types_strings);
		return TINI_SUCCESS;
	}
	if (tini_streq(name, "booleans")) {
		tini_section_set(section, &target->booleans, types_booleans);
		return TINI_SUCCESS;
	}
	if (tini_streq(name, "ints")) {
		tini_section_set(section, &target->ints, types_ints);
		return TINI_SUCCESS;
	}
	if (tini_streq(name, "doubles")) {
		tini_section_set(section, &target->doubles, types_doubles);
		return TINI_SUCCESS;
	}
	return TINI_MISSING_SECTION;
}

static void
test_basic(void)
{
	static const char cfg[] = 
		"global1 = true\n"
		"global2 = 12345\n"
		"\n"
		"[section1]\n"
		"name = stuff\n"
		;

	struct small target = {};

	struct tini_ctx ctx = tini_ctx_make(load_small, &target);

	mu_assert_int_eq(tini_parse(&ctx, cfg, sizeof(cfg)-1, 0), TINI_SUCCESS);
	mu_assert_int_eq(target.global1, true);
	mu_assert_int_eq(target.global2, 12345);
	mu_assert_str_eq(target.section1.name, "stuff");
}

static void
test_types(void)
{
	static const char cfg[] =
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

	struct types target = {};

	struct tini_ctx ctx = tini_ctx_make(load_types, &target);

	mu_assert_int_eq(tini_parse(&ctx, cfg, sizeof(cfg)-1, 0), TINI_SUCCESS);
	mu_assert_str_eq(target.strings.value1, "test");
	mu_assert_int_eq(target.booleans.value1, true);
	mu_assert_int_eq(target.booleans.value2, true);
	mu_assert_int_eq(target.booleans.value3, true);
	mu_assert_int_eq(target.booleans.value4, false);
	mu_assert_int_eq(target.booleans.value5, false);
	mu_assert_int_eq(target.booleans.value6, false);
	mu_assert_int_eq(target.ints.value1, 123);
	mu_assert_int_eq(target.ints.value2, -123);
	mu_assert_int_eq(target.ints.value3, 0xffff);
	mu_assert_int_eq(target.ints.value4, 0644);
	mu_assert_flt_eq(target.doubles.value1, 1.23);
}

static void
test_invalid_too_big(void)
{
	static const char cfg[] = 
		"[section1]\n"
		"name = testing testing 123\n"
		;

	struct small target = {};

	struct tini_ctx ctx = tini_ctx_make(load_small, &target);

	mu_assert_int_eq(tini_parse(&ctx, cfg, sizeof(cfg)-1, 0), TINI_STRING_TOO_BIG);
}

static void
test_invalid_int(void)
{
	static const char cfg[] = 
		"global2 = foo\n"
		;

	struct small target = {};

	struct tini_ctx ctx = tini_ctx_make(load_small, &target);

	mu_assert_int_eq(tini_parse(&ctx, cfg, sizeof(cfg)-1, 0), TINI_INTEGER_FORMAT);
}

struct labels {
	struct labels_foo {
		char id;
		char name[16];
	} foos[2];
	size_t nfoos;
};

static const struct tini_field labels_foo_fields[] = {
	tini_field_make(struct labels_foo, name),
};

static enum tini_result
load_section(struct tini_section *section,
			const struct tini *name,
			const struct tini *label,
			void *udata)
{
	struct labels *target = udata;
	if (tini_streq(name, "foo") && label->length == 1 && target->nfoos < 2) {
		struct labels_foo *foo = &target->foos[target->nfoos++];
		foo->id = label->start[0];
		tini_section_set(section, foo, labels_foo_fields);
		return TINI_SUCCESS;
	}
	return TINI_MISSING_SECTION;
}

static void
test_label(void)
{
	static const char cfg[] = 
		"[foo:x]\n"
		"name = foo\n"
		"[foo:y]\n"
		"name = bar\n"
		;


	struct labels target = { .foos = 0 };

	struct tini_ctx ctx = tini_ctx_make(load_section, &target);

	mu_assert_int_eq(tini_parse(&ctx, cfg, sizeof(cfg)-1, 0), TINI_SUCCESS);
	mu_assert_int_eq(target.nfoos, 2);
	mu_assert_int_eq(target.foos[0].id, 'x');
	mu_assert_str_eq(target.foos[0].name, "foo");
	mu_assert_int_eq(target.foos[1].id, 'y');
	mu_assert_str_eq(target.foos[1].name, "bar");
}

int
main(void)
{
	mu_init("parse");

	mu_run(test_basic);
	mu_run(test_types);
	mu_run(test_invalid_too_big);
	mu_run(test_invalid_int);
	mu_run(test_label);
}

