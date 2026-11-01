#include "picoc.h"
#include "interpreter.h"

#include <stdio.h>
#include <string.h>

static void (*nwcc_output_callback)(const char *, int);
static bool (*nwcc_input_callback)(char *, int, void *);
static void *nwcc_input_context;

void nwcc_picoc_set_output_callback(void (*callback)(const char *, int))
{
    nwcc_output_callback = callback;
}

void nwcc_picoc_capture_write(const char *data, int length)
{
    if (nwcc_output_callback != NULL)
        nwcc_output_callback(data, length);
    else
        fwrite(data, 1, (size_t)length, stdout);
}

void nwcc_picoc_set_input_callback(bool (*callback)(char *, int, void *),
                                   void *context)
{
    nwcc_input_callback = callback;
    nwcc_input_context = context;
}

bool nwcc_picoc_read_input(char *buffer, int length)
{
    return nwcc_input_callback != NULL &&
           nwcc_input_callback(buffer, length, nwcc_input_context);
}

void PlatformInit(Picoc *pc)
{
    (void)pc;
}

void PlatformCleanup(Picoc *pc)
{
    (void)pc;
}

char *PlatformGetLine(char *buffer, int max_length, const char *prompt)
{
    (void)buffer;
    (void)max_length;
    (void)prompt;
    return NULL;
}

int PlatformGetCharacter(void)
{
    return EOF;
}

void PlatformPutc(unsigned char out_ch, union OutputStreamInfo *stream)
{
    (void)stream;
    nwcc_picoc_capture_write((const char *)&out_ch, 1);
}

void PlatformExit(Picoc *pc, int return_value)
{
    pc->PicocExitValue = return_value;
    longjmp(pc->PicocExitBuf, 1);
}

void PicocPlatformScanFile(Picoc *pc, const char *file_name)
{
    (void)pc;
    (void)file_name;
}

void PlatformLibraryInit(Picoc *pc)
{
    PicocIncludeAllSystemHeaders(pc);
}
