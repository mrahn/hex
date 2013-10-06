#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Judy.h>

#ifndef SIZE
#error "SIZE not defined"
#endif

#define LEN (SIZE + 1)
#define LIN(x, y) ((x) * LEN + (y))
#define X(f) ((f) / LEN)
#define Y(f) ((f) % LEN)

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#define L 0
#define R 1
#define N 2

static char const* const show_player[3] = {"L", "R", "."};

typedef struct
{
  uint8_t* seen;
  int* open;
  int begin;
  int end;
} *PState_DFS;

PState_DFS new_state()
{
  PState_DFS state = malloc (sizeof (*state));
  state->seen = (uint8_t*) malloc (LEN * LEN * sizeof (uint8_t));
  state->open = (int*) malloc (LEN * LEN * sizeof (int));
  if (!state->seen || !state->open)
  {
    fprintf (stderr, "could not allocate memory\n");
    exit (EXIT_FAILURE);
  }
  return state;
}
void release_state (PState_DFS state)
{
  free (state->seen);
  free (state->open);
  free (state);
}
void push (PState_DFS state, int x, int y)
{
  state->open[state->end++] = x;
  state->open[state->end++] = y;
  state->seen[LIN (x, y)] = 1;
}
int pop (PState_DFS state)
{
  return state->open[state->begin++];
}
void prepare_state (PState_DFS state, int f)
{
  for (int i = 0; i < LEN * LEN; ++i)
  {
    state->seen[i] = 0;
  }
  state->begin = 0;
  state->end = 0;
  push (state, X(f), Y(f));
}
int not_done (PState_DFS state)
{
  return state->begin < state->end;
}
int not_seen (PState_DFS state, int x, int y)
{
  return !state->seen[LIN (x, y)];
}

typedef struct
{
  uint8_t player;
  uint8_t winner;
  uint8_t* taken;
} *PPosition_type;

PPosition_type new_position()
{
  PPosition_type pos = malloc (sizeof (*pos));
  pos->player = R;
  pos->winner = N;
  pos->taken = (uint8_t*) malloc (LEN * LEN * sizeof (uint8_t));
  if (!pos->taken)
  {
    fprintf (stderr, "could not allocate memory\n");
    exit (EXIT_FAILURE);
  }
  for (int i = 0; i < LEN * LEN; ++i)
  {
    pos->taken[i] = N;
  }
  return pos;
}
void release_position (PPosition_type pos)
{
  free (pos->taken);
  free (pos);
}

static unsigned long _cnt_put = 0;
static unsigned long _cnt_ins = 0;
static unsigned long _cnt_hit = 0;

void unput (PPosition_type pos, int f)
{
  pos->winner = N;
  pos->taken[f] = N;
  pos->player = 1 - pos->player;
}

uint8_t winner_from (PPosition_type pos, PState_DFS state, int f)
{
  prepare_state (state, f);

  int mi = (pos->player == L) ? X(f) : Y(f);
  int ma = (pos->player == L) ? X(f) : Y(f);

  while (not_done (state))
  {
    int const px = pop (state);
    int const py = pop (state);

#define DO(dx, dy)                                                      \
    if (  (px + dx) >= 0 && (px + dx) <= SIZE                           \
       && (py + dy) >= 0 && (py + dy) <= SIZE                           \
       && pos->taken[LIN ((px + dx), (py + dy))] == pos->player         \
       )                                                                \
    {                                                                   \
      if (not_seen (state, px + dx, py + dy))                           \
      {                                                                 \
        mi = MIN (mi, (pos->player == L) ? (px + dx) : (py + dy));      \
        ma = MAX (ma, (pos->player == L) ? (px + dx) : (py + dy));      \
                                                                        \
        if (mi == 0 && ma == SIZE)                                      \
        {                                                               \
          return pos->player;                                           \
        }                                                               \
                                                                        \
        push (state, px + dx, py + dy);                                 \
      }                                                                 \
    }                                                                   \

    DO ( 0, 1);
    DO ( 1, 1);
    DO ( 1, 0);
    DO ( 0,-1);
    DO (-1,-1);
    DO (-1, 0);

#undef DO
  }

  return N;
}

void put (PPosition_type pos, PState_DFS state, int f)
{
  if (++_cnt_put % 1000000 == 0)
  {
    fprintf (stderr, "...put %lu\n", _cnt_put);
  };

  pos->winner = winner_from (pos, state, f);
  pos->taken[f] = pos->player;
  pos->player = 1 - pos->player;
}

void show (PPosition_type pos)
{
  printf ("%s%s\n", show_player[pos->player], show_player[pos->winner]);

  for (int x = 0; x <= 2 * SIZE; ++x)
  {
    for (int y = -2 * SIZE; y <= 2 * SIZE; ++y)
    {
      int const qx = (2 * x + y) / 4;
      int const rx = (2 * x + y) % 4;

      int const qy = (2 * x - y) / 4;
      int const ry = (2 * x - y) % 4;

      if (  rx == 0
         && ry == 0
         && qx >= 0 && qx <= SIZE
         && qy >= 0 && qy <= SIZE
         )
      {
        printf ("%s", show_player[pos->taken[LIN (qx, qy)]]);
      }
      else
      {
        printf (" ");
      }
    }
    printf ("\n");
  }
}

Word_t encode (PPosition_type pos)
{
  Word_t Index = 0;

  for (int i = 0; i < LEN * LEN; ++i)
  {
    Index <<= 2;
    Index += pos->taken[i];
  }

  Word_t Mirror = 0;

  for (int i = LEN * LEN - 1; i >= 0; --i)
  {
    Mirror <<= 2;
    Mirror += pos->taken[i];

    assert (LEN * LEN - 1 - i == LIN (SIZE - X(i), SIZE - Y(i)));
  }

  return MIN (Index, Mirror);
}

PPosition_type decode (Word_t Index, Word_t Value)
{
  int count[3] = {0,0,0};
  PPosition_type pos = new_position();

  for (int i = 0; i < LEN * LEN; ++i)
  {
    ++count[Index & 3];
    pos->taken[LEN * LEN - i - 1] = Index & 3;
    Index >>= 2;
  }
  pos->winner = Value;
  pos->player = (count[0] > count[1]) ? R : L;

  return pos;
}

void save_pjarray (Pvoid_t PJArray);
static int loading = 0;

#define INS(k,v)                                          \
  do                                                      \
  {                                                       \
    PWord_t PValue;                                       \
                                                          \
    JLI (PValue, *PJArray, k);                            \
                                                          \
    if (PValue == PJERR)                                  \
    {                                                     \
      fprintf (stderr, "JHSI-Error: Out of memory...\n"); \
                                                          \
      exit (EXIT_FAILURE);                                \
    }                                                     \
                                                          \
    *PValue = v;                                          \
                                                          \
    ++_cnt_ins;                                           \
                                                          \
    if (!loading && _cnt_ins % 10000000 == 0)             \
    {                                                     \
      fprintf (stderr, "saving to disk\n");               \
      save_pjarray (*PJArray);                            \
    }                                                     \
                                                          \
  } while (0)

uint8_t _winning (PPosition_type pos, PState_DFS state, Pvoid_t* PJArray)
{
  if (pos->winner != N)
  {
    return 1;
  }

  Word_t const Index = encode (pos);

  {
    PWord_t PValue;

    JLG (PValue, *PJArray, Index);

    if (PValue)
    {
      ++_cnt_hit;

      return *PValue != pos->player;
    }
  }

  for (int f = 0; f < LEN * LEN; ++f)
  {
    if (pos->taken[f] == N)
    {
      put (pos, state, f);
      const uint8_t w = _winning (pos, state, PJArray);
      unput (pos, f);

      if (w)
      {
        INS (Index, pos->player);

        return 0;
      }
    }
  }

  INS (Index, 1 - pos->player);

  return 1;
}

#define WRITE(d)                                                        \
  size_t w = fwrite (buf[d], sizeof (Word_t), buf_pos[d], dat[d]);      \
                                                                        \
  if (w != buf_pos[d])                                                  \
  {                                                                     \
    fprintf (stderr, "write error: %s\n", strerror (errno));            \
  }                                                                     \
                                                                        \
  buf_pos[d] = 0

void save_pjarray (Pvoid_t PJArray)
{
  FILE* dat[2];

  int const buf_size = (1 << 20);

  PWord_t buf[2];
  int buf_pos[2] = {0,0};

  for (uint8_t d = L; d < N; ++d)
  {
    char fname[100 + 1];

    snprintf (fname, 100, "hex.%i.%s.dat", SIZE, show_player[d]);

    dat[d] = fopen (fname, "wb+");

    if (!dat[d])
    {
      fprintf ( stderr, "could not open dat file %s: %s\n"
              , fname, strerror (errno)
              );

      exit (EXIT_FAILURE);
    }

    buf[d] = malloc (buf_size * sizeof (Word_t));

    if (!buf[d])
    {
      fprintf (stderr, "could not allocate memory\n");
      exit (EXIT_FAILURE);
    }
  }

  Word_t Index = 0;
  PWord_t PValue;

  JLF (PValue, PJArray, Index);

  while (PValue)
  {
    buf[*PValue][buf_pos[*PValue]++] = Index;

    if (buf_pos[*PValue] == buf_size)
    {
      WRITE (*PValue);
    }

    JLN (PValue, PJArray, Index);
  }

  for (uint8_t d = L; d < N; ++d)
  {
    WRITE (d);
    free (buf[d]);
    fclose (dat[d]);
  }
}
#undef WRITE

int load_pjarray (Pvoid_t* PJArray)
{
  int const buf_size = (1 << 20);

  for (uint8_t d = L; d < N; ++d)
  {
    char fname[100 + 1];

    snprintf (fname, 100, "hex.%i.%s.dat", SIZE, show_player[d]);

    FILE* dat = fopen (fname, "r");

    if (!dat)
    {
      fprintf ( stderr, "could not open dat file %s: %s\n"
              , fname, strerror (errno)
              );

      return 2;
    }

    fprintf (stderr, "loading data from %s...\n", fname);

    PWord_t buf = malloc (buf_size * sizeof (Word_t));

    if (!buf)
    {
      fprintf (stderr, "could not allocate memory\n");

      return 1;
    }

    size_t r;

    while ((r = fread (buf, sizeof (Word_t), buf_size, dat)) > 0)
    {
      for (size_t i = 0; i < r; ++i)
      {
        INS (buf[i], d);
      }
    }

    free (buf);
    fclose (dat);
  }

  fprintf (stderr, "retrieved %lu games\n", _cnt_ins);

  return 0;
}
#undef INS

int main()
{
  Pvoid_t PJArray = (Pvoid_t) NULL;
  PPosition_type pos = new_position();
  PState_DFS state = new_state();

  loading = 1;
  load_pjarray (&PJArray);
  loading = 0;

  for (int f = 0; f < LEN * LEN; ++f)
  {
    if (pos->taken[f] == N)
    {
      put (pos, state, f);
      if (_winning (pos, state, &PJArray))
      {
        show (pos);
      }
      unput (pos, f);
    }
  }

  save_pjarray (PJArray);

  Word_t Rc_word;

  JLFA (Rc_word, PJArray);

  printf ( "put %lu ins %lu hit %lu Judy-bytes %lu\n"
         , _cnt_put, _cnt_ins, _cnt_hit, Rc_word
         );

  release_position (pos);
  release_state (state);
}
