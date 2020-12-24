/* $OpenBSD$ */

/*
 * Copyright (c) 2008 Nicholas Marriott <nicholas.marriott@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#if defined(HAVE_CURSES_H)
#include <curses.h>
#elif defined(HAVE_NCURSES_H)
#include <ncurses.h>
#endif
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <term.h>

#include "tmux.h"
#include "tty-term.h"

static char	*tty_term_strip(const char *);

struct tty_terms tty_terms = LIST_HEAD_INITIALIZER(tty_terms);

static char *
tty_term_strip(const char *s)
{
	const char     *ptr;
	static char	buf[8192];
	size_t		len;

	/* Ignore strings with no padding. */
	if (strchr(s, '$') == NULL)
		return (xstrdup(s));

	len = 0;
	for (ptr = s; *ptr != '\0'; ptr++) {
		if (*ptr == '$' && *(ptr + 1) == '<') {
			while (*ptr != '\0' && *ptr != '>')
				ptr++;
			if (*ptr == '>')
				ptr++;
			if (*ptr == '\0')
				break;
		}

		buf[len++] = *ptr;
		if (len == (sizeof buf) - 1)
			break;
	}
	buf[len] = '\0';

	return (xstrdup(buf));
}

static char *
tty_term_override_next(const char *s, size_t *offset)
{
	static char	value[8192];
	size_t		n = 0, at = *offset;

	if (s[at] == '\0')
		return (NULL);

	while (s[at] != '\0') {
		if (s[at] == ':') {
			if (s[at + 1] == ':') {
				value[n++] = ':';
				at += 2;
			} else
				break;
		} else {
			value[n++] = s[at];
			at++;
		}
		if (n == (sizeof value) - 1)
			return (NULL);
	}
	if (s[at] != '\0')
		*offset = at + 1;
	else
		*offset = at;
	value[n] = '\0';
	return (value);
}

void
tty_term_apply(struct tty_term *term, const char *capabilities, int quiet)
{
	const struct tty_term_code_entry	*ent;
	struct tty_code				*code;
	size_t                                   offset = 0;
	char					*cp, *value, *s;
	const char				*errstr, *name = term->name;
	u_int					 i;
	int					 n, remove;

	while ((s = tty_term_override_next(capabilities, &offset)) != NULL) {
		if (*s == '\0')
			continue;
		value = NULL;

		remove = 0;
		if ((cp = strchr(s, '=')) != NULL) {
			*cp++ = '\0';
			value = xstrdup(cp);
			if (strunvis(value, cp) == -1) {
				free(value);
				value = xstrdup(cp);
			}
		} else if (s[strlen(s) - 1] == '@') {
			s[strlen(s) - 1] = '\0';
			remove = 1;
		} else
			value = xstrdup("");

		if (!quiet) {
			if (remove)
				log_debug("%s override: %s@", name, s);
			else if (*value == '\0')
				log_debug("%s override: %s", name, s);
			else
				log_debug("%s override: %s=%s", name, s, value);
		}

		for (i = 0; i < tty_term_ncodes(); i++) {
			ent = &tty_term_codes[i];
			if (strcmp(s, ent->name) != 0)
				continue;
			code = &term->codes[i];

			if (remove) {
				code->type = TTYCODE_NONE;
				continue;
			}
			switch (ent->type) {
			case TTYCODE_NONE:
				break;
			case TTYCODE_STRING:
				if (code->type == TTYCODE_STRING)
					free(code->value.string);
				code->value.string = xstrdup(value);
				code->type = ent->type;
				break;
			case TTYCODE_NUMBER:
				n = strtonum(value, 0, INT_MAX, &errstr);
				if (errstr != NULL)
					break;
				code->value.number = n;
				code->type = ent->type;
				break;
			case TTYCODE_FLAG:
				code->value.flag = 1;
				code->type = ent->type;
				break;
			}
		}

		free(value);
	}
}

void
tty_term_apply_overrides(struct tty_term *term)
{
	struct options_entry		*o;
	struct options_array_item	*a;
	union options_value		*ov;
	const char			*s;
	size_t				 offset;
	char				*first;

	o = options_get_only(global_options, "terminal-overrides");
	a = options_array_first(o);
	while (a != NULL) {
		ov = options_array_item_value(a);
		s = ov->string;

		offset = 0;
		first = tty_term_override_next(s, &offset);
		if (first != NULL && fnmatch(first, term->name, 0) == 0)
			tty_term_apply(term, s + offset, 0);
		a = options_array_next(a);
	}
}

struct tty_term *
tty_term_create(struct tty *tty, char *name, int *feat, int fd, char **cause)
{
	struct tty_term				*term;
	const struct tty_term_code_entry	*ent;
	struct tty_code				*code;
	struct options_entry			*o;
	struct options_array_item		*a;
	union options_value			*ov;
	u_int					 i;
	int		 			 n, error;
	const char				*s, *acs;
	size_t					 offset;
	char					*first;

	log_debug("adding term %s", name);

	term = (struct tty_term *) xcalloc(1, sizeof *term);
	term->tty = tty;
	term->name = xstrdup(name);
	term->codes = (struct tty_code *) xcalloc(tty_term_ncodes(), sizeof *term->codes);
	LIST_INSERT_HEAD(&tty_terms, term, entry);

	/* Set up curses terminal. */
	if (setupterm(name, fd, &error) != OK) {
		switch (error) {
		case 1:
			xasprintf(cause, "can't use hardcopy terminal: %s",
			    name);
			break;
		case 0:
			xasprintf(cause, "missing or unsuitable terminal: %s",
			    name);
			break;
		case -1:
			xasprintf(cause, "can't find terminfo database");
			break;
		default:
			xasprintf(cause, "unknown error");
			break;
		}
		goto error;
	}

	/* Fill in codes. */
	for (i = 0; i < tty_term_ncodes(); i++) {
		ent = &tty_term_codes[i];

		code = &term->codes[i];
		code->type = TTYCODE_NONE;
		switch (ent->type) {
		case TTYCODE_NONE:
			break;
		case TTYCODE_STRING:
			s = tigetstr((char *) ent->name);
			if (s == NULL || s == (char *) -1)
				break;
			code->type = TTYCODE_STRING;
			code->value.string = tty_term_strip(s);
			break;
		case TTYCODE_NUMBER:
			n = tigetnum((char *) ent->name);
			if (n == -1 || n == -2)
				break;
			code->type = TTYCODE_NUMBER;
			code->value.number = n;
			break;
		case TTYCODE_FLAG:
			n = tigetflag((char *) ent->name);
			if (n == -1)
				break;
			code->type = TTYCODE_FLAG;
			code->value.flag = n;
			break;
		}
	}

	/* Apply terminal features. */
	o = options_get_only(global_options, "terminal-features");
	a = options_array_first(o);
	while (a != NULL) {
		ov = options_array_item_value(a);
		s = ov->string;

		offset = 0;
		first = tty_term_override_next(s, &offset);
		if (first != NULL && fnmatch(first, term->name, 0) == 0)
			tty_add_features(feat, s + offset, ":");
		a = options_array_next(a);
	}

	/* Delete curses data. */
#if !defined(NCURSES_VERSION_MAJOR) || NCURSES_VERSION_MAJOR > 5 || \
    (NCURSES_VERSION_MAJOR == 5 && NCURSES_VERSION_MINOR > 6)
	del_curterm(cur_term);
#endif

	/* Apply overrides so any capabilities used for features are changed. */
	tty_term_apply_overrides(term);

	/* These are always required. */
	if (!tty_term_has(term, TTYC_CLEAR)) {
		xasprintf(cause, "terminal does not support clear");
		goto error;
	}
	if (!tty_term_has(term, TTYC_CUP)) {
		xasprintf(cause, "terminal does not support cup");
		goto error;
	}

	/*
	 * If TERM has XT or clear starts with CSI then it is safe to assume
	 * the terminal is derived from the VT100. This controls whether device
	 * attributes requests are sent to get more information.
	 *
	 * This is a bit of a hack but there aren't that many alternatives.
	 * Worst case tmux will just fall back to using whatever terminfo(5)
	 * says without trying to correct anything that is missing.
	 *
	 * Also add few features that VT100-like terminals should either
	 * support or safely ignore.
	 */
	s = tty_term_string(term, TTYC_CLEAR);
	if (tty_term_flag(term, TTYC_XT) || strncmp(s, "\033[", 2) == 0) {
		term->flags |= TERM_VT100LIKE;
		tty_add_features(feat, "bpaste,focus,title", ",");
	}

	/* Add RGB feature if terminal has RGB colours. */
	if ((tty_term_flag(term, TTYC_TC) || tty_term_has(term, TTYC_RGB)) &&
	    (!tty_term_has(term, TTYC_SETRGBF) ||
	    !tty_term_has(term, TTYC_SETRGBB)))
		tty_add_features(feat, "RGB", ",");
	if (tty_term_has(term, TTYC_SETRGBF) &&
	    tty_term_has(term, TTYC_SETRGBB))
		term->flags |= TERM_RGBCOLOURS;

	/* Apply the features and overrides again. */
	tty_apply_features(term, *feat);
	tty_term_apply_overrides(term);

	/*
	 * Terminals without am (auto right margin) wrap at at $COLUMNS - 1
	 * rather than $COLUMNS (the cursor can never be beyond $COLUMNS - 1).
	 *
	 * Terminals without xenl (eat newline glitch) ignore a newline beyond
	 * the right edge of the terminal, but tmux doesn't care about this -
	 * it always uses absolute only moves the cursor with a newline when
	 * also sending a linefeed.
	 *
	 * This is irritating, most notably because it is painful to write to
	 * the very bottom-right of the screen without scrolling.
	 *
	 * Flag the terminal here and apply some workarounds in other places to
	 * do the best possible.
	 */
	if (!tty_term_flag(term, TTYC_AM))
		term->flags |= TERM_NOAM;

	/* Generate ACS table. If none is present, use nearest ASCII. */
	memset(term->acs, 0, sizeof term->acs);
	if (tty_term_has(term, TTYC_ACSC))
		acs = tty_term_string(term, TTYC_ACSC);
	else
		acs = "a#j+k+l+m+n+o-p-q-r-s-t+u+v+w+x|y<z>~.";
	for (; acs[0] != '\0' && acs[1] != '\0'; acs += 2)
		term->acs[(u_char) acs[0]][0] = acs[1];

	/* Log the capabilities. */
	for (i = 0; i < tty_term_ncodes(); i++)
		log_debug("%s%s", name, tty_term_describe(term, (enum tty_code_code)i));

	return (term);

error:
	tty_term_free(term);
	return (NULL);
}

void
tty_term_free(struct tty_term *term)
{
	u_int	i;

	log_debug("removing term %s", term->name);

	for (i = 0; i < tty_term_ncodes(); i++) {
		if (term->codes[i].type == TTYCODE_STRING)
			free(term->codes[i].value.string);
	}
	free(term->codes);

	LIST_REMOVE(term, entry);
	free(term->name);
	free(term);
}

int
tty_term_has(struct tty_term *term, enum tty_code_code code)
{
	return (term->codes[code].type != TTYCODE_NONE);
}

const char *
tty_term_string(struct tty_term *term, enum tty_code_code code)
{
	if (!tty_term_has(term, code))
		return ("");
	if (term->codes[code].type != TTYCODE_STRING)
		fatalx("not a string: %d", code);
	return (term->codes[code].value.string);
}

const char *
tty_term_string1(struct tty_term *term, enum tty_code_code code, int a)
{
	return (tparm((char *) tty_term_string(term, code), a, 0, 0, 0, 0, 0, 0, 0, 0));
}

const char *
tty_term_string2(struct tty_term *term, enum tty_code_code code, int a, int b)
{
	return (tparm((char *) tty_term_string(term, code), a, b, 0, 0, 0, 0, 0, 0, 0));
}

const char *
tty_term_string3(struct tty_term *term, enum tty_code_code code, int a, int b,
    int c)
{
	return (tparm((char *) tty_term_string(term, code), a, b, c, 0, 0, 0, 0, 0, 0));
}

const char *
tty_term_ptr1(struct tty_term *term, enum tty_code_code code, const void *a)
{
	return (tparm((char *) tty_term_string(term, code), (long)a, 0, 0, 0, 0, 0, 0, 0, 0));
}

const char *
tty_term_ptr2(struct tty_term *term, enum tty_code_code code, const void *a,
    const void *b)
{
	return (tparm((char *) tty_term_string(term, code), (long)a, (long)b, 0, 0, 0, 0, 0, 0, 0));
}

int
tty_term_number(struct tty_term *term, enum tty_code_code code)
{
	if (!tty_term_has(term, code))
		return (0);
	if (term->codes[code].type != TTYCODE_NUMBER)
		fatalx("not a number: %d", code);
	return (term->codes[code].value.number);
}

int
tty_term_flag(struct tty_term *term, enum tty_code_code code)
{
	if (!tty_term_has(term, code))
		return (0);
	if (term->codes[code].type != TTYCODE_FLAG)
		fatalx("not a flag: %d", code);
	return (term->codes[code].value.flag);
}

const char *
tty_term_describe(struct tty_term *term, enum tty_code_code code)
{
	static char	 s[256];
	char		 out[128];

	switch (term->codes[code].type) {
	case TTYCODE_NONE:
		xsnprintf(s, sizeof s, "%4u: %s: [missing]",
		    code, tty_term_codes[code].name);
		break;
	case TTYCODE_STRING:
		strnvis(out, term->codes[code].value.string, sizeof out,
		    VIS_OCTAL|VIS_CSTYLE|VIS_TAB|VIS_NL);
		xsnprintf(s, sizeof s, "%4u: %s: (string) %s",
		    code, tty_term_codes[code].name,
		    out);
		break;
	case TTYCODE_NUMBER:
		xsnprintf(s, sizeof s, "%4u: %s: (number) %d",
		    code, tty_term_codes[code].name,
		    term->codes[code].value.number);
		break;
	case TTYCODE_FLAG:
		xsnprintf(s, sizeof s, "%4u: %s: (flag) %s",
		    code, tty_term_codes[code].name,
		    term->codes[code].value.flag ? "true" : "false");
		break;
	}
	return (s);
}
