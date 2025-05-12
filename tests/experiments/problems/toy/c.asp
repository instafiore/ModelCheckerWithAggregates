a:- b, not k.
b:- a.
b:- c, d, not a.
d:- a.
c:- d.
b :- #sum{ 1: a; 2: b} >= 2.

c:- e.
e:- c.
a :- not not a.
a | b | c | d | e.
{k}.