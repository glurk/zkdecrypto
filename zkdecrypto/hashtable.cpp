#ifndef hashtable_cpp
#define hashtable_cpp

#include "hashtable.h"

bool isPrime( int n );
int nextPrime( int n );

// Construct the hash table.
template <class Object>
HashTable<Object>::
HashTable()
  : array( nextPrime( 101 ) )
{
    makeEmpty( );
}

// Insert item x into the hash table. If the item is
// already present, do nothing
template <class Object>
void HashTable<Object>::
insert( const Object & x )
{
      // Insert x as active
    int currentPos = findPos( x );
    if( isActive( currentPos ) )
		return;
    array[ currentPos ] = HashEntry( x, ACTIVE );
    if( ++occupied > array.size( ) / 2 )
        rehash( );
}

// Expand the hash table.
template <class Object>
void HashTable<Object>::
rehash( )
{
    vector<HashEntry> oldArray = array;

      // Create new double-sized, empty table
    array.resize( nextPrime( 2 * oldArray.size( ) ) );
    for( int j = 0; j < array.size( ); j++ )
        array[ j ].info = EMPTY;

      // Copy table over
    makeEmpty( );
    for( int i = 0; i < oldArray.size( ); i++ )
        if( oldArray[ i ].info == ACTIVE )
            insert( oldArray[ i ].element );
}

// Hash function, can only handle strings.
// If you want to hash other objects you will have to 
// create a hash table for them
template <class Object>
unsigned int HashTable<Object>::
hash( const string & key ) const
{
    unsigned int hashVal = 0;
//	cout << key << "%";
    for( size_t i = 0; i < key.size(); i++ )
        hashVal =  ( hashVal << 5 ) ^ key[ i ] ^ hashVal;
    return hashVal;
}

// Method that performs quadratic probing resolution.
// Return the position where the search for x terminates.
template <class Object>
int HashTable<Object>::
findPos( const Object & x )
{
    int collisionNum = 0;
    int currentPos = hash( x ) % array.size( );
    while( array[ currentPos ].info != EMPTY &&
           array[ currentPos ].element != x )
    {
        currentPos += 2 * ++collisionNum - 1;  // Compute ith probe
        if( currentPos >= array.size( ) )
            currentPos -= array.size( );
    }
//	cout << currentPos << " ";
    return currentPos;
}

// Remove item x from the hash table.
template <class Object>
void HashTable<Object>::
remove( const Object & x )
{
    int currentPos = findPos( x );
    if( isActive( currentPos ) )
        array[ currentPos ].info = DELETED;
}

// Find item x in the hash table.
// Return a pointer to the matching item or 0 if not found
template <class Object>
Object * HashTable<Object>::
find( const Object & x )
{
    int currentPos = findPos( x );
    if( isActive( currentPos ) )
        return & (array[ currentPos ].element);
    else
        return 0;
}

// Make the hash table logically empty.
template <class Object>
void HashTable<Object>::
makeEmpty( )
{
    occupied = 0;
    for( int i = 0; i < (unsigned)array.size( ); i++ )
        array[ i ].info = EMPTY;
}

// Return true if currentPos exists and is active.
template <class Object>
bool HashTable<Object>::
isActive( int currentPos ) const
{
    return array[ currentPos ].info == ACTIVE;
}


// Internal method to test if a positive number is prime.
// Not an efficient algorithm.
template <class Object>
bool HashTable<Object>::
isPrime( int n ) const 
{
    if( n == 2 || n == 3 )
        return true;

    if( n == 1 || n % 2 == 0 )
        return false;

    for( int i = 3; i * i <= n; i += 2 )
        if( n % i == 0 )
            return false;

    return true;
}

// Internal method to return a prime number at least as large as n.
// Assumes n > 0.
template <class Object>
int HashTable<Object>::
nextPrime( int n ) const
{
    if( n % 2 == 0 )
        n++;

    for( ; !isPrime( n ); n += 2 )
        ;

    return n;
}

#endif
