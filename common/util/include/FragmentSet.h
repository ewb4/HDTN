#ifndef FRAGMENT_SET_H
#define FRAGMENT_SET_H 1

/*
Manages contiguous data in a set.  Contiguous data that does not abut must be split up
into pairs of start and end indices called a data_fragment_t
Used by LtpFragmentSet and AGGREGATE CUSTODY SIGNAL(ACS) / CUSTODY TRANSFER ENHANCEMENT BLOCK(CTEB)
*/

#include <cstdint>
#include <vector>
#include <set>
#include "hdtn_util_export.h"

class HDTN_UTIL_EXPORT FragmentSet {
public:
    struct HDTN_UTIL_EXPORT data_fragment_t {
        uint64_t beginIndex;
        uint64_t endIndex;

        data_fragment_t(); //a default constructor: X()
        data_fragment_t(uint64_t paramBeginIndex, uint64_t paramEndIndex);
        ~data_fragment_t(); //a destructor: ~X()
        data_fragment_t(const data_fragment_t& o); //a copy constructor: X(const X&)
        data_fragment_t(data_fragment_t&& o); //a move constructor: X(X&&)
        data_fragment_t& operator=(const data_fragment_t& o); //a copy assignment: operator=(const X&)
        data_fragment_t& operator=(data_fragment_t&& o); //a move assignment: operator=(X&&)
        bool operator==(const data_fragment_t & o) const; //operator ==
        bool operator!=(const data_fragment_t & o) const; //operator !=
        bool operator<(const data_fragment_t & o) const; //operator < (no overlap no abut)
        static bool SimulateSetKeyFind(const data_fragment_t & key, const data_fragment_t & keyInSet);
    };

public:
    
    static void InsertFragment(std::set<data_fragment_t> & fragmentSet, data_fragment_t key);
    static bool ContainsFragmentEntirely(const std::set<data_fragment_t> & fragmentSet, const data_fragment_t & key);
    static bool DoesNotContainFragmentEntirely(const std::set<data_fragment_t> & fragmentSet, const data_fragment_t & key);
    static void RemoveFragment(std::set<data_fragment_t> & fragmentSet, const data_fragment_t & key);
    static void PrintFragmentSet(const std::set<data_fragment_t> & fragmentSet);
};

#endif // FRAGMENT_SET_H

