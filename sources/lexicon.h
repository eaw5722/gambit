//
// FILE: lexicon.h -- Declaration of the efg<->nfg strategy correspondence
//
// $Id$
//

#ifndef LEXICON_H
#define LEXICON_H


#include "glist.h"

typedef gArray<int> Correspondence;

template <class T> class Lexicon   {
  public:
    Nfg<T> *N;
    gArray<gList<Correspondence *> > strategies;

    Lexicon(const Efg<T> &);
    ~Lexicon();

    void MakeStrategy(class EFPlayer *p);
    void MakeReducedStrats(const class EFSupport &, class EFPlayer *, class Node *, class Node *);
    void MakeLink(Efg<T> *, Nfg<T> *);
};


#endif   // LEXICON_H
