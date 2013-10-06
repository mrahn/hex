#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Judy.h>

#ifndef SIZE
#define SIZE 2
#endif

#define LEN (SIZE + 1)
#define LIN(x, y) ((x) * LEN + (y))
#define X(f) ((f) / LEN)
#define Y(f) ((f) % LEN)

#define L 0
#define R 1
#define N 2

typedef struct
{
  uint8_t player;
  uint8_t winner;
  uint8_t* taken;
  uint8_t* seen;
  int* open;
} Position_type, *PPosition_type;

PPosition_type new()
{
  PPosition_type pos = malloc (sizeof (Position_type));
  pos->player = R;
  pos->winner = N;
  pos->taken = (uint8_t*) malloc (LEN * LEN * sizeof (uint8_t));
  pos->seen = (uint8_t*) malloc (LEN * LEN * sizeof (uint8_t));
  pos->open = (int*) malloc (LEN * LEN * sizeof (int));
  for (int i = 0; i < LEN * LEN; ++i)
  {
    pos->taken[i] = N;
  }
  return pos;
}
void release (PPosition_type pos)
{
  free (pos->taken);
  free (pos->seen);
  free (pos->open);
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

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

uint8_t winner_from (PPosition_type pos, int f)
{
  for (int i = 0; i < LEN * LEN; ++i)
  {
    pos->seen[i] = 0;
  }

  int mi = (pos->player == L) ? X(f) : Y(f);
  int ma = (pos->player == L) ? X(f) : Y(f);

  pos->open[0] = X(f);
  pos->open[1] = Y(f);

  pos->seen[f] = 1;

  int begin = 0;
  int end = 2;

  while (begin < end)
  {
    int const px = pos->open[begin++];
    int const py = pos->open[begin++];

#define DO(dx, dy)                                                      \
    if (  (px + dx) >= 0 && (px + dx) <= SIZE                           \
       && (py + dy) >= 0 && (py + dy) <= SIZE                           \
       && pos->taken[LIN ((px + dx), (py + dy))] == pos->player         \
       )                                                                \
      {                                                                 \
        if (!pos->seen[LIN ((px + dx), (py + dy))])                     \
          {                                                             \
            mi = MIN (mi, (pos->player == L) ? (px + dx) : (py + dy));  \
            ma = MAX (ma, (pos->player == L) ? (px + dx) : (py + dy));  \
                                                                        \
            if (mi == 0 && ma == SIZE)                                  \
              {                                                         \
                return pos->player;                                     \
              }                                                         \
                                                                        \
            pos->open[end++] = (px + dx);                               \
            pos->open[end++] = (py + dy);                               \
                                                                        \
            pos->seen[LIN ((px + dx), (py + dy))] = 1;                  \
          }                                                             \
      }                                                                 \

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

void put (PPosition_type pos, int f)
{
  if (++_cnt_put % 1000000 == 0)
  {
    fprintf (stderr, "...put %lu\n", _cnt_put);
  };

  pos->winner = winner_from (pos, f);
  pos->taken[f] = pos->player;
  pos->player = 1 - pos->player;
}

void show (PPosition_type pos)
{
  static char const* const show_player[3] = {"L", "R", "."};

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

  return Index;
}

PPosition_type decode (Word_t Index, Word_t Value)
{
  int count[3] = {0,0,0};
  PPosition_type pos = new();

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

#define INS(v)                                            \
  do                                                      \
  {                                                       \
    PWord_t PValue;                                       \
                                                          \
    JLI (PValue, *PJArray, Index);                        \
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
  } while (0)


uint8_t _winning (PPosition_type pos, Pvoid_t* PJArray)
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
      put (pos, f);
      const uint8_t w = _winning (pos, PJArray);
      unput (pos, f);

      if (w)
      {
        INS (pos->player);

        return 0;
      }
    }
  }

  INS (1 - pos->player);

  return 1;
}

void save_pjarray (Pvoid_t PJArray)
{
  char fname[2][100 + 1];

  snprintf (fname[L], 100, "hex.%i.L.dat", SIZE);
  snprintf (fname[R], 100, "hex.%i.R.dat", SIZE);

  FILE* dat[2] = { fopen (fname[L], "wb+")
                 , fopen (fname[R], "wb+")
                 };

  int const buf_size = 1 << 20;

  PWord_t buf[2] = { malloc (buf_size * sizeof (Word_t))
                   , malloc (buf_size * sizeof (Word_t))
                   };

  int buf_pos[2] = {0,0};

  if (!dat[L] || !dat[R] || !buf[L] || !buf[R])
  {
    fprintf (stderr, "upps\n");
    exit (EXIT_FAILURE);
  }

  Word_t Index = 0;
  PWord_t PValue;

  JLF (PValue, PJArray, Index);

  while (PValue)
  {
    buf[*PValue][buf_pos[*PValue]++] = Index;

    if (buf_pos[*PValue] == buf_size)
    {
      fwrite (buf[*PValue], sizeof (Word_t), buf_pos[*PValue], dat[*PValue]);

      buf_pos[*PValue] = 0;
    }

    JLN (PValue, PJArray, Index);
  }

  fwrite (buf[L], sizeof (Word_t), buf_pos[L], dat[L]);
  fwrite (buf[R], sizeof (Word_t), buf_pos[R], dat[R]);

  free (buf[L]);
  free (buf[R]);
  fclose (dat[L]);
  fclose (dat[R]);
}

int main()
{
  Pvoid_t PJArray = (Pvoid_t) NULL;
  PPosition_type pos = new();

  for (int f = 0; f < LEN * LEN; ++f)
  {
    if (pos->taken[f] == N)
    {
      put (pos, f);
      if (_winning (pos, &PJArray))
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

  release (pos);
}
