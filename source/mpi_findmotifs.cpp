// Implement your solutions in this file
#include "findmotifs.h"
#include "hamming.h"
#include <mpi.h>
#include <iostream>
#include <bitset>
#include <stdlib.h>

#define MPI_TAG_SLAVE_TERMINATING 10
#define MPI_TAG_MASTER_NO_NUMS_TO_SEND 20


// structure to do a depth first search of the 2^l tree
typedef struct
{
  bits_t  val;
  unsigned int  flag;
  unsigned int pos;
} node_t;



std::vector<bits_t> findmotifs_worker(const unsigned int n,
				      const unsigned int l,
				      const unsigned int d,
				      const bits_t* input,
				      const unsigned int startbitpos,
				      bits_t start_value)
{
  std::vector<bits_t> results;

  // TODO: implement your solution here
  bits_t first = input[0];

  bits_t msk=0;
  msk |= ((bits_t)0xffff << 48);
  msk |= ((bits_t)0xffff << 32);
  msk |= ((bits_t)0xffff << 16);
  msk |= (bits_t)(0xffff);

  //  std::cout << "DEBUG:findmotifs_worker start with start_value=" << start_value <<", position ="<<startbitpos<< std::endl ;

  // flip the bits of s1 to get the list when hamming distance is less than 1
  std::vector<node_t*> stack;
  node_t  *firstNode = (node_t*)malloc(sizeof(node_t));
  firstNode->val=start_value;
  // depth first search of the 2^l tree
  firstNode->pos = startbitpos;
  // set as 1 as this string has already been explored in the master to this point
  firstNode->flag = 1;
  stack.push_back(firstNode);


  while ( !stack.empty()) {

    bool searchTree=true;
    node_t *popNode = (node_t*)stack.back();
    node_t *tempN =  (node_t*)malloc(sizeof(node_t));
    tempN->val = popNode->val;
    tempN->pos = popNode->pos;
    tempN->flag = popNode->flag;
    stack.pop_back();
    //debug
    free(popNode);
    popNode = NULL;

    //std::cout << "DEBUG:findmotifs_worker popped value " << (std::bitset<64>) tempN->val<<"="<<tempN->val<<","<< tempN->pos<< std::endl ;

    unsigned int h = 0;

    // flag indicates that this bit string has been checked
    //  before
    if ( tempN->flag != 1 ) {

      // check the hamming distance of the bit string with the first number
      h =  hamming(tempN->val,first);
  
      //std::cout << "DEBUG: findmotifs_worker hamming distance=" <<h<<", bits="<<(std::bitset<64>)first<<"="<<first<<","<< tempN->pos << std::endl ;
      //std::cout << "DEBUG: findmotifs_worker hamming distance=" <<h<<", bits="<<(std::bitset<64>)tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;
  
      // if hamming distance is not more than d
      // check for every other number in the input list
      if ( h <= d ){
  
	bool lessThanD=true;
  
	// check if hamming distance for all other n-1 ints is no more than d
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
	if ( lessThanD ) {
          //std::cout << "DEBUG: findmotifs_worker FOUND hamming distance=" <<h<<", bits="<<(std::bitset<64>)tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;
          results.push_back(tempN->val);
	}
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

      // this bit string is the different than the original so set flag to 0
      tempNn->flag  = 0;

      //std::cout << "DEBUG:findmotifs_worker adding next bit flipped "  << (std::bitset<64>) tempNn->val<<"="<< tempNn->val<<","<< tempNn->pos << std::endl ;

      stack.push_back(tempNn);

      // this bit string is the same as the original so set flag to 1
      tempN->flag  = 1;

      //std::cout << "DEBUG:findmotifs_worker adding same with position incremented"  << (std::bitset<64>) tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;

      stack.push_back(tempN);

    }
    else {

      free(tempN);

    }

  }

  //free(firstNode);
  //debug
  return results;
}



void worker_main()
{
  // TODO:

  // get rank
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::cout<<"DEBUG:worker_main start slave pid="<<rank<<std::endl;


  // ----------------------------------------------------------
  // 1.) receive input from master (including n, l, d, input, master-depth)
  // ----------------------------------------------------------
  unsigned int n;
  unsigned int l;
  unsigned int d;
  unsigned int master_depth;

  MPI_Recv(&n, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_Recv(&l, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_Recv(&d, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_Recv(&master_depth, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  bits_t *input = (bits_t*)malloc(sizeof(bits_t)*n);
  MPI_Recv(&(input[0]), n, MPI_UINT64_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  for ( unsigned int i=0;i<n;i++){
    //    std::cout<<"DEBUG:worker_main slave pid="<<rank<<" recieved input from master i="<<i<<", val = "<<input[i]<<std::endl;
  }

  //  std::cout<<"DEBUG:worker_main slave pid="<<rank<<", recieved input from master n,l,d,master_depth="<<n<<","<<l<<","<<d<<","<<master_depth<<std::endl;


  // ----------------------------------------------------------
  // 2. receive number to check explore from master
  //    or receive the no more numbers from master
  // ----------------------------------------------------------
  // 2.) while the master is sending work:
  //      a) receive subproblems
  //      b) locally solve (using findmotifs_worker(...))
  //      c) send results to master

  bool wait_for_more_numbers_from_master = true;

  while ( wait_for_more_numbers_from_master ) {

    bits_t nm;
    MPI_Status  mpi_status;

    // receive the unit64_t buffer from master
    MPI_Recv(&nm, 1 , MPI_UINT64_T, 0, MPI_ANY_TAG, MPI_COMM_WORLD,&mpi_status);

    // if the tag indicates master has no more numbers to send
    // else read the number from the master add the bits_t from the buffer to results
    if ( mpi_status.MPI_TAG == MPI_TAG_MASTER_NO_NUMS_TO_SEND ) { 

      wait_for_more_numbers_from_master = false;
      //      std::cout <<"DEBUG:worker_main slave pid="<<rank<<", no tree to explore message received from master "<<std::endl;

    }
    else {

      //      std::cout<<"DEBUG:worker_main slave pid="<<rank<<" recvd from master, val ="<<nm<<std::endl;

      std::vector<bits_t> results = findmotifs_worker(n,l,d,input,(unsigned int)((sizeof(bits_t)*8) - l + master_depth),nm);
   
      // -----------------------------------------------------
      //   send results back to the master
      // -----------------------------------------------------
      if ( results.size() ) {

	MPI_Send(&(results[0]), results.size(), MPI_UINT64_T, 0, 0, MPI_COMM_WORLD);

	//	std::cout <<"DEBUG:worker_main slave pid="<<rank<<", sending results to master, "<<"n="<<n<<std::endl;

      }

    }

  }

  //----------------------------------------------------------
  // 3.) you have to figure out a communication protocoll:
  //     - how does the master tell the workers that no more work will come?
  //       (i.e., when to break loop 2)
  //----------------------------------------------------------
  bits_t rank64 = rank;

  MPI_Send(&rank64, 1, MPI_UINT64_T, 0, MPI_TAG_SLAVE_TERMINATING, MPI_COMM_WORLD);

  //  std::cout <<"DEBUG:worker_main slave pid="<<rank<<", sending terminating message to master, "<<std::endl;


  //----------------------------------------------------------
  // 4.) clean up: e.g. free allocated space
  //----------------------------------------------------------
  free(input);

}





std::vector<bits_t> findmotifs_master(const unsigned int n,
                                      const unsigned int l,
                                      const unsigned int d,
                                      const bits_t* input,
                                      const unsigned int till_depth,
				      const int p)
{
  std::vector<bits_t> results;
  
  // TODO: implement your solution here

  // TODO: implement your solution here
  //  std::cout << "DEBUG:findmotifs_master starting with " << ",n="<<n<<",l="<<l<<",d="<<d<<",till_depth="<<till_depth<< std::endl;

  int pid = 0;

  bits_t msk=0;
  msk |= ((bits_t)0xffff << 48);
  msk |= ((bits_t)0xffff << 32);
  msk |= ((bits_t)0xffff << 16);
  msk |= (bits_t)(0xffff);

  bits_t first = input[0];

  //std::cout << "DEBUG:findmotifs_master fstart with irst=" << first << std::endl ;

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

    //std::cout << "DEBUG:findmotifs_master popped value" << (std::bitset<64>) tempN->val<<"="<<tempN->val<< std::endl ;

    unsigned int h = 0;

    // flag indicates that this bit string has been checked
    //  before
    if ( tempN->flag != 1 ) {

      // check the hamming distance of the bit string with the first number
      h =  hamming(tempN->val,first);
  
      //std::cout << "DEBUG: findmotifs_master hamming distance=" <<h<<", bits="<<(std::bitset<64>)first<<"="<<first<<","<< tempN->pos << std::endl ;
      //std::cout << "DEBUG: findmotifs_master hamming distance=" <<h<<", bits="<<(std::bitset<64>)tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;
  
      // if hamming distance is not more than d
      // check for every other number in the input list
      if ( h <= d ){
  
	bool lessThanD=true;
  
	// check if hamming distance for all other n-1 ints is no more than d
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
	if ( lessThanD ) {
          //std::cout << "DEBUG: findmotifs_master FOUND hamming distance=" <<h<<", bits="<<(std::bitset<64>)tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;
          results.push_back(tempN->val);
	}
  
      }

    }


    // continue to further explore the bit string if hamming distance with first 
    // is less than d
    // there are bits left to flip

    tempN->pos++;

    if ( searchTree && (h < d) && (tempN->pos <= 64) ){

      // explore this bit string only till a depth of till_depth elements
      //  after that send it to the slave for further exploration
      if ( tempN->pos <=  ((sizeof(bits_t)*8) - l + till_depth) ) {

	node_t *tempNn = (node_t*)malloc(sizeof(node_t));

	tempNn->pos = tempN->pos;

	tempNn->val  = tempN->val xor (((bits_t)1)<<((sizeof(bits_t)*8)-tempN->pos));

	// this bit string is the different than the original so set flag to 0
	tempNn->flag  = 0;

	//std::cout << "DEBUG:findmotifs_master adding with next bit flipped "  << (std::bitset<64>) tempNn->val<<"="<< tempNn->val<<","<< tempNn->pos << std::endl ;

	stack.push_back(tempNn);

	// this bit string is the same as the original so set flag to 1
	tempN->flag  = 1;

	//std::cout << "DEBUG:findmotifs_master adding same with position incremented "  << (std::bitset<64>) tempN->val<<"="<< tempN->val<<","<< tempN->pos << std::endl ;

	stack.push_back(tempN);
      }
      else {
      
	// pick the slave from 1 to p-1 on a round robin
	pid++; 
	pid = pid%p;
	if ( pid == 0 ) pid++;

	//	std::cout << "DEBUG:findmotifs_master sending "  << (std::bitset<64>) tempN->val<<"="<< tempN->val<<", to slave pid="<< pid << std::endl ;

	MPI_Send(&(tempN->val),1,MPI_UINT64_T,pid,0,MPI_COMM_WORLD);

	free(tempN);

      }

    }
    else {

      free(tempN);

    }

  }

  //free(firstNode);

  return results;

}



std::vector<bits_t> master_main(unsigned int n, unsigned int l, unsigned int d,
                                const bits_t* input, unsigned int master_depth)
{

  // TODO
  std::cout << "DEBUG:master_main start master with "<<"n="<<n<<", l="<<l<<", d="<<d<<", master_depth="<<master_depth<<std::endl;

  // get size
  int p;
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  //----------------------------------------------------------------------
  // 1.) send input to all workers (including n, l, d, input, depth)
  //----------------------------------------------------------------------
  for(int i = 1;i<p;i++) {

    MPI_Send(&n, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
    MPI_Send(&l, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
    MPI_Send(&d, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
    MPI_Send(&master_depth, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);

    bits_t * in = (bits_t *) malloc(sizeof(bits_t) * n);
    for(unsigned int k = 0; k<n ; k++) {
      in[k] = input[k];
    }

    MPI_Send(&(in[0]), n, MPI_UINT64_T, i, 0, MPI_COMM_WORLD);
    free(in);
    in = NULL;

    //    std::cout <<"DEBUG:main_master sent to slave pid="<<i<<" - information n,l,d,master_depth"<<std::endl;
  }

  //----------------------------------------------------------------------
  // 2.) solve problem till depth `master_depth` and then send subproblems
  //     to the workers and receive solutions in each communication
  //     Use your implementation of `findmotifs_master(...)` here.
  //----------------------------------------------------------------------
  std::vector<bits_t> results;
  results = findmotifs_master(n,l,d,input,master_depth,p);

  //----------------------------------------------------------------------
  // 3.) send no more trees to all workers 
  //----------------------------------------------------------------------
  for(int i = 1;i<p;i++) {

    bits_t empty = 0;

    MPI_Send(&empty, 1, MPI_UINT64_T, i, MPI_TAG_MASTER_NO_NUMS_TO_SEND, MPI_COMM_WORLD);

    //    std::cout <<"DEBUG:main_master sending no more trees to slave pid="<<i<<std::endl;

  }

  //----------------------------------------------------------------------
  // 3.) receive last round of solutions
  // 4.) terminate (and let the workers know)
  //----------------------------------------------------------------------
  int slave_terminated_count = 0;

  while ( slave_terminated_count < (p-1)) {

    int cnt;
    MPI_Status  mpi_status;
 
    // Probe the message sent from slave to know the size of the buffer
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

    // from the probe status get the size of the buffer
    MPI_Get_count(&mpi_status, MPI_UINT64_T, &cnt);

    // allocate the buffer space
    bits_t* recvBuffer = (bits_t*)malloc(sizeof(bits_t) * cnt);

    // receive the buffer from slave
    MPI_Recv(recvBuffer, cnt , MPI_UINT64_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,&mpi_status);

    // if the tag indicates slave is terminating keep the count,
    // else add the bits_t from the buffer to results
    if ( mpi_status.MPI_TAG == MPI_TAG_SLAVE_TERMINATING ) { 

      slave_terminated_count++;

      std::cout <<"DEBUG:main_master master recved message from terminating slave pid="<<mpi_status.MPI_SOURCE
		<<", count of slaves terminated ="<<slave_terminated_count<<std::endl;

    }
    else {

      for (int i=0;i<cnt;i++) {

	results.push_back(recvBuffer[i]);

	//	std::cout<<"DEBUG:main_master received results val ="<<recvBuffer[i]<<" from slave pid="<<mpi_status.MPI_SOURCE<<std::endl;

      }

    }

    free(recvBuffer);

  }

  return results;

}
