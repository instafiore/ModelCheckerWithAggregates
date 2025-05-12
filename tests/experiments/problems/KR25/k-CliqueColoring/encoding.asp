1 {assign(X,C) : colour(C)} 1 :- node(X).

:- not saturate.
in(X) :- id_3(X).
id_3(X) :- node(X), saturate.

saturate :- id_1.
id_1 :- in(X), in(Y), X != Y, not link(X,Y).

saturate :- node(X), N = #count{Y:node(Y)},
        #sum{2*N, saturate: saturate;
            -N, X: in(X);
            -1, Y: in(Y), node(Y), Y != X;
            1, Y: in(Y), link(X,Y)
        } >= 0.

% saturate :- in(X), in(Y), assign(X,CX), assign(Y,CY), CX != CY.
saturate :- id_2.
id_2 :- in(X), in(Y), assign(X,CX), assign(Y,CY), CX != CY.


link(X,Y) :- link(Y,X).


node(X) :- link(X,Y).
node(Y) :- link(X,Y).