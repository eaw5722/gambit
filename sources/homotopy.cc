// Includes //////////////////////////////////////////////////////////////

#include <values.h>
#include <math.h>
#include "homotopy.h"
#include "hompack.h"
#include "gmatrix.h"
#include "nfgciter.h"

HomQreParams::HomQreParams(gStatus &s)
  : FuncMinParams(s), powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01), 
    fullGraph(false), pxifile(&gnull)
{ }

HomQreParams::HomQreParams(gOutput &, gOutput &pxi, gStatus &s)
  : FuncMinParams(s), powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01), 
    fullGraph(false), pxifile(&pxi)
{ }

static void WritePXIHeader(gOutput &pxifile, const Nfg &N,
			   const HomQreParams &params)
{
  pxifile << "Dimensionality:\n";
  pxifile << N.NumPlayers() << " ";
  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    pxifile << N.NumStrats(pl) << " ";
  pxifile << "\n";
  N.WriteNfgFile(pxifile, 6);

  pxifile << "Settings:\n" << params.minLam;
  pxifile << "\n" << params.maxLam << "\n" << params.delLam;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << params.powLam << "\n";
  
  int numcols = N.ProfileLength() + 2;

  pxifile << "DataFormat:\n" << numcols;
  
  for (int i = 1; i <= numcols; i++)
    pxifile << " " << i;
 
  pxifile << "\nData:\n";
}

void HomQre(const Nfg &nfg, HomQreParams &params,
	   const MixedProfile<gNumber> &start,
	   gList<MixedSolution> &solutions,
	   long &nevals, long &nits)
{ 
  NFSupport supp(start.Support());
  echo_payoffs(supp);
  
  int N = supp.TotalNumStrats()-nfg.NumPlayers();
  gVector<double> Y(N+1);
  gVector<double> A(0);
  gVector<double> ssp(8);
  double max_lambda = 1; // Vale Murthy 1/24/00
  
  int flag = -2;
  int jeval_num = 0;
  double arclength = 0;
  
  // initialize lambda to zero
  Y[1] = 0.0;
  
  // initialize Y
  int j = 1;
  for(int pl = 1;pl<=nfg.NumPlayers();pl++)
    for(int i = 1;i < supp.NumStrats(pl);i++) {
      Y[j+1] = start(pl,i);
      j++;
    }

  gout << "\nY: " << Y;  

  // Use default stepsize parameters
  ssp = 0;
  
  if (params.pxifile) 
    WritePXIHeader(*params.pxifile, nfg, params);
  
  fixpnf(supp, params, solutions, N, Y, flag, 1., 1., .1, .1, true,
  	 A, ssp, jeval_num,arclength,max_lambda,false);

  printf("Exiting flag: %d\n",flag);
  printf("Done\n");
}

template <class T> void VectorToProfile(gVector<T> &vec, MixedProfile<gNumber> &sol)
{
  NFSupport &supp(sol);
  int j = 2;
  for(int pl = 1;pl<=supp.Game().NumPlayers();pl++) {
    T resid = 1;
    int i;
    for(i=1;i<supp.NumStrats(pl);i++) {
      sol(pl,i) = Y[j];
      resid -= Y[j];
      j++;
    }
    sol(pl,i) =  resid;
  }
}



// functions /////////////////////////////////////////////////////////////

template <class T> T my_log(T x,T eps)
{
/*
if (x < eps)
 {
 printf("x=%lf\n",x);
 scanf("");
 }
*/

 if (x>= eps) return log(x);
  else return -MAXFLOAT;
// return log(x);
}

template <class T> void rhojac( const NFSupport &supp, const gVector<T> &A, const T lambda,
             const gVector<T> &X, gVector<T> &V, int K )
{
  int n_strats =supp.TotalNumStrats() ;  
  int n_players = supp.Game().NumPlayers();
  
  gVector<T> Vxh(n_strats-n_players);
  gVector<T> Vx(n_strats-n_players);
  gVector<T> xh(n_strats-n_players);
  T dh = 1E-12;
  
  if (K == 1) {
    rho(supp, A, lambda+dh, X, Vxh);
    rho(supp, A, lambda, X, Vx);
    V = (Vxh-Vx)/dh;
  } // end of K == 1 i.e. w.r.t. lambda
  
  
  if ((K >= 2) && (K <= n_strats-n_players+1 )) {
    xh = X;
    xh[K-1] += dh;
    rho(supp, A, lambda, xh,Vxh);
    rho(supp, A, lambda, X, Vx);
    V = (Vxh-Vx)/dh;
  } // end of ((K >= 2) && (K <= n_strats-n_players+1 ))

} // end of rhojac


template <class T> void rho( const NFSupport &supp, const gVector<T> &A, const T lambda,
          const gVector<T> &X, gVector<T> &V   )
{
  T eps = 0.00;
  T tlambda = 1/(1-lambda) - 1; // switch in hompack3 as wells as here

  MixedProfile<T> sol(supp);
  int j = 1;
  for(int pl = 1;pl<=supp.Game().NumPlayers();pl++) {
    T resid = 1;
    int i;
    for(i=1;i<supp.NumStrats(pl);i++) {
      sol(pl,i) = X[j];
      resid -= X[j];
      j++;
    }
    sol(pl,i) =  resid;
  }

  j=1;
  for(int pl=1;pl<=supp.Game().NumPlayers();pl++)
    for(int i = 2; i <= supp.NumStrats(pl); i++) {
      T x =  (T) my_log((T)sol(pl,1),eps)- (T) my_log((T)sol(pl,i),eps) 
	- tlambda*((T)(sol.Payoff(pl,pl,1)-sol.Payoff(pl,pl,i)));
      V[j] = x;
      j++;
    }  
}

template <class T> void display_vector(const gVector<T> p,char * msg, int n,T ptag)
{
  int i;
  printf("%s:(",msg);
  for(i = 1; i < n; i++)
    printf("%4.2f,",p[i]);
  printf("%4.2f,%4.2f)\n",p[n],ptag);
}



#ifdef UNUSED
template <class T> void rhojact( const gVector<T> &A, const NFSupport &supp, 
             const gMatrix<T> &row_payoffs,
             const gMatrix<T> &col_payoffs, const T lambda,
             const gVector<T> &X, gVector<T> &V, int K )
{
  
  int n_rows = row_payoffs.NumRows();
  int n_cols = row_payoffs.NumColumns();
  
  
  gVector<T> p(n_rows-1);
  gVector<T> q(n_cols-1);
  
  // set up p (row player's probabilities over strategies
  // and q (column player's probabilities over strategies
  
  // X[1] .. X[n_rows-1] - p[1] ... p[n_rows-1]
  // X[n_rows] .. X[n_rows+n_cols-2] - q[1] ... q[n_cols-1]
  
  // remaining  p[n_rows] and q{n_cols] specified implicitly
  
  int i,j;
  for(i = 1;i < n_rows;i++) {
    p[i] = X[i];
  }
  
  for(i = 1;i < n_cols;i++) {
    q[i] = X[n_rows+i-1];
  }
  

  
  T result;
  T p_resid;
  T q_resid;
  T a,b;
  T sum;
  

// Calculate 1 - sum of other probabilities
  sum = 0.0;
  for(i = 1; i <= n_cols - 1; i++)
    sum += q[i];
  q_resid = 1.0 - sum;
  
  sum = 0.0;
  for(i = 1; i <= n_rows -1; i++)
    sum += p[i];
  p_resid = 1.0 - sum;
  
  // debugging 1/27/00 Vale Murthy
  // display_vector(p,"p",n_rows-1,p_resid);
  // display_vector(q,"q",n_cols-1,q_resid);
  // printf ("K=%d\n",K);
  
  if (K == 1) {
    //derivatives w.r.t lambda
    //V[1] ... V[n_rows-1]
    //= ln[p1]-ln[p2] - stuff, ..., ln{p1]-ln[p_nrows] -stuff
    
    
    a  = eu(q,row_payoffs,-1,n_cols-1);
    a  += q_resid*row_payoffs(1,n_cols);
    
    for(i = 2; i <= n_rows; i++) {
      
      b = eu(q,row_payoffs,-i,n_cols-1);
      b += q_resid*row_payoffs(i,n_cols);

      result = b-a;
      result /= pow(1-lambda,2.0); // Vale Murthy 3/6/00
      V[i-1] = result;
    }

     //derivatives w.r.t lambda
     //V[n_rows] ... V[n_rows + n_cols-2]


    a  = eu(p,col_payoffs,1,n_rows-1);
    a += p_resid*col_payoffs(n_rows,1);
    
    for(j = 2; j <= n_cols; j++) {
      b = eu(p,col_payoffs,j,n_rows-1);
      b += p_resid*col_payoffs(n_rows,j);
      result = b - a;
      result /= pow(1-lambda,2.0); // Vale Murthy 3/6/00
      V[n_rows + j - 2] = result;
    }
    
    
  } // end of K == 1 i.e. w.r.t. lambda
  
  
  // w.r.t p_1
  
  if (K == 2) {
    
    //V[1] ... V[n_rows-2] <-> ln(p_1)-ln(p_i) - lambda*stuff
    // w.r.t p_1
    for(i = 2; i <= n_rows-1; i++)
      V[i-1] = 1.0/p[1];
    
    
    //V[n_rows -1] = ln(p_1) - ln(1-p_1-...-p_(nrows-1)) - lambda*stuff;
    // w.r.t. p_1
    V[n_rows-1] = 1.0/p[1] + 1.0/p_resid;
    
    
    //V[n_rows] .. V[n_rows + n_cols - 2] <-> ln(q_1)-ln(q_j) - lambda*stuff
    /*
      stuff = sum(i=1,i=n_rows-1,p_i*(cPayoff(i,1)-cPayoff(i,j)))
      + (1-p_1-...-p_(n_rows-1))*(cPayoff(n_rows,1)-cPayoff(n_rows,j)
    */
    // w.r.t p_1
    
    for (j = 2; j <= n_cols; j++)
      {
	a = col_payoffs(1,1)-col_payoffs(1,j);
	b = -(col_payoffs(n_rows,1)-col_payoffs(n_rows,j));
	result = -(a+b);   // Vale Murthy 3/6/00
	result *= lambda/(1-lambda);
	V[n_rows + j - 2] = result;
      }
    
    
  } // end of K == 2 i.e. w.r.t p_1
  
  
  // w.r.t p_2 ... p_(nrows-1)
  if ( (K >= 3) && (K <= n_rows) ) {
    
    for (i = 1; i <= n_rows-1; i++)
      V[i] = 0.0;
    
    V[K-2] = -1.0/p[K-1];
    
    V[n_rows-1] = 1.0/p_resid;
    
    
    //ln(q_1) - ln(q_2) - lambda * stuff
    // w.r.t p_2 ... p_(n_rows-1)  <-> K = 3 ... K = n_rows
    
    for(j = 2; j <= n_cols;j++) {
      a = col_payoffs(K-1,1)-col_payoffs(K-1,j);
      b = -(col_payoffs(n_rows,1)-col_payoffs(n_rows,j));
      result = -(a+b); // Vale Murthy 3/6/00
      result *= lambda/(1-lambda);
      V[n_rows-1+j-1] = result;
    }
    
    
  } // end of ((K >= 3) && (K <= n_rows))
  
  int iloc = K - n_rows;
  
  
  // ln(p_1) - ln(p_2) - lambda*stuff
  // w.r.t. q_1 <-> K = n_rows + 1
  if (K == n_rows +1) {
    
    for(i = 2; i <= n_rows;i++)
      {
	a =  row_payoffs(1,iloc) - row_payoffs(i,iloc);
	b =  -( row_payoffs(1,n_cols)-row_payoffs(i,n_cols) );
	result = -(a+b); // Vale Murthy 3/6/00
	result *= lambda/(1-lambda);
	V[i-1] = result;
      }
    
    
    // ln(q_1) - ln(q_j) - lambda*stuff
    // w.r.t. q_1 <-> K = n_rows + 1
    for(j = 2; j < n_cols; j++)
      V[n_rows - 2+ j] = 1.0/q[1];
    
    V[n_rows -2 + n_cols] = 1.0/q[1]+1.0/q_resid;
    
    
  } // end of (K == n_rows +1)

  
  
  if  ( (K > n_rows+1) && (K <= n_rows + n_cols -1)  ) {
    
    
    // ln(p_1) - ln(p_i) - lambda*stuff
    // w.r.t. q_2 ... q_(n_cols-1) K = n_rows + 1 ... K = n_rows + n_cols -1
    
    for(i = 2; i <= n_rows;i++) {
      a =  row_payoffs(1,iloc) - row_payoffs(i,iloc);
      b =  -(row_payoffs(1,n_cols)-row_payoffs(i,n_cols));
      result = -(a+b);  // Vale Murthy 3/6/00
      result *= lambda/(1-lambda);
      V[i-1] = result;
    }
    
    //ln(q_1) - ln(q_j) - lambda * stuff
    // w.r.t. q_2 ... q_(n_cols-1)
    for(j = 2; j < n_cols; j++)
      V[n_rows -2 + j] = 0.0;
    
    V[n_rows-2+iloc] = -1.0/q[iloc];
    
    
    V[n_rows + n_cols - 2] = 1.0/q_resid;
    
  } // end of ( (K >= n_rows+1) && (K <= n_rows+ n_cols -1))
  
  /*
    for(i = 1;i <= n_cols + n_rows - 2;i++)
    printf("V_t[%d]=%6.3lf ",i,V[i]);
    printf("\n");
  */
  
} // end of rhojact

template <class T> T eu(const gVector<T> p,const gMatrix<T> M,
	  const int indicator,const int n)
{
  int i;
  T sum = 0,result= 0;
  
  if (indicator < 0) { // row product
    sum = 0.0;
    for(i = 1; i <= n; i++)
      sum += M(abs(indicator),i)*p[i];
    result = sum;
  }
  
  if (indicator > 0) { //  column product
    sum = 0.0;
    for(i = 1; i <= n; i++)
      sum += M(i,abs(indicator))*p[i];
    result = sum;
  }
  
  if (indicator == 0) printf("Error in eu! \n");
  
  return result;
}
#endif

// assumes that the row and column are consistently dimensioned
void echo_payoffs(const NFSupport &supp)
{
  NfgContIter iter(supp);
  
  do {
    gArray<int> strats = iter.Get();
    NFOutcome *out(iter.GetOutcome());
    gout << iter.Get() << ":  (";
    for(int pl=1;pl<=supp.Game().NumPlayers();pl++)
      gout << " " << supp.Game().Payoff(out, pl);
    gout << ")\n";
  }
  while(iter.NextContingency());
}

template <class T> void F( const gVector<T> &X, gVector<T> &V )  { }
template <class T> void Fjac( const gVector<T> &X, gVector<T> &V, int K )  { }

// Instantiations

template void display_vector(const gVector<double>,char *, int,double);
template double my_log(double,double);
#ifdef UNUSED
template void rhojact( const gVector<double> &, const NFSupport &, const gMatrix<double> &,
		       const gMatrix<double> &, const double,
		       const gVector<double> &, gVector<double> &, int);
template double eu(const gVector<double>,const gMatrix<double>,
		   const int,const int);
#endif

// template double tl(double);

template void rho( const NFSupport &, const gVector<double> &, const double,
          const gVector<double> &, gVector<double> &);
template void rhojac( const NFSupport &, const gVector<double> &, const double,
             const gVector<double> &, gVector<double> &, int);
template void F( const gVector<double> &, gVector<double> &);
template void Fjac( const gVector<double> &, gVector<double> &, int);