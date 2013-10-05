#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>

#include <boost/foreach.hpp>

class point_type
{
public:
  explicit point_type()
    : _x()
    , _y()
  {}
  explicit point_type (int x, int y)
    : _x (x)
    , _y (y)
  {}
  int x() const
  {
    return _x;
  }
  int y() const
  {
    return _y;
  }
private:
  int _x;
  int _y;
};

bool operator== (point_type const& a, point_type const& b)
{
  return a.x() == b.x() && a.y() == b.y();
}
std::ostream& operator<< (std::ostream& os, point_type const& p)
{
  return os << "(" << p.x() << "," << p.y() << ")";
}
point_type operator+ (point_type const& a, point_type const& b)
{
  return point_type (a.x() + b.x(), a.y() + b.y());
}
point_type operator- (point_type const& a, point_type const& b)
{
  return point_type (a.x() - b.x(), a.y() - b.y());
}

typedef std::vector<point_type> points_type;

points_type board (int s)
{
  points_type b;

  for (int x (0); x <= s; ++x)
  {
    for (int y (0); y <= s; ++y)
    {
      b.push_back (point_type (x, y));
    }
  }

  return b;
}

bool in_range (int s, int v)
{
  return v >= 0 && v <= s;
}
bool in_range (int s, point_type const& p)
{
  return in_range (s, p.x()) && in_range (s, p.y());
}

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
int proj (player_type pl, point_type const& p)
{
  switch (pl)
  {
  case L: return p.x();
  case R: return p.y();
  default: throw std::runtime_error ("proj (N)");
  }
}

class position_type
{
public:
  position_type (int size)
    : _size (size)
    , _player (L)
    , _winner (N)
    , _taken (new player_type[(_size + 1) * (_size + 1)])
    , _cnt_put (0)
    , _cnt_unput (0)
    , _seen (new bool[(_size + 1) * (_size + 1)])
  {
    std::fill (_taken, _taken + (_size + 1) * (_size + 1), N);
  }
  ~position_type()
  {
    std::cout << "put " << _cnt_put << " unput " << _cnt_unput << std::endl;
    delete[] _taken;
    delete[] _seen;
  }
  int size() const
  {
    return _size;
  }
  player_type player() const
  {
    return _player;
  }
  player_type winner() const
  {
    return _winner;
  }
  player_type stone (point_type const& p) const
  {
    return _taken[p.x() * (1 + _size) + p.y()];
  }

  void put (point_type const& f)
  {
    if (++_cnt_put % 100000 == 0)
    {
      std::cout << "...put " << _cnt_put << std::endl;
    };

    _winner = winner_from (f);
    _taken[f.x() * (1 + _size) + f.y()] = _player;
    _player = other (_player);
  }
  void unput (point_type const& f)
  {
    ++_cnt_unput;
    _winner = N;
    _taken[f.x() * (1 + _size) + f.y()] = N;
    _player = other (_player);
  }

private:
  player_type winner_from (point_type const& f) const
  {
    std::vector<point_type> open;

    std::fill (_seen, _seen + (_size + 1) * (_size + 1), false);

    int mi (proj (_player, f));
    int ma (proj (_player, f));

    open.push_back (f);

    while (not (open.empty()))
    {
      point_type const p (open.back()); open.pop_back();

#define DO(d...)                                                \
      {                                                         \
        point_type const n (p + point_type (d));                \
                                                                \
        if (in_range (_size, n) && stone (n) == player())       \
        {                                                       \
          if (!_seen[n.x() * (1 + _size) + n.y()])              \
          {                                                     \
            _seen[n.x() * (1 + _size) + n.y()] = true;          \
                                                                \
            mi = std::min (mi, proj (_player, n));              \
            ma = std::max (ma, proj (_player, n));              \
                                                                \
            if (mi == 0 && ma == _size)                         \
            {                                                   \
              return _player;                                   \
            }                                                   \
                                                                \
            open.push_back (n);                                 \
          }                                                     \
        }                                                       \
      }

      DO ( 0, 1);
      DO ( 1, 1);
      DO ( 1, 0);
      DO ( 0,-1);
      DO (-1,-1);
      DO (-1, 0);
    }

    return N;
  }

  int _size;
  player_type _player;
  player_type _winner;
  player_type* _taken;

  unsigned long _cnt_put;
  unsigned long _cnt_unput;

  bool* _seen;
};

std::ostream& operator<< (std::ostream& os, position_type const& pos)
{
  os << pos.player() << pos.winner() << std::endl;

  for (int x (0); x <= 2 * pos.size(); ++x)
  {
    for (int y (-2 * pos.size()); y <= 2 * pos.size() ; ++y)
    {
      point_type const p (y, x);
      point_type const q ( (2 * p.y() + p.x()) / 4
                         , (2 * p.y() - p.x()) / 4
                         );
      point_type const r ( (2 * p.y() + p.x()) % 4
                         , (2 * p.y() - p.x()) % 4
                         );

      if (r.x() == 0 && r.y() == 0 && in_range (pos.size(), q))
      {
        os << pos.stone (q);
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

  BOOST_FOREACH (point_type const& f, board (pos.size()))
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

int main (int argc, char** argv)
{
  position_type b ((argc > 1) ? atoi (argv[1]) : 2);

  BOOST_FOREACH (point_type const& f, board (b.size()))
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
}
