// https://nachtimwald.com/2019/09/08/making-minizip-easier-to-use/ because fuck minizip

#include "zipper.h"

bool zipper_add_dir(zipFile zfile, const char *dirname)
{
    char   *temp;
    size_t  len;
    int     ret;

    if (zfile == NULL || dirname == NULL || *dirname == '\0')
        return false; 

    len  = strlen(dirname);
    temp = (char*)calloc(1, len+2);
    memcpy(temp, dirname, len+2);
    if (temp[len-1] != '/') {
        temp[len] = '/';
        temp[len+1] = '\0';
    } else {
        temp[len] = '\0';
    }

    ret = zipOpenNewFileInZip64(zfile, temp, NULL, NULL, 0, NULL, 0, NULL, 0, 0, 0);
    if (ret != ZIP_OK)
        return false;
    free(temp);
    zipCloseFileInZip(zfile);
    return ret==ZIP_OK?true:false;
}
