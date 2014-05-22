#ifndef PRELUDE_H
#define PRELUDE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>


typedef enum {
              INT_T,
              CHAR_T,
              FLOAT_T,
              STR_T,
            } RType;

typedef struct {
  RType type;
  union {
    int i_datum;
    char c_datum;
    float f_datum;
    char * s_datum;
  };
} GenericReturn;


typedef struct {
  bool (*g_p) (GenericReturn);
  bool (*i_p) (int);
  bool (*f_p) (float);
  bool (*c_p) (char);
  bool (*s_p) (char *);
} PredicateDispatch;


/* swap :: int *, int * -> void
   Intercambia dos elementos enteros en memoria */

void swap(int * a, int * b) {
  int t = *a;
  *a = *b;
  *b = t;
}

/* trim :: [*char] -> *char
   Elimina el espacio en blanco antes y despues de un string
   Tomado de: http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way */
char * trim(char *str) {
  size_t len = 0;
  char *front_p = str - 1;
  char *end_p = NULL;

  if( str == NULL )
    return NULL;

  if( str[0] == '\0' )
    return str;

  len = strlen(str);
  end_p = str + len;

  while( isspace(*(++front_p)) );
  while( isspace(*(--end_p)) && end_p != front_p );

  if( str + len - 1 != end_p )
    *(end_p + 1) = '\0';
  else if( front_p != str &&  end_p == front_p )
    *str = '\0';

  end_p = str;
  if( front_p != str ) {
    while( *front_p ) *end_p++ = *front_p++;
    *end_p = '\0';
  }

  return str;
}


/* rstdin_s :: void -> void
   Lee stdin por un string de tamanyo s y lo guarda en data */
void rstdin_s(char * data, int s) {
  if (fgets(data, s, stdin)) {
    trim(data);
    return;
  }
  else {
    printf ("Datos invalidos, intente de nuevo.\n");
    return rstdin_s(data, s);
  }
}

/* rstdin_c :: void -> int
   Lee stdin por un caracter y limpia el buffer de stdin */
char rstdin_c(void) {
  /* En linux la terminacion de linea es \n, en windows es \n\r,
     por tanto, lee todo el buffer stdin y regresa el primer caracter. */
  char c[5];
  rstdin_s(c, sizeof c);
  return c[0];
}

/* is_float_p :: char[] -> bool
   Verifica que una cadena de caracteres `str` puede ser un numero flotante
*/
bool is_float_p(char * str) {
  int p_quantity = 0;
  for (int i = 0; i <= (int) strlen(str) - 1; i++) {
    if (i == 0 && str[i] == '-') // '-' solo puede existir en la primera posicion
      continue;
    if (str[i] == '.' && p_quantity == 0) { // solo puede haber un punto
      p_quantity++;
      continue;
    }
    if (!(isdigit(str[i]))) {
      return false;
    }
  }
  return true;
}

/* rstdin_f :: void -> int
   Lee stdin por un numero flotante, verifica que es realmente un numero y lo regresa */
float rstdin_f(void) {
  /* Mismo problema con la terminacion de lineas, misma solucion */
  char d[256];
  float ret;

  rstdin_s(d, sizeof d);
  if (is_float_p(d)) {
    ret = strtod(d, NULL);
    if (errno) {
      printf("Error encontrado: %s\n, intente de nuevo", strerror(errno));
      return rstdin_f();
    }
    return ret;
  } else {
    printf ("%s no es un numero, intente de nuevo: ", d);
    return rstdin_f();
  }
}

/* is_int_p :: char[] -> bool
   Toma una cadena de caracteres `d` y verifica que puede ser transformada a un entero
*/
bool is_int_p (char * d){
  for (int i = 0; i <= (int) strlen(d) - 1; i++) {
    if (i == 0 && d[i] == '-') /* Unicamente '-' puede aparecer en la primera posicion */
      continue;
    if (!(isdigit(d[i]))) {
      return false;
    }
  }
  return true;
}

/* rstdin_d :: void -> int
   Lee stdin por un numero, verifica que es realmente un numero y lo regresa */
int rstdin_d(void) {
  /* Mismo problema con la terminacion de lineas, misma solucion */
  char d[256];
  int ret;
  rstdin_s(d, sizeof d);
  if (is_int_p(d)) {
    ret = strtol(d, NULL, 10);
    if (errno) {
      printf("Error encontrado: %s\n, intente de nuevo", strerror(errno));
      return rstdin_d();
    }
    return ret;
  } else {
    printf ("%s no es un numero, intente de nuevo: ", d);
    return rstdin_d();
  }
}

/* rstdin_g :: void -> GenericReturn

   Lee stdin por uno de [int||float||char] lo regresa en un struct
   tipo GenericReturn

   Acepta un puntero a un buffer de char[] que sera usado en vez de
   crear uno en el scope actual. Este buffer debe ser almenos de 256
   elementos.
*/
GenericReturn rstdin_g(char * s) {
  char d[256];
  GenericReturn ret;

  rstdin_s(d, sizeof d);

  if (is_int_p(d)) {
    ret.type = INT_T;
    ret.i_datum = strtol(d, NULL, 10);
  }
  else if (is_float_p(d)) {
    ret.type = FLOAT_T;
    ret.f_datum = strtod(d, NULL);
  }
  else if (strlen(trim(d)) == 1) {
    ret.type = CHAR_T;
    ret.c_datum = (trim(d))[0];
  }
  else if (s != NULL) {
    ret.type = STR_T;
    ret.s_datum = strcpy(s, d);
  }
  else { // No se paso buffer desde el stackframe anterior
    printf("Entrada incorrecta, intente de nuevo");
    rstdin_g(s);
  }
  return ret;
}

/* with_validator_rstdin_g :: char *, (PredicateDispatch) -> GenericReturn
   Lee desde stdin usando `rstdin_g`, dependiendo el tipo leido aplica la
   funcion de validacion correspondiente de un struct de tipo `PredicateDispatch`
   Si la entrada falla la validacion se pide al usuario que introduzca de
   nuevo los datos.
*/
GenericReturn with_validator_rstdin_g(char * s, PredicateDispatch p) {
  GenericReturn ret = rstdin_g(s);
  if (p.g_p != NULL) {            /* Do a validation in the ret object */
    while(!(p.g_p(ret))) {
      ret = with_validator_rstdin_g(s, p);
    }
  }
  if (p.i_p != NULL && ret.type == INT_T) {
    while (!(p.i_p(ret.i_datum))) {
      ret = with_validator_rstdin_g(s, p);
      if (ret.type != INT_T) break;
    }
  }
  if (p.f_p != NULL && ret.type == FLOAT_T) {
    while (!(p.f_p(ret.f_datum))) {
      ret = with_validator_rstdin_g(s, p);
      if (ret.type != FLOAT_T) break;
    }
  }
  if (p.c_p != NULL && ret.type == CHAR_T) {
    while (!(p.c_p(ret.c_datum))) {
      ret = with_validator_rstdin_g(s, p);
      if (ret.type != CHAR_T) break;
    }
  }
  if (p.s_p != NULL && ret.type == STR_T) {
    while (!(p.s_p(ret.s_datum))) {
      ret = with_validator_rstdin_g(s, p);
      if (ret.type != STR_T) break;
    }
  }
  return ret;
}

/* with_slimit_read_d :: int -> int
   Lee stdin por un numero, si es mayor que n, pregunta de nuevo, hasta que el numero leido sea menor que n*/
int with_slimit_read_d(int n) {
  int m;
  do {
    printf ("(no debe ser mayor a %d): ", n);
    m = rstdin_d();
  } while (m > n);
  return m;
}

/* with_ilimit_read_d :: int -> int
   Lee stdin por un numero, si es menor que n, pregunta de nuevo, hasta que el numero leido sea mayor que n*/
int with_ilimit_read_d(int n) {
  int m;
  do {
    printf ("(debe ser mayor a %d): ", n);
    m = rstdin_d();
  } while (m < n);
  return m;
}

/* with_islimit_read_d :: int int -> int
   Lee stdin por un numero que sear mayor o igual que n y menor o igual que n */
int with_islimit_read_d(int n, int m) {
  int r;
  bool flag = false;
  do {
    if (flag)
      printf("Digito fuera de rango\n");
    printf ("[%d, %d]: ", n, m);
    r = rstdin_d();
    flag = true;
  } while (r < n || r > m);
  return r;
}

/* y_or_n_p :: *str -> bool
   Toma una cadena de caracteres, la imprime y espera entrada del
   usuario, si la entrada es s regresa true, n regresa false,
   cualquier otra hace que se repita.
   like http://clhs.lisp.se/Body/f_y_or_n.htm <3 Lisp :) */
bool y_or_n_p(char * str) {
  char q;
  do {
    printf("%s (s/n): ", str);
    q = rstdin_c();
  } while (q != 's' && q != 'S' && q != 'n' && q != 'N'); /* Repite hasta que la entrada sea valida, ie:(s/S/n/N) */
  return (q == 's' || q == 'S') ? true : false;
}

/* with_message_repeat :: *(void), *char -> void
   Toma un puntero a una funcion, llama la funcion y muestra un string
   dado, hasta que el usuario lo desee
*/
void with_message_repeat(void (*f)(void), char * str) {
  do {
    (*f)();
  } while ((y_or_n_p(str)));
}

/* repeat_till_user :: *(void) -> void
   Toma un puntero a una funcion, llama repetidamente la funcion hasta que
   el usuario lo desee
*/
void repeat_till_user(void (*f)(void)) {
  with_message_repeat(f, "Repetir?");
}

/* show_menu :: [* char] int -> int
   Toma un arreglo de cadenas y el tamanyo de este, muestra un menu,
   espera por la entrada del usuario y la valida */
int show_menu(char **options, int len) {
  int a;
  for (int i = 0; i < len; i++) {
    printf ("%d - %s\n",i, options[i]);
  }
  a = with_islimit_read_d(0, len - 1);
  return a;
}

/* prnt_centered :: [* char], int, int -> void
   Toma una linea, la imprime centrada dentro del limite especificado
   agregando el borde deseado, line debe ser de un tamanyo menor que
   max_width */
void print_centered(const char * line, const int max_width, const char border) {
  int ws = max_width - strlen(line);
  printf ("%c%*s%s%*s%c\n", border, ws/2, " ", line,
          ws % 2 == 0 ? ws / 2 : (ws / 2) + 1, " ", border);
}

/* greedy_align_paragraph :: [char], int -> void
   Toma un arreglo de caracteres y el largo maximo de una linea, imprime
   el texto alineado.

   Se inicializa un arreglo de caracteres de tamanyo max_width, el
   cual representara una linea y funcionara como buffer. Usando strtok
   se divide line en tokens, estos se concatenan a tmp hasta que este se
   llene, cuando esto sucede se imprime el string y el buffer se limpia */
void greedy_align_paragraph(char * line, int max_width) {
  char tmp[max_width]; 	/* String temporal */
  char delims[] = " ";	/* Delimitadores para strtok y strcspn */
  int token_len = 0;		/* Tamanyo de un token */
  char * token =  strtok(line, delims);
  int len;
  tmp[0] = '\0';		/* Asumimos que max_width es mayor que 0, inicializa el primer elemento en null */

  while (true) {
    /* Si hay un puntero a un token, calcula su tamanyo. */
    if (token) {
      token_len = strcspn(token, delims);
    }

    /* Si tmp llego a su limite o no hay token, imprime tmp */
    len = strlen(tmp);
    if (token_len + len > (max_width - 4) || !token) {
      print_centered(tmp, max_width, '*');
      memset(tmp, -1, max_width);
      tmp[0] = '\0';
    }

    /* Si no hay token termina el bucle */
    if (!token)
      break;

    /* Si existe algo en tmp, entonces concatena un espacio */
    if (strlen(tmp) > 0) {
      strcat(tmp, " ");
    }

    /* Concatena el token a tmp */
    strcat(tmp, token);

    /* Recupera el siguiente token */
    token = strtok(NULL, delims);

  }
}

/* contains_char_p String, String -> Boolean

   Toma dos cadenas, verifica si alguno de los elementos de `chars`
   existen en `str`
 */
bool contains_char_p(char * str, char * chars) {
  int len_str = strlen(str);
  int len_chr = strlen(chars);
  for (int i = 0; i < len_str; i++)
    for (int j = 0; j < len_chr; j++)
      if (str[i] == chars[j]) return true;
  return false;
}

/* median :: [int], int -> int

   Calcula la media de un arreglo, toma el arreglo y el tamanyo de
   este */
int median(int * arr, int len) {
  int acc = 0;
  for (int i = 0; i < len; ++i)
    acc += *(arr + i);
  return acc / len;
}

/* show_banner :: [char] -> void
   Toma dos arreglos de caracteres, el nombre del programa y la
   descripcion del programa.  Muestra el nombre del programa, la
   descripcion y posteriormente los integrantes del equipo
*/
void show_banner(char * name, char * desc) {
  for (int i = 0; i < 80; i++)
    printf ("*");
  putchar('\n');
  /* asumimos que el nombre es menor de 78 caracteres */
  print_centered(name, 78, '*');
  printf ("*%79c\n", '*');
  greedy_align_paragraph(desc, 78);
  printf ("*%79c\n", '*');

  for (int i = 0; i < 80; i++)
    printf ("*");
  putchar('\n');
}

/* str_reverse :: *char -> *char
   Da vuelta a un string */
char * str_reverse(char * str) {
  int r = strlen(str) - 1;
  int l = 0;

  char c;
  while (l < r){
    c = str[r];
    str[r] = str[l];
    str[l] = c;
    l++;
    r--;
  }
  return str;
}

/* str_append_c :: *char, char -> *char
   Concatena un caracter al final de un string */
char * str_append_c(char * str, char c) {
  int l;
  l = strlen(str);
  *(str + l) = c;
  *(str + l + 1) = '\0';
  return str;
}

/* d_in_range_p :: int, int, int -> bool
   Verifica si un numero entero dado se encuentra en el intervalo abierto
   dado. */
bool d_in_range_p(int n, int il, int sl) {
  return n >= il && n <= sl ? true : false;
}

/* clrscr :: void -> void
   Limpia la pantalla portablemente
*/
void clrscr(void) {
#ifdef _WIN32
  system("cls");
#elif __unix
  system("clear");
#else
  for (int i = 0; i < 100; i++)
    putchar("\n");
#endif
}

/* suspend :: *str -> void
   Muestra una cadena de caracteres `str` y pausa la ejecucion del programa.
*/
void suspend(char * str) {
  printf("%s\n", str);
#ifdef _WIN32
  system("pause");
#elif __unix
  system("read -n1 -r");
#else
  getchar();
#endif
}

/* eq :: int, int -> bool
   Verifica si dos enteros son iguales */
bool eq(int a, int b) {
  return a == b ? true : false;
}

/* eq_f :: float, float -> bool
   Verifica si dos flotantes son aproximadamente iguales
*/
bool eq_f(float a, float b) {
  float epsilon = 0.001;
  return ((a - epsilon) < b) && ((a + epsilon) > b) ? true : false;
}

/* prompt :: String, RType, void*, size -> void
   Muestra un mensaje `str`, y lee stdin por el tipo indicado en
   `type`, guarda el dato en `storage`
 */
void prompt(char * str, RType type, void * storage, int storage_size) {
  printf("%s: ", str);
  switch (type) {
  case INT_T:
    *((int*) storage) = rstdin_d();
    break;
  case CHAR_T:
    *((char*) storage) = rstdin_c();
    break;
  case FLOAT_T:
    *((float*) storage) = rstdin_f();
    break;
  case STR_T:
    rstdin_s(storage, storage_size);
    break;
  }
}

/* abort_if_null :: void *, String -> void
   Toma un puntero y un string, si el puntero apunta a NULL
   termina la ejecucion del programa */
void abort_if_null(void * ptr, const char str[]) {
  if (ptr == NULL) {
    printf("%s", str);
    abort();
  }
}

/* obtain_file_handle :: String, String -> FILE *
   Abre un archivo y regresa un puntero a este, verificando que el
   puntero no sea NULL
*/
FILE * obtain_file_handle(char * filename, char * attr) {
  FILE * fp;
  fp = fopen(filename, attr);
  abort_if_null(fp, "Imposible abrir el archivo para escribir.\n");
  return fp;
}

/* char_in_string_p :: Char, String -> Bool
   Verifica si un caracter pertenece a un string
 */
bool char_in_string_p(char c, char * str) {
  int len = strlen(str);

  for (int i = 0; i < len; i++) {
    if (c == str[i]) return true;
  }
  return false;
}

/* print_menu_grid :: String[], int , String, int -> void
   Imprime un menu con `cols` numero de columnas, siendo
   `optarr` las descripciones y `keys` la tecla opcion.
*/
void print_menu_grid(char *optarr[], int optarr_size, char * keys, int cols) {
  /* Determine field size, which is equal to the largest string in optarr */
  int len = 0, tmp;
  for (int i = 0; i < optarr_size; i++) {
    tmp = strlen(optarr[i]);
    if (tmp > len)
      len = tmp;
  }

  for (int i = 0; i < optarr_size; i++) {
    printf("  %c) %-*s", keys[i], len, optarr[i]);
    if ((i + 1) % cols == 0)
      putchar('\n');
  }
  putchar('\n');
}

/* Simplicity is the ultimate sophistication - LDV */
#endif
