#include <time.h>

typedef struct tm TimeRecord;

typedef struct ScheduleRecord {
  int id;
  TimeRecord date;
  char title[64];
  char desc[256];
} ScheduleRecord;

typedef struct SRListItem {
  ScheduleRecord elt;
  struct SRListItem * next;
} SRListItem;

typedef struct SRListHead {
  int len;
  SRListItem * head;
} SRListHead;
