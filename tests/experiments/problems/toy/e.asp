

a | b | c | d | e.
a:- b.
b:- a.

b:- #sum{1, c : c; 1, d: d} >= 2.
d:- a.
c:- d.

c:- e.
e:- c.




