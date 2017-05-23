// Implement your solutions in this file
#include "findmotifs.h"
#include "hamming.h"
#include <iostream>
#include <bitset>
#include <stdlib.h>

// structure to do a depth first search of the 2^l tree
typedef struct 
{
  bits_t  val;
  unsigned int  flag;
  unsigned int pos;
} node_t;


// implements the sequential findmotifs function
std::vector<bits_t> findmotifs(unsigned int n, unsigned int l,
                               unsigned int d, const bits_t* input)
{
  // If you are not familiar with C++ (using std::vector):
  // For the output (return value) `result`:
  //                  The function asks you to return all values which are
  //                  of a hamming distance `d` from all input values. You
  //                  should return all these values in the return value
  //                  `result`, which is a std::vector.
  //                  For each valid value that you find (i.e., each output
  //                  value) you add it to the output by doing:
  //                      result.push_back(value);
  //                  Note: No other functionality of std::vector is needed.
  // You can get the size of a vector (number of elements) using:
  //                      result.size()

  // create an empty vector
  std::vector<bits_t> results;

  // TODO: implement your solution here
  //std::cout << "DEBUG:findmotiffs" << "n="<<n<<"l="<<l<<"d="<<d<< std::endl;

  bits_t msk=0;
  msk |= ((bits_t)0xffff << 48);
  msk |= ((bits_t)0xffff << 32);
  msk |= ((bits_t)0xffff << 16);
  msk |= (bits_t)(0xffff);

  bits_t first = input[0];

  //std::cout << "DEBUG:findmotifs first=" << first << std::endl ;

  // flip the bits of s1 to get the list when hamming distance is less than 1
  std::vector<node_t*> stack;
  node_t  *firstNode = (node_t*)malloc(sizeof(node_t));
  firstNode->val=first;
  // depth first search of the 2^l tree
  firstNode->pos = (sizeof(bits_t)*8)-l;
  firstNode->flag = 0;
  stack.push_back(firstNode);

  while ( !stack.empty()) {

    bool searchTree=true;
    node_t *popNode = (node_t*)stack.back();
    node_t *tempN =  (node_t*)malloc(sizeof(node_t));
    tempN->val = popNode->val;
    tempN->pos = popNode->pos;
    tempN->flag = popNode->flag;
    stack.pop_back();
    free(popNode);
    popNode = NULL;

    //std::cout << "DEBUG:findmotifs popped value" << (std::bitset<64>) tempN->val<<"="<<tempN->val<< std::endl ;

    unsigned int h = 0;

    // flag indicates that this bit string has been checked
    //  before
    if ( tempN->flag != 1 ) {

      h =  hamming(tempN->val,first);
  
      //std::cout << "DEBUG:findmotifs  hamming distance=" <<h<<", bits="<<(std::bitset<64>)first<<"="<<first<<","<< tempN->pos << std::endl ;
      //std::cout << "DEBUG:findmotifs  hamming distance=" <<h<<", bits="<<(std::bitset<64>)tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;
  
      // check the hamming distance of the bit string with the first number      
      if ( h <= d ){
  
	bool lessThanD=true;
  
	// if hamming distance is not more than d
	// check for every other number in the input list
	for (unsigned int x=1;x<n;x++) {
  
          // if hamming dist > d for any 1 then this bit string not to be added to result
          if ( hamming(tempN->val,input[x]) > d) {
  
	    lessThanD=false;
  
	    // if the hamming distnce of > d comes from the part of the bit string
	    // till pos explored so far, then we do not need to explore the string further 
	    unsigned int N = (sizeof(bits_t)*8)-tempN->pos;
  
	    bits_t nbit =  (tempN->val&(msk<<N)) | ( input[x] & ((((bits_t)1)<< N) - 1));
  
	    if ( hamming(input[x],nbit) > d )
	      searchTree=false;

	    break;
	  }
  
	}
  
	// hamming distance no more than d for all the numbers
	// keep this bit string in the result
	if ( lessThanD )
          results.push_back(tempN->val);
  
      }

    }


    // continue to further explore the bit string if hamming distance with first 
    // is less than d
    // there are bits left to flip

    tempN->pos++;

    if ( searchTree && (h < d) && (tempN->pos <= 64) ){

      node_t *tempNn = (node_t*)malloc(sizeof(node_t));

      tempNn->pos = tempN->pos;

      tempNn->val  = tempN->val xor (((bits_t)1)<<((sizeof(bits_t)*8)-tempN->pos));

      tempNn->flag  = 0;

      //std::cout << "DEBUG:findmotifs  adding"  << (std::bitset<64>) tempNn->val<<"="<< tempNn->val<<","<< tempNn->pos << std::endl ;

      stack.push_back(tempNn);

      // this bit string is the same as the original so set flag to 1
      tempN->flag  = 1;

      //std::cout << "DEBUG:findmotifs  adding"  << (std::bitset<64>) tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;

      stack.push_back(tempN);

    } else {

      free(tempN);
    }
     

  }

  //free(firstNode);

  return results;
}
