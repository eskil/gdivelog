/*
    This file is part of gdivelog.

    gdivelog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    gdivelog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with gdivelog; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef IMPORT_DIVE_H
#define IMPORT_DIVE_H

gint import_profile (gint dive_id, GArray * profile);
gint import_dive (gchar * dive_datetime, glong dive_duration,
		    gdouble dive_maxdepth, gdouble dive_mintemp,
		    gdouble dive_maxtemp, gchar * dive_notes,
		    gdouble dive_visibility, gdouble dive_weight,
		    GArray * sites, GArray * buddies, GArray * equipment,
		    GArray * types, GArray * tanks, GArray * profile,
		    gboolean use_template);

#endif /* IMPORT_DIVE_H */
