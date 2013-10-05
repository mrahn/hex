#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>

#include <boost/foreach.hpp>

#ifndef SIZE
#define SIZE 2
#endif

enum player_type { L, R, N };

std::ostream& operator<< (std::ostream& os, const player_type& pl)
{
  switch (pl)
  {
  case L: return os << "L";
  case R: return os << "R";
  default: return os << ".";
  }
}
player_type other (player_type pl)
{
  switch (pl)
  {
  case L: return R;
  case R: return L;
  default: throw std::runtime_error ("other (N)");
  }
}

class position_type
{
public:
  position_type ()
    : _player (L)
    , _winner (N)
    , _taken (new player_type[(SIZE + 1) * (SIZE + 1)])
    , _cnt_put (0)
    , _cnt_unput (0)
    , _seen (new bool[(SIZE + 1) * (SIZE + 1)])
    , _open (new int[2 * (SIZE + 1) * (SIZE + 1)])
  {
    std::fill (_taken, _taken + (SIZE + 1) * (SIZE + 1), N);
  }
  ~position_type()
  {
    std::cout << "put " << _cnt_put << " unput " << _cnt_unput << std::endl;
    delete[] _taken;
    delete[] _seen;
    delete[] _open;
  }
  player_type player() const
  {
    return _player;
  }
  player_type winner() const
  {
    return _winner;
  }
  player_type stone (int x, int y) const
  {
    return _taken[x * (1 + SIZE) + y];
  }

  void put (int x, int y)
  {
    if (++_cnt_put % 100000 == 0)
    {
      std::cout << "...put " << _cnt_put << std::endl;
    };

    _winner = winner_from (x, y);
    _taken[x * (1 + SIZE) + y] = _player;
    _player = other (_player);
  }
  void unput (int x, int y)
  {
    ++_cnt_unput;
    _winner = N;
    _taken[x * (1 + SIZE) + y] = N;
    _player = other (_player);
  }

private:
  player_type winner_from (int x, int y) const
  {
    std::fill (_seen, _seen + (SIZE + 1) * (SIZE + 1), false);

    int mi ((_player == L) ? x : y);
    int ma ((_player == L) ? x : y);

    int pos (0);
    int end (0);

    _open[end++] = x;
    _open[end++] = y;

    _seen[x * (1 + SIZE) + y] = true;

    while (pos < end)
    {
      int const px (_open[pos++]);
      int const py (_open[pos++]);

#define DO(dx, dy)                                              \
      {                                                         \
        int const nx (px + dx);                                 \
        int const ny (py + dy);                                 \
                                                                \
        if (  nx >= 0 && nx <= SIZE                             \
           && ny >= 0 && ny <= SIZE                             \
           && _taken[nx * (1 + SIZE) + ny] == _player           \
           )                                                    \
        {                                                       \
          if (!_seen[nx * (1 + SIZE) + ny])                     \
          {                                                     \
            mi = std::min (mi, (_player == L) ? nx : ny);       \
            ma = std::max (ma, (_player == L) ? nx : ny);       \
                                                                \
            if (mi == 0 && ma == SIZE)                          \
            {                                                   \
              return _player;                                   \
            }                                                   \
                                                                \
            _open[end++] = nx;                                  \
            _open[end++] = ny;                                  \
                                                                \
            _seen[nx * (1 + SIZE) + ny] = true;                \
          }                                                     \
        }                                                       \
      }

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

  player_type _player;
  player_type _winner;
  player_type* _taken;

  unsigned long _cnt_put;
  unsigned long _cnt_unput;

  bool* _seen;
  int* _open;
};

std::ostream& operator<< (std::ostream& os, position_type const& pos)
{
  os << pos.player() << pos.winner() << std::endl;

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
        os << pos.stone (qx, qy);
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
  if (pos.winner() != N)
  {
    return true;
  }

  for (int x (0); x <= SIZE; ++x)
  {
    for (int y (0); y <= SIZE; ++y)
    {
      if (pos.stone (x, y) == N)
      {
        pos.put (x, y);
        const bool w (_winning (pos));
        pos.unput (x, y);

        if (w)
        {
          return false;
        }
      }
    }
  }

  return true;
}

int main()
{
  position_type b;

  for (int x (0); x <= SIZE; ++x)
  {
    for (int y (0); y <= SIZE; ++y)
    {
      if (b.stone (x, y) == N)
      {
        b.put (x, y);
        if (_winning (b))
        {
          std::cout << b << std::endl;
        }
        b.unput (x, y);
      }
    }
  }
}
