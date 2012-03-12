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

#ifndef DIVE_TANK_DB_H
#define DIVE_TANK_DB_H

gboolean dive_tank_db_dive_tanklist_load_store (gint dive_id,
						gpointer load_list_data,
						gpointer
						dive_tanklist_callback);
gint dive_tank_db_insert (gint dive_id, gint tank_id,
			    gdouble dive_tank_avg_depth, gdouble dive_tank_O2,
			    gdouble dive_tank_He, glong dive_tank_stime,
			    glong dive_tank_etime,
			    gdouble dive_tank_spressure,
			    gdouble dive_tank_epressure);
gboolean dive_tank_db_update (gint dive_tank_id, gint tank_id,
			      gdouble dive_tank_avg_depth,
			      gdouble dive_tank_O2, gdouble dive_tank_He,
			      glong dive_tank_stime, glong dive_tank_etime,
			      gdouble dive_tank_spressure,
			      gdouble dive_tank_epressure);
gboolean dive_tank_db_delete (gint dive_tank_id);

#endif /* DIVE_TANK_DB_H */
