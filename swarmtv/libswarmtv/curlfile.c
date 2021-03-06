/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: getinmemory.c,v 1.13 2008-09-06 04:28:45 yangtse Exp $
 *
 * Example source code to show how the callback function can be used to
 * download data into a chunk of memory instead of storing it in a file.
 *
 * This exact source code has not been verified to work.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sqlite3.h>

#include <curl/curl.h>
//#include <curl/types.h>
#include <curl/easy.h>

#include "types.h"
#include "curlfile.h"
#include "regexp.h"
#include "logfile.h"
#include "config.h"

#ifdef __MINGW32__
#define strtok_r( _s, _sep, _lasts ) \
( *(_lasts) = strtok( (_s), (_sep) ) )
#endif /* !__MINGW32__ */

/* Size of array to store regexp match pointers in */
#define REGEXPSIZE      40      
/* End of line Character sequence in http header */
#define HEADEREOL       "\r\n"  
/* The max amount of redirects  to follow */
#define MAX_REDIR       3       
/* Time to wait before connection timeout on curl download */
#define CONNECT_TIMEOUT 20  
#define CURL_TIMEOUT 60  

/*
 * Reallocate 
 */
static void *myrealloc(void *ptr, size_t size);

/*
 * Reallocate bigger memory when the file is big.
 */
static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocating
   *      NULL pointers, so we take care of it here */ 
  if(ptr)
    return realloc(ptr, size);
  else
    return calloc(size, 1);
}

/*
 * Callback function to download content to memory
 */
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)data;

  mem->memory = myrealloc((void*) mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

/*
 * Callback function to download header to memory
 */
static size_t WriteHeaderCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)data;

  mem->header = myrealloc((void*) mem->header, mem->headersize + realsize + 1);
  if (mem->header) {
    memcpy(&(mem->header[mem->headersize]), ptr, realsize);
    mem->headersize += realsize;
    mem->header[mem->headersize] = 0;
  }
  return realsize;
}


/*
 * Set proxy settings to Curl instance
 */
static void rsstcurlproxy(rsstor_handle *handle, CURL *curl_handle)
{
  char *enabled=NULL;
  char *url=NULL;
  char *userpass=NULL;
  char *type=NULL;

  /*
   * Get variables for proxy
   * CONF_PROXYENABLE  "proxy_enable"
   * CONF_PROXYURL     "proxy_url"
   * CONF_PROXYUSEPASS "proxy_userpass"
   */
  rsstconfiggetproperty(handle, CONF_PROXYENABLE, &enabled);
  rsstconfiggetproperty(handle, CONF_PROXYURL, &url);
  rsstconfiggetproperty(handle, CONF_PROXYUSEPASS, &userpass);
  rsstconfiggetproperty(handle, CONF_PROXYTYPE, &type);

  /*
   * See if proxy support is set
   */
  if(strcasecmp(enabled, "y") == 0){
    curl_easy_setopt(curl_handle, CURLOPT_PROXY, url); 
    curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, userpass); 

    /*
     * Translate proxy type to curl proxy type
     */
    if(strcasecmp(type, RSST_PROXY_SOCKS4) == 0){
      curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4); 
    }
    if(strcasecmp(type, RSST_PROXY_SOCKS5) == 0){
      curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5); 
    }
  }

  /*
   * Clean up
   */
  free(enabled);
  free(url);
  free(userpass);
  free(type);
}


/*
 * Initialize the CURL handle.
 */
static void rsstcurlinit(rsstor_handle *handle, CURL **curl_handle)
{
  /* Initialize the curl session */ 
  *curl_handle = curl_easy_init();
  if(*curl_handle == NULL) {
    return;
  }

  /* Set max redirects */
  curl_easy_setopt(*curl_handle, CURLOPT_MAXREDIRS, MAX_REDIR);

  /* Set connection timeout on 1 minute */
  curl_easy_setopt(*curl_handle, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
  curl_easy_setopt(*curl_handle, CURLOPT_TIMEOUT, CURL_TIMEOUT);

  /* some servers don't like requests that are made without a user-agent
   *      field, so we provide one */ 
  curl_easy_setopt(*curl_handle, CURLOPT_USERAGENT, "SwarmTV");

  /* Set generate error string */
  curl_easy_setopt(*curl_handle, CURLOPT_ENCODING, "");

  /* Make curl Follow redirects */
  curl_easy_setopt(*curl_handle, CURLOPT_FOLLOWLOCATION, 1);

  /*
   * Handle proxy settings here
   */
  rsstcurlproxy(handle, *curl_handle);
}

/*
 * Download URL and put the resulting data in chunk.
 */
int rsstdownloadtobuffer(rsstor_handle *handle, char *url, MemoryStruct *chunk)
{
  CURL *curl_handle=NULL;
  int 	rc=0;
  int   retval=0;
	char *realurl=NULL;
	char *cleanurl=NULL;
	char *userpass=NULL;
  char 	errorBuffer[CURL_ERROR_SIZE];

  chunk->memory=NULL; /* we expect realloc(NULL, size) to work */ 
  chunk->size = 0;    /* no data at this point */ 
  chunk->header=NULL; /* header is empty */
  chunk->headersize=0;  /* headersize is 0 */

	/*
	 * Determine if the URL holds user names and passwords.
	 */
	rc = rsstgetusernamepassword(url, &cleanurl, &userpass);
	switch(rc) {
		case 0:
			/*
			 * No password
			 */
			realurl=calloc(1, strlen(url)+1);
			strncpy(realurl, url, strlen(url));
			rsstwritelog(LOG_DEBUG, "Realurl			: %s\n", realurl);
			break;
		case 1:
			/*
			 * Password found 
			 */
			rsstwritelog(LOG_DEBUG, "Passwd_url			: %s\n", url);
			rsstwritelog(LOG_DEBUG, "Userpass				: %s\n", userpass);
			rsstwritelog(LOG_DEBUG, "Cleanurl				: %s\n", cleanurl);

		default:
			/*
			 * Any other case
			 */
			realurl=cleanurl;
      cleanurl=NULL;
      retval=-1;
      break;
  }

  /* Generic initialization */
  if(retval == 0) {
    rsstcurlinit(handle, &curl_handle);
  }

  if(curl_handle == NULL) {
    retval = -1;
  } else {
    /* specify URL to get */ 
    curl_easy_setopt(curl_handle, CURLOPT_URL, realurl);

    /* send all data to this function  */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* send headers to this function */
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);

    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);

    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)chunk);

    /* Set generate error string */
    curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);

    /* If a user name and password is set add it to the options */
    if(userpass != NULL) {
      curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
      curl_easy_setopt(curl_handle, CURLOPT_USERPWD, userpass);
    }

    /* get it! */ 
    rc = curl_easy_perform(curl_handle);

    /* cleanup curl stuff */ 
    curl_easy_cleanup(curl_handle);
  }

  /*
   * Now, our chunk.memory points to a memory block that is chunk.size
   * bytes big and contains the remote file.
   *
   * Do something nice with it!
   *
   * You should be aware of the fact that at this point we might have an
   * allocated data block, and nothing has yet deallocated that data. So when
   * you're done with it, you should free() it as a nice application.
   */ 

  /*
   * Cleanup
   */
  free(realurl);
  free(userpass);

  return retval;
}

/*
 * Download callback to file.
 */
static size_t curlfwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct FtpFile *out=(struct FtpFile *)stream;
  if(out && !out->stream) {
    /* open file for writing */ 
    out->stream=fopen(out->filename, "wb");
    if(!out->stream)
      return -1; /* failure, can't open file to write */ 
  }
  return fwrite(buffer, size, nmemb, out->stream);
}

/*
 * Download to file.
 * arguments 
 * url	URL to download from
 * path	Path to store downloaded content in.
 * return
 * return 0 on success and -1 on failure.
 */
int rsstdownloadtofile(rsstor_handle *handle, char *url, char *path)
{
  CURL *curl;
  CURLcode res;
  struct FtpFile ftpfile;
  int rc = 0;

  /*
   * Initialize ftp file struct
   */
  ftpfile.filename = path;
  ftpfile.stream = NULL;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  //curl = curl_easy_init();
  rsstcurlinit(handle, &curl);
  if(curl) {
    /*
		 * Set the URL the download is originating from.
     */ 
    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* Define our callback to get called when there's data to be written */ 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlfwrite);
    /* Set a pointer to our struct to pass to the callback */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
    /* Make curl Follow redirects */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    /* Set max redirects */
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIR);

    /* Switch on full protocol/debug output */ 
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);

    /* always cleanup */ 
    curl_easy_cleanup(curl);

    if(CURLE_OK != res) {
      /* we failed */ 
      rsstwritelog(LOG_ERROR, "curl told us %d %s:%d", res, __FILE__, __LINE__);
      rc = -1; // Set Error 
    }
  }

  if(ftpfile.stream)
    fclose(ftpfile.stream); /* close the local file */ 

  curl_global_cleanup();

  return rc;
}


/*
 * Free the download and clean the leftovers
 * Arguments :
 * chunk 	pointer to downloaded data, NULL pointers are ignored.
 */
void rsstfreedownload(MemoryStruct *chunk)
{
  if(chunk != NULL) {
    free(chunk->memory);
    free(chunk->header);
  }

  return;
}

/*
 * extract from HTTP-header
 * This method extracts a given field from the http-header.
 * The pointer returned contains the value.
 * after use free the returned string
 * Arguments
 * name		Header value name.
 * value	Pointer to store value in.
 * chuck	Buffer to read header from.
 * Return
 * 0 on success, -1 on failure
 */
int rsstgetheadersvalue(char *name, char **value, MemoryStruct *chunk)
{
  char *header=NULL;
  char *latest=NULL;
  char *token=NULL;
	char *lastvalue=NULL;
  char regexp[REGEXPSIZE+1];
  int rc=0;
	int retvalue=-1;

  /* 
   * Initialize vars
   */
  memset(regexp, 0, REGEXPSIZE+1);
  *value=NULL;

  /* 
   * Sanity checks
   */
  if(chunk == NULL || chunk->header == NULL) {
    *value = 0;
    return -1;
  }

  /*
   * Copy the input buffer to temp buffer.
   */
	rsstalloccopy(&header, chunk->header, strlen(chunk->header));

  /*
   * Create regexp for getting header
   */
  snprintf(regexp, REGEXPSIZE, "^ *%s ?: ?(.*)", name);

  /*
   * Use strtok to split the header, and match line by line
   */
  token = strtok_r(header, HEADEREOL, &latest);
  while(token != NULL) {
    /*
		 * Get the first match from the header
		 */
    rc =  rsstcapturefirstmatch(regexp, 0, token, value);
    if(rc == 0) {
      rsstwritelog(LOG_DEBUG, "Found '%s'->'%s'", token, *value);
			/*
			 * We want the last matching value only.
			 * Free previous found value.
			 */
			free(lastvalue);
			lastvalue=*value;
			retvalue=0;
    }

    /* 
		 * Next token
		 */
    token = strtok_r(NULL, HEADEREOL, &latest);
  }

  /*
   * Cleanup.
   */
  free(header);

  /* 
	 * Return the value
	 */
  return retvalue;
}

/*
 * Write retrieved buffer to file.
 * arguments
 * filename		path to store file in
 * buffer			Buffer holding information.
 * return
 * 0 on success -1 on failure
 */
int rsstwritebuffer(char *filename, MemoryStruct *buffer) 
{
  FILE          *file;
  int           rc;
  unsigned int  cur_char;

  rsstwritelog(LOG_DEBUG,"Writing to : '%s' length '%ld' %s:%d", filename, buffer->size, __FILE__, __LINE__);

  /*
   * Save file to test.torrent
   */
  file = fopen(filename, "w+"); 
  if(file == NULL) {
    rsstwritelog(LOG_ERROR, "Could not open file : '%s'", strerror(errno));
    return -1;
  }

  /* not very optimized, but coping for writing small files. */
  for (cur_char = 0; cur_char < buffer->size; ++cur_char) {
    rc = fputc(*(buffer->memory + cur_char), file);
    if(rc == EOF) {
      rsstwritelog(LOG_ERROR, "Could not write to file");
      fclose(file);
      return -1;
    }
  }

  /*
   * close file descriptor 
   */
  fclose(file);

  /*
   * success !
   */
  return 0;
}

