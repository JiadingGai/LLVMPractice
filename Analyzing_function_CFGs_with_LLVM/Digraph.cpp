#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>

class BasicBlock {
public:
  int i;
};

class Digraph 
{
  int V;
  int E;

  std::map<BasicBlock*, std::vector<BasicBlock*> > adj;

public:
  typedef std::map<BasicBlock*, std::vector<BasicBlock*> >::iterator GraphIteratorTy;
  typedef std::vector<BasicBlock *>::iterator AdjIteratorTy;
  
  Digraph(int V)
  {
    this->V = V;
    this->E = 0;
  }

  int getNumNodes() { return V; }
  int getNumEdges() { return E; }

  GraphIteratorTy begin() { return adj.begin(); }
  GraphIteratorTy end() { return adj.end(); }

  void addEdge(BasicBlock *v, BasicBlock *w)
  {
    adj[v].push_back(w);
    E++;
  }

  std::vector<BasicBlock *> getAdjs(BasicBlock *v)
  {
    return adj[v];
  }

  Digraph* reverseGraph()
  {
    Digraph *R = new Digraph(V);
    
    for (GraphIteratorTy I = begin(), E = end(); I != E; ++I) {
      BasicBlock *A = I->first;
      std::vector<BasicBlock *> B = I->second;

      for (AdjIteratorTy II = B.begin(), EE = B.end(); II != EE; ++I) {
        R->addEdge(*II, A);
      }
    }

    return R;
  }
};

class DirectedDFS
{
  std::map<BasicBlock *, bool> Marked;

  void dfs(Digraph *G, BasicBlock* v) 
  {
    Marked[v] = true;

    std::vector<BasicBlock *> Adjv = G->getAdjs(v);
    for (Digraph::AdjIteratorTy II = Adjv.begin(), EE = Adjv.end(); II != EE; ++II) {
      BasicBlock *X = *II;
        
      if (!Marked[X]) dfs(G, X);
    }
  }

public:

  DirectedDFS(Digraph *G, std::vector<BasicBlock *> &S)
  {
    for (Digraph::AdjIteratorTy II = S.begin(), EE = S.end(); II != EE; ++II) {
      BasicBlock *Source = *II;

      if (!Marked[Source]) 
        dfs(G, Source);
    }
  }

  // Find vertices in G that are reachable from s.
  DirectedDFS(Digraph *G, BasicBlock* s)
  {
    dfs(G, s);
  }

  bool marked(BasicBlock* v) { return Marked[v]; }
};

class TransitiveClosure
{
  std::map<BasicBlock*, DirectedDFS*> all;

  typedef std::map<BasicBlock*, DirectedDFS*>::iterator TransitiveClosureIteratorTy;

public:


  TransitiveClosure(Digraph *G)
  {
    for (Digraph::GraphIteratorTy I = G->begin(), E = G->end(); I != E; ++I) {
      BasicBlock *A = I->first;
     
      all[A] = new DirectedDFS(G, A);
    }
  }

  bool reachable(BasicBlock *v, BasicBlock *w)
  {
    return all[v]->marked(w);
  }

  ~TransitiveClosure() {
    for (TransitiveClosureIteratorTy It = all.begin(), Ie = all.end(); It != Ie; ++It) {
      delete It->second;
    }
  }
};

int main()
{
  int V = 4;

  Digraph *G = new Digraph(V);

  BasicBlock a, b, c, d;
  a.i = 0;
  b.i = 1;
  c.i = 2;
  d.i = 3;

  G->addEdge(&a, &b);
  G->addEdge(&a, &c);
  G->addEdge(&c, &b);
  G->addEdge(&a, &d);

  DirectedDFS DD(G, &c);

  printf("%d,%d,%d,%d\n", DD.marked(&a), DD.marked(&b), DD.marked(&c), DD.marked(&d));

  TransitiveClosure TC(G);

  for (Digraph::GraphIteratorTy I = G->begin(), E = G->end(); I != E; ++I) {
    BasicBlock *A = I->first;
    
    for (Digraph::GraphIteratorTy J = G->begin(), F = G->end(); J!= F; ++J) {
      BasicBlock *B = J->first;

      if (TC.reachable(A, B)) {
        std::cout << A->i << " reaches " << B->i << std::endl;
      }
    }
  }

  delete G;

  return 0;
}
