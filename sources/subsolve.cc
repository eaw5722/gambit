//
// FILE: subsolve.cc -- Implementation of solve-by-subgame algorithms
//
// $Id$
//

// This file is a mess as far as Borland instantiations are concerned. The
// positions of each #pragma option and instantiations are critical.

#include "efg.h"
#include "efgutils.h"
#include "nfg.h"
#include "nfstrat.h"
#include "gwatch.h"
#include "rational.h"

#include "subsolve.imp"




template class SubgameSolver<double>;
template class SubgameSolver<gRational>;


#include "garray.imp"

template class gArray<gArray<Infoset *> *>;

template bool operator==(const gArray<EFOutcome *> &, const gArray<EFOutcome *> &);
template bool operator!=(const gArray<EFOutcome *> &, const gArray<EFOutcome *> &);

template gOutput &operator<<(gOutput &, const gArray<EFOutcome *> &);


#include "glist.imp"

template class gList<gVector<double> >;
template class gNode<gVector<double> >;
template class gList<gVector<gRational> >;
template class gNode<gVector<gRational> >;

template class gList<EFOutcome *>;
template class gNode<EFOutcome *>;

template class gList<gArray<EFOutcome *> >;
template class gNode<gArray<EFOutcome *> >;

