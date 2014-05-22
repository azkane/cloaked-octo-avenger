#include "lib/prelude.h"
#include "agenda.h"

void usage(void) {
  printf("Uso:\n\"$ agenda\" - Abre el archivo por default (agenda.txt)\n\"$agenda <archivo>\" Usa <archivo> como el archivo.\n");
}

/* List implementation */
SRListHead * push_sr(SRListHead * head, ScheduleRecord sr) {
  SRListItem * psr = (SRListItem*) malloc(sizeof(SRListItem));
  psr->elt = sr;
  /* memcpy(&psr->elt, &sr, sizeof(ScheduleRecord)); */
  psr->next = head->head;
  head->head = psr;
  head->len += 1;
  return head;
}

ScheduleRecord * peek_elt(SRListHead * head, int pos) {
  if (head->len == 0 || pos > (head->len)-1) return NULL;
  SRListItem * traverse = head->head;

  for (int i = 0; i < head->len; i++) {
    if (i == pos)
      return &traverse->elt;
    traverse = traverse->next;
  }
}

void delete_sr_elt(SRListHead * head, int pos) {
  if (pos < head->len) return;
  SRListItem * traverse = head->head;
  SRListItem * next;
  if (pos == 0) {
    head->head = traverse->next;
    head->len -= 1;
    free(traverse);
    return;
  }

  for (int i = 1 ; i < head->len; i++) {
    if (i == pos - 1) {         /* next element is to be freed */
      next = traverse->next;
      traverse->next = next->next;
      head->len -= 1;
      free(next);
      return;
    }
    traverse = traverse->next;
  }
}


/* Schedule Load */
FILE * with_abort_load(char * schedulef) {
  FILE *fp = fopen(schedulef, "r");
  char sign[8];
  if (fp == NULL) {
    errno = 0;                  /* Crea una nueva agenda, descarta errno */
    return NULL;
  }
  else {
    fgets(sign, sizeof(sign), fp);
    if(strcmp(sign, "AGENDA\n")) {
      printf("El archivo %s no es una agenda.\n", schedulef);
      abort();
    }
  }

  return fp;
}

/* C and its lack of lambdas and function composition, curse you for
   making me repeat myself and write functions like this
*/
int with_abort_is_int_and_to_int(char * str) {
  if (is_int_p(str)) {
    return strtol(str, NULL, 10);
  }
  else {
    printf("Errores en la agenda.\n");
    abort();
  }
}

ScheduleRecord with_abort_parse_record(char * record, int len) {
  char str[len];
  char buckets[8][256];
  char delims[2] = {0x17,'\0'};
  char *tok = NULL;

  strncpy(str, record, len);
  tok = strtok(str, delims);

  /* There should be only 8 tokens, however hardcoding it could be a source of bugs */
  for (int i = 0; i < 8; i++) {
    strncpy(buckets[i], tok, 255);
    tok = strtok(NULL, delims);
  }

  while(tok) {strtok(NULL, delims);} /* If for some reason theres padding */

  ScheduleRecord ret;
  TimeRecord date = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  ret.id = with_abort_is_int_and_to_int(buckets[0]);
  date.tm_year = with_abort_is_int_and_to_int(buckets[1]);
  date.tm_mon = with_abort_is_int_and_to_int(buckets[2]);
  date.tm_mday = with_abort_is_int_and_to_int(buckets[3]);
  date.tm_hour = with_abort_is_int_and_to_int(buckets[4]);
  date.tm_min = with_abort_is_int_and_to_int(buckets[5]);
  strncpy(ret.title, buckets[6], sizeof(ret.title));
  strncpy(ret.desc, buckets[7], sizeof(ret.desc));

  memcpy(&ret.date, &date, sizeof(TimeRecord));

  return ret;
}


SRListHead read_records_from_file(FILE * schedule) {
  char line[1024];
  SRListHead srlisth = {0, NULL};
  while(fgets(line, sizeof(line), schedule)) {
    push_sr(&srlisth, with_abort_parse_record (line, sizeof(line)));
  }
  return srlisth;
}

SRListHead schedule_load(char * schedulef) {
  FILE * fp = with_abort_load(schedulef);
  SRListHead schedule;

  if (fp == NULL) {
    schedule.head = NULL;
    schedule.len = 0;
  } else {
    schedule = read_records_from_file(fp);
    fclose(fp);
  }

  return schedule;
}

void save_schedule(SRListHead * head, char * schedulef) {
  FILE * fp = fopen(schedulef, "w");
  fprintf(fp, "AGENDA\n");

  SRListItem * traverse = head->head;
  while(traverse) {
    fprintf(fp, "%d%c%d%c%d%c%d%c%d%c%d%c%s%c%s\n", traverse->elt.id, 0x17, traverse->elt.date.tm_year, 0x17,
            traverse->elt.date.tm_mon, 0x17, traverse->elt.date.tm_mday, 0x17, traverse->elt.date.tm_hour, 0x17,
            traverse->elt.date.tm_min, 0x17, traverse->elt.title, 0x17, traverse->elt.desc);
    traverse = traverse->next;
  }
  fclose(fp);
}

/* Menu */
void pretty_print_sr(ScheduleRecord sr) {
  char timerep[128];
  char buf[256];
  strftime(timerep, sizeof(timerep), "%I:%M %p - %A %B %Y", &sr.date);
  snprintf(buf, sizeof(buf), "%s", sr.title);
  print_centered(buf, 80, '*');
  print_centered(timerep, 80, '*');
  snprintf(buf, sizeof(buf), "%s", sr.desc);
  greedy_align_paragraph(buf, 80);
  print_centered("***", 80, '*');
}

void print_sr_redux(ScheduleRecord sr) {
  char timerep[128];
  strftime(timerep, sizeof(timerep), "%I:%M %p - %A %B %Y", &sr.date);
  printf("%s - %s\n", sr.title, timerep);
}

void display_schedule(SRListHead * head) {
  ScheduleRecord * SRptr;
  for (int i = 0; i < head->len; i++) {
    printf("%d - ", i);
    SRptr = peek_elt(head, i);
    abort_if_null(SRptr, "Un error a ocurrido: head esta vacio, pero display_schedule fue llamado.\n");
    print_sr_redux(*SRptr);
    print_centered("***", 80, ' ');
  }
}

ScheduleRecord new_record(void) {
  ScheduleRecord ret;
  TimeRecord tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  ret.id = 0;
  prompt("Introduzca titulo", STR_T, &ret.title, sizeof(ret.title));
  prompt("Introduzca descripcion", STR_T, &ret.desc, sizeof(ret.desc));
  printf("Introduzca hora del evento ");
  tm.tm_hour = with_islimit_read_d(0, 24);
  printf("Introduzca dia del mes del evento ");
  tm.tm_mday = with_islimit_read_d(1, 31);
  printf("Introduzca mes del evento ");
  tm.tm_mon = with_islimit_read_d(1, 12) - 1;

  printf("Introduzca anyo del evento ");
  tm.tm_year = with_ilimit_read_d(2014) - 1900;

  memcpy(&ret.date, &tm, sizeof(TimeRecord));

  return ret;
}

void edit_help(void) {
  char *options[] = {"Editar titulo.", "Editar descripcion.", "Editar hora.",
                     "Editar dia.", "Editar mes.", "Editar anyo.",
                     "Ayuda.", "Terminar."};
  char keys[] = "tnhdmayq";
  print_menu_grid(options, 8, keys, 3);
}

void edit_record(ScheduleRecord * sr) {
  printf("Registro a editar:\n");
  pretty_print_sr(*sr);

  edit_help();

  char opt;
  do {
    do {
      prompt("Introduzca una opcion", CHAR_T, &opt, 0);
    } while(!char_in_string_p(opt, "tTnNdDhHmMaAyYqQ"));
    opt = tolower(opt);

    switch (opt) {
    case 't' :
      prompt("Introduzca un nuevo titulo", STR_T, &sr->title, sizeof(sr->title));
      break;
    case 'n' :
      prompt("Introduzca una nueva descripcion", STR_T, &sr->desc, sizeof(sr->desc));
      break;
    case 'h' :
      printf("Introduzca una nueva hora ");
      sr->date.tm_hour = with_islimit_read_d(0, 24);
      break;
    case 'd' :
      printf("Introduzca un nuevo dia ");
      sr->date.tm_mday = with_islimit_read_d(1, 31);
      break;
    case 'm' :
      printf("Introduzca un nuevo mes ");
      sr->date.tm_mon = with_islimit_read_d(0, 11);
      break;
    case 'a' :
      printf("Introduzca un nuevo mes ");
      sr->date.tm_year = with_ilimit_read_d(2014) - 1900;
      break;
    case 'y' :
      edit_help();
      break;
    case 'q' :
      return;
      break;
    }
  } while(y_or_n_p("Desea editar algo mas?"));
}

void menu_help(void) {
  char *options[] = {"Detalles de registro.", "Nuevo registro.", "Editar registro.",
                     "Borrar registro.", "Mostrar los registros.", "Ayuda.",
                     "Salir."};
  char keys[] = "onedlhq";
  print_menu_grid(options, 7, keys, 3);
}

void roll_the_intro(SRListHead * head, char * schedulef) {
  if (head->len == 0) {
    print_centered("La agenda esta vacia.\n", 80, ' ');
  } else {
    printf("Registros en la agenda:\n");
    display_schedule(head);
  }
  printf("Opciones disponibles:\n");
  menu_help();

  char option;
  bool repeated = false;
  int reg;

  while(true) {
    do {
      if (repeated) printf("Opcion incorrecta.\n");
      prompt("Introduzca una opcion", CHAR_T, &option, 0);
      repeated = true;
    } while(!char_in_string_p(option, "oOnNeEdDlLhHqQ"));
    repeated = false;

    option = tolower(option);
    switch (option) {
    case 'o':
      if(head->len == 0) {
        printf("No hay ningun registro.\n");
        break;
      } else if(head->len == 1) {
        pretty_print_sr(*(peek_elt(head, reg)));
        break;
      }
      printf("Introduzca el indice del registro que desea abrir ");
      reg = with_islimit_read_d(0, head->len - 1);
      pretty_print_sr(*(peek_elt(head, reg)));
      break;
    case 'n':
      push_sr(head, new_record());
      printf("Registro guardado.\n");
      break;
    case 'e':
      printf("Introduzca el indice del registro que desea editar ");
      if(head->len == 0) {
        printf("No hay ningun registro.\n");
        break;
      } else if(head->len == 1) {
        pretty_print_sr(*(peek_elt(head, reg)));
        break;
      }
      reg = with_islimit_read_d(0, head->len - 1);
      edit_record(peek_elt(head, reg));
      printf("Registro guardado!\n");
      break;
    case 'd':
      printf("Introduzca el indice del elemento a eliminar ");
      if(head->len == 0) {
        printf("No hay ningun registro.\n");
        break;
      } else if(head->len == 1) {
        pretty_print_sr(*(peek_elt(head, reg)));
        break;
      }
      reg = with_islimit_read_d(0, head->len - 1);
      delete_sr_elt(head, reg);
      display_schedule(head);
      /* Once a record is deleted, display a new list?, should the items move down? or the index remain fixed during execution? questions questions... */
      break;
    case 'l':
      display_schedule(head);
      break;
    case 'h':
      menu_help();
      break;
    case 'q':
      save_schedule(head, schedulef);
      while(head->head) {
        delete_sr_elt(head, 0);
      }
      return;
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  usage();
  SRListHead schedule;
  if (argc > 1) {
    schedule = schedule_load(argv[1]);
    roll_the_intro(&schedule, argv[1]);
  } else {
    schedule = schedule_load("agenda.txt");
    roll_the_intro(&schedule, "agenda.txt");
  }
  return 0;
}
