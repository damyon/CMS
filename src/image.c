#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "config.h"
#include "env.h"
#include "strings.h"
#include "errors.h"
#include "logging.h"
#include "file.h"
#include "structs.h"
#include "package.h"
#include "objects.h"
#include "dbcalls.h"
#include "config.h"
#include "malloc.h"
#include "image.h"
#include "request.h"

char *escapeConvertShellArg(char *src) {
	char *result = NULL;
	int i = 0, j = 0;

	result = (char *) malloc(sizeof(char) * ((strlen(src) * 2) + 1));

	for (i = 0; i < strlen(src); i++) {
		result[j++] = src[i];
		if (src[i] == '%')
		  result[j++] = src[i];
		if (src[i] == '\\')
		  result[j++] = src[i];
		if (src[i] == '\'')
		  result[j++] = src[i];
	}
	result[j] = '\0';

	return result;
}

/**********************************************************************
* execConvert...
*
* Execute the image convert program.
**********************************************************************/
int execConvert(char *infile, char *outfile, char *dimensions, char *text, char *align, char *fontface, 
		char *fontsize, char *strokewidth, char *strokecolour, char *fillcolour, Env *env, void *sqlsock) {
  char *args[26], *filepath = NULL;
  int i = 0, fid = 0, timestamp = 0, ret = 0;

  args[0] = getConvertBinPath();
  args[1] = infile;

  i = 2;

  if (dimensions != NULL) {
	  args[i++] = "-scale";
	  dimensions = escapeConvertShellArg(dimensions);
	  args[i++] = dimensions;
  }

  if (text != NULL) {
	  if (fontsize != NULL) {
	  	args[i++] = "-pointsize";
		fontsize = escapeConvertShellArg(fontsize);
	  	args[i++] = fontsize;
	  }
	  if (fontface != NULL) {
  	        timestamp = getTimeValue(getEnvValue(ISOTIMETOK, env), getEnvValue(CTIMETOK, env));
                if (isXRefValid(fontface, timestamp, sqlsock, env, &fid) == E_OK) {
		      if (userHasReadAccess(fid, env, sqlsock) == E_OK) {
                            if (generateFilePath(fid, &filepath) == E_OK) {
	  	                args[i++] = "-font";
	  	                args[i++] = filepath;
			    }
		      }
		}
	  }
	  if (strokecolour != NULL) {
	  	args[i++] = "-stroke";
		strokecolour = escapeConvertShellArg(strokecolour);
	  	args[i++] = strokecolour;
	  }
	  if (strokewidth != NULL) {
	  	args[i++] = "-strokewidth";
		strokewidth = escapeConvertShellArg(strokewidth);
	  	args[i++] = strokewidth;
	  }
	  if (align != NULL) {
	  	args[i++] = "-gravity";
		align = escapeConvertShellArg(align);
	  	args[i++] = align;
	  }
	  if (fillcolour != NULL) {
	        args[i++] = "-fill";
		fillcolour = escapeConvertShellArg(fillcolour);
		args[i++] = fillcolour;
	  }
	  text = escapeConvertShellArg(text);
	  args[i++] = "-annotate";
	  args[i++] = "0x0";
	  args[i++] = text;
          args[i++] = "-stroke";
          args[i++] = "none";
          args[i++] = "-annotate";
          args[i++] = "0x0";
          args[i++] = text;
  }

  args[i++] = outfile;
  args[i++] = NULL;

  ret = execvp(getConvertBinPath(), args);
  dhufree(filepath);
  dhufree(fontsize);
  dhufree(dimensions);
  dhufree(strokecolour);
  dhufree(strokewidth);
  dhufree(align);
  dhufree(text);
  dhufree(fillcolour);

  return ret;
}

/**********************************************************************
* runConvert...
*
* Fork before we execute the image convert program.
**********************************************************************/
int runConvert(char *infile, char *outfile, char *dimensions, char *text, char *align, char *fontface, 
		char *fontsize, char *strokewidth, char *strokecolour, char *fillcolour, Env *env, void *sqlsock) {
  int pid = 0, status = RESOURCEERROR, timeout = 0;

  pid = fork();
  if (pid == 0) {
    // child
    execConvert(infile, outfile, dimensions, text, align, fontface, fontsize, strokewidth, strokecolour, fillcolour, env, sqlsock);

    _exit(0);
  } else if (pid > 0) {
    while (waitpid(pid, &status, WNOHANG) == 0) {
      sleep(1);
      timeout++;
      if (timeout == 5) {
	kill(pid, SIGKILL);
	break;
      }
    }
  }
  if (status != 0) {
    return 0;
  }

  return 0;
}

/**********************************************************************
* resizeImage...
*
* Resizes an image to the dimensions specified and modifies the 
* file object passed as a parameter.
* @param FileObject file - The file object to resize.
* @param char * dimensions - The new dimensions for the image. (eg 40X60)
**********************************************************************/
int resizeImage(char **filedataptr,  int *filelenptr, Env *env, void *sqlsock) {
  char *filedata = NULL;
  char *tmpfilein = NULL;
  char *tmpfileout = NULL;
  char *tmpdimensions = NULL,
       *tmptext = NULL,
       *tmpalign = NULL,
       *tmpfontface = NULL,
       *tmpfontsize = NULL,
       *tmpstrokewidth = NULL,
       *tmpstrokecolour = NULL,
       *tmpfillcolour = NULL,
       *sep = NULL,
       *dimensions = NULL;
  int filelen = 0;
  int err = 0;

  if (filedataptr == NULL || filelenptr == NULL) {
    return RESOURCEERROR;
  }

  logDebug("Convert : %s\n", getConvertBinPath());
  if (strcmp(getConvertBinPath(), "") == 0) {
    return E_OK;
  }

  filedata = *filedataptr;
  filelen = *filelenptr;
  
  tmpfilein = getTmpFile(0);
  tmpfileout = getTmpFile(1);

  logDebug("Write file : %s\n", tmpfilein);
  if ((err = writeFile(tmpfilein, filedata, filelen)) != E_OK) {
    dhufree(tmpfilein);
    dhufree(tmpfileout);
    return err;
  }

  dimensions = getEnvValue("image-size", env);
  logDebug("Resize image: %s\n", dimensions);
  if (dimensions) {
    if ((sep = strchr(dimensions, '-')) == NULL) {
      tmpdimensions = dhustrdup(dimensions);
    } else {
      vstrdupcat(&tmpdimensions, dimensions, "!", NULL);
      sep = strchr(tmpdimensions, '-');
      *sep = 'x';
    }
  }
  
  tmptext = getEnvValue("image-text", env);
  tmpalign = getEnvValue("image-align", env);
  tmpfontface = getEnvValue("image-fontface", env);
  tmpfontsize = getEnvValue("image-fontsize", env);
  tmpstrokewidth = getEnvValue("image-strokewidth", env);
  tmpstrokecolour = getEnvValue("image-strokecolour", env);
  tmpfillcolour = getEnvValue("image-fillcolour", env);

  if ((err = runConvert(tmpfilein, tmpfileout, tmpdimensions, tmptext, tmpalign, tmpfontface, tmpfontsize, tmpstrokewidth, tmpstrokecolour, tmpfillcolour, env, sqlsock)) != E_OK) {
    unlink(tmpfilein);
    unlink(tmpfileout);
    dhufree(tmpfilein);
    dhufree(tmpfileout);
    dhufree(tmpdimensions);
    return err;
  }
  dhufree(tmpdimensions);

  if ((err = readFile(tmpfileout, &filedata, &filelen)) != E_OK) {
    unlink(tmpfilein);
    unlink(tmpfileout);
    dhufree(tmpfilein);
    dhufree(tmpfileout);
    return err;
  }

  dhufree(*filedataptr);
  *filedataptr = filedata;
  *filelenptr = filelen;
  
  unlink(tmpfilein);
  unlink(tmpfileout);
  dhufree(tmpfilein);
  dhufree(tmpfileout);
  return E_OK;
}

