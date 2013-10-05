#include <iostream>

#ifndef SIZE
#define SIZE 2
#endif

#define LEN (SIZE + 1)
#define LIN(x, y) ((x) * LEN + (y))
#define X(f) ((f) / LEN)
#define Y(f) ((f) % LEN)

typedef uint8_t player_type;

#define L 0
#define R 1
#define N 2

static char const* const show_player[3] = {"L", "R", "."};
static player_type _player = L;
static player_type _winner = N;

static unsigned long _cnt_put = 0;
static unsigned long _cnt_unput = 0;

class position_type
{
public:
  position_type ()
    : _taken (new player_type[LEN * LEN])
    , _seen (new bool[LEN * LEN])
    , _open (new int[LEN * LEN])
  {
    for (int i (0); i < LEN * LEN; ++i)
    {
      _taken[i] = N;
    }
  }
  ~position_type()
  {
    delete[] _taken;
    delete[] _seen;
    delete[] _open;
  }
  player_type stone (int v) const
  {
    return _taken[v];
  }

  void put (int f)
  {
    if (++_cnt_put % 1000000 == 0)
    {
      std::cout << "...put " << _cnt_put << std::endl;

      if (_cnt_put >= 100000000)
      {
        exit (0);
      }
    };

    _winner = winner_from (f);
    _taken[f] = _player;
    _player = 1 - _player;
  }
  void unput (int f)
  {
    ++_cnt_unput;
    _winner = N;
    _taken[f] = N;
    _player = 1 - _player;
  }

private:
  player_type winner_from (int f) const
  {
    std::fill (_seen, _seen + LEN * LEN, false);

    int mi ((_player == L) ? X(f) : Y(f));
    int ma ((_player == L) ? X(f) : Y(f));

    int pos (0);
    int end (0);

    _open[end++] = X(f);
    _open[end++] = Y(f);

    _seen[f] = true;

    while (pos < end)
    {
      int const px (_open[pos++]);
      int const py (_open[pos++]);

#define DO(dx, dy)                                                      \
      if (  (px + dx) >= 0 && (px + dx) <= SIZE                         \
         && (py + dy) >= 0 && (py + dy) <= SIZE                         \
         && _taken[LIN ((px + dx), (py + dy))] == _player               \
         )                                                              \
      {                                                                 \
        if (!_seen[LIN ((px + dx), (py + dy))])                         \
        {                                                               \
          mi = std::min ( mi                                            \
                        , (_player == L) ? (px + dx) : (py + dy)        \
                        );                                              \
          ma = std::max ( ma                                            \
                        , (_player == L) ? (px + dx) : (py + dy)        \
                        );                                              \
                                                                        \
          if (mi == 0 && ma == SIZE)                                    \
          {                                                             \
            return _player;                                             \
          }                                                             \
                                                                        \
          _open[end++] = (px + dx);                                     \
          _open[end++] = (py + dy);                                     \
                                                                        \
          _seen[LIN ((px + dx), (py + dy))] = true;                     \
        }                                                               \
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

  player_type* _taken;

  bool* _seen;
  int* _open;
};

std::ostream& operator<< (std::ostream& os, position_type const& pos)
{
  os << show_player[_player]
     << show_player[_winner]
     << std::endl;

  for (int x (0); x <= 2 * SIZE; ++x)
  {
    for (int y (-2 * SIZE); y <= 2 * SIZE; ++y)
    {
      int const qx ((2 * x + y) / 4);
      int const rx ((2 * x + y) % 4);

      int const qy ((2 * x - y) / 4);
      int const ry ((2 * x - y) % 4);

      if (  rx == 0
         && ry == 0
         && qx >= 0 && qx <= SIZE
         && qy >= 0 && qy <= SIZE
         )
      {
        os << show_player[pos.stone (LIN (qx, qy))];
      }
      else
      {
        os << " ";
      }
    }
    os << std::endl;
  }

  return os;
}

bool _winning (position_type& pos)
{
  if (_winner != N)
  {
    return true;
  }

  for (int f (0); f < LEN * LEN; ++f)
  {
    if (pos.stone (f) == N)
    {
      pos.put (f);
      const bool w (_winning (pos));
      pos.unput (f);

      if (w)
      {
        return false;
      }
    }
  }

  return true;
}

int main()
{
  position_type b;

  for (int f (0); f < LEN * LEN; ++f)
  {
    if (b.stone (f) == N)
    {
      b.put (f);
      if (_winning (b))
      {
        std::cout << b << std::endl;
      }
      b.unput (f);
    }
  }

  printf ("put %lu unput %lu\n", _cnt_unput, _cnt_unput);
}
