#include <algorithm>
#include <iostream>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

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

std::size_t hash_value (point_type const& p)
{
  return boost::hash<std::pair<int,int>>()(std::make_pair (p.x(), p.y()));
}
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
point_type rect_coord (point_type const& p)
{
  return point_type ( 2 * (p.x() - p.y())
                    ,      p.x() + p.y()
                    );
}
point_type hex_coord (point_type const& p)
{
  return point_type ( (2 * p.y() + p.x()) / 4
                    , (2 * p.y() - p.x()) / 4
                    );
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
points_type _directions()
{
  points_type d;
  d.push_back (point_type (0,1));
  d.push_back (point_type (1,1));
  d.push_back (point_type (1,0));
  return d;
}
points_type directions()
{
  static points_type d (_directions());
  return d;
}

points_type neighbour (point_type const& p)
{
  points_type n;

  BOOST_FOREACH (point_type const& d, directions())
  {
    n.push_back (p + d);
    n.push_back (p - d);
  }

  return n;
}
bool in_range (int s, int v)
{
  return v >= 0 && v <= s;
}
points_type neighbourN (int s, point_type const& p)
{
  static boost::unordered_map<point_type, points_type> c;

  auto known (c.find (p));

  if (known != c.end())
  {
    return known->second;
  }

  points_type n (neighbour (p));
  points_type nn (n.size());

  auto it
    (std::copy_if ( n.begin(), n.end(), nn.begin()
                  , [s](point_type const& a)
                    {
                      return in_range (s, a.x()) && in_range (s, a.y());
                    }
                  )
    );
  nn.resize (std::distance (nn.begin(), it));

  c.insert (std::make_pair (p, nn));

  return nn;
}

enum player_type { L, R };

std::ostream& operator<< (std::ostream& os, const player_type& pl)
{
  return os << ((pl == L) ? "L" : "R");
}
player_type other (player_type pl)
{
  return (pl == L) ? R : L;
}
int proj (player_type pl, point_type const& p)
{
  return (pl == L) ? p.x() : p.y();
}

class position_type
{
public:
  position_type (int size)
    : _size (size)
    , _player (L)
    , _winner (boost::none)
    , _taken ()
    , _cnt_put (0)
    , _cnt_unput (0)
  {}
  ~position_type()
  {
    std::cout << "put " << _cnt_put << " unput " << _cnt_unput << std::endl;
  }
  int size() const
  {
    return _size;
  }
  player_type player() const
  {
    return _player;
  }
  boost::optional<player_type> winner() const
  {
    return _winner;
  }
  boost::optional<player_type> stone (point_type const& p) const
  {
    auto it (_taken.find (p));

    if (it != _taken.end())
    {
      return it->second;
    }

    return boost::none;
  }

  void put (point_type const& f)
  {
    if (++_cnt_put % 100000 == 0)
    {
      std::cout << "...put " << _cnt_put << std::endl;
    };

    int mi (proj (_player, f));
    int ma (proj (_player, f));

    BOOST_FOREACH (point_type const& p, component (f))
    {
      mi = std::min (mi, proj (_player, p));
      ma = std::max (ma, proj (_player, p));
    }

    if (mi == 0 && ma == size())
    {
      _winner = _player;
    }

    _taken.insert (std::make_pair (f, _player));
    _player = other (_player);
  }
  void unput (point_type const& f)
  {
    ++_cnt_unput;
    _winner = boost::none;
    _taken.erase (f);
    _player = other (_player);
  }

private:
  points_type component (point_type const& f) const
  {
    points_type c;
    std::vector<point_type> open;
    boost::unordered_set<point_type> seen;

    open.push_back (f);

    while (not (open.empty()))
    {
      c.push_back (open.back()); open.pop_back();

      BOOST_FOREACH (point_type const& n, neighbourN (size(), c.back()))
      {
        auto n_pl (stone (n));

        if (n_pl && *n_pl == player() && seen.insert (n).second)
        {
          open.push_back (n);
        }
      }
    }

    return c;
  }

  int _size;
  player_type _player;
  boost::optional<player_type> _winner;
  boost::unordered_map<point_type, player_type> _taken;

  unsigned long _cnt_put;
  unsigned long _cnt_unput;
};

std::ostream& operator<< (std::ostream& os, position_type const& pos)
{
  os << pos.player();

  if (pos.winner())
  {
    os << *pos.winner();
  }
  else
  {
    os << "?";
  }

  os << std::endl;

  boost::unordered_set<point_type> rc;
  int m (0);

  BOOST_FOREACH (point_type const& f, board (pos.size()))
  {
    point_type const r (rect_coord (f));

    rc.insert (r);

    m = std::max (m, std::max (abs (r.x()), abs (r.y())));
  }

  for (int x (0); x <= m; ++x)
  {
    for (int y (-m); y <= m; ++y)
    {
      point_type const p (y, x);
      point_type const h (hex_coord (p));

      if (rc.count (p))
      {
        boost::optional<player_type> const pl (pos.stone (h));

        if (pl)
        {
          os << *pl;
        }
        else
        {
          os << ".";
        }
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
  if (pos.winner())
  {
    return true;
  }

  BOOST_FOREACH (point_type const& f, board (pos.size()))
  {
    if (not (pos.stone (f)))
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
  position_type b (2);

  BOOST_FOREACH (point_type const& f, board (b.size()))
  {
    if (not (b.stone (f)))
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
