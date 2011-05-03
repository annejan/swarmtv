/*
 *  A libESMTP Example Application.
 *  Copyright (C) 2001,2002  Brian Stafford <brian@stafford.uklinux.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Send email containing information about torrent
 * @arguments 
 * db	database pointers
 * downed 
 * @return
 *
 */
void rsstsendemail(rsstor_handle *handle, downloaded_struct *downed);

/*
 * Uses the mail routine to send a testmail.
 * Arguments :
 * testxt, test message to send.
 */
void rssttestmail(rsstor_handle *handle, char *testtxt);

