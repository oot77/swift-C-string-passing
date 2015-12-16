// C hack: replace printf to collect output and return complete string by using
// a line buffer.
// Beware of calling tprntNewLine so the line is added to the return string!

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define OK 0      // linux return value: 0 = successful
#define ENOMEM 12 // linux return value: 12 = out of memory

// outLineBuffer collects one output line by several calls to tprntf
#define INITIAL_SIZE_OF_RETURNBUFFER 10 // reduced for tests (would be 16*1024)
#define INCR_SIZE_OF_RETURNBUFFER 5 // reduced for testing (would be 1024*1024)
#define OUTLINE_BUFFER_MAXSIZE 4095
char outLineBuffer[sizeof(char)*OUTLINE_BUFFER_MAXSIZE] = "";
char *tReturnString;
size_t sizeOfReturnBuffer, curPosOutBuffer = 0, lenOutLine = 0;

// printf replacement to collect the parts of one output line in outLineBuffer.
static int tprntf(const char *fmt, ...)
{
    const size_t maxLen = sizeof(char)*OUTLINE_BUFFER_MAXSIZE;
    va_list arg;
    int rVal;
    
    va_start (arg, fmt);
    rVal = vsnprintf (&outLineBuffer[lenOutLine], maxLen-lenOutLine, fmt, arg);
    va_end (arg);
    lenOutLine += strlen(&outLineBuffer[lenOutLine]);

    return rVal;
}

// fputs replacement to collect the parts of one output line in outLineBuffer.
static int tPuts(const char *s, FILE *stream)
{
    const size_t maxLen = sizeof(char)*OUTLINE_BUFFER_MAXSIZE;
    int rVal;

    if (stream == stdout) {
        rVal = snprintf (&outLineBuffer[lenOutLine], maxLen-lenOutLine, "%s",s);
        lenOutLine += strlen(&outLineBuffer[lenOutLine]);
        if (rVal < 0) {
            return EOF;
        } else {
            return rVal;
        }
    } else {
        return fputs(s, stream);
    }
}

// Output line is now complete: copy to return buffer and reset line buffer.
//   Don't forget to call this (especially for the last prints) so the line 
//   is added to the return string!
static void tprntNewLine()
{
    size_t newSize;
    long remainingLenOutBuffer, neededSize;
    char *newOutBuffer;

    remainingLenOutBuffer = sizeOfReturnBuffer-curPosOutBuffer-1;
    lenOutLine++; // + newline character (\n)
    remainingLenOutBuffer -= lenOutLine;

    if (remainingLenOutBuffer < 0) {
        //newSize = sizeOfReturnBuffer + sizeof(char)*INCR_SIZE_OF_RETURNBUFFER;
        neededSize = -remainingLenOutBuffer;
        if (neededSize < sizeof(char)*INCR_SIZE_OF_RETURNBUFFER)
            neededSize = sizeof(char)*INCR_SIZE_OF_RETURNBUFFER;
        newSize = sizeOfReturnBuffer + neededSize;

        if ((newOutBuffer = realloc(tReturnString, newSize)) != 0) {
            tReturnString = newOutBuffer;
            sizeOfReturnBuffer = newSize;
        } else {
            // just write part that is still available:
            lenOutLine += remainingLenOutBuffer;
            //remainingLenOutBuffer = 0;
        }
    }

    snprintf(&tReturnString[curPosOutBuffer],lenOutLine+1,"%s\n",outLineBuffer);

    curPosOutBuffer += lenOutLine;
    outLineBuffer[0] = 0;
    lenOutLine = 0;
}

void tFreeMemory ()
{
    free(tReturnString);
}

// printf replacement: necessary because #define replaces all printf with tprntf
int prf(char *fmt, ...)
{
    int result;
    va_list args;
    va_start(args, fmt);
    result = vprintf(fmt, args);
    va_end(args);
    return result;
}

#ifndef COLLECT_STDOUT_IN_BUFFER
#define COLLECT_STDOUT_IN_BUFFER
#define printf tprntf
#define fputs tPuts
#endif

// For testing with C compiler. Rename when used in Xcode project e.g. to mymain
int main(int argc, char *argv[])
{
    int i;
    sizeOfReturnBuffer = INITIAL_SIZE_OF_RETURNBUFFER*sizeof(char);
    if ((tReturnString = malloc(sizeOfReturnBuffer)) == 0) {
        // "Sorry we are out of memory. Please close other apps and try again!"
        return ENOMEM; 
    }
    tReturnString[0] = 0;
    curPosOutBuffer = 0;

    for (i = 0; i < argc; i++) printf("%s ", argv[i]);
    tprntNewLine();

    printf("%s", "ABC\t");
    printf("%d", 12);
    tprntNewLine(); // enough space for that ;-)
    fputs("DEF\t", stdout);
    printf("%d", 34);
    tprntNewLine(); // realloc necessary ...
    printf("%s", "xxxxxxxxx 80 chars are way more than the current buffer "
	         "could handle! xxxxxxxxxx\t");
    printf("%d", 56);
    tprntNewLine(); // again realloc (test: too small INCR_SIZE_OF_RETURNBUFFER)

#ifdef COLLECT_STDOUT_IN_BUFFER //undo rename to view results:
#undef printf
#endif
    printf("tReturnString at the end:\n>%s<\n", tReturnString);
    tFreeMemory ()
    return OK;
}
