#ifndef _RAND_H_

#define _RAND_H_

// https://stackoverflow.com/a/24005529/13877490
static unsigned long int next = 1;
int random(void) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

#endif // _RAND_H_