#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Renders a human readable size from the bytes
 */
const char* __far renderHumanReadableSize(unsigned long size)
{
  static const char* __far suffix[] = {"b", "K", "M", "G", "T"};
  static const char* __far format_0 = "%.01lf%s";
  static char output[48];

  memset(output, 48, 0);

  int i = 0;
  double dBytes = size;
  if(size > 1024)
  {
    for(i=0; (size/1024) > 0 && i < 4; i++, size /= 1024)
    {
      dBytes = size / 1024.0;
    }
  }
  if(i == 0)
  {
    sprintf(output, "%db", size);
  }
  else
  {
    sprintf(output, format_0, dBytes, suffix[i]);
  }
  return output;
}
