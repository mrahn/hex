
import qualified Data.Maybe (isNothing, maybe)
import qualified Data.Set (empty, member, insert)
import qualified Data.List (nub, sort, genericLength)
import qualified Data.Map (Map, empty, insert, lookup, mapKeys)

ap2 f (x,y) = (f x, f y)

direction = [(0,1),(1,1),(1,0)]
neighbour_direction = direction ++ map (ap2 negate) direction

shift (a,b) (c,d) = (a + c, b + d)

neighbour = zipWith shift neighbour_direction . repeat

neighbourN n = filter (uncurry (&&) . ap2 valid) . neighbour
  where valid c = and [c >= 0, c <= n]

board n = [ (x,y) | x <- [0..n], y <- [0..n] ]

rect_coord (a,b) = (2 * (a - b), a + b)
hex_coord (a,b) = (div (2 * b + a) 4, div (2 * b - a) 4)

prop_coord p = p == (hex_coord . rect_coord $ p)

on_rect_board f g b = [0..m] >>= \ x -> return $ [-m..m] >>= \ y -> do
  let p = (y,x)
  let h = hex_coord p
  return $ if elem p rc then f h else g h
  where m = maximum . map (uncurry max . ap2 abs) $ rc
        rc = map rect_coord b

data Player = L | R deriving (Eq, Ord, Read, Show)

other L = R
other R = L

proj L = fst
proj R = snd

data Position = Position
                { size :: Int
                , player :: Player
                , winner :: Maybe Player
                , taken :: Data.Map.Map (Int,Int) Player
                }
                deriving (Eq, Ord)

empty n = Position n L Nothing Data.Map.empty

mirror p = p { taken = Data.Map.mapKeys swap (taken p) }
  where swap (x,y) = (size p - x,size p - y)
rotate p = p { taken = Data.Map.mapKeys swap (taken p) }
  where swap (x,y) = (y,x)

normal p = minimum [p, mirror p]

stone p = flip Data.Map.lookup (taken p)
fields = board . size

instance Show Position where
  show p = the_player . the_result . nl . unlines . map concat
    . on_rect_board nice (const " ") . fields $ p
    where nice = Data.Maybe.maybe "." show . stone p
          the_player = (++) (show . player $ p)
          the_result = (++) (case winner p of Just pl -> show pl; _ -> "?")
          nl = (++) "\n"

friend p = filter ((==) (Just . player $ p) . stone p) . neighbourN (size p)

component p f = componentC Data.Set.empty [f]
  where componentC cache (x:xs) | Data.Set.member x cache = componentC cache xs
        componentC cache (x:xs) =
          x : componentC (Data.Set.insert x cache) (xs ++ friend p x)
        componentC _ _ = []

bound f = foldr1 (\ (a,b) (c,d) -> (f a c, f b d))

won pl p f = let c = component p f
                 low = (proj pl) (bound min c)
                 high = (proj pl) (bound max c)
            in and [low == 0, high == size p]

put p f = p { player = other (player p)
            , winner = if won (player p) p f then Just (player p) else Nothing
            , taken = Data.Map.insert f (player p) (taken p)
            }
puts p = foldl put p

free p = filter (Data.Maybe.isNothing . stone p) . fields $ p

suc p | Data.Maybe.isNothing (winner p) = map (put p) (free p)
suc _ = []

positions p = case suc p of
  [] -> [[p]]
  ps -> map (p:) $ concatMap (positions) ps

winning p = filter is_winner . suc $ p
  where is_winner c = case (winner c) of
          Just pl -> pl == player p
          Nothing -> null . winning $ c

onub = onub' Data.Set.empty
  where onub' cache (x:xs)
          | Data.Set.member x cache = onub' cache xs
        onub' cache (x:xs) = x : onub' (Data.Set.insert x cache) xs
        onub' _ _ = []

level = concat . map suc
levelO = onub . level

main = mapM_ print
     $ zip [0..]
     $ scanl (\(_,s) x -> (length x, length x + s)) (0,0)
     $ takeWhile (not . null)
     $ iterate levelO [empty 2]
