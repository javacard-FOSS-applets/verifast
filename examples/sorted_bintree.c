#include "stdlib.h"
#include "assert.h"

struct tree{
  int value;
  struct tree *left;
  struct tree *right;
};

/*@
predicate tree(struct tree *t,bintree b)
  requires switch(b){
    case nil: return t==0;
    case cons(a,bl,br): return t->value |-> ?v &*& t->left |-> ?l &*& t->right |-> ?r 
			&*& malloc_block_tree(t) &*& tree(l,bl) &*& tree(r,br) &*& t!=0 &*& a==v ;
  }&*& inorder(b)==true;

inductive bintree = |nil |cons(int,bintree,bintree);

fixpoint bool t_contains(bintree b, int v) {
  switch (b) {
    case nil: return false;
    case cons(a,l,r): return (a==v ? true: (v < a? t_contains(l,v):t_contains(r,v)));
  }
}
fixpoint bintree tree_add(bintree b, int x) {
  switch (b) {
    case nil: return cons(x,nil,nil);
    case cons(v,l,r): return x < v? cons(v,tree_add(l,x),r):
			(x==v? cons(v,l,r):cons(v,l,tree_add(r,x)));
  }
}
fixpoint int max(bintree b){
  switch(b){
    case cons(a,bl,br): return (br==nil? a:max(br));
    case nil: return 0;
  }
}
fixpoint int min(bintree b){
  switch(b){
    case cons(a,bl,br): return (bl==nil? a:min(bl));
    case nil: return 0;
  }
}
fixpoint bintree tree_rem(bintree b, int x) {
  switch (b) {
    case cons(v,l,r): return l==nil&&r==nil&&x==v? nil:
			(x==v&&l==nil? r:
			(x==v&&r==nil? l:
			(x==v? cons(max(l),tree_rem(l,max(l)),r):
			x < v? cons(v,tree_rem(l,x),r):cons(v,l,tree_rem(r,x)) ))) ;
    case nil: return nil;
  }
}
fixpoint bool inorder(bintree b){
  switch(b){
	case nil: return true;
	case cons(a,bl,br): return (bl==nil? true:max(bl)<a)&& (br==nil? true: a < min(br))
		&& inorder(bl) && inorder(br);
  }
}
lemma void min_le_max(bintree b)
  requires inorder(b)==true &*& b!=nil;
  ensures min(b)<=max(b);
{
  switch(b){
	case nil:
	case cons(v,l,r):if(l!=nil||r!=nil){
			   if(l==nil){
			     min_le_max(r);
			   }else{
			     if(r!=nil){
			       min_le_max(r);
			     }
			     min_le_max(l);
			   }
			}
  }
}
lemma void contains_max(bintree r)
  requires r!=nil && inorder(r)==true;
  ensures t_contains(r,max(r))==true;
{
  switch(r){
	case nil:
	case cons(a,b,c):if(c!=nil){
			   min_le_max(c);
			   contains_max(c);
                           /* Needed by Redux. */ if (a == max(r)) {} else {}
                           /* Needed by Redux. */ if ((max(r) < a) == true) {} else {}
			 }
  }
}
lemma void contains_min(bintree r)
  requires r!=nil &*& inorder(r)==true;
  ensures t_contains(r,min(r))==true;
{
  switch(r){
	case nil:
	case cons(a,b,c):if(b!=nil){
			  min_le_max(b);
			  contains_min(b);
                          /* Needed by Redux. */ if (a == min(r)) {} else {}
                          /* Needed by Redux. */ if ((min(r) < a) == true) {} else {}
			 }
  }
}
lemma void max_conj_add(bintree l,int v,int x)
  requires x < v &*& (max(l)<v||l==nil) &*& inorder(l)==true;
  ensures max(tree_add(l,x))<v &*& inorder(l)==true;
{
  switch(l){
	case nil:
	case cons(a,b,c):if(x < a){
                          /* Needed by Redux. */ if (b == nil) {} else {}
			  max_conj_add(b,a,x);
			 }
			 if(a < x){
                          /* Needed by Redux. */ if (c == nil) {} else {}
			  max_conj_add(c,v,x);
			 }
                         /* Needed by Redux. */ if ((x < a) == true) {} else { if (x == a) {} else {} }
                         /* Needed by Redux. */ if (c == nil) {} else {}
                         /* Needed by Redux. */ if (x == a) {} else {}
                         /* Needed by Redux. */ if (tree_add(c, x) == nil) {} else {}
  }
}
lemma void min_conj_add(bintree r,int v,int x)
  requires v < x &*& (v < min(r)||r==nil) &*& inorder(r)==true;
  ensures v < min(tree_add(r,x)) &*& inorder(r)==true;
{
  switch(r){
	case nil:
	case cons(a,b,c):if(a < x){
			  min_conj_add(c,a,x);
			 }
			 if(x < a){
			  min_conj_add(b,v,x);
			 }
  }
}
lemma void max_conj_rem(bintree l,int v,int x)
  requires x < v &*& (max(l)<v||l==nil) &*& inorder(l)==true;
  ensures (max(tree_rem(l,x))<v||tree_rem(l,x)==nil) &*& inorder(l)==true;
{
  switch(l){
	case nil:
	case cons(a,b,c):if(x < a){
			  max_conj_rem(b,a,x);
			 }
			 if(a < x){
			  max_conj_rem(c,v,x);
			 }
  }
}

lemma void tree_add_inorder(bintree b, int x)
    requires inorder(b)==true &*& t_contains(b,x)==false;
    ensures inorder(tree_add(b,x))==true &*& t_contains(tree_add(b,x),x)==true;
{
    switch (b) {
        case nil:
        case cons(v,l,r):if(x < v){
			  max_conj_add(l,v,x);
			  tree_add_inorder(l,x);
			 }
			 if(v < x){
			  min_conj_add(r,v,x);
			  tree_add_inorder(r,x);
		  	 }
    }
}
lemma void min_all(bintree r,int x)
  requires t_contains(r,x)==true &*& inorder(r)==true;
  ensures min(r)<=x;
{
  switch(r){
	case nil:
	case cons(a,b,c):if(b!=nil){
			   contains_max(b);
			   min_all(b,max(b));
			 }
			 if(t_contains(b,x)){
			   min_all(b,x);
			 }
  }
}
lemma void max_all(bintree r,int x)
  requires inorder(r)==true &*& t_contains(r,x)==true &*& x!=max(r);
  ensures x < max(r);
{
  switch(r){
	case nil:
	case cons(a,b,c):if(c!=nil){
			   contains_min(c);
			   min_le_max(c);
			 }
			 if(t_contains(c,x)){
			   max_all(c,x);
			 }
  }
}
lemma void min_conj_rem(bintree r,int v,int x)
  requires v < x &*& (v < min(r)||r==nil) &*& inorder(r)==true;
  ensures (v < min(tree_rem(r,x))||tree_rem(r,x)==nil) &*& inorder(r)==true;
{
  switch(r){
	case nil:
	case cons(a,b,c):if(a < x){
			  min_conj_rem(c,a,x);
			 }
			 if(x < a){
			  min_conj_rem(b,v,x);
			 }
			 if(b!=nil&&c!=nil){
			  contains_max(b);
			  min_all(b,max(b));
			  min_conj_rem(b,v,max(b));
			 }
  }
}
lemma void contains_rem_max(bintree b)
  requires inorder(b)==true &*& b!=nil &*& tree_rem(b,max(b))!=nil &*& inorder(tree_rem(b,max(b)))==true;
  ensures t_contains(b,max(tree_rem(b,max(b))))==true;
{
  switch (b) {
        case nil:
        case cons(v,l,r):if(l==nil||r!=nil){
			   if(tree_rem(r,max(r))==nil){
			     min_le_max(r);
		  	     contains_max(tree_rem(b,max(b)));
			   }else{
			     min_le_max(r);
			     if(v!=max(tree_rem(b,max(b)))){
			       max_all(tree_rem(b,max(b)),v);
			     }
			     contains_rem_max(r);
			   }
			 }else{
			   contains_max(tree_rem(b,max(b)));
			 }
  }
}
lemma void tree_rem_inorder(bintree b, int x)
    requires inorder(b)==true &*& t_contains(b,x)==true;
    ensures inorder(tree_rem(b,x))==true&*& t_contains(tree_rem(b,x),x)==false;
{
    switch (b) {
        case nil:
        case cons(v,l,r):if(x < v){
			  max_conj_rem(l,v,x);
			  tree_rem_inorder(l,x);
			 }
		  	 if(v < x){
			  min_conj_rem(r,v,x);
			  tree_rem_inorder(r,x);
			 }
			 if(x==v){
				if(l==nil && r!=nil){
					if(t_contains(r,x)==false){
					  contains_min(r);
					}else{
					  min_all(r,x);
					}
				}if(r==nil&&l!=nil){
					if(t_contains(l,x)==false){
					  contains_min(l);
					}else{
					  max_all(l,x);
					}
				}if(r!=nil&&l!=nil){
				   if(tree_rem(l,max(l))!=nil){
				     contains_max(l);
				     tree_rem_inorder(l,max(l));
				     contains_rem_max(l);
				     if(max(l)!=max(tree_rem(l,max(l)))){
				       max_all(l,max(tree_rem(l,max(l))));
				       if(t_contains(r,x)==false){
				         contains_min(r);
				       }else{
				         min_all(r,x);
				       }
				     }else{
				       contains_max(tree_rem(l,max(l)));
				     }
				   }else{
				     if(t_contains(r,x)==false){
				       contains_min(r);
				     }else{
				       min_all(r,x);
				     }
				   }	
			    }
		  	}
  }
}
@*/
struct tree *init_tree(int x)
  //@ requires true;
  //@ ensures tree(result,cons(x,nil,nil));
{
  struct tree *t = malloc(sizeof(struct tree));
  if(t!=0){
    t->value=x;
    t->left=0;
    t->right=0;
    //@ struct tree *l = t->left;
    //@ close tree(l,nil);
    //@ struct tree *r = t->right;
    //@ close tree(r,nil);
    //@ int v = t->value;
    //@ close tree(t,cons(x,nil,nil));
    return t;
  }else{
	abort();
  }
}

void free_tree(struct tree *t)
  //@ requires tree(t,?b);
  //@ ensures emp;
{
  if(t==0){
    //@ open tree(t,b);
  }else{
    //@ open tree(t,b);
    struct tree *l=t->left;
    struct tree *r=t->right;
    free_tree(l);
    free_tree(r);
    free(t);
  }
}
bool contains(struct tree *t,int x)
  //@ requires tree(t,?b);
  //@ ensures tree(t,b) &*& result==t_contains(b,x);
{
  if(t==0){
    //@ open tree(t,b);
    //@ close tree(t,nil);
    return false;
  }else{
    //@ open tree(t,b);
    int v=t->value;
    struct tree *l=t->left;
    struct tree *r=t->right;
    if(v==x){
      //@close tree(t,b);
      return true;
    }else if(x < v){
      bool temp1=contains(l,x);
      //@close tree(t,b);
      return temp1;
    }else{
      bool temp2=contains(r,x);
      //@close tree(t,b);
      return temp2;
    }
  }
}
void add(struct tree *t, int x)
  //@ requires tree(t,?b) &*& b!=nil &*& false==t_contains(b,x) &*& inorder(b)==true;
  //@ ensures tree(t,tree_add(b,x)) &*& inorder(tree_add(b,x))==true;
 {
  //@ open tree(t,b);
  int v=t->value;
  struct tree *l=t->left;
  //@ open tree(l,?bl);
  //@ close tree(l,bl);
  struct tree *r=t->right;
  //@ open tree(r,?br);
  //@ close tree(r,br);
  if(x < v){
    if(l!=0){
      add(l,x);
      //@ tree_add_inorder(b,x);
      //@ close tree(t,cons(v,tree_add(bl,x),br));
    }else{
      struct tree *temp=init_tree(x);
      t->left=temp;
      //@ open tree(l,bl);
      //@ close tree(t,cons(v,cons(x,nil,nil),br));
      //@ tree_add_inorder(b,x);
    }
  }else{
    if(v < x){
      if(r!=0){
        add(r,x);
        //@ tree_add_inorder(b,x);
        //@ close tree(t,cons(v,bl,tree_add(br,x)));	
      }else{
        struct tree *temp=init_tree(x);
        t->right=temp;
        //@ open tree(r,br);
        //@ close tree(t,cons(v,bl,cons(x,nil,nil)));
      }
    }
  }
}
int maximum(struct tree *t)
  //@ requires tree(t,?b) &*& b!=nil &*& inorder(b)==true;
  //@ ensures result==max(b) &*& tree(t,b);
{
  //@ open tree(t,b);
  int v=t->value;
  struct tree *r=t->right;
  //@ open tree(r,?br);
  //@ close tree(r,br);
  if(r==0){
    //@ close tree(t,b);
    return v;
  }else{
    int m= maximum(r);
    //@ close tree(t,b);
    return m;
  }
}
struct tree* remove(struct tree *t, int x)
  //@ requires tree(t,?b) &*& b!=nil &*& true==t_contains(b,x) &*& inorder(b)==true;
  //@ ensures tree(result,tree_rem(b,x))&*& inorder(tree_rem(b,x))==true &*& false==t_contains(tree_rem(b,x),x);
{
  //@ open tree(t,b);
  int v=t->value;
  struct tree *l=t->left;
  //@ open tree(l,?bl);
  //@ close tree(l,bl);
  struct tree *r=t->right;
  //@ open tree(r,?br);
  //@ close tree(r,br);
  //@ tree_rem_inorder(b,x);
  if(x < v){
    if(l!=0){
      struct tree *temp=remove(l,x);
      t->left=temp;
      //@ close tree(t,cons(v,tree_rem(bl,x),br));
      return t;
    }
  }
  if(v < x){
    if(r!=0){
      struct tree *temp=remove(r,x);
      t->right=temp;
      //@ close tree(t,cons(v,bl,tree_rem(br,x)));
      return t;
    }
  }
  if(v==x){
    if(l!=0&&r==0){
      //@ open tree(r,nil);
      free(t);
      return l;
    }
    if(l==0&&r==0){
      //@ close tree(t,b);
      free_tree(t);
      //@ close tree(0,nil);
      return 0;
    }
    if(l==0&&r!=0){
      //@ open tree(l,nil);
      free(t);
      return r;
    }
    if(l!=0&&r!=0){
      struct tree *temp=0;
      int m=maximum(l);
      t->value=m;
      //@ contains_max(bl);
      temp=remove(l,m);
      t->left=temp;
      //@ close tree(t,cons(m,tree_rem(bl,m),br));
      return t;
    }
  }
}
int main()
  //@ requires true;
  //@ ensures true;
{
  struct tree *t1=0;
  struct tree *t2=0;
  struct tree *t3=0;
  bool a=false;
  bool b=false;
  bool c=false;
  bool d=false;
  bool e=false;
  bool f=false;

  t1 = init_tree(3);

  b= contains(t1,2);
  assert(!b);
  add(t1,2);

  a= contains(t1,2);
  assert(a);
  
  c= contains(t1,3);
  assert(c);

  t2=remove(t1,3);
  d= contains(t2,3);
  assert(!d);
  
  add(t2,3);
  e= contains(t2,2);
  assert(e);
  
  t3=remove(t2,3);
  f= contains(t3,3);
  assert(!f);

  free_tree(t3);

  return 0;
}