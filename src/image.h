/************************************************************************
* image.h
*
* Image Management functions.
************************************************************************/
#ifndef _IMAGE_H
#define _IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************************
* resizeImage...
*
* Resizes an image to the dimensions specified and modifies the
* file object passed as a parameter.
* @param FileObject file - The file object to resize.
* @param char * dimensions - The new dimensions for the image. (eg 40X60)
**********************************************************************/
int resizeImage(char **dataptr, int *datalen, Env *env, void *sqlsock);

#ifdef __cplusplus
}
#endif


#endif // _IMAGE_H
