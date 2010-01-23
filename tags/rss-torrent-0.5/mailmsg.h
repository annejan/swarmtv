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
 * db       : pointer to sqlite3 database
 * subject  : pointer to mail subject
 * msqtext  : Message text for mail body
 */
int sendrssmail(sqlite3 *db, const char *subject, const char *msgtxt);

/*
 * Send an email through esmtp
 * host     : host used to send message through "localhost:25"
 * from     : from address "test@foobar.com"
 * to       : to address "test2@foobar.com"
 * subject  : pointer to mail-subject text
 * message  : pointer to message inside email has to start with '\r\n' or strange things will happen
 */
int sendmail(const char *host, const char *from, const char *to, const char *subject, const char *message);
