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

#define __ISO_C_VISIBLE 2010

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>

#include <openssl/ssl.h>
#include <auth-client.h>
#include <libesmtp.h>
#include <sqlite3.h>

#include "types.h"
#include "config.h"
#include "logfile.h"

#if !defined (__GNUC__) || __GNUC__ < 2
# define __attribute__(x)
#endif
#define unused      __attribute__((unused))

const char *readlinefp_cb (void **buf, int *len, void *arg);
/* static void monitor_cb (const char *buf, int buflen, int writing, void *arg); */
static void print_recipient_status (smtp_recipient_t recipient,
    const char *mailbox, void *arg);
/* static int authinteract (auth_client_request_t request, char **result, int fields, 
    void *arg); */
/* static int tlsinteract (char *buf, int buflen, int rwflag, void *arg); */
/* static void event_cb (int event_no, void *arg, ...); */
void usage (void);
void version (void);


/*
 * predeclaration
 */
int rsstsendmail(const char *host, const char *from, const char *to, const char *subject, const char *msgtxt);

/*
 * Send email, using settings in the config database.
 * Arguments
 * db       : pointer to sqlite3 database
 * subject  : pointer to mail subject
 * msqtext  : Message text for mail body
 * returns
 * 0 on success, -1 on error
 */
int rsstsendrssmail(sqlite3 *db, const char *subject, const char *msgtxt)
{
  int  rc;
  char *smtpenable = NULL;
  char *smtpto = NULL;
  char *smtpfrom = NULL;
  char *smtphost = NULL;
  char *sendmessage = NULL;
	rsstor_handle handle;

	/*
	 * REMOVE IN FUTURE
	 */
	handle.db=db;

  /*
   * Test if mailing is enabled
   */
  rsstconfiggetproperty(&handle, CONF_SMTPENABLE, &smtpenable);
  if(smtpenable == NULL || *smtpenable != 'Y') {
    /*
     * Mailing disabled, moving on.
     */
    free(smtpenable);
    return 0;
  }

  /*
   * Get the rest of the settings
   */
  rsstconfiggetproperty(&handle, CONF_SMTPTO, &smtpto);
  rsstconfiggetproperty(&handle, CONF_SMTPFROM, &smtpfrom);
  rsstconfiggetproperty(&handle, CONF_SMTPHOST, &smtphost);

  /*
   * add '\r\n' to beginning and end of message text
   */
  sendmessage = calloc(strlen(msgtxt) + 5, 1);
  sprintf(sendmessage, "\r\n%s\r\n", msgtxt);

  /*
   * send message
   */
  rc = rsstsendmail(smtphost, smtpfrom, smtpto, subject, sendmessage);

  /*
   * cleanup 
   */
  free(smtpenable);
  free(smtpto);
  free(smtpfrom);
  free(smtphost);
  free(sendmessage);

  /*
   * return mail return code
   */
  return rc;
}

/*
 * Send an email 
 * host : smtp server to use
 * from : from address
 * to   : to address
 * subject : mail subject
 * msgtext : message text
 * returns :
 * 0 on success, else -1
 */
int rsstsendmail(const char *host, const char *from, const char *to, const char *subject, const char *msgtxt)
{
	int						 retval=0;
  smtp_session_t session;
  smtp_message_t message;
  smtp_recipient_t recipient;
  const smtp_status_t *status;

  /* This program sends only one message at a time.  Create an SMTP
     session and add a message to it. */
  auth_client_init ();
  session = smtp_create_session ();
  message = smtp_add_message (session);

  /*
   * When TLS is available use that
   */
  smtp_starttls_enable (session, Starttls_ENABLED);

  /*
   * Monitor for now
   */
  //smtp_set_monitorcb (session, monitor_cb, stdout, 1);

  /* Set the host running the SMTP server.  LibESMTP has a default port
     number of 587, however this is not widely deployed so the port
     is specified as 25 along with the default MTA host. */
  smtp_set_server (session, host ? host : "localhost:25");

#if 0
  /* Do what's needed at application level to use authentication.
  */
  authctx = auth_create_context ();
  auth_set_mechanism_flags (authctx, AUTH_PLUGIN_PLAIN, 0);
  auth_set_interact_cb (authctx, authinteract, NULL);

  /* Use our callback for X.509 certificate passwords.  If STARTTLS is
     not in use or disabled in configure, the following is harmless. */
  smtp_starttls_set_password_cb (tlsinteract, NULL);
  smtp_set_eventcb(session, event_cb, NULL);

  /* Now tell libESMTP it can use the SMTP AUTH extension.
  */
  if (!noauth)
    smtp_auth_set_context (session, authctx);
#endif

  /* 
	 * Set the reverse path for the mail envelope.  (NULL is OK)
	 */
  smtp_set_reverse_path (message, from);

  /* 
	 * RFC 2822 doesn't require recipient headers but a To: header would
   * be nice to have if not present. 
	 */ 
  smtp_set_header (message, "From", from, from);
  smtp_set_header (message, "To", to, to);
  smtp_set_header (message, "Reply-To", "None", "None");

  /* 
	 * Set the Subject: header.  For no reason, we want the supplied subject
   * to override any subject line in the message headers. 
	 */
  smtp_set_header (message, "Subject", subject);

  /*
   * Add recipient to email
   */
  recipient = smtp_add_recipient (message, to);
  //smtp_dsn_set_notify (recipient, notify);

  /*
   * Set message text for email
   */
  smtp_set_message_str(message, (char*) msgtxt);

  /* 
	 * Initiate a connection to the SMTP server and transfer the
	 * message. 
	 */
  if (!smtp_start_session (session))
  {
    char buf[128];

    rsstwritelog (LOG_ERROR, "SMTP server problem %s\n",
        smtp_strerror (smtp_errno (), buf, sizeof buf));
		retval = -1;
  }
  else
  {
    /* 
		 * Report on the success or otherwise of the mail transfer.
		 */
    status = smtp_message_transfer_status (message);
    rsstwritelog(LOG_DEBUG, "%d %s", status->code,
        (status->text != NULL) ? status->text : "\n");
    smtp_enumerate_recipients (message, print_recipient_status, NULL);
  }

  /* 
	 * Free resources consumed by the program.
	 */
  smtp_destroy_session (session);
  //auth_destroy_context (authctx);
  //fclose (fp);
  auth_client_exit ();
  //exit (0);
  return retval;
}

/*
 * Uses the mail routine to send a test mail.
 * Arguments :
 * testxt, test message to send.
 */
void rssttestmail(rsstor_handle *handle, char *testtxt)
{
	int 		rc=0;
	char 		*enable=NULL;
	sqlite3 *db=NULL;

	/*
	 * Get db pointer
	 */
	db = handle->db;

	/*
	 * Test if mail is enabled.
	 */
	rc = rsstconfiggetproperty(handle, CONF_SMTPENABLE, &enable);
	if(strcmp(enable, "Y") != 0){
		printf("Email notifications are not enabled.\n");
		printf("Please enable them by putting value 'Y' in config value '%s'.\n", CONF_SMTPENABLE);
		free(enable);
		return;
	}
	free(enable);

	/*
	 * Test mail settings.
	 */
	rc = rsstsendrssmail(db, testtxt, testtxt);
	if(rc == 0) {
		printf("Test mail sent successful!\n");
	} else {
		printf("Test mail sending failed!\n");
	}
}

/*
 * Callback to print the recipient status 
 * Argument
 * recipient	
 * mailbox		
 */
static void print_recipient_status(smtp_recipient_t recipient,
    												const char *mailbox, void *arg unused)
{
  const smtp_status_t *status;

  status = smtp_recipient_status (recipient);
  rsstwritelog(LOG_DEBUG, "%s: %d %s", mailbox, status->code, status->text);
}

/* Callback function to read the message from a file.  Since libESMTP
   does not provide callbacks which translate line endings, one must
   be provided by the application.

   The message is read a line at a time and the newlines converted
   to \r\n.  Unfortunately, RFC 822 states that bare \n and \r are
   acceptable in messages and that individually they do not constitute a
   line termination.  This requirement cannot be reconciled with storing
   messages with Unix line terminations.  RFC 2822 rescues this situation
   slightly by prohibiting lone \r and \n in messages.

   The following code cannot therefore work correctly in all situations.
   Furthermore it is very inefficient since it must search for the \n.
   */
#define BUFLEN	8192

#if 0
static void monitor_cb (const char *buf, int buflen, int writing, void *arg)
{
  FILE *fp = arg;

  if (writing == SMTP_CB_HEADERS)
  {
    fputs ("H: ", fp);
    fwrite (buf, 1, buflen, fp);
    return;
  }

  fputs (writing ? "C: " : "S: ", fp);
  fwrite (buf, 1, buflen, fp);
  if (buf[buflen - 1] != '\n')
    putc ('\n', fp);
}
#endif

#if 0
/* 
 * Callback to request user/password info.  Not thread safe. 
 */
static int authinteract (auth_client_request_t request, char **result, int fields, void *arg unused)
{
  char prompt[64];
  static char resp[512];
  char *p, *rp;
  int i, n, tty;

  /*
   * This should get automated when enabled
   * leaving this for future use 
   */

  rp = resp;
  for (i = 0; i < fields; i++)
  {
    n = snprintf (prompt, sizeof prompt, "%s%s: ", request[i].prompt,
        (request[i].flags & AUTH_CLEARTEXT) ? " (not encrypted)"
        : "");
    if (request[i].flags & AUTH_PASS)
      result[i] = getpass (prompt);
    else
    {
      tty = open ("/dev/tty", O_RDWR);
      write (tty, prompt, n);
      n = read (tty, rp, sizeof resp - (rp - resp));
      close (tty);
      p = rp + n;
      while (isspace (p[-1]))
        p--;
      *p++ = '\0';
      result[i] = rp;
      rp = p;
    }
  }
  return 1;
}
#endif

#if 0
/*
 * Arguments
 * buf
 * buflen
 * rwflag
 * arg
 * return
 */
static int tlsinteract (char *buf, int buflen, int rwflag unused, void *arg unused)
{
  char *pw;
  int len;

  pw = getpass ("certificate password");
  len = strlen (pw);
  if (len + 1 > buflen)
    return 0;
  strcpy (buf, pw);
  return len;
}
#endif


#if 0
/*
 * handles peer certificate
 */
static int handle_invalid_peer_certificate(long vfy_result)
{
  const char *k ="rare error";
  switch(vfy_result) {
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
      k="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT"; break;
    case X509_V_ERR_UNABLE_TO_GET_CRL:
      k="X509_V_ERR_UNABLE_TO_GET_CRL"; break;
    case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
      k="X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE"; break;
    case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
      k="X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE"; break;
    case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
      k="X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY"; break;
    case X509_V_ERR_CERT_SIGNATURE_FAILURE:
      k="X509_V_ERR_CERT_SIGNATURE_FAILURE"; break;
    case X509_V_ERR_CRL_SIGNATURE_FAILURE:
      k="X509_V_ERR_CRL_SIGNATURE_FAILURE"; break;
    case X509_V_ERR_CERT_NOT_YET_VALID:
      k="X509_V_ERR_CERT_NOT_YET_VALID"; break;
    case X509_V_ERR_CERT_HAS_EXPIRED:
      k="X509_V_ERR_CERT_HAS_EXPIRED"; break;
    case X509_V_ERR_CRL_NOT_YET_VALID:
      k="X509_V_ERR_CRL_NOT_YET_VALID"; break;
    case X509_V_ERR_CRL_HAS_EXPIRED:
      k="X509_V_ERR_CRL_HAS_EXPIRED"; break;
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
      k="X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD"; break;
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
      k="X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD"; break;
    case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
      k="X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD"; break;
    case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
      k="X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD"; break;
    case X509_V_ERR_OUT_OF_MEM:
      k="X509_V_ERR_OUT_OF_MEM"; break;
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
      k="X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT"; break;
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
      k="X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN"; break;
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
      k="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY"; break;
    case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
      k="X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE"; break;
    case X509_V_ERR_CERT_CHAIN_TOO_LONG:
      k="X509_V_ERR_CERT_CHAIN_TOO_LONG"; break;
    case X509_V_ERR_CERT_REVOKED:
      k="X509_V_ERR_CERT_REVOKED"; break;
    case X509_V_ERR_INVALID_CA:
      k="X509_V_ERR_INVALID_CA"; break;
    case X509_V_ERR_PATH_LENGTH_EXCEEDED:
      k="X509_V_ERR_PATH_LENGTH_EXCEEDED"; break;
    case X509_V_ERR_INVALID_PURPOSE:
      k="X509_V_ERR_INVALID_PURPOSE"; break;
    case X509_V_ERR_CERT_UNTRUSTED:
      k="X509_V_ERR_CERT_UNTRUSTED"; break;
    case X509_V_ERR_CERT_REJECTED:
      k="X509_V_ERR_CERT_REJECTED"; break;
  }
  rsstwritelog(LOG_DEBUG, "SMTP_EV_INVALID_PEER_CERTIFICATE: %ld: %s\n", vfy_result, k);
  return 1; /* Accept the problem */
}
#endif

#if 0
static void event_cb(int event_no, void *arg,...)
{
  va_list alist;
  int *ok;

  va_start(alist, arg);
  switch(event_no) {
    case SMTP_EV_CONNECT: 
    case SMTP_EV_MAILSTATUS:
    case SMTP_EV_RCPTSTATUS:
    case SMTP_EV_MESSAGEDATA:
    case SMTP_EV_MESSAGESENT:
    case SMTP_EV_DISCONNECT: break;
    case SMTP_EV_WEAK_CIPHER: {
                                int bits;
                                bits = va_arg(alist, long); ok = va_arg(alist, int*);
                                rsstwritelog(LOG_DEBUG, "SMTP_EV_WEAK_CIPHER, bits=%d - accepted.\n", bits);
                                *ok = 1; break;
                              }
    case SMTP_EV_STARTTLS_OK:
                             rsstwritelog(LOG_DEBUG, "SMTP_EV_STARTTLS_OK - TLS started here."); 
                             break;
    case SMTP_EV_INVALID_PEER_CERTIFICATE: {
                                             long vfy_result;
                                             vfy_result = va_arg(alist, long); ok = va_arg(alist, int*);
                                             *ok = handle_invalid_peer_certificate(vfy_result);
                                             break;
                                           }
    case SMTP_EV_NO_PEER_CERTIFICATE: {
                                        ok = va_arg(alist, int*); 
                                        rsstwritelog(LOG_DEBUG, "SMTP_EV_NO_PEER_CERTIFICATE - accepted.");
                                        *ok = 1; break;
                                      }
    case SMTP_EV_WRONG_PEER_CERTIFICATE: {
                                           ok = va_arg(alist, int*);
                                           rsstwritelog(LOG_DEBUG, "SMTP_EV_WRONG_PEER_CERTIFICATE - accepted.");
                                           *ok = 1; break;
                                         }
    case SMTP_EV_NO_CLIENT_CERTIFICATE: {
                                          ok = va_arg(alist, int*); 
                                          rsstwritelog(LOG_DEBUG, "SMTP_EV_NO_CLIENT_CERTIFICATE - accepted.");
                                          *ok = 1; break;
                                        }
    default:
                                        rsstwritelog(LOG_DEBUG, "Got event: %d - ignored.\n", event_no);
  }
  va_end(alist);
}
#endif

