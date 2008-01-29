#pragma warning(disable:4786) 

#ifndef hashtable_h
#define hashtable_h

#include <string>
#include <vector>
using std::vector;
using std::string;


// QuadraticProbing Hash table class.
// Object must have operator!= and global function
//    unsigned int hash( const Object & x );
// CONSTRUCTION: with no parameters or another hash table.
//
// ******************PUBLIC OPERATIONS*********************
// void insert( x )       --> Insert x
// void remove( x )       --> Remove x
// Object find( x )       --> Return item that matches x
// void makeEmpty( )      --> Remove all items
// ******************ERRORS********************************

template <class Object>
class HashTable
{
 public:
  HashTable();
  
  int findPos( const Object & x );
  void makeEmpty( );
  void insert( const Object & x );
  void remove( const Object & x );
  Object * find( const Object & x );
  enum EntryType { ACTIVE, EMPTY, DELETED };
 private:
  struct HashEntry
  {
    Object    element;
    EntryType info;
    
    HashEntry( const Object & e = Object( ), EntryType i = EMPTY )
      : element( e ), info( i ) { }
  };
  
  int occupied;
  bool isActive( int currentPos ) const;
  void rehash( );
  bool isPrime( int n ) const;
  int nextPrime( int n ) const;

 protected:
  unsigned int hash( const string & key ) const;
  vector<HashEntry> array;
};

#include "hashtable.cpp"

#endif
