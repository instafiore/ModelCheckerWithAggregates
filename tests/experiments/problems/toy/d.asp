a:- b.
b:- a.
% a :- not k.
b:- c, d.
d:- a.
c:- d.

c:- e.
e:- c.

% {k}.

a | b | c | d | e.
