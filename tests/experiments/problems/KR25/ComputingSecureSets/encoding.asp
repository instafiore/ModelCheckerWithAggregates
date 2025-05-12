edge(X,Y) :- edge(Y,X).
vertex(X) :- edge(X,_).

1 {s(V) : vertex(V)}.

x(V) :- id_1(V).
id_1(V) :- s(V), saturate.

:- not saturate.

% satisfy inequality 
pos(V) :- x(V).
pos(V) :- id_2(V).
id_2(V) :- edge(U,V), x(U), s(V).

neg(V) :- id_3(V).
id_3(V) :- edge(U,V), x(U), not s(V).

x(U) : edge(U,V) :- neg(V).

saturate :-  #sum{1, U: pos(U); -1, U: neg(U)} >= 0.
