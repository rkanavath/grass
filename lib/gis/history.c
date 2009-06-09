/*!
 * \file gis/history.c
 *
 * \brief GIS Library - History management
 *
 * (C) 2001-2009 GRASS Development Team
 *
 * This program is free software under the GNU General Public License 
 * (>=v2). Read the file COPYING that comes with GRASS for details.
 *
 * \author Original author CERL
 */

#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>

static void print_history_error(const char *, const char *, FILE *);

/*!
 * \brief Read raster history file
 *
 * This routine reads the history file for the raster map <i>name</i>
 * in <i>mapset</i> into the <i>hist</i> structure.
 *
 * A diagnostic message is printed and -1 is returned if there is an
 * error reading the history file. Otherwise, 0 is returned.
 *
 * \param name map name
 * \param mapset mapset name
 * \param hist pointer to History structure which holds history info
 *
 * \return -1 on error
 * \return 0 on success
 */
int G_read_history(const char *name, const char *mapset, struct History *hist)
{
    FILE *fd;

    G_zero(hist, sizeof(struct History));
    fd = G_fopen_old("hist", name, mapset);
    if (!fd) {
	print_history_error(name, mapset, fd);
	return -1;
    }

    if (!G_getl(hist->mapid, sizeof(hist->mapid), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->mapid);

    if (!G_getl(hist->title, sizeof(hist->title), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->title);

    if (!G_getl(hist->mapset, sizeof(hist->mapset), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->mapset);

    if (!G_getl(hist->creator, sizeof(hist->creator), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->creator);

    if (!G_getl(hist->maptype, sizeof(hist->maptype), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->maptype);

    if (!G_getl(hist->datsrc_1, sizeof(hist->datsrc_1), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->datsrc_1);

    if (!G_getl(hist->datsrc_2, sizeof(hist->datsrc_2), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->datsrc_2);

    if (!G_getl(hist->keywrd, sizeof(hist->keywrd), fd)) {
	print_history_error(name, mapset, fd);
	return -1;
    }
    G_ascii_check(hist->keywrd);

    hist->edlinecnt = 0;
    while ((hist->edlinecnt < MAXEDLINES) &&
	   (G_getl
	    (hist->edhist[hist->edlinecnt], sizeof(hist->edhist[0]), fd))) {
	G_ascii_check(hist->edhist[hist->edlinecnt]);
	hist->edlinecnt++;
    }

    fclose(fd);

    return 0;
}

void print_history_error(const char *name, const char *mapset, FILE *fd)
{
    if (fd != NULL)
	fclose(fd);
    
    if (mapset) {
	G_warning(_("Unable to get history information for <%s@%s>"),
		  name, mapset);
    }
    else { /* write */
	G_warning(_("Unable to write history information for <%s>"), name);
    }
}

/*!
 * \brief Write raster history file
 *
 * This routine writes the history file for the raster map
 * <i>name</i> in the current mapset from the <i>hist</i> structure.
 *
 * A diagnostic message is printed and -1 is returned if there is an
 * error writing the history file. Otherwise, 0 is returned.
 *
 * <b>Note:</b> The <i>hist</i> structure should first be initialized
 * using G_short_history().
 *
 * \param name map name
 * \param[out] hist pointer to History structure which holds history info
 *
 * \return -1 on error
 * \return 0 on success
 */
int G_write_history(const char *name, struct History *hist)
{
    FILE *fd;
    int i;

    fd = G_fopen_new("hist", name);
    if (!fd) {
	print_history_error(name, NULL, fd);
	return -1;
    }
    
    fprintf(fd, "%s\n", hist->mapid);
    fprintf(fd, "%s\n", hist->title);
    fprintf(fd, "%s\n", hist->mapset);
    fprintf(fd, "%s\n", hist->creator);
    fprintf(fd, "%s\n", hist->maptype);
    fprintf(fd, "%s\n", hist->datsrc_1);
    fprintf(fd, "%s\n", hist->datsrc_2);
    fprintf(fd, "%s\n", hist->keywrd);

    for (i = 0; i < hist->edlinecnt; i++)
	fprintf(fd, "%s\n", hist->edhist[i]);

    fclose(fd);
    
    return 0;
}

/*!
 * \brief Initialize history structure
 *
 * This routine initializes the <i>hist</i> structure, recording the
 * date, user, module name and the raster map <i>name</i>
 * structure. The <i>type</i> is an anachronism from earlier versions
 * of GRASS and should be specified as "raster".
 *
 * <b>Note:</b> This routine only initializes the data structure. It
 * does not write the history file.
 *
 * \param name map name
 * \param type map type
 * \param hist pointer to History structure which holds history info
 */
void G_short_history(const char *name, const char *type, struct History *hist)
{
    strncpy(hist->mapid, G_date(), RECORD_LEN);
    strncpy(hist->title, name, RECORD_LEN);
    strncpy(hist->mapset, G_mapset(), RECORD_LEN);
    strncpy(hist->creator, G_whoami(), RECORD_LEN);
    strncpy(hist->maptype, type, RECORD_LEN);

    sprintf(hist->keywrd, _("generated by %s"), G_program_name());
    strcpy(hist->datsrc_1, "");
    strcpy(hist->datsrc_2, "");
    hist->edlinecnt = 0;
}

/*!
 * \brief Save command line to raster history structure
 *
 * This routine takes an existing (run G_short_history first() history
 * structure and adds the command line to the end of the comments
 * array, as cleaned & expanded by the parser.
 *
 * History file is limited to [80]x[50], as defined in include/gis.h
 *
 *  - First version had for loops of [i][j] character assignments and ending
 *    nulls, but using the string libraries is cleaner and less bug prone.
 *  - Second version had white space detection, intelligent wrapping, and
 *    indentation of continued lines, but this proved a pain in the neck for 
 *    things like r.patch which can have long strings without any
 *    parser-acceptable breaks.
 *  - This is MK-III, simplified, but that's good: it's cut & paste-able.
 *
 * Note: use G_write_history() to write the structure.
 *
 * Sample Usage:
 * \code
 *   struct History history;
 *   G_short_history(rasterfile, "raster", &history);
 *   G_command_history(&history);
 *   G_write_history(rasterfile, &history);
 * \endcode
 *
 * \param hist pointer to History structure which holds history info
 *
 * \return 0 on success
 * \return 1 on failure (history file full, no change)
 * \return 2 on failure (history file full, added as much as we could)
 */
int G_command_history(struct History *hist)
{
    int j, cmdlen;
    char *cmdlin;

    cmdlin = G_recreate_command();
    cmdlen = strlen(cmdlin);

    if (hist->edlinecnt > MAXEDLINES - 2) {
	G_warning(_("Not enough room in history file to record command line"));
	return 1;
    }

    if (hist->edlinecnt > 0) {	/* add a blank line if preceding history exists */
	strcpy(hist->edhist[hist->edlinecnt], "");
	hist->edlinecnt++;
    }

    if (cmdlen < 70) {		/* ie if it will fit on a single line */
	sprintf(hist->edhist[hist->edlinecnt], G_recreate_command());
	hist->edlinecnt++;
    }
    else {			/* multi-line required */
	j = 0;			/* j is the current position in the command line string */
	while ((cmdlen - j) > 70) {
	    strncpy(hist->edhist[hist->edlinecnt], &cmdlin[j], 68);
	    hist->edhist[hist->edlinecnt][68] = '\0';
	    strcat(hist->edhist[hist->edlinecnt], "\\");
	    j += 68;
	    hist->edlinecnt++;
	    if (hist->edlinecnt > MAXEDLINES - 2) {
		G_warning(_("Not enough room in history file for command line (truncated)"));
		return 2;
	    }
	}
	if ((cmdlen - j) > 0) {	/* ie anything left */
	    strcpy(hist->edhist[hist->edlinecnt], &cmdlin[j]);
	    hist->edlinecnt++;
	}
    }
    return 0;
}
