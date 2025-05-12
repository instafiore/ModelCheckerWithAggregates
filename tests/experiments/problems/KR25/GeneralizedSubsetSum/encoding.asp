{true(exists,X,C)} :- var(exists,X,C).
:- not saturate.
true(all,X,C) :- var(all,X,C), saturate.
saturate :- k(K), #sum{C, exists,X : true(exists,X,C); C, all,X : true(all,X,C)} != K.