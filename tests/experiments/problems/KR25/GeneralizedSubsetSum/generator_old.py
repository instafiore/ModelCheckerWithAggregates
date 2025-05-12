#!/usr/bin/env python3

def smt(x,y,k):
    out = open("gss-%d-%d-%d.smt" % (len(x), len(y), k), "w")
    for i in range(0,len(x)):
        print("(declare-const x%d Bool)" % (i+1,), file=out)

    print("(assert (forall (%s) (not (= (+ %s %s) %d))))" % (
        " ".join(["(y%d Bool)" % (i+1,) for i in range(0,len(y))]),
        " ".join(["(ite x%d %d 0)" % (i+1,x[i]) for i in range(0,len(x))]),
        " ".join(["(ite y%d %d 0)" % (i+1,y[i]) for i in range(0,len(y))]),
        k
    ), file=out)

    print("(check-sat-using (then qe smt))", file=out)
    print("(get-model)", file=out)


def asp(x,y,k):
    out = open("gss-%d-%d-%d.asp" % (len(x), len(y), k), "w")
    for i in range(0,len(x)):
        print("var(exists,x%d,%d)." % (i+1, x[i]), file=out)

    for i in range(0,len(y)):
        print("var(all,y%d,%d)." % (i+1, y[i]), file=out)

    print("k(%d)." % (k,), file=out)
    
    print("""
        {true(exists,X,C)} :- var(exists,X,C).
        :- not saturate.
        true(all,X,C) :- var(all,X,C), saturate.
        saturate :- k(K), #sum{C, exists,X : true(exists,X,C); C, all,X : true(all,X,C)} != K.
    """, file=out)

def hex(x,y,k):
    out = open("gss-%d-%d-%d.hex" % (len(x), len(y), k), "w")
    for i in range(0,len(x)):
        print("var(exists,x%d,%d)." % (i+1, x[i]), file=out)

    for i in range(0,len(y)):
        print("var(all,y%d,%d)." % (i+1, y[i]), file=out)

    print("k(%d)." % (k,), file=out)
    
    print("""
        true(0,C,X) v false(0,C,X) :- var(exists,X,C).
        :- not saturate.
        true(0,C,X) :- var(all,X,C), saturate.
        saturate :- &sumD0[true](1).
        true(1,K,k) :- k(K).
    """, file=out)


if __name__ == "__main__":
    for i in range(2,40,3):
        x = range(1,i+1)
        y = range(1,i+1)
        k = i * (i+1) / 2
        for j in [0,2,5,10]:
            # smt(x,y,k + int(k * j / 100))
            asp(x,y,k + int(k * j / 100))
            # hex(x,y,k + int(k * j / 100))

