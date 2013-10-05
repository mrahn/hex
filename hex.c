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

static char const* const show_player[3] = {"L", "R", "."};

static uint8_t _player = R;
static uint8_t _winner = N;

static unsigned long _cnt_put = 0;
static unsigned long _cnt_ins = 0;
static unsigned long _cnt_hit = 0;

static uint8_t* _taken;
static uint8_t* _seen;
static int* _open;
static int _depth = 0;

void unput (int f)
{
  _winner = N;
  _taken[f] = N;
  _player = 1 - _player;
  --_depth;
}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

uint8_t winner_from (int f)
{
  for (int i = 0; i < LEN * LEN; ++i)
  {
    _seen[i] = 0;
  }

  int mi = (_player == L) ? X(f) : Y(f);
  int ma = (_player == L) ? X(f) : Y(f);

  _open[0] = X(f);
  _open[1] = Y(f);

  _seen[f] = 1;

  int pos = 0;
  int end = 2;

  while (pos < end)
  {
    int const px = _open[pos++];
    int const py = _open[pos++];

#define DO(dx, dy)                                              \
    if (  (px + dx) >= 0 && (px + dx) <= SIZE                   \
       && (py + dy) >= 0 && (py + dy) <= SIZE                   \
       && _taken[LIN ((px + dx), (py + dy))] == _player         \
       )                                                        \
    {                                                           \
      if (!_seen[LIN ((px + dx), (py + dy))])                   \
      {                                                         \
        mi = MIN (mi, (_player == L) ? (px + dx) : (py + dy));  \
        ma = MAX (ma, (_player == L) ? (px + dx) : (py + dy));  \
                                                                \
        if (mi == 0 && ma == SIZE)                              \
        {                                                       \
          return _player;                                       \
        }                                                       \
                                                                \
        _open[end++] = (px + dx);                               \
        _open[end++] = (py + dy);                               \
                                                                \
        _seen[LIN ((px + dx), (py + dy))] = 1;                  \
      }                                                         \
    }                                                           \

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

void put (int f)
{
  if (++_cnt_put % 1000000 == 0)
  {
    fprintf (stderr, "...put %lu\n", _cnt_put);
  };

  _winner = winner_from (f);
  _taken[f] = _player;
  _player = 1 - _player;
  ++_depth;
}

void show()
{
  printf ("%s%s\n", show_player[_player], show_player[_winner]);

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
        printf ("%s", show_player[_taken[LIN (qx, qy)]]);
      }
      else
      {
        printf (" ");
      }
    }
    printf ("\n");
  }
}

#define INS(v)                                            \
  do                                                      \
  {                                                       \
    JHSI (PValue, *PJHSArray, _taken, LEN * LEN);         \
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
  } while (0)


    /* if (_depth < 2)                                       \ */
    /* {                                                     \ */
    /*   uint8_t w = _winner;                                \ */
    /*                                                       \ */
    /*   _winner = v;                                        \ */
    /*                                                       \ */
    /*   show();                                             \ */
    /*                                                       \ */
    /*   _winner = w;                                        \ */
    /* }                                                     \ */

uint8_t _winning (Pvoid_t* PJHSArray)
{
  if (_winner != N)
  {
    return 1;
  }

  PWord_t PValue;

  JHSG (PValue, *PJHSArray, _taken, LEN * LEN);

  if (PValue)
  {
    ++_cnt_hit;

    return *PValue == _player;
  }

  for (int f = 0; f < LEN * LEN; ++f)
  {
    if (_taken[f] == N)
    {
      put (f);
      const uint8_t w = _winning (PJHSArray);
      unput (f);

      if (w)
      {
        INS (1 - _player);

        return 0;
      }
    }
  }

  INS (_player);

  return 1;
}

int main()
{
  _taken = (uint8_t*) malloc (LEN * LEN * sizeof (uint8_t));
  _seen = (uint8_t*) malloc (LEN * LEN * sizeof (uint8_t));
  _open = (int*) malloc (LEN * LEN * sizeof (int));

  if (!_taken || !_seen || !_open)
  {
    fprintf (stderr, "could not allocate memory\n");
    exit (EXIT_FAILURE);
  }

  for (int i = 0; i < LEN * LEN; ++i)
  {
    _taken[i] = N;
  }

  Pvoid_t PJHSArray = (Pvoid_t) NULL;

  for (int f = 0; f < LEN * LEN; ++f)
  {
    if (_taken[f] == N)
    {
      put (f);
      if (_winning (&PJHSArray))
      {
        show();
      }
      unput (f);
    }
  }

  Word_t Rc_word;

  JHSFA (Rc_word, PJHSArray);

  printf ( "put %lu ins %lu hit %lu Judy-bytes %lu\n"
         , _cnt_put, _cnt_ins, _cnt_hit, Rc_word
         );

  free (_taken);
  free (_seen);
  free (_open);
}
