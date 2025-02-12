/*************************************************************************/
/*  vita-mksfoex.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/*
# _____	 ___ ____	 ___ ____
#  ____|   |	____|   |		| |____|
# |	 ___|   |	 ___|	____| |	\	PSPDEV Open Source Project.
#-----------------------------------------------------------------------
# Review pspsdk README & LICENSE files for further details.
#
# New and improved mksfo
# $Id$
*/
#define _CRT_SECURE_NO_DEPRECATE

#include "export.h"
#include "types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PSF_MAGIC 0x46535000
#define PSF_VERSION 0x00000101

struct SfoHeader {
	uint32_t magic;
	uint32_t version;
	uint32_t keyofs;
	uint32_t valofs;
	uint32_t count;
};

struct SfoEntry {
	uint16_t nameofs;
	uint8_t alignment;
	uint8_t type;
	uint32_t valsize;
	uint32_t totalsize;
	uint32_t dataofs;
};

#define PSF_TYPE_BIN 0
#define PSF_TYPE_STR 2
#define PSF_TYPE_VAL 4

struct EntryContainer {
	const char *name;
	int type;
	uint32_t value;
	const char *data;
};

struct EntryContainer g_defaults[] = {
	{ "APP_VER", PSF_TYPE_STR, 0, "00.00" },
	{ "ATTRIBUTE", PSF_TYPE_VAL, 0x8000, NULL },
	{ "ATTRIBUTE2", PSF_TYPE_VAL, 0, NULL },
	{ "ATTRIBUTE_MINOR", PSF_TYPE_VAL, 0x10, NULL },
	{ "BOOT_FILE", PSF_TYPE_STR, 32, "" },
	{ "CATEGORY", PSF_TYPE_STR, 0, "gd" },
	{ "CONTENT_ID", PSF_TYPE_STR, 48, "" },
	{ "EBOOT_APP_MEMSIZE", PSF_TYPE_VAL, 0, NULL },
	{ "EBOOT_ATTRIBUTE", PSF_TYPE_VAL, 0, NULL },
	{ "EBOOT_PHY_MEMSIZE", PSF_TYPE_VAL, 0, NULL },
	{ "LAREA_TYPE", PSF_TYPE_VAL, 0, NULL },
	{ "NP_COMMUNICATION_ID", PSF_TYPE_STR, 16, "" },
	{ "PARENTAL_LEVEL", PSF_TYPE_VAL, 0, NULL },
	{ "PSP2_DISP_VER", PSF_TYPE_STR, 0, "00.000" },
	{ "PSP2_SYSTEM_VER", PSF_TYPE_VAL, 0, NULL },
	{ "STITLE", PSF_TYPE_STR, 52, "Homebrew" },
	{ "TITLE", PSF_TYPE_STR, 0x80, "Homebrew" },
	{ "TITLE_ID", PSF_TYPE_STR, 0, "ABCD99999" },
	{ "VERSION", PSF_TYPE_STR, 0, "00.00" },
};

#define MAX_OPTIONS (256)

static const char *g_title = NULL;
static int g_empty = 0;
static struct EntryContainer g_vals[MAX_OPTIONS];

struct EntryContainer *find_free() {
	int i;

	for (i = 0; i < MAX_OPTIONS; i++) {
		if (g_vals[i].name == NULL) {
			return &g_vals[i];
		}
	}

	return NULL;
}

struct EntryContainer *find_name(const char *name) {
	int i;

	for (i = 0; i < MAX_OPTIONS; i++) {
		if ((g_vals[i].name != NULL) && (strcmp(g_vals[i].name, name) == 0)) {
			return &g_vals[i];
		}
	}

	return NULL;
}

int add_string(char *str) {
	char *equals = NULL;
	struct EntryContainer *entry;

	equals = strchr(str, '=');
	if (equals == NULL) {
		fprintf(stderr, "Invalid option (no =)\n");
		return 0;
	}

	*equals++ = 0;

	if ((entry = find_name(str))) {
		entry->data = equals;
	} else {
		entry = find_free();
		if (entry == NULL) {
			fprintf(stderr, "Maximum options reached\n");
			return 0;
		}

		memset(entry, 0, sizeof(struct EntryContainer));
		entry->name = str;
		entry->type = PSF_TYPE_STR;
		entry->data = equals;
	}

	return 1;
}

int add_dword(char *str) {
	char *equals = NULL;
	struct EntryContainer *entry;

	equals = strchr(str, '=');
	if (equals == NULL) {
		fprintf(stderr, "Invalid option (no =)\n");
		return 0;
	}

	*equals++ = 0;

	if ((entry = find_name(str))) {
		entry->value = strtoul(equals, NULL, 0);
	} else {
		entry = find_free();
		if (entry == NULL) {
			fprintf(stderr, "Maximum options reached\n");
			return 0;
		}

		memset(entry, 0, sizeof(struct EntryContainer));
		entry->name = str;
		entry->type = PSF_TYPE_VAL;
		entry->value = strtoul(equals, NULL, 0);
	}

	return 1;
}

int mksfoex(ParamSFOStruct *sfo, String outDir) {
	FILE *fp;
	unsigned int i;
	char head[8192];
	char keys[8192];
	char data[8192];
	struct SfoHeader *h;
	struct SfoEntry *e;
	char *k;
	char *d;
	unsigned int align;
	unsigned int keyofs;
	unsigned int count;

	struct EntryContainer *entry;

	g_empty = 0;
	memset(g_vals, 0, sizeof(EntryContainer) * MAX_OPTIONS);
	for (i = 0; i < (sizeof(g_defaults) / sizeof(struct EntryContainer)); i++) {
		entry = find_free();
		if (entry == NULL) {
			fprintf(stderr, "Maximum options reached\n");
			return 0;
		}
		*entry = g_defaults[i];
	}

	if ((entry = find_name("TITLE_ID"))) {
		if (sfo->title_id.length() != 9) {
			fprintf(stderr, "TITLE_ID must be 9 characters long\n");
			return 1;
		}
		entry->data = sfo->title_id.to_upper().utf8().get_data();
	}

	if ((entry = find_name("VERSION"))) {
		if (sfo->version.length() != 5 || sfo->version.find_char('.') != 2) {
			entry->data = "01.00"; // Default to 01.00 if invalid version
		} else {
			entry->data = sfo->version.utf8().get_data();
		}
	}

	if ((entry = find_name("PARENTAL_LEVEL"))) {
		if (sfo->parental_level < 0 || sfo->parental_level > 11) {
			entry->value = 0;
		} else {
			entry->value = sfo->parental_level;
		}
	}

	if (sfo->title.length() > 0) {
		g_title = sfo->title.utf8().get_data();
	}

	if (g_title) {
		entry = find_name("TITLE");
		entry->data = g_title;
		entry = find_name("STITLE");
		entry->data = g_title;
	}

	entry = find_name("ATTRIBUTE2");
	entry->value = 12;

	memset(head, 0, sizeof(head));
	memset(keys, 0, sizeof(keys));
	memset(data, 0, sizeof(data));
	h = (struct SfoHeader *)head;
	e = (struct SfoEntry *)(head + sizeof(struct SfoHeader));
	k = keys;
	d = data;
	SW(&h->magic, PSF_MAGIC);
	SW(&h->version, PSF_VERSION);
	count = 0;

	for (i = 0; g_vals[i].name; i++) {
		SW(&h->count, ++count);
		SW(&e->nameofs, k - keys);
		SW(&e->dataofs, d - data);
		SW(&e->alignment, 4);
		SW(&e->type, g_vals[i].type);

		strcpy(k, g_vals[i].name);
		k += strlen(k) + 1;
		if (e->type == PSF_TYPE_VAL) {
			SW(&e->valsize, 4);
			SW(&e->totalsize, 4);
			SW((uint32_t *)d, g_vals[i].value);
			d += 4;
		} else {
			int totalsize;
			int valsize = 0;

			if (g_vals[i].data)
				valsize = strlen(g_vals[i].data) + 1;
			totalsize = (g_vals[i].value) ? (g_vals[i].value) : ((valsize + 3) & ~3);
			SW(&e->valsize, valsize);
			SW(&e->totalsize, totalsize);
			memset(d, 0, totalsize);

			if (g_vals[i].data)
				memcpy(d, g_vals[i].data, valsize);
			d += totalsize;
		}
		e++;
	}

	keyofs = (char *)e - head;
	SW(&h->keyofs, keyofs);
	align = 3 - ((unsigned int)(k - keys) & 3);
	while (align < 3) {
		k++;
		align--;
	}

	SW(&h->valofs, keyofs + (k - keys));

	String output = outDir + "/param.sfo";
	fp = fopen(output.utf8().get_data(), "wb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open filename %s\n", output.utf8().get_data());
		return 0;
	}

	fwrite(head, 1, (char *)e - head, fp);
	fwrite(keys, 1, k - keys, fp);
	fwrite(data, 1, d - data, fp);
	fclose(fp);

	return 0;
}
